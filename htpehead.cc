/*
 *	HT Editor
 *	htpehead.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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
#include "htatom.h"
#include "htcoff.h"
#include "htctrl.h"
#include "htendian.h"
#include "hthex.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpe.h"
#include "htpehead.h"
#include "httag.h"
#include "htstring.h"
#include "snprintf.h"

#include "pestruct.h"

#include <string.h>

int_hash pe_optional_magics[] =
{
	{COFF_OPTMAGIC_ROMIMAGE, "ROM image"},
	{COFF_OPTMAGIC_PE32, "PE/PE32"},
	{COFF_OPTMAGIC_PE64, "PE32+/PE64"},
	{0, 0}
};

int_hash pe_subsystems[] =
{
	{0, "generic"},
	{PE_SUBSYSTEM_NATIVE, "native"},
	{PE_SUBSYSTEM_WINDOWS_GUI, "Windows GUI"},
	{PE_SUBSYSTEM_WINDOWS_CUI, "Windows CUI"},
	{PE_SUBSYSTEM_OS2_CUI, "OS/2 CUI"},
	{PE_SUBSYSTEM_POSIX_CUI, "POSIX CUI"},
	{9, "Windows CE GUI"},
	{10, "EFI"},
	{11, "EFI/boot"},
	{12, "EFI/runtime"},
	{0, 0}
};

ht_mask_ptable pemagic[] = {
	{"magic",						STATICTAG_EDIT_DWORD_LE("00000000")},
	{0, 0}
};

ht_mask_ptable pe32header[] = {
	{"optional magic",				STATICTAG_EDIT_WORD_LE("00000014")" "STATICTAG_DESC_WORD_LE("00000014", ATOM_PE_OPTIONAL_MAGICS_STR)},
	{"major linker version",			STATICTAG_EDIT_BYTE("00000016")},
	{"minor linker version",			STATICTAG_EDIT_BYTE("00000017")},
	{"size of code",				STATICTAG_EDIT_DWORD_LE("00000018")},
	{"size of data",				STATICTAG_EDIT_DWORD_LE("0000001c")},
	{"size of bss",				STATICTAG_EDIT_DWORD_LE("00000020")},
	{"entry point",				STATICTAG_EDIT_DWORD_LE("00000024")},
	{"code base",					STATICTAG_EDIT_DWORD_LE("00000028")},
	{"data base",					STATICTAG_EDIT_DWORD_LE("0000002c")},
	{0, 0}
};

ht_mask_ptable pe32header_nt[] = {
	{"image base",					STATICTAG_EDIT_DWORD_LE("00000030")},
	{"section alignment",			STATICTAG_EDIT_DWORD_LE("00000034")},
	{"file alignment",				STATICTAG_EDIT_DWORD_LE("00000038")},
	{"major OS version",			STATICTAG_EDIT_WORD_LE("0000003c")},
	{"minor OS version",			STATICTAG_EDIT_WORD_LE("0000003e")},
	{"major image version",			STATICTAG_EDIT_WORD_LE("00000040")},
	{"minor image version",			STATICTAG_EDIT_WORD_LE("00000042")},
	{"major subsystem version",		STATICTAG_EDIT_WORD_LE("00000044")},
	{"minor subsystem version",		STATICTAG_EDIT_WORD_LE("00000046")},
	{"Win32 version",				STATICTAG_EDIT_DWORD_LE("00000048")},
	{"size of image",				STATICTAG_EDIT_DWORD_LE("0000004c")},
	{"size of headers",				STATICTAG_EDIT_DWORD_LE("00000050")},
	{"checksum",					STATICTAG_EDIT_DWORD_LE("00000054")},
	{"subsystem",					STATICTAG_EDIT_WORD_LE("00000058")" "STATICTAG_DESC_WORD_LE("00000058", ATOM_PE_SUBSYSTEMS_STR)},
	{"dll characteristics",			STATICTAG_EDIT_WORD_LE("0000005a")},
	{"stack reserve",				STATICTAG_EDIT_DWORD_LE("0000005c")},
	{"stack commit",				STATICTAG_EDIT_DWORD_LE("00000060")},
	{"heap reserve",				STATICTAG_EDIT_DWORD_LE("00000064")},
	{"heap commit",				STATICTAG_EDIT_DWORD_LE("00000068")},
	{"loader flags",				STATICTAG_EDIT_DWORD_LE("0000006c")},
	{"number of directory entries",	STATICTAG_EDIT_DWORD_LE("00000070")},
	{0, 0}
};

ht_mask_ptable pe64header_nt[] = {
	{"image base",					STATICTAG_EDIT_QWORD_LE("0000002c")},
	{"section alignment",			STATICTAG_EDIT_DWORD_LE("00000034")},
	{"file alignment",				STATICTAG_EDIT_DWORD_LE("00000038")},
	{"major OS version",			STATICTAG_EDIT_WORD_LE("0000003c")},
	{"minor OS version",			STATICTAG_EDIT_WORD_LE("0000003e")},
	{"major image version",			STATICTAG_EDIT_WORD_LE("00000040")},
	{"minor image version",			STATICTAG_EDIT_WORD_LE("00000042")},
	{"major subsystem version",		STATICTAG_EDIT_WORD_LE("00000044")},
	{"minor subsystem version",		STATICTAG_EDIT_WORD_LE("00000046")},
	{"Win32 version",				STATICTAG_EDIT_DWORD_LE("00000048")},
	{"size of image",				STATICTAG_EDIT_DWORD_LE("0000004c")},
	{"size of headers",				STATICTAG_EDIT_DWORD_LE("00000050")},
	{"checksum",					STATICTAG_EDIT_DWORD_LE("00000054")},
	{"subsystem",					STATICTAG_EDIT_WORD_LE("00000058")" "STATICTAG_DESC_WORD_LE("00000058", ATOM_PE_SUBSYSTEMS_STR)},
	{"dll characteristics",			STATICTAG_EDIT_WORD_LE("0000005a")},
	{"stack reserve",				STATICTAG_EDIT_QWORD_LE("0000005c")},
	{"stack commit",				STATICTAG_EDIT_QWORD_LE("00000064")},
	{"heap reserve",				STATICTAG_EDIT_QWORD_LE("0000006c")},
	{"heap commit",				STATICTAG_EDIT_QWORD_LE("00000074")},
	{"loader flags",				STATICTAG_EDIT_DWORD_LE("0000007c")},
	{"number of directory entries",	STATICTAG_EDIT_DWORD_LE("00000080")},
	{0, 0}
};

ht_mask_ptable pe32header_nt_dirs[] = {
	{"export directory             (rva/size)", STATICTAG_EDIT_DWORD_LE("00000000")" "STATICTAG_EDIT_DWORD_LE("00000004")" "STATICTAG_REF("0000000000000000", "03", "raw")" "STATICTAG_REF("0000000100000000", "04", "cook")},
	{"import directory             (rva/size)", STATICTAG_EDIT_DWORD_LE("00000008")" "STATICTAG_EDIT_DWORD_LE("0000000c")" "STATICTAG_REF("0000000000000001", "03", "raw")" "STATICTAG_REF("0000000200000000", "04", "cook")},
	{"resource directory           (rva/size)", STATICTAG_EDIT_DWORD_LE("00000010")" "STATICTAG_EDIT_DWORD_LE("00000014")" "STATICTAG_REF("0000000000000002", "03", "raw")" "STATICTAG_REF("0000000300000000", "04", "cook")},
	{"exception directory          (rva/size)", STATICTAG_EDIT_DWORD_LE("00000018")" "STATICTAG_EDIT_DWORD_LE("0000001c")" "STATICTAG_REF("0000000000000003", "03", "raw")},
	{"security directory           (rva/size)", STATICTAG_EDIT_DWORD_LE("00000020")" "STATICTAG_EDIT_DWORD_LE("00000024")" "STATICTAG_REF("0000000000000004", "03", "raw")},
	{"base relocation table        (rva/size)", STATICTAG_EDIT_DWORD_LE("00000028")" "STATICTAG_EDIT_DWORD_LE("0000002c")" "STATICTAG_REF("0000000000000005", "03", "raw")},
	{"debug directory              (rva/size)", STATICTAG_EDIT_DWORD_LE("00000030")" "STATICTAG_EDIT_DWORD_LE("00000034")" "STATICTAG_REF("0000000000000006", "03", "raw")},
	{"description string           (rva/size)", STATICTAG_EDIT_DWORD_LE("00000038")" "STATICTAG_EDIT_DWORD_LE("0000003c")" "STATICTAG_REF("0000000000000007", "03", "raw")},
	{"machine value (GP)           (rva/size)", STATICTAG_EDIT_DWORD_LE("00000040")" "STATICTAG_EDIT_DWORD_LE("00000044")" "STATICTAG_REF("0000000000000008", "03", "raw")},
	{"thread local storage (TLS)   (rva/size)", STATICTAG_EDIT_DWORD_LE("00000048")" "STATICTAG_EDIT_DWORD_LE("0000004c")" "STATICTAG_REF("0000000000000009", "03", "raw")},
	{"load configuration directory (rva/size)", STATICTAG_EDIT_DWORD_LE("00000050")" "STATICTAG_EDIT_DWORD_LE("00000054")" "STATICTAG_REF("000000000000000a", "03", "raw")},
	{"bound import directory       (ofs/size)", STATICTAG_EDIT_DWORD_LE("00000058")" "STATICTAG_EDIT_DWORD_LE("0000005c")" "STATICTAG_REF("000000040000000b", "03", "raw")},
	{"import address table (IAT)   (rva/size)", STATICTAG_EDIT_DWORD_LE("00000060")" "STATICTAG_EDIT_DWORD_LE("00000064")" "STATICTAG_REF("000000000000000c", "03", "raw")},
	{"delay import descriptor      (rva/size)", STATICTAG_EDIT_DWORD_LE("00000068")" "STATICTAG_EDIT_DWORD_LE("0000006c")" "STATICTAG_REF("000000000000000d", "03", "raw")},
	{"COM+ runtime header          (rva/size)", STATICTAG_EDIT_DWORD_LE("00000070")" "STATICTAG_EDIT_DWORD_LE("00000074")" "STATICTAG_REF("000000000000000e", "03", "raw")" "STATICTAG_REF("0000000f00000000", "04", "cook")},
	{"reserved (15)                (rva/size)", STATICTAG_EDIT_DWORD_LE("00000078")" "STATICTAG_EDIT_DWORD_LE("0000007c")" "STATICTAG_REF("000000000000000f", "03", "raw")},
	{0, 0}
};

ht_view *htpeheader_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	FILEOFS h=pe_shared->header_ofs;
	ht_pe_header_viewer *v=new ht_pe_header_viewer();
	v->init(b, DESC_PE_HEADER, VC_EDIT | VC_SEARCH, file, group);
	register_atom(ATOM_COFF_MACHINES, coff_machines);
	register_atom(ATOM_COFF_CHARACTERISTICS, coff_characteristics);
	register_atom(ATOM_COFF_SECTION_CHARACTERISTICS, coff_section_characteristics);
	register_atom(ATOM_PE_OPTIONAL_MAGICS, pe_optional_magics);
	register_atom(ATOM_PE_SUBSYSTEMS, pe_subsystems);

	ht_mask_sub *s;
	ht_collapsable_sub *cs;
	
	s=new ht_mask_sub();
	s->init(file, 0);
	char info[128];
	sprintf(info, "* PE header at offset %08x", h);
	s->add_mask(info);
	v->insertsub(s);

	/* FIXME: */
	bool pe_bigendian = false;
	
	s=new ht_mask_sub();
	s->init(file, 1);
	s->add_staticmask_ptable(pemagic, h, pe_bigendian);
	
	/* COFF header */
	s->add_staticmask_ptable(coffheader, h+4, pe_bigendian);
	cs=new ht_collapsable_sub();
	cs->init(file, s, 1, "COFF header", 1);
	v->insertsub(cs);
	
	/* optional header */
	s=new ht_mask_sub();
	s->init(file, 2);
	word opt;
	file->seek(h+24);
	file->read(&opt, 2);
	opt = create_host_int(&opt, 2, little_endian);
	switch (opt) {
		case COFF_OPTMAGIC_PE32:
		case COFF_OPTMAGIC_PE64: {
			bool pe32 = (opt==COFF_OPTMAGIC_PE32);
			s->add_staticmask_ptable(pe32header, h+4, pe_bigendian);
			cs=new ht_collapsable_sub();
			cs->init(file, s, 1, "optional header", 1);
			v->insertsub(cs);
			
			s=new ht_mask_sub();
			s->init(file, 3);
			s->add_staticmask_ptable(pe32?pe32header_nt:pe64header_nt, h+4, pe_bigendian);
			cs=new ht_collapsable_sub();
			cs->init(file, s, 1, "optional header: NT fields", 1);
			v->insertsub(cs);
			
			s=new ht_mask_sub();
			s->init(file, 4);
			s->add_staticmask_ptable(pe32header_nt_dirs, h+4+(pe32?0x74:0x84), pe_bigendian);
			cs=new ht_collapsable_sub();
			cs->init(file, s, 1, "optional header: directories", 1);
			v->insertsub(cs);
			break;
		}
		default: {
			s->add_staticmask("optional magic                                   "STATICTAG_EDIT_WORD_LE("00000014")" "STATICTAG_DESC_WORD_LE("00000014", ATOM_PE_OPTIONAL_MAGICS_STR), h+4, pe_bigendian);
			s->add_mask("-------------------------------------------------------------------------");
			s->add_mask("Unsupported optional magic ! If you get this message in an original");
			s->add_mask("(unmodified) file, please contact us (see help).");
			cs=new ht_collapsable_sub();
			cs->init(file, s, 1, "optional header", 1);
			v->insertsub(cs);
		}
	}
	
	/* section headers */
	
	for (UINT i=0; i<pe_shared->sections.section_count; i++) {
		s=new ht_mask_sub();
		s->init(file, 100+i);

		s->add_staticmask_ptable(coff_section, h+24+pe_shared->coffheader.optional_header_size+i*COFF_SIZEOF_SECTION_HEADER, pe_bigendian);

		char nm[9];
		memmove(nm, pe_shared->sections.sections[i].name, 8);
		nm[8]=0;

		char t[256];
		ht_snprintf(t, sizeof t, "section header %d: %-8s rva %08x vsize %08x", i, nm, pe_shared->sections.sections[i].data_address, pe_shared->sections.sections[i].data_vsize);

		cs=new ht_collapsable_sub();
		cs->init(file, s, 1, t, 1);
	
		v->insertsub(cs);
	}
	return v;
}

