/* 
 *	HT Editor
 *	htle.cc
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

#include "errno.h"
#include <stdlib.h>

#include "endianess.h"
#include "htle.h"
#include "htlehead.h"
#include "htleimg.h"
#include "htleobj.h"
#include "htleent.h"
#include "htlepage.h"
#include "htlevxd.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "lestruct.h"
#include "log.h"
#include "tools.h"

#define LE_SEG_ADDR(i) (LE_shared->objmap.header[(i)].page_map_index-1) *\
			LE_shared->hdr.pagesize + LE_BASE_ADDR

static format_viewer_if *htle_ifs[] = {
	&htleheader_if,
	&htlevxd_if,
	&htlepagemaps_if,
	&htleobjects_if,
	&htleentrypoints_if,
	&htleimage_if,
	0
};

static ht_view *htle_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte lemagic[2];
	FileOfs h=get_newexe_header_ofs(file);
	file->seek(h);
	
	if (file->read(lemagic, 2) != 2 
	 || lemagic[0] != LE_MAGIC0 || lemagic[1] != LE_MAGIC1) return 0;

	ht_le *g=new ht_le();
	g->init(b, file, htle_ifs, format_group, h);
	return g;
}

format_viewer_if htle_if = {
	htle_init,
	0
};

void ht_le::init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs h)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_LE, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_le");

	String fn;
	LOG("%y: LE: found header at 0x%08qx", &file->getFilename(fn), h);
	ht_le_shared_data *le_shared = ht_malloc(sizeof (ht_le_shared_data));
	shared_data = le_shared;
	le_shared->v_header = NULL;
	le_shared->v_objects = NULL;
	le_shared->v_pagemaps = NULL;
	le_shared->v_image = NULL;
	le_shared->v_le_vxd = NULL;
	le_shared->hdr_ofs = h;
	le_shared->linear_file = NULL;
	le_shared->reloc_file = NULL;
	le_shared->best_entrypoint = LE_ADDR_INVALID;

	// FIXME: byteorder handling...
	le_shared->byteorder = little_endian;

	/* read LE header */
	file->seek(h);
	file->readx(&le_shared->hdr, sizeof le_shared->hdr);
	createHostStruct(&le_shared->hdr, LE_HEADER_struct, le_shared->byteorder);

	le_shared->is_vxd = le_shared->hdr.winresoff 
		|| le_shared->hdr.winreslen
		|| le_shared->hdr.devid
		|| le_shared->hdr.ddkver;

	read_pagemap();
	read_objects();

	ht_le_page_file *lfile = new ht_le_page_file(file, false, &le_shared->pagemap, 
		le_shared->pagemap.count, le_shared->hdr.pagesize);
	le_shared->linear_file = lfile;

	do_fixups();
	check_vxd();

	ht_format_group::init_ifs(ifs);
}

void ht_le::done()
{
	ht_format_group::done();

	ht_le_shared_data *le_shared=(ht_le_shared_data*)shared_data;
	
	free(le_shared->objmap.header);
	free(le_shared->objmap.vsize);
	free(le_shared->objmap.psize);
	
	free(le_shared->pagemap.offset);
	free(le_shared->pagemap.vsize);
	free(le_shared->pagemap.psize);

	delete le_shared->linear_file;
	delete le_shared->reloc_file;

	free(le_shared);
}

