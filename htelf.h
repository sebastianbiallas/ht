/* 
 *	HT Editor
 *	htelf.h
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

#ifndef __HTELF_H__
#define __HTELF_H__

#include "elfstruc.h"
#include "formats.h"
#include "htanaly.h"
#include "htendian.h"
#include "htformat.h"
#include "relfile.h"

#define DESC_ELF "elf - unix exe/link format"
#define DESC_ELF_HEADER "elf/header"
#define DESC_ELF_SECTION_HEADERS "elf/section headers"
#define DESC_ELF_PROGRAM_HEADERS "elf/program headers"
#define DESC_ELF_SYMTAB "elf/symbol table %s (%d)"
#define DESC_ELF_RELOCTAB "elf/relocation table %s (%d)"
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
#define ATOM_ELF_R_386_TYPE_STR		 "454c460b"

extern format_viewer_if htelf_if;

struct elf_section_headers {
	UINT count;
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
	UINT count;
	union {
		ELF_PROGRAM_HEADER32 *pheaders32;
		ELF_PROGRAM_HEADER64 *pheaders64;
	};
};

struct ht_elf_reloc_section {
	elf32_addr address;
	UINT reloc_shidx;
};

struct ht_elf_shared_data {
	FILEOFS header_ofs;
	ELF_HEADER ident;
	endianess byte_order;
	union {
		ELF_HEADER32 header32;
		ELF_HEADER64 header64;
	};
	elf_section_headers sheaders;
	char **shnames;
	elf_program_headers pheaders;
	UINT symtables;
	UINT reloctables;
	ht_format_viewer *v_image;
	ht_elf_reloc_section *htrelocs;
	UINT fake_undefined_section;
};

/*
 *	CLASS ht_elf_aviewer
 */

class ht_elf_aviewer: public ht_aviewer {
public:
	ht_elf_shared_data *elf_shared;
	ht_streamfile *file;
		   void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, analyser *Analyser, ht_elf_shared_data *elf_shared);
	virtual void set_analyser(analyser *a);
};

/*
 *	CLASS ht_elf
 */

class ht_elf: public ht_format_group {
protected:
	bool loc_enum;
/* new */
			void auto_relocate();
			void fake_undefined_symbols();
			UINT find_reloc_section_for(UINT si);
			void relocate_section(ht_reloc_file *f, UINT si, UINT rsi, elf32_addr a);
public:
			void init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS header_ofs);
	virtual	void done();
/* overwritten */
	virtual	void loc_enum_start();
	virtual	bool loc_enum_next(ht_format_loc *loc);
};

/*
 *	CLASS ht_elf32_reloc_entry
 */

class ht_elf32_reloc_entry: public ht_data {
public:
	UINT	type;
	union {
		dword r_32;
		dword r_pc32;
	} relocs;
	
	ht_elf32_reloc_entry(UINT symtabidx, elf32_addr offset, UINT type, UINT symbolidx, elf32_addr addend, ht_elf_shared_data *data, ht_streamfile *file);
};

/*
 *	CLASS ht_elf32_reloc_file
 */

class ht_elf32_reloc_file: public ht_reloc_file {
protected:
	ht_elf_shared_data *data;
/* overwritten */
	virtual void	reloc_apply(ht_data *reloc, byte *data);
	virtual void	reloc_unapply(ht_data *reloc, byte *data);
public:
		   void	init(ht_streamfile *streamfile, bool own_streamfile, ht_elf_shared_data *data);
};

bool elf_phys_and_mem_section(elf_section_header *s, UINT elfclass);
bool elf_valid_section(elf_section_header *s, UINT elfclass);

bool elf_addr_to_section(elf_section_headers *section_headers, UINT elfclass, ADDR addr, int *section);
bool elf_addr_to_ofs(elf_section_headers *section_headers, UINT elfclass, ADDR addr, dword *ofs);
bool elf_addr_is_valid(elf_section_headers *section_headers, UINT elfclass, ADDR addr);
//bool elf_addr_is_physical(elf_section_headers *section_headers, UINT elfclass, ADDR addr);

bool elf_ofs_to_addr(elf_section_headers *section_headers, UINT elfclass, dword ofs, ADDR *addr);
bool elf_ofs_to_section(elf_section_headers *section_headers, UINT elfclass, dword ofs, ADDR *section);
//bool elf_ofs_to_addr_and_section(elf_section_headers *section_headers, UINT elfclass, dword ofs, ADDR *addr, ADDR *section);

#endif /* !__HTELF_H__ */
