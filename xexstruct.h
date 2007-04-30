/* 
 *	HT Editor
 *	xexstruct.h
 *
 *	Copyright (C) 2006 Sebastian Biallas (sb@biallas.net)
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

#ifndef __XEXSTRUCT_H_
#define __XEXSTRUCT_H_

#include "io/types.h"
#include "tools.h"

typedef unsigned int RVA;

#define XEX_MAGIC_LENGTH 4
#define XEX_MAGIC0	'X'
#define XEX_MAGIC1	'E'
#define XEX_MAGIC2	'X'
#define XEX_MAGIC3	'2'

struct XEX_IMAGE_HEADER {
	byte	magic_id[XEX_MAGIC_LENGTH];
	uint32	flags;
	uint32	size;
	uint32	res;
	uint32	file_header_offset;
	uint32	number_of_sections;
} PACKED;

#define XEX_HEADER_FIELD_MODULES	0x0002ff
#define XEX_HEADER_FIELD_LOADERINFO	0x0003ff
#define XEX_HEADER_FIELD_FILENAME	0x0080ff
#define XEX_HEADER_FIELD_LOADBASE	0x010001
#define XEX_HEADER_FIELD_ENTRY		0x010100
#define XEX_HEADER_FIELD_BASE		0x010201
#define XEX_HEADER_FIELD_IMPORT		0x0103ff
#define XEX_HEADER_FIELD_IDS		0x018002
#define XEX_HEADER_FIELD_ORIG_FILENAME	0x0183ff
#define XEX_HEADER_FIELD_RESMAP2	0x0200ff
#define XEX_HEADER_FIELD_UNK0		0x020104 // 80078884
#define XEX_HEADER_FIELD_STACK_SIZE	0x020200 // 800788bc
#define XEX_HEADER_FIELD_CACHE_INFO	0x020301
#define XEX_HEADER_FIELD_MEDIAINFO	0x040006
#define XEX_HEADER_FIELD_LAN_KEY	0x040404
#define XEX_HEADER_FIELD_IMPORT_UNK	0xe10402

#define XEX_MEDIA_HD		0x00000001
#define XEX_MEDIA_DVD_X2	0x00000002
#define XEX_MEDIA_DVD_CD	0x00000004
#define XEX_MEDIA_DVD_5		0x00000008
#define XEX_MEDIA_DVD_9		0x00000010
#define XEX_MEDIA_FLASH		0x00000020
#define XEX_MEDIA_MEMORY_UNIT	0x00000080
#define XEX_MEDIA_MASS_STORAGE	0x00000100
#define XEX_MEDIA_SMB		0x00000200
#define XEX_MEDIA_RAM		0x00000400
#define XEX_MEDIA_INSECURE	0x01000000
#define XEX_MEDIA_SAVE_GAME	0x02000000
#define XEX_MEDIA_LOCAL		0x04000000
#define XEX_MEDIA_LIVE		0x08000000
#define XEX_MEDIA_XBOX		0x10000000

struct XEX_IMAGE_HEADER_INFO_ENTRY {
	union {
		struct {
			byte	res;
			byte	classe;
			byte	type;
			byte	size;
		} b;
		uint32 type PACKED;
	} PACKED;
	uint32	value;
} PACKED;

enum {
	XEX_LOADER_NONE = 0,
	XEX_LOADER_RAW = 1,
	XEX_LOADER_COMPRESSED = 2,
};

struct XEX_LOADER_INFO_HEADER {
	uint16 crypted;
	uint16 type;
} PACKED;

struct XEX_RAW_LOADER_ENTRY {
	uint32 raw;
	uint32 pad;
} PACKED;

struct XEX_FILE_HEADER {
	uint32 hdr_size;
	uint32 image_size;
	uint8  key[256];
	uint32 unk1;
	uint32 image_flags;
	uint32 load_address;
	uint8  hash1[20];
	uint32 unk2;
	uint8  hash2[20];
	uint8  unk3[16];
	uint8  loader_key[16];
	uint32 unk4;
	uint8  hash3[20];
	uint32 region;
	uint32 media_mask;
	uint32 pages;
} PACKED;

extern byte XEX_IMAGE_HEADER_struct[];
extern byte XEX_IMAGE_HEADER_INFO_ENTRY_struct[];
extern byte XEX_LOADER_INFO_HEADER_struct[];
extern byte XEX_RAW_LOADER_ENTRY_struct[];
extern byte XEX_FILE_HEADER_struct[];

#endif
