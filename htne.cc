/* 
 *	HT Editor
 *	htne.cc
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

#include "htapp.h"
#include "htendian.h"
#include "htiobox.h"
#include "htne.h"
#include "htneent.h"
#include "htnehead.h"
#include "htneimp.h"
#include "htnenms.h"
#include "htneobj.h"
#include "htneimg.h"
#include "htnewexe.h"
#include "ne_analy.h"
#include "nestruct.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

format_viewer_if *htne_ifs[] = {
	&htneheader_if,
	&htnesegments_if,
	&htneimports_if,
	&htneentrypoints_if,
	&htnenames_if,
	&htneimage_if,
	0
};

ht_view *htne_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	byte nemagic[2];
	FILEOFS h = get_newexe_header_ofs(file);
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

void ht_ne::init(bounds *b, ht_streamfile *f, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS h)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_NE, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_ne");

	LOG("%s: NE: found header at %08x", file->get_filename(), h);

	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)malloc(sizeof (ht_ne_shared_data));
	shared_data = ne_shared;

	ne_shared->fake_segment = 0;
	ne_shared->hdr_ofs = h;
	ne_shared->entrypoints = NULL;
	ne_shared->v_image = NULL;

	file->seek(h);
	file->read(&ne_shared->hdr, sizeof ne_shared->hdr);
	create_host_struct(&ne_shared->hdr, NE_HEADER_struct, little_endian);

/* read segment descriptors */
	ne_shared->segments.segment_count = ne_shared->hdr.cseg;
	ne_shared->segments.segments = (NE_SEGMENT *)malloc(sizeof (NE_SEGMENT) * ne_shared->segments.segment_count);

	bool reloc_needed = false;
	NE_SEGMENT *s = ne_shared->segments.segments;
	file->seek(h+ne_shared->hdr.segtab);
	for (dword i = 0; i < ne_shared->segments.segment_count; i++) {
		file->read(s, sizeof *s);
		create_host_struct(s, NE_SEGMENT_struct, little_endian);
		if (s->flags & NE_HASRELOC) reloc_needed = true;
		s++;
	}
/* read entrypoint descriptors */
	FILEOFS o = h + ne_shared->hdr.enttab;
	NE_ENTRYPOINT_HEADER e;
	file->seek(o);
	file->read(&e, sizeof e);
	create_host_struct(&e, NE_ENTRYPOINT_HEADER_struct, little_endian);
	o += sizeof e;

	ht_clist *ep = new ht_clist();
	ep->init();

	dword index = 1;
	while (o<h+ne_shared->hdr.enttab+ne_shared->hdr.cbenttab) {
		for (int i=0; i<e.entry_count; i++) {
			if (e.seg_index==0) {
			} else if (e.seg_index==0xff) {
				o+=sizeof (NE_ENTRYPOINT_MOVABLE);
				
				NE_ENTRYPOINT_MOVABLE me;
				file->read(&me, sizeof me);
				create_host_struct(&me, NE_ENTRYPOINT_MOVABLE_struct, little_endian);

				ep->set(index, new ht_ne_entrypoint(index, me.seg, me.offset, 0));
			} else {
				o+=sizeof (NE_ENTRYPOINT_FIXED);
				
				NE_ENTRYPOINT_FIXED fe;
				file->seek(o);
				file->read(&fe, sizeof fe);
				create_host_struct(&fe, NE_ENTRYPOINT_FIXED_struct, little_endian);
				ep->set(index, new ht_ne_entrypoint(index, e.seg_index, fe.offset, 0));
			}
			index++;
		}
		file->seek(o);
		file->read(&e, sizeof e);
		create_host_struct(&e, NE_ENTRYPOINT_HEADER_struct, little_endian);
		o += sizeof e;
	}

	ne_shared->entrypoints = ep;
/* do relocations */
	if (reloc_needed) {
		ht_ne_reloc_file *rf = new ht_ne_reloc_file();
		rf->init(file, false, (ht_ne_shared_data*)shared_data);

		if (relocate(rf)) {
			rf->finalize();
			file = rf;
			own_file = true;
		} else {
			errorbox("%s: NE relocations seem to be corrupted.", file->get_filename());
			rf->done();
			delete rf;
		}
	}

	ht_format_group::init_ifs(ifs);
}

void ht_ne::done()
{
	ht_format_group::done();

	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	if (ne_shared->entrypoints) {
		ne_shared->entrypoints->destroy();
		delete ne_shared->entrypoints;
	}
	free(shared_data);
}

void ht_ne::loc_enum_start()
{
	loc_enum=true;
}

bool ht_ne::loc_enum_next(ht_format_loc *loc)
{
	ht_ne_shared_data *sh=(ht_ne_shared_data*)shared_data;
	if (loc_enum) {
		loc->name="ne";
		loc->start=sh->hdr_ofs;
		loc->length=file->get_size()-loc->start;	/* FIXME: ENOTOK */
		
		loc_enum=false;
		return true;
	}
	return false;
}

