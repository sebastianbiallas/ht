/* 
 *	HT Editor
 *	coff_s.h
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

#ifndef __COFF_S_H_
#define __COFF_S_H_

#include "io/types.h"
#include "tools.h"

typedef unsigned int RVA;

struct COFF_HEADER {
	uint16 machine;
	uint16 section_count;
	uint32 timestamp;
	uint32 symbol_table_offset;
	uint32 symbol_count;
	uint16 optional_header_size;
	uint16 characteristics;
} PACKED;

#define COFF_RELOCS_STRIPPED		   	0x0001  // Relocation info stripped from file.
#define COFF_EXECUTABLE_IMAGE		   	0x0002  // File is executable  (i.e. no unresolved externel references).
#define COFF_LINE_NUMS_STRIPPED			0x0004  // Line nunbers stripped from file.
#define COFF_LOCAL_SYMS_STRIPPED	   	0x0008  // Local symbols stripped from file.
#define COFF_AGGRESIVE_WS_TRIM	  		0x0010  // Agressively trim working set
#define COFF_LARGE_ADDRESS	  		0x0020  // Large address aware
#define COFF_BYTES_REVERSED_LO	  		0x0080  // Bytes of machine uint16 are reversed.
#define COFF_32BIT_MACHINE			0x0100  // 32 bit uint16 machine.
#define COFF_DEBUG_STRIPPED		   	0x0200  // Debugging info stripped from file in .DBG file
#define COFF_REMOVABLE_RUN_FROM_SWAP   		0x0400  // If Image is on removable media, copy and run from the swap file.
#define COFF_NET_RUN_FROM_SWAP	   		0x0800  // If Image is on Net, copy and run from the swap file.
#define COFF_SYSTEM				0x1000  // System File.
#define COFF_DLL				0x2000  // File is a DLL.
#define COFF_UP_SYSTEM_ONLY		   	0x4000  // File should only be run on a UP machine
#define COFF_BYTES_REVERSED_HI	  		0x8000  // Bytes of machine uint16 are reversed.

#define COFF_MACHINE_UNKNOWN		   	0
#define COFF_MACHINE_I386		   	0x14c   // Intel 386
#define COFF_MACHINE_I486		   	0x14d   // Intel 486
#define COFF_MACHINE_I586		  	0x14e   // Intel 586
#define COFF_MACHINE_R3000BE			0x160   // MIPS big-endian
#define COFF_MACHINE_R3000			0x162   // MIPS little-endian
#define COFF_MACHINE_R4000			0x166   // MIPS little-endian
#define COFF_MACHINE_R10000		   	0x168   // MIPS little-endian
#define COFF_MACHINE_ALPHA			0x184   // Alpha_AXP
#define COFF_MACHINE_SH3			0x1a2   // Hitachi SH3
#define COFF_MACHINE_SH4		   	0x1a6   // Hitachi SH4
#define COFF_MACHINE_ARM			0x1c0   // ARM
#define COFF_MACHINE_THUMB			0x1c2   // THUMB
#define COFF_MACHINE_POWERPC_BE		   	0x1df   // IBM PowerPC Big-Endian (?)
#define COFF_MACHINE_POWERPC_LE		   	0x1f0   // IBM PowerPC Little-Endian
#define COFF_MACHINE_POWERPC64_BE		0x1f2   // IBM PowerPC64 Big-Endian (?)
#define COFF_MACHINE_IA64			0x200   // Intel IA64
#define COFF_MACHINE_MIPS16			0x266   // MIPS16
#define COFF_MACHINE_68k			0x268   // Motorola 68k
#define COFF_MACHINE_ALPHA_AXP_64		0x284   // Alpha AXP 64
#define COFF_MACHINE_MIPSf			0x366   // MIPSf
#define COFF_MACHINE_MIPS16f			0x466   // MIPS16f
#define COFF_MACHINE_AMD64			0x8664  // AMD 64

// FIXME: not yet implemented: XCOFF64, no sample file available
//#define COFF_MACHINE_POWERPC64_BE	   	0x1ef   // XCOFF 64Bit Big-Endian (PowerPC only ?)

//
// Optional header format.
//

#define COFF_OPTSIZE_0				0x00

#define COFF_OPTSIZE_COFF32			0x1c
#define COFF_OPTSIZE_XCOFF32			0x48

#define COFF_OPTSIZE_PE32			0xe0


#define COFF_OPTMAGIC_ROMIMAGE			0x107

#define COFF_OPTMAGIC_PE32			0x10b
#define COFF_OPTMAGIC_COFF32			0x10b

#define COFF_OPTMAGIC_PE64			0x20b

struct	COFF_OPTIONAL_HEADER32 {
	uint16 magic;
	byte major_linker_version;
	byte minor_linker_version;
	uint32 code_size;
	uint32 data_size;
	uint32 bss_size;
	uint32 entrypoint_address;
	uint32 code_base;
	uint32 data_base;
} PACKED;

/*
 *	same as COFF_OPTIONAL_HEADER32 but no data_base
 */
