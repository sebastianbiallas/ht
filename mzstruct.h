/* 
 *	HT Editor
 *	mzstruct.h
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

#ifndef __MZSTRUCT_H_
#define __MZSTRUCT_H_

#include "global.h"
#include "tools.h"

#define IMAGE_MZ_MAGIC0	'M'
#define IMAGE_MZ_MAGIC1 'Z'

struct IMAGE_MZ_HEADER {
	word magic HTPACKED;
	word sizelp HTPACKED;
	word sizep HTPACKED;
	word reloc_count HTPACKED;
	word header_size HTPACKED;
	word minalloc HTPACKED;
	word maxalloc HTPACKED;
	word ss HTPACKED;
	word sp HTPACKED;
	word checksum HTPACKED;
	word ip HTPACKED;
	word cs HTPACKED;
	word reloc_ofs HTPACKED;
	word overlay_num HTPACKED;
	word res[4] HTPACKED;
	word oemid HTPACKED;
	word oeminfo HTPACKED;
	word res2[10] HTPACKED;
	dword newexe_ofs HTPACKED;
};

extern byte MZ_HEADER_struct[];

#endif /* __MZSTRUCT_H_ */
