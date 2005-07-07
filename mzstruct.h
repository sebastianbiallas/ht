/* 
 *	HT Editor
 *	mzstruct.h
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

#ifndef __MZSTRUCT_H_
#define __MZSTRUCT_H_

#include "io/types.h"
#include "tools.h"

#define IMAGE_MZ_MAGIC0	'M'
#define IMAGE_MZ_MAGIC1 'Z'

struct IMAGE_MZ_HEADER {
	uint16 magic PACKED;
	uint16 sizelp PACKED;
	uint16 sizep PACKED;
	uint16 reloc_count PACKED;
	uint16 header_size PACKED;
	uint16 minalloc PACKED;
	uint16 maxalloc PACKED;
	uint16 ss PACKED;
	uint16 sp PACKED;
	uint16 checksum PACKED;
	uint16 ip PACKED;
	uint16 cs PACKED;
	uint16 reloc_ofs PACKED;
	uint16 overlay_num PACKED;
	uint16 res[4] PACKED;
	uint16 oemid PACKED;
	uint16 oeminfo PACKED;
	uint16 res2[10] PACKED;
	uint32 newexe_ofs PACKED;
};

extern byte MZ_HEADER_struct[];

#endif /* __MZSTRUCT_H_ */
