/* 
 *	HT Editor
 *	htnenms.cc
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

#include "htatom.h"
#include "htendian.h"
#include "htne.h"
#include "htnenms.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

#include <stdlib.h>
#include <string.h>

void assign_entrypoint_name(ht_ne_shared_data *ne_shared, UINT i, char *name)
{
	if (ne_shared->entrypoints) {
		ht_ne_entrypoint *e = (ht_ne_entrypoint*)ne_shared->entrypoints->get(i);
		if (e) {
			e->name = strdup(name);
		} else fprintf(stderr, "entry %d not available\n", i);
	} /* else trouble :-) */
}

ht_view *htnenames_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared=(ht_ne_shared_data *)group->get_shared_data();

	dword h=ne_shared->hdr_ofs;
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_NE_NAMES, VC_EDIT | VC_SEARCH, file, group);
	ht_mask_sub *m=new ht_mask_sub();
	m->init(file, 0);

	char line[256];	/* secure */
	char *n;
	int i;

	ht_snprintf(line, sizeof line, "* NE resident names table at offset %08x", h+ne_shared->hdr.restab);
	m->add_mask(line);

	file->seek(h+ne_shared->hdr.restab);
	i=0;
	while (*(n=getstrp(file))) {
		char buf[2];
		file->read(buf, 2);
		word ent = create_host_int(buf, 2, little_endian);
		if (!i) {
			ht_snprintf(line, sizeof line, "description: %s", n);
		} else {
			ht_snprintf(line, sizeof line, "%04x %s", ent, n);
			assign_entrypoint_name(ne_shared, ent, n);
		}
		free(n);
		m->add_mask(line);
		i++;
	}
	free(n);

	m->add_mask("----------------------------------------------------------------");
	ht_snprintf(line, sizeof line, "* NE non-resident names table at offset %08x", ne_shared->hdr.nrestab);
	m->add_mask(line);

	file->seek(ne_shared->hdr.nrestab);
	i=0;
	while (*(n=getstrp(file))) {
		char buf[2];
		file->read(buf, 2);
		word ent = create_host_int(buf, 2, little_endian);
		if (!i) {
			ht_snprintf(line, sizeof line, "description: %s", n);
		} else {
			ht_snprintf(line, sizeof line, "%04x %s", ent, n);
			assign_entrypoint_name(ne_shared, ent, n);
		}
		free(n);
		m->add_mask(line);
		i++;
	}
	free(n);

	v->insertsub(m);

	return v;
}

format_viewer_if htnenames_if = {
	htnenames_init,
	NULL
};