void ht_le::do_fixups()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FileOfs h = le_shared->hdr_ofs;

	uint32 *page_fixup_ofs = ht_malloc(sizeof (uint32) * (le_shared->hdr.pagecnt+1));
	uint32 *page_fixup_size = ht_malloc(sizeof (uint32) * (le_shared->hdr.pagecnt));

	file->seek(h+le_shared->hdr.fpagetab);
	for (uint i=0; i<le_shared->hdr.pagecnt+1; i++) {
		char buf[4];
		file->readx(buf, 4);
		uint32 ofs = createHostInt(buf, 4, little_endian);
		page_fixup_ofs[i] = ofs;
	}

	for (uint i=0; i<le_shared->hdr.pagecnt; i++) {
		page_fixup_size[i] = page_fixup_ofs[i+1] - page_fixup_ofs[i];
	}

	ht_le_reloc_file *rfile = new ht_le_reloc_file(le_shared->linear_file, 
					false, le_shared);

	le_shared->reloc_file = rfile;

	uint error_count = 0;

	for (uint i=0; i < le_shared->hdr.pagecnt; i++) {
		// size of fixup record table for segment
		uint32 size = page_fixup_size[i];
		uint obj_ofs = i * le_shared->hdr.pagesize;

		file->seek(h+le_shared->hdr.frectab+page_fixup_ofs[i]);
		bool error = false;
		while (size>0 && !error) {
			// addr_type + reloc_type
			LE_FIXUP f;
			if (sizeof f > size) { error = true; break; }
			size -= sizeof f;
			file->readx(&f, sizeof f);
			createHostStruct(&f, LE_FIXUP_struct, le_shared->byteorder);
			/* only internal references (16/32) supported for now... */
			if ((f.reloc_type != 0) && (f.reloc_type != 16)) {
				error = true;
				break;
			}

			// is address_type supported ?
			switch (f.address_type & LE_FIXUP_ADDR_TYPE_MASK) {
				case LE_FIXUP_ADDR_TYPE_0_8:
				case LE_FIXUP_ADDR_TYPE_16_0:
				case LE_FIXUP_ADDR_TYPE_16_16:
				case LE_FIXUP_ADDR_TYPE_0_16:
				case LE_FIXUP_ADDR_TYPE_16_32:
				case LE_FIXUP_ADDR_TYPE_0_32:
				case LE_FIXUP_ADDR_TYPE_REL32:
					break;
				default:
					error = true;
					break;
			}
			if (error) break;

			uint multi_count = 0;
			uint16 src_ofs;
			bool multi_ofs = (f.address_type & LE_FIXUP_ADDR_MULTIPLE);
			if (multi_ofs) {
				// number of entries in offset table
				char buf[1];
				if (sizeof buf > size) { error = true; break; }
				size -= sizeof buf;
				file->readx(buf, sizeof buf);
				multi_count = createHostInt(buf, 1, little_endian);
			} else {
				// single source offset
				char buf[2];
				if (sizeof buf > size) { error = true; break; }
				size -= sizeof buf;
				file->readx(buf, sizeof buf);
				src_ofs = createHostInt(buf, 2, little_endian);
			}

			switch (f.reloc_type & LE_FIXUP_RELOC_TYPE_MASK) {
				case LE_FIXUP_RELOC_TYPE_INTERNAL:
					uint16 target_seg;
					uint32 target_ofs;
					if (f.reloc_type & LE_FIXUP_RELOC_TARGET32) {
						LE_FIXUP_INTERNAL32 x;
						if (sizeof x > size) { error = true; break; }
						size -= sizeof x;
						file->readx(&x, sizeof x);
						createHostStruct(&x, LE_FIXUP_INTERNAL32_struct, le_shared->byteorder);
						target_seg = x.seg-1;
						target_ofs = x.ofs;
					} else {
						LE_FIXUP_INTERNAL16 x;
						if (sizeof x > size) { error = true; break; }
						size -= sizeof x;
						file->readx(&x, sizeof x);
						createHostStruct(&x, LE_FIXUP_INTERNAL16_struct, le_shared->byteorder);
						target_seg = x.seg-1;
						target_ofs = x.ofs;
					}

					if (multi_ofs) {
						for (uint j=0; j<multi_count; j++) {
							char buf[2];
							if (sizeof buf > size) { error = true; break; }
							size -= sizeof buf;
							file->readx(buf, sizeof buf);
							src_ofs = createHostInt(buf, sizeof buf, little_endian);
							rfile->insert_reloc(obj_ofs + src_ofs, new ht_le_reloc_entry(obj_ofs + src_ofs, target_seg, LE_MAKE_ADDR(le_shared, target_seg, target_ofs), f.address_type, f.reloc_type));
						}
					} else {
						rfile->insert_reloc(obj_ofs + src_ofs, new ht_le_reloc_entry(obj_ofs + src_ofs, target_seg, LE_MAKE_ADDR(le_shared, target_seg, target_ofs), f.address_type, f.reloc_type));
					}
					break;
				case LE_FIXUP_RELOC_TYPE_IMPORT_ORD:
					error = true;
					break;
				case LE_FIXUP_RELOC_TYPE_IMPORT_NAME:
					error = true;
					break;
				case LE_FIXUP_RELOC_TYPE_OSFIXUP:
					error = true;
					break;
			}
		}
		if (error) error_count++;
	}

	free(page_fixup_ofs);
	free(page_fixup_size);

	if (error_count) {
		String fn;
		// FIXME: once complete:
		// "%s: NE relocations seem to be corrupted.", file->get_filename());
		LOG_EX(LOG_WARN, "%y: LE: invalid and/or unsupported relocations found.", &file->getFilename(fn));
		errorbox("%y: LE: invalid and/or unsupported relocations found.", &fn);
	} else {
		String fn;
		LOG("%y: LE: relocations present, relocation simulation layer enabled", &file->getFilename(fn));
	}
	rfile->finalize();
}