bool ht_ne::create_fake_segment()
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	UINT i = ne_shared->hdr.cseg;
	NE_SEGMENT *newsegs = (NE_SEGMENT*)malloc(sizeof *newsegs * (ne_shared->segments.segment_count+1));
	memcpy(newsegs, ne_shared->segments.segments, sizeof *newsegs * ne_shared->segments.segment_count);
	ne_shared->segments.segment_count++;
	free(ne_shared->segments.segments);
	ne_shared->segments.segments = newsegs;
	newsegs[i].offset = 0xffff;
	newsegs[i].size = 0;
	newsegs[i].flags = NE_DATA;
	newsegs[i].minalloc = 0xffff;
	ne_shared->fake_segment = i;
	return true;
}

bool ht_ne::relocate(ht_reloc_file *rf)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	if (!create_fake_segment()) return false;
	/* if bound only relocate first segment. Is this "The Right Thing" ? */
	/* Works for me though. */
	UINT c = (ne_shared->hdr.flags & NE_FLAGS_BOUND) ? 1 : ne_shared->segments.segment_count;
	for (dword i = 0; i < c; i++) {
		if (ne_shared->segments.segments[i].flags & NE_HASRELOC) {
			FILEOFS seg_ofs = NE_get_seg_ofs(ne_shared, i);
			FILEOFS f = seg_ofs + NE_get_seg_psize(ne_shared, i);
			char buf[2];
			file->seek(f);
			file->read(buf, 2);
			f += 2;
			int c = create_host_int(buf, 2, little_endian);
			for (int j = 0; j < c; j++) {
				NE_RELOC_HEADER reloc;
				file->seek(f);
				file->read(&reloc, sizeof reloc);
				create_host_struct(&reloc, NE_RELOC_HEADER_struct, little_endian);
				f += sizeof reloc;
				switch (reloc.flags & NE_RF_RT_MASK) {
					case NE_RF_INTERNAL: {
						NE_RELOC_INTERNAL sreloc;
						file->read(&sreloc, sizeof sreloc);
						create_host_struct(&sreloc, NE_RELOC_INTERNAL_struct, little_endian);
						f += sizeof sreloc;
						if (sreloc.seg == 0xff) {
//                              	ne_shared->entrypoint->;
							if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xf1, 0)) return false;
						} else {
							if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, sreloc.seg, sreloc.ofs)) return false;
						}
						break;
					}
					case NE_RF_IMPORT_ORD: {
						NE_RELOC_IMPORT sreloc;
						file->read(&sreloc, sizeof sreloc);
						create_host_struct(&sreloc, NE_RELOC_IMPORT_struct, little_endian);
						f += sizeof sreloc;
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xf2, 0)) return false;
						break;
					}
					case NE_RF_IMPORT_NAME: {
						NE_RELOC_IMPORT sreloc;
						file->read(&sreloc, sizeof sreloc);
						create_host_struct(&sreloc, NE_RELOC_IMPORT_struct, little_endian);
						f += sizeof sreloc;
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xf3, 0)) return false;
						break;
					}
					case NE_RF_OSFIXUP: {
						NE_RELOC_FIXUP sreloc;
						file->read(&sreloc, sizeof sreloc);
						create_host_struct(&sreloc, NE_RELOC_FIXUP_struct, little_endian);
						f += sizeof sreloc;
						if (!relocate_single(rf, i, seg_ofs + reloc.src_ofs, reloc.type, reloc.flags, 0xdead, 0xcafebabe)) return false;
						break;
					}
				}
			}
		}
	}
	return true;
}

bool ht_ne::relocate_single(ht_reloc_file *rf, UINT seg, FILEOFS ofs, UINT type, UINT flags, word value_seg, word value_ofs)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data*)shared_data;
	while (1) {
		rf->insert_reloc(ofs, new ht_ne_reloc_entry(type, flags & NE_RF_ADD, value_seg, value_ofs));
		if (flags & NE_RF_ADD) break;
		switch (type & NE_RT_MASK) {
			case NE_RT_SEG16:
			case NE_RT_PTR32:
			case NE_RT_OFS16:
			case NE_RT_PTR48:
			case NE_RT_OFS32:
				break;
			/* FIXME: we would want to read a word (offset) out of
			   the file, but we can't because there's just one relocated
			   byte. What wrong ? */
			case NE_RT_OFS8:
			/* unknown relocation record */
			default:
				return false;
		}
		char buf[2];
		file->seek(ofs);
		file->read(buf, 2);
		word r = create_host_int(buf, 2, little_endian);
		if (r == 0xffff) break;
		ADDR a = NE_MAKE_ADDR(seg+1, r);
		if (!NE_addr_to_ofs(ne_shared, a, &ofs))
			return false;
	}
	return true;
}

/*
 *	CLASS ht_ne_aviewer
 */

