/* 
 *	HT Editor
 *	htle.cc
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

#include <stdlib.h>

#include "htendian.h"
#include "htle.h"
#include "htlehead.h"
#include "htleimg.h"
#include "htleobj.h"
#include "htleent.h"
#include "htlepage.h"
#include "htlevxd.h"
#include "htnewexe.h"
#include "lestruct.h"
#include "log.h"

format_viewer_if *htle_ifs[] = {
	&htleheader_if,
	&htlevxd_if,
	&htlepagemaps_if,
	&htleobjects_if,
	&htleentrypoints_if,
	&htleimage_if,
	0
};

ht_view *htle_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	byte lemagic[2];
	FILEOFS h=get_newexe_header_ofs(file);
	file->seek(h);
	file->read(lemagic, 2);
	if ((lemagic[0]!=LE_MAGIC0) || (lemagic[1]!=LE_MAGIC1)) return 0;

	ht_le *g=new ht_le();
	g->init(b, file, htle_ifs, format_group, h);
	return g;
}

format_viewer_if htle_if = {
	htle_init,
	0
};

void ht_le::init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS h)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_LE, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_le");

	LOG("%s: LE: found header at %08x", file->get_filename(), h);
	ht_le_shared_data *le_shared = (ht_le_shared_data*)malloc(sizeof(ht_le_shared_data));
	shared_data = le_shared;
	le_shared->v_header = NULL;
	le_shared->v_objects = NULL;
	le_shared->v_pagemaps = NULL;
	le_shared->v_image = NULL;
	le_shared->v_le_vxd = NULL;
	le_shared->hdr_ofs = h;
	le_shared->linear_file = NULL;
	le_shared->reloc_file = NULL;

     // FIXME: byteorder handling...
	le_shared->byteorder = little_endian;

	/* read LE header */
	file->seek(h);
	file->read(&le_shared->hdr, sizeof le_shared->hdr);
	create_host_struct(&le_shared->hdr, LE_HEADER_struct, le_shared->byteorder);

	le_shared->is_vxd = (le_shared->hdr.winresoff) ||
		(le_shared->hdr.winreslen) ||
		(le_shared->hdr.devid) ||
		(le_shared->hdr.ddkver);

	read_pagemap();
	read_objects();

	ht_le_page_file *lfile = new ht_le_page_file();
	lfile->init(file, false, &le_shared->pagemap, le_shared->pagemap.count,
		le_shared->hdr.pagesize);
	le_shared->linear_file = lfile;

	do_fixups();
	check_vxd();

	ht_format_group::init_ifs(ifs);
}

void ht_le::done()
{
	ht_format_group::done();

	ht_le_shared_data *le_shared=(ht_le_shared_data*)shared_data;
	
	if (le_shared->objmap.header) free(le_shared->objmap.header);
	if (le_shared->objmap.vsize) free(le_shared->objmap.vsize);
	if (le_shared->objmap.psize) free(le_shared->objmap.psize);
	
	if (le_shared->pagemap.offset) free(le_shared->pagemap.offset);
	if (le_shared->pagemap.vsize) free(le_shared->pagemap.vsize);
	if (le_shared->pagemap.psize) free(le_shared->pagemap.psize);

	if (le_shared->linear_file) {
		le_shared->linear_file->done();
		delete le_shared->linear_file;
	}

	if (le_shared->reloc_file) {
		le_shared->reloc_file->done();
		delete le_shared->reloc_file;
	}

	free(le_shared);
}

void ht_le::do_fixups()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FILEOFS h = le_shared->hdr_ofs;

	uint32 *page_fixup_ofs = (uint32*)malloc(sizeof (uint32) * (le_shared->hdr.pagecnt+1));
	uint32 *page_fixup_size = (uint32*)malloc(sizeof (uint32) * (le_shared->hdr.pagecnt));

	file->seek(h+le_shared->hdr.fpagetab);
	for (UINT i=0; i<le_shared->hdr.pagecnt+1; i++) {
		char buf[4];
		file->read(buf, 4);
		uint32 ofs = create_host_int(buf, 4, little_endian);
		page_fixup_ofs[i] = ofs;
	}

	for (UINT i=0; i<le_shared->hdr.pagecnt; i++) {
		page_fixup_size[i] = page_fixup_ofs[i+1] - page_fixup_ofs[i];
	}

	ht_le_reloc_file *rfile = new ht_le_reloc_file();
	rfile->init(le_shared->linear_file, false, le_shared);
	
	le_shared->reloc_file = rfile;

	LE_FIXUP f;
	file->seek(h+le_shared->hdr.frectab+page_fixup_ofs[0]);
	file->read(&f, sizeof f);
	create_host_struct(&f, LE_FIXUP_struct, le_shared->byteorder);
	if ((f.address_type == 0x08) && (f.reloc_type == 0x0)) {
		/* if single source */
		char buf[2];
		file->read(buf, 2);
		uint16 pofs = create_host_int(buf, 2, little_endian);
		/**/
		LE_FIXUP_INTERNAL i;
		file->read(&i, sizeof i);
		/**/
		rfile->insert_reloc(pofs, new ht_le_reloc_entry(i.ofs, 8, 0));
	}
}

