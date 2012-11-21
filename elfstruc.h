/* 
 *	HT Editor
 *	elfstruc.h
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

/*	mostly assembled from TIS ELF 1.1g and BFD-ELF 	*/

#ifndef __ELFSTRUC_H__
#define __ELFSTRUC_H__

#include "io/types.h"

/* all architectures */
typedef unsigned char elf_unsigned_char;

/* 32bit architectures */
typedef uint32 elf32_addr;
typedef uint16 elf32_half;
typedef uint32 elf32_off;
typedef uint32 elf32_sword;
typedef uint32 elf32_word;

/* 64bit architectures */
typedef uint64 elf64_addr;
typedef uint64 elf64_off;
typedef uint64 elf64_sword;
typedef uint64 elf64_word;
typedef uint32 elf64_half;
typedef uint16 elf64_quarter;

/*
 *	ELF header
 */

/* e_ident */
#define ELF_EI_MAG0			0
#define ELF_EI_MAG1			1
#define ELF_EI_MAG2			2
#define ELF_EI_MAG3			3
#define ELF_EI_CLASS			4
#define ELF_EI_DATA			5
#define ELF_EI_VERSION			6
#define ELF_EI_OSABI			7
#define ELF_EI_ABIVERSION		8
#define ELF_EI_PAD			9

#define EI_NIDENT			16

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASSNONE			0
#define ELFCLASS32			1
#define ELFCLASS64			2

#define ELFDATANONE			0
#define ELFDATA2LSB			1
#define ELFDATA2MSB			2

#define ELFOSABI_SYSV			0
#define ELFOSABI_HPUX			1
#define ELFOSABI_NETBSD			2
#define ELFOSABI_LINUX			3
#define ELFOSABI_HURD			4
#define ELFOSABI_86OPEN			5
#define ELFOSABI_SOLARIS		6
#define ELFOSABI_MONTEREY		7
#define ELFOSABI_IRIX			8
#define ELFOSABI_FREEBSD		9
#define ELFOSABI_TRU64			10
#define ELFOSABI_MODESTO		11
#define ELFOSABI_OPENBSD		12
#define ELFOSABI_ARM			97
#define ELFOSABI_STANDALONE		255

/* e_type */
#define ELF_ET_NONE			0
#define ELF_ET_REL			1
#define ELF_ET_EXEC			2
#define ELF_ET_DYN			3
#define ELF_ET_CORE			4
#define ELF_ET_LOOS			0xFE00
#define ELF_ET_HIOS			0xFEFF
#define ELF_ET_LOPROC			0xFF00
#define ELF_ET_HIPROC			0xFFFF

/* e_machine */
#define ELF_EM_NONE			0
#define ELF_EM_M32			1
#define ELF_EM_SPARC			2
#define ELF_EM_386			3
#define ELF_EM_68K			4
#define ELF_EM_88K			5
#define ELF_EM_486			6	/* Intel 80486 */
#define ELF_EM_860			7	/* Intel 80860 */
#define ELF_EM_MIPS			8	/* MIPS R3000 (officially, big-endian only) */
#define ELF_EM_S370			9	/* IBM System/370 */
#define ELF_EM_MIPS_RS4_BE		10	/* MIPS R4000 big-endian */ /* Depreciated */
#define ELF_EM_MIPS_RS3_LE		10	/* MIPS R3000 little-endian (Oct 4 1999 Draft)*/ /* Depreciated */

#define ELF_EM_PARISC			15	/* HPPA */

#define ELF_EM_VPP550			17	/* Fujitsu VPP500 */
#define ELF_EM_SPARC32PLUS		18	/* Sun's "v8plus" */
#define ELF_EM_960			19	/* Intel 80960 */
#define ELF_EM_PPC			20	/* PowerPC */
#define ELF_EM_PPC64			21	/* 64-bit PowerPC */
#define ELF_EM_S390                     22	/* IBM S/390 */
#define ELF_EM_SPU                      23	/* Sony/Toshiba/IBM SPU */

