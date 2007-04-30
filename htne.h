/* 
 *	HT Editor
 *	htne.h
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

#ifndef __HTNE_H__
#define __HTNE_H__

#include "htobj.h"
#include "formats.h"
#include "relfile.h"

#include "nestruct.h"

#define DESC_NE "ne - win16,os2 new exe"
#define DESC_NE_HEADER "ne/header"
#define DESC_NE_SEGMENTS "ne/segments"
#define DESC_NE_IMPORTS "ne/imports"
#define DESC_NE_ENTRYPOINTS "ne/entrypoints"
#define DESC_NE_NAMES "ne/names"
#define DESC_NE_IMAGE "ne/image"

#define ATOM_NE_FLAGS			0x4e450000
#define ATOM_NE_FLAGS_STR		 "4e450000"

#define ATOM_NE_OS				0x4e450001
#define ATOM_NE_OS_STR			 "4e450001"

#define ATOM_NE_SEGFLAGS			0x4e450002
#define ATOM_NE_SEGFLAGS_STR		 "4e450002"

#define ATOM_NE_ENTFLAGS			0x4e450003
#define ATOM_NE_ENTFLAGS_STR		 "4e450003"

struct ne_segment_headers {
	uint segment_count;
	NE_SEGMENT *segments;
};

class ne_import_rec: public Object {
public:
	uint addr;
	uint module;
	bool byname;
	union {
		uint name_ofs;
		uint ord;
	};

			ne_import_rec(uint a, uint mod, bool b, uint i);

	virtual	int	compareTo(const Object *obj) const;
};

class ht_aviewer;
struct ht_ne_shared_data {
	uint32 hdr_ofs;
	NE_HEADER hdr;
	ne_segment_headers segments;
	uint modnames_count;
	char **modnames;
	uint fake_segment;
	Container *entrypoints;
	Container *imports;
	ht_aviewer *v_image;
};

class ht_ne: public ht_format_group {
protected:
	bool loc_enum;

		bool create_fake_segment();
		bool relocate(ht_reloc_file *rf);
		bool relocate_single(ht_reloc_file *rf, uint seg, FileOfs ofs, uint type, uint flags, uint16 value_seg, uint16 value_ofs);
public:
		void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs h);
	virtual	void done();
/* overwritten */
	virtual void loc_enum_start();
	virtual bool loc_enum_next(ht_format_loc *loc);
};

extern format_viewer_if htne_if;

/*
 *	CLASS ht_ne_entrypoint
 */

class ht_ne_entrypoint: public Object {
public:
	uint ordinal;
	uint seg;
	uint offset;
	uint flags;
	char *name;

		ht_ne_entrypoint(uint ordinal, uint seg, uint offset, uint flags);
	virtual	~ht_ne_entrypoint();
};

/*
 *	CLASS ht_ne_reloc_entry
 */

class ht_ne_reloc_entry: public Object {
public:
	uint mode;
	bool add;

	uint16 seg;
	uint16 ofs;

	ht_ne_reloc_entry(uint mode, bool add, uint16 seg, uint16 ofs);
};

/*
 *	CLASS ht_ne_reloc_file
 */

class ht_ne_reloc_file: public ht_reloc_file {
protected:
	ht_ne_shared_data *data;
/* overwritten */
	virtual void	reloc_apply(Object *reloc, byte *data);
	virtual bool	reloc_unapply(Object *reloc, byte *data);
public:
			ht_ne_reloc_file(File *streamfile, bool own_streamfile, ht_ne_shared_data *data);
};

/*
 *
 */

FileOfs NE_get_seg_ofs(ht_ne_shared_data *NE_shared, uint i);
uint32 NE_get_seg_addr(ht_ne_shared_data *NE_shared, uint i);
uint NE_get_seg_psize(ht_ne_shared_data *NE_shared, uint i);
uint NE_get_seg_vsize(ht_ne_shared_data *NE_shared, uint i);

bool NE_addr_to_segment(ht_ne_shared_data *NE_shared, uint32 Addr, int *segment);
bool NE_addr_is_physical(ht_ne_shared_data *NE_shared, uint32 Addr);
bool NE_addr_to_ofs(ht_ne_shared_data *NE_shared, uint32 Addr, FileOfs *ofs);

bool NE_ofs_to_addr(ht_ne_shared_data *NE_shared, FileOfs ofs, uint32 *Addr);

#define NEAddress uint32
#define NE_MAKE_ADDR(seg, ofs) ((seg)*0x10000+(ofs))
#define NE_ADDR_SEG(a) ((a)>>16)
#define NE_ADDR_OFS(a) ((a)&0xffff)

#endif /* __HTNE_H__ */

