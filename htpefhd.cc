/* 
 *	HT Editor
 *	htpefhd.cc
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

#include "pefstruc.h"
#include "atom.h"
#include "htpef.h"
#include "htpefhd.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable pef_header[]=
{
	{"tag1",		STATICTAG_EDIT_BYTE("00000000")" "
				STATICTAG_EDIT_BYTE("00000001")" "
				STATICTAG_EDIT_BYTE("00000002")" "
				STATICTAG_EDIT_BYTE("00000003")" = "
				STATICTAG_EDIT_CHAR("00000000")
				STATICTAG_EDIT_CHAR("00000001")
				STATICTAG_EDIT_CHAR("00000002")
				STATICTAG_EDIT_CHAR("00000003")},
	{"tag2",		STATICTAG_EDIT_BYTE("00000004")" "
				STATICTAG_EDIT_BYTE("00000005")" "
				STATICTAG_EDIT_BYTE("00000006")" "
				STATICTAG_EDIT_BYTE("00000007")" = "
				STATICTAG_EDIT_CHAR("00000004")
				STATICTAG_EDIT_CHAR("00000005")
				STATICTAG_EDIT_CHAR("00000006")
				STATICTAG_EDIT_CHAR("00000007")},
	{"architecture",	STATICTAG_EDIT_DWORD_BE("00000008")" = "
				STATICTAG_EDIT_CHAR("00000008")
				STATICTAG_EDIT_CHAR("00000009")
				STATICTAG_EDIT_CHAR("0000000a")
				STATICTAG_EDIT_CHAR("0000000b")" "
				STATICTAG_DESC_DWORD_BE("00000008", ATOM_PEF_ARCH_STR)},
	{"format version",	STATICTAG_EDIT_DWORD_BE("0000000c")},
	{"timestamp",		STATICTAG_EDIT_DWORD_BE("00000010")},
	{"oldDefVersion",	STATICTAG_EDIT_DWORD_BE("00000014")},
	{"oldImpVersion",	STATICTAG_EDIT_DWORD_BE("00000018")},
	{"currentVersion",	STATICTAG_EDIT_DWORD_BE("0000001c")},
	{"sectionCount",	STATICTAG_EDIT_WORD_BE("00000020")},
	{"instSectionCount",	STATICTAG_EDIT_WORD_BE("00000022")},
	{"reserved",		STATICTAG_EDIT_DWORD_BE("00000024")},
	{0, 0}
};

static ht_mask_ptable pef_sectionheader[]=
{
	{"nameofs",		STATICTAG_EDIT_DWORD_BE("00000000")},
	{"default address",	STATICTAG_EDIT_DWORD_BE("00000004")},
	{"total size",		STATICTAG_EDIT_DWORD_BE("00000008")},
	{"unpacked size",	STATICTAG_EDIT_DWORD_BE("0000000c")},
	{"packed size",		STATICTAG_EDIT_DWORD_BE("00000010")},
	{"container offset",	STATICTAG_EDIT_DWORD_BE("00000014")},
	{"section kind",	STATICTAG_EDIT_BYTE("00000018")" "STATICTAG_DESC_BYTE("00000018", ATOM_PEF_SECTION_KIND_STR)},
	{"share kind",		STATICTAG_EDIT_BYTE("00000019")" "STATICTAG_DESC_BYTE("00000019", ATOM_PEF_SHARE_KIND_STR)},
	{"alignment",		STATICTAG_EDIT_BYTE("0000001a")},
	{"reserved",		STATICTAG_EDIT_BYTE("0000001b")},
	{0, 0}
};

static int_hash pef_arch[] =
{
	{0x70777063,		"PowerPC"},
	{0x6d36386b,		"M68K"},
	{0, 0}
};

static int_hash pef_sectionKind[] =
{
	{PEF_SK_Code,		"code"},
	{PEF_SK_UnpackedData,	"unpacked data"},
	{PEF_SK_PatternInitData,"pattern-initialized data"},
	{PEF_SK_ConstData,	"const data"},
	{PEF_SK_Loader,		"loader"},
	{PEF_SK_Debug,		"debug?"},
	{PEF_SK_ExecutableData,	"code/data"},
	{PEF_SK_Exception,	"exception?"},
	{PEF_SK_Traceback,	"traceback?"},
	{0, 0}
};

static int_hash pef_shareKind[] =
{
	{PEF_SHK_ProcessShare,		"process share"},
	{PEF_SHK_GlobalShare,		"global share"},
	{PEF_SHK_ProtectedShare,	"protected share"},
	{0, 0}
};

static ht_mask_ptable pef_loader_info_header[]=
{
	{"main section",		STATICTAG_EDIT_DWORD_BE("00000000")},
	{"main offset",			STATICTAG_EDIT_DWORD_BE("00000004")},
	{"init section",		STATICTAG_EDIT_DWORD_BE("00000008")},
	{"init offset",			STATICTAG_EDIT_DWORD_BE("0000000c")},
	{"term section",		STATICTAG_EDIT_DWORD_BE("00000010")},
	{"term offset",			STATICTAG_EDIT_DWORD_BE("00000014")},
	{"imported library count",	STATICTAG_EDIT_DWORD_BE("00000018")},
	{"total imported symbol count",	STATICTAG_EDIT_DWORD_BE("0000001c")},
	{"reloc section count",		STATICTAG_EDIT_DWORD_BE("00000020")},
	{"reloc section offset",	STATICTAG_EDIT_DWORD_BE("00000024")},
	{"load strings offset",		STATICTAG_EDIT_DWORD_BE("00000028")},
	{"export hash offset",		STATICTAG_EDIT_DWORD_BE("0000002c")},
	{"export hash table power",	STATICTAG_EDIT_DWORD_BE("00000030")},
	{"exported symbol count",	STATICTAG_EDIT_DWORD_BE("00000034")},
	{0, 0}
};

/*static ht_mask_ptable pef_loader_reloc_header[]=
{
	{""},
	{0, 0}
};*/