void ht_le::check_vxd()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FileOfs h = le_shared->hdr_ofs;

	/* VxD */
	if (le_shared->is_vxd) {
		/* test if really VxD and find VxD descriptor */
		LE_ENTRYPOINT_BUNDLE b;
		file->seek(h+le_shared->hdr.enttab);
		file->readx(&b, sizeof b);
		le_shared->is_vxd = false;
		if ((b.entry_count == 1) && (b.flags & LE_ENTRYPOINT_BUNDLE_VALID) &&
		(b.flags & LE_ENTRYPOINT_BUNDLE_32BIT) && (b.obj_index == 1)) {
			LE_ENTRYPOINT32 e;
			file->readx(&e, sizeof e);
			createHostStruct(&e, LE_ENTRYPOINT32_struct, le_shared->byteorder);
			if (e.flags & LE_ENTRYPOINT_EXPORTED) {
				/* linearized address for ht_le_page_file */
				uint32 vxd_desc_ofs = (le_shared->objmap.header[0].
					page_map_index-1)*le_shared->hdr.pagesize + e.offset;

				le_shared->reloc_file->seek(vxd_desc_ofs);
				le_shared->reloc_file->readx(&le_shared->vxd_desc, sizeof le_shared->vxd_desc);
				createHostStruct(&le_shared->vxd_desc, LE_VXD_DESCRIPTOR_struct, le_shared->byteorder);

				le_shared->vxd_desc_linear_ofs = vxd_desc_ofs;
				le_shared->is_vxd = true;
			}
		}
	}
}

void ht_le::read_pagemap()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FileOfs h = le_shared->hdr_ofs;

	le_shared->pagemap.count=le_shared->hdr.pagecnt;
	le_shared->pagemap.offset = ht_malloc(le_shared->pagemap.count*sizeof *le_shared->pagemap.offset);
	le_shared->pagemap.psize = ht_malloc(le_shared->pagemap.count*sizeof *le_shared->pagemap.psize);
	le_shared->pagemap.vsize = ht_malloc(le_shared->pagemap.count*sizeof *le_shared->pagemap.vsize);

	uint32 last_page_offset=0, last_page=0;
	for (uint32 i=0; i<le_shared->hdr.pagecnt; i++) {
		LE_PAGE_MAP_ENTRY e;
		file->seek(h+le_shared->hdr.pagemap+i*4);
		file->readx(&e, sizeof e);
		createHostStruct(&e, LE_PAGE_MAP_ENTRY_struct, le_shared->byteorder);
		
		/* FIXME: is this formula correct ? it comes straight from my docs... */
		uint32 eofs=(e.high+e.low-1)*le_shared->hdr.pagesize+le_shared->hdr.datapage;
		le_shared->pagemap.offset[i] = eofs;

		if (le_shared->pagemap.offset[i]>last_page_offset) {
			last_page_offset = le_shared->pagemap.offset[i];
			last_page = i;
		}
	}

	for (uint32 i=0; i<le_shared->hdr.pagecnt; i++) {
		le_shared->pagemap.vsize[i]=0;	/* filled by read_objects() */
		if (i==last_page)
			le_shared->pagemap.psize[i]=le_shared->hdr.lastpagesize;
		else
			le_shared->pagemap.psize[i]=le_shared->hdr.pagesize;
	}
}