struct	COFF_OPTIONAL_HEADER64 {
	uint16 magic;
	byte major_linker_version;
	byte minor_linker_version;
	uint32 code_size;
	uint32 data_size;
	uint32 bss_size;
	uint32 entrypoint_address;
	uint32 code_base;
} PACKED;

/*
 *	Section header
 */

#define COFF_SIZEOF_SHORT_NAME			8

struct COFF_SECTION_HEADER {
	byte name[COFF_SIZEOF_SHORT_NAME];
	uint32 data_vsize;	// or data_phys_address !
	uint32 data_address;
	uint32 data_size;
	uint32 data_offset;
	uint32 relocation_offset;
	uint32 linenumber_offset;
	uint16 relocation_count;
	uint16 linenumber_count;
	uint32 characteristics;
} PACKED;

#define COFF_SIZEOF_SECTION_HEADER		40

/*
 * Section characteristics.
 */

//      COFF_SCN_TYPE_REG			0x00000000  // Reserved.
//      COFF_SCN_TYPE_DSECT			0x00000001  // Reserved.
//      COFF_SCN_TYPE_NOLOAD			0x00000002  // Reserved.
//      COFF_SCN_TYPE_GROUP			0x00000004  // Reserved.
#define COFF_SCN_TYPE_NO_PAD			0x00000008  // Reserved.
//      COFF_SCN_TYPE_COPY			0x00000010  // Reserved.

#define COFF_SCN_CNT_CODE			0x00000020	// Section contains code.
#define COFF_SCN_CNT_INITIALIZED_DATA		0x00000040	// Section contains initialized data.
#define COFF_SCN_CNT_UNINITIALIZED_DATA		0x00000080	// Section contains uninitialized data.

#define COFF_SCN_LNK_OTHER			0x00000100	// Reserved.
#define COFF_SCN_LNK_INFO			0x00000200	// Section contains comments or some other type of information.
//      COFF_SCN_TYPE_OVER			0x00000400	// Reserved.
#define COFF_SCN_LNK_REMOVE			0x00000800	// Section contents will not become part of image.
#define COFF_SCN_LNK_COMDAT			0x00001000	// Section contents comdat.
//						0x00002000	// Reserved.

//      COFF_SCN_MEM_PROTECTED - Obsolete	0x00004000
#define COFF_SCN_MEM_FARDATA			0x00008000
//      COFF_SCN_MEM_SYSHEAP  - Obsolete	0x00010000
#define COFF_SCN_MEM_PURGEABLE			0x00020000
#define COFF_SCN_MEM_16BIT			0x00020000
#define COFF_SCN_MEM_LOCKED			0x00040000
#define COFF_SCN_MEM_PRELOAD			0x00080000

#define COFF_SCN_ALIGN_1BYTES			0x00100000	//
#define COFF_SCN_ALIGN_2BYTES			0x00200000	//
#define COFF_SCN_ALIGN_4BYTES			0x00300000	//
#define COFF_SCN_ALIGN_8BYTES			0x00400000	//
#define COFF_SCN_ALIGN_16BYTES		   	0x00500000	// Default alignment if no others are specified.
#define COFF_SCN_ALIGN_32BYTES		   	0x00600000	//
#define COFF_SCN_ALIGN_64BYTES		   	0x00700000	//
// Unused					0x00800000

#define COFF_SCN_LNK_NRELOC_OVFL		0x01000000	// Section contains extended relocations.
#define COFF_SCN_MEM_DISCARDABLE		0x02000000	// Section can be discarded.
#define COFF_SCN_MEM_NOT_CACHED			0x04000000	// Section is not cachable.
#define COFF_SCN_MEM_NOT_PAGED			0x08000000	// Section is not pageable.
#define COFF_SCN_MEM_SHARED			0x10000000	// Section is shareable.
#define COFF_SCN_MEM_EXECUTE			0x20000000	// Section is executable.
#define COFF_SCN_MEM_READ			0x40000000	// Section is readable.
#define COFF_SCN_MEM_WRITE			0x80000000	// Section is writeable.

extern byte COFF_HEADER_struct[];
extern byte COFF_OPTIONAL_HEADER32_struct[];
extern byte COFF_OPTIONAL_HEADER64_struct[];
extern byte COFF_SECTION_HEADER_struct[];

#endif /* !__COFF_S_H_ */
