/* 
 *	HT Editor
 *	pestruct.h
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

#ifndef __PESTRUCT_H_
#define __PESTRUCT_H_

#include "global.h"
#include "tools.h"

#include "coff_s.h"

#define PE_MAGIC0	'P'
#define PE_MAGIC1	'E'
#define PE_MAGIC2	0
#define PE_MAGIC3	0

//
// Directory format.
//

typedef struct	PE_DATA_DIRECTORY {
    dword	  address HTPACKED;
    dword	  size HTPACKED;
};

#define PE_NUMBEROF_DIRECTORY_ENTRIES	  	16

//
// Optional header format.
//

typedef struct	PE_OPTIONAL_HEADER32_NT {
// NT additional fields.
	dword image_base HTPACKED;
	dword section_alignment HTPACKED;
	dword file_alignment HTPACKED;
	word major_os_version	HTPACKED;
	word minor_os_version	HTPACKED;
	word major_image_version	HTPACKED;
	word minor_image_version	HTPACKED;
	word major_subsystem_version HTPACKED;
	word minor_subsystem_version HTPACKED;
	dword win32_version HTPACKED;
	dword image_size HTPACKED;
	dword header_size HTPACKED;
	dword checksum HTPACKED;
	word	subsystem HTPACKED;
	word dll_characteristics HTPACKED;
	dword stack_reserve_size HTPACKED;
	dword stack_commit_size HTPACKED;
	dword heap_reserve_size HTPACKED;
	dword heap_commit_size HTPACKED;
	dword loader_flags HTPACKED;
	dword directory_count HTPACKED;
	PE_DATA_DIRECTORY directory[PE_NUMBEROF_DIRECTORY_ENTRIES] HTPACKED;
};

typedef struct	PE_OPTIONAL_HEADER64_NT {
// NT additional fields.
	qword image_base HTPACKED;
	dword section_alignment HTPACKED;
	dword file_alignment HTPACKED;
	word major_os_version	HTPACKED;
	word minor_os_version	HTPACKED;
	word major_image_version	HTPACKED;
	word minor_image_version	HTPACKED;
	word major_subsystem_version HTPACKED;
	word minor_subsystem_version HTPACKED;
	dword win32_version HTPACKED;
	dword image_size HTPACKED;
	dword header_size HTPACKED;
	dword checksum HTPACKED;
	word	subsystem HTPACKED;
	word dll_characteristics HTPACKED;
	qword stack_reserve_size HTPACKED;
	qword stack_commit_size HTPACKED;
	qword heap_reserve_size HTPACKED;
	qword heap_commit_size HTPACKED;
	dword loader_flags HTPACKED;
	dword directory_count HTPACKED;
	PE_DATA_DIRECTORY directory[PE_NUMBEROF_DIRECTORY_ENTRIES] HTPACKED;
};

// Subsystem Values

#define PE_SUBSYSTEM_NATIVE			   1	  // Image doesn't require a subsystem.
#define PE_SUBSYSTEM_WINDOWS_GUI		   2	  // Image runs in the Windows GUI subsystem.
#define PE_SUBSYSTEM_WINDOWS_CUI		   3	  // Image runs in the Windows character subsystem.
#define PE_SUBSYSTEM_OS2_CUI			   5	  // image runs in the OS/2 character subsystem.
#define PE_SUBSYSTEM_POSIX_CUI		   7	  // image run  in the Posix character subsystem.
#define PE_SUBSYSTEM_RESERVED8		   8	  // image run  in the 8 subsystem.


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
	dword characteristics HTPACKED;
	dword timestamp HTPACKED;
	word major_version HTPACKED;
	word minor_version HTPACKED;
	dword name_address HTPACKED;
	dword ordinal_base HTPACKED;
	dword function_count HTPACKED;
	dword name_count HTPACKED;
	dword function_table_address HTPACKED;
	dword name_table_address HTPACKED;
	dword ordinal_table_address HTPACKED;
};

/*
 *	Import
 */

struct PE_THUNK_DATA {
	union {
		dword forwarder_string HTPACKED;
		dword function_desc_address HTPACKED;
		dword ordinal HTPACKED;
		dword data_address HTPACKED;
	};
};

struct PE_THUNK_DATA_64 {
	union {
		qword forwarder_string HTPACKED;
		qword function_desc_address HTPACKED;
		qword ordinal HTPACKED;
		qword data_address HTPACKED;
	};
};

struct PE_IMPORT_DESCRIPTOR {
	union {
		dword characteristics HTPACKED; 		// 0 for terminating null import descriptor
		dword original_first_thunk HTPACKED;  	// rva to original unbound IAT
	};
	dword timestamp HTPACKED;		// 0 if not bound,
											// -1 if bound, and real date\time stamp
											//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
											// O.W. date/time stamp of DLL bound to (Old BIND)

	dword forwarder_chain HTPACKED;  	// -1 if no forwarders
	dword name HTPACKED;
	dword first_thunk HTPACKED; 		// rva to IAT (if bound this IAT has actual addresses)
};

/*
 *	Delay Import
 */

struct PE_DELAY_IMPORT_DESCRIPTOR {
	dword attributes HTPACKED;
	dword name HTPACKED;
	dword module_handle HTPACKED;
	dword delay_iat HTPACKED;
	dword delay_int HTPACKED;
	dword bound_delay_import_table HTPACKED;
	dword unload_delay_import_table HTPACKED;
	dword timestamp HTPACKED;
};

/*
 *	Resource
 */

struct PE_RESOURCE_DIRECTORY {
	dword characteristics HTPACKED;
	dword timedate_stamp HTPACKED;
	word major_version HTPACKED;
	word minor_version HTPACKED;
	word name_count HTPACKED;
	word id_count HTPACKED;
//    PE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
};

#define PE_RESOURCE_NAME_IS_STRING			0x80000000
#define PE_RESOURCE_DATA_IS_DIRECTORY		0x80000000

struct PE_RESOURCE_DIRECTORY_ENTRY {
	dword   name HTPACKED;				// or id
	dword   offset_to_directory HTPACKED;	// or data
};

/*typedef struct PE_RESOURCE_DIRECTORY_STRING {
    word    Length;
    CHAR    NameString[ 1 ];
};


typedef struct PE_RESOURCE_DIR_STRING_U {
    word    Length;
    WCHAR   NameString[ 1 ];
};*/

struct PE_RESOURCE_DATA_ENTRY {
    dword	  offset_to_data HTPACKED;
    dword	  size HTPACKED;
    dword	  codepage HTPACKED;
    dword	  reserved HTPACKED;
};

/*
 *   IL
 */

#define PE_IL_DIRECTORY_ATTRIBUTES_HAD_NATIVE 0x1
#define PE_IL_DIRECTORY_ATTRIBUTES_INT64      0x2

struct PE_IL_DIRECTORY {
	dword size HTPACKED;
	word major_version HTPACKED;
	word minor_version HTPACKED;
	dword metadata_section_rva HTPACKED;
	dword metadata_section_size HTPACKED;
	dword attributes HTPACKED;
};

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