void ht_le::check_vxd()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FILEOFS h = le_shared->hdr_ofs;

	/* VxD */
	if (le_shared->is_vxd) {
		/* test if really VxD and find VxD descriptor */
		LE_ENTRYPOINT_BUNDLE b;
		file->seek(h+le_shared->hdr.enttab);
		file->read(&b, sizeof b);
		le_shared->is_vxd = false;
		if ((b.entry_count == 1) && (b.flags & LE_ENTRYPOINT_BUNDLE_VALID) &&
		(b.flags & LE_ENTRYPOINT_BUNDLE_32BIT) && (b.obj_index == 1)) {
			LE_ENTRYPOINT32 e;
			file->read(&e, sizeof e);
			create_host_struct(&e, LE_ENTRYPOINT32_struct, le_shared->byteorder);
			if (e.flags & LE_ENTRYPOINT_EXPORTED) {
				/* linearized address for ht_le_page_file */
				uint32 vxd_desc_ofs = (le_shared->objmap.header[0].
					page_map_index-1)*le_shared->hdr.pagesize + e.offset;
				le_shared->linear_file->seek(vxd_desc_ofs);
				le_shared->linear_file->read(&le_shared->vxd_desc, sizeof le_shared->vxd_desc);
				create_host_struct(&le_shared->vxd_desc, LE_VXD_DESCRIPTOR_struct, le_shared->byteorder);
				le_shared->vxd_desc_linear_ofs = vxd_desc_ofs;
				le_shared->is_vxd = true;
			}
		}
	}
}

