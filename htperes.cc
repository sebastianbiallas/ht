/*
 *	HT Editor
 *	htperes.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "formats.h"
#include "htapp.h"
#include "htctrl.h"
#include "htdialog.h"
#include "htendian.h"
#include "hthex.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htnewexe.h"
#include "htobj.h"
#include "htpe.h"
#include "htperes.h"
#include "htstring.h"
#include "httree.h"
#include "log.h"
#include "pestruct.h"
#include "stream.h"

#include <string.h>

static int_hash restypes[] = {
	{0x0001, "cursors"},
	{0x0002, "bitmaps"},
	{0x0003, "icons"},
	{0x0004, "menus"},
	{0x0005, "dialogs"},
	{0x0006, "string tables"},
	{0x0007, "font directories"},
	{0x0008, "fonts"},
	{0x0009, "accelerators"},
	{0x000a, "custom resource data"},
	{0x000b, "message tables"},
	{0x000c, "group cursors"},
	{0x000e, "group icons"},
	{0x0010, "version information"},
	{0x0011, "dialog includes"},
	{0x0013, "pnp"},
	{0x0014, "vxd"},
	{0x0015, "animated cursors"},
	{ 0 }
};

static int_hash languages[] = {
	{7,  "german"},
	{9,  "english"},
	{10, "spanish"},
	{12, "french"},
	{16, "italian"},
	{ 0 }
};

class ht_pe_resource_leaf: public ht_data {
public:
	dword offset;
	dword size;
};

static ht_streamfile *peresource_file;
static FILEOFS peresource_dir_ofs;
static ht_static_treeview *peresource_tree;
static char peresource_string[128];
static pe_section_headers *peresource_section_headers;

static void read_resource_dir(void *node, int ofs, int level)
{
	if (level>2) return; 		/* no deep recursions please (for
							   currupted resource directories) */
	PE_RESOURCE_DIRECTORY dir;
// get directory
	peresource_file->seek(peresource_dir_ofs+ofs);
	if (peresource_file->read(&dir, sizeof dir) != (sizeof dir)) return;
	create_host_struct(&dir, PE_RESOURCE_DIRECTORY_struct, little_endian);
// get entries
	PE_RESOURCE_DIRECTORY_ENTRY entry;
	for (int i=0; i<dir.name_count+dir.id_count; i++) {
		peresource_file->seek(peresource_dir_ofs+ofs+sizeof dir+i*8);
		peresource_file->read(&entry, sizeof entry);
		create_host_struct(&entry, PE_RESOURCE_DIRECTORY_ENTRY_struct, little_endian);
		char *rm = peresource_string;
		if (entry.offset_to_directory & 0x80000000) {
			bool hasname = entry.name & 0x80000000;
			PE_RESOURCE_DIRECTORY subdir;
			peresource_file->seek(peresource_dir_ofs+entry.offset_to_directory & 0x7fffffff);
			peresource_file->read(&subdir, sizeof subdir);
			create_host_struct(&subdir, PE_RESOURCE_DIRECTORY_struct, little_endian);
			if (hasname) {
				peresource_file->seek(peresource_dir_ofs+entry.name & 0x7fffffff);
				char *name = getstrw(peresource_file);
				rm += sprintf(rm, "%s [%d]", name, subdir.name_count+subdir.id_count);
				free(name);
			} else {
				char *s = (!level) ? s = matchhash(entry.name & 0xffff, restypes) : NULL;
				if (s) {
					rm += sprintf(rm, "ID %04x, %s [%d]", entry.name & 0xffff, s, subdir.name_count+subdir.id_count);
				} else {
					rm += sprintf(rm, "ID %04x [%d]", entry.name & 0xffff, subdir.name_count+subdir.id_count);
				}
			}
			void *n = peresource_tree->add_child(node, peresource_string);
			read_resource_dir(n, entry.offset_to_directory & 0x7fffffff, level+1);
		} else {
			char *s = matchhash((char)entry.name, languages);
			if (s) {
				rm += sprintf(rm, "resource, %s (%04x) ", s, entry.name & 0xffff);
			} else {
				rm += sprintf(rm, "resource, unknown language (%04x) ", entry.name & 0xffff);
			}

			PE_RESOURCE_DATA_ENTRY data;
			peresource_file->seek(peresource_dir_ofs+entry.offset_to_directory);
			peresource_file->read(&data, sizeof data);
			create_host_struct(&data, PE_RESOURCE_DATA_ENTRY_struct, little_endian);
			
			ht_pe_resource_leaf *xdata = NULL;
			FILEOFS dofs=0;
			if (pe_rva_to_ofs(peresource_section_headers, data.offset_to_data, &dofs)) {
				xdata = new ht_pe_resource_leaf();
				xdata->offset = dofs;
				xdata->size = data.size;
				rm += sprintf(rm, "offset %08x", dofs);
			} else {
				rm += sprintf(rm, "offset ? (rva %08x, currupt)", data.offset_to_data);
			}
			sprintf(rm, " size %08x", data.size);
			peresource_tree->add_child(node, peresource_string, xdata);
		}
	}
}

