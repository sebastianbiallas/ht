/* 
 *	HT Editor
 *	htle.h
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

#ifndef __HTLE_H__
#define __HTLE_H__

#include "htobj.h"
#include "formats.h"

#include "lestruct.h"

#define DESC_LE "le - win,os2 linear exe"
#define DESC_LE_HEADER "le/header"
#define DESC_LE_OBJECTS "le/objects"
#define DESC_LE_PAGEMAP "le/page map"
#define DESC_LE_ENTRYPOINTS "le/entrypoints"
#define DESC_LE_IMAGE "le/image"

#define ATOM_LE_MACHINE			0x4c450000
#define ATOM_LE_MACHINE_STR		 "4c450000"

#define ATOM_LE_OS				0x4c450001
#define ATOM_LE_OS_STR			 "4c450001"

#define ATOM_LE_OBJFLAGS   		0x4c450002
#define ATOM_LE_OBJFLAGS_STR		 "4c450002"

#define ATOM_LE_ENTRY_FLAGS		0x4c450003
#define ATOM_LE_ENTRY_FLAGS_STR	 "4c450003"

#define ATOM_LE_ENTRY_BUNDLE_FLAGS		0x4c450004
#define ATOM_LE_ENTRY_BUNDLE_FLAGS_STR	 "4c450004"

struct ht_le_shared_data {
	dword hdr_ofs;
	IMAGE_LE_HEADER hdr;
	ht_viewer *v_header;
	ht_viewer *v_objects;
	ht_viewer *v_pagemaps;
	ht_viewer *v_image;
	ht_le_objmap objmap;
	ht_le_pagemap pagemap;
};

class ht_le: public ht_format_group {
protected:
	bool loc_enum;
public:
			void init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS h);
	virtual	void done();
/* overwritten */
	virtual   void loc_enum_start();
	virtual   bool loc_enum_next(ht_format_loc *loc);
};

extern format_viewer_if htle_if;

#endif /* __HTLE_H__ */