void ht_le::read_objects()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FileOfs h = le_shared->hdr_ofs;

	le_shared->objmap.count = le_shared->hdr.objcnt;
	le_shared->objmap.header = ht_malloc(le_shared->objmap.count*sizeof *le_shared->objmap.header);
	le_shared->objmap.vsize = ht_malloc(le_shared->objmap.count * sizeof *le_shared->objmap.vsize);
	le_shared->objmap.psize = ht_malloc(le_shared->objmap.count * sizeof *le_shared->objmap.psize);

	for (uint i=0; i<le_shared->hdr.objcnt; i++) {
		file->seek(h+le_shared->hdr.objtab+i*24);
		file->readx(&le_shared->objmap.header[i], sizeof *le_shared->objmap.header);
		createHostStruct(&le_shared->objmap.header[i], LE_OBJECT_HEADER_struct, le_shared->byteorder);

		/* sum up page sizes to find object's physical size */
		uint psize = 0;
		for (uint j=0; j<le_shared->objmap.header[i].page_map_count; j++) {
			psize += le_shared->pagemap.psize[j+le_shared->objmap.header[i].page_map_index-1];
			/* FIXME: security hole: array-index uncontrolled */
			if (j == le_shared->objmap.header[i].page_map_count-1)
				le_shared->pagemap.vsize[j+le_shared->objmap.header[i].page_map_index-1]=le_shared->objmap.header[i].vsize % le_shared->hdr.pagesize;
			else
				le_shared->pagemap.vsize[j+le_shared->objmap.header[i].page_map_index-1]=le_shared->hdr.pagesize;
		}
// FIXME: alternative which one is right ???
#if 1
		le_shared->objmap.psize[i] = MIN(psize, le_shared->objmap.header[i].vsize);
		le_shared->objmap.vsize[i] = MIN(psize, le_shared->objmap.header[i].vsize);
#else
		le_shared->objmap.psize[i] = le_shared->objmap.header[i].vsize;
		le_shared->objmap.vsize[i] = le_shared->objmap.header[i].vsize;
#endif
	}

/* create temporary address space for LEAddress's */
/*	le_shared->le_addr = ht_malloc(sizeof *le_shared->le_addr * le_shared->objmap.count);
	uint a = 0;
	for (uint i = 0; i<le_shared->objmap.count; i++) {
		le_shared->le_addr[i] = a;
		a += le_shared->objmap.header[i].page_map_count * le_shared->hdr.pagesize;
	}*/
}

void ht_le::loc_enum_start()
{
//	loc_enum=true;
}

bool ht_le::loc_enum_next(ht_format_loc *loc)
{
#if 0
	ht_le_shared_data *sh=(ht_le_shared_data*)shared_data;
	if (loc_enum) {
		loc->name="le";
		loc->start=sh->hdr_ofs;
		loc->length=file->get_size()-loc->start;	/* FIXME: ENOTOK */
		
		loc_enum=false;
		return true;
	}
#endif	
	return false;
}

/*
 *	CLASS ht_le_page_file
 */

ht_le_page_file::ht_le_page_file(File *file, bool own_file, ht_le_pagemap *pm, uint32 pms, uint32 ps)
	: FileLayer(file, own_file)
{
	pagemap = pm;
	pagemapsize = pms;
	page_size = ps;
	ofs = 0;
}

bool ht_le_page_file::isdirty(FileOfs offset, FileOfs range)
{
	FileOfs mofs;
	FileOfs msize;
	while (range) {
		FileOfs s = range;
		if (!map_ofs(offset, &mofs, &msize)) break;
		if (s > msize) s = msize;
		bool isdirty;
		mFile->cntl(FCNTL_MODS_IS_DIRTY, mofs, s, &isdirty);
		if (isdirty) return true;
		range -= s;
		ofs += s;
	}
	return false;
}

/**
 *	Map a paged and linearized LE offset to its corresponding
 *	physical file offset
 */
bool ht_le_page_file::map_ofs(uint32 lofs, FileOfs *pofs, FileOfs *maxsize)
{
	uint i = lofs / page_size, j = lofs % page_size;
	if (i < pagemapsize) {
		if (j < pagemap->vsize[i]) {
			*pofs = pagemap->offset[i]+j;
			*maxsize = pagemap->vsize[i]-j;
			return true;
		}
	}
	return false;
}

