/* 
 *	HT Editor
 *	machostruc.h
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

#ifndef __MACHOSTRUC_H__
#define __MACHOSTRUC_H__

#include "global.h"

struct MACHO_HEADER {
	byte	magic[4];
	uint32	cputype;
	uint32	cpusubtype;
	uint32	filetype;
	uint32	ncmds;
	uint32	sizeofcmds;
	uint32	flags;
};

/* Constants for the filetype field of the mach_header */
#define	MH_OBJECT	0x1		/* relocatable object file */
#define	MH_EXECUTE	0x2		/* demand paged executable file */
#define	MH_FVMLIB	0x3		/* fixed VM shared library file */
#define	MH_CORE		0x4		/* core file */
#define	MH_PRELOAD	0x5		/* preloaded executable file */
#define	MH_DYLIB	0x6		/* dynamicly bound shared library file*/
#define	MH_DYLINKER	0x7		/* dynamic link editor */
#define	MH_BUNDLE	0x8		/* dynamicly bound bundle file */

/* Constants for the flags field of the mach_header */
#define	MH_NOUNDEFS	0x1		/* the object file has no undefined references, can be executed */
#define	MH_INCRLINK	0x2		/* the object file is the output of an incremental link against a base file and can't be link edited again */
#define MH_DYLDLINK	0x4		/* the object file is input for the dynamic linker and can't be staticly link edited again */
#define MH_BINDATLOAD	0x8		/* the object file's undefined references are bound by the dynamic linker when loaded. */
#define MH_PREBOUND	0x10		/* the file has it's dynamic undefined references prebound. */

struct MACHO_COMMAND {
	uint32 cmd;			/* type of load command */
	uint32 cmdsize;			/* total size of command in bytes */
};

/* Constants for the cmd field of all load commands, the type */
#define	LC_SEGMENT	0x1	/* segment of this file to be mapped */
#define	LC_SYMTAB	0x2	/* link-edit stab symbol table info */
#define	LC_SYMSEG	0x3	/* link-edit gdb symbol table info (obsolete) */
#define	LC_THREAD	0x4	/* thread */
#define	LC_UNIXTHREAD	0x5	/* unix thread (includes a stack) */
#define	LC_LOADFVMLIB	0x6	/* load a specified fixed VM shared library */
#define	LC_IDFVMLIB	0x7	/* fixed VM shared library identification */
#define	LC_IDENT	0x8	/* object identification info (obsolete) */
#define LC_FVMFILE	0x9	/* fixed VM file inclusion (internal use) */
#define LC_PREPAGE      0xa     /* prepage command (internal use) */
#define	LC_DYSYMTAB	0xb	/* dynamic link-edit symbol table info */
#define	LC_LOAD_DYLIB	0xc	/* load a dynamicly linked shared library */
#define	LC_ID_DYLIB	0xd	/* dynamicly linked shared lib identification */
#define LC_LOAD_DYLINKER 0xe	/* load a dynamic linker */
#define LC_ID_DYLINKER	0xf	/* dynamic linker identification */
#define	LC_PREBOUND_DYLIB 0x10	/* modules prebound for a dynamicly linked shared library */

struct MACHO_SEGMENT_COMMAND {
	uint32	cmd;		/* LC_SEGMENT */
	uint32	cmdsize;	/* includes sizeof section structs */
	byte	segname[16];	/* segment name */
	uint32	vmaddr;		/* memory address of this segment */
	uint32	vmsize;		/* memory size of this segment */
	uint32	fileoff;	/* file offset of this segment */
	uint32	filesize;	/* amount to map from the file */
	uint32	maxprot;	/* maximum VM protection */
	uint32	initprot;	/* initial VM protection */
	uint32	nsects;		/* number of sections in segment */
	uint32	flags;		/* flags */
};

