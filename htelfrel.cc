/* 
 *	HT Editor
 *	htelfrel.cc
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
#include "htelfrel.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

#include <stdlib.h>

static int_hash elf_r_386_type[] =
{
	{ELF_R_386_NONE,		"ELF_R_386_NONE"},
	{ELF_R_386_32,			"ELF_R_386_32"},
	{ELF_R_386_PC32,		"ELF_R_386_PC32"},
	{ELF_R_386_GOT32,		"ELF_R_386_GOT32"},
	{ELF_R_386_PLT32,		"ELF_R_386_PLT32"},
	{ELF_R_386_COPY,		"ELF_R_386_COPY"},
	{ELF_R_386_GLOB_DAT,		"ELF_R_386_GLOB_DAT"},
	{ELF_R_386_JMP_SLOT,		"ELF_R_386_JMP_SLOT"},
	{ELF_R_386_RELATIVE,		"ELF_R_386_RELATIVE"},
	{ELF_R_386_GOTOFF,		"ELF_R_386_GOTOFF"},
	{ELF_R_386_GOTPC,		"ELF_R_386_GOTPC"},
	{0, 0}
};

static int_hash elf_r_x86_64_type[] =
{
	{ELF_R_X86_64_NONE,		"ELF_R_X86_64_NONE"},
	{ELF_R_X86_64_64,		"ELF_R_X86_64_64"},
	{ELF_R_X86_64_PC32,		"ELF_R_X86_64_PC32"},
	{ELF_R_X86_64_GOT32,		"ELF_R_X86_64_GOT32"},
	{ELF_R_X86_64_PLT32,		"ELF_R_X86_64_PLT32"},
	{ELF_R_X86_64_COPY,		"ELF_R_X86_64_COPY"},
	{ELF_R_X86_64_GLOB_DAT,		"ELF_R_X86_64_GLOB_DAT"},
	{ELF_R_X86_64_JUMP_SLOT,	"ELF_R_X86_64_JUMP_SLOT"},
	{ELF_R_X86_64_RELATIVE,		"ELF_R_X86_64_RELATIVE"},
	{ELF_R_X86_64_GOTPCREL,		"ELF_R_X86_64_GOTPCREL"},
	{ELF_R_X86_64_32,		"ELF_R_X86_64_32"},
	{ELF_R_X86_64_32S,		"ELF_R_X86_64_32S"},
	{ELF_R_X86_64_16,		"ELF_R_X86_64_16"},
	{ELF_R_X86_64_PC16,		"ELF_R_X86_64_PC16"},
	{ELF_R_X86_64_8,		"ELF_R_X86_64_8"},
	{ELF_R_X86_64_PC8,		"ELF_R_X86_64_PC8"},
	{0, 0}
};

static ht_view *htelfreloctable_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();

	if (elf_shared->ident.e_ident[ELF_EI_CLASS] == ELFCLASS32)
	{
		if (elf_shared->header32.e_machine != ELF_EM_386)
			return NULL;
	} else if (elf_shared->ident.e_ident[ELF_EI_CLASS] == ELFCLASS64) {
		if (elf_shared->header32.e_machine != ELF_EM_X86_64)
			return NULL;
	} else {
		return NULL;
	}
	if ( elf_shared->ident.e_ident[ELF_EI_DATA] != ELFDATA2LSB)
		return NULL;

	bool elf32 = elf_shared->ident.e_ident[ELF_EI_CLASS] == ELFCLASS32;
	uint skip = elf_shared->reloctables;
	uint reloctab_shidx = ELF_SHN_UNDEF;
	uint reloctab_sh_type = ELF_SHT_NULL;
	for (uint i=1; i<elf_shared->sheaders.count; i++) {
		reloctab_sh_type = elf32 ? elf_shared->sheaders.sheaders32[i].sh_type : elf_shared->sheaders.sheaders64[i].sh_type;
		if ((reloctab_sh_type == ELF_SHT_REL) || (reloctab_sh_type==ELF_SHT_RELA)) {
			if (!skip--) {
				reloctab_shidx=i;
				break;
			}
		}
	}
	if (!isValidELFSectionIdx(elf_shared, reloctab_shidx)) return NULL;

	FileOfs h;
	
	/* section index of associated symbol table */
	int si_symbol;
	
	/* section index of section to be relocated */
	int si_dest;

	String reloctab_name("?");
	int rel_size;
	int rela_size;
	uint relnum;

	if (elf32) {
		h = elf_shared->sheaders.sheaders32[reloctab_shidx].sh_offset;
		si_symbol = elf_shared->sheaders.sheaders32[reloctab_shidx].sh_link;
		si_dest = elf_shared->sheaders.sheaders32[reloctab_shidx].sh_info;
		if (isValidELFSectionIdx(elf_shared, elf_shared->header32.e_shstrndx)) {
			file->seek(elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset
				+ elf_shared->sheaders.sheaders32[reloctab_shidx].sh_name);
			file->readStringz(reloctab_name);
		}
		rel_size = sizeof (ELF_REL32);
		rela_size = sizeof (ELF_RELA32);
		relnum = elf_shared->sheaders.sheaders32[reloctab_shidx].sh_size / (reloctab_sh_type == ELF_SHT_REL ? rel_size : rela_size);
	} else {
		h = elf_shared->sheaders.sheaders64[reloctab_shidx].sh_offset;
		si_symbol = elf_shared->sheaders.sheaders64[reloctab_shidx].sh_link;
		si_dest = elf_shared->sheaders.sheaders64[reloctab_shidx].sh_info;
		if (isValidELFSectionIdx(elf_shared, elf_shared->header64.e_shstrndx)) {
			file->seek(elf_shared->sheaders.sheaders64[elf_shared->header64.e_shstrndx].sh_offset
				+ elf_shared->sheaders.sheaders64[reloctab_shidx].sh_name);
			file->readStringz(reloctab_name);
		}
		rel_size = sizeof (ELF_REL64);
		rela_size = sizeof (ELF_RELA64);
		relnum = elf_shared->sheaders.sheaders64[reloctab_shidx].sh_size / (reloctab_sh_type == ELF_SHT_REL ? rel_size : rela_size);
	}

	char desc[128];
	ht_snprintf(desc, sizeof desc, DESC_ELF_RELOCTAB, &reloctab_name, reloctab_shidx);
	
	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, desc, VC_EDIT, file, group);
	
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	
	registerAtom(ATOM_ELF_R_386_TYPE, elf_r_386_type);
	registerAtom(ATOM_ELF_R_X86_64_TYPE, elf_r_x86_64_type);

	char t[256];
	ht_snprintf(t, sizeof t, "* ELF relocation table at offset %08qx, relocates section %d, symtab %d", h, si_dest, si_symbol);

	m->add_mask(t);

	bool elf_bigendian = (elf_shared->ident.e_ident[ELF_EI_DATA] == ELFDATA2MSB);
	tag_endian endianness = elf_bigendian ? tag_endian_big : tag_endian_little;