#define ELF_EM_V800			36	/* NEC V800 series */
#define ELF_EM_FR20			37	/* Fujitsu FR20 */
#define ELF_EM_RH32			38	/* TRW RH32 */
#define ELF_EM_MCORE			39	/* Motorola M*Core */ /* May also be taken by Fujitsu MMA */
#define ELF_EM_RCE			39	/* Old name for MCore */
#define ELF_EM_ARM			40	/* ARM */
#define ELF_EM_OLD_ALPHA		41	/* Digital Alpha */
#define ELF_EM_SH			42	/* Hitachi SH */
#define ELF_EM_SPARCV9			43	/* SPARC v9 64-bit */
#define ELF_EM_TRICORE			44	/* Siemens Tricore embedded processor */
#define ELF_EM_ARC			45	/* Argonaut RISC Core, Argonaut Technologies Inc. */
#define ELF_EM_H8_300			46	/* Hitachi H8/300 */
#define ELF_EM_H8_300H			47	/* Hitachi H8/300H */
#define ELF_EM_H8S			48	/* Hitachi H8S */
#define ELF_EM_H8_500			49	/* Hitachi H8/500 */
#define ELF_EM_IA_64			50	/* Intel IA-64 Processor */
#define ELF_EM_MIPS_X			51	/* Stanford MIPS-X */
#define ELF_EM_COLDFIRE			52	/* Motorola Coldfire */
#define ELF_EM_68HC12			53	/* Motorola M68HC12 */
#define ELF_EM_MMA			54	/* Fujitsu Multimedia Accelerator */
#define ELF_EM_PCP			55	/* Siemens PCP */
#define ELF_EM_NCPU			56	/* Sony nCPU embedded RISC processor */
#define ELF_EM_NDR1			57	/* Denso NDR1 microprocesspr */
#define ELF_EM_STARCORE			58	/* Motorola Star*Core processor */
#define ELF_EM_ME16			59	/* Toyota ME16 processor */
#define ELF_EM_ST100			60	/* STMicroelectronics ST100 processor */
#define ELF_EM_TINYJ			61	/* Advanced Logic Corp. TinyJ embedded processor */
#define ELF_EM_X86_64			62	/* X86-64 (AMD Opteron) */
#define ELF_EM_PDSP                     63	/* Sony DSP Processor */
#define ELF_EM_PDP10                    64	/* Digital Equipment Corp. PDP-10 */
#define ELF_EM_PDP11                    65	/* Digital Equipment Corp. PDP-11 */
#define ELF_EM_FX66			66	/* Siemens FX66 microcontroller */
#define ELF_EM_ST9PLUS			67	/* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define ELF_EM_ST7			68	/* STMicroelectronics ST7 8-bit microcontroller */
#define ELF_EM_68HC16			69	/* Motorola MC68HC16 Microcontroller */
#define ELF_EM_68HC11			70	/* Motorola MC68HC11 Microcontroller */
#define ELF_EM_68HC08			71	/* Motorola MC68HC08 Microcontroller */
#define ELF_EM_68HC05			72	/* Motorola MC68HC05 Microcontroller */
#define ELF_EM_SVX			73	/* Silicon Graphics SVx */
#define ELF_EM_ST19			74	/* STMicroelectronics ST19 8-bit microcontroller */
#define ELF_EM_VAX			75	/* Digital VAX */
#define ELF_EM_CRIS		 76	/* Axis Communications 32-bit embedded processor */
#define ELF_EM_JAVELIN	 77	/* Infineon Technologies 32-bit embedded cpu */
#define ELF_EM_FIREPATH	 78	/* Element 14 64-bit DSP processor */
#define ELF_EM_ZSP		 79	/* LSI Logic's 16-bit DSP processor */
#define ELF_EM_MMIX		 80	/* Donald Knuth's educational 64-bit processor */
#define ELF_EM_HUANY	 81	/* Harvard's machine-independent format */
#define ELF_EM_PRISM	 82	/* SiTera Prism */
#define ELF_EM_AVR		 83	/* Atmel AVR 8-bit microcontroller */
#define ELF_EM_FR30		 84	/* Fujitsu FR30 */
#define ELF_EM_D10V		 85	/* Mitsubishi D10V */
#define ELF_EM_D30V		 86	/* Mitsubishi D30V */
#define ELF_EM_V850		 87	/* NEC v850 */
#define ELF_EM_M32R		 88	/* Renesas M32R (formerly Mitsubishi M32R) */
#define ELF_EM_MN10300	 89	/* Matsushita MN10300 */
#define ELF_EM_MN10200	 90	/* Matsushita MN10200 */
#define ELF_EM_PJ		 91	/* picoJava */
#define ELF_EM_OPENRISC	 92	/* OpenRISC 32-bit embedded processor */
#define ELF_EM_ARC_A5	 93	/* ARC Cores Tangent-A5 */
#define ELF_EM_XTENSA	 94	/* Tensilica Xtensa Architecture */
#define ELF_EM_VIDEOCORE	 95	/* Alphamosaic VideoCore processor */
#define ELF_EM_TMM_GPP	 96	/* Thompson Multimedia General Purpose Processor */
#define ELF_EM_NS32K	 97	/* National Semiconductor 32000 series */
#define ELF_EM_TPC	 98	/* Tenor Network TPC processor */
#define ELF_EM_SNP1K	 99	/* Trebia SNP 1000 processor */
#define ELF_EM_ST200	100	/* STMicroelectronics ST200 microcontroller */
#define ELF_EM_IP2K	101	/* Ubicom IP2022 micro controller */
#define ELF_EM_MAX	102	/* MAX Processor */
#define ELF_EM_CR	103	/* National Semiconductor CompactRISC */
#define ELF_EM_F2MC16	104	/* Fujitsu F2MC16 */
#define ELF_EM_MSP430	105	/* TI msp430 micro controller */
#define ELF_EM_BLACKFIN	106	/* ADI Blackfin */
#define ELF_EM_SE_C33	107	/* S1C33 Family of Seiko Epson processors */
#define ELF_EM_SEP	108	/* Sharp embedded microprocessor */
#define ELF_EM_ARCA	109	/* Arca RISC Microprocessor */
#define ELF_EM_UNICORE	110	/* Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University */
#define ELF_EM_EXCESS	111	/* eXcess: 16/32/64-bit configurable embedded CPU */
#define ELF_EM_DXP	112	/* Icera Semiconductor Inc. Deep Execution Processor */
#define ELF_EM_ALTERA_NIOS2	113	/* Altera Nios II soft-core processor */
#define ELF_EM_CRX	114	/* National Semiconductor CRX */
#define ELF_EM_XGATE	115	/* Motorola XGATE embedded processor */
#define ELF_EM_C166	116	/* Infineon C16x/XC16x processor */
#define ELF_EM_M16C	117	/* Renesas M16C series microprocessors */
#define ELF_EM_DSPIC30F	118	/* Microchip Technology dsPIC30F Digital Signal Controller */
#define ELF_EM_CE	119	/* Freescale Communication Engine RISC core */
#define ELF_EM_M32C	120	/* Renesas M32C series microprocessors */

