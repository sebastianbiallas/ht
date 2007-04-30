/* 
 *	HT Editor
 *	htmzhead.cc
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

#include "htmz.h"
#include "htmzhead.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable mzheader[]=
{
	{"magic",				STATICTAG_EDIT_WORD_LE("00000000")},
	{"bytes on last page of file",		STATICTAG_EDIT_WORD_LE("00000002")},
	{"pages in file",			STATICTAG_EDIT_WORD_LE("00000004")},
	{"number of relocations",		STATICTAG_EDIT_WORD_LE("00000006")},
	{"size of header in paragraphs",	STATICTAG_EDIT_WORD_LE("00000008")},
	{"minimum extra paragraphs needed",	STATICTAG_EDIT_WORD_LE("0000000a")},
	{"maximum extra paragraphs needed",	STATICTAG_EDIT_WORD_LE("0000000c")},
	{"initial SS value",			STATICTAG_EDIT_WORD_LE("0000000e")},
	{"initial SP value",			STATICTAG_EDIT_WORD_LE("00000010")},
	{"checksum",				STATICTAG_EDIT_WORD_LE("00000012")},
	{"initial IP value",			STATICTAG_EDIT_WORD_LE("00000014")},
	{"initial CS value",			STATICTAG_EDIT_WORD_LE("00000016")},
	{"offset of relocation table",		STATICTAG_EDIT_WORD_LE("00000018")},
	{"overlay number",			STATICTAG_EDIT_WORD_LE("0000001a")},
	{"reserved",				STATICTAG_EDIT_BYTE("0000001c")" "STATICTAG_EDIT_BYTE("0000001d")" "STATICTAG_EDIT_BYTE("0000001e")" "STATICTAG_EDIT_BYTE("0000001f")" "STATICTAG_EDIT_BYTE("00000020")" "STATICTAG_EDIT_BYTE("00000021")" "STATICTAG_EDIT_BYTE("00000022")" "STATICTAG_EDIT_BYTE("00000023")},
	{"reserved",				STATICTAG_EDIT_BYTE("00000024")" "STATICTAG_EDIT_BYTE("00000025")" "STATICTAG_EDIT_BYTE("00000026")" "STATICTAG_EDIT_BYTE("00000027")" "STATICTAG_EDIT_BYTE("00000028")" "STATICTAG_EDIT_BYTE("00000029")" "STATICTAG_EDIT_BYTE("0000002a")" "STATICTAG_EDIT_BYTE("0000002b")},
	{"reserved",				STATICTAG_EDIT_BYTE("0000002c")" "STATICTAG_EDIT_BYTE("0000002d")" "STATICTAG_EDIT_BYTE("0000002e")" "STATICTAG_EDIT_BYTE("0000002f")" "STATICTAG_EDIT_BYTE("00000030")" "STATICTAG_EDIT_BYTE("00000031")" "STATICTAG_EDIT_BYTE("00000032")" "STATICTAG_EDIT_BYTE("00000033")},
	{"reserved",				STATICTAG_EDIT_BYTE("00000034")" "STATICTAG_EDIT_BYTE("00000035")" "STATICTAG_EDIT_BYTE("00000036")" "STATICTAG_EDIT_BYTE("00000037")" "STATICTAG_EDIT_BYTE("00000038")" "STATICTAG_EDIT_BYTE("00000039")" "STATICTAG_EDIT_BYTE("0000003a")" "STATICTAG_EDIT_BYTE("0000003b")},
	{"file offset of new executable header",STATICTAG_EDIT_DWORD_LE("0000003c")},
	{0, 0}
};

static ht_view *htmzheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_MZ_HEADER, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* MZ header at offset 0x%08x (paragraph=16 bytes, page=512 bytes)", 0);	/* FIXME: hmm, seems to be always 0 ?! */
	m->add_mask(info);
	m->add_staticmask_ptable(mzheader, 0, false);
	v->insertsub(m);
	return v;
}

format_viewer_if htmzheader_if = {
	htmzheader_init,
	0
};