static ht_view *htperesources_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32 && pe_shared->opt_magic!=COFF_OPTMAGIC_PE64) return NULL;

	bool pe32 = (pe_shared->opt_magic==COFF_OPTMAGIC_PE32);

	dword sec_rva, sec_size;
	if (pe32) {
		sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].address;
		sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].size;
	} else {
		sec_rva = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].address;
		sec_size = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].size;
	}
	if (!sec_rva || !sec_size) return NULL;

	ht_static_treeview *t=new ht_pe_resource_viewer();
	t->init(b, DESC_PE_RESOURCES);

	void *root;
/* get resource directory offset */
	/* 1. get resource directory rva */
	FILEOFS iofs;
	dword irva;
	if (pe32) {
		irva=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].address;
//		dword isize=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].size;
	} else {
		irva=pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_RESOURCE].address;
	}
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, irva, &iofs)) goto pe_read_error;

	LOG("%s: PE: reading resource directory at offset %08x, rva %08x", file->get_filename(), iofs, irva);

	peresource_file=file;
	peresource_dir_ofs=iofs;
	peresource_tree=t;
	peresource_section_headers=&pe_shared->sections;

	root=t->add_child(0, "pe resources");

	read_resource_dir(root, 0, 0);

	t->adjust(root, true);
	t->update();
	pe_shared->v_resources = t;
	return t;
pe_read_error:
	errorbox("%s: PE resource directory seems to be corrupted.", file->get_filename());
	t->done();
	delete t;
	return NULL;
}

format_viewer_if htperesources_if = {
	htperesources_init,
	0
};

/*
 *	CLASS ht_pe_resource_viewer
 */

void ht_pe_resource_viewer::init(bounds *b, char *desc)
{
	ht_static_treeview::init(b, desc);
	VIEW_DEBUG_NAME("ht_pe_resource_viewer");
}

void ht_pe_resource_viewer::done()
{
	ht_static_treeview::done();
}

void ht_pe_resource_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_vstate_restore:
			vstate_restore((ht_data*)msg->data1.ptr);
			clearmsg(msg);
			return;
	}
	ht_static_treeview::handlemsg(msg);
}

void ht_pe_resource_viewer::select_node(void *node)
{
	static_node *s=(static_node*)node;

	if (s->data) {
		ht_group *vr_group=group;
		while (strcmp(vr_group->desc, VIEWERGROUP_NAME)) vr_group=vr_group->group;
		ht_view *c=vr_group->getfirstchild();
		ht_format_viewer *hexv = NULL;
		while (c) {
			if (c->desc && (strcmp(c->desc, DESC_HEX)==0)) {
				hexv=(ht_format_viewer*)c;
				break;
			}
			c=c->next;
		}
		if (hexv) {
			vstate_save();
			hexv->goto_offset(((ht_pe_resource_leaf*)s->data)->offset, false);
			hexv->pselect_set(((ht_pe_resource_leaf*)s->data)->offset,
				((ht_pe_resource_leaf*)s->data)->offset+((ht_pe_resource_leaf*)s->data)->size);
			app->focus(hexv);
		}
	}
}

class ht_pe_resource_viewer_vstate: public ht_data {
public:
	void *node;
};

ht_data *ht_pe_resource_viewer::vstate_create()
{
	ht_pe_resource_viewer_vstate *v = new ht_pe_resource_viewer_vstate();
	v->node = get_cursor_node();
	return v;
}

bool ht_pe_resource_viewer::vstate_save()
{
	ht_data *vs = vstate_create();
	if (vs) {
		htmsg m;
		m.msg = msg_vstate_save;
		m.type = mt_empty;
		m.data1.ptr = vs;
		m.data2.ptr = this;
		app->sendmsg(&m);
		return true;
	}
	return false;
}

void ht_pe_resource_viewer::vstate_restore(ht_data *d)
{
	ht_pe_resource_viewer_vstate *v = new ht_pe_resource_viewer_vstate();
	goto_node(v->node);
}
