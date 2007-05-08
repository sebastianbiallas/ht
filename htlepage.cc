/* 
 *	HT Editor
 *	htlepage.cc
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

#include "htnewexe.h"
#include "htle.h"
#include "htlepage.h"
#include "httag.h"
#include "strtools.h"
#include "formats.h"
#include "snprintf.h"

#include "lestruct.h"

#include <stdlib.h>

static ht_mask_ptable lepagemap[]=
{
	{"page high",	STATICTAG_EDIT_WORD_LE("00000000")},
	{"page low",	STATICTAG_EDIT_BYTE("00000002")},
	{"flags",		STATICTAG_EDIT_BYTE("00000003")},
	{0, 0}
};

static ht_view *htlepagemaps_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_le_shared_data *le_shared=(ht_le_shared_data *)group->get_shared_data();

	FileOfs h = le_shared->hdr_ofs;
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_LE_PAGEMAP, VC_EDIT | VC_SEARCH, file, group);
	
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);

	char t[64];
	ht_snprintf(t, sizeof t, "* LE page maps at offset %08qx", h+le_shared->hdr.pagemap);
	m->add_mask(t);

	v->insertsub(m);

	/* FIXME: */
	bool le_bigendian = false;

	for (uint32 i=0; i<le_shared->hdr.pagecnt; i++) {
		m=new ht_mask_sub();
		m->init(file, i);
		
		ht_snprintf(t, sizeof t, "--- page %d at %08x ---", i+1, le_shared->pagemap.offset[i]);
		m->add_staticmask_ptable(lepagemap, h+le_shared->hdr.pagemap+i*4, le_bigendian);
		ht_collapsable_sub *cs=new ht_collapsable_sub();
		cs->init(file, m, 1, t, 1);
		v->insertsub(cs);
	}

	le_shared->v_pagemaps=v;
	return v;
}

format_viewer_if htlepagemaps_if = {
	htlepagemaps_init,
	0
};
