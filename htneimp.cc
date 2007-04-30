/* 
 *	HT Editor
 *	htneimp.cc
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

#include "atom.h"
#include "endianess.h"
#include "htne.h"
#include "htneent.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

#include <stdlib.h>

static ht_view *htneimports_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data *)group->get_shared_data();

	FileOfs h = ne_shared->hdr_ofs;
	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_NE_IMPORTS, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);

	char line[256];	/* secure */
	ht_snprintf(line, sizeof line, "* NE imported names and module reference table at offset %08qx / %08qx", h+ne_shared->hdr.imptab, h+ne_shared->hdr.modtab);
	m->add_mask(line);

	for (uint i=0; i<ne_shared->modnames_count; i++) {
		ht_snprintf(line, sizeof line, "%0d: %s", i+1, ne_shared->modnames[i]);
		m->add_mask(line);
	}
	v->insertsub(m);

	return v;
}

format_viewer_if htneimports_if = {
	htneimports_init,
	0
};
