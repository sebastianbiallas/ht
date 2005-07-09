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
	UINT segment_count;
	NE_SEGMENT *segments;
};

class ne_import_rec: public ht_data {
public:
	UINT addr;
	UINT module;
	bool byname;
	union {
		UINT name_ofs;
		UINT ord;
	};

	ne_import_rec(UINT a, UINT mod, bool b, UINT i)
	{
		addr = a;
		module = mod;
		byname = b;
		ord = i;
	}
};

class ht_aviewer;
struct ht_ne_shared_data {
	dword hdr_ofs;
	NE_HEADER hdr;
	ne_segment_headers segments;
	UINT modnames_count;
	char **modnames;
	UINT fake_segment;
	ht_list *entrypoints;
	ht_tree *imports;
	ht_aviewer *v_image;
};

class ht_ne: public ht_format_group {
protected:
	bool loc_enum;

			bool create_fake_segment();
			bool relocate(ht_reloc_file *rf);
			bool relocate_single(ht_reloc_file *rf, UINT seg, FILEOFS ofs, UINT type, UINT flags, word value_seg, word value_ofs);
public:
			void init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS h);
	virtual	void done();
/* overwritten */
	virtual   void loc_enum_start();
	virtual   bool loc_enum_next(ht_format_loc *loc);
};

extern format_viewer_if htne_if;

/*
 *	CLASS ht_ne_entrypoint
 */

class ht_ne_entrypoint: public ht_data {
public:
	UINT ordinal;
	UINT seg;
	UINT offset;
	UINT flags;
	char *name;

		ht_ne_entrypoint(UINT ordinal, UINT seg, UINT offset, UINT flags);
	virtual	~ht_ne_entrypoint();
};

/*
 *	CLASS ht_ne_reloc_entry
 */

class ht_ne_reloc_entry: public ht_data {
public:
	UINT mode;
	bool add;

	word seg;
	word ofs;

	ht_ne_reloc_entry(UINT mode, bool add, word seg, word ofs);
};

/*
 *	CLASS ht_ne_reloc_file
 */

class ht_ne_reloc_file: public ht_reloc_file {
protected:
	ht_ne_shared_data *data;
/* overwritten */
	virtual void	reloc_apply(ht_data *reloc, byte *data);
	virtual bool	reloc_unapply(ht_data *reloc, byte *data);
public:
		   void	init(ht_streamfile *streamfile, bool own_streamfile, ht_ne_shared_data *data);
};

/*
 *
 */

FILEOFS NE_get_seg_ofs(ht_ne_shared_data *NE_shared, UINT i);
dword NE_get_seg_addr(ht_ne_shared_data *NE_shared, UINT i);
UINT NE_get_seg_psize(ht_ne_shared_data *NE_shared, UINT i);
UINT NE_get_seg_vsize(ht_ne_shared_data *NE_shared, UINT i);

bool NE_addr_to_segment(ht_ne_shared_data *NE_shared, dword Addr, int *segment);
bool NE_addr_is_physical(ht_ne_shared_data *NE_shared, dword Addr);
bool NE_addr_to_ofs(ht_ne_shared_data *NE_shared, dword Addr, FILEOFS *ofs);

bool NE_ofs_to_addr(ht_ne_shared_data *NE_shared, FILEOFS ofs, dword *Addr);

#define NEAddress dword
#define NE_MAKE_ADDR(seg, ofs) ((seg)*0x10000+(ofs))
#define NE_ADDR_SEG(a) ((a)>>16)
#define NE_ADDR_OFS(a) ((a)&0xffff)

#endif /* __HTNE_H__ */

