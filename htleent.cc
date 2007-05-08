/* 
 *	HT Editor
 *	htleent.cc
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
#include "htnewexe.h"
#include "htle.h"
#include "htlehead.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

#include "lestruct.h"

/* entry bundle */

static ht_tag_flags_s le_entry_bundle_flags[] =
{
	{-1, "LE - entrypoint bundle flags"},
	{0,  "[00] valid"},
	{1,  "[01] 32bit (16bit otherwise)"},
	{0, 0}
};

static ht_mask_ptable le_entry_bundle_header[]=
{
	{"number of entries",	STATICTAG_EDIT_BYTE("00000000")},
	{"bundle flags",		STATICTAG_EDIT_BYTE("00000001")" "STATICTAG_FLAGS("00000001", ATOM_LE_ENTRY_BUNDLE_FLAGS_STR)},
	{"object index",		STATICTAG_EDIT_WORD_LE("00000002")},
	{0,0}
};

/* entry */

static ht_tag_flags_s le_entry_flags[] =
{
	{-1, "LE - entrypoint flags"},
	{0,  "[00] exported"},
	{1,  "[01] uses shared segment"},
	{0, 0}
};

static ht_mask_ptable le_entry16[]=
{
	{"flags",		STATICTAG_EDIT_BYTE("00000000")" "STATICTAG_FLAGS("00000000", ATOM_LE_ENTRY_FLAGS_STR)},
	{"offset",	STATICTAG_EDIT_WORD_LE("00000001")},
	{0, 0}
};

static ht_mask_ptable le_entry32[]=
{
	{"flags",		STATICTAG_EDIT_BYTE("00000000")" "STATICTAG_FLAGS("00000000", ATOM_LE_ENTRY_FLAGS_STR)},
	{"offset",	STATICTAG_EDIT_DWORD_LE("00000001")},
	{0, 0}
};

static ht_view *htleentrypoints_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_le_shared_data *le_shared=(ht_le_shared_data *)group->get_shared_data();

	int h=le_shared->hdr_ofs;
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_LE_ENTRYPOINTS, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m=new ht_mask_sub();
	m->init(file, 0);
	registerAtom(ATOM_LE_ENTRY_FLAGS, le_entry_flags);
	registerAtom(ATOM_LE_ENTRY_BUNDLE_FLAGS, le_entry_bundle_flags);
	char info[128];
	
	ht_snprintf(info, sizeof info, "* LE entry header at offset 0x%08qx", h+le_shared->hdr.enttab);

	/* FIXME: false */
	bool le_bigendian = false;
	m->add_mask(info);
	m->add_staticmask_ptable(le_entry_bundle_header, h+le_shared->hdr.enttab, le_bigendian);
	v->insertsub(m);

	FileOfs o=h+le_shared->hdr.enttab;
	while (1) {
		char t[32];
		LE_ENTRYPOINT_BUNDLE hdr;
		hdr.entry_count=0;
		file->seek(o);
		o+=file->read(&hdr, sizeof hdr);
		if (!hdr.entry_count) break;
		const char *flags_str;
		if (hdr.flags & LE_ENTRYPOINT_BUNDLE_32BIT) {
			flags_str="32-bit";
		} else {
			flags_str="16-bit";
		}
		for (int i=0; i<hdr.entry_count; i++) {
			m=new ht_mask_sub();
			m->init(file, i);
			if (hdr.flags & LE_ENTRYPOINT_BUNDLE_32BIT) {
				m->add_staticmask_ptable(le_entry32, o, le_bigendian);
				o+=1+4;
			} else {
				m->add_staticmask_ptable(le_entry16, o, le_bigendian);
				o+=1+2;
			}
			ht_snprintf(t, sizeof t, "--- entry %d (%s) ---", i+1, flags_str);

			ht_collapsable_sub *cs=new ht_collapsable_sub();
			cs->init(file, m, 1, t, 1);
			v->insertsub(cs);
		}
	}
	
	return v;
}

format_viewer_if htleentrypoints_if = {
	htleentrypoints_init,
	0
};
