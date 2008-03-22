/* 
 *	HT Editor
 *	htpeexp.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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
#include "data.h"
#include "htdebug.h"
#include "endianess.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeexp.h"
#include "strtools.h"
#include "httag.h"
#include "log.h"
#include "pe_analy.h"
#include "snprintf.h"
#include "stream.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

static ht_view *htpeexports_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return NULL;

	uint32 sec_rva, sec_size;
	sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].address;
	sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].size;
	if (!sec_rva || !sec_size) return NULL;

	int h0=new_timer();
	start_timer(h0);

	uint32 *efunct=NULL, *enamet=NULL;
	uint16 *eordt=NULL;
	String filename;
	char *esectionbuf = NULL;
	char eline[256];
	FileOfs ename_ofs;
	char *ename;
	bool *lord;
	String s;
	ConstMemMapFile *efile = NULL;

	ht_group *g = NULL;
	Bounds c;
	ht_statictext *head;
	ht_pe_export_viewer *v = NULL;
	
	file->getFilename(filename);

	/* get export directory offset */
	/* 1. get export directory rva */
	FileOfs eofs;
	uint32 erva=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].address;
	uint32 esize=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].size;
	
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, erva, &eofs)) goto pe_read_error;
	LOG("%y: PE: reading export directory at offset 0x%08qx, rva %08x, size %08x...", &filename, eofs, erva, esize);

	/* make a memfile out of this section */
	esectionbuf = ht_malloc(esize);
	file->seek(eofs);
	file->readx(esectionbuf, esize);

	efile = new ConstMemMapFile(esectionbuf, esize, eofs);
	file = efile;
	
	/* read export directory header */
	PE_EXPORT_DIRECTORY edir;
	file->seek(eofs);
	file->readx(&edir, sizeof edir);
	createHostStruct(&edir, PE_EXPORT_DIRECTORY_struct, little_endian);

	/* get export filename */
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.name_address, &ename_ofs)) goto pe_read_error;
	file->seek(ename_ofs);
	ename = efile->fgetstrz();

	/* read in function entrypoint table */
	FileOfs efunct_ofs;
	efunct = ht_malloc(edir.function_count*sizeof *efunct);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.function_table_address, &efunct_ofs)) goto pe_read_error;
	file->seek(efunct_ofs);
	file->readx(efunct, edir.function_count*sizeof *efunct);
	for (uint i=0; i<edir.function_count;i++) {
		efunct[i] = createHostInt(efunct+i, sizeof *efunct, little_endian);
	}

	if (edir.name_count) {
		/* read in name address table */
		FileOfs enamet_ofs;
		enamet = ht_malloc(edir.name_count*sizeof *enamet);
		if (!pe_rva_to_ofs(&pe_shared->sections, edir.name_table_address, &enamet_ofs)) goto pe_read_error;
		file->seek(enamet_ofs);
		file->readx(enamet, edir.name_count*sizeof *enamet);
		for (uint i=0; i<edir.name_count; i++) {
			enamet[i] = createHostInt(enamet+i, sizeof *enamet, little_endian);
		}

		/* read in ordinal table */
		FileOfs eordt_ofs;
		eordt = ht_malloc(edir.name_count*sizeof *eordt);
		if (!pe_rva_to_ofs(&pe_shared->sections, edir.ordinal_table_address, &eordt_ofs)) goto pe_read_error;
		file->seek(eordt_ofs);
		file->readx(eordt, edir.name_count*sizeof *eordt);
		for (uint i=0; i<edir.name_count; i++) {
			eordt[i] = createHostInt(eordt+i, sizeof *eordt, little_endian);
		}

		lord = ht_malloc(sizeof *lord*edir.function_count);
		memset(lord, 0, sizeof *lord*edir.function_count);
		for (uint i=0; i < edir.name_count; i++) {
			if (eordt[i] < edir.function_count) {
				lord[eordt[i]]=true;
			}
		}

		/* exports by ordinal */
		for (uint i=0; i<edir.function_count; i++) {
			if (lord[i]) continue;

			RVA f = efunct[i];

			ht_pe_export_function *efd = new ht_pe_export_function(f, i+edir.ordinal_base);
			pe_shared->exports.funcs->insert(efd);
		}
		free(lord);

		/* exports by name */
		for (uint i=0; i < edir.name_count; i++) {
			uint o = eordt[i];
			RVA f = efunct[o];
			FileOfs en;
			if (!pe_rva_to_ofs(&pe_shared->sections, *(enamet+i), &en)) goto pe_read_error;
			file->seek(en);
			efile->readStringz(s);

			ht_pe_export_function *efd = new ht_pe_export_function(f, o+edir.ordinal_base, s.contentChar());
			pe_shared->exports.funcs->insert(efd);
		}
	} else {
		/* exports by ordinal */
		for (uint i=0; i < edir.function_count; i++) {
			RVA f = efunct[i];

			ht_pe_export_function *efd = new ht_pe_export_function(f, i+edir.ordinal_base);
			pe_shared->exports.funcs->insert(efd);
		}
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
	ht_snprintf(eline, sizeof eline, "* PE export directory at offset %08qx (dllname = %s)", eofs, ename);
	head = new ht_statictext();
	head->init(&c, eline, align_left);

	g->insert(head);
	g->insert(v);
//
	for (uint i=0; i<pe_shared->exports.funcs->count(); i++) {
		ht_pe_export_function *e = (ht_pe_export_function *)(*pe_shared->exports.funcs)[i];
		char ord[32], addr[32];
		ht_snprintf(ord, sizeof ord, "%04x", e->ordinal);
		ht_snprintf(addr, sizeof addr, "%08x", e->address);
		v->insert_str(i, ord, addr, e->byname ? e->name : "<by ordinal>");
	}
//
	v->update();

	g->setpalette(palkey_generic_window_default);

	stop_timer(h0);
//	LOG("%y: PE: %d ticks (%d msec) to read exports", filename, get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

	free(ename);

	free(efunct);
	free(enamet);
	free(eordt);
	pe_shared->v_exports = v;
	delete efile;
	free(esectionbuf);
	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%y: PE export directory seems to be corrupted.", &filename);
	free(efunct);
	free(enamet);
	free(eordt);
	if (g) {
		g->done();
		delete g;
	}
	if (v) {
		v->done();
		delete v;
	}
	delete efile;
	free(esectionbuf);
	return NULL;
}

