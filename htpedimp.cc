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

#include "htapp.h"
#include "htendian.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeimp.h"
#include "htpedimp.h"
#include "stream.h"
#include "httag.h"
#include "htstring.h"
#include "formats.h"

#include <stdlib.h>

ht_view *htpedelayimports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return 0;

	dword sec_rva, sec_size;
	sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].address;
	sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].size;
	if (!sec_rva || !sec_size) return 0;

	int h0=new_timer();
	start_timer(h0);

	int dll_count=0;
	int function_count=0;
//	ht_cached_stream *cached_file=0;

	ht_group *g;
	bounds c;
	ht_statictext *head;

	ht_pe_import_mask *m=0;
	
	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, "pe/delay-imports-g");

	c.y++;
	c.h--;
	ht_pe_dimport_viewer *v=new ht_pe_dimport_viewer();
	v->init(&c, DESC_PE_DIMPORTS, VC_SEARCH, file, group);

	PE_DELAY_IMPORT_DESCRIPTOR dimport;
	int dll_index;
	dword f;
	char iline[256];

	c.y--;
	c.h=1;
/* get delay import directory offset */
	/* 1. get import directory rva */
	FILEOFS iofs;
	dword irva=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].address;
//	dword isize=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].size;
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, irva, &iofs)) goto pe_read_error;
	LOG("%s: PE: reading delay-import directory at offset %08x, rva %08x, size %08x...", file->get_filename(), iofs, irva, pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_DELAY_IMPORT].size);

//	cached_file=new ht_cached_stream(file, 128*1024, CACHED_STREAM_CACHE_READS, 2048);
//     file=cached_file;		/* we dont need to care about "file" */

/*** read delay import directory ***/
	dll_index=0;
	f=0;

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
		dword nva, ava;
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
				delete name;
			}
			pe_shared->dimports.funcs->insert(func);
			ntofs+=4;
			atofs+=4;
			f++;
		}

		delete dllname;
		iofs+=sizeof dimport;
		dll_index++;
	}

	stop_timer(h0);
//	LOG("%s: PE: %d ticks (%d msec) to read delay-imports", file->get_name(), get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

//     delete cached_file;

	sprintf(iline, "Library        FT_VA    Name/Ordinal    %d delay-imports from %d libraries", function_count, dll_count);
	head=new ht_statictext();
	head->init(&c, iline, align_left);

	sprintf(iline, "* PE delay-import directory at offset %08x", iofs);

	m=new ht_pe_import_mask();
	m->init(file, &pe_shared->dimports, iline);

	v->insertsub(m);

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	pe_shared->v_dimports=v;

	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%s: PE delay-import directory seems to be corrupted.", file->get_filename());
//	if (cached_file) delete cached_file;
	v->done();
	delete v;
	if (m) {
		m->done();
		delete m;
	}		
	g->done();
	delete g;
	return 0;
}

format_viewer_if htpedelayimports_if = {
	htpedelayimports_init,
	0
};

/*
 *	CLASS ht_pe_dimport_viewer
 */

void ht_pe_dimport_viewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *group)
{
	ht_uformat_viewer::init(b, desc, caps, file, group);
	VIEW_DEBUG_NAME("ht_pe_dimport_viewer");
}

void ht_pe_dimport_viewer::done()
{
	ht_uformat_viewer::done();
}

char *ht_pe_dimport_viewer::func(UINT i, bool execute)
{
	switch (i) {
		case 3:
			if (execute) {
				((ht_pe_import_mask*)cursor_sub)->sortbyva();
				dirtyview();
			}
			return "sortva";
		case 4:
			if (execute) {
				((ht_pe_import_mask*)cursor_sub)->sortbyname();
				dirtyview();
			}
			return "sortname";
		case 5:
			if (execute) {
				((ht_pe_import_mask*)cursor_sub)->unsort();
				dirtyview();
			}
			return "unsort";
	}
	return ht_uformat_viewer::func(i, execute);
}

int ht_pe_dimport_viewer::ref_sel(ID id_low, ID id_high)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	if (pe_shared->v_image->goto_address(id_low, this)) {
		app->focus(pe_shared->v_image);
	} else errorbox("can't follow: %s %08x is not valid !", "delay-import address", id_low);
	return 1;
}

