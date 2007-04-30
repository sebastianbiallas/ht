/* 
 *	HT Editor
 *	pestruct.h
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

#ifndef __PESTRUCT_H_
#define __PESTRUCT_H_

#include "io/types.h"
#include "tools.h"

#include "coff_s.h"

#define PE_MAGIC0	'P'
#define PE_MAGIC1	'E'
#define PE_MAGIC2	0
#define PE_MAGIC3	0

//
// Directory format.
//

struct	PE_DATA_DIRECTORY {
    uint32	  address;
    uint32	  size;
} PACKED;

#define PE_NUMBEROF_DIRECTORY_ENTRIES	  	16

//
// Optional header format.
//

struct	PE_OPTIONAL_HEADER32_NT {
// NT additional fields.
	uint32 image_base;
	uint32 section_alignment;
	uint32 file_alignment;
	uint16 major_os_version;
	uint16 minor_os_version;
	uint16 major_image_version;
	uint16 minor_image_version;
	uint16 major_subsystem_version;
	uint16 minor_subsystem_version;
	uint32 win32_version;
	uint32 image_size;
	uint32 header_size;
	uint32 checksum;
	uint16 subsystem;
	uint16 dll_characteristics;
	uint32 stack_reserve_size;
	uint32 stack_commit_size;
	uint32 heap_reserve_size;
	uint32 heap_commit_size;
	uint32 loader_flags;
	uint32 directory_count;
	PE_DATA_DIRECTORY directory[PE_NUMBEROF_DIRECTORY_ENTRIES];
} PACKED;

struct	PE_OPTIONAL_HEADER64_NT {
// NT additional fields.
	uint64 image_base;
	uint32 section_alignment;
	uint32 file_alignment;
	uint16 major_os_version;
	uint16 minor_os_version;
	uint16 major_image_version;
	uint16 minor_image_version;
	uint16 major_subsystem_version;
	uint16 minor_subsystem_version;
	uint32 win32_version;
	uint32 image_size;
	uint32 header_size;
	uint32 checksum;
	uint16	subsystem;
	uint16 dll_characteristics;
	uint64 stack_reserve_size;
	uint64 stack_commit_size;
	uint64 heap_reserve_size;
	uint64 heap_commit_size;
	uint32 loader_flags;
	uint32 directory_count;
	PE_DATA_DIRECTORY directory[PE_NUMBEROF_DIRECTORY_ENTRIES];
} PACKED;

// Subsystem Values

#define PE_SUBSYSTEM_NATIVE				 1  // image doesn't require a subsystem.
#define PE_SUBSYSTEM_WINDOWS_GUI			 2  // image runs in the Windows GUI subsystem.
#define PE_SUBSYSTEM_WINDOWS_CUI			 3  // image runs in the Windows character subsystem.
#define PE_SUBSYSTEM_OS2_CUI				 5  // image runs in the OS/2 character subsystem.
#define PE_SUBSYSTEM_POSIX_CUI			 7  // image run  in the Posix character subsystem.
#define PE_SUBSYSTEM_RESERVED8			 8  // image run  in the 8 subsystem.
#define PE_SUBSYSTEM_CE_GUI				 9  // image runs in the Windows CE subsystem.
#define PE_SUBSYSTEM_EFI_APPLICATION		10  // image is an EFI application.
#define PE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER	11  // image is an EFI driver that provides boot services.
#define PE_SUBSYSTEM_EFI_RUNTIME_DRIVER		12  // image is an EFI driver that provides runtime services.

// Directory Entries

#define PE_DIRECTORY_ENTRY_EXPORT			 0	  // Export Directory
#define PE_DIRECTORY_ENTRY_IMPORT			 1	  // Import Directory
#define PE_DIRECTORY_ENTRY_RESOURCE		 2	  // Resource Directory
#define PE_DIRECTORY_ENTRY_EXCEPTION		 3	  // Exception Directory
#define PE_DIRECTORY_ENTRY_SECURITY		 4	  // Security Directory
#define PE_DIRECTORY_ENTRY_BASERELOC		 5	  // Base Relocation Table
#define PE_DIRECTORY_ENTRY_DEBUG			 6	  // Debug Directory
#define PE_DIRECTORY_ENTRY_COPYRIGHT		 7	  // Description String
#define PE_DIRECTORY_ENTRY_GLOBALPTR		 8	  // Machine Value (MIPS GP)
#define PE_DIRECTORY_ENTRY_TLS			 9	  // TLS Directory
#define PE_DIRECTORY_ENTRY_LOAD_CONFIG		10	  // Load Configuration Directory
#define PE_DIRECTORY_ENTRY_BOUND_IMPORT		11	  // Bound Import Directory in headers
#define PE_DIRECTORY_ENTRY_IAT			12	  // Import Address Table
#define PE_DIRECTORY_ENTRY_DELAY_IMPORT		13	  // Delay Import Directory
#define PE_DIRECTORY_ENTRY_IL			14	  // IL (e.g. MS .NET)

/*
 *	Export
 */

