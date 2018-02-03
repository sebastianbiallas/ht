/* 
 *	HT Editor
 *	htelfsym.cc
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
#include "htelfsym.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

#include <stdlib.h>

static int_hash elf_st_bind[] =
{
	{ELF_STB_LOCAL, 	"local"},
	{ELF_STB_GLOBAL, 	"global"},
	{ELF_STB_WEAK, 		"weak"},
	{0, 0}
};

static int_hash elf_st_type[] =
{
	{ELF_STT_NOTYPE,	"no type"},
	{ELF_STT_OBJECT,	"object"},
	{ELF_STT_FUNC,		"func"},
	{ELF_STT_SECTION,	"section"},
	{ELF_STT_FILE,		"file"},
	{ELF_STT_COMMON,	"common"},
	{0, 0}
};

static ht_view *htelfsymboltable_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();

	if (elf_shared->ident.e_ident[ELF_EI_CLASS]!=ELFCLASS32 &&
		elf_shared->ident.e_ident[ELF_EI_CLASS]!=ELFCLASS64) return 0;

	bool elf32 = elf_shared->ident.e_ident[ELF_EI_CLASS] == ELFCLASS32;
	uint skip = elf_shared->symtables;
	uint symtab_shidx = ELF_SHN_UNDEF;
	for (uint i=1; i<elf_shared->sheaders.count; i++) {
		if (elf32 ?
			(elf_shared->sheaders.sheaders32[i].sh_type == ELF_SHT_SYMTAB) || (elf_shared->sheaders.sheaders32[i].sh_type==ELF_SHT_DYNSYM) :
			(elf_shared->sheaders.sheaders64[i].sh_type == ELF_SHT_SYMTAB) || (elf_shared->sheaders.sheaders64[i].sh_type==ELF_SHT_DYNSYM) ) {
			if (!skip--) {
				symtab_shidx = i;
				break;
			}
		}
	}
	if (!isValidELFSectionIdx(elf_shared, symtab_shidx)) return NULL;

	FileOfs h = elf32 ? elf_shared->sheaders.sheaders32[symtab_shidx].sh_offset : elf_shared->sheaders.sheaders64[symtab_shidx].sh_offset;

	/* associated string table offset (from sh_link) */
	FileOfs sto;
	String symtab_name("?");
	if (elf32) {
		uint idx = elf_shared->sheaders.sheaders32[symtab_shidx].sh_link;
		if (idx >= elf_shared->sheaders.count) return NULL;
		sto = elf_shared->sheaders.sheaders32[idx].sh_offset;
		if (isValidELFSectionIdx(elf_shared, elf_shared->header32.e_shstrndx)) {
			file->seek(elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset
				+ elf_shared->sheaders.sheaders32[symtab_shidx].sh_name);
			file->readStringz(symtab_name);
		}
	} else {
		uint idx = elf_shared->sheaders.sheaders64[symtab_shidx].sh_link;
		if (idx >= elf_shared->sheaders.count) return NULL;
		sto = elf_shared->sheaders.sheaders64[idx].sh_offset;
		if (isValidELFSectionIdx(elf_shared, elf_shared->header64.e_shstrndx)) {
			file->seek(elf_shared->sheaders.sheaders64[elf_shared->header64.e_shstrndx].sh_offset
				+ elf_shared->sheaders.sheaders64[symtab_shidx].sh_name);
			file->readStringz(symtab_name);
		}
	}

	char desc[128];
	ht_snprintf(desc, sizeof desc, DESC_ELF_SYMTAB, &symtab_name, symtab_shidx);

	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, desc, VC_EDIT | VC_SEARCH, file, group);

	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);

	registerAtom(ATOM_ELF_ST_BIND, elf_st_bind);

	char t[256];
	ht_snprintf(t, sizeof t, "* ELF symtab at offset %08qx", h);

	m->add_mask(t);

	bool elf_bigendian = (elf_shared->ident.e_ident[ELF_EI_DATA] == ELFDATA2MSB);
	uint symnum = elf32 ?
		elf_shared->sheaders.sheaders32[symtab_shidx].sh_size / sizeof (ELF_SYMBOL32) :
		elf_shared->sheaders.sheaders64[symtab_shidx].sh_size / sizeof (ELF_SYMBOL64);

	/* find maximum section name length */
	uint msnl = 0;
	for (uint i=0; i < symnum; i++) {
		elf64_quarter st_shndx;
		if (elf32)
		{
			ELF_SYMBOL32 sym;
			file->seek(h+i*sizeof (ELF_SYMBOL32));
			file->readx(&sym, sizeof sym);
			createHostStruct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);
			file->seek(sto+sym.st_name);
			st_shndx = sym.st_shndx;
		} else {
			ELF_SYMBOL64 sym;
			file->seek(h+i*sizeof (ELF_SYMBOL64));
			file->readx(&sym, sizeof sym);
			createHostStruct(&sym, ELF_SYMBOL64_struct, elf_shared->byte_order);
			file->seek(sto+sym.st_name);
			st_shndx = sym.st_shndx;
		}

		char *name = file->fgetstrz();
		/* FIXME: error handling (also in elf_analy.cc) */
		if (!name) continue;

		uint len = 0;
		switch (st_shndx) {
		case ELF_SHN_UNDEF:
			len = strlen("*undefined");
			break;
		case ELF_SHN_ABS:
			len = strlen("*absolute");
			break;
		case ELF_SHN_COMMON:
			len = strlen("*common");
			break;
		default:
			if (isValidELFSectionIdx(elf_shared, st_shndx)) {
				String s("");
				FileOfs so = UINT64_MAX, no = UINT64_MAX;
				if (elf32 && isValidELFSectionIdx(elf_shared, elf_shared->header32.e_shstrndx))
				{
					so = elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset;
					no = elf_shared->sheaders.sheaders32[st_shndx].sh_name;
				} else if (isValidELFSectionIdx(elf_shared, elf_shared->header64.e_shstrndx)) {
					so = elf_shared->sheaders.sheaders64[elf_shared->header64.e_shstrndx].sh_offset;
					no = elf_shared->sheaders.sheaders64[st_shndx].sh_name;
				}
				if (so != UINT64_MAX && no != UINT64_MAX)
				{
					file->seek(so + no);
					file->readStringz(s);
				}

				len = s.length();
			}
			break;
		}

		if (len > msnl)
			msnl = len;

		free(name);
	}

	/* create format string for section column */
	char secstrfmt[10];
	snprintf(secstrfmt, sizeof secstrfmt, "%%-%ds  ", msnl);

	char *tt = t;
