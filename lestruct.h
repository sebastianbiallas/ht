/* 
 *	HT Editor
 *	lestruct.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __LESTRUCT_H_
#define __LESTRUCT_H_

#define IMAGE_LE_CPU_286	0x01
#define IMAGE_LE_CPU_386	0x02
#define IMAGE_LE_CPU_486	0x03
#define IMAGE_LE_CPU_586	0x04
#define IMAGE_LE_CPU_N10	0x20
#define IMAGE_LE_CPU_N11	0x21
#define IMAGE_LE_CPU_R2000 0x40
#define IMAGE_LE_CPU_R6000 0x41
#define IMAGE_LE_CPU_R4000 0x42

#define IMAGE_LE_OS_OS2 0x01
#define IMAGE_LE_OS_WIN 0x02
#define IMAGE_LE_OS_DOS4	0x03
#define IMAGE_LE_OS_WIN386 0x04

#define LE_MAGIC0	'L'
#define LE_MAGIC1	'E'

struct IMAGE_LE_HEADER {
	word		magic;				// Magic number
	byte		border;				// The byte ordering for the VXD
	byte		worder;				// The word ordering for the VXD
	dword	level;				// The EXE format level for now = 0
	word		cpu;					// The CPU type
	word		os;					// The OS type
	dword	ver;					// Module version
	dword	mflags;				// Module flags
	dword	pagecnt;				// Module # pages
	dword	startobj;				// Object # for instruction pointer
	dword	eip;					// Extended instruction pointer
	dword	stackobj;				// Object # for stack pointer
	dword	esp;					// Extended stack pointer
	dword	pagesize;				// VXD page size
	dword	lastpagesize;			// Last page size in VXD
	dword	fixupsize;			// Fixup section size
	dword	fixupsum;				// Fixup section checksum
	dword	ldrsize;				// Loader section size
	dword	ldrsum;				// Loader section checksum
	dword	objtab;				// Object table offset
	dword	objcnt;				// Number of objects in module
	dword	pagemap;				// Object page map offset
	dword	itermap;				// Object iterated data map offset
	dword	rsrctab;				// Offset of Resource Table
	dword	rsrccnt;				// Number of resource entries
	dword	restab;				// Offset of resident name table
	dword	enttab;				// Offset of Entry Table
	dword	dirtab;				// Offset of Module Directive Table
	dword	dircnt;				// Number of module directives
	dword	fpagetab;				// Offset of Fixup Page Table
	dword	frectab;				// Offset of Fixup Record Table
	dword	impmod;				// Offset of Import Module Name Table
	dword	impmodcnt;			// Number of entries in Import Module Name Table
	dword	impproc;				// Offset of Import Procedure Name Table
	dword	pagesum;				// Offset of Per-Page Checksum Table
	dword	datapage;				// Offset of Enumerated Data Pages
	dword	preload;				// Number of preload pages
	dword	nrestab;				// Offset of Non-resident Names Table
	dword	cbnrestab;			// Size of Non-resident Name Table
	dword	nressum;				// Non-resident Name Table Checksum
	dword	autodata;				// Object # for automatic data object
	dword	debuginfo;			// Offset of the debugging information
	dword	debuglen;				// The length of the debugging info. in bytes
	dword	instpreload;			// Number of instance pages in preload section of VXD file
	dword	instdemand;			// Number of instance pages in demand load section of VXD file
	dword	heapsize;				// Size of heap - for 16-bit apps
	byte		res3[12];				// Reserved words
	dword	winresoff;
	dword	winreslen;
	word		devid;				// Device ID for VxD
	word		ddkver;				// DDK version for VxD
  };

/*
 ³ ³ ³ ³    ³ ÀÂÙ     ³ ³ ³ ³  ³ ³ ³ ÀÄ0Ä 1: Readable
 ³ ³ ³ ³    ³  ³      ³ ³ ³ ³  ³ ³ ÀÄÄÄ1Ä 1: Writable
 ³ ³ ³ ³    ³  ³      ³ ³ ³ ³  ³ ÀÄÄÄÄÄ2Ä 1: Executable
 ³ ³ ³ ³    ³  ³      ³ ³ ³ ³  ÀÄÄÄÄÄÄÄ3Ä 1: Resource
 ³ ³ ³ ³    ³  ³      ³ ³ ³ ÀÄÄÄÄÄÄÄÄÄÄ4Ä 1: Discardable
 ³ ³ ³ ³    ³  ³      ³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄ5Ä 1: Shared
 ³ ³ ³ ³    ³  ³      ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄ6Ä 1: Preloaded
 ³ ³ ³ ³    ³  ³      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ7Ä 1: Invalid
 ³ ³ ³ ³    ³  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ8,9ÄÄ Type: 00 - Normal
 ³ ³ ³ ³    ³                                    01 - Zero-filled
 ³ ³ ³ ³    ³                                    10 - Resident
 ³ ³ ³ ³    ³                                    11 - Resident/contiguous
 ³ ³ ³ ³    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ10Ä "RESIDENT/LONG_LOCABLE"
 ³ ³ ³ ³
 ³ ³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ12Ä "16:16_ALIAS"
 ³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ13Ä 1: "BIG" (USE32)
 ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ14Ä 1: Conforming
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ15Ä "Object_i/o_privilege_level"
*/

#define IMAGE_LE_OBJECT_FLAG_READABLE		0x00000001
#define IMAGE_LE_OBJECT_FLAG_WRITEABLE		0x00000002
#define IMAGE_LE_OBJECT_FLAG_EXECUTABLE		0x00000004
#define IMAGE_LE_OBJECT_FLAG_RESOURCE		0x00000008
#define IMAGE_LE_OBJECT_FLAG_DISCARDABLE	0x00000010
#define IMAGE_LE_OBJECT_FLAG_SHARED		0x00000020
#define IMAGE_LE_OBJECT_FLAG_PRELOADED		0x00000040
#define IMAGE_LE_OBJECT_FLAG_INVALID		0x00000080

#define IMAGE_LE_OBJECT_FLAG_USE32			0x00002000

#define IMAGE_LE_SIZEOF_HEADER	   196

struct IMAGE_LE_OBJECT_HEADER	{
	dword	virtual_size;
	dword	base_reloc_addr;
	dword	flags;
	dword	page_map_index;
	dword	page_map_count;
	byte		name[4];
};

struct IMAGE_LE_PAGE_MAP_ENTRY {
	word high;
	byte	low;
	byte	flags;
};


#define IMAGE_LE_ENTRYPOINT_VALID	1
#define IMAGE_LE_ENTRYPOINT_32BIT	2

struct IMAGE_LE_ENTRYPOINT_HEADER {
	byte entry_count;
	byte flags;
	word obj_index;
};

#define IMAGE_LE_SIZEOF_OBJECT_HEADER	24

/*
 *	internal
 */

struct ht_le_objmap {
	IMAGE_LE_OBJECT_HEADER *header;
	dword *psize;
	dword *vsize;
	int count;
};

struct ht_le_pagemap {
	dword *offset;
	dword *psize;
	dword *vsize;
	int count;
};

#endif /* __LESTRUCT_H_ */
