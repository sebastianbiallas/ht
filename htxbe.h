/* 
 *	HT Editor
 *	htxbe.h
 *
 *	Copyright (C) 2003 Stefan Esser
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

#ifndef __HTXBE_H__
#define __HTXBE_H__

#include "formats.h"
#include "xbestruct.h"
#include "htxbeimp.h"

#define DESC_XBE "xbe - XBOX executable"
#define DESC_XBE_HEADER "xbe/header"
#define DESC_XBE_IMAGE "xbe/image"
#define DESC_XBE_IMPORTS "xbe/imports"


#define ATOM_XBE_INIT_FLAGS 			0x58420000
#define ATOM_XBE_INIT_FLAGS_STR			 "58420000"

#define ATOM_XBE_SECTION_FLAGS 			0x58420001
#define ATOM_XBE_SECTION_FLAGS_STR 		 "58420001"

#define ATOM_XBE_MEDIA_FLAGS 			0x58420002
#define ATOM_XBE_MEDIA_FLAGS_STR 		 "58420002"

#define ATOM_XBE_REGION 			0x58420003
#define ATOM_XBE_REGION_STR 			 "58420003"

#define ATOM_XBE_LIBRARY_FLAGS 			0x58420004
#define ATOM_XBE_LIBRARY_FLAGS_STR 		 "58420004"


extern format_viewer_if htxbe_if;

struct xbe_section_headers {
	uint32	number_of_sections;
	uint32	base_address;
	XBE_SECTION_HEADER *sections;
};

struct ht_xbe_shared_data {
	XBE_IMAGE_HEADER header;
	XBE_CERTIFICATE certificate;
	XBE_LIBRARY_VERSION *libraries;

	char *headerspace;
	xbe_section_headers sections;

	ht_xbe_import imports;


	ht_format_viewer *v_header;
	ht_view *v_exports;
	ht_view *v_imports;
	ht_view *v_dimports;
	ht_view *v_resources;
	ht_view *v_il;
	ht_format_viewer *v_image;	
};

/*
 *	ht_xbe
 */
class ht_xbe: public ht_format_group {
protected:
	bool loc_enum;
public:
		void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs);
	virtual	void done();
	/* overwritten */
	virtual   void loc_enum_start();
	virtual   bool loc_enum_next(ht_format_loc *loc);
};

bool xbe_rva_to_section(xbe_section_headers *section_headers, RVA rva, int *section);
bool xbe_rva_to_ofs(xbe_section_headers *section_headers, RVA rva, FileOfs *ofs);
bool xbe_rva_is_valid(xbe_section_headers *section_headers, RVA rva);
bool xbe_rva_is_physical(xbe_section_headers *section_headers, RVA rva);

bool xbe_ofs_to_rva(xbe_section_headers *section_headers, FileOfs ofs, RVA *rva);
bool xbe_ofs_to_section(xbe_section_headers *section_headers, FileOfs ofs, int *section);
bool xbe_ofs_to_rva_and_section(xbe_section_headers *section_headers, FileOfs ofs, RVA *rva, int *section);

bool xbe_section_name_to_section(xbe_section_headers *section_headers, const char *name, int *section);

#endif /* !__HTXBE_H__ */
