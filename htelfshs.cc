/* 
 *	HT Editor
 *	htelfshs.cc
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
#include "except.h"
#include "htelf.h"
#include "htelfshs.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable elfsectionheader32[]=
{
	{"name string index",	STATICTAG_EDIT_DWORD_VE("00000000")},
	{"type",				STATICTAG_EDIT_DWORD_VE("00000004")" ("STATICTAG_DESC_DWORD_VE("00000004", ATOM_ELF_SH_TYPE_STR)")"},
	{"flags",				STATICTAG_EDIT_DWORD_VE("00000008")" "STATICTAG_FLAGS("00000008", ATOM_ELF_SH_FLAGS_STR)},
	{"address",			STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"offset",			STATICTAG_EDIT_DWORD_VE("00000010")},
	{"size",				STATICTAG_EDIT_DWORD_VE("00000014")},
	{"link",				STATICTAG_EDIT_DWORD_VE("00000018")},
	{"info",				STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"alignment",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"entsize",			STATICTAG_EDIT_DWORD_VE("00000024")},
	{0, 0}
};

static ht_mask_ptable elfsectionheader64[]=
{
	{"name string index",	STATICTAG_EDIT_DWORD_VE("00000000")},
	{"type",				STATICTAG_EDIT_DWORD_VE("00000004")" ("STATICTAG_DESC_DWORD_VE("00000004", ATOM_ELF_SH_TYPE_STR)")"},
	{"flags",				STATICTAG_EDIT_QWORD_VE("00000008")" "STATICTAG_FLAGS("00000008", ATOM_ELF_SH_FLAGS_STR)},
	{"address",			STATICTAG_EDIT_QWORD_VE("00000010")},
	{"offset",			STATICTAG_EDIT_QWORD_VE("00000018")},
	{"size",				STATICTAG_EDIT_QWORD_VE("00000020")},
	{"link",				STATICTAG_EDIT_QWORD_VE("00000028")},
	{"info",				STATICTAG_EDIT_QWORD_VE("00000030")},
	{"alignment",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"entsize",			STATICTAG_EDIT_DWORD_VE("0000003c")},
	{0, 0}
};

static int_hash elf_sh_type[] =
{
	{ELF_SHT_NULL, 		"null"},
	{ELF_SHT_PROGBITS,	"progbits"},
	{ELF_SHT_SYMTAB,	"symbol table"},
	{ELF_SHT_STRTAB,	"string table"},
	{ELF_SHT_RELA,		"relocation table + addends"},
	{ELF_SHT_HASH,		"symbol hash table"},
	{ELF_SHT_DYNAMIC,	"dynamic linking information"},
	{ELF_SHT_NOTE,		"note"},
	{ELF_SHT_NOBITS,	"nobits"},
	{ELF_SHT_REL,		"relocation table"},
	{ELF_SHT_SHLIB,		"shlib"},
	{ELF_SHT_DYNSYM,	"dynamic linking symbol table"},
	{ELF_SHT_INIT_ARRAY,	"init array"},
	{ELF_SHT_FINI_ARRAY,	"fini array"},
	{ELF_SHT_PREINIT_ARRAY,	"preinit array"},
	{0, 0}
};

static ht_tag_flags_s elf_sh_flags[] =
{
	{0,  "[00] writable"},
	{1,  "[01] alloc"},
	{2,  "[02] executable"},
	{3,  "[03] ???"},
	{4,  "[04] merge"},
	{5,  "[05] strings"},
	{6,  "[06] info link"},
	{7,  "[07] link order"},
	{8,  "[08] OS non-conforming"},
	{0, 0}
};

static ht_view *htelfsectionheaders_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();

	if (elf_shared->sheaders.count == 0) {
		return NULL;
	}

	ht_uformat_viewer *v = NULL;
	bool elf_bigendian = elf_shared->ident.e_ident[ELF_EI_DATA]==ELFDATA2MSB;
	if (elf_shared->ident.e_ident[ELF_EI_CLASS] == ELFCLASS32) {
		v = new ht_uformat_viewer();
		v->init(b, DESC_ELF_SECTION_HEADERS, VC_EDIT, file, group);

		registerAtom(ATOM_ELF_SH_TYPE, elf_sh_type);
		registerAtom(ATOM_ELF_SH_FLAGS, elf_sh_flags);

		FileOfs h = elf_shared->header32.e_shoff;

		ht_mask_sub *m = new ht_mask_sub();
		m->init(file, 0);

		char info[128];
		ht_snprintf(info, sizeof info, "* ELF section headers at offset 0x%08qx", h);

		m->add_mask(info);

		v->insertsub(m);

		elf_shared->shnames = ht_malloc(elf_shared->sheaders.count * sizeof *elf_shared->shnames);
		FileOfs so;
		if (elf_shared->header32.e_shstrndx < elf_shared->sheaders.count) {
			so = elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset;
		} else {
			so = -1;
		}
		String s;
		for (uint i=0; i < elf_shared->sheaders.count; i++) {
			s = "?";

			try {
				if (so != -1) {
					file->seek(so + elf_shared->sheaders.sheaders32[i].sh_name);
					file->readStringz(s);
				}
			} catch (const Exception &x) {
				// and now?
			}

			char t[1024];
			ht_snprintf(t, sizeof t, "section %d: %y", i, &s);
			elf_shared->shnames[i] = ht_strdup(s.contentChar());

			ht_mask_sub *n = new ht_mask_sub();
			n->init(file, i);

			n->add_staticmask_ptable(elfsectionheader32, h+i*elf_shared->header32.e_shentsize, elf_bigendian);

			ht_collapsable_sub *cn = new ht_collapsable_sub();
			cn->init(file, n, 1, t, 1);

			v->insertsub(cn);
		}
	} else if (elf_shared->ident.e_ident[ELF_EI_CLASS]==ELFCLASS64) {
		v = new ht_uformat_viewer();
		v->init(b, DESC_ELF_SECTION_HEADERS, VC_EDIT, file, group);

		registerAtom(ATOM_ELF_SH_TYPE, elf_sh_type);
		registerAtom(ATOM_ELF_SH_FLAGS, elf_sh_flags);

		/* FIXME: 64-bit */
		FileOfs h = elf_shared->header64.e_shoff;

		ht_mask_sub *m=new ht_mask_sub();
		m->init(file, 0);

		char info[128];
		ht_snprintf(info, sizeof info, "* ELF section headers at offset 0x%08qx", h);

		m->add_mask(info);

		v->insertsub(m);

		elf_shared->shnames = ht_malloc(elf_shared->sheaders.count * sizeof *elf_shared->shnames);

		FileOfs so = elf_shared->sheaders.sheaders64[elf_shared->header64.e_shstrndx].sh_offset;
		for (uint i=0; i < elf_shared->sheaders.count; i++) {
			char *s;
			try {
				file->seek(so+elf_shared->sheaders.sheaders64[i].sh_name);
				s = file->fgetstrz();
			} catch (const EOFException &) {
				s = ht_strdup("?");
			}
			char t[1024];
			ht_snprintf(t, sizeof t, "section %d: %s", i, s);
			elf_shared->shnames[i]=s;

			ht_mask_sub *n=new ht_mask_sub();
			n->init(file, i);

			n->add_staticmask_ptable(elfsectionheader64, h+i*elf_shared->header64.e_shentsize, elf_bigendian);

			ht_collapsable_sub *cn=new ht_collapsable_sub();
			cn->init(file, n, 1, t, 1);

			v->insertsub(cn);
		}
	}

	return v;
}

format_viewer_if htelfsectionheaders_if = {
	htelfsectionheaders_init,
	0
};