void ht_le::read_pagemap()
{
	ht_le_shared_data *le_shared = (ht_le_shared_data*)shared_data;
	FILEOFS h = le_shared->hdr_ofs;

	le_shared->pagemap.count=le_shared->hdr.pagecnt;
	le_shared->pagemap.offset=(dword*)malloc(le_shared->pagemap.count*sizeof *le_shared->pagemap.offset);
	le_shared->pagemap.psize=(dword*)malloc(le_shared->pagemap.count*sizeof *le_shared->pagemap.psize);
	le_shared->pagemap.vsize=(dword*)malloc(le_shared->pagemap.count*sizeof *le_shared->pagemap.vsize);

	dword last_page_offset=0, last_page=0;
	for (dword i=0; i<le_shared->hdr.pagecnt; i++) {
		LE_PAGE_MAP_ENTRY e;
		file->seek(h+le_shared->hdr.pagemap+i*4);
		file->read(&e, sizeof e);
		create_host_struct(&e, LE_PAGE_MAP_ENTRY_struct, le_shared->byteorder);
		
		/* FIXME: is this formula correct ? it comes straight from my docs... */
		dword eofs=(e.high+e.low-1)*le_shared->hdr.pagesize+le_shared->hdr.datapage;
		le_shared->pagemap.offset[i] = eofs;

		if (le_shared->pagemap.offset[i]>last_page_offset) {
			last_page_offset = le_shared->pagemap.offset[i];
			last_page = i;
		}
	}

	for (dword i=0; i<le_shared->hdr.pagecnt; i++) {
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
	FILEOFS h = le_shared->hdr_ofs;

	le_shared->objmap.count=le_shared->hdr.objcnt;
	le_shared->objmap.header=(LE_OBJECT_HEADER*)malloc(le_shared->objmap.count*sizeof *le_shared->objmap.header);
	le_shared->objmap.vsize=(dword*)malloc(le_shared->objmap.count*sizeof *le_shared->objmap.vsize);
	le_shared->objmap.psize=(dword*)malloc(le_shared->objmap.count*sizeof *le_shared->objmap.psize);

	for (dword i=0; i<le_shared->hdr.objcnt; i++) {
		file->seek(h+le_shared->hdr.objtab+i*24);
		file->read(&le_shared->objmap.header[i], sizeof *le_shared->objmap.header);
		create_host_struct(&le_shared->objmap.header[i], LE_OBJECT_HEADER_struct, le_shared->byteorder);
		
		/* sum up page sizes to find object's physical size */
		dword psize = 0;
		for (dword j=0; j<le_shared->objmap.header[i].page_map_count; j++) {
			psize += le_shared->pagemap.psize[j+le_shared->objmap.header[i].page_map_index-1];
			/* FIXME: security hole: array-index uncontrolled */
			if (j == le_shared->objmap.header[i].page_map_count-1)
				le_shared->pagemap.vsize[j+le_shared->objmap.header[i].page_map_index-1]=le_shared->objmap.header[i].virtual_size % le_shared->hdr.pagesize;
			else
				le_shared->pagemap.vsize[j+le_shared->objmap.header[i].page_map_index-1]=le_shared->hdr.pagesize;
		}
		le_shared->objmap.psize[i]=psize;

		le_shared->objmap.vsize[i]=le_shared->objmap.header[i].virtual_size;
	}
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

void ht_le_page_file::init(ht_streamfile *file, bool own_file, ht_le_pagemap *pm, dword pms, dword ps)
{
	ht_layer_streamfile::init(file, own_file);
	pagemap = pm;
	pagemapsize = pms;
	page_size = ps;
	ofs = 0;
}

bool ht_le_page_file::isdirty(FILEOFS offset, UINT range)
{
	FILEOFS mofs;
	UINT msize;
	while (range) {
		dword s=range;
		if (!map_ofs(offset, &mofs, &msize)) break;
		if (s>msize) s=msize;
		bool isdirty;
		streamfile->cntl(FCNTL_MODS_IS_DIRTY, mofs, s, &isdirty);
		if (isdirty) return 1;
		range-=s;
		ofs+=s;
	}
	return 0;
}

int ht_le_page_file::map_ofs(dword qofs, FILEOFS *offset, dword *maxsize)
{
	dword i=qofs/page_size, j=qofs % page_size;
	if (i<pagemapsize) {
		if (j<pagemap->vsize[i]) {
			*offset=pagemap->offset[i]+j;
			*maxsize=pagemap->vsize[i]-j;
			return 1;
		}
	}
	return 0;
}

UINT ht_le_page_file::read(void *buf, UINT size)
{
	FILEOFS mofs;
	UINT msize;
	int c=0;
	while (size) {
		dword s=size;
		if (!map_ofs(ofs, &mofs, &msize)) break;
		if (s>msize) s=msize;
		streamfile->seek(mofs);
		s=streamfile->read(buf, s);
		if (!s) break;
		((byte*)buf)+=s;
		size-=s;
		c+=s;
		ofs+=s;
	}
	return c;
}

int ht_le_page_file::seek(FILEOFS offset)
{
	ofs=offset;
	return 0;
}

FILEOFS ht_le_page_file::tell()
{
	return ofs;
}

UINT ht_le_page_file::write(const void *buf, UINT size)
{
	dword mofs, msize;
	int c=0;
	while (size) {
		dword s=size;
		if (!map_ofs(ofs, &mofs, &msize)) break;
		if (s>msize) s=msize;
		streamfile->seek(mofs);
		((byte*)buf)+=streamfile->write(buf, s);
		size-=s;
		c+=s;
		ofs+=s;
	}
	return c;
}

/*
 *	CLASS ht_le_reloc_entry
 */

ht_le_reloc_entry::ht_le_reloc_entry(uint32 t, uint8 at, uint8 rt)
{
	target_ofs = t;
	address_type = at;
	reloc_type = rt;
}

/*
 *	CLASS ht_le_reloc_file
 */

void ht_le_reloc_file::init(ht_streamfile *s, bool os, ht_le_shared_data *d)
{
	ht_reloc_file::init(s, os);
	data = d;
}

void ht_le_reloc_file::reloc_apply(ht_data *reloc, byte *data)
{
	ht_le_reloc_entry *e = (ht_le_reloc_entry*)reloc;

	create_foreign_int(data, e->target_ofs, 4, little_endian);
/*	switch (e->mode & NE_RT_MASK) {
		case NE_RT_OFS8:
			create_foreign_int(data, e->ofs, 1, little_endian);
			break;
		case NE_RT_SEG16:
			create_foreign_int(data, e->seg, 2, little_endian);
			break;
		case NE_RT_OFS16:
			create_foreign_int(data, e->ofs, 2, little_endian);
			break;
		case NE_RT_PTR32:
			create_foreign_int(data, e->ofs, 2, little_endian);
			create_foreign_int(data+2, e->seg, 2, little_endian);
			break;
		case NE_RT_OFS32:
			create_foreign_int(data, e->ofs, 4, little_endian);
			break;
		case NE_RT_PTR48:
			create_foreign_int(data, e->ofs, 4, little_endian);
			create_foreign_int(data+4, e->seg, 2, little_endian);
			break;
	}*/
}

bool ht_le_reloc_file::reloc_unapply(ht_data *reloc, byte *data)
{
	return false;
}


