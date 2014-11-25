/*
 *	HT Editor
 *	htpereloc.cc
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeimp.h"
#include "stream.h"
#include "strtools.h"
#include "httag.h"
#include "log.h"
#include "pe_analy.h"
#include "snprintf.h"
#include "tools.h"
#include "htpereloc.h"

#include <stdlib.h>
#include <string.h>

#include <deque>
#include <algorithm>


static int_hash pe_fixup_type_desc[] =
{
	{IMAGE_REL_BASED_ABSOLUTE, "IGNORE"},
	{IMAGE_REL_BASED_HIGH, "HIGH"},
	{IMAGE_REL_BASED_LOW, "LOW"},
	{IMAGE_REL_BASED_HIGHLOW, "HIGHLOW"},
	{IMAGE_REL_BASED_HIGHADJ, "HIGHADJ"},
	{IMAGE_REL_BASED_MIPS_JMPADDR, "MIPS_JMPADDR"},
	{IMAGE_REL_BASED_SECTION, "SECTION"},
	{IMAGE_REL_BASED_REL32, "REL32"},
	{IMAGE_REL_BASED_MIPS_JMPADDR16, "MIPS_JMPADDR16"},
	{IMAGE_REL_BASED_DIR64, "DIR64"},
	{IMAGE_REL_BASED_HIGH3ADJ, "HIGH3ADJ"},
	{0, 0}
};

static int ht_getfixuptypesize(int type) {

	int size=0;

	switch(type) {
		case IMAGE_REL_BASED_HIGHLOW:
			size=4;
			break;

		case IMAGE_REL_BASED_ABSOLUTE:
			size=0;
			break;

		case IMAGE_REL_BASED_DIR64:
			size=8;
			break;

	}

	return size;

}

struct fixupque_sortCB {
	        bool operator()(const fixup_listentry &a, const fixup_listentry &b)
	        {
	            return a.addr < b.addr;
	        }
} ;


static ht_view *htpereloc_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32 && pe_shared->opt_magic!=COFF_OPTMAGIC_PE64) return NULL;
	bool pe32 = (pe_shared->opt_magic==COFF_OPTMAGIC_PE32);

	uint32 sec_rva, sec_size;
	if (pe32) {
		sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_BASERELOC].address;
		sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_BASERELOC].size;
	} else {
		sec_rva = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_BASERELOC].address;
		sec_size = pe_shared->pe64.header_nt.directory[PE_DIRECTORY_ENTRY_BASERELOC].size;
	}
	if (!sec_rva || !sec_size) return NULL;

// ---

	ht_group *g;
	ht_statictext *head;
	Bounds c;
	String fn, s, dllname;
	char iline[256];
	FileOfs rel_fofs;
	PE_FIXUP_BLOCK fixup_block;
	uint16 fixup_entry, rfixup_entry;
	uint32 fixup_type, fixup_offset;			// fixup_addr, fixup_chkaddr;
	RVA fixup_addr, fixup_chkaddr;
	FileOfs fixup_fileofs;
	uint32 sec_pntr;
	uint32 blk_pntr;
	int fixup_id;
	const char *fixup_typedesc;
	std::deque<fixup_listentry>::iterator fixupque_lit;
	bool isSorted;

	fixup_listentry fixupentry;

// --

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_PE_RELOC"-g");

	c.y++;
	c.h--;
	ht_pe_reloc_viewer *v=new ht_pe_reloc_viewer();
	v->init(&c, DESC_PE_RELOC, group);
	c.y--;

	if (!pe_rva_to_ofs(&pe_shared->sections, sec_rva, &rel_fofs)) goto pe_read_error;
	LOG("%y: PE: reading relocations directory at offset 0x%08qx, rva 0x%08x, size 0x%08x...", &file->getFilename(fn), rel_fofs, sec_rva, sec_size);

// --

	ht_snprintf(iline, sizeof iline, "* PE relocations directory at offset %08qx", rel_fofs);
	head=new ht_statictext();
	c.h=1;
	head->init(&c, iline, align_left);

	g->insert(head);
	g->insert(v);

	fixup_id = 0;
	isSorted = true;
	fixup_chkaddr = 0;
	sec_pntr = 0;
	while (sec_pntr < sec_size) {

		file->seek(rel_fofs + sec_pntr);
		file->readx(&fixup_block, sizeof fixup_block);
		createHostStruct(&fixup_block, PE_FIXUP_BLOCK_struct, little_endian);

//		LOG("fixup block: rva 0x%08x, size 0x%08x", fixup_block.Page_RVA, fixup_block.Block_Size);
		if (!fixup_block.Block_Size) break;

		blk_pntr = sizeof fixup_block;
		while (blk_pntr < fixup_block.Block_Size) {

			file->seek(rel_fofs + sec_pntr + blk_pntr);
			file->readx(&rfixup_entry, 2);

			fixup_entry = createHostInt(&rfixup_entry, STRUCT_ENDIAN_16, little_endian);
			fixup_type = fixup_entry >> 12;
			fixup_offset = fixup_entry & 0xfff;
			fixup_addr = fixup_block.Page_RVA + fixup_offset;

			if (!pe_rva_to_ofs(&pe_shared->sections, fixup_addr, &fixup_fileofs)) {
				fixup_fileofs = 0;
			}

			fixup_typedesc = matchhash(fixup_type, pe_fixup_type_desc);
			if (!fixup_typedesc) fixup_typedesc = "INVALID";

			char txt_addr[32];
			ht_snprintf(txt_addr, sizeof txt_addr, "%08x", fixup_addr);
			char txt_ofs[32];
			ht_snprintf(txt_ofs, sizeof txt_ofs, "%08x", fixup_fileofs);
			v->insert_str(fixup_id, txt_addr, fixup_typedesc, txt_ofs);

			fixupentry.addr = fixup_addr;
			fixupentry.size = ht_getfixuptypesize(fixup_type);
			fixupentry.idCol = fixup_id;
			pe_shared->fixupque.push_back(fixupentry);

			if (fixup_addr < fixup_chkaddr) {																// check if sorting is needed
				isSorted = false;
			}
			fixup_chkaddr = fixup_addr;

			fixup_id++;

//			LOG("fixup: offset 0x%08x, type %s (0x%02x)", fixup_offset, fixup_typedesc, fixup_type);

			blk_pntr += 2;
		}

		sec_pntr += fixup_block.Block_Size;
	}

	if (!isSorted) {
		LOG("%y: PE: sorting relocation table...", &file->getFilename(fn));
	    std::sort(pe_shared->fixupque.begin(), pe_shared->fixupque.end(), fixupque_sortCB() );

	}

// Test aid: Show stored list
//
//	for (fixupque_lit = pe_shared->fixupque.begin();fixupque_lit != pe_shared->fixupque.end();fixupque_lit++) {
//
//				char addr[32],size[32];
//				ht_snprintf(addr, sizeof addr, "%08x", fixupque_lit->addr);
//				ht_snprintf(size, sizeof addr, "%02x", fixupque_lit->size);
//				v->insert_str(fixup_id, addr, size);
//				fixup_id++;
//
//	}


	v->update();

	g->setpalette(palkey_generic_window_default);

	return g;

pe_read_error:

	errorbox("%y: PE import section seems to be corrupted.", &file->getFilename(fn));
	g->done();
	delete g;
	return NULL;
}

format_viewer_if htpereloc_if = {
	htpereloc_init,
	0
};



/*
 *	CLASS ht_pe_reloc_viewer
 */

