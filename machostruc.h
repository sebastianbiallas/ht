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
#define	LC_PREBOUND_DYLIB 0x10	/* modules prebound for a dynamicly */
				/*  linked shared library */

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

extern byte MACHO_HEADER_struct[];
extern byte MACHO_COMMAND_struct[];
extern byte MACHO_SEGMENT_COMMAND_struct[];

#endif /* __MACHOSTRUC_H__ */
