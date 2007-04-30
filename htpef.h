/* 
 *	HT Editor
 *	htpef.h
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

#ifndef __HTPEF_H__
#define __HTPEF_H__

#include "pefstruc.h"
#include "htpefimp.h"
#include "formats.h"
#include "endianess.h"
#include "htformat.h"

#define DESC_PEF		"pef"
#define DESC_PEF_HEADER		"pef/header"
#define DESC_PEF_IMPORTS	"pef/imports"
#define DESC_PEF_IMAGE		"pef/image"

#define ATOM_PEF_ARCH				0x50454600
#define ATOM_PEF_ARCH_STR			 "50454600"

#define ATOM_PEF_SECTION_KIND 			0x50454601
#define ATOM_PEF_SECTION_KIND_STR		 "50454601"

#define ATOM_PEF_SHARE_KIND 			0x50454602
#define ATOM_PEF_SHARE_KIND_STR			 "50454602"

extern format_viewer_if htpef_if;

struct pef_section_headers {
	uint count;
	PEF_SECTION_HEADER *sheaders;
};

struct ht_pef_shared_data {
	FileOfs header_ofs;
	PEF_CONTAINER_HEADER contHeader;
	Endianess byte_order;
	pef_section_headers sheaders;
	PEF_ARCH arch;
	FileOfs loader_info_header_ofs; 	// 0 if not loader_info_header
	PEF_LOADER_INFO_HEADER loader_info_header;
	
	ht_pef_import imports;
	
	ht_format_viewer *v_header;
	ht_view *v_imports;
	ht_format_viewer *v_image;
};

/*
 *	ht_pef
 */

class ht_pef: public ht_format_group {
protected:
	bool loc_enum;
	/* new */
public:
			void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs);
	virtual	void done();
};

bool pef_phys_and_mem_section(PEF_SECTION_HEADER *s);
bool pef_valid_section(PEF_SECTION_HEADER *s);

bool pef_addr_to_section(pef_section_headers *section_headers, PEFAddress addr, int *section);
bool pef_addr_to_ofs(pef_section_headers *section_headers, PEFAddress addr, uint32 *ofs);
bool pef_addr_is_valid(pef_section_headers *section_headers, PEFAddress addr);

bool pef_ofs_to_addr(pef_section_headers *section_headers, uint32 ofs, PEFAddress *addr);
bool pef_ofs_to_section(pef_section_headers *section_headers, uint32 ofs, uint32 *section);

#endif /* !__HTPEF_H__ */