#define ELF_EM_TSK3000	131	/* Altium TSK3000 core */
#define ELF_EM_RS08	132	/* Freescale RS08 embedded processor */

#define ELF_EM_ECOG2	134	/* Cyan Technology eCOG2 microprocessor */
#define ELF_EM_SCORE        135     /* Sunplus Score */
#define ELF_EM_DSP24	136	/* New Japan Radio (NJR) 24-bit DSP Processor */
#define ELF_EM_VIDEOCORE3	137	/* Broadcom VideoCore III processor */
#define ELF_EM_LATTICEMICO32 138	/* RISC processor for Lattice FPGA architecture */
#define ELF_EM_SE_C17	139	/* Seiko Epson C17 family */

#define ELF_EM_MMDSP_PLUS	160	/* STMicroelectronics 64bit VLIW Data Signal Processor */
#define ELF_EM_CYPRESS_M8C	161	/* Cypress M8C microprocessor */
#define ELF_EM_R32C	162	/* Renesas R32C series microprocessors */
#define ELF_EM_TRIMEDIA	163	/* NXP Semiconductors TriMedia architecture family */
#define ELF_EM_QDSP6	164	/* QUALCOMM DSP6 Processor */
#define ELF_EM_8051	165	/* Intel 8051 and variants */
#define ELF_EM_STXP7X	166	/* STMicroelectronics STxP7x family */
#define ELF_EM_NDS32	167	/* Andes Technology compact code size embedded RISC processor family */
#define ELF_EM_ECOG1	168	/* Cyan Technology eCOG1X family */
#define ELF_EM_ECOG1X	168	/* Cyan Technology eCOG1X family */
#define ELF_EM_MAXQ30	169	/* Dallas Semiconductor MAXQ30 Core Micro-controllers */
#define ELF_EM_XIMO16	170	/* New Japan Radio (NJR) 16-bit DSP Processor */
#define ELF_EM_MANIK	171	/* M2000 Reconfigurable RISC Microprocessor */
#define ELF_EM_CRAYNV2	172	/* Cray Inc. NV2 vector architecture */
#define ELF_EM_RX	173	/* Renesas RX family */
#define ELF_EM_METAG	174	/* Imagination Technologies META processor architecture */
#define ELF_EM_MCST_ELBRUS	175	/* MCST Elbrus general purpose hardware architecture */
#define ELF_EM_ECOG16	176	/* Cyan Technology eCOG16 family */
#define ELF_EM_CR16	177	/* National Semiconductor CompactRISC 16-bit processor */
#define ELF_EM_ETPU	178	/* Freescale Extended Time Processing Unit */
#define ELF_EM_SLE9X	179	/* Infineon Technologies SLE9X core */
#define ELF_EM_L1OM	180	/* Intel L1OM */

