/* 
 *	HT Editor
 *	htpeexp.cc
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
#include "htdata.h"
#include "htdebug.h"
#include "htendian.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeexp.h"
#include "htstring.h"
#include "httag.h"
#include "log.h"
#include "pe_analy.h"
#include "snprintf.h"
#include "stream.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

ht_view *htpeexports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return NULL;

	dword sec_rva, sec_size;
	sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].address;
	sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].size;
	if (!sec_rva || !sec_size) return NULL;

	int h0=new_timer();
	start_timer(h0);

	dword *efunct=NULL, *enamet=NULL;
	word *eordt=NULL;
	ht_mem_file *efile;
	ht_streamfile *origfile = file;
	const char *filename = file->get_filename();
	char *esectionbuf;
	char eline[256];
	FILEOFS ename_ofs;
	char *ename;
	bool *lord;

	ht_group *g = NULL;
	bounds c;
	ht_statictext *head;
	ht_pe_export_viewer *v = NULL;
	
/* get export directory offset */
	/* 1. get export directory rva */
	FILEOFS eofs;
	dword erva=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].address;
	dword esize=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].size;
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, erva, &eofs)) goto pe_read_error;
	LOG("%s: PE: reading export directory at offset %08x, rva %08x, size %08x...", filename, eofs, erva, esize);

/* make a memfile out of this section */
	efile=new ht_mem_file();
	efile->init(eofs, esize, FAM_READ | FAM_WRITE);
	esectionbuf=(char*)malloc(esize);
	file->seek(eofs);
	file->read(esectionbuf, esize);
	efile->write(esectionbuf, esize);
	file=efile;
	free(esectionbuf);
/* read export directory header */
	PE_EXPORT_DIRECTORY edir;
	file->seek(eofs);
	file->read(&edir, sizeof edir);
	create_host_struct(&edir, PE_EXPORT_DIRECTORY_struct, little_endian);
/* get export filename */
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.name_address, &ename_ofs)) goto pe_read_error;
	file->seek(ename_ofs);
	ename=fgetstrz(file);
/* read in function entrypoint table */
	FILEOFS efunct_ofs;
	efunct=(dword*)malloc(edir.function_count*sizeof *efunct);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.function_table_address, &efunct_ofs)) goto pe_read_error;
	file->seek(efunct_ofs);
	file->read(efunct, edir.function_count*sizeof *efunct);
	for (UINT i=0; i<edir.function_count;i++) {
		efunct[i] = create_host_int(efunct+i, sizeof *efunct, little_endian);
	}
/* read in name address table */
	FILEOFS enamet_ofs;
	enamet=(dword*)malloc(edir.name_count*sizeof *enamet);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.name_table_address, &enamet_ofs)) goto pe_read_error;
	file->seek(enamet_ofs);
	file->read(enamet, edir.name_count*sizeof *enamet);
	for (UINT i=0; i<edir.name_count; i++) {
		enamet[i] = create_host_int(enamet+i, sizeof *enamet, little_endian);
	}
/* read in ordinal table */
	FILEOFS eordt_ofs;
	eordt=(word*)malloc(edir.name_count*sizeof *eordt);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.ordinal_table_address, &eordt_ofs)) goto pe_read_error;
	file->seek(eordt_ofs);
	file->read(eordt, edir.name_count*sizeof *eordt);
	for (UINT i=0; i<edir.name_count; i++) {
		eordt[i] = create_host_int(eordt+i, sizeof *eordt, little_endian);
	}

	lord=(bool*)malloc(sizeof *lord*edir.function_count);
	memset(lord, 0, sizeof *lord*edir.function_count);
	for (UINT i=0; i < edir.name_count; i++) {
		if (eordt[i] < edir.function_count) {
			lord[eordt[i]]=true;
		}
	}

/* exports by ordinal */
	for (UINT i=0; i<edir.function_count; i++) {
		if (lord[i]) continue;

		RVA f = efunct[i];

		ht_pe_export_function *efd = new ht_pe_export_function(f, i+edir.ordinal_base);
		pe_shared->exports.funcs->insert(efd);
	}
	free(lord);
/* exports by name */
	for (UINT i=0; i < edir.name_count; i++) {
		UINT o = eordt[i];
		RVA f = efunct[o];
		FILEOFS en;
		if (!pe_rva_to_ofs(&pe_shared->sections, *(enamet+i), &en)) goto pe_read_error;
		file->seek(en);
		char *s = fgetstrz(file);

		ht_pe_export_function *efd = new ht_pe_export_function(f, o+edir.ordinal_base, s);
		pe_shared->exports.funcs->insert(efd);
		free(s);
	}

