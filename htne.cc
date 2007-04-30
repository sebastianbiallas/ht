/*
 *	HT Editor
 *	htne.cc
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

#include "log.h"
#include "endianess.h"
#include "htiobox.h"
#include "htne.h"
#include "htneent.h"
#include "htnehead.h"
#include "htneimp.h"
#include "htnenms.h"
#include "htneobj.h"
#include "htneimg.h"
#include "htnewexe.h"
#include "nestruct.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

static format_viewer_if *htne_ifs[] = {
	&htneheader_if,
	&htnesegments_if,
	&htneimports_if,
	&htneentrypoints_if,
	&htnenames_if,
	&htneimage_if,
	0
};

ne_import_rec::ne_import_rec(uint a, uint mod, bool b, uint i)
{
	addr = a;
	module = mod;
	byname = b;
	ord = i;
}

int ne_import_rec::compareTo(const Object *obj) const
{
	ne_import_rec *b=(ne_import_rec*)obj;
	if (module == b->module) {
		if (byname == b->byname) {
			if (byname) {
				return name_ofs - b->name_ofs;
			} else {
				return ord - b->ord;
			}
		}
		return byname - b->byname;
	}
	return module - b->module;
}

static ht_view *htne_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte nemagic[2];
	FileOfs h = get_newexe_header_ofs(file);
	file->seek(h);
	file->read(nemagic, 2);
	if ((nemagic[0]!=NE_MAGIC0) || (nemagic[1]!=NE_MAGIC1))
		return 0;

	ht_ne *g=new ht_ne();
	g->init(b, file, htne_ifs, format_group, h);
	return g;
}

format_viewer_if htne_if = {
	htne_init,
	0
};

void ht_ne::init(Bounds *b, File *f, format_viewer_if **ifs, ht_format_group *format_group, FileOfs h)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_NE, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_ne");

	String fn;
	LOG("%y: NE: found header at 0x%08qx", &file->getFilename(fn), h);

	ht_ne_shared_data *ne_shared = ht_malloc(sizeof (ht_ne_shared_data));
	shared_data = ne_shared;

	ne_shared->fake_segment = 0;
	ne_shared->hdr_ofs = h;
	ne_shared->entrypoints = NULL;
	ne_shared->imports = NULL;
	ne_shared->v_image = NULL;

	file->seek(h);
	file->readx(&ne_shared->hdr, sizeof ne_shared->hdr);
	createHostStruct(&ne_shared->hdr, NE_HEADER_struct, little_endian);

	/* read segment descriptors */
	ne_shared->segments.segment_count = ne_shared->hdr.cseg;
	ne_shared->segments.segments = ht_malloc(sizeof (NE_SEGMENT) * ne_shared->segments.segment_count);

	bool reloc_needed = false;
	NE_SEGMENT *s = ne_shared->segments.segments;
	file->seek(h+ne_shared->hdr.segtab);
	for (uint32 i = 0; i < ne_shared->segments.segment_count; i++) {
		file->readx(s, sizeof *s);
		createHostStruct(s, NE_SEGMENT_struct, little_endian);
		if (s->flags & NE_HASRELOC) reloc_needed = true;
		s++;
	}

	/* read entrypoint descriptors */
	FileOfs o = h + ne_shared->hdr.enttab;
	NE_ENTRYPOINT_HEADER e;
	file->seek(o);
	file->readx(&e, sizeof e);
	createHostStruct(&e, NE_ENTRYPOINT_HEADER_struct, little_endian);
	o += sizeof e;

	Array *ep = new Array(true);

	uint32 index = 1;
	while (o<h+ne_shared->hdr.enttab+ne_shared->hdr.cbenttab) {
		for (int i=0; i<e.entry_count; i++) {
			if (e.seg_index==0) {
			} else if (e.seg_index==0xff) {
				o+=sizeof (NE_ENTRYPOINT_MOVABLE);
				
				NE_ENTRYPOINT_MOVABLE me;
				file->readx(&me, sizeof me);
				createHostStruct(&me, NE_ENTRYPOINT_MOVABLE_struct, little_endian);

				ep->forceSetByIdx(index, new ht_ne_entrypoint(index, me.seg, me.offset, 0));
			} else {
				o+=sizeof (NE_ENTRYPOINT_FIXED);
				
				NE_ENTRYPOINT_FIXED fe;
				file->seek(o);
				file->readx(&fe, sizeof fe);
				createHostStruct(&fe, NE_ENTRYPOINT_FIXED_struct, little_endian);
				ep->forceSetByIdx(index, new ht_ne_entrypoint(index, e.seg_index, fe.offset, 0));
			}
			index++;
		}
		file->seek(o);
		file->readx(&e, sizeof e);
		createHostStruct(&e, NE_ENTRYPOINT_HEADER_struct, little_endian);
		o += sizeof e;
	}

	ne_shared->entrypoints = ep;

	/* read module names */
	o = h + ne_shared->hdr.modtab;
	FileOfs no = h + ne_shared->hdr.imptab;
	file->seek(o);
	ne_shared->modnames_count = ne_shared->hdr.cmod;
	ne_shared->modnames = ht_malloc(sizeof *ne_shared->modnames * ne_shared->modnames_count);
	for (uint i=0; i<ne_shared->hdr.cmod; i++) {
		char buf[2];
		file->seek(o+i*2);
		if (file->read(buf, 2) != 2) break;
		int w = createHostInt(buf, 2, little_endian);
		file->seek(no+w);
		ne_shared->modnames[i] = file->readstrp();
	}

	/* do relocations */
	if (reloc_needed) {
		ht_ne_reloc_file *rf = new ht_ne_reloc_file(file, false, (ht_ne_shared_data*)shared_data);

		if (relocate(rf)) {
			rf->finalize();
			file = rf;
			own_file = true;
			LOG("%y: NE: relocations present, relocation simulation layer enabled", &fn);
		} else {
			LOG_EX(LOG_WARN, "%y: NE relocations seem to be corrupted.", &fn);
			errorbox("%y: NE relocations seem to be corrupted.", &fn);
			delete rf;
		}
	}

	ht_format_group::init_ifs(ifs);
}

