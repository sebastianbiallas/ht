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

#include "io/types.h"

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
	uint16		magic PACKED;				// Magic number
	byte		border PACKED;				// The byte ordering for the VXD
	byte		worder PACKED;				// The uint16 ordering for the VXD
	uint32	level PACKED;				// The EXE format level for now = 0
	uint16		cpu PACKED;					// The CPU type
	uint16		os PACKED;					// The OS type
	uint32	ver PACKED;					// Module version
	uint32	mflags PACKED;				// Module flags
	uint32	pagecnt PACKED;				// Module # pages
	uint32	startobj PACKED;				// Object # for instruction pointer
	uint32	eip PACKED;					// Extended instruction pointer
	uint32	stackobj PACKED;				// Object # for stack pointer
	uint32	esp PACKED;					// Extended stack pointer
	uint32	pagesize PACKED;				// VXD page size
	uint32	lastpagesize PACKED;			// Last page size in VXD
	uint32	fixupsize PACKED;				// Fixup section size
	uint32	fixupsum PACKED;				// Fixup section checksum
	uint32	ldrsize PACKED;				// Loader section size
	uint32	ldrsum PACKED;				// Loader section checksum
	uint32	objtab PACKED;				// Object table offset
	uint32	objcnt PACKED;				// Number of objects in module
	uint32	pagemap PACKED;				// Object page map offset
	uint32	itermap PACKED;				// Object iterated data map offset
	uint32	rsrctab PACKED;				// Offset of Resource Table
	uint32	rsrccnt PACKED;				// Number of resource entries
	uint32	restab PACKED;				// Offset of resident name table
	uint32	enttab PACKED;				// Offset of Entry Table
	uint32	dirtab PACKED;				// Offset of Module Directive Table
	uint32	dircnt PACKED;				// Number of module directives
	uint32	fpagetab PACKED;				// Offset of Fixup Page Table
	uint32	frectab PACKED;				// Offset of Fixup Record Table
	uint32	impmod PACKED;				// Offset of Import Module Name Table
	uint32	impmodcnt PACKED;				// Number of entries in Import Module Name Table
	uint32	impproc PACKED;				// Offset of Import Procedure Name Table
	uint32	pagesum PACKED;				// Offset of Per-Page Checksum Table
	uint32	datapage PACKED;				// Offset of Enumerated Data Pages
	uint32	preload PACKED;				// Number of preload pages
	uint32	nrestab PACKED;				// Offset of Non-resident Names Table
	uint32	cbnrestab PACKED;				// Size of Non-resident Name Table
	uint32	nressum PACKED;				// Non-resident Name Table Checksum
	uint32	autodata PACKED;				// Object # for automatic data object
	uint32	debuginfo PACKED;				// Offset of the debugging information
	uint32	debuglen PACKED;				// The length of the debugging info. in bytes
	uint32	instpreload PACKED;			// Number of instance pages in preload section of VXD file
	uint32	instdemand PACKED;			// Number of instance pages in demand load section of VXD file
	uint32	heapsize PACKED;				// Size of heap - for 16-bit apps
	byte		res3[12] PACKED;				// Reserved words
	uint32	winresoff PACKED;
	uint32	winreslen PACKED;
	uint16		devid PACKED;				// Device ID for VxD
	uint16		ddkver PACKED;				// DDK version for VxD
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
	uint32	vsize PACKED;
	uint32	base_reloc_addr PACKED;
	uint32	flags PACKED;
	uint32	page_map_index PACKED;
	uint32	page_map_count PACKED;
	byte		name[4] PACKED;
};

struct LE_PAGE_MAP_ENTRY {
	uint16 high PACKED;
	byte	low PACKED;
	byte	flags PACKED;
};

/*
 *	ENTRYPOINTS
 */

#define LE_ENTRYPOINT_BUNDLE_VALID		(1<<0)
#define LE_ENTRYPOINT_BUNDLE_32BIT		(1<<1)

struct LE_ENTRYPOINT_BUNDLE {
	byte entry_count PACKED;
	byte flags PACKED;
	uint16 obj_index PACKED;
};

#define LE_ENTRYPOINT_EXPORTED	(1<<0)
#define LE_ENTRYPOINT_SHARED		(1<<1)

struct LE_ENTRYPOINT16 {
	byte flags PACKED;
	uint16 offset PACKED;
};

struct LE_ENTRYPOINT32 {
	byte flags PACKED;
	uint32 offset PACKED;
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
	uint8	address_type PACKED;
	uint8	reloc_type PACKED;
};

// if address_type == 8, reloc_type = 0
struct LE_FIXUP_INTERNAL16 {
	uint8	seg PACKED;
	uint16	ofs PACKED;
};

struct LE_FIXUP_INTERNAL32 {
	uint8	seg PACKED;
	uint32	ofs PACKED;
};

/*
 *	VxD specific
 */

struct LE_VXD_DESCRIPTOR {
	uint32	next PACKED;
	uint16	sdk_version PACKED;
	uint16	device_number PACKED;
	uint8	version_major PACKED;
	uint8	version_minor PACKED;
	uint16	flags PACKED;
	uint8	name[8] PACKED;		// not (null-)terminated, fill with spaces
	uint32	init_order PACKED;
	uint32    ctrl_ofs PACKED;
	uint32    v86_ctrl_ofs PACKED;
	uint32    pm_ctrl_ofs PACKED;
	uint32    v86_ctrl_csip PACKED;
	uint32    pm_ctrl_csip PACKED;
	uint32	rm_ref_data PACKED;	// "Reference data from real mode"
	uint32	service_table_ofs PACKED;
	uint32	service_table_size PACKED;
	uint32	win32_service_table_ofs PACKED;
	uint32	prev PACKED;			// normally 'verP' (Prev)
	uint32	size PACKED;			// size of this structure (0x50 = 80)
	uint32	reserved0 PACKED;		// normally '1vsR' (Rsv1)
	uint32	reserved1 PACKED;		// normally '2vsR' (Rsv2)
	uint32	reserved2 PACKED;		// normally '3vsR' (Rsv3)
};

/*
 *	internal
 */

struct ht_le_objmap {
	LE_OBJECT *header;
	uint *psize;
	uint *vsize;
	uint count;
};

struct ht_le_pagemap {
	uint *offset;
	uint *psize;
	uint *vsize;
	uint count;
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