format_viewer_if htpeheader_if = {
	htpeheader_init,
	0
};

/*
 *	CLASS ht_pe_header_viewer
 */

void ht_pe_header_viewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *group)
{
	ht_uformat_viewer::init(b, desc, caps, file, group);
	VIEW_DEBUG_NAME("ht_pe_header_viewer");
}

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

int ht_pe_header_viewer::ref_sel(LINE_ID *id)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();
	switch (id->id1) {
		case 0: {
			ht_format_viewer *hexv = find_hex_viewer(group);
			if (hexv) {
				UINT rva;
				UINT size;
				if (pe_shared->opt_magic == COFF_OPTMAGIC_PE32) {
					rva = pe_shared->pe32.header_nt.directory[id->id2].address;
					size = pe_shared->pe32.header_nt.directory[id->id2].size;
				} else {
					rva = pe_shared->pe64.header_nt.directory[id->id2].address;
					size = pe_shared->pe64.header_nt.directory[id->id2].size;
				}
				FILEOFS ofs = 0;
				if (pe_rva_to_ofs(&pe_shared->sections, rva, &ofs)) {
					vstate_save();
					hexv->goto_offset(ofs, false);
					hexv->pselect_set(ofs, ofs+size);
					app->focus(hexv);
				} else errorbox("Can't follow: directory RVA %08x is not valid !", rva);
			}
			break;
		}			
		case 1:
			if (pe_shared->v_exports) {
				vstate_save();
				app->focus(pe_shared->v_exports);
			}
			break;
		case 2:
			if (pe_shared->v_imports) {
				vstate_save();
				app->focus(pe_shared->v_imports);
			}
			break;
		case 3:
			if (pe_shared->v_resources) {
				vstate_save();
				app->focus(pe_shared->v_resources);
			}
			break;
		case 4: {
			ht_format_viewer *hexv = find_hex_viewer(group);
			if (hexv) {
				UINT ofs;
				UINT size;
				if (pe_shared->opt_magic == COFF_OPTMAGIC_PE32) {
					ofs = pe_shared->pe32.header_nt.directory[id->id2].address;
					size = pe_shared->pe32.header_nt.directory[id->id2].size;
				} else {
					ofs = pe_shared->pe64.header_nt.directory[id->id2].address;
					size = pe_shared->pe64.header_nt.directory[id->id2].size;
				}
				if (size) {
					vstate_save();
					hexv->goto_offset(ofs, false);
					hexv->pselect_set(ofs, ofs+size);
					app->focus(hexv);
				} else errorbox("can't follow: No bound import directory!", ofs);
			}
			break;
		}
		case 15: {
			if (pe_shared->v_il) {
				vstate_save();
				app->focus(pe_shared->v_il);
			}
			break;
		}
	}
	return 1;
}

