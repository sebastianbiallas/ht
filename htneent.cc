/* 
 *	HT Editor
 *	htneent.cc
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

#include "atom.h"
#include "htctrl.h"
#include "endianess.h"
#include "htiobox.h"
#include "htne.h"
#include "htneent.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_tag_flags_s ne_entflags[] =
{
	{-1, "NE - entrypoint flags"},
	{0,  "[00] exported"},
	{1,  "[01] single data"},
	{2,  "[02] reserved"},
	{0, 0}
};

static ht_view *htneentrypoints_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data *)group->get_shared_data();

	FileOfs h = ne_shared->hdr_ofs;
	ht_ne_entrypoint_viewer *v = new ht_ne_entrypoint_viewer();
	v->init(b, DESC_NE_ENTRYPOINTS, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);

	registerAtom(ATOM_NE_ENTFLAGS, ne_entflags);

	char line[1024];
	ht_snprintf(line, sizeof line, "* NE entrypoint table at offset 0x%08qx", h+ne_shared->hdr.enttab);
	m->add_mask(line);

	FileOfs o = h + ne_shared->hdr.enttab;
	NE_ENTRYPOINT_HEADER e;

	uint32 index = 1;
	while (o + sizeof e < h+ne_shared->hdr.enttab+ne_shared->hdr.cbenttab) {
		file->seek(o);
		file->read(&e, sizeof e);
		createHostStruct(&e, NE_ENTRYPOINT_HEADER_struct, little_endian);
		o += sizeof e;

		if (e.seg_index==0) {
/*			sprintf(line, "null entries [%d]", e.entry_count);
			m->add_mask(line);*/
		} else if (e.seg_index==0xff) {
			ht_snprintf(line, sizeof line, "entrypoints for movable segment [%d entries]", e.entry_count);
			m->add_mask(line);
		} else {
			ht_snprintf(line, sizeof line, "entrypoints for fixed segment %d [%d entries]", e.seg_index, e.entry_count);
			m->add_mask(line);
		}
		// FIXME: dont use sprintf
		for (int i=0; i<e.entry_count; i++) {
			if (e.seg_index == 0) {
			} else if (e.seg_index == 0xff) {
				char *l = line;
				char *l_end = line+sizeof line - 1;
				l += ht_snprintf(l, sizeof line, "%04x: ", index);
				l = tag_make_edit_byte(l, l_end-l, o+3);
				l += ht_snprintf(l, l_end-l, ":");
				l = tag_make_edit_word(l, l_end-l, o+4, tag_endian_little);
				l += ht_snprintf(l, l_end-l, " ");
				l = tag_make_ref(l, l_end-l, o, 0xff, 0, 0, "goto");
				l += ht_snprintf(l, l_end-l, " flags=");
				l = tag_make_edit_byte(l, l_end-l, o);
				l += ht_snprintf(l, l_end-l, " ");
				l = tag_make_flags(l, l_end-l, ATOM_NE_ENTFLAGS, o);
				*l = 0;
				m->add_mask(line);
				o += sizeof (NE_ENTRYPOINT_MOVABLE);
			} else {
				char *l = line;
				char *l_end = line+sizeof line - 1;
				l += ht_snprintf(l, l_end-l, "%04x:    ", index);
				l = tag_make_edit_word(l, l_end-l, o+1, tag_endian_little);
				l += ht_snprintf(l, l_end-l, " ");
				l = tag_make_ref(l, l_end-l, o, e.seg_index, 0, 0, "goto");
				l += ht_snprintf(l, l_end-l, " flags=");
				l = tag_make_edit_byte(l, l_end-l, o);
				l += ht_snprintf(l, l_end-l, " ");
				l = tag_make_flags(l, l_end-l, ATOM_NE_ENTFLAGS, o);
				*l = 0;
				m->add_mask(line);
				o += sizeof (NE_ENTRYPOINT_FIXED);
			}
			index++;
		}
	}

	v->insertsub(m);

	return v;
}

format_viewer_if htneentrypoints_if = {
	htneentrypoints_init,
	NULL
};

/*
 *	CLASS ht_ne_entrypoint_viewer
 */

bool ht_ne_entrypoint_viewer::ref_sel(LINE_ID *id)
{
/*   FIXNEW
	uint seg = id_high;
	FileOfs o = id_low;
	ADDR a;
	if (seg == 0xff) {
		NE_ENTRYPOINT_MOVABLE e;
		file->seek(o);
		file->read(&e, sizeof e);
		createHostStruct(&e, NE_ENTRYPOINT_MOVABLE_struct, little_endian);
		a = NE_MAKE_ADDR(e.seg, e.offset);
	} else {
		NE_ENTRYPOINT_FIXED e;
		file->seek(o);
		file->read(&e, sizeof e);
		createHostStruct(&e, NE_ENTRYPOINT_FIXED_struct, little_endian);
		a = NE_MAKE_ADDR(seg, e.offset);
	}

	ht_ne_shared_data *ne_shared=(ht_ne_shared_data *)format_group->get_shared_data();

	if (ne_shared->v_image->goto_address2(a, this)) {
		app->focus(ne_shared->v_image);
	} else errorbox("can't follow: address %y is not valid!", a);
	return 1;*/
	return false;
}
