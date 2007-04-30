/* 
 *	HT Editor
 *	nestruct.h
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

#ifndef __NESTRUCT_H_
#define __NESTRUCT_H_

#define NE_MAGIC0	'N'
#define NE_MAGIC1	'E'

#define NE_OS_UNKNOWN		0x0000
#define NE_OS_OS2			0x0001
#define NE_OS_WINDOWS		0x0002
#define NE_OS_DOS4			0x0003

#define NE_FLAGS_NOTAPROCESS	0x8000	  /* Not a process */
#define NE_FLAGS_ERRORS		0x2000	  /* Errors in image */
#define NE_FLAGS_SELFLOAD	0x0800

#define NE_FLAGS_NOTWINCOMPAT	0x0100	  /* Not compatible with P.M. Windowing */
#define NE_FLAGS_WINCOMPAT	0x0200	  /* Compatible with P.M. Windowing */
#define NE_FLAGS_WINAPI		0x0300	  /* Uses P.M. Windowing API */

#define NE_FLAGS_FLOAT		0x0080	  /* Floating-point instructions */
#define NE_FLAGS_I80386		0x0040	  /* 80386 instructions */
#define NE_FLAGS_I80286		0x0020	  /* 80286 instructions */
#define NE_FLAGS_I8086		0x0010	  /* 8086 instructions */
#define NE_FLAGS_PROT_ONLY	0x0008	  /* Runs in protected mode only */
#define NE_FLAGS_PPLI		0x0004	  /* Per-Process Library Initialization */
#define NE_FLAGS_INST		0x0002	  /* Instance data */
#define NE_FLAGS_SOLO		0x0001	  /* Solo data */

struct NE_HEADER {
	uint16   magic;                    // Magic number
	byte   ver;                      // Version number
	byte   rev;                      // Revision number
	uint16   enttab;                   // Offset of Entry Table
	uint16   cbenttab;                 // Number of bytes in Entry Table
	uint32  crc;                      // Checksum of whole file
	uint16   flags;                    // Flag uint16
	uint16   autodata;                 // Automatic data segment number
	uint16   heap;                     // Initial heap allocation
	uint16   stack;                    // Initial stack allocation
	uint32  csip;                     // Initial CS:IP setting
	uint32  sssp;                     // Initial SS:SP setting
	uint16   cseg;                     // Count of file segments
	uint16   cmod;                     // Entries in Module Reference Table
	uint16   cbnrestab;                // Size of non-resident name table
	uint16   segtab;                   // Offset of Segment Table
	uint16   rsrctab;                  // Offset of Resource Table
	uint16   restab;                   // Offset of resident name table
	uint16   modtab;                   // Offset of Module Reference Table
	uint16   imptab;                   // Offset of Imported Names Table
	uint32  nrestab;                  // Offset of Non-resident Names Table
	uint16   cmovent;                  // Count of movable entries
	uint16   align;                    // Segment alignment shift count
	uint16   cres;                     // Count of resource segments
	byte   os;                       // Target Operating system
	byte   flagsothers;              // Other .EXE flags
	uint16   pretthunks;               // offset to return thunks
	uint16   psegrefbytes;             // offset to segment ref. bytes
	uint16   swaparea;                 // Minimum code swap area size
	uint16   expver;                   // Expected Windows version number
} PACKED;

/*
 *	segment
 */

struct NE_SEGMENT {
	uint16 offset;
	uint16 size;
	uint16 flags;
	uint16 minalloc;
} PACKED;

#define NE_DATA		0x0001

#define NE_ITERATED		0x0008
#define NE_MOVABLE 		0x0010
#define NE_SHARABLE		0x0020
#define NE_PRELOAD 		0x0040
#define NE_READONLY		0x0080
#define NE_HASRELOC		0x0100
#define NE_CONFORMING	0x0200
/* 2 bits priv-level */
#define NE_DISCARDABLE	0x1000
#define NE_32BIT      	0x2000
#define NE_HUGE       	0x4000

/*
 *	entrypoint table
 */

struct NE_ENTRYPOINT_HEADER {
	byte entry_count;
	byte seg_index;
} PACKED;

/*  00H	     1	       Entry point flags
			  Bit(s)	  Significance
			  0	  0 = entry point not exported
				  1 = entry point exported
			  1	  0 = entry point uses instance data
				  1 = entry point uses single data
			  2	  Reserved
			  3Ä7	  Number of stack parameter words
  01H	     2	       Int 3FH instruction (CDH 3FH)
  03H	     1	       Segment number of entry point
  04H	     2	       Offset of entry point within segment
*/

#define NE_ENT_FLAGS_EXPORTED		0x01
#define NE_ENT_FLAGS_SINGLE_DATA	0x02

#define NE_ENT_FLAGS_PARAM_WORDS	0xf8

struct NE_ENTRYPOINT_MOVABLE {
	byte flags;
	uint16 int3f;
	byte seg;
	uint16 offset;
} PACKED;

/*  00H	     1	       Entry point flags (See above)
    01H	     2	       Offset of entry point within segment*/

struct NE_ENTRYPOINT_FIXED {
	byte flags;
	uint16 offset;
} PACKED;

/* relocations */

struct NE_RELOC_HEADER {
	byte type;
	byte flags;
	uint16 src_ofs;
} PACKED;

#define NE_RF_ADD			0x04		  /* Additive fixup */

#define NE_RF_RT_MASK		0x03		  /* Reference type mask */

#define NE_RF_INTERNAL		0x00		  /* Internal reference */
#define NE_RF_IMPORT_ORD		0x01		  /* Import by ordinal */
#define NE_RF_IMPORT_NAME	0x02		  /* Import by name */
#define NE_RF_OSFIXUP		0x03		  /* Operating system fixup */

#define NE_RT_MASK			0x0f		  /* Source type mask */
#define NE_RT_OFS8			0x00		  /* lo byte */
#define NE_RT_SEG16			0x02		  /* 16-bit segment */
#define NE_RT_PTR32			0x03		  /* 32-bit pointer */
#define NE_RT_OFS16			0x05		  /* 16-bit offset */
#define NE_RT_PTR48			0x0B		  /* 48-bit pointer */
#define NE_RT_OFS32			0x0D		  /* 32-bit offset */

struct NE_RELOC_INTERNAL {
	byte seg;
	byte reserved;
	uint16 ofs;
};

struct NE_RELOC_IMPORT {
	uint16 module;
	union {
		uint16 ord;
		uint16 name_ofs;
	};
};

struct NE_RELOC_FIXUP {
	uint16 type;
	uint16 reserved;
};

extern byte NE_HEADER_struct[];
extern byte NE_SEGMENT_struct[];
extern byte NE_ENTRYPOINT_HEADER_struct[];
extern byte NE_ENTRYPOINT_MOVABLE_struct[];
extern byte NE_ENTRYPOINT_FIXED_struct[];
extern byte NE_RELOC_HEADER_struct[];
extern byte NE_RELOC_INTERNAL_struct[];
extern byte NE_RELOC_IMPORT_struct[];
extern byte NE_RELOC_FIXUP_struct[];

#endif /* __NESTRUCT_H_ */

