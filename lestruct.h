/* 
 *	HT Editor
 *	lestruct.h
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

#ifndef __LESTRUCT_H_
#define __LESTRUCT_H_

#include "global.h"

#define LE_CPU_286	0x01
#define LE_CPU_386	0x02
#define LE_CPU_486	0x03
#define LE_CPU_586	0x04
#define LE_CPU_N10	0x20
#define LE_CPU_N11	0x21
#define LE_CPU_R2000 0x40
#define LE_CPU_R6000 0x41
#define LE_CPU_R4000 0x42

#define LE_OS_OS2 0x01
#define LE_OS_WIN 0x02
#define LE_OS_DOS4	0x03
#define LE_OS_WIN386 0x04

#define LE_MAGIC0	'L'
#define LE_MAGIC1	'E'

#define LE_SIZEOF_HEADER				196

struct LE_HEADER {
	word		magic HTPACKED;				// Magic number
	byte		border HTPACKED;				// The byte ordering for the VXD
	byte		worder HTPACKED;				// The word ordering for the VXD
	dword	level HTPACKED;				// The EXE format level for now = 0
	word		cpu HTPACKED;					// The CPU type
	word		os HTPACKED;					// The OS type
	dword	ver HTPACKED;					// Module version
	dword	mflags HTPACKED;				// Module flags
	dword	pagecnt HTPACKED;				// Module # pages
	dword	startobj HTPACKED;				// Object # for instruction pointer
	dword	eip HTPACKED;					// Extended instruction pointer
	dword	stackobj HTPACKED;				// Object # for stack pointer
	dword	esp HTPACKED;					// Extended stack pointer
	dword	pagesize HTPACKED;				// VXD page size
	dword	lastpagesize HTPACKED;			// Last page size in VXD
	dword	fixupsize HTPACKED;				// Fixup section size
	dword	fixupsum HTPACKED;				// Fixup section checksum
	dword	ldrsize HTPACKED;				// Loader section size
	dword	ldrsum HTPACKED;				// Loader section checksum
	dword	objtab HTPACKED;				// Object table offset
	dword	objcnt HTPACKED;				// Number of objects in module
	dword	pagemap HTPACKED;				// Object page map offset
	dword	itermap HTPACKED;				// Object iterated data map offset
	dword	rsrctab HTPACKED;				// Offset of Resource Table
	dword	rsrccnt HTPACKED;				// Number of resource entries
	dword	restab HTPACKED;				// Offset of resident name table
	dword	enttab HTPACKED;				// Offset of Entry Table
	dword	dirtab HTPACKED;				// Offset of Module Directive Table
	dword	dircnt HTPACKED;				// Number of module directives
	dword	fpagetab HTPACKED;				// Offset of Fixup Page Table
	dword	frectab HTPACKED;				// Offset of Fixup Record Table
	dword	impmod HTPACKED;				// Offset of Import Module Name Table
	dword	impmodcnt HTPACKED;				// Number of entries in Import Module Name Table
	dword	impproc HTPACKED;				// Offset of Import Procedure Name Table
	dword	pagesum HTPACKED;				// Offset of Per-Page Checksum Table
	dword	datapage HTPACKED;				// Offset of Enumerated Data Pages
	dword	preload HTPACKED;				// Number of preload pages
	dword	nrestab HTPACKED;				// Offset of Non-resident Names Table
	dword	cbnrestab HTPACKED;				// Size of Non-resident Name Table
	dword	nressum HTPACKED;				// Non-resident Name Table Checksum
	dword	autodata HTPACKED;				// Object # for automatic data object
	dword	debuginfo HTPACKED;				// Offset of the debugging information
	dword	debuglen HTPACKED;				// The length of the debugging info. in bytes
	dword	instpreload HTPACKED;			// Number of instance pages in preload section of VXD file
	dword	instdemand HTPACKED;			// Number of instance pages in demand load section of VXD file
	dword	heapsize HTPACKED;				// Size of heap - for 16-bit apps
	byte		res3[12] HTPACKED;				// Reserved words
	dword	winresoff HTPACKED;
	dword	winreslen HTPACKED;
	word		devid HTPACKED;				// Device ID for VxD
	word		ddkver HTPACKED;				// DDK version for VxD
  };

/*
 1 1 1 1  1 1
 5 4 3 2  1 0 9 8     7 6 5 4  3 2 1 0
 ş ş ş ş  ş ş ş ş     ş ş ş ş  ş ş ş ş
 ³   ³      ³ ³ ³         ³ ³    ³
 ³   ³      ÀÄÅÄÙ         ³ ³    ÀÄÄÄÄÄ2ÄInitialization ( Only for DLL ):
 ³   ³        ³           ³ ³              0: Global
 ³   ³        ³           ³ ³              1: Per-Process
 ³   ³        ³           ³ ³
 ³   ³        ³           ³ ÀÄÄÄÄÄÄÄÄÄÄ4Ä1:No internal fixup in exe image
 ³   ³        ³           ÀÄÄÄÄÄÄÄÄÄÄÄÄ5Ä1:No external fixup in exe image
 ³   ³        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ8,9,10Ä  0 - Unknown
 ³   ³                                      1 - Incompatible with PM windowing
 ³   ³                                      2 -   Compatible with PM windowing
 ³   ³                                      3 - Uses PM windowing API
 ³   ³
 ³   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ13Ä1: Module not loadable
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ15Ä1: Module is DLL rather then program
*/
#define LE_MODULE_FLAG_INIT_PER_PROCESS		(1<<2)

