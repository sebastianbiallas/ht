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

#include "formats.h"
#include "htanaly.h"
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
#include "log.h"
#include "pe_analy.h"
#include "snprintf.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

ht_view *htpeimports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32 && pe_shared->opt_magic!=COFF_OPTMAGIC_PE64) return NULL;

	bool pe32 = (pe_shared->opt_magic==COFF_OPTMAGIC_PE32);

	dword sec_rva, sec_size;
	if (pe32) {
		sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].address;
		sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].size;
	} else {
		sec_rva = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].address;
		sec_size = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].size;
	}
	if (!sec_rva || !sec_size) return NULL;

	int h0=new_timer();
	start_timer(h0);

	ht_group *g;
	bounds c;

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_PE_IMPORTS"-g");
	ht_statictext *head;

	int dll_count=0;
	int function_count=0;

	c.y++;
	c.h--;
	ht_pe_import_viewer *v=new ht_pe_import_viewer();
	v->init(&c, DESC_PE_IMPORTS, group);

	c.y--;
	c.h=1;

	PE_IMPORT_DESCRIPTOR import;
	FILEOFS dofs;
	UINT dll_index;
	char iline[256];
	
	/* get import directory offset */
	/* 1. get import directory rva */
	FILEOFS iofs;
	RVA irva;
	UINT isize;
	if (pe32) {
		irva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].address;
		isize = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].size;
	} else {
		irva = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].address;
		isize = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_IMPORT].size;
	}
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, irva, &iofs)) goto pe_read_error;
	LOG("%s: PE: reading import directory at offset %08x, rva %08x, size %08x...", file->get_filename(), iofs, irva, isize);

	/* make a memfile out of the whole file */

	/* doesn't work because import information is NOT contained into
	   the import directory !!! only the import dir header. (I !love M$) */

	/*** read import directory ***/
	dofs = iofs;
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
		 *   The first thunk table will be overwritten by the system/loader with
		 *	the function entry-points. So the program will treat this table
		 *	as if it contained just imported addresses.
		 */
		RVA fthunk_rva = import.first_thunk;
		FILEOFS fthunk_ofs;
		if (!pe_rva_to_ofs(&pe_shared->sections, fthunk_rva, &fthunk_ofs)) goto pe_read_error;
		/*
		 *   ...and Original First Thunk (OFT)
		 * 	I saw executables that have the OFT ptr set to 0 and seem to
		 *	use the FT ptr instead, but also some that have both !=0 and use
		 *	the original one (bound executables).
		 */
		RVA thunk_rva;
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
		PE_THUNK_DATA_64 thunk64;
		UINT thunk_count = 0;
		file->seek(thunk_ofs);
		while (1) {
			if (pe32) {
				file->read(&thunk, sizeof thunk);
				create_host_struct(&thunk, PE_THUNK_DATA_struct, little_endian);
				if (!thunk.ordinal) break;
			} else {
				file->read(&thunk64, sizeof thunk64);
				create_host_struct(&thunk64, PE_THUNK_DATA_64_struct, little_endian);
				if (!QWORD_GET_LO(thunk64.ordinal)) break;
			}
			thunk_count++;
		}

		PE_THUNK_DATA *thunk_table;
		PE_THUNK_DATA_64 *thunk_table64;
		thunk_table=(PE_THUNK_DATA*)malloc(sizeof *thunk_table * thunk_count);
		thunk_table64=(PE_THUNK_DATA_64*)malloc(sizeof *thunk_table64 * thunk_count);
		file->seek(thunk_ofs);
		if (pe32) {
			file->read(thunk_table, sizeof *thunk_table * thunk_count);
			// FIXME: ?
			for (UINT i=0; i<thunk_count; i++) {
				create_host_struct(thunk_table+i, PE_THUNK_DATA_struct, little_endian);
			}
		} else {
			file->read(thunk_table64, sizeof *thunk_table64 * thunk_count);
			// FIXME: ?
			for (UINT i=0; i<thunk_count; i++) {
				create_host_struct(thunk_table64+i, PE_THUNK_DATA_64_struct, little_endian);
			}
		}
		for (dword i=0; i<thunk_count; i++) {
			function_count++;
			ht_pe_import_function *func;
			/* follow (original) first thunk */
			if (pe32) {
				thunk = *(thunk_table+i);

				if (thunk.ordinal & 0x80000000) {
					/* by ordinal */
					func = new ht_pe_import_function(dll_index, fthunk_rva, thunk.ordinal&0xffff);
				} else {
					/* by name */
					FILEOFS function_desc_ofs;
					word hint = 0;
					if (!pe_rva_to_ofs(&pe_shared->sections, thunk.function_desc_address, &function_desc_ofs)) goto pe_read_error;
					file->seek(function_desc_ofs);
					file->read(&hint, 2);
					hint = create_host_int(&hint, 2, little_endian);
					char *name = fgetstrz(file);
					func = new ht_pe_import_function(dll_index, fthunk_rva, name, hint);
					free(name);
				}
			} else {
				thunk64 = *(thunk_table64+i);

				// FIXME: is this correct ?
				if (QWORD_GET_LO(thunk64.ordinal) & 0x80000000) {
					/* by ordinal */
					func = new ht_pe_import_function(dll_index, fthunk_rva, QWORD_GET_LO(thunk64.ordinal)&0xffff);
				} else {
					/* by name */
					FILEOFS function_desc_ofs;
					word hint = 0;
					if (!pe_rva_to_ofs(&pe_shared->sections, QWORD_GET_LO(thunk64.function_desc_address), &function_desc_ofs)) goto pe_read_error;
					file->seek(function_desc_ofs);
					file->read(&hint, 2);
					hint = create_host_int(&hint, 2, little_endian);
					char *name = fgetstrz(file);
					func = new ht_pe_import_function(dll_index, fthunk_rva, name, hint);
					free(name);
				}
			}
			pe_shared->imports.funcs->insert(func);

			if (pe32) {
				thunk_ofs+=4;
				thunk_rva+=4;
				fthunk_ofs+=4;
				fthunk_rva+=4;
			} else {
				thunk_ofs+=8;
				thunk_rva+=8;
				fthunk_ofs+=8;
				fthunk_rva+=8;
			}
		}
		
		dll_index++;

		free(thunk_table);

		free(dllname);
	}

	stop_timer(h0);