// sdgfdg
	c = *b;
	c.x = 0;
	c.y = 0;
	g = new ht_group();
	g->init(&c, VO_RESIZE, DESC_PE_EXPORTS"-g");

	c.y = 1;
	c.h--;
	v = new ht_pe_export_viewer();
	v->init(&c, group);

	c.y = 0;
	c.h = 1;
	ht_snprintf(eline, sizeof eline, "* PE export directory at offset %08x (dllname = %s)", eofs, ename);
	head = new ht_statictext();
	head->init(&c, eline, align_left);

	g->insert(head);
	g->insert(v);
//
	for (UINT i=0; i<pe_shared->exports.funcs->count(); i++) {
		ht_pe_export_function *e = (ht_pe_export_function *)pe_shared->exports.funcs->get(i);
		char ord[32], addr[32];
		ht_snprintf(ord, sizeof ord, "%04x", e->ordinal);
		ht_snprintf(addr, sizeof addr, "%08x", e->address);
		v->insert_str(i, ord, addr, e->byname ? e->name : "<by ordinal>");
	}
//
	v->update();

	g->setpalette(palkey_generic_window_default);

	stop_timer(h0);
//	LOG("%s: PE: %d ticks (%d msec) to read exports", filename, get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

	if (ename) free(ename);

	efile->done();
	delete efile;

	if (efunct) free(efunct);
	if (enamet) free(enamet);
	if (eordt) free(eordt);
	pe_shared->v_exports=v;
	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%s: PE export directory seems to be corrupted.", origfile->get_filename());
	if (efunct) free(efunct);
	if (enamet) free(enamet);
	if (eordt) free(eordt);
	if (g) {
		g->done();
		delete g;
	}
	if (v) {
		v->done();
		delete v;
	}
	return NULL;
}

format_viewer_if htpeexports_if = {
	htpeexports_init,
	NULL
};

/*
 *	CLASS ht_pe_export_viewer
 */

void	ht_pe_export_viewer::init(bounds *b, ht_format_group *fg)
{
	ht_text_listbox::init(b, 3, 2, LISTBOX_QUICKFIND);
	options |= VO_BROWSABLE;
	desc = strdup(DESC_PE_EXPORTS);
	format_group = fg;
}

void	ht_pe_export_viewer::done()
{
	ht_text_listbox::done();
}

char *ht_pe_export_viewer::func(UINT i, bool execute)
{
	ht_text_listbox_sort_order sortord;
	switch (i) {
		case 2:
			if (execute) {
				sortord.col = 0;
				sortord.compare_func = strcmp;
				sort(1, &sortord);
			}
			return "sortord";
		case 3:
			if (execute) {
				sortord.col = 1;
				sortord.compare_func = strcmp;
				sort(1, &sortord);
			}
			return "sortva";
		case 4:
			if (execute) {
				sortord.col = 2;
				sortord.compare_func = strcmp;
				sort(1, &sortord);
			}
			return "sortname";
	}
	return NULL;
}

void ht_pe_export_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_funcexec:
			if (func(msg->data1.integer, 1)) {
				clearmsg(msg);
				return;
			}
			break;
		case msg_funcquery: {
			char *s=func(msg->data1.integer, 0);
			if (s) {
				msg->msg=msg_retval;
				msg->data1.str=s;
			}
			break;
		}
		case msg_keypressed: {
			if (msg->data1.integer == K_Return) {
				select_entry(e_cursor);
				clearmsg(msg);
			}
			break;
		}
	}
	ht_text_listbox::handlemsg(msg);
}

void ht_pe_export_viewer::select_entry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;

	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	ht_pe_export_function *e = (ht_pe_export_function*)pe_shared->exports.funcs->get(i->id);
	if (!e) return;
	if (pe_shared->v_image) {
		ht_aviewer *av = (ht_aviewer*)pe_shared->v_image;
		PEAnalyser *a = (PEAnalyser*)av->analy;
		Address *addr;
		if (pe_shared->opt_magic == COFF_OPTMAGIC_PE32) {
			addr = a->createAddress32(e->address+pe_shared->pe32.header_nt.image_base);
		} else {
			addr = a->createAddress64(to_qword(e->address)+pe_shared->pe64.header_nt.image_base);
		}
		if (av->gotoAddress(addr, NULL)) {
			app->focus(av);
			vstate_save();
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
			errorbox("can't follow: %s %y is not valid !", "export address", addr);
		}
		delete addr;
	} else errorbox("can't follow: no image viewer");
}

/*
 *	class ht_pe_export_function
 */

ht_pe_export_function::ht_pe_export_function(RVA addr, UINT ord)
{
	ordinal = ord;
	address = addr;
	byname = false;
}

ht_pe_export_function::ht_pe_export_function(RVA addr, UINT ord, char *n)
{
	ordinal = ord;
	name = ht_strdup(n);
	address = addr;
	byname = true;
}

ht_pe_export_function::~ht_pe_export_function()
{
	if ((byname) && (name)) free(name);
}

