/* 
 *	HT Editor
 *	htpe.h
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

#ifndef __HTPE_H__
#define __HTPE_H__

#include "formats.h"
#include "pestruct.h"
#include "htpeexp.h"
#include "htpeil.h"
#include "htpeimp.h"
#include "htpedimp.h"

#define DESC_PE "pe - win32 portable exe"
#define DESC_PE_HEADER "pe/header"
#define DESC_PE_IMPORTS "pe/imports"
#define DESC_PE_DIMPORTS "pe/delay-imports"
#define DESC_PE_EXPORTS "pe/exports"
#define DESC_PE_RESOURCES "pe/resources"
#define DESC_PE_IMAGE "pe/image"
#define DESC_PE_IL "pe/il"

#define ATOM_PE_MACHINES 			0x50450000
#define ATOM_PE_MACHINES_STR			 "50450000"

#define ATOM_PE_OPTIONAL_MAGICS 		0x50450001
#define ATOM_PE_OPTIONAL_MAGICS_STR 		 "50450001"

#define ATOM_PE_SUBSYSTEMS 			0x50450002
#define ATOM_PE_SUBSYSTEMS_STR 			 "50450002"

#define ATOM_PE_CHARACTERISTICS			0x50450003
#define ATOM_PE_CHARACTERISTICS_STR		 "50450003"

#define ATOM_PE_SECTION_CHARACTERISTICS		0x50450004
#define ATOM_PE_SECTION_CHARACTERISTICS_STR	 "50450004"

extern format_viewer_if htpe_if;

struct pe_section_headers {
	UINT section_count;
	COFF_SECTION_HEADER *sections;
};

struct ht_pe_shared_data {
	FILEOFS header_ofs;
	COFF_HEADER coffheader;
	word opt_magic;
	union {
		struct {
			COFF_OPTIONAL_HEADER32 header;
			PE_OPTIONAL_HEADER32_NT header_nt;
		} pe32;
		struct {
			COFF_OPTIONAL_HEADER64 header;
			PE_OPTIONAL_HEADER64_NT header_nt;
		} pe64;
	};
	pe_section_headers sections;
	ht_pe_il *il;
	ht_pe_export exports;
	ht_pe_import imports;
	ht_pe_import dimports;
	ht_format_viewer *v_header;
	ht_view *v_exports;
	ht_view *v_imports;
	ht_view *v_dimports;
	ht_view *v_resources;
	ht_format_viewer *v_image;	
};

/*
 *	CLASS ht_pe
 */

class ht_pe: public ht_format_group {
protected:
	bool loc_enum;
public:
			void init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS header_ofs);
	virtual	void done();
/* overwritten */
	virtual   void loc_enum_start();
	virtual   bool loc_enum_next(ht_format_loc *loc);
};

bool pe_rva_to_section(pe_section_headers *section_headers, RVA rva, int *section);
bool pe_rva_to_ofs(pe_section_headers *section_headers, RVA rva, FILEOFS *ofs);
bool pe_rva_is_valid(pe_section_headers *section_headers, RVA rva);
bool pe_rva_is_physical(pe_section_headers *section_headers, RVA rva);

bool pe_ofs_to_rva(pe_section_headers *section_headers, FILEOFS ofs, RVA *rva);
bool pe_ofs_to_section(pe_section_headers *section_headers, FILEOFS ofs, int *section);
bool pe_ofs_to_rva_and_section(pe_section_headers *section_headers, FILEOFS ofs, RVA *rva, int *section);

#endif /* !__HTPE_H__ */