#define LE_MODULE_FLAG_NO_EXT_FIXUP          (1<<4)
#define LE_MODULE_FLAG_NO_INT_FIXUP          (1<<5)

#define LE_MODULE_FLAG_WINDOWING_MASK        (7<<8)

#define LE_MODULE_FLAG_NOT_LOADABLE          (1<<13)
#define LE_MODULE_FLAG_LIBRARY               (1<<15)

/*
 *	LE Objects (aka segments, aka sections)
 */

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

#define LE_OBJECT_FLAG_READABLE         (1<<0)
#define LE_OBJECT_FLAG_WRITEABLE        (1<<1)
#define LE_OBJECT_FLAG_EXECUTABLE       (1<<2)
#define LE_OBJECT_FLAG_RESOURCE         (1<<3)
#define LE_OBJECT_FLAG_DISCARDABLE      (1<<4)
#define LE_OBJECT_FLAG_SHARED           (1<<5)
#define LE_OBJECT_FLAG_PRELOADED        (1<<6)
#define LE_OBJECT_FLAG_INVALID          (1<<7)

#define LE_OBJECT_FLAG_USE32            (1<<13)

#define LE_SIZEOF_OBJECT				24

struct LE_OBJECT	{
	dword	vsize HTPACKED;
	dword	base_reloc_addr HTPACKED;
	dword	flags HTPACKED;
	dword	page_map_index HTPACKED;
	dword	page_map_count HTPACKED;
	byte		name[4] HTPACKED;
};

struct LE_PAGE_MAP_ENTRY {
	word high HTPACKED;
	byte	low HTPACKED;
	byte	flags HTPACKED;
};

/*
 *	ENTRYPOINTS
 */

#define LE_ENTRYPOINT_BUNDLE_VALID		(1<<0)
#define LE_ENTRYPOINT_BUNDLE_32BIT		(1<<1)

struct LE_ENTRYPOINT_BUNDLE {
	byte entry_count HTPACKED;
	byte flags HTPACKED;
	word obj_index HTPACKED;
};

#define LE_ENTRYPOINT_EXPORTED	(1<<0)
#define LE_ENTRYPOINT_SHARED		(1<<1)

struct LE_ENTRYPOINT16 {
	byte flags HTPACKED;
	word offset HTPACKED;
};

struct LE_ENTRYPOINT32 {
	byte flags HTPACKED;
	dword offset HTPACKED;
};

/*
 *	FIXUPS
 */

