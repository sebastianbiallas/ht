/* 
 *	HT Editor
 *	htxex.h
 *
 *	Copyright (C) 2006 Sebastian Biallas (sb@biallas.net)
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

#ifndef __HTXEX_H__
#define __HTXEX_H__

#include "formats.h"
#include "xexstruct.h"

#define DESC_XEX "xex - xenon executable"
#define DESC_XEX_HEADER "xex/header"


extern format_viewer_if htxex_if;

struct xex_info_entry {
	FileOfs start;
	FileOfs size;
	uint32 type;
};

struct xex_loader_info_raw {
	Array *sections;
};

struct xex_loader_info_compressed {
	uint32 window;
};

class XEXLoaderRawSection: public Object {
public:
	uint32 raw;
	uint32 pad;
	
	XEXLoaderRawSection(uint32 aRaw, uint32 aPad)
	    : raw(aRaw), pad(aPad) {}
};

struct xex_loader_info {
	int type;
	union {
		xex_loader_info_raw raw;
		xex_loader_info_compressed compressed;
	};
};

struct XexImportFunc {
	uint32 ord;
	uint32 patch;
	uint32 ia;
};

struct XexImportLib {
	char *name;
	int func_count;
	XexImportFunc *funcs;
};

struct XexImports {
	FileOfs ofs;
	uint32	lib_count;
	XexImportLib *libs;
};

struct XexPage {
	uint32 flags;
};

struct XexPages {
	uint32 page_shift;
	XexPage *page;	
};

struct ht_xex_shared_data {
	XEX_IMAGE_HEADER header;
	XEX_IMAGE_HEADER_INFO_ENTRY *info_table;
	xex_info_entry *info_table_cooked;

	XEX_FILE_HEADER file_header;
	xex_loader_info loader_info;
	XexImports imports;
	XexPages pages;
	
	uint32 original_base_address;
	uint32 image_base;
	uint32 image_size;
	uint32 entrypoint;

	MemoryFile *image;

	ht_format_viewer *v_header;
	ht_view *v_imports;
	ht_format_viewer *v_image;	
};

/*
 *	ht_xex
 */
class ht_xex: public ht_format_group {
protected:
	bool loc_enum;
public:
		void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs);
	virtual	void done();
};

bool xex_rva_to_ofs(ht_xex_shared_data *xex_shared, RVA rva, FileOfs &ofs);
bool xex_ofs_to_rva(ht_xex_shared_data *xex_shared, FileOfs ofs, RVA &rva);
uint32 xex_get_rva_flags(ht_xex_shared_data *xex_shared, RVA rva);

#endif /* !__HTXEX_H__ */