bool ht_le_page_file::unmap_ofs(FileOfs pofs, uint32 *lofs)
{
	for (uint i=0; i<pagemapsize; i++) {
		if ((pofs >= pagemap->offset[i]) && (pofs < pagemap->offset[i]+pagemap->vsize[i])) {
			*lofs = pofs - pagemap->offset[i] + i*page_size;
			return true;
		}
	}
	return false;
}

uint ht_le_page_file::read(void *aBuf, uint size)
{
	FileOfs mofs;
	FileOfs msize;
	int c = 0;
	byte *buf = (byte *)aBuf;
	while (size) {
		uint s = size;
		if (!map_ofs(ofs, &mofs, &msize)) break;
		if (s > msize) s = msize;
		mFile->seek(mofs);
		s = mFile->read(buf, s);
		if (!s) break;
		buf += s;
		size -= s;
		c += s;
		ofs += s;
	}
	return c;
}

void ht_le_page_file::seek(FileOfs offset)
{
	ofs = offset;
}

FileOfs ht_le_page_file::tell() const
{
	return ofs;
}

int ht_le_page_file::vcntl(uint cmd, va_list vargs)
{
	switch (cmd) {
		case FCNTL_MODS_CLEAR_DIRTY_RANGE: {
			FileOfs o = va_arg(vargs, FileOfs);
			FileOfs s = va_arg(vargs, FileOfs);
			FileOfs ts, ms;
			int e;

			do {
				if (!map_ofs(o, &o, &ms)) return EINVAL;
				ts = (s < ms) ? s : ms;
				e = FileLayer::cntl(cmd, o, ts);
				if (e) return e;
				s -= ts;
			} while (s);
			return 0;
		}
		case FCNTL_MODS_IS_DIRTY: {
			FileOfs o = va_arg(vargs, FileOfs);
			FileOfs s = va_arg(vargs, FileOfs);
			bool *b = va_arg(vargs, bool*);
			FileOfs ts, ms;
			int e;

			*b = false;
			do {
				if (!map_ofs(o, &o, &ms)) return EINVAL;
				ts = (s < ms) ? s : ms;
				e = mFile->cntl(cmd, o, ts, b);
				if (e) return e;
				if (*b) break;
				s -= ts;
			} while (s);
			return 0;
		}
	}
	return FileLayer::vcntl(cmd, vargs);
}

uint ht_le_page_file::write(const void *aBuf, uint size)
{
	FileOfs mofs;
	FileOfs msize;
	int c = 0;
	const byte *buf = (const byte *)aBuf;
	while (size) {
		uint s = size;
		if (!map_ofs(ofs, &mofs, &msize)) break;
		if (s>msize) s = msize;
		mFile->seek(mofs);
		buf += mFile->write(buf, s);
		size -= s;
		c += s;
		ofs += s;
	}
	return c;
}

/*
 *	CLASS ht_le_reloc_entry
 */

ht_le_reloc_entry::ht_le_reloc_entry(uint o, uint s, LEAddress a, uint8 at, uint8 rt)
{
	ofs = o;
	seg = s;
	addr = a;
	address_type = at;
	reloc_type = rt;
}

/*
 *	CLASS ht_le_reloc_file
 */

ht_le_reloc_file::ht_le_reloc_file(File *s, bool os, ht_le_shared_data *d)
	: ht_reloc_file(s, os)
{
	data = d;
}

void ht_le_reloc_file::reloc_apply(Object *reloc, byte *data)
{
	ht_le_reloc_entry *e = (ht_le_reloc_entry*)reloc;

	switch (e->address_type & LE_FIXUP_ADDR_TYPE_MASK) {
		case LE_FIXUP_ADDR_TYPE_0_8:
			createForeignInt(data, e->addr, 1, little_endian);
			break;
		case LE_FIXUP_ADDR_TYPE_16_0:
			createForeignInt(data, e->seg, 2, little_endian);
			break;
		case LE_FIXUP_ADDR_TYPE_16_16:
			createForeignInt(data, e->addr, 2, little_endian);
			createForeignInt(data, e->seg, 2, little_endian);
			break;
		case LE_FIXUP_ADDR_TYPE_0_16:
			createForeignInt(data, e->addr, 2, little_endian);
			break;
		case LE_FIXUP_ADDR_TYPE_16_32:
			createForeignInt(data, e->addr, 4, little_endian);
			createForeignInt(data, e->seg, 2, little_endian);
			break;
		case LE_FIXUP_ADDR_TYPE_0_32:
			createForeignInt(data, e->addr, 4, little_endian);
			break;
		case LE_FIXUP_ADDR_TYPE_REL32:
			createForeignInt(data, e->addr - LE_BASE_ADDR - e->ofs - 4, 4, little_endian);
			break;
	}
}