void ht_ne_aviewer::init(bounds *b, char *desc, int caps, ht_streamfile *File, ht_format_group *format_group, analyser *Analyser, ht_ne_shared_data *NE_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analyser);
	ne_shared = NE_shared;
	file = File;
}

void ht_ne_aviewer::set_analyser(analyser *a)
{
	((ne_analyser*)a)->ne_shared = ne_shared;
	((ne_analyser*)a)->file = file;
	analy = a;
	analy_sub->set_analyser(a);
}

/*
 *	CLASS ht_ne_entrypoint
 */

ht_ne_entrypoint::ht_ne_entrypoint(UINT Ordinal, UINT Seg, UINT Offset, UINT Flags)
{
	ordinal = Ordinal;
	seg = Seg;
	offset = Offset;
	flags = Flags;
	name = NULL;
}

/*
 *	CLASS ht_ne_reloc_entry
 */

ht_ne_reloc_entry::ht_ne_reloc_entry(UINT Mode, bool Add, word Seg, word Ofs)
{
	mode = Mode;
	add = Add;
	seg = Seg;
	ofs = Ofs;
}

/*
 *	CLASS ht_ne_reloc_file
 */

void ht_ne_reloc_file::init(ht_streamfile *s, bool os, ht_ne_shared_data *d)
{
	ht_reloc_file::init(s, os);
	data = d;
}

void ht_ne_reloc_file::reloc_apply(ht_data *reloc, byte *data)
{
	ht_ne_reloc_entry *e = (ht_ne_reloc_entry*)reloc;

	switch (e->mode & NE_RT_MASK) {
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
	}
}

void ht_ne_reloc_file::reloc_unapply(ht_data *reloc, byte *data)
{
//	ht_ne_reloc_entry *e = (ht_ne_reloc_entry*)reloc;
}

/*
 *
 */

FILEOFS NE_get_seg_ofs(ht_ne_shared_data *NE_shared, UINT i)
{
		return (NE_shared->segments.segments[i].offset << NE_shared->hdr.align);
}

ADDR NE_get_seg_addr(ht_ne_shared_data *NE_shared, UINT i)
{
		return (i+1)*0x10000;
}

UINT NE_get_seg_psize(ht_ne_shared_data *NE_shared, UINT i)
{
// FIXME: documented like this everywhere, but doesnt work for dmcolor.dll e.g.
//		return NE_shared->segments.segments[i].size ? NE_shared->segments.segments[i].size : 0x10000;
		return NE_shared->segments.segments[i].size;
}

UINT NE_get_seg_vsize(ht_ne_shared_data *NE_shared, UINT i)
{
// FIXME: documented like this everywhere, but doesnt work for dmcolor.dll e.g.
//		return NE_shared->segments.segments[i].minalloc ? NE_shared->segments.segments[i].minalloc : 0x10000;
		return NE_shared->segments.segments[i].minalloc;
}

bool NE_addr_to_segment(ht_ne_shared_data *NE_shared, ADDR Addr, int *segment)
{
	for (UINT i = 0; i < NE_shared->segments.segment_count; i++) {
		ADDR base = NE_get_seg_addr(NE_shared, i);
		UINT evsize = MAX(NE_get_seg_vsize(NE_shared, i), NE_get_seg_psize(NE_shared, i));
		if ((Addr >= base) && (Addr < base + evsize)) {
			*segment = i;
			return true;
		}
	}
	return false;
}

bool NE_addr_is_physical(ht_ne_shared_data *NE_shared, ADDR Addr)
{
	for (UINT i = 0; i < NE_shared->segments.segment_count; i++) {
		ADDR base = NE_get_seg_addr(NE_shared, i);
		UINT epsize = MIN(NE_get_seg_vsize(NE_shared, i), NE_get_seg_psize(NE_shared, i));
		if ((Addr >= base) && (Addr < base + epsize)) return true;
	}
	return false;
}

bool NE_addr_to_ofs(ht_ne_shared_data *NE_shared, ADDR Addr, FILEOFS *ofs)
{
	for (UINT i = 0; i < NE_shared->segments.segment_count; i++) {
		ADDR base = NE_get_seg_addr(NE_shared, i);
		UINT epsize = MIN(NE_get_seg_vsize(NE_shared, i), NE_get_seg_psize(NE_shared, i));
		if ((Addr >= base) && (Addr < base + epsize)) {
			*ofs = NE_get_seg_ofs(NE_shared, i) + (Addr - base);
			return true;
		}
	}
	return false;
}

bool NE_ofs_to_addr(ht_ne_shared_data *NE_shared, FILEOFS ofs, ADDR *Addr)
{
	for (UINT i = 0; i < NE_shared->segments.segment_count; i++) {
		FILEOFS sofs = NE_get_seg_ofs(NE_shared, i);
		if ((ofs >= sofs) && (ofs < sofs + NE_get_seg_psize(NE_shared, i))) {
			*Addr = NE_get_seg_addr(NE_shared, i) + (ofs - sofs);
			return true;
		}
	}
	return false;
}