/* e_version */
#define ELF_EV_NONE			0
#define ELF_EV_CURRENT			1

struct ELF_HEADER {
	elf_unsigned_char e_ident[EI_NIDENT];
};

struct ELF_HEADER32 {
	elf32_half e_type;
	elf32_half e_machine;
	elf32_word e_version;
	elf32_addr e_entry;
	elf32_off e_phoff;
	elf32_off e_shoff;
	elf32_word e_flags;
	elf32_half e_ehsize;
	elf32_half e_phentsize;
	elf32_half e_phnum;
	elf32_half e_shentsize;
	elf32_half e_shnum;
	elf32_half e_shstrndx;
};

struct ELF_HEADER64 {
	elf64_quarter e_type;
	elf64_quarter e_machine;
	elf64_half e_version;
	elf64_addr e_entry;
	elf64_off e_phoff;
	elf64_off e_shoff;
	elf64_half e_flags;
	elf64_quarter e_ehsize;
	elf64_quarter e_phentsize;
	elf64_quarter e_phnum;
	elf64_quarter e_shentsize;
	elf64_quarter e_shnum;
	elf64_quarter e_shstrndx;
};

/*
 *	ELF section header
 */

/* indices */
#define ELF_SHN_UNDEF		0
#define ELF_SHN_ABS		0xfff1
#define ELF_SHN_COMMON		0xfff2

/* sh_type */
#define ELF_SHT_NULL		0
#define ELF_SHT_PROGBITS	1
#define ELF_SHT_SYMTAB		2
#define ELF_SHT_STRTAB		3
#define ELF_SHT_RELA		4
#define ELF_SHT_HASH		5
#define ELF_SHT_DYNAMIC		6
#define ELF_SHT_NOTE		7
#define ELF_SHT_NOBITS		8
#define ELF_SHT_REL		9
#define ELF_SHT_SHLIB		10
#define ELF_SHT_DYNSYM		11

