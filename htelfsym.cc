/* 
 *	HT Editor
 *	htelfsym.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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
#include "htatom.h"
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

static ht_view *htelfsymboltable_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();

	if (elf_shared->ident.e_ident[ELF_EI_CLASS]!=ELFCLASS32) return 0;

	UINT skip = elf_shared->symtables;
	UINT symtab_shidx = ELF_SHN_UNDEF;
	for (UINT i=1; i<elf_shared->sheaders.count; i++) {
		if ((elf_shared->sheaders.sheaders32[i].sh_type == ELF_SHT_SYMTAB) || (elf_shared->sheaders.sheaders32[i].sh_type==ELF_SHT_DYNSYM)) {
			if (!skip--) {
				symtab_shidx = i;
				break;
			}
		}
	}
	if (symtab_shidx == ELF_SHN_UNDEF) return 0;

	FILEOFS h = elf_shared->sheaders.sheaders32[symtab_shidx].sh_offset;

	/* associated string table offset (from sh_link) */
	FILEOFS sto = elf_shared->sheaders.sheaders32[elf_shared->sheaders.sheaders32[symtab_shidx].sh_link].sh_offset;

	file->seek(elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset+elf_shared->sheaders.sheaders32[symtab_shidx].sh_name);
	char *symtab_name = fgetstrz(file);
	char desc[128];
	ht_snprintf(desc, sizeof desc, DESC_ELF_SYMTAB, symtab_name, symtab_shidx);
	free(symtab_name);

	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, desc, VC_EDIT | VC_SEARCH, file, group);

	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);

	register_atom(ATOM_ELF_ST_BIND, elf_st_bind);

	char t[256];
	ht_snprintf(t, sizeof t, "* ELF symtab at offset %08x", h);

	m->add_mask(t);

	m->add_mask("idx  binding  type     value    size     section     name");

	bool elf_bigendian = (elf_shared->ident.e_ident[ELF_EI_DATA] == ELFDATA2MSB);
	UINT symnum = elf_shared->sheaders.sheaders32[symtab_shidx].sh_size / sizeof (ELF_SYMBOL32);
	for (UINT i=0; i<symnum; i++) {
		ELF_SYMBOL32 sym;
		file->seek(h+i*sizeof (ELF_SYMBOL32));
		file->read(&sym, sizeof sym);
		create_host_struct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);
		file->seek(sto+sym.st_name);          
		char *name = fgetstrz(file);
		/* FIXME: error handling (also in elf_analy.cc) */
		if (!name) continue;

		char *tt = t;

		tt += ht_snprintf(tt, sizeof t - (tt-t), "%04x ", i);

		char *bind = matchhash(ELF32_ST_BIND(sym.st_info), elf_st_bind);
		if (bind) {
			tt += ht_snprintf(tt, sizeof t - (tt-t), "%-8s ", bind);
		} else {
			tt += ht_snprintf(tt, sizeof t - (tt-t), "? (%d) ", ELF32_ST_BIND(sym.st_info));
		}

		char *type = matchhash(ELF32_ST_TYPE(sym.st_info), elf_st_type);
		if (type) {
			tt += ht_snprintf(tt, sizeof t - (tt-t), "%-8s ", type);
		} else {
			tt += ht_snprintf(tt, sizeof t - (tt-t), "? (%d) ", ELF32_ST_TYPE(sym.st_info));
		}

		FILEOFS so = elf_shared->sheaders.sheaders32[elf_shared->header32.e_shstrndx].sh_offset;
		tt = tag_make_edit_dword(tt, h+i*sizeof (ELF_SYMBOL32)+4, elf_bigendian ? tag_endian_big : tag_endian_little);
		*tt++ = ' ';
		tt = tag_make_edit_dword(tt, h+i*sizeof (ELF_SYMBOL32)+8, elf_bigendian ? tag_endian_big : tag_endian_little);
		*tt++ = ' ';
		*tt = 0;
		switch (sym.st_shndx) {
			case ELF_SHN_UNDEF:
				tt += ht_snprintf(tt, sizeof t - (tt-t), "*undefined  ");
				break;
			case ELF_SHN_ABS:
				tt += ht_snprintf(tt, sizeof t - (tt-t), "*absolute   ");
				break;
			case ELF_SHN_COMMON:
				tt += ht_snprintf(tt, sizeof t - (tt-t), "*common     ");
				break;
			default: {
				file->seek(so+elf_shared->sheaders.sheaders32[sym.st_shndx].sh_name);
				char *s = fgetstrz(file);
				tt += ht_snprintf(tt, sizeof t - (tt-t), "%-11s ", s);
				free(s);
				break;
			}
		}
		tt += ht_snprintf(tt, sizeof t - (tt-t), "%s", name);
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