/* Constants for the flags field of the segment_command */
#define	SG_HIGHVM	0x1	/* the file contents for this segment is for the high part of the VM space, the low part is zero filled (for stacks in core files) */
#define	SG_FVMLIB	0x2	/* this segment is the VM that is allocated by a fixed VM library, for overlap checking in the link editor */
#define	SG_NORELOC	0x4	/* this segment has nothing that was relocated in it and nothing relocated to it, that is it maybe safely replaced without relocation */

struct MACHO_SECTION {
	byte	sectname[16];	/* name of this section */
	byte	segname[16];	/* segment this section goes in */
	uint32	vmaddr;		/* memory address of this section */
	uint32	vmsize;		/* size in bytes of this section */
	uint32	fileoff;	/* file offset of this section */
	uint32	align;		/* section alignment (power of 2) */
	uint32	reloff;		/* file offset of relocation entries */
	uint32	nreloc;		/* number of relocation entries */
	uint32	flags;		/* flags (section type and attributes)*/
	uint32	reserved1;	/* reserved */
	uint32	reserved2;	/* reserved */
};

/*
 * The flags field of a section structure is separated into two parts a section
 * type and section attributes.  The section types are mutually exclusive (it
 * can only have one type) but the section attributes are not (it may have more
 * than one attribute).
 */

#define MACHO_SECTION_TYPE		 0x000000ff	/* 256 section types */
#define MACHO_SECTION_ATTRIBUTES	 0xffffff00	/*  24 section attributes */

/* Constants for the type of a section */
#define	MACHO_S_REGULAR		0x0	/* regular section */
#define	MACHO_S_ZEROFILL		0x1	/* zero fill on demand section */
#define	MACHO_S_CSTRING_LITERALS	0x2	/* section with only literal C strings*/
#define	MACHO_S_4BYTE_LITERALS	0x3	/* section with only 4 byte literals */
#define	MACHO_S_8BYTE_LITERALS	0x4	/* section with only 8 byte literals */
#define	MACHO_S_LITERAL_POINTERS	0x5	/* section with only pointers to */
					/*  literals */
/*
 * For the two types of symbol pointers sections and the symbol stubs section
 * they have indirect symbol table entries.  For each of the entries in the
 * section the indirect symbol table entries, in corresponding order in the
 * indirect symbol table, start at the index stored in the reserved1 field
 * of the section structure.  Since the indirect symbol table entries
 * correspond to the entries in the section the number of indirect symbol table
 * entries is inferred from the size of the section divided by the size of the
 * entries in the section.  For symbol pointers sections the size of the entries
 * in the section is 4 bytes and for symbol stubs sections the byte size of the
 * stubs is stored in the reserved2 field of the section structure.
 */
#define	MACHO_S_NON_LAZY_SYMBOL_POINTERS	0x6	/* section with only non-lazy
						   symbol pointers */
#define	MACHO_S_LAZY_SYMBOL_POINTERS		0x7	/* section with only lazy symbol
						   pointers */
#define	MACHO_S_SYMBOL_STUBS			0x8	/* section with only symbol
						   stubs, byte size of stub in
						   the reserved2 field */
#define	MACHO_S_MOD_INIT_FUNC_POINTERS	0x9	/* section with only function
						   pointers for initialization*/
/*
 * Constants for the section attributes part of the flags field of a section
 * structure.
 */
#define MACHO_SECTION_ATTRIBUTES_USR	 0xff000000	/* User setable attributes */
#define MACHO_S_ATTR_PURE_INSTRUCTIONS 0x80000000	/* section contains only true
						   machine instructions */
#define MACHO_SECTION_ATTRIBUTES_SYS	 0x00ffff00	/* system setable attributes */
#define MACHO_S_ATTR_SOME_INSTRUCTIONS 0x00000400	/* section contains some
						   machine instructions */
#define MACHO_S_ATTR_EXT_RELOC	 0x00000200	/* section has external
						   relocation entries */
#define MACHO_S_ATTR_LOC_RELOC	 0x00000100	/* section has local
						   relocation entries */