#define ELF_SHT_INIT_ARRAY	14
#define ELF_SHT_FINI_ARRAY	15
#define ELF_SHT_PREINIT_ARRAY	16

/* sh_flags */
#define ELF_SHF_WRITE			(1<<0)
#define ELF_SHF_ALLOC			(1<<1)
#define ELF_SHF_EXECINSTR		(1<<2)
// 1<<3 missing
#define ELF_SHF_MERGE			(1<<4)
#define ELF_SHF_STRINGS			(1<<5)
#define ELF_SHF_INFO_LINK		(1<<6)
#define ELF_SHF_LINK_ORDER		(1<<7)
#define ELF_SHF_OS_NONCONFORMING	(1<<8)

struct ELF_SECTION_HEADER32 {
	elf32_word sh_name;
	elf32_word sh_type;
	elf32_word sh_flags;
	elf32_addr sh_addr;     
	elf32_off sh_offset;
	elf32_word sh_size;
	elf32_word sh_link;
	elf32_word sh_info;
	elf32_word sh_addralign;
	elf32_word sh_entsize;
};

struct ELF_SECTION_HEADER64 {
	elf64_half sh_name;
	elf64_half sh_type;
	elf64_word sh_flags;
	elf64_addr sh_addr;
	elf64_off sh_offset;
	elf64_word sh_size;
	elf64_half sh_link;
	elf64_half sh_info;
	elf64_word sh_addralign;
	elf64_word sh_entsize;
};

/*
 *	ELF program header
 */

#define ELF_PT_NULL		0
#define ELF_PT_LOAD		1
#define ELF_PT_DYNAMIC		2
#define ELF_PT_INTERP		3
#define ELF_PT_NOTE		4
#define ELF_PT_SHLIB		5
#define ELF_PT_PHDR		6
#define ELF_PT_TLS		7          /* Thread-local storage segment  */
#define ELF_PT_NUM		8          /* Number of defined types       */
#define ELF_PT_GNU_EH_FRAME	0x6474e550 /* GCC .eh_frame_hdr segment     */
#define ELF_PT_GNU_STACK	0x6474e551 /* Indicates stack executability */
#define ELF_PT_GNU_RELRO	0x6474e552 /* Read-only after relocation    */ 
#define ELF_PT_PAX_FLAGS	0x65041580 /* Indicates PaX flag markings */

struct ELF_PROGRAM_HEADER32 {
	elf32_word p_type;
	elf32_off p_offset;
	elf32_addr p_vaddr;
	elf32_addr p_paddr;
	elf32_word p_filesz;
	elf32_word p_memsz;
	elf32_word p_flags;
	elf32_word p_align;
} PACKED;

struct ELF_PROGRAM_HEADER64 {
	elf64_half p_type;
	elf64_half p_flags;
	elf64_off p_offset;
	elf64_addr p_vaddr;
	elf64_addr p_paddr;
	elf64_word p_filesz;
	elf64_word p_memsz;
	elf64_word p_align;
};

struct ELF_NHEADER32 {
	elf32_word n_descsz;
	elf32_word n_namesz;
	elf32_word n_type;
} PACKED;

/*
 *	ELF symbol
 */

#define ELF_STB_LOCAL		0
#define ELF_STB_GLOBAL		1
#define ELF_STB_WEAK		2

#define ELF_STT_NOTYPE		0
#define ELF_STT_OBJECT		1
#define ELF_STT_FUNC		2
#define ELF_STT_SECTION		3
#define ELF_STT_FILE		4
#define ELF_STT_COMMON		5

#define ELF32_ST_BIND(i)		((i)>>4)
#define ELF32_ST_TYPE(i)		((i)&0xf)
#define ELF32_ST_INFO(b,t)	(((b)>>4)|((t)&0xf))

