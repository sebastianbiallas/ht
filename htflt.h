/* 
 *	HT Editor
 *	htflt.h
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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

#ifndef __HTFLT_H__
#define __HTFLT_H__

#include "fltstruc.h"
#include "formats.h"
#include "endianess.h"
#include "htformat.h"
#include "relfile.h"

#define DESC_FLT "flat - binary format"
#define DESC_FLT_HEADER "flat/header"
#define DESC_FLT_IMAGE "flat/image"


extern format_viewer_if htflt_if;

typedef uint32 FLTAddress;

struct ht_flt_shared_data {
	FileOfs header_ofs;
	flat_hdr header;

	FLTAddress code_start;
	FLTAddress code_end;
	FLTAddress data_start;
	FLTAddress data_end;
	FLTAddress bss_start;
	FLTAddress bss_end;
};

/*
 *	CLASS ht_flt
 */

class ht_flt: public ht_format_group {
public:
			void init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs);
	virtual	void done();
};


#endif /* !__HTELF_H__ */

