/* 
 *	HT Editor
 *	htmz.cc
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

#include "endianess.h"
#include "htmz.h"
#include "htmzhead.h"
#include "htmzrel.h"
#include "htmzimg.h"
#include "mzstruct.h"
#include "stream.h"

static format_viewer_if *htmz_ifs[] = {
	&htmzheader_if,
//	&htmzrel_if,
	&htmzimage_if,
	0
};

static ht_view *htmz_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte magic[2];
	file->seek(0);
	if (file->read(magic, 2) != 2 
	 || magic[0] != IMAGE_MZ_MAGIC0 || magic[1] != IMAGE_MZ_MAGIC1)
		return NULL;

	ht_mz *g = new ht_mz();
	g->init(b, file, htmz_ifs, format_group);
	return g;
}

format_viewer_if htmz_if = {
	htmz_init,
	0
};

void ht_mz::init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_MZ, file, false, true, 0, format_group);
	ht_mz_shared_data *mz_shared = ht_malloc(sizeof (ht_mz_shared_data));
	shared_data = mz_shared;
	file->seek(0);
	file->read(&mz_shared->header, sizeof mz_shared->header);
	createHostStruct(&mz_shared->header, MZ_HEADER_struct, little_endian);
	shared_data = mz_shared;
	ht_format_group::init_ifs(ifs);
}

void ht_mz::done()
{
	free(shared_data);
	ht_format_group::done();
}

void ht_mz::loc_enum_start()
{
	loc_enum=1;
}

bool ht_mz::loc_enum_next(ht_format_loc *loc)
{
	ht_mz_shared_data *sh=(ht_mz_shared_data*)shared_data;
	if (loc_enum) {
		loc->name = "mz";
		loc->start = 0;
		loc->length= sh->header.header_size*16+(sh->header.sizep-1)*512+
			     sh->header.sizelp;

		loc_enum = 0;
		return true;
	}
	return false;
}