#define LE_FIXUP_ADDR_TYPE_MASK		(15<<0)
#define LE_FIXUP_ADDR_TYPE_0_8		0
#define LE_FIXUP_ADDR_TYPE_16_0		2
#define LE_FIXUP_ADDR_TYPE_16_16		3
#define LE_FIXUP_ADDR_TYPE_0_16		5
#define LE_FIXUP_ADDR_TYPE_16_32		6
#define LE_FIXUP_ADDR_TYPE_0_32		7
#define LE_FIXUP_ADDR_TYPE_REL32		8
#define LE_FIXUP_ADDR_16_16			(1<<4)
#define LE_FIXUP_ADDR_MULTIPLE		(1<<5)

#define   LE_FIXUP_RELOC_TYPE_MASK		(3<<0)
#define        LE_FIXUP_RELOC_TYPE_INTERNAL		0
#define        LE_FIXUP_RELOC_TYPE_IMPORT_ORD	1
#define        LE_FIXUP_RELOC_TYPE_IMPORT_NAME	2
#define        LE_FIXUP_RELOC_TYPE_OSFIXUP		3	// ?
#define   LE_FIXUP_RELOC_ADDITIVE		(1<<2)
#define   LE_FIXUP_RELOC_TARGET32		(1<<4)
#define   LE_FIXUP_RELOC_ADDITIVE32	(1<<5)
#define   LE_FIXUP_RELOC_ORDINAL16		(1<<6)
#define   LE_FIXUP_RELOC_IORD8		(1<<7)

struct LE_FIXUP {
	uint8	address_type HTPACKED;
	uint8	reloc_type HTPACKED;
};

// if address_type == 8, reloc_type = 0
struct LE_FIXUP_INTERNAL16 {
	uint8	seg HTPACKED;
	uint16	ofs HTPACKED;
};

struct LE_FIXUP_INTERNAL32 {
	uint8	seg HTPACKED;
	uint32	ofs HTPACKED;
};

/*
 *	VxD specific
 */

struct LE_VXD_DESCRIPTOR {
	uint32	next HTPACKED;
	uint16	sdk_version HTPACKED;
	uint16	device_number HTPACKED;
	uint8	version_major HTPACKED;
	uint8	version_minor HTPACKED;
	uint16	flags HTPACKED;
	uint8	name[8] HTPACKED;		// not (null-)terminated, fill with spaces
	uint32	init_order HTPACKED;
	uint32    ctrl_ofs HTPACKED;
	uint32    v86_ctrl_ofs HTPACKED;
	uint32    pm_ctrl_ofs HTPACKED;
	uint32    v86_ctrl_csip HTPACKED;
	uint32    pm_ctrl_csip HTPACKED;
	uint32	rm_ref_data HTPACKED;	// "Reference data from real mode"
	uint32	service_table_ofs HTPACKED;
	uint32	service_table_size HTPACKED;
	uint32	win32_service_table_ofs HTPACKED;
	uint32	prev HTPACKED;			// normally 'verP' (Prev)
	uint32	size HTPACKED;			// size of this structure (0x50 = 80)
	uint32	reserved0 HTPACKED;		// normally '1vsR' (Rsv1)
	uint32	reserved1 HTPACKED;		// normally '2vsR' (Rsv2)
	uint32	reserved2 HTPACKED;		// normally '3vsR' (Rsv3)
};

/*
 *	internal
 */

struct ht_le_objmap {
	LE_OBJECT *header;
	UINT *psize;
	UINT *vsize;
	UINT count;
};

struct ht_le_pagemap {
	UINT *offset;
	UINT *psize;
	UINT *vsize;
	UINT count;
};

/**/

extern byte LE_HEADER_struct[];
extern byte LE_FIXUP_struct[];
extern byte LE_ENTRYPOINT16_struct[];
extern byte LE_ENTRYPOINT32_struct[];
extern byte LE_VXD_DESCRIPTOR_struct[];
extern byte LE_FIXUP_INTERNAL_struct[];
extern byte LE_OBJECT_HEADER_struct[];
extern byte LE_PAGE_MAP_ENTRY_struct[];
extern byte LE_FIXUP_INTERNAL16_struct[];
extern byte LE_FIXUP_INTERNAL32_struct[];

#endif /* __LESTRUCT_H_ */

