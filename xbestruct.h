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

typedef struct XBE_IMAGE_HEADER {
    byte	magic_id[XBE_MAGIC_LENGTH] PACKED;
    byte	signature[XBE_SIZE_OF_SIGNATURE] PACKED;
    uint32	base_address PACKED;
    uint32	size_of_headers PACKED;
    uint32	size_of_image PACKED;
    uint32	size_of_imageheader PACKED;
    uint32	timedate PACKED;
    uint32	certificate_address PACKED;
    uint32	number_of_sections PACKED;
    uint32	section_header_address PACKED;
    uint32	initialisation_flags PACKED;
    uint32	entry_point PACKED;
    uint32	tls_address PACKED;
    uint32	pe_stack_commit PACKED;
    uint32	pe_heap_reserve PACKED;
    uint32	pe_heap_commit PACKED;
    uint32	pe_base_address PACKED;
    uint32	pe_size_of_image PACKED;
    uint32	pe_checksum PACKED;
    uint32	pe_timedate PACKED;
    uint32	debug_pathname_address PACKED;
    uint32	debug_filename_address PACKED;
    uint32	debug_unicode_filename_address PACKED;
    uint32	kernel_image_thunk_address PACKED;
    uint32	non_kernel_import_directory_address PACKED;
    uint32	number_of_library_versions PACKED;
    uint32	library_versions_address PACKED;
    uint32	kernel_library_version_address PACKED;
    uint32	xapi_library_version_address PACKED;
    uint32	logo_bitmap_address PACKED;
    uint32	logo_bitmap_size PACKED;
};


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
    uint32	size_of_certificate PACKED;
    uint32	timedate PACKED;
    uint32	title_id PACKED;
    uint16	title_name[XBE_TITLE_NAME_LENGTH] PACKED;
    uint32	alternate_title_ids[XBE_NUM_ALTERNATE] PACKED;
    uint32	allowed_media PACKED;
    uint32	game_region PACKED;
    uint32	game_ratings PACKED;
    uint32	disk_number PACKED;
    uint32	version PACKED;
    byte	lan_key[XBE_LAN_KEY_LENGTH] PACKED;
    byte	signature_key[XBE_SIGNATURE_KEY_LENGTH] PACKED;
    byte	alternate_signature_keys[XBE_NUM_ALTERNATE][XBE_SIGNATURE_KEY_LENGTH] PACKED;    
};


#define XBE_SECTION_FLAGS_WRITABLE	1
#define XBE_SECTION_FLAGS_PRELOAD	2
#define XBE_SECTION_FLAGS_EXECUTABLE	4
#define XBE_SECTION_FLAGS_INSERTEDFILE	8
#define XBE_SECTION_FLAGS_HEADPAGE_RO	16
#define XBE_SECTION_FLAGS_TAILPAGE_RO	32

#define XBE_SECTION_DIGEST_LENGTH	20

typedef struct XBE_SECTION_HEADER {
    uint32	section_flags PACKED;
    uint32	virtual_address PACKED;
    uint32	virtual_size PACKED;
    uint32	raw_address PACKED;
    uint32	raw_size PACKED;
    uint32	section_name_address PACKED;
    uint32	section_name_ref_count PACKED;
    uint32	head_shared_page_ref_count_address PACKED;
    uint32	tail_shared_page_ref_count_address PACKED;
    byte	section_digest[XBE_SECTION_DIGEST_LENGTH] PACKED;
};

#define XBE_LIBRARY_NAME_LENGTH		8

typedef struct XBE_LIBRARY_VERSION {
    byte	library_name[XBE_LIBRARY_NAME_LENGTH] PACKED;
    uint16	major_version PACKED;
    uint16	minor_version PACKED;
    uint16	build_version PACKED;
    uint16	library_flags PACKED;
};


typedef struct XBE_TLS_DIRECTORY {
    uint32	data_start_address PACKED;
    uint32	data_end_address PACKED;
    uint32	tls_index_address PACKED;
    uint32	tls_callback_address PACKED;
    uint32	size_of_zero_fill PACKED;
    uint32	characteristics PACKED;
};

extern byte XBE_IMAGE_HEADER_struct[];
extern byte XBE_CERTIFICATE_struct[];
extern byte XBE_SECTION_HEADER_struct[];
extern byte XBE_LIBRARY_VERSION_struct[];
extern byte XBE_TLS_DIRECTORY_struct[];


#endif