struct PE_EXPORT_DIRECTORY {
	uint32 characteristics;
	uint32 timestamp;
	uint16 major_version;
	uint16 minor_version;
	uint32 name_address;
	uint32 ordinal_base;
	uint32 function_count;
	uint32 name_count;
	uint32 function_table_address;
	uint32 name_table_address;
	uint32 ordinal_table_address;
} PACKED;

/*
 *	Import
 */

struct PE_THUNK_DATA {
	union {
		uint32 forwarder_string;
		uint32 function_desc_address;
		uint32 ordinal;
		uint32 data_address;
	};
} PACKED;

struct PE_THUNK_DATA_64 {
	union {
		uint64 forwarder_string;
		uint64 function_desc_address;
		uint64 ordinal;
		uint64 data_address;
	};
} PACKED;

struct PE_IMPORT_DESCRIPTOR {
	union {
		uint32 characteristics; 		// 0 for terminating null import descriptor
		uint32 original_first_thunk;  	// rva to original unbound IAT
	};
	uint32 timestamp;		// 0 if not bound,
											// -1 if bound, and real date\time stamp
											//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
											// O.W. date/time stamp of DLL bound to (Old BIND)

	uint32 forwarder_chain;  	// -1 if no forwarders
	uint32 name;
	uint32 first_thunk; 		// rva to IAT (if bound this IAT has actual addresses)
} PACKED;

/*
 *	Delay Import
 */

struct PE_DELAY_IMPORT_DESCRIPTOR {
	uint32 attributes;
	uint32 name;
	uint32 module_handle;
	uint32 delay_iat;
	uint32 delay_int;
	uint32 bound_delay_import_table;
	uint32 unload_delay_import_table;
	uint32 timestamp;
} PACKED;

/*
 *	Resource
 */

struct PE_RESOURCE_DIRECTORY {
	uint32 characteristics;
	uint32 timedate_stamp;
	uint16 major_version;
	uint16 minor_version;
	uint16 name_count;
	uint16 id_count;
//    PE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} PACKED;

#define PE_RESOURCE_NAME_IS_STRING			0x80000000
#define PE_RESOURCE_DATA_IS_DIRECTORY		0x80000000

struct PE_RESOURCE_DIRECTORY_ENTRY {
	uint32   name;				// or id
	uint32   offset_to_directory;	// or data
} PACKED;

/* struct PE_RESOURCE_DIRECTORY_STRING {
    uint16    Length;
    CHAR    NameString[ 1 ];
} PACKED;


struct PE_RESOURCE_DIR_STRING_U {
    uint16    Length;
    WCHAR   NameString[ 1 ];
} PACKED;*/

struct PE_RESOURCE_DATA_ENTRY {
    uint32	  offset_to_data;
    uint32	  size;
    uint32	  codepage;
    uint32	  reserved;
} PACKED;

/*
 *   IL
 */

#define PE_IL_DIRECTORY_ATTRIBUTES_HAD_NATIVE 0x1
#define PE_IL_DIRECTORY_ATTRIBUTES_INT64      0x2

struct PE_IL_DIRECTORY {
	uint32 size;
	uint16 major_version;
	uint16 minor_version;
	uint32 metadata_section_rva;
	uint32 metadata_section_size;
	uint32 attributes;
} PACKED;

extern byte PE_DATA_DIRECTORY_struct[];
extern byte PE_OPTIONAL_HEADER32_NT_struct[];
extern byte PE_OPTIONAL_HEADER64_NT_struct[];
extern byte PE_EXPORT_DIRECTORY_struct[];
extern byte PE_THUNK_DATA_struct[];
extern byte PE_THUNK_DATA_64_struct[];
extern byte PE_IMPORT_DESCRIPTOR_struct[];
extern byte PE_DELAY_IMPORT_DESCRIPTOR_struct[];
extern byte PE_RESOURCE_DIRECTORY_struct[];
extern byte PE_RESOURCE_DIRECTORY_ENTRY_struct[];
extern byte PE_RESOURCE_DATA_ENTRY_struct[];
extern byte PE_IL_DIRECTORY_struct[];

#endif