struct MACHO_PPC_THREAD_STATE {
	uint32 srr0;	/* Instruction address register (PC) */
	uint32 srr1;	/* Machine state register (supervisor) */
	uint32 r0;
	uint32 r1;
	uint32 r2;
	uint32 r3;
	uint32 r4;
	uint32 r5;
	uint32 r6;
	uint32 r7;
	uint32 r8;
	uint32 r9;
	uint32 r10;
	uint32 r11;
	uint32 r12;
	uint32 r13;
	uint32 r14;
	uint32 r15;
	uint32 r16;
	uint32 r17;
	uint32 r18;
	uint32 r19;
	uint32 r20;
	uint32 r21;
	uint32 r22;
	uint32 r23;
	uint32 r24;
	uint32 r25;
	uint32 r26;
	uint32 r27;
	uint32 r28;
	uint32 r29;
	uint32 r30;
	uint32 r31;

	uint32 cr;      /* Condition register */
	uint32 xer;	/* User's integer exception register */
	uint32 lr;	/* Link register */
	uint32 ctr;	/* Count register */
	uint32 mq;	/* MQ register (601 only) */

	uint32 vrsave;	/* Vector Save Register */
};

union MACHO_THREAD_STATE {
	MACHO_PPC_THREAD_STATE ppc;
};

#define FLAVOR_PPC_THREAD_STATE		1
#define FLAVOR_PPC_FLOAT_STATE		2
#define FLAVOR_PPC_EXCEPTION_STATE	3
#define FLAVOR_PPC_VECTOR_STATE		4
#define FLAVOR_THREAD_STATE_NONE	7

struct MACHO_THREAD_COMMAND {
	uint32	cmd;		/* LC_THREAD or  LC_UNIXTHREAD */
	uint32	cmdsize;	/* total size of this command */
	uint32	flavor;		/* flavor of thread state */
	uint32	count;		/* count of longs in thread state */
	MACHO_THREAD_STATE state;
};

union MACHO_COMMAND_U {
	MACHO_COMMAND cmd;
	MACHO_SEGMENT_COMMAND segment;
	MACHO_THREAD_COMMAND thread;
};

struct MACHO_SYMTAB_COMMAND {
	uint32	cmd;		/* LC_SYMTAB */
	uint32	cmdsize;	/* sizeof(struct symtab_command) */
	uint32	symoff;		/* symbol table offset */
	uint32	nsyms;		/* number of symbol table entries */
	uint32	stroff;		/* string table offset */
	uint32	strsize;	/* string table size in bytes */
};

struct MACHO_SYMTAB_NLIST {
	uint32	strx;
	uint8	type;
	uint8	sect;
	uint16	desc;
	uint32	value;
};

// masks for type
#define MACHO_SYMBOL_N_STAB	0xe0
#define MACHO_SYMBOL_N_PEXT	0x10
#define MACHO_SYMBOL_N_TYPE	0x0e
#define MACHO_SYMBOL_N_EXT	0x01

#define MACHO_SYMBOL_TYPE_N_UNDF	0x00
#define MACHO_SYMBOL_TYPE_N_ABS		0x02
#define MACHO_SYMBOL_TYPE_N_INDR	0x0a
#define MACHO_SYMBOL_TYPE_N_PBUD	0x0c
#define MACHO_SYMBOL_TYPE_N_SECT	0x0e

extern byte MACHO_HEADER_struct[];
extern byte MACHO_COMMAND_struct[];
extern byte MACHO_SEGMENT_COMMAND_struct[];
extern byte MACHO_SECTION_struct[];
extern byte MACHO_THREAD_COMMAND_struct[];	// .state not included !
extern byte MACHO_PPC_THREAD_STATE_struct[];
extern byte MACHO_SYMTAB_COMMAND_struct[];
extern byte MACHO_SYMTAB_NLIST_struct[];

#endif /* __MACHOSTRUC_H__ */
