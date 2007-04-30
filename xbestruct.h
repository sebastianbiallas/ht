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

#include "io/types.h"
#include "tools.h"

typedef unsigned int RVA;

#define XBE_MAGIC_LENGTH 4
#define XBE_MAGIC0	'X'
#define XBE_MAGIC1	'B'
#define XBE_MAGIC2	'E'
#define XBE_MAGIC3	'H'

#define XBE_SIZE_OF_SIGNATURE	256

struct XBE_IMAGE_HEADER {
    byte	magic_id[XBE_MAGIC_LENGTH];
    byte	signature[XBE_SIZE_OF_SIGNATURE];
    uint32	base_address;
    uint32	size_of_headers;
    uint32	size_of_image;
    uint32	size_of_imageheader;
    uint32	timedate;
    uint32	certificate_address;
    uint32	number_of_sections;
    uint32	section_header_address;
    uint32	initialisation_flags;
    uint32	entry_point;
    uint32	tls_address;
    uint32	pe_stack_commit;
    uint32	pe_heap_reserve;
    uint32	pe_heap_commit;
    uint32	pe_base_address;
    uint32	pe_size_of_image;
    uint32	pe_checksum;
    uint32	pe_timedate;
    uint32	debug_pathname_address;
    uint32	debug_filename_address;
    uint32	debug_unicode_filename_address;
    uint32	kernel_image_thunk_address;
    uint32	non_kernel_import_directory_address;
    uint32	number_of_library_versions;
    uint32	library_versions_address;
    uint32	kernel_library_version_address;
    uint32	xapi_library_version_address;
    uint32	logo_bitmap_address;
    uint32	logo_bitmap_size;
} PACKED;


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

struct XBE_CERTIFICATE {
    uint32	size_of_certificate;
    uint32	timedate;
    uint32	title_id;
    uint16	title_name[XBE_TITLE_NAME_LENGTH];
    uint32	alternate_title_ids[XBE_NUM_ALTERNATE];
    uint32	allowed_media;
    uint32	game_region;
    uint32	game_ratings;
    uint32	disk_number;
    uint32	version;
    byte	lan_key[XBE_LAN_KEY_LENGTH];
    byte	signature_key[XBE_SIGNATURE_KEY_LENGTH];
    byte	alternate_signature_keys[XBE_NUM_ALTERNATE][XBE_SIGNATURE_KEY_LENGTH];    
} PACKED;


#define XBE_SECTION_FLAGS_WRITABLE	1
#define XBE_SECTION_FLAGS_PRELOAD	2
#define XBE_SECTION_FLAGS_EXECUTABLE	4
#define XBE_SECTION_FLAGS_INSERTEDFILE	8
#define XBE_SECTION_FLAGS_HEADPAGE_RO	16
#define XBE_SECTION_FLAGS_TAILPAGE_RO	32

#define XBE_SECTION_DIGEST_LENGTH	20

struct XBE_SECTION_HEADER {
    uint32	section_flags;
    uint32	virtual_address;
    uint32	virtual_size;
    uint32	raw_address;
    uint32	raw_size;
    uint32	section_name_address;
    uint32	section_name_ref_count;
    uint32	head_shared_page_ref_count_address;
    uint32	tail_shared_page_ref_count_address;
    byte	section_digest[XBE_SECTION_DIGEST_LENGTH];
} PACKED;

#define XBE_LIBRARY_NAME_LENGTH		8

struct XBE_LIBRARY_VERSION {
    byte	library_name[XBE_LIBRARY_NAME_LENGTH];
    uint16	major_version;
    uint16	minor_version;
    uint16	build_version;
    uint16	library_flags;
} PACKED;


struct XBE_TLS_DIRECTORY {
    uint32	data_start_address;
    uint32	data_end_address;
    uint32	tls_index_address;
    uint32	tls_callback_address;
    uint32	size_of_zero_fill;
    uint32	characteristics;
} PACKED;

extern byte XBE_IMAGE_HEADER_struct[];
extern byte XBE_CERTIFICATE_struct[];
extern byte XBE_SECTION_HEADER_struct[];
extern byte XBE_LIBRARY_VERSION_struct[];
extern byte XBE_TLS_DIRECTORY_struct[];


#endif
