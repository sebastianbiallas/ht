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
#include "tools.h"

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

	for (UINT i=0; i<le_shared->hdr.pagecnt; i++) {
     	// size of fixup record table for segment
     	uint32 size = page_fixup_size[i];
          UINT obj_ofs = i * le_shared->hdr.pagesize;

		file->seek(h+le_shared->hdr.frectab+page_fixup_ofs[i]);
          while (size>0) {
          	bool error = false;
	          // addr_type + reloc_type
			LE_FIXUP f;
			if (sizeof f > size) break;
			size -= sizeof f;
			file->read(&f, sizeof f);
			create_host_struct(&f, LE_FIXUP_struct, le_shared->byteorder);
               assert(f.reloc_type == 0);
               assert( (f.address_type == 7) || (f.address_type == 8) || (f.address_type == 39) || (f.address_type == 40) );

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
	               	// no, unsupported...
                         error = true;
     	          	break;
          	}
               if (error) break;

               UINT multi_count = 0;
               uint16 src_ofs;
               bool multi_ofs = (f.address_type & LE_FIXUP_ADDR_MULTIPLE);
			if (multi_ofs) {
               	// number of entries in offset table
				char buf[1];
		          if (sizeof buf > size) break;
				size -= sizeof buf;
				file->read(buf, sizeof buf);
				multi_count = create_host_int(buf, 1, little_endian);
               } else {
               	// single source offset
				char buf[2];
		          if (sizeof buf > size) break;
				size -= sizeof buf;
				file->read(buf, sizeof buf);
				src_ofs = obj_ofs + create_host_int(buf, 2, little_endian);
			}

	          switch (f.reloc_type & LE_FIXUP_RELOC_TYPE_MASK) {
				case LE_FIXUP_RELOC_TYPE_INTERNAL:
					LE_FIXUP_INTERNAL16 x;
		     	     if (sizeof x > size) { error = true; break; }
    			     	size -= sizeof x;
					file->read(&x, sizeof x);
					create_host_struct(&x, LE_FIXUP_INTERNAL16_struct, le_shared->byteorder);

                         if (multi_ofs) {
                         	for (int j=0; j<multi_count; j++) {
							char buf[2];
				     	     if (sizeof buf > size) { error = true; break; }
							size -= sizeof buf;
							file->read(buf, sizeof buf);
							src_ofs = obj_ofs + create_host_int(buf, 2, little_endian);
							rfile->insert_reloc(src_ofs, new ht_le_reloc_entry(LE_MAKE_ADDR(le_shared, i, src_ofs), LE_MAKE_ADDR(le_shared, x.seg-1, x.ofs), f.address_type, f.reloc_type));
                              }
                         } else {
						rfile->insert_reloc(src_ofs, new ht_le_reloc_entry(LE_MAKE_ADDR(le_shared, i, src_ofs), LE_MAKE_ADDR(le_shared, x.seg-1, x.ofs), f.address_type, f.reloc_type));
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
               if (error) break;
		}
     }
	rfile->finalize();
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

				le_shared->reloc_file->seek(vxd_desc_ofs);
				le_shared->reloc_file->read(&le_shared->vxd_desc, sizeof le_shared->vxd_desc);
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

	le_shared->objmap.count = le_shared->hdr.objcnt;
	le_shared->objmap.header = (LE_OBJECT*)malloc(le_shared->objmap.count*sizeof *le_shared->objmap.header);
	le_shared->objmap.vsize = (UINT*)malloc(le_shared->objmap.count * sizeof *le_shared->objmap.vsize);
	le_shared->objmap.psize = (UINT*)malloc(le_shared->objmap.count * sizeof *le_shared->objmap.psize);

	for (UINT i=0; i<le_shared->hdr.objcnt; i++) {
		file->seek(h+le_shared->hdr.objtab+i*24);
		file->read(&le_shared->objmap.header[i], sizeof *le_shared->objmap.header);
		create_host_struct(&le_shared->objmap.header[i], LE_OBJECT_HEADER_struct, le_shared->byteorder);
		
		/* sum up page sizes to find object's physical size */
		UINT psize = 0;
		for (UINT j=0; j<le_shared->objmap.header[i].page_map_count; j++) {
			psize += le_shared->pagemap.psize[j+le_shared->objmap.header[i].page_map_index-1];
			/* FIXME: security hole: array-index uncontrolled */
			if (j == le_shared->objmap.header[i].page_map_count-1)
				le_shared->pagemap.vsize[j+le_shared->objmap.header[i].page_map_index-1]=le_shared->objmap.header[i].vsize % le_shared->hdr.pagesize;
			else
				le_shared->pagemap.vsize[j+le_shared->objmap.header[i].page_map_index-1]=le_shared->hdr.pagesize;
		}
		le_shared->objmap.psize[i]=psize;

		le_shared->objmap.vsize[i]=le_shared->objmap.header[i].vsize;
	}

/* create temporary address space for LEAddress's */
	le_shared->le_addr = (LEAddress*)malloc(sizeof *le_shared->le_addr * le_shared->objmap.count);
	UINT a = 0;
	for (UINT i = 0; i<le_shared->objmap.count; i++) {
		le_shared->le_addr[i] = a;
		a += le_shared->objmap.header[i].page_map_count * le_shared->hdr.pagesize;
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

ht_le_reloc_entry::ht_le_reloc_entry(UINT o, LEAddress a, uint8 at, uint8 rt)
{
	ofs = o;
	addr = a;
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

	if (e->ofs == 134238960) {
     	int sdf = 24 ;
     }

	switch (e->address_type & LE_FIXUP_ADDR_TYPE_MASK) {
		case LE_FIXUP_ADDR_TYPE_0_8:
              	break;
		case LE_FIXUP_ADDR_TYPE_16_0:
              	break;
		case LE_FIXUP_ADDR_TYPE_16_16:
              	break;
		case LE_FIXUP_ADDR_TYPE_0_16:
              	break;
		case LE_FIXUP_ADDR_TYPE_16_32:
              	break;
		case LE_FIXUP_ADDR_TYPE_0_32:
			create_foreign_int(data, e->addr, 4, little_endian);
              	break;
		case LE_FIXUP_ADDR_TYPE_REL32:
			create_foreign_int(data, e->addr - e->ofs - 4, 4, little_endian);
              	break;
	}
}

bool ht_le_reloc_file::reloc_unapply(ht_data *reloc, byte *data)
{
	return false;
}

/*
 *
 */

FILEOFS LE_get_seg_ofs(ht_le_shared_data *LE_shared, UINT i)
{
     assert(i<LE_shared->objmap.count);
	return i * LE_shared->hdr.pagesize * LE_shared->objmap.header[i].page_map_index;
}

LEAddress LE_get_seg_addr(ht_le_shared_data *LE_shared, UINT i)
{
     assert(i<LE_shared->objmap.count);
	return LE_shared->le_addr[i]+LE_BASE_ADDR;
}

UINT LE_get_seg_psize(ht_le_shared_data *LE_shared, UINT i)
{
     assert(i<LE_shared->objmap.count);
	return LE_shared->objmap.psize[i];
}

UINT LE_get_seg_vsize(ht_le_shared_data *LE_shared, UINT i)
{
     assert(i<LE_shared->objmap.count);
	return LE_shared->objmap.vsize[i];
}

bool LE_addr_to_segment(ht_le_shared_data *LE_shared, LEAddress Addr, int *segment)
{
	for (UINT i = 0; i < LE_shared->objmap.count; i++) {
		LEAddress base = LE_get_seg_addr(LE_shared, i);
		UINT evsize = MAX(LE_get_seg_vsize(LE_shared, i), LE_get_seg_psize(LE_shared, i));
		if ((Addr >= base) && (Addr < base + evsize)) {
			*segment = i;
			return true;
		}
	}
	return false;
}

bool LE_addr_is_physical(ht_le_shared_data *LE_shared, LEAddress Addr)
{
	for (UINT i = 0; i < LE_shared->objmap.count; i++) {
		LEAddress base = LE_get_seg_addr(LE_shared, i);
		UINT psize = LE_get_seg_psize(LE_shared, i);
		if ((Addr >= base) && (Addr < base + psize)) return true;
	}
	return false;
}

bool LE_addr_to_ofs(ht_le_shared_data *LE_shared, LEAddress Addr, FILEOFS *ofs)
{
	for (UINT i = 0; i < LE_shared->objmap.count; i++) {
		LEAddress base = LE_get_seg_addr(LE_shared, i);
		UINT psize = LE_get_seg_psize(LE_shared, i);
		if ((Addr >= base) && (Addr < base + psize)) {
			*ofs = i * LE_shared->hdr.pagesize + (Addr - base);
			return true;
          }
	}
	return false;
}

bool LE_ofs_to_addr(ht_le_shared_data *LE_shared, FILEOFS ofs, LEAddress *Addr)
{
	for (UINT i = 0; i < LE_shared->objmap.count; i++) {
		FILEOFS sofs = LE_get_seg_ofs(LE_shared, i);
		if ((ofs >= sofs) && (ofs < sofs + LE_get_seg_psize(LE_shared, i))) {
			*Addr = LE_get_seg_addr(LE_shared, i) + (ofs - sofs);
			return true;
		}
	}
	return false;
}

LEAddress LE_MAKE_ADDR(ht_le_shared_data *LE_shared, uint16 seg, uint32 ofs)
{
	return LE_shared->le_addr[seg] + ofs + LE_BASE_ADDR;
}

uint16 LE_ADDR_SEG(ht_le_shared_data *LE_shared, LEAddress a)
{
	for (UINT i = 0; i<LE_shared->objmap.count; i++) {
     	LEAddress addr = LE_shared->le_addr[i] + LE_BASE_ADDR;
     	if ((a >= addr) && (a < addr + LE_shared->objmap.vsize[i])) {
          	return i;
          }
	}
     return 0xffff;
}

uint32 LE_ADDR_OFS(ht_le_shared_data *LE_shared, LEAddress a)
{
	for (UINT i = 0; i<LE_shared->objmap.count; i++) {
     	LEAddress addr = LE_shared->le_addr[i] + LE_BASE_ADDR;
     	if ((a >= addr) && (a < addr + LE_shared->objmap.vsize[i])) {
          	return a-addr;
          }
	}
     return 0;
}

