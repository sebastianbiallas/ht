/* 
 *	HT Editor
 *	htne.h
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

#ifndef __HTNE_H__
#define __HTNE_H__

#include "htanaly.h"
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

struct ht_ne_shared_data {
	dword hdr_ofs;
	NE_HEADER hdr;
	ne_segment_headers segments;
	UINT fake_segment;
	ht_list *entrypoints;
	ht_format_viewer *v_image;
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
 *	CLASS ht_ne_aviewer
 */

class ht_ne_aviewer: public ht_aviewer {
public:
	ht_ne_shared_data *ne_shared;
	ht_streamfile *file;
		   void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, analyser *Analyser, ht_ne_shared_data *ne_shared);
	virtual void set_analyser(analyser *a);
};

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
	virtual void	reloc_unapply(ht_data *reloc, byte *data);
public:
		   void	init(ht_streamfile *streamfile, bool own_streamfile, ht_ne_shared_data *data);
};

/*
 *
 */

FILEOFS NE_get_seg_ofs(ht_ne_shared_data *NE_shared, UINT i);
ADDR NE_get_seg_addr(ht_ne_shared_data *NE_shared, UINT i);
UINT NE_get_seg_psize(ht_ne_shared_data *NE_shared, UINT i);
UINT NE_get_seg_vsize(ht_ne_shared_data *NE_shared, UINT i);

bool NE_addr_to_segment(ht_ne_shared_data *NE_shared, ADDR Addr, int *segment);
bool NE_addr_is_physical(ht_ne_shared_data *NE_shared, ADDR Addr);
bool NE_addr_to_ofs(ht_ne_shared_data *NE_shared, ADDR Addr, FILEOFS *ofs);

bool NE_ofs_to_addr(ht_ne_shared_data *NE_shared, FILEOFS ofs, ADDR *Addr);

#define NE_MAKE_ADDR(seg, ofs) ((seg)*0x10000+(ofs))

#endif /* __HTNE_H__ */