format_viewer_if htpeexports_if = {
	htpeexports_init,
	NULL
};

/*
 *	CLASS ht_pe_export_viewer
 */

void	ht_pe_export_viewer::init(Bounds *b, ht_format_group *fg)
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

const char *ht_pe_export_viewer::func(uint i, bool execute)
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
		case 4:
			if (execute) {
				sortord.col = 1;
				sortord.compare_func = strcmp;
				sort(1, &sortord);
			}
			return "sortva";
		case 5:
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
			const char *s=func(msg->data1.integer, 0);
			if (s) {
				msg->msg=msg_retval;
				msg->data1.cstr=s;
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

bool ht_pe_export_viewer::select_entry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;

	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	ht_pe_export_function *e = (ht_pe_export_function*)(*pe_shared->exports.funcs)[i->id];
	if (!e) return true;
	if (pe_shared->v_image) {
		ht_aviewer *av = (ht_aviewer*)pe_shared->v_image;
		PEAnalyser *a = (PEAnalyser*)av->analy;
		Address *addr;
		if (pe_shared->opt_magic == COFF_OPTMAGIC_PE32) {
			addr = a->createAddress32(e->address+pe_shared->pe32.header_nt.image_base);
		} else {
			addr = a->createAddress64(e->address+pe_shared->pe64.header_nt.image_base);
		}
		if (av->gotoAddress(addr, NULL)) {
			app->focus(av);
			vstate_save();
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
			errorbox("can't follow: %s %y is not valid!", "export address", addr);
		}
		delete addr;
	} else errorbox("can't follow: no image viewer");
	return true;
}

/*
 *	class ht_pe_export_function
 */

ht_pe_export_function::ht_pe_export_function(RVA addr, uint ord)
{
	ordinal = ord;
	address = addr;
	byname = false;
}

ht_pe_export_function::ht_pe_export_function(RVA addr, uint ord, const char *n)
{
	ordinal = ord;
	name = ht_strdup(n);
	address = addr;
	byname = true;
}

ht_pe_export_function::~ht_pe_export_function()
{
	if (byname) free(name);
}