ht_view *htpefheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pef_shared_data *pef_shared = (ht_pef_shared_data *)group->get_shared_data();

	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_PEF_HEADER, VC_EDIT, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* PEF header at offset %08qx", pef_shared->header_ofs);
	registerAtom(ATOM_PEF_ARCH, pef_arch);
	registerAtom(ATOM_PEF_SHARE_KIND, pef_shareKind);
	registerAtom(ATOM_PEF_SECTION_KIND, pef_sectionKind);
	m->add_mask(info);
	m->add_staticmask_ptable(pef_header, pef_shared->header_ofs, true);
	v->insertsub(m);
	
	for (int i=0; i < pef_shared->contHeader.sectionCount; i++) {
		m = new ht_mask_sub();
		m->init(file, 1);
		ht_snprintf(info, sizeof info, "Section %d", i);
		m->add_staticmask_ptable(pef_sectionheader,
			pef_shared->header_ofs+sizeof(pef_shared->contHeader)
			+i*sizeof(PEF_SECTION_HEADER), true);
		ht_collapsable_sub *cs = new ht_collapsable_sub();
		cs->init(file, m, 1, info, 1);
		v->insertsub(cs);
	}

	if (pef_shared->loader_info_header_ofs) {
		m = new ht_mask_sub();
		m->init(file, 1);
		ht_snprintf(info, sizeof info, "Loader header at offset %08qx", pef_shared->loader_info_header_ofs);
		m->add_staticmask_ptable(pef_loader_info_header, pef_shared->loader_info_header_ofs, true);
		ht_collapsable_sub *cs = new ht_collapsable_sub();
		cs->init(file, m, 1, info, 1);
		v->insertsub(cs);
		/* relocation headers */
//		pef_shared->loader_info_header_ofs + sizeof pef_shared->loader_info_header
//			+ pef_shared->loader_info_header.importedLibraryCount*sizeof(PEF_ImportedLibrary)
	}
	
	return v;
}

format_viewer_if htpefheader_if = {
	htpefheader_init,
	0
};
