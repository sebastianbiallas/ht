/* 
 *	HT Editor
 *	htle.cc
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

#include <stdlib.h>

#include "htapp.h"
#include "htle.h"
#include "htlehead.h"
#include "htleimg.h"
#include "htleobj.h"
#include "htleent.h"
#include "htlepage.h"
#include "htnewexe.h"
#include "lestruct.h"

format_viewer_if *htle_ifs[] = {
	&htleheader_if,
	&htlepagemaps_if,
	&htleobjects_if,
	&htleentrypoints_if,
	&htleimage_if,
	0
};

ht_view *htle_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	byte lemagic[2];
	FILEOFS h=get_newexe_header_ofs(file);
	file->seek(h);
	file->read(lemagic, 2);
	if ((lemagic[0]!=LE_MAGIC0) || (lemagic[1]!=LE_MAGIC1)) return 0;

	ht_le *g=new ht_le();
	g->init(b, file, htle_ifs, format_group, h);
	return g;
}

format_viewer_if htle_if = {
	htle_init,
	0
};

void ht_le::init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS h)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_LE, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_le");

	LOG("%s: LE: found header at %08x", file->get_filename(), h);
	ht_le_shared_data *le_shared=new ht_le_shared_data;
	shared_data=le_shared;
	le_shared->v_header=0;
	le_shared->v_objects=0;
	le_shared->v_pagemaps=0;
	le_shared->v_image=0;
	le_shared->hdr_ofs=h;
	file->seek(h);
	file->read(&le_shared->hdr, sizeof le_shared->hdr);

	shared_data=le_shared;

	ht_format_group::init_ifs(ifs);
}

void ht_le::done()
{
	ht_le_shared_data *le_shared=(ht_le_shared_data*)shared_data;
	
	if (le_shared->objmap.header) free(le_shared->objmap.header);
	if (le_shared->objmap.vsize) free(le_shared->objmap.vsize);
	if (le_shared->objmap.psize) free(le_shared->objmap.psize);
	
	if (le_shared->pagemap.offset) free(le_shared->pagemap.offset);
	if (le_shared->pagemap.vsize) free(le_shared->pagemap.vsize);
	if (le_shared->pagemap.psize) free(le_shared->pagemap.psize);

	ht_format_group::done();
}

void ht_le::loc_enum_start()
{
	loc_enum=true;
}

bool ht_le::loc_enum_next(ht_format_loc *loc)
{
	ht_le_shared_data *sh=(ht_le_shared_data*)shared_data;
	if (loc_enum) {
		loc->name="le";
		loc->start=sh->hdr_ofs;
		loc->length=file->get_size()-loc->start;	/* FIXME: ENOTOK */
		
		loc_enum=false;
		return true;
	}
	return false;
}

