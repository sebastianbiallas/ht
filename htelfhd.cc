/* 
 *	HT Editor
 *	htelfhd.cc
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

#include "elfstruc.h"
#include "atom.h"
#include "htelf.h"
#include "htelfhd.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable elfheader[]=
{
	{"ident", 0},
	{"    magic",		STATICTAG_EDIT_BYTE("00000000") 
	                    " " STATICTAG_EDIT_BYTE("00000001") 
	                    " " STATICTAG_EDIT_BYTE("00000002")
	                    " " STATICTAG_EDIT_BYTE("00000003")
	                    " = " STATICTAG_EDIT_CHAR("00000000") 
	                          STATICTAG_EDIT_CHAR("00000001") 
	                          STATICTAG_EDIT_CHAR("00000002") 
	                          STATICTAG_EDIT_CHAR("00000003")},
	{"    class",		STATICTAG_EDIT_BYTE("00000004") " (" STATICTAG_DESC_BYTE("00000004", ATOM_ELF_CLASS_STR) ")"},
	{"    data",		STATICTAG_EDIT_BYTE("00000005") " (" STATICTAG_DESC_BYTE("00000005", ATOM_ELF_DATA_STR) ")"},
	{"    version",		STATICTAG_EDIT_BYTE("00000006")},
	{"    OS ABI",		STATICTAG_EDIT_BYTE("00000007") " (" STATICTAG_DESC_BYTE("00000007", ATOM_ELF_OS_ABI_STR) ")"},
	{"    version",		STATICTAG_EDIT_BYTE("00000008")},
	{"    reserved",	STATICTAG_EDIT_BYTE("00000009") 
	                    " " STATICTAG_EDIT_BYTE("0000000a")
	                    " " STATICTAG_EDIT_BYTE("0000000b")
	                    " " STATICTAG_EDIT_BYTE("0000000c")
	                    " " STATICTAG_EDIT_BYTE("0000000d")
	                    " " STATICTAG_EDIT_BYTE("0000000e")
	                    " " STATICTAG_EDIT_BYTE("0000000f")},
	{0, 0}
};

static ht_mask_ptable elfheader32[]=
{
	{"type",			STATICTAG_EDIT_WORD_VE("00000010")
	                           " (" STATICTAG_DESC_WORD_VE("00000010", ATOM_ELF_TYPE_STR) ")"},
	{"machine",			STATICTAG_EDIT_WORD_VE("00000012") " (" STATICTAG_DESC_WORD_VE("00000012", ATOM_ELF_MACHINE_STR) ")"},
	{"version",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"entrypoint",			STATICTAG_EDIT_DWORD_VE("00000018")},
	{"program header offset",	STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"section header offset",	STATICTAG_EDIT_DWORD_VE("00000020")},
	{"flags",			STATICTAG_EDIT_DWORD_VE("00000024")},
	{"elf header size",		STATICTAG_EDIT_WORD_VE("00000028")},
	{"program header entry size",	STATICTAG_EDIT_WORD_VE("0000002a")},
	{"program header count",	STATICTAG_EDIT_WORD_VE("0000002c")},
	{"section header entry size",	STATICTAG_EDIT_WORD_VE("0000002e")},
	{"section header count",	STATICTAG_EDIT_WORD_VE("00000030")},
	{"section header strtab section index", STATICTAG_EDIT_WORD_VE("00000032")},
	{0, 0}
};

static ht_mask_ptable elfheader64[]=
{
	{"type",			STATICTAG_EDIT_WORD_VE("00000010") " (" STATICTAG_DESC_WORD_VE("00000010", ATOM_ELF_TYPE_STR)")"},
	{"machine",			STATICTAG_EDIT_WORD_VE("00000012") " (" STATICTAG_DESC_WORD_VE("00000012", ATOM_ELF_MACHINE_STR)")"},
	{"version",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"entrypoint",			STATICTAG_EDIT_QWORD_VE("00000018")},
	{"program header offset",	STATICTAG_EDIT_QWORD_VE("00000020")},
	{"section header offset",	STATICTAG_EDIT_QWORD_VE("00000028")},
	{"flags",			STATICTAG_EDIT_DWORD_VE("00000030")},
	{"elf header size",		STATICTAG_EDIT_WORD_VE("00000034")},
	{"program header entry size",	STATICTAG_EDIT_WORD_VE("00000036")},
	{"program header count",	STATICTAG_EDIT_WORD_VE("00000038")},
	{"section header entry size",	STATICTAG_EDIT_WORD_VE("0000003a")},
	{"section header count",	STATICTAG_EDIT_WORD_VE("0000003c")},
	{"section header strtab section index",STATICTAG_EDIT_WORD_VE("0000003e")},
	{0, 0}
};

static int_hash elf_class[] =
{
	{ELFCLASSNONE, "invalid class type"},
	{ELFCLASS32, "32-bit objects"},
	{ELFCLASS64, "64-bit objects"},
	{0, 0}
};

static int_hash elf_data[] =
{
	{ELFDATANONE, "invalid data encoding"},
	{ELFDATA2LSB, "LSB encoding"},
	{ELFDATA2MSB, "MSB encoding"},
	{0, 0}
};

static int_hash elf_os_abi[] =
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

static int_hash elf_type[] =
{
	{ELF_ET_NONE, "no file type"},
	{ELF_ET_REL, "relocatable file"},
	{ELF_ET_EXEC, "executable file"},
	{ELF_ET_DYN, "shared object file"},
	{ELF_ET_CORE, "core file"},
	{0, 0}
};

static int_hash elf_machine[] =
{
	{ELF_EM_NONE,  		"no machine"},
	{ELF_EM_M32,   		"AT&T WE32100"},
	{ELF_EM_SPARC, 		"SPARC"},
	{ELF_EM_386,   		"Intel 80386"},
	{ELF_EM_68K,   		"Motorola 68000"},
	{ELF_EM_88K,   		"Motorola 88000"},
	{ELF_EM_486,		"Intel 80486"},
	{ELF_EM_860,   		"Intel 80860"},
	{ELF_EM_MIPS,		"MIPS R3000 big-endian"},
	{ELF_EM_S370,		"IBM System/370"},
	{ELF_EM_MIPS_RS4_BE,	"MIPS R4000 big-endian"},

	{ELF_EM_PARISC,		"HPPA"},

	{ELF_EM_VPP550,		"Fujitsu VPP500"},
	{ELF_EM_SPARC32PLUS,	"Sun's \"v8plus\""},
	{ELF_EM_960,		"Intel 80960"},
	{ELF_EM_PPC,		"PowerPC"},
	{ELF_EM_PPC64,		"64-bit PowerPC"},
	{ELF_EM_S390,		"IBM S/390"},
	{ELF_EM_SPU,		"Sony/Toshiba/IBM SPU"},
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
	{ELF_EM_H8_300,		"Hitachi H8/300"},
	{ELF_EM_H8_300H,	"Hitachi H8/300H"},
	{ELF_EM_H8S,		"Hitachi H8S"},
	{ELF_EM_H8_500,		"Hitachi H8/500"},
	{ELF_EM_IA_64,		"Intel IA-64 Processor"},
	{ELF_EM_MIPS_X,		"Stanford MIPS-X"},
	{ELF_EM_COLDFIRE,	"Motorola Coldfire"},
	{ELF_EM_68HC12,		"Motorola M68HC12"},
	{ELF_EM_MMA,		"Fujitsu Multimedia Accelerator"},
	{ELF_EM_PCP,		"Siemens PCP"},
	{ELF_EM_NCPU,		"Sony nCPU embedded RISC processor"},
	{ELF_EM_NDR1,		"Denso NDR1 microprocesspr"},
	{ELF_EM_STARCORE,	"Motorola Star*Core processor"},
	{ELF_EM_ME16,		"Toyota ME16 processor"},
	{ELF_EM_ST100,		"STMicroelectronics ST100 processor"},
	{ELF_EM_TINYJ,		"Advanced Logic Corp. TinyJ embedded processor"},
	{ELF_EM_X86_64,		"x86-64 (AMD Opteron)"},
	{ELF_EM_PDSP,		"Sony DSP Processor"},
	{ELF_EM_PDP10,          "Digital Equipment Corp. PDP-10"},
	{ELF_EM_PDP11,          "Digital Equipment Corp. PDP-11"},
	{ELF_EM_FX66,		"Siemens FX66 microcontroller"},
	{ELF_EM_ST9PLUS,	"STMicroelectronics ST9+ 8/16 bit microcontroller"},
	{ELF_EM_ST7,		"STMicroelectronics ST7 8-bit microcontroller"},
	{ELF_EM_68HC16,		"Motorola MC68HC16 Microcontroller"},
	{ELF_EM_68HC11,		"Motorola MC68HC11 Microcontroller"},
	{ELF_EM_68HC08,		"Motorola MC68HC08 Microcontroller"},
	{ELF_EM_68HC05,		"Motorola MC68HC05 Microcontroller"},
	{ELF_EM_SVX,		"Silicon Graphics SVx"},
	{ELF_EM_ST19,		"STMicroelectronics ST19 8-bit microcontroller"},
	{ELF_EM_VAX,		"Digital VAX"},
	{ELF_EM_CRIS,		"Axis Communications 32-bit CRIS"},
	{ELF_EM_JAVELIN,	"Infineon Technologies 32-bit \"Javelin\""},
	{ELF_EM_FIREPATH,	"Element 14 64-bit \"FirePath\" DSP"},
	{ELF_EM_ZSP,		"LSI Logic 16-bit \"ZSP\" DSP"},
	{ELF_EM_MMIX,		"Donald Knuth's 64-bit MMIX"},
	{ELF_EM_HUANY,		"Harvard's machine-independent format"},
	{ELF_EM_PRISM,		"SiTera Prism"},
	{ELF_EM_AVR,		"Atmel AVR 8-bit microcontroller"},
	{ELF_EM_FR30,		"Fujitsu FR30"},
	{ELF_EM_D10V,		"Mitsubishi D10V"},
	{ELF_EM_D30V,		"Mitsubishi D30V"},
	{ELF_EM_V850,		"NEC v850"},
	{ELF_EM_M32R,		"Renesas/Mitsubishi M32R"},
	{ELF_EM_MN10300,	"Matsushita MN10300"},
	{ELF_EM_MN10200,	"Matsushita MN10200"},
	{ELF_EM_PJ,		"picoJava"},
	{ELF_EM_OPENRISC,	"OpenRISC 32-bit"},
	{ELF_EM_ARC_A5,		"ARC Cores Tangent-A5"},
	{ELF_EM_XTENSA,		"Tensilica Xtensa"},
	{ELF_EM_VIDEOCORE,	"Alphamosaic VideoCore"},
	{ELF_EM_TMM_GPP,	"Thompson Multimedia General Purpose Processor"},
	{ELF_EM_NS32K,		"National Semiconductor 32000 series"},
	{ELF_EM_TPC,		"Tenor Network TPC processor"},
	{ELF_EM_SNP1K,		"Trebia SNP 1000 processor"},
	{ELF_EM_ST200,		"STMicroelectronics ST200 microcontroller"},
	{ELF_EM_IP2K,		"Ubicom IP2022 micro controller"},
	{ELF_EM_MAX,		"MAX Processor"},
	{ELF_EM_CR,		"National Semiconductor CompactRISC"},
	{ELF_EM_F2MC16,		"Fujitsu F2MC16"},
	{ELF_EM_MSP430,		"TI msp430 micro controller"},
	{ELF_EM_BLACKFIN,	"ADI Blackfin"},
	{ELF_EM_SE_C33,		"Seiko Epson S1C33"},
	{ELF_EM_SEP,		"Sharp embedded microprocessor"},
	{ELF_EM_ARCA,		"Arca RISC Microprocessor"},
	{ELF_EM_UNICORE,	"PKU-Unity/MPRC Unicore"},
	{ELF_EM_EXCESS,		"eXcess embedded CPU"},
	{ELF_EM_DXP,		"Icera Semiconductor Deep Execution Processor"},
	{ELF_EM_ALTERA_NIOS2,	"Altera Nios II"},
	{ELF_EM_CRX,		"National Semiconductor CRX"},
	{ELF_EM_XGATE,		"Motorola XGATE"},
	{ELF_EM_C166,		"Infineon C16x/XC16x processor"},
	{ELF_EM_M16C,		"Renebas M16C series microprocessors"},
	{ELF_EM_DSPIC30F,	"Microchip Technology dsPIC30F Digital Signal Controller"},
	{ELF_EM_CE,		"Freescale Communication Engine RISC core"},
	{ELF_EM_M32C,		"Renesas M32C series microprocessors"},
	{ELF_EM_TSK3000,	"Altium TSK3000"},
	{ELF_EM_RS08,		"Freescale RS08"},
	{ELF_EM_ECOG2,		"Cyan Technology eCOG2"},
	{ELF_EM_SCORE,		"Sunplus S+core"},
	{ELF_EM_DSP24,		"New Japan Radi (NJR) 24-bit DSP"},
	{ELF_EM_VIDEOCORE3,	"Broadcom VideoCore III"},
	{ELF_EM_LATTICEMICO32,	"Lattice FPGA RISC"},
	{ELF_EM_SE_C17,		"Seiko Epson C17"},
	{ELF_EM_TI_C6000,	"TI C6X DSP"},
	{ELF_EM_MMDSP_PLUS,	"STMicroelectronics 64bit VLIW Data Signal Processor"},
	{ELF_EM_CYPRESS_M8C,	"Cypress M8C"},
	{ELF_EM_R32C,		"Renesas R32C series microprocessors"},
	{ELF_EM_TRIMEDIA,	"NXP Semiconductor TriMedia"},
	{ELF_EM_QDSP6,		"Qualcomm DSP6"},
	{ELF_EM_8051,		"Intel 8051"},
	{ELF_EM_STXP7X,		"STMicroelectronics STxP7x family"},
	{ELF_EM_NDS32,		"Andes Technology NDS32"},
	{ELF_EM_ECOG1,		"Cyan Technology eCOG1X family"},
	{ELF_EM_ECOG1X,		"Cyan Technology eCOG1X family"},
	{ELF_EM_MAXQ30,		"Dallas Semiconductor MAXQ30"},
	{ELF_EM_XIMO16,		"New Japan Radio XIMO16"},
	{ELF_EM_MANIK,		"M2000 Reconfigurable RISC Microprocessor"},
	{ELF_EM_CRAYNV2,	"Cray Inc. NV2 vector architecture"},
	{ELF_EM_RX,		"Renesas RX family"},
	{ELF_EM_METAG,		"Imagination Technologies META processor"},
	{ELF_EM_MCST_ELBRUS,	"MCST Elbrus"},
	{ELF_EM_ECOG16,		"Cyan Technology eCOG16 family"},
	{ELF_EM_CR16,		"National Semiconductor CompactRISC 16-bit processor"},
	{ELF_EM_ETPU,		"Freescale Extended Time Processing Unit"},
	{ELF_EM_SLE9X,		"Infineon Technologies SLE9X core"},
	{ELF_EM_L1OM,		"Intel L1OM (Knights Landing)"},
	{ELF_EM_K1OM,		"Intel K1OM (Knights Corner/Xeon Phi)"},
	{ELF_EM_AARCH64,	"ARM 64-bit Architecture"},
	{ELF_EM_AVR32,		"Atmel Corporation 32-bit microprocessor family"},
	{ELF_EM_TILEPRO,	"Tilera TILEPro"},
	{ELF_EM_MICROBLAZE,	"Xilinx MicroBlaze"},
	{ELF_EM_TILEGX,		"Tilera TILE-Gx"},
	{ELF_EM_RISCV,		"RISC-V"},
	{ELF_EM_BPF,		"Linux BPF in-kernel VM"},
	{ELF_EM_FRV,		"Fujitsu FR-V"},
	{ELF_EM_ALPHA,		"Alpha"},
	{0, NULL}
};

ht_view *htelfheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();
	
	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_ELF_HEADER, VC_EDIT, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* ELF header at offset 0x%08qx", elf_shared->header_ofs);
	registerAtom(ATOM_ELF_CLASS, elf_class);
	registerAtom(ATOM_ELF_DATA, elf_data);
	registerAtom(ATOM_ELF_OS_ABI, elf_os_abi);
	registerAtom(ATOM_ELF_TYPE, elf_type);
	registerAtom(ATOM_ELF_MACHINE, elf_machine);
	m->add_mask(info);
	m->add_staticmask_ptable(elfheader, elf_shared->header_ofs, true);
	bool elf_bigendian = (elf_shared->ident.e_ident[ELF_EI_DATA] == ELFDATA2MSB);
	
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
	case ELFCLASS32:
		m->add_staticmask_ptable(elfheader32, elf_shared->header_ofs, elf_bigendian);
		break;
	case ELFCLASS64:
		m->add_staticmask_ptable(elfheader64, elf_shared->header_ofs, elf_bigendian);
		break;
	}
	
	v->insertsub(m);
	return v;
}

format_viewer_if htelfheader_if = {
	htelfheader_init,
	0
};