void ht_ne::done()
{
	ht_format_group::done();

	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;

	if (ne_shared->segments.segments) {
		free(ne_shared->segments.segments);
	}
	
	if (ne_shared->modnames) {
		for (uint i=0; i<ne_shared->modnames_count; i++) {
			free(ne_shared->modnames[i]);
		}
		free(ne_shared->modnames);
	}
	
	delete ne_shared->entrypoints;
	delete ne_shared->imports;

	free(shared_data);
}

void ht_ne::loc_enum_start()
{
//	loc_enum=true;
}

bool ht_ne::loc_enum_next(ht_format_loc *loc)
{
#if 0
	ht_ne_shared_data *sh=(ht_ne_shared_data*)shared_data;
	if (loc_enum) {
		loc->name="ne";
		loc->start=sh->hdr_ofs;
		loc->length=file->get_size()-loc->start;	/* FIXME: ENOTOK */
		
		loc_enum=false;
		return true;
	}
#endif
	return false;
}

bool ht_ne::create_fake_segment()
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	uint i = ne_shared->hdr.cseg;
	NE_SEGMENT *newsegs = ht_malloc(sizeof *newsegs * (ne_shared->segments.segment_count+1));
	memcpy(newsegs, ne_shared->segments.segments, sizeof *newsegs * ne_shared->segments.segment_count);
	ne_shared->segments.segment_count++;
	free(ne_shared->segments.segments);
	ne_shared->segments.segments = newsegs;
	newsegs[i].offset = 0;
	newsegs[i].size = 0;
	newsegs[i].flags = NE_DATA;
	newsegs[i].minalloc = 0;		// = 64k
	ne_shared->fake_segment = i;
	return true;
}