#define tt_end sizeof t - (tt-t)
	if (elf32) {
		tt += ht_snprintf(t, tt_end, "idx  binding  type     value    size     ");
	} else {
		tt += ht_snprintf(t, tt_end, "idx  binding  type     value            size             ");
	}
	tt += ht_snprintf(tt, tt_end, secstrfmt, "section");
	tt += ht_snprintf(tt, tt_end, "name");
	m->add_mask(t);

	for (uint i=0; i < symnum; i++) {
		elf64_quarter st_shndx;
		int st_bind;
		int st_type;
		if (elf32)
		{
			ELF_SYMBOL32 sym;
			file->seek(h+i*sizeof (ELF_SYMBOL32));
			file->readx(&sym, sizeof sym);
			createHostStruct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);
			file->seek(sto+sym.st_name);
			st_shndx = sym.st_shndx;
			st_bind = ELF32_ST_BIND(sym.st_info);
			st_type = ELF32_ST_TYPE(sym.st_info);
		} else {
			ELF_SYMBOL64 sym;
			file->seek(h+i*sizeof (ELF_SYMBOL64));
			file->readx(&sym, sizeof sym);
			createHostStruct(&sym, ELF_SYMBOL64_struct, elf_shared->byte_order);
			file->seek(sto+sym.st_name);
			st_shndx = sym.st_shndx;
			st_bind = ELF64_ST_BIND(sym.st_info);
			st_type = ELF64_ST_TYPE(sym.st_info);
		}
		char *name = file->fgetstrz();
		/* FIXME: error handling (also in elf_analy.cc) */
		if (!name) continue;

		char *tt = t;

		tt += ht_snprintf(tt, tt_end, "%04x ", i);

		const char *bind = matchhash(st_bind, elf_st_bind);
		if (bind) {
			tt += ht_snprintf(tt, tt_end, "%-8s ", bind);
		} else {
			tt += ht_snprintf(tt, tt_end, "? (%d) ", st_bind);
		}

		const char *type = matchhash(st_type, elf_st_type);
		if (type) {
			tt += ht_snprintf(tt, tt_end, "%-8s ", type);
		} else {
			tt += ht_snprintf(tt, tt_end, "? (%d) ", st_type);
		}

		FileOfs so = UINT64_MAX;
		if (elf32) {
			if (isValidELFSectionIdx(elf_shared, elf_shared->header32.e_shstrndx))
				so = elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset;

			tt = tag_make_edit_dword(tt, tt_end, h + i*sizeof (ELF_SYMBOL32) + 4,
				elf_bigendian ? tag_endian_big : tag_endian_little);
			tt += ht_snprintf(tt, tt_end, " ");
			tt = tag_make_edit_dword(tt, tt_end, h + i*sizeof (ELF_SYMBOL32) + 8,
				elf_bigendian ? tag_endian_big : tag_endian_little);
			tt += ht_snprintf(tt, tt_end, " ");
		} else {
			if (isValidELFSectionIdx(elf_shared, elf_shared->header64.e_shstrndx))
				so = elf_shared->sheaders.sheaders64[elf_shared->header64.e_shstrndx].sh_offset;

			tt = tag_make_edit_qword(tt, tt_end, h+i*sizeof (ELF_SYMBOL64) + 8,
				elf_bigendian ? tag_endian_big : tag_endian_little);
			tt += ht_snprintf(tt, tt_end, " ");
			tt = tag_make_edit_qword(tt, tt_end, h+i*sizeof (ELF_SYMBOL64) + 16,
				elf_bigendian ? tag_endian_big : tag_endian_little);
			tt += ht_snprintf(tt, tt_end, " ");
		}

		String s("?");
		switch (st_shndx) {
		case ELF_SHN_UNDEF:
			s.assign("*undefined");
			break;
		case ELF_SHN_ABS:
			s.assign("*absolute");
			break;
		case ELF_SHN_COMMON:
			s.assign("*common");
			break;
		default:
			if (isValidELFSectionIdx(elf_shared, st_shndx)  && so != UINT64_MAX) {
				FileOfs no = elf32 ? elf_shared->sheaders.sheaders32[st_shndx].sh_name : elf_shared->sheaders.sheaders64[st_shndx].sh_name;
				file->seek(so + no);
				file->readStringz(s);
			}
			break;
		}
		s.escape("", true);
		tt += ht_snprintf(tt, tt_end, secstrfmt, s.contentChar());
		String sname(name);
		sname.escape("", true);
		tt += ht_snprintf(tt, tt_end, "%y", &sname);
		free(name);
		m->add_mask(t);
	}

	v->insertsub(m);
	return v;
}

format_viewer_if htelfsymboltable_if = {
	htelfsymboltable_init,
	0
};
