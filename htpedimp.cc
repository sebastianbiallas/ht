/* 
 *	HT Editor
 *	htpedimp.cc
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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
#include "htanaly.h"
#include "htctrl.h"
#include "htendian.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeimp.h"
#include "htpedimp.h"
#include "httag.h"
#include "htstring.h"
#include "log.h"
#include "pe_analy.h"
#include "snprintf.h"
#include "stream.h"

#include <stdlib.h>

ht_view *htpedelayimports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return NULL;

	RVA sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].address;
	UINT sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].size;
	if (!sec_rva || !sec_size) return NULL;

	int h0=new_timer();
	start_timer(h0);

	UINT dll_count=0;
	UINT function_count=0;

	ht_group *g;
	bounds c;
	ht_statictext *head;

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_PE_DIMPORTS"-g");

	c.y++;
	c.h--;
	ht_pe_dimport_viewer *v=new ht_pe_dimport_viewer();
	v->init(&c, DESC_PE_DIMPORTS, group);

	PE_DELAY_IMPORT_DESCRIPTOR dimport;
	UINT dll_index;
	char iline[256];

	c.y--;
	c.h=1;
/* get delay import directory offset */
	/* 1. get import directory rva */
	FILEOFS iofs;
	RVA irva=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].address;
//	UINT isize=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].size;
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, irva, &iofs)) goto pe_read_error;
	LOG("%s: PE: reading delay-import directory at offset %08x, rva %08x, size %08x...", file->get_filename(), iofs, irva, pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].size);

/*** read delay import directory ***/
	dll_index=0;

	while (1) {
		file->seek(iofs);
		file->read(&dimport, sizeof dimport);
		create_host_struct(&dimport, PE_DELAY_IMPORT_DESCRIPTOR_struct, little_endian);
		if (!dimport.name) break;
		dword base = dimport.attributes & 1 ? 0 : pe_shared->pe32.header_nt.image_base;

		FILEOFS dllname_ofs;
		if (!pe_rva_to_ofs(&pe_shared->sections, dimport.name-base, &dllname_ofs)) goto pe_read_error;
		file->seek(dllname_ofs);
		char *dllname=fgetstrz(file);
		ht_pe_import_library *lib=new ht_pe_import_library(dllname);
		pe_shared->dimports.libs->insert(lib);
		dll_count++;

		FILEOFS ntofs, atofs;
		UINT nva, ava;
		if (!pe_rva_to_ofs(&pe_shared->sections, dimport.delay_int-base, &ntofs)) goto pe_read_error;
		if (!pe_rva_to_ofs(&pe_shared->sections, dimport.delay_iat-base, &atofs)) goto pe_read_error;
		while (1) {
			ht_pe_import_function *func;
			file->seek(ntofs);
			file->read(&nva, 4);
			nva = create_host_int(&nva, 4, little_endian);
			if (!nva) break;
			function_count++;
			file->seek(atofs);
			file->read(&ava, 4);
			ava = create_host_int(&ava, 4, little_endian);
			if (nva & 0x80000000) {
/* import by ordinal */
				func=new ht_pe_import_function(dll_index, ava, nva&0xffff);
			} else {
				FILEOFS nofs;
/* import by name */
				if (!pe_rva_to_ofs(&pe_shared->sections, nva-base, &nofs)) goto pe_read_error;
				word hint=0;
				file->seek(nofs);
				file->read(&hint, 2);
				hint = create_host_int(&hint, 2, little_endian);
				char *name=fgetstrz(file);
				func=new ht_pe_import_function(dll_index, ava, name, hint);
				free(name);
			}
			pe_shared->dimports.funcs->insert(func);
			ntofs+=4;
			atofs+=4;
		}

		free(dllname);
		iofs+=sizeof dimport;
		dll_index++;
	}

	stop_timer(h0);
//	LOG("%s: PE: %d ticks (%d msec) to read delay-imports", file->get_name(), get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

	ht_snprintf(iline, sizeof iline, "* PE delay-import directory at offset %08x (%d delay-imports from %d libraries)", iofs, function_count, dll_count);

	head=new ht_statictext();
	head->init(&c, iline, align_left);

	g->insert(head);
	g->insert(v);
//
	for (UINT i=0; i<pe_shared->dimports.funcs->count(); i++) {
		ht_pe_import_function *func = (ht_pe_import_function*)pe_shared->dimports.funcs->get(i);
		assert(func);
		ht_pe_import_library *lib = (ht_pe_import_library*)pe_shared->dimports.libs->get(func->libidx);
		assert(lib);
		char addr[32], name[256];
		ht_snprintf(addr, sizeof addr, "%08x", func->address);
		if (func->byname) {
			ht_snprintf(name, sizeof name, "%s", func->name.name);
		} else {
			ht_snprintf(name, sizeof name, "%04x (by ordinal)", func->ordinal);
		}
		v->insert_str(i, lib->name, addr, name);
	}
//
	v->update();

	g->setpalette(palkey_generic_window_default);

	pe_shared->v_dimports=v;

	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%s: PE delay-import directory seems to be corrupted.", file->get_filename());
	v->done();
	delete v;
	g->done();
	delete g;
	return NULL;
}

format_viewer_if htpedelayimports_if = {
	htpedelayimports_init,
	NULL
};

/*
 *	CLASS ht_pe_dimport_viewer
 */

void ht_pe_dimport_viewer::select_entry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;

	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	ht_pe_import_function *e = (ht_pe_import_function*)pe_shared->dimports.funcs->get(i->id);
	if (!e) return;
	if (pe_shared->v_image) {
		ht_aviewer *av = (ht_aviewer*)pe_shared->v_image;
		PEAnalyser *a = (PEAnalyser*)av->analy;
		Address *addr = a->createAddress32(e->address);
		if (av->gotoAddress(addr, NULL)) {
			app->focus(av);
			vstate_save();
		} else errorbox("can't follow: %s %08x is not valid !", "delay-import address", addr);
		delete addr;
	} else errorbox("can't follow: no image viewer");
}

