/* 
 *	HT Editor
 *	htle.h
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

#ifndef __HTLE_H__
#define __HTLE_H__

#include "htobj.h"
#include "formats.h"
#include "relfile.h"

#include "lestruct.h"
#include "endianess.h"

#define DESC_LE "le - win,os2 linear exe"
#define DESC_LE_HEADER "le/header"
#define DESC_LE_VXD "le/vxd descriptor"
#define DESC_LE_OBJECTS "le/objects"
#define DESC_LE_PAGEMAP "le/page map"
#define DESC_LE_ENTRYPOINTS "le/entrypoints"
#define DESC_LE_IMAGE "le/image"

#define ATOM_LE_FLAGS			0x4c450000
#define ATOM_LE_FLAGS_STR		 "4c450000"

#define ATOM_LE_MACHINE			0x4c450001
#define ATOM_LE_MACHINE_STR		 "4c450001"

#define ATOM_LE_OS				0x4c450002
#define ATOM_LE_OS_STR			 "4c450002"

#define ATOM_LE_OBJFLAGS   		0x4c450003
#define ATOM_LE_OBJFLAGS_STR		 "4c450003"

#define ATOM_LE_ENTRY_FLAGS		0x4c450004
#define ATOM_LE_ENTRY_FLAGS_STR	 "4c450004"

#define ATOM_LE_ENTRY_BUNDLE_FLAGS		0x4c450005
#define ATOM_LE_ENTRY_BUNDLE_FLAGS_STR	 "4c450005"

#define LEAddress uint32

#define LE_ADDR_INVALID	0
#define LE_BASE_ADDR	0x400000

class ht_le_page_file;

struct ht_le_shared_data {
	Endianess byteorder;
	FileOfs hdr_ofs;
	LE_HEADER hdr;
	ht_viewer *v_header;
	ht_viewer *v_objects;
	ht_viewer *v_pagemaps;
	ht_viewer *v_image;
	ht_viewer *v_le_vxd;
	ht_le_objmap objmap;
	ht_le_pagemap pagemap;
	bool is_vxd;
	FileOfs vxd_desc_linear_ofs;
	LE_VXD_DESCRIPTOR vxd_desc;
	ht_le_page_file *linear_file;
	ht_reloc_file *reloc_file;
	LEAddress best_entrypoint;
};

class ht_le: public ht_format_group {
protected:
	bool loc_enum;

			void check_vxd();
			void do_fixups();
			void read_pagemap();
			void read_objects();
public:
			void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs h);
	virtual	void done();
/* overwritten */
	virtual   void loc_enum_start();
	virtual   bool loc_enum_next(ht_format_loc *loc);
};

extern format_viewer_if htle_if;

/*
 *	CLASS ht_le_page_file
 */

class ht_le_page_file: public FileLayer {
protected:
	ht_le_pagemap *pagemap;
	uint32 pagemapsize;
	uint32 page_size;
	FileOfs ofs;
public:
			ht_le_page_file(File *file, bool own_file, ht_le_pagemap *pagemap, uint32 pagemapsize, uint32 page_size);
/* overwritten */
	virtual bool isdirty(FileOfs offset, FileOfs range);
	virtual uint read(void *buf, uint size);
	virtual void seek(FileOfs offset);
	virtual FileOfs tell() const;
	virtual int vcntl(uint cmd, va_list vargs);
	virtual uint write(const void *buf, uint size);
/* new */
	   bool map_ofs(uint32 lofs, FileOfs *pofs, FileOfs *maxsize);
	   bool unmap_ofs(FileOfs pofs, uint32 *lofs);
};

/*
 *	CLASS ht_le_reloc_entry
 */

class ht_le_reloc_entry: public Object {
public:
	uint ofs;	// FIXME: hack
	uint seg;
	LEAddress addr;
	uint8 address_type;
	uint8 reloc_type;

	ht_le_reloc_entry(uint ofs, uint seg, LEAddress addr, uint8 address_type, uint8 reloc_type);
};

/*
 *	CLASS ht_le_reloc_file
 */

class ht_le_reloc_file: public ht_reloc_file {
protected:
	ht_le_shared_data *data;
/* overwritten */
	virtual void	reloc_apply(Object *reloc, byte *data);
	virtual bool	reloc_unapply(Object *reloc, byte *data);
public:
			ht_le_reloc_file(File *streamfile, bool own_streamfile, ht_le_shared_data *data);
};

FileOfs LE_get_seg_ofs(ht_le_shared_data *LE_shared, uint i);
LEAddress LE_get_seg_addr(ht_le_shared_data *LE_shared, uint i);
uint LE_get_seg_psize(ht_le_shared_data *LE_shared, uint i);
uint LE_get_seg_vsize(ht_le_shared_data *LE_shared, uint i);

bool LE_addr_to_segment(ht_le_shared_data *LE_shared, LEAddress Addr, int *segment);
bool LE_addr_is_physical(ht_le_shared_data *LE_shared, LEAddress Addr);
bool LE_addr_to_ofs(ht_le_shared_data *LE_shared, LEAddress Addr, FileOfs *ofs);

bool LE_ofs_to_addr(ht_le_shared_data *LE_shared, FileOfs ofs, LEAddress *Addr);

LEAddress LE_MAKE_ADDR(ht_le_shared_data *LE_shared, uint16 seg, uint32 ofs);
uint16 LE_ADDR_SEG(ht_le_shared_data *LE_shared, LEAddress a);
uint32 LE_ADDR_OFS(ht_le_shared_data *LE_shared, LEAddress a);

#endif /* __HTLE_H__ */

