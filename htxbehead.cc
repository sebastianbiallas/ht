/*
 *	HT Editor
 *	htxbehead.cc
 *
 *	Copyright (C) 2003 Stefan Esser
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

#include "formats.h"
#include "htapp.h"
#include "atom.h"
#include "htcoff.h"
#include "htctrl.h"
#include "endianess.h"
#include "hthex.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htxbe.h"
#include "htxbehead.h"
#include "httag.h"
#include "strtools.h"
#include "snprintf.h"

#include "xbestruct.h"

#include <string.h>

static ht_mask_ptable xbemagic[] = {
	{"magic",						STATICTAG_EDIT_CHAR("00000000")STATICTAG_EDIT_CHAR("00000001")STATICTAG_EDIT_CHAR("00000002")STATICTAG_EDIT_CHAR("00000003")},
	{0, 0}
};

static ht_tag_flags_s xbe_init_flags[] =
{
	{-1, "XBE - initialisation flags"},
	{0,  "[00] Mount Utility Drive"},
	{1,  "[01] Format Utility Drive"},
	{2,  "[02] Limit to 64MB"},
	{3,  "[03] Dont setup harddisk"},
	{0, 0}
};

static ht_mask_ptable xbeimageheader[] = {
	{"base address",				STATICTAG_EDIT_DWORD_LE("00000104")},
	{"size of headers",			STATICTAG_EDIT_DWORD_LE("00000108")},
	{"size of image",			STATICTAG_EDIT_DWORD_LE("0000010c")},
	{"size of imageheader",				STATICTAG_EDIT_DWORD_LE("00000110")},
	{"timestamp",				STATICTAG_EDIT_DWORD_LE("00000114")},
	{"certificate address",				STATICTAG_EDIT_DWORD_LE("00000118")},
	{"number of sections",				STATICTAG_EDIT_DWORD_LE("0000011c")},
	{"section header address",					STATICTAG_EDIT_DWORD_LE("00000120")},
	{"initialisation flags",			STATICTAG_EDIT_DWORD_LE("00000124")" "STATICTAG_FLAGS("00000124", ATOM_XBE_INIT_FLAGS_STR)},
	{"entry point",					STATICTAG_EDIT_DWORD_LE("00000128")},
	{"TLS address",					STATICTAG_EDIT_DWORD_LE("0000012c")},
	{"PE stack commit",					STATICTAG_EDIT_DWORD_LE("00000130")},
	{"PE heap reserve",					STATICTAG_EDIT_DWORD_LE("00000134")},
	{"PE heap commit",					STATICTAG_EDIT_DWORD_LE("00000138")},
	{"PE base address",					STATICTAG_EDIT_DWORD_LE("0000013c")},
	{"PE size of image",					STATICTAG_EDIT_DWORD_LE("00000140")},
	{"PE checksum",					STATICTAG_EDIT_DWORD_LE("00000144")},
	{"PE timestamp",					STATICTAG_EDIT_DWORD_LE("00000148")},
	{"debug pathname address",					STATICTAG_EDIT_DWORD_LE("0000014c")},
	{"debug filename address",					STATICTAG_EDIT_DWORD_LE("00000150")},
	{"debug unicode filename address",					STATICTAG_EDIT_DWORD_LE("00000154")},
	{"kernel image thunk address",					STATICTAG_EDIT_DWORD_LE("00000158")},
	{"non-kernel import directoy address",					STATICTAG_EDIT_DWORD_LE("0000015c")},
	{"number of library versions",					STATICTAG_EDIT_DWORD_LE("00000160")},
	{"library versions address",					STATICTAG_EDIT_DWORD_LE("00000164")},
	{"kernel library version",					STATICTAG_EDIT_DWORD_LE("00000168")},
	{"xapi library version",					STATICTAG_EDIT_DWORD_LE("0000016c")},
	{"logo bitmap address",					STATICTAG_EDIT_DWORD_LE("00000170")},
	{"logo bitmap size",					STATICTAG_EDIT_DWORD_LE("00000174")},
	{0, 0}
};

static ht_tag_flags_s xbe_media_flags[] =
{
	{-1, "XBE - allowed media flags"},
	{0,  "[00] Harddisk"},
	{1,  "[01] DVD X2"},
	{2,  "[02] DVD CD"},
	{3,  "[03] CD"},
	{4,  "[04] DVD-5 RO"},
	{5,  "[05] DVD-9 RO"},
	{6,  "[06] DVD-5 RW"},
	{7,  "[07] DVD-9 RW"},
	{8,  "[08] Dongle"},
	{9,  "[09] Mediaboard"},
	{30,  "[30] Nonsecure Harddisk"},
	{31,  "[31] Nonsecure Mode"},
	{0, 0}
};

static ht_tag_flags_s xbe_region_codes[] =
{
	{ -1, "XBE - game region codes"},
	{ 0,  "[00] USA"},
	{ 1,  "[01] Japan"},
	{ 2,  "[02] Rest of World"},
	{ 31,  "[31] Manufacturing" },
	{0, 0}
};


static ht_mask_ptable xbecertificate[] = {
	{"certificate size",				STATICTAG_EDIT_DWORD_LE("00000000")},
	{"timestamp",				STATICTAG_EDIT_DWORD_LE("00000004")},
	{"title id",				STATICTAG_EDIT_DWORD_LE("00000008")},
	{"alternate title ids",				STATICTAG_EDIT_DWORD_LE("0000005c") "  " STATICTAG_EDIT_DWORD_LE("00000060") "  " STATICTAG_EDIT_DWORD_LE("00000064")},
	{"",				STATICTAG_EDIT_DWORD_LE("00000068") "  " STATICTAG_EDIT_DWORD_LE("0000006c") "  " STATICTAG_EDIT_DWORD_LE("00000070")},
	{"",				STATICTAG_EDIT_DWORD_LE("00000074") "  " STATICTAG_EDIT_DWORD_LE("00000078") "  " STATICTAG_EDIT_DWORD_LE("0000007c")},
	{"",				STATICTAG_EDIT_DWORD_LE("00000080") "  " STATICTAG_EDIT_DWORD_LE("00000084") "  " STATICTAG_EDIT_DWORD_LE("00000088")},
	{"",				STATICTAG_EDIT_DWORD_LE("0000008c") "  " STATICTAG_EDIT_DWORD_LE("00000090") "  " STATICTAG_EDIT_DWORD_LE("00000094")},
	{"",				STATICTAG_EDIT_DWORD_LE("00000098")},
	{"allowed media",			STATICTAG_EDIT_DWORD_LE("0000009c")" ("STATICTAG_FLAGS("0000009c", ATOM_XBE_MEDIA_FLAGS_STR)")"},
	{"game region",				STATICTAG_EDIT_DWORD_LE("000000a0")" ("STATICTAG_FLAGS("000000a0", ATOM_XBE_REGION_STR)")"},
	{"game ratings",				STATICTAG_EDIT_DWORD_LE("000000a4")},
	{"disk number",				STATICTAG_EDIT_DWORD_LE("000000a8")},
	{"version",				STATICTAG_EDIT_DWORD_LE("000000ac")},
	{"lan key",			STATICTAG_EDIT_BYTE("000000b0") STATICTAG_EDIT_BYTE("000000b1") STATICTAG_EDIT_BYTE("000000b2") STATICTAG_EDIT_BYTE("000000b3") STATICTAG_EDIT_BYTE("000000b4") STATICTAG_EDIT_BYTE("000000b5") STATICTAG_EDIT_BYTE("000000b6") STATICTAG_EDIT_BYTE("000000b7") STATICTAG_EDIT_BYTE("000000b8") STATICTAG_EDIT_BYTE("000000b9") STATICTAG_EDIT_BYTE("000000ba") STATICTAG_EDIT_BYTE("000000bb") STATICTAG_EDIT_BYTE("000000bc") STATICTAG_EDIT_BYTE("000000bd") STATICTAG_EDIT_BYTE("000000be") STATICTAG_EDIT_BYTE("000000bf")},
	{"signature key",			STATICTAG_EDIT_BYTE("000000c0") STATICTAG_EDIT_BYTE("000000c1") STATICTAG_EDIT_BYTE("000000c2") STATICTAG_EDIT_BYTE("000000c3") STATICTAG_EDIT_BYTE("000000c4") STATICTAG_EDIT_BYTE("000000c5") STATICTAG_EDIT_BYTE("000000c6") STATICTAG_EDIT_BYTE("000000c7") STATICTAG_EDIT_BYTE("000000c8") STATICTAG_EDIT_BYTE("000000c9") STATICTAG_EDIT_BYTE("000000ca") STATICTAG_EDIT_BYTE("000000cb") STATICTAG_EDIT_BYTE("000000cc") STATICTAG_EDIT_BYTE("000000cd") STATICTAG_EDIT_BYTE("000000ce") STATICTAG_EDIT_BYTE("000000cf")},
	{"alternate signature keys",			STATICTAG_EDIT_BYTE("000000d0") STATICTAG_EDIT_BYTE("000000d1") STATICTAG_EDIT_BYTE("000000d2") STATICTAG_EDIT_BYTE("000000d3") STATICTAG_EDIT_BYTE("000000d4") STATICTAG_EDIT_BYTE("000000d5") STATICTAG_EDIT_BYTE("000000d6") STATICTAG_EDIT_BYTE("000000d7") STATICTAG_EDIT_BYTE("000000d8") STATICTAG_EDIT_BYTE("000000d9") STATICTAG_EDIT_BYTE("000000da") STATICTAG_EDIT_BYTE("000000db") STATICTAG_EDIT_BYTE("000000dc") STATICTAG_EDIT_BYTE("000000dd") STATICTAG_EDIT_BYTE("000000de") STATICTAG_EDIT_BYTE("000000df")},
	{"",			STATICTAG_EDIT_BYTE("000000e0") STATICTAG_EDIT_BYTE("000000e1") STATICTAG_EDIT_BYTE("000000e2") STATICTAG_EDIT_BYTE("000000e3") STATICTAG_EDIT_BYTE("000000e4") STATICTAG_EDIT_BYTE("000000e5") STATICTAG_EDIT_BYTE("000000e6") STATICTAG_EDIT_BYTE("000000e7") STATICTAG_EDIT_BYTE("000000e8") STATICTAG_EDIT_BYTE("000000e9") STATICTAG_EDIT_BYTE("000000ea") STATICTAG_EDIT_BYTE("000000eb") STATICTAG_EDIT_BYTE("000000ec") STATICTAG_EDIT_BYTE("000000ed") STATICTAG_EDIT_BYTE("000000ee") STATICTAG_EDIT_BYTE("000000ef")},
	{"",			STATICTAG_EDIT_BYTE("000000f0") STATICTAG_EDIT_BYTE("000000f1") STATICTAG_EDIT_BYTE("000000f2") STATICTAG_EDIT_BYTE("000000f3") STATICTAG_EDIT_BYTE("000000f4") STATICTAG_EDIT_BYTE("000000f5") STATICTAG_EDIT_BYTE("000000f6") STATICTAG_EDIT_BYTE("000000f7") STATICTAG_EDIT_BYTE("000000f8") STATICTAG_EDIT_BYTE("000000f9") STATICTAG_EDIT_BYTE("000000fa") STATICTAG_EDIT_BYTE("000000fb") STATICTAG_EDIT_BYTE("000000fc") STATICTAG_EDIT_BYTE("000000fd") STATICTAG_EDIT_BYTE("000000fe") STATICTAG_EDIT_BYTE("000000ff")},
	{"",			STATICTAG_EDIT_BYTE("00000100") STATICTAG_EDIT_BYTE("00000101") STATICTAG_EDIT_BYTE("00000102") STATICTAG_EDIT_BYTE("00000103") STATICTAG_EDIT_BYTE("00000104") STATICTAG_EDIT_BYTE("00000105") STATICTAG_EDIT_BYTE("00000106") STATICTAG_EDIT_BYTE("00000107") STATICTAG_EDIT_BYTE("00000108") STATICTAG_EDIT_BYTE("00000109") STATICTAG_EDIT_BYTE("0000010a") STATICTAG_EDIT_BYTE("0000010b") STATICTAG_EDIT_BYTE("0000010c") STATICTAG_EDIT_BYTE("0000010d") STATICTAG_EDIT_BYTE("0000010e") STATICTAG_EDIT_BYTE("0000010f")},
	{"",			STATICTAG_EDIT_BYTE("00000110") STATICTAG_EDIT_BYTE("00000111") STATICTAG_EDIT_BYTE("00000112") STATICTAG_EDIT_BYTE("00000113") STATICTAG_EDIT_BYTE("00000114") STATICTAG_EDIT_BYTE("00000115") STATICTAG_EDIT_BYTE("00000116") STATICTAG_EDIT_BYTE("00000117") STATICTAG_EDIT_BYTE("00000118") STATICTAG_EDIT_BYTE("00000119") STATICTAG_EDIT_BYTE("0000011a") STATICTAG_EDIT_BYTE("0000011b") STATICTAG_EDIT_BYTE("0000011c") STATICTAG_EDIT_BYTE("0000011d") STATICTAG_EDIT_BYTE("0000011e") STATICTAG_EDIT_BYTE("0000011f")},
	{"",			STATICTAG_EDIT_BYTE("00000120") STATICTAG_EDIT_BYTE("00000121") STATICTAG_EDIT_BYTE("00000122") STATICTAG_EDIT_BYTE("00000123") STATICTAG_EDIT_BYTE("00000124") STATICTAG_EDIT_BYTE("00000125") STATICTAG_EDIT_BYTE("00000126") STATICTAG_EDIT_BYTE("00000127") STATICTAG_EDIT_BYTE("00000128") STATICTAG_EDIT_BYTE("00000129") STATICTAG_EDIT_BYTE("0000012a") STATICTAG_EDIT_BYTE("0000012b") STATICTAG_EDIT_BYTE("0000012c") STATICTAG_EDIT_BYTE("0000012d") STATICTAG_EDIT_BYTE("0000012e") STATICTAG_EDIT_BYTE("0000012f")},
	{"",			STATICTAG_EDIT_BYTE("00000130") STATICTAG_EDIT_BYTE("00000131") STATICTAG_EDIT_BYTE("00000132") STATICTAG_EDIT_BYTE("00000133") STATICTAG_EDIT_BYTE("00000134") STATICTAG_EDIT_BYTE("00000135") STATICTAG_EDIT_BYTE("00000136") STATICTAG_EDIT_BYTE("00000137") STATICTAG_EDIT_BYTE("00000138") STATICTAG_EDIT_BYTE("00000139") STATICTAG_EDIT_BYTE("0000013a") STATICTAG_EDIT_BYTE("0000013b") STATICTAG_EDIT_BYTE("0000013c") STATICTAG_EDIT_BYTE("0000013d") STATICTAG_EDIT_BYTE("0000013e") STATICTAG_EDIT_BYTE("0000013f")},
	{"",			STATICTAG_EDIT_BYTE("00000140") STATICTAG_EDIT_BYTE("00000141") STATICTAG_EDIT_BYTE("00000142") STATICTAG_EDIT_BYTE("00000143") STATICTAG_EDIT_BYTE("00000144") STATICTAG_EDIT_BYTE("00000145") STATICTAG_EDIT_BYTE("00000146") STATICTAG_EDIT_BYTE("00000147") STATICTAG_EDIT_BYTE("00000148") STATICTAG_EDIT_BYTE("00000149") STATICTAG_EDIT_BYTE("0000014a") STATICTAG_EDIT_BYTE("0000014b") STATICTAG_EDIT_BYTE("0000014c") STATICTAG_EDIT_BYTE("0000014d") STATICTAG_EDIT_BYTE("0000014e") STATICTAG_EDIT_BYTE("0000014f")},
	{"",			STATICTAG_EDIT_BYTE("00000150") STATICTAG_EDIT_BYTE("00000151") STATICTAG_EDIT_BYTE("00000152") STATICTAG_EDIT_BYTE("00000153") STATICTAG_EDIT_BYTE("00000154") STATICTAG_EDIT_BYTE("00000155") STATICTAG_EDIT_BYTE("00000156") STATICTAG_EDIT_BYTE("00000157") STATICTAG_EDIT_BYTE("00000158") STATICTAG_EDIT_BYTE("00000159") STATICTAG_EDIT_BYTE("0000015a") STATICTAG_EDIT_BYTE("0000015b") STATICTAG_EDIT_BYTE("0000015c") STATICTAG_EDIT_BYTE("0000015d") STATICTAG_EDIT_BYTE("0000015e") STATICTAG_EDIT_BYTE("0000015f")},
	{"",			STATICTAG_EDIT_BYTE("00000160") STATICTAG_EDIT_BYTE("00000161") STATICTAG_EDIT_BYTE("00000162") STATICTAG_EDIT_BYTE("00000163") STATICTAG_EDIT_BYTE("00000164") STATICTAG_EDIT_BYTE("00000165") STATICTAG_EDIT_BYTE("00000166") STATICTAG_EDIT_BYTE("00000167") STATICTAG_EDIT_BYTE("00000168") STATICTAG_EDIT_BYTE("00000169") STATICTAG_EDIT_BYTE("0000016a") STATICTAG_EDIT_BYTE("0000016b") STATICTAG_EDIT_BYTE("0000016c") STATICTAG_EDIT_BYTE("0000016d") STATICTAG_EDIT_BYTE("0000016e") STATICTAG_EDIT_BYTE("0000016f")},
	{"",			STATICTAG_EDIT_BYTE("00000170") STATICTAG_EDIT_BYTE("00000171") STATICTAG_EDIT_BYTE("00000172") STATICTAG_EDIT_BYTE("00000173") STATICTAG_EDIT_BYTE("00000174") STATICTAG_EDIT_BYTE("00000175") STATICTAG_EDIT_BYTE("00000176") STATICTAG_EDIT_BYTE("00000177") STATICTAG_EDIT_BYTE("00000178") STATICTAG_EDIT_BYTE("00000179") STATICTAG_EDIT_BYTE("0000017a") STATICTAG_EDIT_BYTE("0000017b") STATICTAG_EDIT_BYTE("0000017c") STATICTAG_EDIT_BYTE("0000017d") STATICTAG_EDIT_BYTE("0000017e") STATICTAG_EDIT_BYTE("0000017f")},
	{"",			STATICTAG_EDIT_BYTE("00000180") STATICTAG_EDIT_BYTE("00000181") STATICTAG_EDIT_BYTE("00000182") STATICTAG_EDIT_BYTE("00000183") STATICTAG_EDIT_BYTE("00000184") STATICTAG_EDIT_BYTE("00000185") STATICTAG_EDIT_BYTE("00000186") STATICTAG_EDIT_BYTE("00000187") STATICTAG_EDIT_BYTE("00000188") STATICTAG_EDIT_BYTE("00000189") STATICTAG_EDIT_BYTE("0000018a") STATICTAG_EDIT_BYTE("0000018b") STATICTAG_EDIT_BYTE("0000018c") STATICTAG_EDIT_BYTE("0000018d") STATICTAG_EDIT_BYTE("0000018e") STATICTAG_EDIT_BYTE("0000018f")},
	{"",			STATICTAG_EDIT_BYTE("00000190") STATICTAG_EDIT_BYTE("00000191") STATICTAG_EDIT_BYTE("00000192") STATICTAG_EDIT_BYTE("00000193") STATICTAG_EDIT_BYTE("00000194") STATICTAG_EDIT_BYTE("00000195") STATICTAG_EDIT_BYTE("00000196") STATICTAG_EDIT_BYTE("00000197") STATICTAG_EDIT_BYTE("00000198") STATICTAG_EDIT_BYTE("00000199") STATICTAG_EDIT_BYTE("0000019a") STATICTAG_EDIT_BYTE("0000019b") STATICTAG_EDIT_BYTE("0000019c") STATICTAG_EDIT_BYTE("0000019d") STATICTAG_EDIT_BYTE("0000019e") STATICTAG_EDIT_BYTE("0000019")},
	{"",			STATICTAG_EDIT_BYTE("000001a0") STATICTAG_EDIT_BYTE("000001a1") STATICTAG_EDIT_BYTE("000001a2") STATICTAG_EDIT_BYTE("000001a3") STATICTAG_EDIT_BYTE("000001a4") STATICTAG_EDIT_BYTE("000001a5") STATICTAG_EDIT_BYTE("000001a6") STATICTAG_EDIT_BYTE("000001a7") STATICTAG_EDIT_BYTE("000001a8") STATICTAG_EDIT_BYTE("000001a9") STATICTAG_EDIT_BYTE("000001aa") STATICTAG_EDIT_BYTE("000001ab") STATICTAG_EDIT_BYTE("000001ac") STATICTAG_EDIT_BYTE("000001ad") STATICTAG_EDIT_BYTE("000001ae") STATICTAG_EDIT_BYTE("000001af")},
	{"",			STATICTAG_EDIT_BYTE("000001b0") STATICTAG_EDIT_BYTE("000001b1") STATICTAG_EDIT_BYTE("000001b2") STATICTAG_EDIT_BYTE("000001b3") STATICTAG_EDIT_BYTE("000001b4") STATICTAG_EDIT_BYTE("000001b5") STATICTAG_EDIT_BYTE("000001b6") STATICTAG_EDIT_BYTE("000001b7") STATICTAG_EDIT_BYTE("000001b8") STATICTAG_EDIT_BYTE("000001b9") STATICTAG_EDIT_BYTE("000001ba") STATICTAG_EDIT_BYTE("000001bb") STATICTAG_EDIT_BYTE("000001bc") STATICTAG_EDIT_BYTE("000001bd") STATICTAG_EDIT_BYTE("000001be") STATICTAG_EDIT_BYTE("000001bf")},
	{"",			STATICTAG_EDIT_BYTE("000001c0") STATICTAG_EDIT_BYTE("000001c1") STATICTAG_EDIT_BYTE("000001c2") STATICTAG_EDIT_BYTE("000001c3") STATICTAG_EDIT_BYTE("000001c4") STATICTAG_EDIT_BYTE("000001c5") STATICTAG_EDIT_BYTE("000001c6") STATICTAG_EDIT_BYTE("000001c7") STATICTAG_EDIT_BYTE("000001c8") STATICTAG_EDIT_BYTE("000001c9") STATICTAG_EDIT_BYTE("000001ca") STATICTAG_EDIT_BYTE("000001cb") STATICTAG_EDIT_BYTE("000001cc") STATICTAG_EDIT_BYTE("000001cd") STATICTAG_EDIT_BYTE("000001ce") STATICTAG_EDIT_BYTE("000001cf")},

	{0, 0}	
};

static ht_tag_flags_s xbe_section_flags[] =
{
	{-1, "XBE - section flags"},
	{0,  "[00] Writeable"},
	{1,  "[01] Preload"},
	{2,  "[02] Executable"},
	{3,  "[03] Inserted File"},
	{4,  "[04] Head Page Read Only"},
	{5,  "[05] Tail Page Read Only"},
	{0, 0}
};

static ht_mask_ptable xbesectionheader[] = {
	{"section flags",				STATICTAG_EDIT_DWORD_LE("00000000")" "STATICTAG_FLAGS("00000000", ATOM_XBE_SECTION_FLAGS_STR)},
	{"virtual address",			STATICTAG_EDIT_DWORD_LE("00000004")},
	{"virtual size",			STATICTAG_EDIT_DWORD_LE("00000008")},
	{"raw address",			STATICTAG_EDIT_DWORD_LE("0000000c")},
	{"raw size",			STATICTAG_EDIT_DWORD_LE("00000010")},
	{"section name address",			STATICTAG_EDIT_DWORD_LE("00000014")},
	{"section name reference counter",			STATICTAG_EDIT_DWORD_LE("00000018")},
	{"head shared page reference counter address",		STATICTAG_EDIT_DWORD_LE("0000001c")},
	{"tail shared page reference counter address",			STATICTAG_EDIT_DWORD_LE("00000020")},
	{0, 0}
};

static ht_tag_flags_s xbe_library_flags[] =
{
	{-1, "XBE - library flags"},
	{15,  "[15] Debug Build"},
	{0, 0}
};


static ht_mask_ptable xbelibraryversion[] = {
	{"library name",			STATICTAG_EDIT_CHAR("00000000")STATICTAG_EDIT_CHAR("00000001")STATICTAG_EDIT_CHAR("00000002")STATICTAG_EDIT_CHAR("00000003")STATICTAG_EDIT_CHAR("00000004")STATICTAG_EDIT_CHAR("00000005")STATICTAG_EDIT_CHAR("00000006")STATICTAG_EDIT_CHAR("00000007")},
	{"major version",			STATICTAG_EDIT_WORD_LE("00000008")},
	{"minor version",			STATICTAG_EDIT_WORD_LE("0000000a")},
	{"build version",			STATICTAG_EDIT_WORD_LE("0000000c")},
	{"library flags",			STATICTAG_EDIT_WORD_LE("0000000e")" "STATICTAG_FLAGS("0000000a", ATOM_XBE_LIBRARY_FLAGS_STR)},
	{0, 0}
};

static ht_view *htxbeheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_xbe_shared_data *xbe_shared=(ht_xbe_shared_data *)group->get_shared_data();

	ht_xbe_header_viewer *v = new ht_xbe_header_viewer();
	v->init(b, DESC_XBE_HEADER, VC_EDIT | VC_SEARCH, file, group);

	registerAtom(ATOM_XBE_INIT_FLAGS, xbe_init_flags);
	registerAtom(ATOM_XBE_SECTION_FLAGS, xbe_section_flags);
	registerAtom(ATOM_XBE_MEDIA_FLAGS, xbe_media_flags);
	registerAtom(ATOM_XBE_REGION, xbe_region_codes);
	registerAtom(ATOM_XBE_LIBRARY_FLAGS, xbe_library_flags);

	ht_mask_sub *s;
	ht_collapsable_sub *cs;
	
	s = new ht_mask_sub();
	s->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* XBE header");
	s->add_mask(info);
	v->insertsub(s);

	/* FIXME: */
	bool xbe_bigendian = false;
	
	s = new ht_mask_sub();
	s->init(file, 1);
	s->add_staticmask_ptable(xbemagic, 0x0, xbe_bigendian);
	
	/* image header */
	s->add_staticmask_ptable(xbeimageheader, 0x0, xbe_bigendian);
	cs = new ht_collapsable_sub();
	cs->init(file, s, 1, "image header", 1);
	v->insertsub(cs);

	/* image header */
	s = new ht_mask_sub();
	s->init(file, 2);
	s->add_staticmask_ptable(xbecertificate, xbe_shared->header.certificate_address-xbe_shared->header.base_address, xbe_bigendian);
	cs = new ht_collapsable_sub();
	cs->init(file, s, 1, "certificate", 1);
	v->insertsub(cs);
	
	/* library versions */
	
	for (uint i=0; i < xbe_shared->header.number_of_library_versions; i++) {
		s = new ht_mask_sub();
		s->init(file, 50+i);

		s->add_staticmask_ptable(xbelibraryversion, xbe_shared->header.library_versions_address-xbe_shared->header.base_address+i*sizeof *xbe_shared->libraries, xbe_bigendian);

		char t[256];
		ht_snprintf(t, sizeof t, "library %d: %-9s %d.%d.%d", i, &xbe_shared->libraries[i].library_name, xbe_shared->libraries[i].major_version, xbe_shared->libraries[i].minor_version, xbe_shared->libraries[i].build_version);

		cs=new ht_collapsable_sub();
		cs->init(file, s, 1, t, 1);
	
		v->insertsub(cs);
	}
	
	/* section headers */

	for (uint i=0; i<xbe_shared->sections.number_of_sections; i++) {
		const char *name;
//		uint ofs;
	
		s=new ht_mask_sub();
		s->init(file, 100+i);

		s->add_staticmask_ptable(xbesectionheader, xbe_shared->header.section_header_address-xbe_shared->header.base_address+i*sizeof *xbe_shared->sections.sections, xbe_bigendian);

		if (xbe_shared->sections.sections[i].section_name_address) {
		
			name = (char *)xbe_shared->sections.sections[i].section_name_address;

		} else {
			name = "<empty>";
		}

		char t[256];
		ht_snprintf(t, sizeof t, "section header %d: %s - rva %08x vsize %08x", i, name, xbe_shared->sections.sections[i].virtual_address, xbe_shared->sections.sections[i].virtual_size);

		cs=new ht_collapsable_sub();
		cs->init(file, s, 1, t, 1);
	
		v->insertsub(cs);
	}
	return v;
}

format_viewer_if htxbeheader_if = {
	htxbeheader_init,
	0
};

/*
 *	CLASS ht_pe_header_viewer
 */

void ht_xbe_header_viewer::init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *group)
{
	ht_uformat_viewer::init(b, desc, caps, file, group);
	VIEW_DEBUG_NAME("ht_xbe_header_viewer");
}

/*
static ht_format_viewer *find_hex_viewer(ht_group *group)
{
	// FIXME: God forgive us...
	ht_group *vr_group=group;
	while (strcmp(vr_group->desc, VIEWERGROUP_NAME)) vr_group=vr_group->group;
	ht_view *c=vr_group->getfirstchild();
	while (c) {
		if (c->desc && (strcmp(c->desc, DESC_HEX)==0)) {
			return (ht_format_viewer*)c;
		}
		c=c->next;
	}
	return NULL;
}
*/

bool ht_xbe_header_viewer::ref_sel(LINE_ID *id)
{
	return true;
}
