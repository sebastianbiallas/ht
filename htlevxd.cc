/* 
 *	HT Editor
 *	htlevxd.cc
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
#include "htlevxd.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

#include "lestruct.h"

static ht_mask_ptable le_vxd_header[]=
{
	{"next",							STATICTAG_EDIT_DWORD_LE("00000000")},
	{"SDK version",					STATICTAG_EDIT_WORD_LE("00000004")},
	{"device number",					STATICTAG_EDIT_WORD_LE("00000006")},
	{"version major",					STATICTAG_EDIT_BYTE("00000008")},
	{"version minor",					STATICTAG_EDIT_BYTE("00000009")},
	{"flags",							STATICTAG_EDIT_WORD_LE("0000000a")},
	{"name",							STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")},
	{"init order",						STATICTAG_EDIT_DWORD_LE("00000014")},
	{"offset of control function",		STATICTAG_EDIT_DWORD_LE("00000018")},
	{"offset of V86-control function",		STATICTAG_EDIT_DWORD_LE("0000001c")},
	{"offset of PM-control function",		STATICTAG_EDIT_DWORD_LE("00000020")},
	{"CS:IP of V86-control function",		STATICTAG_EDIT_DWORD_LE("00000024")},
	{"CS:IP of PM-control function",		STATICTAG_EDIT_DWORD_LE("00000028")},
	{"reference data from real mode",		STATICTAG_EDIT_DWORD_LE("0000002c")},
	{"offset of service table",			STATICTAG_EDIT_DWORD_LE("00000030")},
	{"size of service table",			STATICTAG_EDIT_DWORD_LE("00000034")},
	{"offset of win32 service table",		STATICTAG_EDIT_DWORD_LE("00000038")},
	{"prev",							STATICTAG_EDIT_CHAR("0000003c")STATICTAG_EDIT_CHAR("0000003d")STATICTAG_EDIT_CHAR("0000003e")STATICTAG_EDIT_CHAR("0000003f")},
	{"size of this structure",			STATICTAG_EDIT_DWORD_LE("00000040")},
	{"reserved",						STATICTAG_EDIT_CHAR("00000044")STATICTAG_EDIT_CHAR("00000045")STATICTAG_EDIT_CHAR("00000046")STATICTAG_EDIT_CHAR("00000047")},
	{"reserved",						STATICTAG_EDIT_CHAR("00000048")STATICTAG_EDIT_CHAR("00000049")STATICTAG_EDIT_CHAR("0000004a")STATICTAG_EDIT_CHAR("0000004b")},
	{"reserved",						STATICTAG_EDIT_CHAR("0000004c")STATICTAG_EDIT_CHAR("0000004d")STATICTAG_EDIT_CHAR("0000004e")STATICTAG_EDIT_CHAR("0000004f")},
	{ NULL, NULL }
};

static ht_view *htlevxd_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_le_shared_data *le_shared=(ht_le_shared_data *)group->get_shared_data();
	if (!le_shared->is_vxd) return NULL;
	File *myfile = le_shared->reloc_file;

	/* FIXME: */
	bool le_bigendian = false;

	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_LE_VXD, VC_EDIT | VC_SEARCH, myfile, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(myfile, 0);

	char info[128];
	ht_snprintf(info, sizeof info, "* LE VXD descriptor in section 1, offset %08qx", le_shared->vxd_desc_linear_ofs);
	m->add_mask(info);
	m->add_staticmask_ptable(le_vxd_header, le_shared->vxd_desc_linear_ofs, le_bigendian);
	v->insertsub(m);

	le_shared->v_le_vxd = v;
	return v;
}

format_viewer_if htlevxd_if = {
	htlevxd_init,
	0
};
