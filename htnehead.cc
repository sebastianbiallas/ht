/* 
 *	HT Editor
 *	htnehead.cc
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
#include "htne.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable neheader[]=
{
	{"magic",							STATICTAG_EDIT_WORD_LE("00000000")},
	{"version",						STATICTAG_EDIT_BYTE("00000002")},
	{"revision",						STATICTAG_EDIT_BYTE("00000003")},
	{"offset of entry table",			STATICTAG_EDIT_WORD_LE("00000004")},
	{"size of entry table",				STATICTAG_EDIT_WORD_LE("00000006")},
	{"crc",							STATICTAG_EDIT_DWORD_LE("00000008")},
	{"flags",							STATICTAG_EDIT_WORD_LE("0000000c")" "STATICTAG_FLAGS("0000000c", ATOM_NE_FLAGS_STR)},
	{"autodata",						STATICTAG_EDIT_WORD_LE("0000000e")},
	{"heap min",						STATICTAG_EDIT_WORD_LE("00000010")},
	{"stack min",						STATICTAG_EDIT_WORD_LE("00000012")},
	{"CS:IP",							STATICTAG_EDIT_WORD_LE("00000016")":"STATICTAG_EDIT_WORD_LE("00000014")},
	{"SS:SP",							STATICTAG_EDIT_WORD_LE("0000001a")":"STATICTAG_EDIT_WORD_LE("00000018")},
	{"number of segments",				STATICTAG_EDIT_WORD_LE("0000001c")},
	{"number of module references",		STATICTAG_EDIT_WORD_LE("0000001e")},
	{"size of non-resident name table",	STATICTAG_EDIT_WORD_LE("00000020")},
	{"offset of segment table",			STATICTAG_EDIT_WORD_LE("00000022")},
	{"offset of resource table",			STATICTAG_EDIT_WORD_LE("00000024")},
	{"offset of resident names table",		STATICTAG_EDIT_WORD_LE("00000026")},
	{"offset of module reference table",	STATICTAG_EDIT_WORD_LE("00000028")},
	{"offset of imported names table",		STATICTAG_EDIT_WORD_LE("0000002a")},
	{"offset of non-resident names table",	STATICTAG_EDIT_DWORD_LE("0000002c")},
	{"number of movable entries",			STATICTAG_EDIT_WORD_LE("00000030")},
	{"segment alignment shift count",		STATICTAG_EDIT_WORD_LE("00000032")},
	{"number of resource segments",		STATICTAG_EDIT_WORD_LE("00000034")},
	{"target os",						STATICTAG_EDIT_BYTE("00000036")" "STATICTAG_DESC_BYTE("00000036", ATOM_NE_OS_STR)},
	{"additional flags",				STATICTAG_EDIT_BYTE("00000037")},
	{"offset of return thunks",			STATICTAG_EDIT_WORD_LE("00000038")},
	{"offset of segment reference bytes",	STATICTAG_EDIT_WORD_LE("0000003a")},
	{"minimum code swap area size",		STATICTAG_EDIT_WORD_LE("0000003c")},
	{"expected os version",				STATICTAG_EDIT_WORD_LE("0000003e")},
	{0, 0}
};

static int_hash ne_os[] =
{
	{NE_OS_UNKNOWN, "all"},
	{NE_OS_OS2, "OS/2"},
	{NE_OS_WINDOWS, "Windows"},
	{NE_OS_DOS4, "DOS 4.x"},
	{0, 0}
};

static ht_tag_flags_s ne_flags[] =
{
	{-1, "NE - file characteristics"},
	{0,  "[00] solo data"},
	{1,  "[01] instance data"},
	{2,  "[02] per-process library initialization"},
	{3,  "[03] protected mode only"},
	{4,  "[04] 8086 instructions"},
	{5,  "[05] 80286 instructions"},
	{6,  "[06] 80386 instructions"},
	{7,  "[07] fpu instructions"},
	{8,  "[08] w0"},
	{9,  "[09] w1"},
	{10, "[10] w2"},
	{11, "[11] bound as family app"},
	{12, "[12] reserved"},
	{13, "[13] errors in image"},
	{14, "[14] reserved"},
	{15, "[15] not a process"},
	{0, 0}
};

static ht_view *htneheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared=(ht_ne_shared_data *)group->get_shared_data();

	int h=ne_shared->hdr_ofs;
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_NE_HEADER, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m=new ht_mask_sub();
	m->init(file, 0);

	registerAtom(ATOM_NE_OS, ne_os);
	registerAtom(ATOM_NE_FLAGS, ne_flags);

	char info[128];
	ht_snprintf(info, sizeof info, "* NE header at offset 0x%08qx", ne_shared->hdr_ofs);
	m->add_mask(info);
	m->add_staticmask_ptable(neheader, h, false);
	v->insertsub(m);

	return v;
}

format_viewer_if htneheader_if = {
	htneheader_init,
	0
};
