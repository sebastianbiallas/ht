/* 
 *	HT Editor
 *	htelf.h
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

#ifndef __HTELF_H__
#define __HTELF_H__

#include "elfstruc.h"
#include "formats.h"
#include "endianess.h"
#include "htformat.h"
#include "relfile.h"

#define DESC_ELF "elf - unix exe/link format"
#define DESC_ELF_HEADER "elf/header"
#define DESC_ELF_SECTION_HEADERS "elf/section headers"
#define DESC_ELF_PROGRAM_HEADERS "elf/program headers"
#define DESC_ELF_SYMTAB "elf/symbol table %y (%d)"
#define DESC_ELF_RELOCTAB "elf/relocation table %y (%d)"
#define DESC_ELF_IMAGE "elf/image"

#define ATOM_ELF_CLASS	 			0x454c4600
#define ATOM_ELF_CLASS_STR			 "454c4600"

#define ATOM_ELF_DATA	 			0x454c4601
#define ATOM_ELF_DATA_STR			 "454c4601"

#define ATOM_ELF_OS_ABI	 			0x454c4602
#define ATOM_ELF_OS_ABI_STR			 "454c4602"

#define ATOM_ELF_TYPE	 			0x454c4603
#define ATOM_ELF_TYPE_STR			 "454c4603"

#define ATOM_ELF_MACHINE 			0x454c4604
#define ATOM_ELF_MACHINE_STR			 "454c4604"

#define ATOM_ELF_SH_TYPE 			0x454c4605
#define ATOM_ELF_SH_TYPE_STR			 "454c4605"

#define ATOM_ELF_SH_FLAGS 			0x454c4606
#define ATOM_ELF_SH_FLAGS_STR			 "454c4606"

#define ATOM_ELF_PH_TYPE 			0x454c4607
#define ATOM_ELF_PH_TYPE_STR			 "454c4607"

#define ATOM_ELF_PH_FLAGS 			0x454c4608
#define ATOM_ELF_PH_FLAGS_STR			 "454c4608"

#define ATOM_ELF_ST_BIND 			0x454c4609
#define ATOM_ELF_ST_BIND_STR			 "454c4609"

#define ATOM_ELF_ST_TYPE 			0x454c460a
#define ATOM_ELF_ST_TYPE_STR			 "454c460a"

#define ATOM_ELF_R_386_TYPE 			0x454c460b
#define ATOM_ELF_R_386_TYPE_STR		 	 "454c460b"

extern format_viewer_if htelf_if;

class FakeAddr: public Object {
public:
	uint secidx;
	uint symidx;
	uint32 addr;

	FakeAddr(uint asecidx, uint asymidx, uint32 aAddr)
		: secidx(asecidx), symidx(asymidx), addr(aAddr)
	{
	}

	virtual int compareTo(const Object *) const;
};

struct elf_section_headers {
	uint count;
	union {
		ELF_SECTION_HEADER32 *sheaders32;
		ELF_SECTION_HEADER64 *sheaders64;
	};
};

union elf_section_header {
	ELF_SECTION_HEADER32 sheaders32;
	ELF_SECTION_HEADER64 sheaders64;
};

struct elf_program_headers {
	uint count;
	union {
		ELF_PROGRAM_HEADER32 *pheaders32;
		ELF_PROGRAM_HEADER64 *pheaders64;
	};
};

struct ht_elf_reloc_section32 {
	elf32_addr	relocAddr;
	uint		relocShIdx;
};

struct ht_elf_shared_data {
	FileOfs header_ofs;
	ELF_HEADER ident;
	Endianess byte_order;
	union {
		ELF_HEADER32 header32;
		ELF_HEADER64 header64;
	};
	elf_section_headers sheaders;
	ht_elf_reloc_section32 *shrelocs;
	char **shnames;
	elf_program_headers pheaders;
	uint symtables;
	uint reloctables;
	ht_format_viewer *v_image;
	int fake_undefined_shidx;
	uint fake_undefined_size;
	Container *undefined2fakeaddr;
};

/*
 *	ht_elf
 */
class ht_elf: public ht_format_group {
protected:
	bool loc_enum;
	/* new */
		void auto_relocate32();
		void fake_undefined_symbols32();
		uint find_reloc_section_for(uint si);
		void relocate_section(ht_reloc_file *f, uint si, uint rsi, elf32_addr a);
public:
		void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs);
	virtual	void done();
	/* extends ? */
	virtual	void loc_enum_start();
	virtual	bool loc_enum_next(ht_format_loc *loc);
};

/*
 *	ht_elf32_reloc_entry
 */

class ht_elf32_reloc_entry: public Object {
public:
	uint	type;
	union {
		uint32 r_32;
		uint32 r_pc32;
	} relocs;

	ht_elf32_reloc_entry(uint type, uint32 A, uint32 P, uint32 S);
};

/*
 *	ht_elf32_reloc_file
 */

class ht_elf32_reloc_file: public ht_reloc_file {
protected:
	ht_elf_shared_data *data;
	/* extends ht_reloc_file */
	virtual void	reloc_apply(Object *reloc, byte *data);
	virtual bool	reloc_unapply(Object *reloc, byte *data);
public:
		   	ht_elf32_reloc_file(File *File, bool own_streamfile, ht_elf_shared_data *data);
};

bool isValidELFSectionIdx(ht_elf_shared_data *elf_shared, int idx);

bool elf_phys_and_mem_section(elf_section_header *s, uint elfclass);
bool elf_valid_section(elf_section_header *s, uint elfclass);

bool elf_addr_to_section(elf_section_headers *section_headers, uint elfclass, ELFAddress addr, int *section);
bool elf_addr_to_ofs(elf_section_headers *section_headers, uint elfclass, ELFAddress addr, FileOfs *ofs);
bool elf_addr_is_valid(elf_section_headers *section_headers, uint elfclass, ELFAddress addr);

bool elf_ofs_to_addr(elf_section_headers *section_headers, uint elfclass, FileOfs ofs, ELFAddress *addr);
bool elf_ofs_to_section(elf_section_headers *section_headers, uint elfclass, FileOfs ofs, uint32 *section);

#endif /* !__HTELF_H__ */
