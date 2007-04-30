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

#include "global.h"
#include "tools.h"

#define IMAGE_MZ_MAGIC0	'M'
#define IMAGE_MZ_MAGIC1 'Z'

struct IMAGE_MZ_HEADER {
	word magic;
	word sizelp;
	word sizep;
	word reloc_count;
	word header_size;
	word minalloc;
	word maxalloc;
	word ss;
	word sp;
	word checksum;
	word ip;
	word cs;
	word reloc_ofs;
	word overlay_num;
	word res[4];
	word oemid;
	word oeminfo;
	word res2[10];
	dword newexe_ofs;
} HTPACKED;

extern byte MZ_HEADER_struct[];

#endif /* __MZSTRUCT_H_ */
