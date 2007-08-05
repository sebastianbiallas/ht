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
	union {
		MACHO_HEADER header;
		MACHO_HEADER64 header64;
	};
	macho_commands cmds;
	macho_sections sections;
	Endianess image_endianess;
	bool _64;
};

/*
 *	CLASS ht_macho
 */

class ht_macho: public ht_format_group {
public:
		void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs, Endianess image_endianess, bool _64);
};

typedef uint64 MACHOAddress;

bool macho_phys_and_mem_section(MACHO_SECTION *s, uint machoclass);
bool macho_valid_section(MACHO_SECTION *s, uint machoclass);

bool macho_addr_to_section(macho_sections *section_headers, uint machoclass, MACHOAddress addr, int *section);
bool macho_addr_to_ofs(macho_sections *section_headers, uint machoclass, MACHOAddress addr, uint32 *ofs);
bool macho_addr_is_valid(macho_sections *section_headers, uint machoclass, MACHOAddress addr);

bool macho_ofs_to_addr(macho_sections *section_headers, uint machoclass, uint32 ofs, MACHOAddress *addr);
bool macho_ofs_to_section(macho_sections *section_headers, uint machoclass, uint32 ofs, uint32 *section);

#endif /* !__HTMACHO_H__ */
