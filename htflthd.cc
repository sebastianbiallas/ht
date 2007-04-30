/* 
 *	HT Editor
 *	htflthd.cc
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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

#include "fltstruc.h"
#include "atom.h"
#include "htflt.h"
#include "htflthd.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable fltheader[]=
{
	{"magic",			STATICTAG_EDIT_DWORD_BE("00000000")},
	{"version",		STATICTAG_EDIT_DWORD_BE("00000004")},
	{"entry",			STATICTAG_EDIT_DWORD_BE("00000008")},
	{"data_start",		STATICTAG_EDIT_DWORD_BE("0000000c")},
	{"data_end",		STATICTAG_EDIT_DWORD_BE("00000010")},
	{"bss_end",		STATICTAG_EDIT_DWORD_BE("00000014")},
	{"stack_size",		STATICTAG_EDIT_DWORD_BE("00000018")},
	{"reloc_start",		STATICTAG_EDIT_DWORD_BE("0000001c")},
	{"reloc_count",		STATICTAG_EDIT_DWORD_BE("00000020")},
	{0, 0}
};

static ht_view *htfltheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_flt_shared_data *flt_shared=(ht_flt_shared_data *)group->get_shared_data();
	
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_FLT_HEADER, VC_EDIT, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* FLAT header at offset %08qx", flt_shared->header_ofs);
	m->add_mask(info);
	m->add_staticmask_ptable(fltheader, flt_shared->header_ofs, true);

	v->insertsub(m);
	return v;
}

format_viewer_if htfltheader_if = {
	htfltheader_init,
	0
};