#define tt_end sizeof t - (tt-t)

	switch (reloctab_sh_type) {
		case ELF_SHT_REL: {
			if (elf32) {
				m->add_mask("destofs  symidx type");
			} else {
				m->add_mask("destofs          symidx     type");
			}
			for (uint i=0; i<relnum; i++) {
				char *tt = t;
				/* dest offset */
				if (elf32) {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rel_size, endianness);
				} else {
					tt = tag_make_edit_qword(tt, tt_end, h+i*rel_size, endianness);
				}
				tt += ht_snprintf(tt, tt_end, " ");
				
				/* symbol (table idx) */
				if (elf32) {
					tt = tag_make_edit_word(tt, tt_end, h+i*rel_size+4+1, endianness);
				} else {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rel_size+8+4, endianness);
				}
				tt += ht_snprintf(tt, tt_end, "   ");

				/* type */
				if (elf32) {
					tt = tag_make_edit_byte(tt, tt_end, h+i*rel_size+4);
				} else {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rel_size+8, endianness);
				}
				tt += ht_snprintf(tt, tt_end, "(");
				/* type */
				if (elf32) {
					tt = tag_make_desc_byte(tt, tt_end, h+i*rel_size+4, ATOM_ELF_R_386_TYPE);
				} else {
					tt = tag_make_desc_dword(tt, tt_end, h+i*rel_size+8, ATOM_ELF_R_X86_64_TYPE, endianness);
				}
				tt += ht_snprintf(tt, tt_end, ")");
				m->add_mask(t);
			}
			break;
		}
		case ELF_SHT_RELA: {
			if (elf32) {
				m->add_mask("destofs  symidx addend   type");
			} else {
				m->add_mask("destofs          symidx     addend           type");
			}
			for (uint i=0; i<relnum; i++) {
				char *tt = t;
				/* dest offset */
				if (elf32) {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rela_size, endianness);
				} else {
					tt = tag_make_edit_qword(tt, tt_end, h+i*rela_size, endianness);
				}
				tt += ht_snprintf(tt, tt_end, " ");
				/* symbol (table idx) */
				if (elf32) {
					tt = tag_make_edit_word(tt, tt_end, h+i*rela_size+4+1, endianness);
				} else {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rela_size+8, endianness);
				}
				tt += ht_snprintf(tt, tt_end, "   ");
				/* addend */
				if (elf32) {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rela_size+4+4, endianness);
				} else {
					tt = tag_make_edit_qword(tt, tt_end, h+i*rela_size+16, endianness);
				}
				tt += ht_snprintf(tt, tt_end, " ");
				/* type */
				if (elf32) {
					tt = tag_make_edit_byte(tt, tt_end, h+i*rela_size+4);
				} else {
					tt = tag_make_edit_dword(tt, tt_end, h+i*rela_size+8, endianness);
				}
				tt += ht_snprintf(tt, tt_end, "(");
				/* type */
				if (elf32) {
					tt = tag_make_desc_byte(tt, tt_end, h+i*rela_size+4, ATOM_ELF_R_386_TYPE);
				} else {
					tt = tag_make_desc_dword(tt, tt_end, h+i*rela_size+8, ATOM_ELF_R_X86_64_TYPE, endianness);
				}
				tt += ht_snprintf(tt, tt_end, ")");
				m->add_mask(t);
			}
			break;
		}
	}
	
	v->insertsub(m);
	return v;
}

format_viewer_if htelfreloctable_if = {
	htelfreloctable_init,
	0
};
