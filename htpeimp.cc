/* 
 *	HT Editor
 *	htpeimp.cc
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
#include "htctrl.h"
#include "htdata.h"
#include "htendian.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeimp.h"
#include "stream.h"
#include "htstring.h"
#include "httag.h"
#include "tools.h"
#include "formats.h"

#include <stdlib.h>
#include <string.h>

/* FIXME: */
static ht_pe_import *g_import;

ht_view *htpeimports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return 0;

	dword sec_rva, sec_size;
	sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].address;
	sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].size;
	if (!sec_rva || !sec_size) return 0;

	int h0=new_timer();
	start_timer(h0);

	ht_group *g;
	bounds c;

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, "pe/imports-g");
	ht_statictext *head;

	int dll_count=0;
	int function_count=0;
//	ht_cached_stream *cached_file=0;

	c.y++;
	c.h--;
	ht_uformat_viewer *v=new ht_pe_import_viewer();
	v->init(&c, DESC_PE_IMPORTS, VC_SEARCH, file, group);

	c.y--;
	c.h=1;

	PE_IMPORT_DESCRIPTOR import;
	int dofs;
	int f;
	int dll_index;
	char iline[256];
	
	ht_pe_import_mask *m = NULL;

/* get import directory offset */
	/* 1. get import directory rva */
	FILEOFS iofs;
	dword irva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].address;
	dword isize = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].size;
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, irva, &iofs)) goto pe_read_error;
	LOG("%s: PE: reading import directory at offset %08x, rva %08x, size %08x...", file->get_filename(), iofs, irva, isize);

/* make a memfile out of the whole file */

/* doesn't work because import information is NOT contained into
   the import directory !!! only the import dir header. (I !love M$) */

/*     htfile *ifile;
	char *isectionbuf;
	ifile=new htmemfile(iofs, isize);
	isectionbuf=(char*)malloc(isize);
	file->seek(iofs);
	file->read(isectionbuf, isize);
	ifile->write(isectionbuf, isize);
	file=ifile;
	delete isectionbuf;*/

//	cached_file=new ht_cached_stream(file, 128*1024, CACHED_STREAM_CACHE_READS, 2048);
//     file=cached_file;		/* we dont need to care about "file" */

/*** read import directory ***/
	dofs = iofs;
	f = 0;
	dll_index = 0;

	while (1) {
		file->seek(dofs);
		file->read(&import, sizeof import);
		create_host_struct(&import, PE_IMPORT_DESCRIPTOR_struct, little_endian);
		if ((!import.characteristics) && (!import.name)) break;
		dofs = file->tell();
/* get name of dll */
		FILEOFS iname_ofs;
		if (!pe_rva_to_ofs(&pe_shared->sections, import.name, &iname_ofs)) goto pe_read_error;
		file->seek(iname_ofs);
		char *dllname = fgetstrz(file);	/* dont forget to free it at the end of the scope !!! */
		dll_count++;

/*** imported functions by name or by ordinal ***/

/*
 *	First thunk (FT)
 *   The first thunk table is what will be overwritten in run-time with
 *	the function entry-points. So the program will refer to this table
 *	as if it contained them.
 */
		dword fthunk_rva = import.first_thunk;
		FILEOFS fthunk_ofs;
		if (!pe_rva_to_ofs(&pe_shared->sections, fthunk_rva, &fthunk_ofs)) goto pe_read_error;
/*
 *   FT vs original FT
 * 	I saw executables that have the original FT ptr set to 0 and seem to
 *	use the FT ptr instead, but also some that have both !=0 and use
 *	the original one (bound executables).
 * 	I don't know if this is right... please tell me if you find out :-)
 */
		dword thunk_rva;
		FILEOFS thunk_ofs;
		if (import.original_first_thunk) {
			thunk_rva = import.original_first_thunk;
			if (!pe_rva_to_ofs(&pe_shared->sections, thunk_rva, &thunk_ofs)) goto pe_read_error;
		} else {
			thunk_rva = fthunk_rva;
			thunk_ofs = fthunk_ofs;
		}
		ht_pe_import_library *lib=new ht_pe_import_library(dllname);
		pe_shared->imports.libs->insert(lib);

		PE_THUNK_DATA thunk;
		dword thunk_count = 0;
		file->seek(thunk_ofs);
		while (1) {
			file->read(&thunk, sizeof thunk);
			create_host_struct(&thunk, PE_THUNK_DATA_struct, little_endian);
			if (!thunk.ordinal)	break;
			thunk_count++;
		}

		PE_THUNK_DATA *thunk_table;
		thunk_table=(PE_THUNK_DATA*)malloc(sizeof *thunk_table * thunk_count);
		file->seek(thunk_ofs);
		file->read(thunk_table, sizeof *thunk_table * thunk_count);
		// FIXME: ?
		for (UINT i=0; i<thunk_count; i++) {
			create_host_struct(thunk_table+i, PE_THUNK_DATA_struct, little_endian);
		}
		for (dword i=0; i<thunk_count; i++) {
			function_count++;
			ht_pe_import_function *func;
/* follow (original) first thunk */
			thunk = *(thunk_table+i);

			if (thunk.ordinal & 0x80000000) {
/* by ordinal */
				func = new ht_pe_import_function(dll_index, fthunk_rva+pe_shared->pe32.header_nt.image_base, thunk.ordinal&0xffff);
			} else {
/* by name */
				FILEOFS function_desc_ofs;
				word hint = 0;
				if (!pe_rva_to_ofs(&pe_shared->sections, thunk.function_desc_address, &function_desc_ofs)) goto pe_read_error;
				file->seek(function_desc_ofs);
				file->read(&hint, 2);
				hint = create_host_int(&hint, 2, little_endian);
				char *name = fgetstrz(file);
				func = new ht_pe_import_function(dll_index, fthunk_rva+pe_shared->pe32.header_nt.image_base, name, hint);
				free(name);
			}
			pe_shared->imports.funcs->insert(func);

			thunk_ofs+=4;
			thunk_rva+=4;
			fthunk_ofs+=4;
			fthunk_rva+=4;
			f++;
		}
		
		dll_index++;

		free(thunk_table);

		free(dllname);
	}

	stop_timer(h0);
