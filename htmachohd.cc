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
	{"magic",		STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cputype",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"cpusubtype",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"filetype",		STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"number of cmds",	STATICTAG_EDIT_DWORD_VE("00000010")},
	{"size of cmds",	STATICTAG_EDIT_DWORD_VE("00000014")},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000018")},
	{0, 0}
};

static ht_mask_ptable macho_segment_header[]=
{
	{"cmd",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cmdsize",		STATICTAG_EDIT_DWORD_VE("00000004")},
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
	{"virtual address",	STATICTAG_EDIT_DWORD_VE("00000018")},
	{"virtual size",	STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"file offset",		STATICTAG_EDIT_DWORD_VE("00000020")},
	{"file size",		STATICTAG_EDIT_DWORD_VE("00000024")},
	{"max VM protection",	STATICTAG_EDIT_DWORD_VE("00000028")},
	{"init VM protection",	STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"number of sections",	STATICTAG_EDIT_DWORD_VE("00000030")},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000034")},
	{0, 0}
};

static ht_mask_ptable macho_section_header[]=
{
	{"section name",
STATICTAG_EDIT_CHAR("00000000")STATICTAG_EDIT_CHAR("00000001")
STATICTAG_EDIT_CHAR("00000002")STATICTAG_EDIT_CHAR("00000003")
STATICTAG_EDIT_CHAR("00000004")STATICTAG_EDIT_CHAR("00000005")
STATICTAG_EDIT_CHAR("00000006")STATICTAG_EDIT_CHAR("00000007")
STATICTAG_EDIT_CHAR("00000008")STATICTAG_EDIT_CHAR("00000009")
STATICTAG_EDIT_CHAR("0000000a")STATICTAG_EDIT_CHAR("0000000b")
STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")
STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")
},
	{"segment name",
STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")
STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")
STATICTAG_EDIT_CHAR("00000014")STATICTAG_EDIT_CHAR("00000015")
STATICTAG_EDIT_CHAR("00000016")STATICTAG_EDIT_CHAR("00000017")
STATICTAG_EDIT_CHAR("00000018")STATICTAG_EDIT_CHAR("00000019")
STATICTAG_EDIT_CHAR("0000001a")STATICTAG_EDIT_CHAR("0000001b")
STATICTAG_EDIT_CHAR("0000001c")STATICTAG_EDIT_CHAR("0000001d")
STATICTAG_EDIT_CHAR("0000001e")STATICTAG_EDIT_CHAR("0000001f")
},
	{"virtual address",		STATICTAG_EDIT_DWORD_VE("00000020")},
	{"virtual size",		STATICTAG_EDIT_DWORD_VE("00000024")},
	{"file offset",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"alignment",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"relocation file offset",	STATICTAG_EDIT_DWORD_VE("00000030")},
	{"number of relocation entries",STATICTAG_EDIT_DWORD_VE("00000034")},
	{"flags",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"reserved1",			STATICTAG_EDIT_DWORD_VE("0000003c")},
	{"reserved2",			STATICTAG_EDIT_DWORD_VE("00000040")},
	{0, 0}
};

static ht_mask_ptable macho_thread_header[]=
{
	{"cmd",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cmdsize",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"flavor",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"count (of 32bit words)",	STATICTAG_EDIT_DWORD_VE("0000000c")},
	{0, 0}
};

static ht_mask_ptable macho_ppc_thread_state[]=
{
	{"srr0",		STATICTAG_EDIT_DWORD_VE("00000000")},
	{"srr1",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"flavor",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"r0",			STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"r1",			STATICTAG_EDIT_DWORD_VE("00000010")},
	{"r2",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"r3",			STATICTAG_EDIT_DWORD_VE("00000018")},
	{"r4",			STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"r5",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"r6",			STATICTAG_EDIT_DWORD_VE("00000024")},
	{"r7",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"r8",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"r9",			STATICTAG_EDIT_DWORD_VE("00000030")},
	{"r10",			STATICTAG_EDIT_DWORD_VE("00000034")},
	{"r11",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"r12",			STATICTAG_EDIT_DWORD_VE("0000003c")},
	{"r13",			STATICTAG_EDIT_DWORD_VE("00000040")},
	{"r14",			STATICTAG_EDIT_DWORD_VE("00000044")},
	{"r15",			STATICTAG_EDIT_DWORD_VE("00000048")},
	{"r16",			STATICTAG_EDIT_DWORD_VE("0000004c")},
	{"r17",			STATICTAG_EDIT_DWORD_VE("00000050")},
	{"r18",			STATICTAG_EDIT_DWORD_VE("00000054")},
	{"r19",			STATICTAG_EDIT_DWORD_VE("00000058")},
	{"r20",			STATICTAG_EDIT_DWORD_VE("0000005c")},
	{"r21",			STATICTAG_EDIT_DWORD_VE("00000060")},
	{"r22",			STATICTAG_EDIT_DWORD_VE("00000064")},
	{"r23",			STATICTAG_EDIT_DWORD_VE("00000068")},
	{"r24",			STATICTAG_EDIT_DWORD_VE("0000006c")},
	{"r25",			STATICTAG_EDIT_DWORD_VE("00000070")},
	{"r26",			STATICTAG_EDIT_DWORD_VE("00000074")},
	{"r27",			STATICTAG_EDIT_DWORD_VE("00000078")},
	{"r28",			STATICTAG_EDIT_DWORD_VE("0000007c")},
	{"r29",			STATICTAG_EDIT_DWORD_VE("00000080")},
	{"r30",			STATICTAG_EDIT_DWORD_VE("00000084")},
	{"r31",			STATICTAG_EDIT_DWORD_VE("00000088")},
	{"cr",			STATICTAG_EDIT_DWORD_VE("0000008c")},
	{"xer",			STATICTAG_EDIT_DWORD_VE("00000090")},
	{"lr",			STATICTAG_EDIT_DWORD_VE("00000094")},
	{"ctr",			STATICTAG_EDIT_DWORD_VE("00000098")},
	{"mq",			STATICTAG_EDIT_DWORD_VE("0000009c")},
	{"vrsave",		STATICTAG_EDIT_DWORD_VE("000000a0")},
	{0, 0}
};

static ht_mask_ptable macho_i386_thread_state[]=
{
	{"eax",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"ebx",			STATICTAG_EDIT_DWORD_VE("00000004")},
	{"ecx",			STATICTAG_EDIT_DWORD_VE("00000008")},
	{"edx",			STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"edi",			STATICTAG_EDIT_DWORD_VE("00000010")},
	{"esi",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"ebp",			STATICTAG_EDIT_DWORD_VE("00000018")},
	{"esp",			STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"ss",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"eflags",		STATICTAG_EDIT_DWORD_VE("00000024")},
	{"eip",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"cs",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"ds",			STATICTAG_EDIT_DWORD_VE("00000030")},
	{"es",			STATICTAG_EDIT_DWORD_VE("00000034")},
	{"fs",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"gs",			STATICTAG_EDIT_DWORD_VE("0000003c")},
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
	bool isbigendian;
	switch (macho_shared->image_endianess) {
		case little_endian: isbigendian = false; break;
		case big_endian: isbigendian = true; break;
	}
	m->add_mask(info);
	m->add_staticmask_ptable(machoheader, macho_shared->header_ofs, isbigendian);

	FILEOFS ofs = macho_shared->header_ofs+7*4/*sizeof MACHO_HEADER*/;
	for (UINT i=0; i<macho_shared->cmds.count; i++) {
		switch (macho_shared->cmds.cmds[i]->cmd.cmd) {
			case LC_SEGMENT: {
				MACHO_SEGMENT_COMMAND *c = (MACHO_SEGMENT_COMMAND *)macho_shared->cmds.cmds[i];
				char segname[17];
				ht_snprintf(segname, sizeof segname, "%s", c->segname);
			    	char info[128];
				sprintf(info, "** segment %s: vaddr %08x vsize %08x fileofs %08x, filesize %08x", segname, c->vmaddr, c->vmsize, c->fileoff, c->filesize);
				m->add_mask(info);
				m->add_staticmask_ptable(macho_segment_header, ofs, isbigendian);
				FILEOFS sofs = sizeof (MACHO_SEGMENT_COMMAND);
				for (UINT j=0; j<c->nsects; j++) {
					sprintf(info, "**** section %d ****", j);
					m->add_mask(info);
					m->add_staticmask_ptable(macho_section_header, ofs+sofs, isbigendian);
					sofs += 9*4+16+16;
				}
				break;
			}
			case LC_SYMTAB: {
			    	char info[128];
				sprintf(info, "** SYMTAB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_SYMSEG: {
			    	char info[128];
				sprintf(info, "** SYMSEG cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_UNIXTHREAD:
			case LC_THREAD: {
				MACHO_THREAD_COMMAND *c = &macho_shared->cmds.cmds[i]->thread;
			    	char info[128];
				sprintf(info, "** %s", (macho_shared->cmds.cmds[i]->cmd.cmd == LC_UNIXTHREAD) ? "UNIXTHREAD" : "THREAD");
				m->add_mask(info);
				m->add_staticmask_ptable(macho_thread_header, ofs, isbigendian);
				switch (macho_shared->header.cputype) {
				case MACHO_CPU_TYPE_I386:
					switch (c->flavor) {
						case -1:
							m->add_staticmask_ptable(macho_i386_thread_state, ofs+4*4/*4 32bit words in thread_header*/, isbigendian);
							break;
					}
					break;
				case MACHO_CPU_TYPE_POWERPC:
					switch (c->flavor) {
						case FLAVOR_PPC_THREAD_STATE:
							m->add_staticmask_ptable(macho_ppc_thread_state, ofs+4*4/*4 32bit words in thread_header*/, isbigendian);
							break;
					}
					break;
				}
				break;
			}
/*			case LC_THREAD: {
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
			}*/
			case LC_LOADFVMLIB: {
			    	char info[128];
				sprintf(info, "** LOADFVMLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_IDFVMLIB: {
			    	char info[128];
				sprintf(info, "** IDFVMLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_IDENT: {
			    	char info[128];
				sprintf(info, "** IDENT (obsolete) cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_FVMFILE: {
			    	char info[128];
				sprintf(info, "** FVMFILE cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_PREPAGE: {
			    	char info[128];
				sprintf(info, "** PREPAGE cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_DYSYMTAB: {
			    	char info[128];
				sprintf(info, "** DYSYMTAB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOAD_DYLIB: {
			    	char info[128];
				sprintf(info, "** LOAD_DYLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_ID_DYLIB: {
			    	char info[128];
				sprintf(info, "** ID_DYLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOAD_DYLINKER: {
			    	char info[128];
				sprintf(info, "** LOAD_DYLINKER cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_ID_DYLINKER: {
			    	char info[128];
				sprintf(info, "** ID_DYLINKER cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_PREBOUND_DYLIB: {
			    	char info[128];
				sprintf(info, "** PREBOUND_DYLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			default: {
			    	char info[128];
				sprintf(info, "** unsupported load command %08x, size %08x", macho_shared->cmds.cmds[i]->cmd.cmd, macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
			}
		}
		ofs += macho_shared->cmds.cmds[i]->cmd.cmdsize;
	}
	v->insertsub(m);
	return v;
}

format_viewer_if htmachoheader_if = {
	htmachoheader_init,
	0
};
