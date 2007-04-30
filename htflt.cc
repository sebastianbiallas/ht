/* 
 *	HT Editor
 *	htelf.cc
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

#include "log.h"
#include "htflt.h"
#include "htflthd.h"
#include "htfltimg.h"
#include "endianess.h"
#include "stream.h"
#include "tools.h"

#include "fltstruc.h"

#include <stdlib.h>

static format_viewer_if *htflt_ifs[] = {
	&htfltheader_if,
	&htfltimage_if,
	0
};

static ht_view *htflt_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte ident[4];
	file->seek(0);
	file->read(&ident, sizeof ident);
	if ((ident[0]!=FLTMAG0) || (ident[1]!=FLTMAG1) ||
		(ident[2]!=FLTMAG2) || (ident[3]!=FLTMAG3) ) return 0;
		
	ht_flt *g=new ht_flt();
	g->init(b, file, htflt_ifs, format_group, 0);
	return g;
}

format_viewer_if htflt_if = {
	htflt_init,
	0
};

/*
 *	CLASS ht_flt
 */
void ht_flt::init(Bounds *b, File *f, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_FLT, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_flt");

	String fn;
	file->getFilename(fn);
	LOG("%y: FLAT: found header at %08qx", &fn, header_ofs);
	
	ht_flt_shared_data *flt_shared = ht_malloc(sizeof (ht_flt_shared_data));
	
	shared_data = flt_shared;
	flt_shared->header_ofs = header_ofs;

	/* read header */
	file->seek(header_ofs);
	file->read(&flt_shared->header, sizeof flt_shared->header);
	createHostStruct(&flt_shared->header, FLAT_HEADER_struct, big_endian);

	flt_shared->code_start = sizeof flt_shared->header;
	flt_shared->code_end = flt_shared->header.data_start;
	flt_shared->data_start = flt_shared->header.data_start;
	flt_shared->data_end = flt_shared->header.data_end;
	flt_shared->bss_start = flt_shared->header.data_end;
	flt_shared->bss_end = flt_shared->header.bss_end;
	/* init ifs */
	ht_format_group::init_ifs(ifs);
}

void ht_flt::done()
{
	ht_format_group::done();
	ht_flt_shared_data *flt_shared=(ht_flt_shared_data *)shared_data;
	free(flt_shared);
}
