/*
 *	HT Editor
 *	htneimg.cc
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

#include "htapp.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htneimg.h"
#include "htstring.h"
#include "formats.h"
#include "tools.h"

#include "nestruct.h"

#include "htanaly.h"
#include "ne_analy.h"

ht_view *htneimage_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared=(ht_ne_shared_data *)group->get_shared_data();

	LOG("%s: NE: loading image (starting analyser)...", file->get_filename());
	ne_analyser *p = new ne_analyser();
	p->init(ne_shared, file);

	bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_NE_IMAGE"-g");
	analy_infoline *head;

	c.y += 2;
	c.h -= 2;
	ht_ne_aviewer *v=new ht_ne_aviewer();
	v->init(&c, DESC_NE_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, ne_shared);

	c.y-=2;
	c.h=2;
	head=new analy_infoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attach_infoline(head);

/* search for lowest/highest */
	ADDR l=(ADDR)-1, h=0;
	NE_SEGMENT *s = ne_shared->segments.segments;
	for (UINT i=0; i<ne_shared->segments.segment_count; i++) {
		ADDR base = (i+1)*0x10000;
		UINT evsize = MAX(NE_get_seg_vsize(ne_shared, i), NE_get_seg_psize(ne_shared, i));
		if (base < l) l = base;
		if (base + evsize > h) h = base + evsize;
		s++;
	}
/**/
	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, p, l, h);
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);

	if (!(ne_shared->hdr.flags & NE_FLAGS_BOUND)) {
		v->goto_address(ne_shared->hdr.csip);
	} /* else what ? */

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	ne_shared->v_image=v;
	return g;
}

format_viewer_if htneimage_if = {
	htneimage_init,
	0
};