//	LOG("%s: PE: %d ticks (%d msec) to read imports", file->get_name(), get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

//     delete cached_file;

	sprintf(iline, "Library        FT_VA    Name/Ordinal    %d imports from %d libraries", function_count, dll_count);
	head=new ht_statictext();
	head->init(&c, iline, align_left);

	sprintf(iline, "* PE import directory at offset %08x", iofs);

	m=new ht_pe_import_mask();
	m->init(file, &pe_shared->imports, iline);

	v->insertsub(m);

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	pe_shared->v_imports=v;
	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%s: PE import section seems to be corrupted.", file->get_filename());
//	if (cached_file) delete cached_file;
	g->done();
	delete g;
	if (m) {
		m->done();
		delete m;
	}		
	v->done();
	delete v;
	return 0;
}

format_viewer_if htpeimports_if = {
	htpeimports_init,
	0
};

/*
 *	class ht_pe_import_library
 */

ht_pe_import_library::ht_pe_import_library()
{
}

ht_pe_import_library::ht_pe_import_library(char *n)
{
	name = ht_strdup(n);
}

ht_pe_import_library::~ht_pe_import_library()
{
	free(name);
}

/*
 *	class ht_pe_import_function
 */

ht_pe_import_function::ht_pe_import_function()
{
	libidx = 0;
	name.name = 0;
	name.hint = 0;
	ordinal = 0;
	address = 0;
}

ht_pe_import_function::ht_pe_import_function(int li, dword a, int o)
{
	libidx = li;
	ordinal = o;
	address = a;
	byname = 0;
}

ht_pe_import_function::ht_pe_import_function(int li, dword a, char *n, int h)
{
	libidx= li;
	name.name = ht_strdup(n);
	name.hint = h;
	address = a;
	byname = 1;
}

ht_pe_import_function::~ht_pe_import_function()
{
	if ((byname) && (name.name)) free(name.name);
}

/*
 *	CLASS ht_pe_import_viewer
 */

void ht_pe_import_viewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *group)
{
	ht_uformat_viewer::init(b, desc, caps, file, group);
	VIEW_DEBUG_NAME("ht_pe_import_viewer");
}

void ht_pe_import_viewer::done()
{
	ht_uformat_viewer::done();
}

char *ht_pe_import_viewer::func(UINT i, bool execute)
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

int ht_pe_import_viewer::ref_sel(ID id_low, ID id_high)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	if (pe_shared->v_image->goto_address(id_low, this)) {
		app->focus(pe_shared->v_image);
	} else errorbox("can't follow: %s %08x is not valid !", "import address", id_low);
	return 1;
}

