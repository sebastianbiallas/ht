/* 
 *	HT Editor
 *	htmachohd.cc
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

#include "machostruc.h"
#include "htatom.h"
#include "htmacho.h"
#include "htmachohd.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable machoheader[]=
{
	{"magic",		STATICTAG_EDIT_DWORD_BE("00000000")},
	{"cputype",		STATICTAG_EDIT_DWORD_BE("00000004")},
	{"cpusubtype",		STATICTAG_EDIT_DWORD_BE("00000008")},
	{"filetype",		STATICTAG_EDIT_DWORD_BE("0000000c")},
	{"number of cmds",	STATICTAG_EDIT_DWORD_BE("00000010")},
	{"size of cmds",	STATICTAG_EDIT_DWORD_BE("00000014")},
	{"flags",		STATICTAG_EDIT_DWORD_BE("00000018")},
	{0, 0}
};

static ht_mask_ptable macho_segment_header[]=
{
	{"cmd",			STATICTAG_EDIT_DWORD_BE("00000000")},
	{"cmdsize",		STATICTAG_EDIT_DWORD_BE("00000004")},
	{"name",
STATICTAG_EDIT_CHAR("00000008")STATICTAG_EDIT_CHAR("00000009")
STATICTAG_EDIT_CHAR("0000000a")STATICTAG_EDIT_CHAR("0000000b")
STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")
STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")
STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")
STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")
STATICTAG_EDIT_CHAR("00000014")STATICTAG_EDIT_CHAR("00000015")
STATICTAG_EDIT_CHAR("00000016")STATICTAG_EDIT_CHAR("00000017")
},
	{"virtual address",	STATICTAG_EDIT_DWORD_BE("00000018")},
	{"virtual size",	STATICTAG_EDIT_DWORD_BE("0000001c")},
	{"file offset",		STATICTAG_EDIT_DWORD_BE("00000020")},
	{"file size",		STATICTAG_EDIT_DWORD_BE("00000024")},
	{"max VM protection",	STATICTAG_EDIT_DWORD_BE("00000028")},
	{"init VM protection",	STATICTAG_EDIT_DWORD_BE("0000002c")},
	{"number of sections",	STATICTAG_EDIT_DWORD_BE("00000030")},
	{"flags",		STATICTAG_EDIT_DWORD_BE("00000034")},
	{0, 0}
};
/*
ht_mask_ptable elfheader[]=
{
	{"ident", 0},
	{"    magic",		STATICTAG_EDIT_BYTE("00000000")" "STATICTAG_EDIT_BYTE("00000001")" "STATICTAG_EDIT_BYTE("00000002")" "STATICTAG_EDIT_BYTE("00000003")" = "STATICTAG_EDIT_CHAR("00000000") STATICTAG_EDIT_CHAR("00000001") STATICTAG_EDIT_CHAR("00000002") STATICTAG_EDIT_CHAR("00000003")},
	{"    class",		STATICTAG_EDIT_BYTE("00000004")" ("STATICTAG_DESC_BYTE("00000004", ATOM_ELF_CLASS_STR)")"},
	{"    data",		STATICTAG_EDIT_BYTE("00000005")" ("STATICTAG_DESC_BYTE("00000005", ATOM_ELF_DATA_STR)")"},
	{"    version",	STATICTAG_EDIT_BYTE("00000006")},
	{"    OS ABI",		STATICTAG_EDIT_BYTE("00000007")" ("STATICTAG_DESC_BYTE("00000007", ATOM_ELF_OS_ABI_STR)")"},
	{"    version",	STATICTAG_EDIT_BYTE("00000008")},
	{"    reserved",	STATICTAG_EDIT_BYTE("00000009")" "STATICTAG_EDIT_BYTE("0000000a")" "STATICTAG_EDIT_BYTE("0000000b")" "STATICTAG_EDIT_BYTE("0000000c")" "STATICTAG_EDIT_BYTE("0000000d")" "STATICTAG_EDIT_BYTE("0000000e")" "STATICTAG_EDIT_BYTE("0000000f")},
	{0, 0}
};
	
ht_mask_ptable elfheader32[]=
{
	{"type",					STATICTAG_EDIT_WORD_VE("00000010")" ("STATICTAG_DESC_WORD_VE("00000010", ATOM_ELF_TYPE_STR)")"},
	{"machine",				STATICTAG_EDIT_WORD_VE("00000012")" ("STATICTAG_DESC_WORD_VE("00000012", ATOM_ELF_MACHINE_STR)")"},
	{"version",				STATICTAG_EDIT_DWORD_VE("00000014")},
	{"entrypoint",				STATICTAG_EDIT_DWORD_VE("00000018")},
	{"program header offset",	STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"section header offset",	STATICTAG_EDIT_DWORD_VE("00000020")},
	{"flags",					STATICTAG_EDIT_DWORD_VE("00000024")},
	{"elf header size",			STATICTAG_EDIT_WORD_VE("00000028")},
	{"program header entry size",	STATICTAG_EDIT_WORD_VE("0000002a")},
	{"program header count",		STATICTAG_EDIT_WORD_VE("0000002c")},
	{"section header entry size",	STATICTAG_EDIT_WORD_VE("0000002e")},
	{"section header count",		STATICTAG_EDIT_WORD_VE("00000030")},
	{"section header strtab section index",STATICTAG_EDIT_WORD_VE("00000032")},
	{0, 0}
};

ht_mask_ptable elfheader64[]=
{
	{"type",					STATICTAG_EDIT_WORD_VE("00000010")" ("STATICTAG_DESC_WORD_VE("00000010", ATOM_ELF_TYPE_STR)")"},
	{"machine",				STATICTAG_EDIT_WORD_VE("00000012")" ("STATICTAG_DESC_WORD_VE("00000012", ATOM_ELF_MACHINE_STR)")"},
	{"version",				STATICTAG_EDIT_DWORD_VE("00000014")},
	{"entrypoint",				STATICTAG_EDIT_QWORD_VE("00000018")},
	{"program header offset",	STATICTAG_EDIT_QWORD_VE("00000020")},
	{"section header offset",	STATICTAG_EDIT_QWORD_VE("00000028")},
	{"flags",					STATICTAG_EDIT_DWORD_VE("00000030")},
	{"elf header size",			STATICTAG_EDIT_WORD_VE("00000034")},
	{"program header entry size",	STATICTAG_EDIT_WORD_VE("00000036")},
	{"program header count",		STATICTAG_EDIT_WORD_VE("00000038")},
	{"section header entry size",	STATICTAG_EDIT_WORD_VE("0000003a")},
	{"section header count",		STATICTAG_EDIT_WORD_VE("0000003c")},
	{"section header strtab section index",STATICTAG_EDIT_WORD_VE("0000003e")},
	{0, 0}
};

int_hash elf_class[] =
{
	{ELFCLASSNONE, "invalid class type"},
	{ELFCLASS32, "32-bit objects"},
	{ELFCLASS64, "64-bit objects"},
	{0, 0}
};

int_hash elf_data[] =
{
	{ELFDATANONE, "invalid data encoding"},
	{ELFDATA2LSB, "LSB encoding"},
	{ELFDATA2MSB, "MSB encoding"},
	{0, 0}
};

int_hash elf_os_abi[] =
{
	{ELFOSABI_SYSV, "System V"},
	{ELFOSABI_HPUX, "HP-UX"},
	{ELFOSABI_LINUX, "Linux"},
	{ELFOSABI_NETBSD, "NetBSD"},
	{ELFOSABI_HURD, "GNU/Hurd"},
	{ELFOSABI_86OPEN, "86Open common IA32 ABI"},
	{ELFOSABI_SOLARIS, "Solaris"},
	{ELFOSABI_MONTEREY, "Monterey"},
	{ELFOSABI_IRIX, "IRIX"},
	{ELFOSABI_FREEBSD, "FreeBSD"},
	{ELFOSABI_TRU64, "TRU64 UNIX"},
	{ELFOSABI_MODESTO, "Novell Modesto"},
	{ELFOSABI_OPENBSD, "OpenBSD"},
	{ELFOSABI_ARM, "ARM"},
	{ELFOSABI_STANDALONE, "standalone"},
	{0, 0}
};

int_hash elf_type[] =
{
	{ELF_ET_NONE, "no file type"},
	{ELF_ET_REL, "relocatable file"},
	{ELF_ET_EXEC, "executable file"},
	{ELF_ET_DYN, "shared object file"},
	{ELF_ET_CORE, "core file"},
	{0, 0}
};

int_hash elf_machine[] =
{
	{ELF_EM_NONE,  	"no machine"},
	{ELF_EM_M32,   	"AT&T WE32100"},
	{ELF_EM_SPARC, 	"SPARC"},
	{ELF_EM_386,   	"Intel 80386"},
	{ELF_EM_68K,   	"Motorola 68000"},
	{ELF_EM_88K,   	"Motorola 88000"},
	{ELF_EM_486,		"Intel 80486"},
	{ELF_EM_860,   	"Intel 80860"},
	{ELF_EM_MIPS,		"MIPS R3000 big-endian"},
	{ELF_EM_S370,		"IBM System/370"},
	{ELF_EM_MIPS_RS4_BE,	"MIPS R4000 big-endian"},

	{ELF_EM_PARISC,	"HPPA"},

	{ELF_EM_VPP550,	"Fujitsu VPP500"},
	{ELF_EM_SPARC32PLUS,	"Sun's \"v8plus\""},
	{ELF_EM_960,		"Intel 80960"},
	{ELF_EM_PPC,		"PowerPC"},
	{ELF_EM_PPC64,		"64-bit PowerPC"},

	{ELF_EM_V800,		"NEC V800 series"},
	{ELF_EM_FR20,		"Fujitsu FR20"},
	{ELF_EM_RH32,		"TRW RH32"},
	{ELF_EM_MCORE,		"Motorola M*Core"},
	{ELF_EM_ARM,		"ARM"},
	{ELF_EM_OLD_ALPHA,	"Digital Alpha"},
	{ELF_EM_SH,		"Hitachi SH"},
	{ELF_EM_SPARCV9,	"SPARC v9 64-bit"},
	{ELF_EM_TRICORE,	"Siemens Tricore embedded processor"},
	{ELF_EM_ARC,		"Argonaut RISC Core, Argonaut Technologies Inc."},
	{ELF_EM_H8_300,	"Hitachi H8/300"},
	{ELF_EM_H8_300H,	"Hitachi H8/300H"},
	{ELF_EM_H8S,		"Hitachi H8S"},
	{ELF_EM_H8_500,	"Hitachi H8/500"},
	{ELF_EM_IA_64,		"Intel IA-64 Processor"},
	{ELF_EM_MIPS_X,	"Stanford MIPS-X"},
	{ELF_EM_COLDFIRE,	"Motorola Coldfire"},
	{ELF_EM_68HC12,	"Motorola M68HC12"},
	{ELF_EM_MMA,		"Fujitsu Multimedia Accelerator"},
	{ELF_EM_PCP,		"Siemens PCP"},
	{ELF_EM_NCPU,		"Sony nCPU embedded RISC processor"},
	{ELF_EM_NDR1,		"Denso NDR1 microprocesspr"},
	{ELF_EM_STARCORE,	"Motorola Star*Core processor"},
	{ELF_EM_ME16,		"Toyota ME16 processor"},
	{ELF_EM_ST100,		"STMicroelectronics ST100 processor"},
	{ELF_EM_TINYJ,		"Advanced Logic Corp. TinyJ embedded processor"},

	{ELF_EM_FX66,		"Siemens FX66 microcontroller"},
	{ELF_EM_ST9PLUS,	"STMicroelectronics ST9+ 8/16 bit microcontroller"},
	{ELF_EM_ST7,		"STMicroelectronics ST7 8-bit microcontroller"},
	{ELF_EM_68HC16,	"Motorola MC68HC16 Microcontroller"},
	{ELF_EM_68HC11,	"Motorola MC68HC11 Microcontroller"},
	{ELF_EM_68HC08,	"Motorola MC68HC08 Microcontroller"},
	{ELF_EM_68HC05,	"Motorola MC68HC05 Microcontroller"},
	{ELF_EM_SVX,		"Silicon Graphics SVx"},
	{ELF_EM_ST19,		"STMicroelectronics ST19 8-bit microcontroller"},
	{ELF_EM_VAX,		"Digital VAX"},

	{ELF_EM_PJ,		"picoJava"},
	{0, 0}
};
*/
ht_view *htmachoheader_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_macho_shared_data *macho_shared=(ht_macho_shared_data *)group->get_shared_data();
	
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_MACHO_HEADER, VC_EDIT, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	sprintf(info, "* Mach-O header at offset %08x", macho_shared->header_ofs);
/*	register_atom(ATOM_ELF_CLASS, elf_class);
	register_atom(ATOM_ELF_DATA, elf_data);
	register_atom(ATOM_ELF_OS_ABI, elf_os_abi);
	register_atom(ATOM_ELF_TYPE, elf_type);
	register_atom(ATOM_ELF_MACHINE, elf_machine);*/
	m->add_mask(info);
	m->add_staticmask_ptable(machoheader, macho_shared->header_ofs, true);

	FILEOFS ofs = macho_shared->header_ofs+7*4/*sizeof MACHO_HEADER*/;
	for (uint i=0; i<macho_shared->header.ncmds; i++) {
		switch (macho_shared->cmds[i]->cmd) {
			case LC_SEGMENT: {
				MACHO_SEGMENT_COMMAND *c = (MACHO_SEGMENT_COMMAND *)macho_shared->cmds[i];
				char segname[17];
				ht_snprintf(segname, sizeof segname, "%s", c->segname);
			    	char info[128];
				sprintf(info, "** vaddr %08x vsize %08x fileofs %08x, filesize %08x: %s", c->vmaddr, c->vmsize, c->fileoff, c->filesize, segname);
				m->add_mask(info);
				m->add_staticmask_ptable(macho_segment_header, ofs, true);
				break;
			}
			case LC_SYMTAB: {
			    	char info[128];
				sprintf(info, "** SYMTAB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_SYMSEG: {
			    	char info[128];
				sprintf(info, "** SYMSEG cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_THREAD: {
			    	char info[128];
				sprintf(info, "** THREAD cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_UNIXTHREAD: {
			    	char info[128];
				sprintf(info, "** UNIXTHREAD cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOADFVMLIB: {
			    	char info[128];
				sprintf(info, "** LOADFVMLIB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_IDFVMLIB: {
			    	char info[128];
				sprintf(info, "** IDFVMLIB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_IDENT: {
			    	char info[128];
				sprintf(info, "** IDENT (obsolete) cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_FVMFILE: {
			    	char info[128];
				sprintf(info, "** FVMFILE cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_PREPAGE: {
			    	char info[128];
				sprintf(info, "** PREPAGE cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_DYSYMTAB: {
			    	char info[128];
				sprintf(info, "** DYSYMTAB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOAD_DYLIB: {
			    	char info[128];
				sprintf(info, "** LOAD_DYLIB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_ID_DYLIB: {
			    	char info[128];
				sprintf(info, "** ID_DYLIB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOAD_DYLINKER: {
			    	char info[128];
				sprintf(info, "** LOAD_DYLINKER cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_ID_DYLINKER: {
			    	char info[128];
				sprintf(info, "** ID_DYLINKER cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_PREBOUND_DYLIB: {
			    	char info[128];
				sprintf(info, "** PREBOUND_DYLIB cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			default: {
			    	char info[128];
				sprintf(info, "** unsupported load command %08x, size %08x", macho_shared->cmds[i]->cmd, macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
			}
		}
		ofs += macho_shared->cmds[i]->cmdsize;
	}
	v->insertsub(m);
	return v;
}

format_viewer_if htmachoheader_if = {
	htmachoheader_init,
	0
};
