/* 
 *	HT Editor
 *	htmach-o.h
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

#ifndef __HTMACHO_H__
#define __HTMACHO_H__

#include "machostruc.h"
#include "formats.h"
#include "endianess.h"
#include "htformat.h"

#define DESC_MACHO "Mach-O - Mach exe/link format"
#define DESC_MACHO_HEADER "Mach-O/header"
/*#define DESC_MACHO_SECTION_HEADERS "elf/section headers"
#define DESC_MACHO_PROGRAM_HEADERS "elf/program headers"
#define DESC_MACHO_SYMTAB "elf/symbol table %s (%d)"
#define DESC_MACHO_RELOCTAB "elf/relocation table %s (%d)"*/
#define DESC_MACHO_IMAGE "Mach-O/image"

extern format_viewer_if htmacho_if;

/*
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

union macho_segment_header {
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

struct ht_elf_reloc_section {
	elf32_addr address;
	uint reloc_shidx;
};
*/
struct macho_commands {
	uint count;
	MACHO_COMMAND_U **cmds;
};

struct macho_sections {
	uint count;
	MACHO_SECTION *sections;
};

struct ht_macho_shared_data {
	FileOfs header_ofs;
	MACHO_HEADER header;
	macho_commands cmds;
	macho_sections sections;
	Endianess image_endianess;
/*	ELF_HEADER ident;
	endianess byte_order;
	union {
		ELF_HEADER32 header32;
		ELF_HEADER64 header64;
	};
	elf_section_headers sheaders;
	char **shnames;
	elf_program_headers pheaders;
	uint symtables;
	uint reloctables;
	ht_format_viewer *v_image;
	ht_elf_reloc_section *htrelocs;
	uint fake_undefined_section;*/
};

/*
 *	CLASS ht_macho
 */

class ht_macho: public ht_format_group {
public:
		void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs, Endianess image_endianess);
};

typedef uint32 MACHOAddress;

bool macho_phys_and_mem_section(MACHO_SECTION *s, uint machoclass);
bool macho_valid_section(MACHO_SECTION *s, uint machoclass);

bool macho_addr_to_section(macho_sections *section_headers, uint machoclass, MACHOAddress addr, int *section);
bool macho_addr_to_ofs(macho_sections *section_headers, uint machoclass, MACHOAddress addr, uint32 *ofs);
bool macho_addr_is_valid(macho_sections *section_headers, uint machoclass, MACHOAddress addr);

bool macho_ofs_to_addr(macho_sections *section_headers, uint machoclass, uint32 ofs, MACHOAddress *addr);
bool macho_ofs_to_section(macho_sections *section_headers, uint machoclass, uint32 ofs, uint32 *section);

#endif /* !__HTMACHO_H__ */