bool ht_ne::relocate(ht_reloc_file *rf)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	if (!create_fake_segment()) return false;

	AVLTree *imports = new AVLTree(true);

	int fake_entry_count = 0;
	/* if selfload only relocate first segment. Is this "The Right Thing" ? */
	/* Works for me though. */
	uint c = (ne_shared->hdr.flags & NE_FLAGS_SELFLOAD) ? 1 : ne_shared->segments.segment_count;
	for (uint32 i = 0; i < c; i++) {
		if (ne_shared->segments.segments[i].flags & NE_HASRELOC) {
			FileOfs seg_ofs = NE_get_seg_ofs(ne_shared, i);
			FileOfs f = seg_ofs + NE_get_seg_psize(ne_shared, i);
			char buf[2];
			file->seek(f);
			file->readx(buf, 2);
			f += 2;
			int c = createHostInt(buf, 2, little_endian);
			for (int j = 0; j < c; j++) {
				NE_RELOC_HEADER reloc;
				file->seek(f);
				file->readx(&reloc, sizeof reloc);
				createHostStruct(&reloc, NE_RELOC_HEADER_struct, little_endian);
				f += sizeof reloc;
				switch (reloc.flags & NE_RF_RT_MASK) {
				case NE_RF_INTERNAL: {
					NE_RELOC_INTERNAL sreloc;
					file->readx(&sreloc, sizeof sreloc);
					createHostStruct(&sreloc, NE_RELOC_INTERNAL_struct, little_endian);
					f += sizeof sreloc;
					if (sreloc.seg == 0xff) {
//		                              	ne_shared->entrypoint->;
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xf1, 0)) return false;
					} else {
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, sreloc.seg, sreloc.ofs)) return false;
					}
					break;
				}
				case NE_RF_IMPORT_ORD: {
					NE_RELOC_IMPORT sreloc;
					file->readx(&sreloc, sizeof sreloc);
					createHostStruct(&sreloc, NE_RELOC_IMPORT_struct, little_endian);
					f += sizeof sreloc;
					if (imports->insert(new ne_import_rec(fake_entry_count, sreloc.module, false, sreloc.ord))) {
//						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xf2, sreloc.ord)) return false;
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, ne_shared->fake_segment+1, fake_entry_count)) return false;
						fake_entry_count++;
					}
					break;
				}
				case NE_RF_IMPORT_NAME: {
					NE_RELOC_IMPORT sreloc;
					file->readx(&sreloc, sizeof sreloc);
					createHostStruct(&sreloc, NE_RELOC_IMPORT_struct, little_endian);
					f += sizeof sreloc;
					if (imports->insert(new ne_import_rec(fake_entry_count, sreloc.module, true, sreloc.name_ofs))) {
//						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xf3, sreloc.name_ofs)) return false;
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, ne_shared->fake_segment+1, fake_entry_count)) return false;
						fake_entry_count++;
					}
					break;
				}
				case NE_RF_OSFIXUP: {
					NE_RELOC_FIXUP sreloc;
					file->readx(&sreloc, sizeof sreloc);
					createHostStruct(&sreloc, NE_RELOC_FIXUP_struct, little_endian);
					f += sizeof sreloc;
					if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xdead, 0xcafebabe)) return false;
					break;
				}
				}
			}
		}
	}
	ne_shared->imports = imports;
	return true;
}

bool ht_ne::relocate_single(ht_reloc_file *rf, uint seg, FileOfs ofs, uint type, uint flags, uint16 value_seg, uint16 value_ofs)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	while (1) {
		if (flags & NE_RF_ADD) break;

		switch (type & NE_RT_MASK) {
		case NE_RT_SEG16:
		case NE_RT_PTR32:
		case NE_RT_OFS16:
		case NE_RT_PTR48:
		case NE_RT_OFS32:
			break;
		case NE_RT_OFS8:
			/* FIXME: we want to read a uint16 (offset) out of the file,
			   but we can't because there's only one relocated
			   byte. Maybe I dont understand NE relocs completely. */
		default:
			/* unknown relocation record */
			return false;
		}
		rf->insert_reloc(ofs, new ht_ne_reloc_entry(type, flags & NE_RF_ADD, value_seg, value_ofs));
		char buf[2];
		file->seek(ofs);
		file->readx(buf, 2);
		uint16 r = createHostInt(buf, 2, little_endian);
		if (r == 0xffff) break;
		NEAddress a = NE_MAKE_ADDR(seg+1, r);
		if (!NE_addr_to_ofs(ne_shared, a, &ofs)) {
			return false;
		}
	}
	return true;
}

/*
 *	CLASS ht_ne_entrypoint
 */

ht_ne_entrypoint::ht_ne_entrypoint(uint Ordinal, uint Seg, uint Offset, uint Flags)
{
	ordinal = Ordinal;
	seg = Seg;
	offset = Offset;
	flags = Flags;
	name = NULL;
}

ht_ne_entrypoint::~ht_ne_entrypoint()
{
	free(name);
}

/*
 *	CLASS ht_ne_reloc_entry
 */

ht_ne_reloc_entry::ht_ne_reloc_entry(uint Mode, bool Add, uint16 Seg, uint16 Ofs)
{
	mode = Mode;
	add = Add;
	seg = Seg;
	ofs = Ofs;
}

/*
 *	CLASS ht_ne_reloc_file
 */

ht_ne_reloc_file::ht_ne_reloc_file(File *s, bool os, ht_ne_shared_data *d)
	: ht_reloc_file(s, os)
	