//	LOG("%s: PE: %d ticks (%d msec) to read imports", file->get_name(), get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

	ht_snprintf(iline, sizeof iline, "* PE import directory at offset %08x (%d functions from %d libraries)", iofs, function_count, dll_count);
	head=new ht_statictext();
	head->init(&c, iline, align_left);

	g->insert(head);
	g->insert(v);
	//
	for (UINT i=0; i<pe_shared->imports.funcs->count(); i++) {
		ht_pe_import_function *func = (ht_pe_import_function*)pe_shared->imports.funcs->get(i);
		assert(func);
		ht_pe_import_library *lib = (ht_pe_import_library*)pe_shared->imports.libs->get(func->libidx);
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

	pe_shared->v_imports=v;
	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%s: PE import section seems to be corrupted.", file->get_filename());
	g->done();
	delete g;
	v->done();
	delete v;
	return NULL;
}

format_viewer_if htpeimports_if = {
	htpeimports_init,
	NULL
};

/*
 *	class ht_pe_import_library
 */

ht_pe_import_library::ht_pe_import_library(char *n)
{
	name = ht_strdup(n);
}

ht_pe_import_library::~ht_pe_import_library()
{
	if (name) free(name);
}

/*
 *	class ht_pe_import_function
 */

ht_pe_import_function::ht_pe_import_function(UINT li, RVA a, UINT o)
{
	libidx = li;
	ordinal = o;
	address = a;
	byname = false;
}

ht_pe_import_function::ht_pe_import_function(UINT li, RVA a, char *n, UINT h)
{
	libidx= li;
	name.name = ht_strdup(n);
	name.hint = h;
	address = a;
	byname = true;
}

ht_pe_import_function::~ht_pe_import_function()
{
	if ((byname) && (name.name)) free(name.name);
}

/*
 *	CLASS ht_pe_import_viewer
 */

void	ht_pe_import_viewer::init(bounds *b, char *Desc, ht_format_group *fg)
{
	ht_text_listbox::init(b, 3, 2, LISTBOX_QUICKFIND);
	options |= VO_BROWSABLE;
	desc = strdup(Desc);
	format_group = fg;
	grouplib = false;
	sortby = 1;
	dosort();
}

void	ht_pe_import_viewer::done()
{
	ht_text_listbox::done();
}

void ht_pe_import_viewer::dosort()
{
	ht_text_listbox_sort_order sortord[2];
	UINT l, s;
	if (grouplib) {
		l = 0;
		s = 1;
	} else {
		l = 1;
		s = 0;
	}
	sortord[l].col = 0;
	sortord[l].compare_func = strcmp;
	sortord[s].col = sortby;
	sortord[s].compare_func = strcmp;
	sort(2, sortord);
}

char *ht_pe_import_viewer::func(UINT i, bool execute)
{
	switch (i) {
		case 2:
			if (execute) {
				grouplib = !grouplib;
				dosort();
			}
			return grouplib ? (char*)"nbylib" : (char*)"bylib";
		case 3:
			if (execute) {
				if (sortby != 1) {
					sortby = 1;
					dosort();
				}
			}
			return "byaddr";
		case 4:
			if (execute) {
				if (sortby != 2) {
					sortby = 2;
					dosort();
				}
			}
			return "byname";
	}
	return NULL;
}

void ht_pe_import_viewer::handlemsg(htmsg *msg)
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
/*		case msg_get_scrollinfo:
			switch (msg->data1.integer) {
				case gsi_pindicator: {
					strcpy((char*)msg->data2.ptr, " Enter to view, Backspace to return here");
					break;
				}
			}
			clearmsg(msg);
			return;*/
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

bool ht_pe_import_viewer::select_entry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;

	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	ht_pe_import_function *e = (ht_pe_import_function*)pe_shared->imports.funcs->get(i->id);
	if (!e) return true;
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
			errorbox("can't follow: %s %y is not valid !", "import address", addr);
		}
		delete addr;
	} else errorbox("can't follow: no image viewer");
	return true;
}