/*
 *	CLASS ht_pe_import_mask
 */

void ht_pe_import_mask::init(ht_streamfile *file, ht_pe_import *i, char *f)
{
	ht_sub::init(file);
	firstline = ht_strdup(f);
	import = i;
	sort_path = 0;
	sort_va = 0;
	sort_name = 0;
}

void ht_pe_import_mask::done()
{
	free(firstline);
	if (sort_va) free(sort_va);
	if (sort_name) free(sort_name);
	ht_sub::done();
}

bool ht_pe_import_mask::convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2)
{
	*id2 = 0;
	*id1 = addr;
	return true;
}

bool ht_pe_import_mask::convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr)
{
	*addr = id1;
	return true;
}

void ht_pe_import_mask::first_line_id(ID *id1, ID *id2)
{
	*id1=0;
	*id2=0;
}

bool ht_pe_import_mask::getline(char *line, ID id1, ID id2)
{
	if (id1>=1) {
		char *l=line;
		UINT i=id1-1;
		if (sort_path) i=sort_path[i];
		ht_pe_import_function *f=(ht_pe_import_function*)import->funcs->get(i);

		ht_pe_import_library *lib=(ht_pe_import_library*)import->libs->get(f->libidx);
		if (f->byname) {
/* FIXME: what about the hint ? */
			l+=sprintf(l, "%-14s %08x ", lib->name, f->address);
			l=tag_make_ref(l, f->address, 0, f->name.name);
		} else {
			char ord[5];
			sprintf(ord, "%04x", f->ordinal);
			l+=sprintf(l, "%-14s %08x ", lib->name, f->address);
			l=tag_make_ref(l, f->address, 0, ord);
		}
		*l=0;
	} else {
		strcpy(line, firstline);
	}
	return 1;
}

void ht_pe_import_mask::last_line_id(ID *id1, ID *id2)
{
	*id1=import->funcs->count();
	*id2=0;
}

int ht_pe_import_mask::next_line_id(ID *id1, ID *id2, int n)
{
	UINT c=import->funcs->count()+1;
	if (*id1+n>=c) {
		int r=c-*id1-1;
		*id1=c-1;
		return r;
	}
	*id1+=n;
	return n;
}

int ht_pe_import_mask::prev_line_id(ID *id1, ID *id2, int n)
{
	if ((UINT)n>*id1) {
		int r=*id1;
		*id1=0;
		return r;
	}
	*id1-=n;
	return n;
}

int compare_import_va(UINT *i1, UINT *i2)
{
	ht_pe_import_function *f1=(ht_pe_import_function *)g_import->funcs->get(*i1);
	ht_pe_import_function *f2=(ht_pe_import_function *)g_import->funcs->get(*i2);
	if (f1->address>f2->address) return 1; else
	if (f1->address<f2->address) return -1; else
		return 0;
}

void ht_pe_import_mask::sortbyva()
{
	if (!sort_va) {
		int c=import->funcs->count();
		sort_va=(UINT*)malloc(sizeof (UINT) * c);
		for (int i=0; i<c; i++) {
			sort_va[i]=i;
		}
		g_import=import;
		qsort(sort_va, c, sizeof (UINT), (int (*)(const void *, const void*))compare_import_va);
	}
	sort_path=sort_va;
}

int compare_import_name(UINT *i1, UINT *i2)
{
	ht_pe_import_function *f1=(ht_pe_import_function *)g_import->funcs->get(*i1);
	ht_pe_import_function *f2=(ht_pe_import_function *)g_import->funcs->get(*i2);
	if (f1->byname) {
		if (f2->byname) {
			return strcmp(f1->name.name, f2->name.name);
		} else {
			return 1;
		}
	} else {
		if (f2->byname) {
			return -1;
		} else {
			if (f1->ordinal>f2->ordinal) return 1; else
			if (f1->ordinal<f2->ordinal) return -1; else
				return 0;
		}
	}
}

void ht_pe_import_mask::sortbyname()
{
	if (!sort_name) {
		int c=import->funcs->count();
		sort_name=(UINT*)malloc(sizeof (UINT) * c);
		for (int i=0; i<c; i++) {
			sort_name[i]=i;
		}
		g_import=import;
		qsort(sort_name, c, sizeof (UINT), (int (*)(const void *, const void*))compare_import_name);
	}
	sort_path=sort_name;
}

void ht_pe_import_mask::unsort()
{
	sort_path=0;
}

