/* 
 *	HT Editor
 *	htleobj.cc
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
#include "htleobj.h"
#include "httag.h"
#include "strtools.h"
#include "formats.h"
#include "snprintf.h"

#include "lestruct.h"

#include <stdlib.h>
#include <string.h>

static ht_mask_ptable leobj[]=
{
	{"virtual size",			STATICTAG_EDIT_DWORD_LE("00000000")},
	{"relocation base address",	STATICTAG_EDIT_DWORD_LE("00000004")},
	{"flags",					STATICTAG_EDIT_DWORD_LE("00000008")" "STATICTAG_FLAGS("00000008", ATOM_LE_OBJFLAGS_STR)},
	{"page map index",			STATICTAG_EDIT_DWORD_LE("0000000c")},
	{"page map count",			STATICTAG_EDIT_DWORD_LE("00000010")},
	{"name",					STATICTAG_EDIT_CHAR("00000014")""STATICTAG_EDIT_CHAR("00000015")""STATICTAG_EDIT_CHAR("00000016")""STATICTAG_EDIT_CHAR("00000017")},
	{0, 0}
};

static ht_tag_flags_s le_objflags[] =
{
	{0,  "[00] readable"},
	{1,  "[01] writable"},
	{2,  "[02] executable"},
	{3,  "[03] resource"},
	{4,  "[04] discardable"},
	{5,  "[05] shared"},
	{6,  "[06] preloaded"},
	{7,  "[07] * reserved"},
	{8,  "[08] * reserved"},
	{9,  "[09] * reserved"},
	{10, "[10] resident"},
	{11, "[11] * reserved"},
	{12, "[12] 16:16 alias"},
	{13, "[13] use32"},
	{14, "[14] conforming"},
	{15, "[15] object i/o privilege level"},
	{0, 0}
};

static ht_view *htleobjects_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_le_shared_data *le_shared=(ht_le_shared_data *)group->get_shared_data();

	uint32 h=le_shared->hdr_ofs;
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_LE_OBJECTS, VC_EDIT, file, group);
	ht_mask_sub *m=new ht_mask_sub();
	m->init(file, 0);

	registerAtom(ATOM_LE_OBJFLAGS, le_objflags);

	/* FIXME: */
	bool le_bigendian = false;

	char t[64];
	ht_snprintf(t, sizeof t, "* LE object headers at offset 0x%08qx", h+le_shared->hdr.objtab);
	m->add_mask(t);

	v->insertsub(m);

	for (uint32 i=0; i<le_shared->hdr.objcnt; i++) {
		m=new ht_mask_sub();
		m->init(file, i);

		char n[5];
		m->add_staticmask_ptable(leobj, h+le_shared->hdr.objtab+i*24, le_bigendian);
		
		memcpy(&n, le_shared->objmap.header[i].name, 4);
		n[4]=0;

		bool use32=le_shared->objmap.header[i].flags & LE_OBJECT_FLAG_USE32;

		ht_snprintf(t, sizeof t, "--- object %d USE%d: %s ---", i+1, use32 ? 32 : 16, (char*)&n);

		ht_collapsable_sub *cs=new ht_collapsable_sub();
		cs->init(file, m, 1, t, 1);
		v->insertsub(cs);
	}

	le_shared->v_objects = v;
	return v;
}

format_viewer_if htleobjects_if = {
	htleobjects_init,
	0
};