#define ELF64_ST_BIND(i)		((i)>>4)
#define ELF64_ST_TYPE(i)		((i)&0xf)
#define ELF64_ST_INFO(b,t)	(((b)>>4)|((t)&0xf))

struct ELF_SYMBOL32 {
	elf32_word st_name;
	elf32_addr st_value;
	elf32_word st_size;
	elf_unsigned_char st_info;
	elf_unsigned_char st_other;
	elf32_half st_shndx;
} PACKED;

struct ELF_SYMBOL64 {
	elf64_half st_name;
	elf_unsigned_char st_info;
	elf_unsigned_char st_other;
	elf64_quarter st_shndx;
	elf64_word st_value;
	elf64_word st_size;
} PACKED;

/*
 *	ELF relocation
 */

#define ELF32_R_SYM(i)		((i)>>8)
#define ELF32_R_TYPE(i)		((unsigned char)(i))
#define ELF32_R_INFO(s,t)	(((s)<<8)+(unsigned char)(t))

#define ELF_R_386_NONE			0
#define ELF_R_386_32			1
#define ELF_R_386_PC32			2
#define ELF_R_386_GOT32			3
#define ELF_R_386_PLT32			4
#define ELF_R_386_COPY			5
#define ELF_R_386_GLOB_DAT		6
#define ELF_R_386_JMP_SLOT		7
#define ELF_R_386_RELATIVE		8
#define ELF_R_386_GOTOFF		9
#define ELF_R_386_GOTPC			10

#define ELF_R_X86_64_NONE		0
#define ELF_R_X86_64_64			1
#define ELF_R_X86_64_PC32		2
#define ELF_R_X86_64_GOT32		3
#define ELF_R_X86_64_PLT32		4
#define ELF_R_X86_64_COPY		5
#define ELF_R_X86_64_GLOB_DAT		6
#define ELF_R_X86_64_JUMP_SLOT		7
#define ELF_R_X86_64_RELATIVE		8
#define ELF_R_X86_64_GOTPCREL		9
#define ELF_R_X86_64_32			10
#define ELF_R_X86_64_32S		11
#define ELF_R_X86_64_16			12
#define ELF_R_X86_64_PC16		13
#define ELF_R_X86_64_8			14
#define ELF_R_X86_64_PC8		15

struct ELF_REL32 {
	elf32_addr	r_offset;
	elf32_word	r_info;
} PACKED;

struct ELF_REL64 {
	elf64_addr	r_offset;
	union {
		elf64_word	r_info;
		struct {
			elf64_half r_type;
			elf64_half r_sym;
		};
	};
} PACKED;

struct ELF_RELA32 {
	elf32_addr	r_offset;
	elf32_word	r_info;
	elf32_sword	r_addend;
} PACKED;

struct ELF_RELA64 {
	elf64_addr	r_offset;
	union {
		elf64_word	r_info;
		struct {
			elf64_half r_type;
			elf64_half r_sym;
		};
	};
	elf64_sword	r_addend;
} PACKED;

extern byte ELF_HEADER_struct[];
extern byte ELF_HEADER32_struct[];
extern byte ELF_SECTION_HEADER32_struct[];
extern byte ELF_PROGRAM_HEADER32_struct[];
extern byte ELF_SYMBOL32_struct[];
extern byte ELF_REL32_struct[];
extern byte ELF_RELA32_struct[];
extern byte ELF_HEADER64_struct[];
extern byte ELF_SECTION_HEADER64_struct[];
extern byte ELF_PROGRAM_HEADER64_struct[];
extern byte ELF_SYMBOL64_struct[];
extern byte ELF_REL64_struct[];
extern byte ELF_RELA64_struct[];

struct ELFAddress {
	union {
		uint32 a32;
		uint64 a64;
	};
};

#endif /* __ELFSTRUC_H__ */
