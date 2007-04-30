/* 
 *	HT Editor
 *	htelfphs.cc
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

#include "elfstruc.h"
#include "atom.h"
#include "htelf.h"
#include "htelfphs.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable elfprogramheader32[]=
{
	{"type",		STATICTAG_EDIT_DWORD_VE("00000000")" ("STATICTAG_DESC_DWORD_VE("00000000", ATOM_ELF_PH_TYPE_STR)")"},
	{"offset",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"virtual address",	STATICTAG_EDIT_DWORD_VE("00000008")},
	{"physical address",	STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"in file size",	STATICTAG_EDIT_DWORD_VE("00000010")},
	{"in memory size",	STATICTAG_EDIT_DWORD_VE("00000014")},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000018")" "STATICTAG_FLAGS("00000018", ATOM_ELF_PH_FLAGS_STR)},
	{"alignment",		STATICTAG_EDIT_DWORD_VE("0000001c")},
	{0, 0}
};

static ht_mask_ptable elfprogramheader64[]=
{
	{"type",		STATICTAG_EDIT_DWORD_VE("00000000")" ("STATICTAG_DESC_DWORD_VE("00000000", ATOM_ELF_PH_TYPE_STR)")"},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000004")" "STATICTAG_FLAGS("00000004", ATOM_ELF_PH_FLAGS_STR)},
	{"offset",		STATICTAG_EDIT_QWORD_VE("00000008")},
	{"virtual address",	STATICTAG_EDIT_QWORD_VE("00000010")},
	{"physical address",	STATICTAG_EDIT_QWORD_VE("00000018")},
	{"in file size",	STATICTAG_EDIT_QWORD_VE("00000020")},
	{"in memory size",	STATICTAG_EDIT_QWORD_VE("00000028")},
	{"alignment",		STATICTAG_EDIT_QWORD_VE("00000030")},
	{0, 0}
};

static int_hash elf_ph_type[] =
{
	{ELF_PT_NULL, 		"null"},
	{ELF_PT_LOAD,		"load"},
	{ELF_PT_DYNAMIC,	"dynamic"},
	{ELF_PT_INTERP,		"interp"},
	{ELF_PT_NOTE,		"note"},
	{ELF_PT_SHLIB,		"shlib"},
	{ELF_PT_PHDR,		"phdr"},
	{ELF_PT_TLS,		"tls"},
	{ELF_PT_NUM,		"num"},
	{ELF_PT_GNU_EH_FRAME,	"gnu eh frame"},
	{ELF_PT_GNU_STACK,	"gnu stack"},
	{ELF_PT_GNU_RELRO,	"gnu relro"},
	{ELF_PT_PAX_FLAGS,	"pax flags"},
	{0, 0}
};

static ht_tag_flags_s elf_ph_flags[] =
{
	{0,  "[00] executable"},
	{1,  "[01] writable"},
	{2,  "[02] readable"},
	{0, 0}
};

static ht_view *htelfprogramheaders_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared = (ht_elf_shared_data *)group->get_shared_data();

	if (!elf_shared->pheaders.count) return NULL;

	bool elf_bigendian = (elf_shared->ident.e_ident[ELF_EI_DATA] == ELFDATA2MSB);
	ht_uformat_viewer *v = NULL;
	if (elf_shared->ident.e_ident[ELF_EI_CLASS]==ELFCLASS32) {
		v = new ht_uformat_viewer();
		v->init(b, DESC_ELF_PROGRAM_HEADERS, VC_EDIT, file, group);
	
		registerAtom(ATOM_ELF_PH_TYPE, elf_ph_type);
		registerAtom(ATOM_ELF_PH_FLAGS, elf_ph_flags);
	
		FileOfs h = elf_shared->header32.e_phoff;
	
		ht_mask_sub *m = new ht_mask_sub();
		m->init(file, 0);
	
		char info[128];
		ht_snprintf(info, sizeof info, "* ELF program headers at offset %08qx", h);
	
		m->add_mask(info);

		v->insertsub(m);
		for (uint i=0; i < elf_shared->pheaders.count; i++) {
		
			ht_mask_sub *n = new ht_mask_sub();
			n->init(file, i);
		
			char t[32];
			const char *etype = matchhash(elf_shared->pheaders.pheaders32[i].p_type, elf_ph_type);
			if (!etype) etype = "?";
			ht_snprintf(t, sizeof t, "entry %d (%s)", i, etype);
		
			n->add_staticmask_ptable(elfprogramheader32, h+i*elf_shared->header32.e_phentsize, elf_bigendian);
		
			ht_collapsable_sub *cn=new ht_collapsable_sub();
			cn->init(file, n, 1, t, 1);
	
			v->insertsub(cn);
		}
	} else if (elf_shared->ident.e_ident[ELF_EI_CLASS]==ELFCLASS64) {
		v = new ht_uformat_viewer();
		v->init(b, DESC_ELF_PROGRAM_HEADERS, VC_EDIT, file, group);
	
		registerAtom(ATOM_ELF_PH_TYPE, elf_ph_type);
		registerAtom(ATOM_ELF_PH_FLAGS, elf_ph_flags);

		FileOfs h = elf_shared->header64.e_phoff;
	
		ht_mask_sub *m=new ht_mask_sub();
		m->init(file, 0);
	
		char info[128];
		ht_snprintf(info, sizeof info, "* ELF program headers at offset 0x%08qx", h);
	
		m->add_mask(info);

		v->insertsub(m);
		for (uint i=0; i<elf_shared->pheaders.count; i++) {
		
			ht_mask_sub *n=new ht_mask_sub();
			n->init(file, i);
		
			char t[32];
			const char *etype=matchhash(elf_shared->pheaders.pheaders64[i].p_type, elf_ph_type);
			if (!etype) etype="?";
			ht_snprintf(t, sizeof t, "entry %d (%s)", i, etype);
		
			n->add_staticmask_ptable(elfprogramheader64, h+i*elf_shared->header64.e_phentsize, elf_bigendian);
		
			ht_collapsable_sub *cn=new ht_collapsable_sub();
			cn->init(file, n, 1, t, 1);
	
			v->insertsub(cn);
		}
	}
	
	return v;
}

format_viewer_if htelfprogramheaders_if = {
	htelfprogramheaders_init,
	0
};
