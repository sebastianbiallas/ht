/* 
 *	HT Editor
 *	peffstruc.h
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

/*	mostly assembled from TIS ELF 1.1g and BFD-ELF 	*/

#ifndef __PEFFSTRUC_H__
#define __PEFFSTRUC_H__

#include "global.h"

enum PEF_ARCH {
	PEFARCH_PowerPC,
	PEFARCH_M68K
};

struct PEF_CONTAINER_HEADER {
	byte	tag1[4] HTPACKED;
	byte	tag2[4] HTPACKED;
	byte	architecture[4] HTPACKED;
	uint32	formatVersion HTPACKED;	// normally 1
	uint32	dateTimeStamp HTPACKED;	// number of seconds measured from January 1, 1904
	uint32	oldDefVersion HTPACKED;
	uint32	oldImpVersion HTPACKED;
	uint32	currentVersion HTPACKED;
	uint16	sectionCount HTPACKED;
	uint16	instSectionCount HTPACKED;
	uint32	reservedA HTPACKED;
};

struct PEF_SECTION_HEADER {
	sint32	nameOffset;	// 0xffffffff for no name
	uint32	defaultAddress;
	uint32	totalSize;
	uint32	unpackedSize;
	uint32	packedSize;
	uint32	containerOffset;
	byte	sectionKind;
	byte	shareKind;
	byte	alignment;
	byte	reservedA;
};

// sectionKind
#define PEF_SK_Code		0	// R-x
#define PEF_SK_UnpackedData	1	// RW-
#define PEF_SK_PatternInitData	2	// RW-
#define PEF_SK_ConstData	3	// R--
#define PEF_SK_Loader		4
#define PEF_SK_Debug		5
#define PEF_SK_ExecutableData	6	// RWX
#define PEF_SK_Exception	7
#define PEF_SK_Traceback	8

// shareKind
#define PEF_SHK_ProcessShare	1
#define PEF_SHK_GlobalShare	4
#define PEF_SHK_ProtectedShare	5

struct PEF_LOADER_INFO_HEADER {
	sint32	mainSection;          
	uint32	mainOffset;           
	sint32	initSection;          
	uint32	initOffset;           
	sint32	termSection;          
	uint32	termOffset;           
	uint32	importedLibraryCount; 
	uint32	totalImportedSymbolCount; 
	uint32	relocSectionCount;    
	uint32	relocInstrOffset;     
	uint32	loaderStringsOffset;  
	uint32	exportHashOffset;     
	uint32	exportHashTablePower; 
	uint32	exportedSymbolCount;  
};

struct PEFAddress {
	uint32 a32;
};

extern byte PEF_SECTION_HEADER_struct[];
extern byte PEF_CONTAINER_HEADER_struct[];
extern byte PEF_LOADER_INFO_HEADER_struct[];

#endif /* __PEFFSTRUC_H__ */