{
	data = d;
}

void ht_ne_reloc_file::reloc_apply(Object *reloc, byte *data)
{
	ht_ne_reloc_entry *e = (ht_ne_reloc_entry*)reloc;

	switch (e->mode & NE_RT_MASK) {
		case NE_RT_OFS8:
			createForeignInt(data, e->ofs, 1, little_endian);
			break;
		case NE_RT_SEG16:
			createForeignInt(data, e->seg, 2, little_endian);
			break;
		case NE_RT_OFS16:
			createForeignInt(data, e->ofs, 2, little_endian);
			break;
		case NE_RT_PTR32:
			createForeignInt(data, e->ofs, 2, little_endian);
			createForeignInt(data+2, e->seg, 2, little_endian);
			break;
		case NE_RT_OFS32:
			createForeignInt(data, e->ofs, 4, little_endian);
			break;
		case NE_RT_PTR48:
			createForeignInt(data, e->ofs, 4, little_endian);
			createForeignInt(data+4, e->seg, 2, little_endian);
			break;
	}
}

bool ht_ne_reloc_file::reloc_unapply(Object *reloc, byte *data)
{
	return false;
//	ht_ne_reloc_entry *e = (ht_ne_reloc_entry*)reloc;
}

/*
 *
 */

FileOfs NE_get_seg_ofs(ht_ne_shared_data *NE_shared, uint i)
{
	return (NE_shared->segments.segments[i].offset << NE_shared->hdr.align);
}

NEAddress NE_get_seg_addr(ht_ne_shared_data *NE_shared, uint i)
{
	return NE_MAKE_ADDR(i+1, 0);
}

uint NE_get_seg_psize(ht_ne_shared_data *NE_shared, uint i)
{
	return (NE_shared->segments.segments[i].size || !NE_shared->segments.segments[i].offset)
		   ? NE_shared->segments.segments[i].size : 0x10000;
//	return NE_shared->segments.segments[i].size;
}

uint NE_get_seg_vsize(ht_ne_shared_data *NE_shared, uint i)
{
	return NE_shared->segments.segments[i].minalloc ? NE_shared->segments.segments[i].minalloc : 0x10000;
//	return NE_shared->segments.segments[i].minalloc;
}

bool NE_addr_to_segment(ht_ne_shared_data *NE_shared, NEAddress Addr, int *segment)
{
	for (uint i = 0; i < NE_shared->segments.segment_count; i++) {
		NEAddress base = NE_get_seg_addr(NE_shared, i);
		uint evsize = MAX(NE_get_seg_vsize(NE_shared, i), NE_get_seg_psize(NE_shared, i));
		if ((Addr >= base) && (Addr < base + evsize)) {
			*segment = i;
			return true;
		}
	}
	return false;
}

bool NE_addr_is_physical(ht_ne_shared_data *NE_shared, NEAddress Addr)
{
	for (uint i = 0; i < NE_shared->segments.segment_count; i++) {
		NEAddress base = NE_get_seg_addr(NE_shared, i);
		uint epsize = MIN(NE_get_seg_vsize(NE_shared, i), NE_get_seg_psize(NE_shared, i));
		if ((Addr >= base) && (Addr < base + epsize)) return true;
	}
	return false;
}

bool NE_addr_to_ofs(ht_ne_shared_data *NE_shared, NEAddress Addr, FileOfs *ofs)
{
	for (uint i = 0; i < NE_shared->segments.segment_count; i++) {
		NEAddress base = NE_get_seg_addr(NE_shared, i);
		uint epsize = MIN(NE_get_seg_vsize(NE_shared, i), NE_get_seg_psize(NE_shared, i));
		if ((Addr >= base) && (Addr < base + epsize)) {
			*ofs = NE_get_seg_ofs(NE_shared, i) + (Addr - base);
			return true;
		}
	}
	return false;
}

bool NE_ofs_to_addr(ht_ne_shared_data *NE_shared, FileOfs ofs, NEAddress *Addr)
{
	for (uint i = 0; i < NE_shared->segments.segment_count; i++) {
		FileOfs sofs = NE_get_seg_ofs(NE_shared, i);
		if ((ofs >= sofs) && (ofs < sofs + NE_get_seg_psize(NE_shared, i))) {
			*Addr = NE_get_seg_addr(NE_shared, i) + (ofs - sofs);
			return true;
		}
	}
	return false;
}
