/*
 *	HT Editor
 *	htcoffhd.cc
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
#include "htcoff.h"
#include "htcoffhd.h"
#include "httag.h"
#include "strtools.h"
#include "formats.h"
#include "snprintf.h"

#include <string.h>

int_hash coff_machines[] =
{
//	{COFF_MACHINE_UNKNOWN, "generic"},
	{COFF_MACHINE_I386, "Intel 386"},
	{COFF_MACHINE_I486, "Intel 486"},
	{COFF_MACHINE_I586, "Intel 586"},
	{COFF_MACHINE_R3000BE, "R3000(be)"},
	{COFF_MACHINE_R3000, "R3000"},
	{COFF_MACHINE_R4000, "R4000"},
	{COFF_MACHINE_R10000, "R10000"},
	{COFF_MACHINE_ALPHA, "Alpha AXP"},
	{COFF_MACHINE_SH3, "Hitachi SH3"},
	{COFF_MACHINE_SH4, "Hitachi SH4"},
	{COFF_MACHINE_ARM, "ARM"},
	{COFF_MACHINE_THUMB, "THUMB"},
	{COFF_MACHINE_POWERPC_LE, "PowerPC (little-endian)"},
	{COFF_MACHINE_POWERPC_BE, "PowerPC (big-endian)"},
	{COFF_MACHINE_POWERPC64_BE, "PowerPC64 (big-endian)"},
	{COFF_MACHINE_IA64, "Intel IA64"},
	{COFF_MACHINE_MIPS16, "MIPS16"},
	{COFF_MACHINE_68k, "Motorola 68k"},
	{COFF_MACHINE_ALPHA_AXP_64, "Alpha AXP 64"},
	{COFF_MACHINE_MIPSf, "MIPSf"},
	{COFF_MACHINE_MIPS16f, "MIPS16f"},
	{COFF_MACHINE_AMD64, "x86-64 (AMD Opteron)"},
	{0, 0}
};

static int_hash coff_optional_magics[] =
{
	{COFF_OPTMAGIC_ROMIMAGE, "ROM image"},
	{COFF_OPTMAGIC_COFF32, "COFF"},
	{0, 0}
};

static int_hash coff_optional_sizes[] =
{
	{COFF_OPTSIZE_COFF32, "COFF32"},
	{COFF_OPTSIZE_XCOFF32, "XCOFF32"},
	{0, 0}
};

ht_tag_flags_s coff_characteristics[] =
{
	{-1, "COFF - file characteristics"},
	{0,  "[00] relocations stripped"},
	{1,  "[01] file is executable"},
	{2,  "[02] line numbers stripped"},
	{3,  "[03] local symbols stripped"},
	{4,  "[04] aggressively trim working set"},
	{5,  "[05] >2 GiB addresses aware"},
	{6,  "[06] * reserved"},
	{7,  "[07] low bytes of machine uint16 are reversed"},
	{8,  "[08] 32 bit machine"},
	{9,  "[09] debugging information stripped"},
	{10, "[10] run from swap if image on removable media"},
	{11, "[11] run from swap if image is on net"},
	{12, "[12] system file"},
	{13, "[13] file is dynamic link library (dll)"},
	{14, "[14] single processor (UP) only"},
	{15, "[15] high bytes of machine uint16 are reversed"},
	{0, 0}
};

ht_tag_flags_s coff_section_characteristics[] =
{
	{-1, "COFF - section characteristics"},
	{0,  "[00] * reserved"},
	{1,  "[01] * reserved"},
	{2,  "[02] * reserved"},
	{3,  "[03] * reserved"},
	{4,  "[04] * reserved"},
	{5,  "[05] code"},
	{6,  "[06] initialized"},
	{7,  "[07] uninitialized"},
	{8,  "[08] * reserved"},
	{9,  "[09] * reserved"},
	{10, "[10] * reserved"},
	{11, "[11] link remove"},
	{12, "[12] comdat"},
	{13, "[13] * reserved"},
	{14, "[14] * reserved (obsolete - protected)"},
	{15, "[15] fardata"},
	{16, "[16] * reserved (obsolete - sysheap)"},
	{17, "[17] purgeable/16bit?"},
	{18, "[18] locked"},
	{19, "[19] preload"},
	{20, "[20] * reserved"},
	{21, "[21] * reserved"},
	{22, "[22] * reserved"},
	{23, "[23] * reserved"},
	{24, "[24] extended relocations"},
	{25, "[25] discardable"},
	{26, "[26] not cachable"},
	{27, "[27] not pageable"},
	{28, "[28] shareable"},
	{29, "[29] executable"},
	{30, "[30] readable"},
	{31, "[31] writable"},
	{0, 0}
};

ht_mask_ptable coffheader[]=
{
	{"machine",			STATICTAG_EDIT_WORD_VE("00000000")" "STATICTAG_DESC_WORD_VE("00000000", ATOM_COFF_MACHINES_STR)},
	{"number of sections",		STATICTAG_EDIT_WORD_VE("00000002")},
	{"time-date stamp",		STATICTAG_EDIT_TIME_VE("00000004")},
	{"pointer to symbol table",	STATICTAG_EDIT_DWORD_VE("00000008")},
	{"number of symbols",		STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"size of optional header",	STATICTAG_EDIT_WORD_VE("00000010")" "STATICTAG_DESC_WORD_VE("00000010", ATOM_COFF_OPTIONAL_SIZES_STR)},
	{"characteristics",		STATICTAG_EDIT_WORD_VE("00000012")" "STATICTAG_FLAGS("00000012", ATOM_COFF_CHARACTERISTICS_STR)},
	{0, 0}
};

ht_mask_ptable coff32header[] = {
	{"optional magic",		STATICTAG_EDIT_WORD_VE("00000014")" "STATICTAG_DESC_WORD_VE("00000014", ATOM_COFF_OPTIONAL_MAGICS_STR)},
	{"major linker version",	STATICTAG_EDIT_BYTE("00000016")},
	{"minor linker version",	STATICTAG_EDIT_BYTE("00000017")},
	{"size of code",		STATICTAG_EDIT_DWORD_VE("00000018")},
	{"size of data",		STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"size of bss",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"entry point",			STATICTAG_EDIT_DWORD_VE("00000024")},
	{"code base",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"data base",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{0, 0}
};

ht_mask_ptable xcoff32header[] = {
	{"optional magic",		STATICTAG_EDIT_WORD_VE("00000014")" "STATICTAG_DESC_WORD_VE("00000014", ATOM_COFF_OPTIONAL_MAGICS_STR)},
	{"major linker version",	STATICTAG_EDIT_BYTE("00000016")},
	{"minor linker version",	STATICTAG_EDIT_BYTE("00000017")},
	{"size of code",		STATICTAG_EDIT_DWORD_VE("00000018")},
	{"size of data",		STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"size of bss",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"address of entrypoint",	STATICTAG_EDIT_DWORD_VE("00000024")},
	{"code base",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"data base",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"address of TOC anchor",	STATICTAG_EDIT_DWORD_VE("00000030")},
	{"section of entrypoint",	STATICTAG_EDIT_WORD_VE("00000034")},
	{"section of .text",		STATICTAG_EDIT_WORD_VE("00000036")},
	{"section of .data",		STATICTAG_EDIT_WORD_VE("00000038")},
	{"section of TOC",		STATICTAG_EDIT_WORD_VE("0000003a")},
	{"section of loader data",	STATICTAG_EDIT_WORD_VE("0000003c")},
	{"section of .bss",		STATICTAG_EDIT_WORD_VE("0000003e")},
	{"max. alignment for .text",	STATICTAG_EDIT_WORD_VE("00000040")},
	{"max. alignment for .data",	STATICTAG_EDIT_WORD_VE("00000042")},
	{"module type field",		STATICTAG_EDIT_WORD_VE("00000044")},
	{"cpu type",			STATICTAG_EDIT_BYTE("00000046")},
	{"cpu type (reserved)",		STATICTAG_EDIT_BYTE("00000047")},
	{"max. stack size",		STATICTAG_EDIT_DWORD_VE("00000048")},
	{"max. data size",		STATICTAG_EDIT_DWORD_VE("0000004c")},
	{"reserved for debuggers",	STATICTAG_EDIT_DWORD_VE("00000050")},
	{"reserved field",		STATICTAG_EDIT_DWORD_VE("00000054")},
	{"reserved field",		STATICTAG_EDIT_DWORD_VE("00000058")},
	{0, 0}
};

ht_mask_ptable coff_section[] = {
	{"name",			STATICTAG_EDIT_CHAR("00000000") STATICTAG_EDIT_CHAR("00000001") STATICTAG_EDIT_CHAR("00000002") STATICTAG_EDIT_CHAR("00000003") STATICTAG_EDIT_CHAR("00000004") STATICTAG_EDIT_CHAR("00000005") STATICTAG_EDIT_CHAR("00000006") STATICTAG_EDIT_CHAR("00000007")},
	{"virt. size OR phys. address",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"virtual address",		STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"size",			STATICTAG_EDIT_DWORD_VE("00000010")},
	{"offset",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"relocation table offset",	STATICTAG_EDIT_DWORD_VE("00000018")},
	{"line number table offset",	STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"relocation count",		STATICTAG_EDIT_WORD_VE("00000020")},
	{"line number count",		STATICTAG_EDIT_WORD_VE("00000022")},
	{"characteristics",		STATICTAG_EDIT_DWORD_VE("00000024")" "STATICTAG_FLAGS("00000024", ATOM_COFF_SECTION_CHARACTERISTICS_STR)},
	{0, 0}
};

static ht_view *htcoffheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_coff_shared_data *coff_shared = (ht_coff_shared_data *)group->get_shared_data();
	bool coff_bigendian = coff_shared->endian == big_endian;

	uint32 h = coff_shared->hdr_ofs;
	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_COFF_HEADER, VC_EDIT, file, group);

	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	registerAtom(ATOM_COFF_MACHINES, coff_machines);
	registerAtom(ATOM_COFF_OPTIONAL_MAGICS, coff_optional_magics);
	registerAtom(ATOM_COFF_OPTIONAL_SIZES, coff_optional_sizes);
	registerAtom(ATOM_COFF_CHARACTERISTICS, coff_characteristics);
	registerAtom(ATOM_COFF_SECTION_CHARACTERISTICS, coff_section_characteristics);
	char info[128];
	ht_snprintf(info, sizeof info, "* COFF header at offset %08x", h);
	m->add_mask(info);

	/* COFF header */
	m->add_mask("--- COFF header ---");
	m->add_staticmask_ptable(coffheader, h, coff_bigendian);
	/* optional header */
	if (coff_shared->coffheader.optional_header_size >= 2) {
		m->add_mask("--- optional header ---");
		uint16 opt = coff_shared->opt_magic;
/*		file->seek(h+20);
		file->read(&opt, 2);*/
		switch (opt) {
			case COFF_OPTMAGIC_COFF32:
				switch (coff_shared->coffheader.optional_header_size) {
					case COFF_OPTSIZE_COFF32:
						m->add_staticmask_ptable(coff32header, h, coff_bigendian);
						break;
					case COFF_OPTSIZE_XCOFF32:
						m->add_staticmask_ptable(xcoff32header, h, coff_bigendian);
						break;
				}
				break;
			default: {
				m->add_staticmask("optional magic                                   "STATICTAG_EDIT_WORD_VE("00000018")" "STATICTAG_DESC_WORD_VE("00000018", ATOM_COFF_OPTIONAL_MAGICS_STR), h+20, coff_bigendian);
				m->add_mask("-------------------------------------------------------------------------");
				m->add_mask("Unsupported optional magic! If you get this message in an original");
				m->add_mask("(unmodified) file, please contact us (see help).");
			}
		}
	}
	/* section headers */
	int sc = coff_shared->sections.section_count;
	int os = coff_shared->coffheader.optional_header_size;
/*	file->seek(h+16);
	file->read(&os, 2);
	file->seek(h+os+20);*/
	v->insertsub(m);

	for (int i=0; i < sc; i++) {
		ht_mask_sub *n=new ht_mask_sub();
		n->init(file, i);

		n->add_staticmask_ptable(coff_section, h+20+os+i*40, coff_bigendian);

		char nm[9];
		memcpy(nm, coff_shared->sections.sections[i].name, 8);
		nm[8] = 0;

		char t[32];
		ht_snprintf(t, sizeof t, "section %d: %s", i, nm);

		ht_collapsable_sub *cn=new ht_collapsable_sub();
		cn->init(file, n, 1, t, 1);

		v->insertsub(cn);
	}
	return v;
}

format_viewer_if htcoffheader_if = {
	htcoffheader_init,
	0
};