void	ht_pe_reloc_viewer::init(Bounds *b, const char *Desc, ht_format_group *fg)
{
	ht_text_listbox::init(b, 3, 3, LISTBOX_NORMAL);				//LISTBOX_QUICKFIND			// init(Bounds *b, int aCols, int aKeycol, uint aListboxcaps)
	options |= VO_BROWSABLE;
	desc = strdup(Desc);
	format_group = fg;

}

const char *ht_pe_reloc_viewer::func(uint i, bool execute)
{
	switch (i) {
/*		case 2:										// TODO: add fkey which opens a simple edit window to edit the relocation at the cursor.
			if (execute) {
			}
			return "fkey2";*/
	}
	return NULL;
}


void ht_pe_reloc_viewer::handlemsg(htmsg *msg)
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

bool ht_pe_reloc_viewer::select_entry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();
	std::deque<fixup_listentry>::iterator fixupque_lit;
	bool entryfound=false;

	for (fixupque_lit = pe_shared->fixupque.begin();fixupque_lit != pe_shared->fixupque.end();fixupque_lit++) {

		if (fixupque_lit->idCol == i->id) {
			entryfound = true;
			break;
		}

	}

	if (entryfound) {
		if (pe_shared->v_image) {
			ht_aviewer *av = (ht_aviewer*)pe_shared->v_image;
			PEAnalyser *a = (PEAnalyser*)av->analy;
			Address *addr;
			if (pe_shared->opt_magic == COFF_OPTMAGIC_PE32) {
				addr = a->createAddress32(fixupque_lit->addr + pe_shared->pe32.header_nt.image_base);
			} else {
				addr = a->createAddress64(fixupque_lit->addr + pe_shared->pe64.header_nt.image_base);
			}

			if (av->gotoAddress(addr, NULL)) {
				app->focus(av);
				vstate_save();
			} else {
				global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
				errorbox("can't follow: %s %y is not valid!", "import address", addr);
			}
			delete addr;
		} else errorbox("can't follow: no image viewer");

	}



	return true;
}







