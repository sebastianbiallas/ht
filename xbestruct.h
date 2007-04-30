/* 
 *	HT Editor
 *	xbestruct.h
 *
 *	Copyright (C) 2003 Stefan Esser
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

#ifndef __XBESTRUCT_H_
#define __XBESTRUCT_H_

#include "global.h"
#include "tools.h"

typedef unsigned int RVA;

#define XBE_MAGIC_LENGTH 4
#define XBE_MAGIC0	'X'
#define XBE_MAGIC1	'B'
#define XBE_MAGIC2	'E'
#define XBE_MAGIC3	'H'

#define XBE_SIZE_OF_SIGNATURE	256

typedef struct XBE_IMAGE_HEADER {
    byte	magic_id[XBE_MAGIC_LENGTH];
    byte	signature[XBE_SIZE_OF_SIGNATURE];
    dword	base_address;
    dword	size_of_headers;
    dword	size_of_image;
    dword	size_of_imageheader;
    dword	timedate;
    dword	certificate_address;
    dword	number_of_sections;
    dword	section_header_address;
    dword	initialisation_flags;
    dword	entry_point;
    dword	tls_address;
    dword	pe_stack_commit;
    dword	pe_heap_reserve;
    dword	pe_heap_commit;
    dword	pe_base_address;
    dword	pe_size_of_image;
    dword	pe_checksum;
    dword	pe_timedate;
    dword	debug_pathname_address;
    dword	debug_filename_address;
    dword	debug_unicode_filename_address;
    dword	kernel_image_thunk_address;
    dword	non_kernel_import_directory_address;
    dword	number_of_library_versions;
    dword	library_versions_address;
    dword	kernel_library_version_address;
    dword	xapi_library_version_address;
    dword	logo_bitmap_address;
    dword	logo_bitmap_size;
} HTPACKED;


#define XBE_TITLE_NAME_LENGTH	40
#define XBE_NUM_ALTERNATE	16
#define XBE_LAN_KEY_LENGTH	16
#define XBE_SIGNATURE_KEY_LENGTH	16

#define XBE_MEDIA_TYPE_HARD_DISK		0x00000001
#define XBE_MEDIA_TYPE_DVD_X2			0x00000002
#define XBE_MEDIA_TYPE_DVD_CD			0x00000004
#define XBE_MEDIA_TYPE_CD			0x00000008
#define XBE_MEDIA_TYPE_DVD_5_RO			0x00000010
#define XBE_MEDIA_TYPE_DVD_9_RO			0x00000020
#define XBE_MEDIA_TYPE_DVD_5_RW			0x00000040
#define XBE_MEDIA_TYPE_DVD_9_RW			0x00000080
#define XBE_MEDIA_TYPE_DONGLE			0x00000100
#define XBE_MEDIA_TYPE_MEDIA_BOARD		0x00000200
#define XBE_MEDIA_TYPE_NONSECURE_HARD_DISK	0x40000000
#define XBE_MEDIA_TYPE_NONSECURE_MODE		0x80000000
#define XBE_MEDIA_TYPE_MEDIA_MASK		0x00FFFFFF

#define XBE_GAME_REGION_NA			0x00000001
#define XBE_GAME_REGION_JAPAN			0x00000002
#define XBE_GAME_REGION_RESTOFWORLD		0x00000004
#define XBE_GAME_REGION_MANUFACTURING		0x80000000

typedef struct XBE_CERTIFICATE {
    dword	size_of_certificate;
    dword	timedate;
    dword	title_id;
    word	title_name[XBE_TITLE_NAME_LENGTH];
    dword	alternate_title_ids[XBE_NUM_ALTERNATE];
    dword	allowed_media;
    dword	game_region;
    dword	game_ratings;
    dword	disk_number;
    dword	version;
    byte	lan_key[XBE_LAN_KEY_LENGTH];
    byte	signature_key[XBE_SIGNATURE_KEY_LENGTH];
    byte	alternate_signature_keys[XBE_NUM_ALTERNATE][XBE_SIGNATURE_KEY_LENGTH];    
} HTPACKED;


#define XBE_SECTION_FLAGS_WRITABLE	1
#define XBE_SECTION_FLAGS_PRELOAD	2
#define XBE_SECTION_FLAGS_EXECUTABLE	4
#define XBE_SECTION_FLAGS_INSERTEDFILE	8
#define XBE_SECTION_FLAGS_HEADPAGE_RO	16
#define XBE_SECTION_FLAGS_TAILPAGE_RO	32

#define XBE_SECTION_DIGEST_LENGTH	20

typedef struct XBE_SECTION_HEADER {
    dword	section_flags;
    dword	virtual_address;
    dword	virtual_size;
    dword	raw_address;
    dword	raw_size;
    dword	section_name_address;
    dword	section_name_ref_count;
    dword	head_shared_page_ref_count_address;
    dword	tail_shared_page_ref_count_address;
    byte	section_digest[XBE_SECTION_DIGEST_LENGTH];
} HTPACKED;

#define XBE_LIBRARY_NAME_LENGTH		8

typedef struct XBE_LIBRARY_VERSION {
    byte	library_name[XBE_LIBRARY_NAME_LENGTH];
    word	major_version;
    word	minor_version;
    word	build_version;
    word	library_flags;
} HTPACKED;


typedef struct XBE_TLS_DIRECTORY {
    dword	data_start_address;
    dword	data_end_address;
    dword	tls_index_address;
    dword	tls_callback_address;
    dword	size_of_zero_fill;
    dword	characteristics;
} HTPACKED;

extern byte XBE_IMAGE_HEADER_struct[];
extern byte XBE_CERTIFICATE_struct[];
extern byte XBE_SECTION_HEADER_struct[];
extern byte XBE_LIBRARY_VERSION_struct[];
extern byte XBE_TLS_DIRECTORY_struct[];


#endif