bool ht_le_reloc_file::reloc_unapply(Object *reloc, byte *data)
{
	return false;
}

/*
 *
 */

FileOfs LE_get_seg_ofs(ht_le_shared_data *LE_shared, uint i)
{
	assert(i<LE_shared->objmap.count);
	return LE_SEG_ADDR(i) - LE_BASE_ADDR;
}

LEAddress LE_get_seg_addr(ht_le_shared_data *LE_shared, uint i)
{
	assert(i<LE_shared->objmap.count);
	return LE_SEG_ADDR(i);
}

uint LE_get_seg_psize(ht_le_shared_data *LE_shared, uint i)
{
	assert(i<LE_shared->objmap.count);
	return LE_shared->objmap.psize[i];
}

uint LE_get_seg_vsize(ht_le_shared_data *LE_shared, uint i)
{
	assert(i<LE_shared->objmap.count);
	return LE_shared->objmap.vsize[i];
}

bool LE_addr_to_segment(ht_le_shared_data *LE_shared, LEAddress Addr, int *segment)
{
	for (uint i = 0; i < LE_shared->objmap.count; i++) {
		LEAddress base = LE_get_seg_addr(LE_shared, i);
		uint evsize = MAX(LE_get_seg_vsize(LE_shared, i), LE_get_seg_psize(LE_shared, i));
		if ((Addr >= base) && (Addr < base + evsize)) {
			*segment = i;
			return true;
		}
	}
	return false;
}

bool LE_addr_is_physical(ht_le_shared_data *LE_shared, LEAddress Addr)
{
	for (uint i = 0; i < LE_shared->objmap.count; i++) {
		LEAddress base = LE_get_seg_addr(LE_shared, i);
		uint psize = LE_get_seg_psize(LE_shared, i);
		if ((Addr >= base) && (Addr < base + psize)) return true;
	}
	return false;
}

bool LE_addr_to_ofs(ht_le_shared_data *LE_shared, LEAddress Addr, FileOfs *ofs)
{
	for (uint i = 0; i < LE_shared->objmap.count; i++) {
		LEAddress base = LE_get_seg_addr(LE_shared, i);
		uint psize = LE_get_seg_psize(LE_shared, i);
		if ((Addr >= base) && (Addr < base + psize)) {
			*ofs = Addr - LE_BASE_ADDR;
			return true;
		}
	}
	return false;
}

bool LE_ofs_to_addr(ht_le_shared_data *LE_shared, FileOfs ofs, LEAddress *Addr)
{
	for (uint i = 0; i < LE_shared->objmap.count; i++) {
		FileOfs sofs = LE_get_seg_ofs(LE_shared, i);
		if ((ofs >= sofs) && (ofs < sofs + LE_get_seg_psize(LE_shared, i))) {
			*Addr = LE_BASE_ADDR + ofs;
			return true;
		}
	}
	return false;
}

LEAddress LE_MAKE_ADDR(ht_le_shared_data *LE_shared, uint16 seg, uint32 ofs)
{
	return LE_SEG_ADDR(seg) + ofs;
}

uint16 LE_ADDR_SEG(ht_le_shared_data *LE_shared, LEAddress a)
{
	for (uint i = 0; i<LE_shared->objmap.count; i++) {
		LEAddress addr = LE_SEG_ADDR(i);
		if ((a >= addr) && (a < addr + LE_shared->objmap.vsize[i])) {
			return i;
		}
	}
	return 0xffff;
}

uint32 LE_ADDR_OFS(ht_le_shared_data *LE_shared, LEAddress a)
{
	for (uint i = 0; i<LE_shared->objmap.count; i++) {
		LEAddress addr = LE_SEG_ADDR(i);
		if ((a >= addr) && (a < addr + LE_shared->objmap.vsize[i])) {
			return a-addr;
		}
	}
	return 0;
}

