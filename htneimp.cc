/* 
 *	HT Editor
 *	htneimp.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "htatom.h"
#include "htendian.h"
#include "htne.h"
#include "htneent.h"
#include "httag.h"
#include "formats.h"

#include <stdlib.h>

ht_view *htneimports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared = (ht_ne_shared_data *)group->get_shared_data();

	FILEOFS h = ne_shared->hdr_ofs;
	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, DESC_NE_IMPORTS, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);

	char line[1024];	/* possible buffer overflow */
	sprintf(line, "* NE imported names and module reference table at offset %08x / %08x", h+ne_shared->hdr.imptab, h+ne_shared->hdr.modtab);
	m->add_mask(line);

	FILEOFS o = h + ne_shared->hdr.modtab;
	FILEOFS no = h + ne_shared->hdr.imptab;
	char *s;
	for (int i=0; i<ne_shared->hdr.cmod; i++) {
		char buf[2];
		file->seek(o+i*2);
		if (file->read(buf, 2) != 2) break;
		int w = create_host_int(buf, 2, little_endian);
		file->seek(no+w);
		s = getstrp(file);
		sprintf(line, "%0d: %s", i+1, s);
		m->add_mask(line);
		free(s);
	}
	v->insertsub(m);

	return v;
}

format_viewer_if htneimports_if = {
	htneimports_init,
	0
};
