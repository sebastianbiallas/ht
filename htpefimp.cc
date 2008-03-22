/*
 *	HT Editor
 *	htpefimp.cc
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
#include "endianess.h"
#include "htiobox.h"
#include "htpal.h"
#include "htpef.h"
#include "htpefimp.h"
#include "stream.h"
#include "strtools.h"
#include "httag.h"
#include "log.h"
#include "pef_analy.h"
#include "snprintf.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

ht_view *htpefimports_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pef_shared_data *pef_shared=(ht_pef_shared_data *)group->get_shared_data();

	if (!pef_shared->loader_info_header_ofs 
	|| !pef_shared->loader_info_header.importedLibraryCount 
	|| !pef_shared->loader_info_header.totalImportedSymbolCount) return NULL;
	
	FileOfs nametable = pef_shared->loader_info_header_ofs + pef_shared->loader_info_header.loaderStringsOffset;
	FileOfs functions_offset = pef_shared->loader_info_header_ofs 
			+ sizeof pef_shared->loader_info_header
			+ pef_shared->loader_info_header.importedLibraryCount
				* sizeof(PEF_ImportedLibrary);

	ht_group *g;
	Bounds c;

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_PEF_IMPORTS"-g");
	ht_statictext *head;

	int lib_count = pef_shared->loader_info_header.importedLibraryCount;
	int function_count = pef_shared->loader_info_header.totalImportedSymbolCount;

	c.y++;
	c.h--;
	ht_pef_import_viewer *v=new ht_pef_import_viewer();
	v->init(&c, DESC_PEF_IMPORTS, group);

	c.y--;
	c.h=1;

	int symbol_num = 0;
	for (uint i=0; i < pef_shared->loader_info_header.importedLibraryCount; i++) {
		file->seek(pef_shared->loader_info_header_ofs + sizeof pef_shared->loader_info_header
			+ i*sizeof(PEF_ImportedLibrary));

		PEF_ImportedLibrary lib;
		file->read(&lib, sizeof lib);
		createHostStruct(&lib, PEF_ImportedLibrary_struct, pef_shared->byte_order);

		file->seek(nametable + lib.nameOffset);
		char *libname = file->fgetstrz();

		ht_pef_import_library *library=new ht_pef_import_library(libname);
		pef_shared->imports.libs->insert(library);

		for (uint j=0; j < lib.importedSymbolCount; j++) {
			file->seek(functions_offset + 4 * (lib.firstImportedSymbol+j));
			uint32 entry;
			file->read(&entry, 4);
			entry = createHostInt(&entry, 4, pef_shared->byte_order);
			
			uint32 symbol_ofs = entry & 0x00ffffff;
			uint32 symbol_class = entry >> 24;

			file->seek(nametable+symbol_ofs);
			char *symbol_name = file->fgetstrz();

			ht_pef_import_function *func = new ht_pef_import_function(i, symbol_num, symbol_name, symbol_class);
			pef_shared->imports.funcs->insert(func);

			free(symbol_name);
			symbol_num++;
		}
		
		free(libname);
	}

	char iline[1024];
	ht_snprintf(iline, sizeof iline, "* PEF import library description at offset %08qx (%d functions from %d libraries)", 
		pef_shared->loader_info_header_ofs + sizeof pef_shared->loader_info_header, 
		function_count, lib_count);
		
	head=new ht_statictext();
	head->init(&c, iline, align_left);

	g->insert(head);
	g->insert(v);
	//
	for (uint i=0; i < pef_shared->imports.funcs->count(); i++) {
		ht_pef_import_function *func = (ht_pef_import_function*)(*pef_shared->imports.funcs)[i];
		assert(func);
		ht_pef_import_library *lib = (ht_pef_import_library*)(*pef_shared->imports.libs)[func->libidx];
		assert(lib);
		char addr[32], name[256];
		ht_snprintf(addr, sizeof addr, "%d", func->num);
		ht_snprintf(name, sizeof name, "%s", func->name);
		v->insert_str(i, lib->name, addr, name);
	}
	//
	v->update();

	g->setpalette(palkey_generic_window_default);

	pef_shared->v_imports = v;
	return g;
pef_read_error:
	String fn;
	errorbox("%y: PEF import library description seems to be corrupted.", &file->getFilename(fn));
	g->done();
	delete g;
	v->done();
	delete v;
	return NULL;
}

format_viewer_if htpefimports_if = {
	htpefimports_init,
	NULL
};

/*
 *	class ht_pef_import_library
 */

ht_pef_import_library::ht_pef_import_library(char *n)
{
	name = ht_strdup(n);
}

ht_pef_import_library::~ht_pef_import_library()
{
	free(name);
}

/*
 *	class ht_pe_import_function
 */

ht_pef_import_function::ht_pef_import_function(uint aLibidx, int aNum, const char *aName, uint aSym_class)
{
	libidx = aLibidx;
	name = ht_strdup(aName);
	num = aNum;
	sym_class = aSym_class;
}

ht_pef_import_function::~ht_pef_import_function()
{
	free(name);
}

/*
 *	CLASS ht_pef_import_viewer
 */

void ht_pef_import_viewer::init(Bounds *b, const char *Desc, ht_format_group *fg)
{
	ht_text_listbox::init(b, 3, 2, LISTBOX_QUICKFIND);
	options |= VO_BROWSABLE;
	desc = strdup(Desc);
	format_group = fg;
	grouplib = false;
	sortby = 1;
	dosort();
}

void	ht_pef_import_viewer::done()
{
	ht_text_listbox::done();
}

void ht_pef_import_viewer::dosort()
{
	ht_text_listbox_sort_order sortord[2];
	uint l, s;
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

const char *ht_pef_import_viewer::func(uint i, bool execute)
{
	switch (i) {
		case 2:
			if (execute) {
				grouplib = !grouplib;
				dosort();
			}
			return grouplib ? (char*)"nbylib" : (char*)"bylib";
		case 5:
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

void ht_pef_import_viewer::handlemsg(htmsg *msg)
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

bool ht_pef_import_viewer::select_entry(void *entry)
{
/*	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;

	ht_pef_shared_data *pef_shared=(ht_pef_shared_data *)format_group->get_shared_data();

	ht_pef_import_function *e = (ht_pef_import_function*)pef_shared->imports.funcs->get(i->id);
	if (!e) return true;
	if (pef_shared->v_image) {
		ht_aviewer *av = (ht_aviewer*)pef_shared->v_image;
		PEFAnalyser *a = (PEFAnalyser*)av->analy;
		Address *addr;
		if (pef_shared->opt_magic == COFF_OPTMAGIC_PE32) {
			addr = a->createAddress32(e->address+pe_shared->pe32.header_nt.image_base);
		} else {
			addr = a->createAddress64(to_qword(e->address)+pe_shared->pe64.header_nt.image_base);
		}
		if (av->gotoAddress(addr, NULL)) {
			app->focus(av);
			vstate_save();
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
			errorbox("can't follow: %s %y is not valid!", "import address", addr);
		}
		delete addr;
	} else errorbox("can't follow: no image viewer");*/
	return true;
}


