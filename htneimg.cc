/*
 *	HT Editor
 *	htneimg.cc
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
#include "htnewexe.h"
#include "htpal.h"
#include "htneimg.h"
#include "strtools.h"
#include "formats.h"
#include "tools.h"

#include "nestruct.h"

#include "htanaly.h"
#include "ne_analy.h"

static ht_view *htneimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_ne_shared_data *ne_shared=(ht_ne_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: NE: loading image (starting analyser)...", &file->getFilename(fn));
	NEAnalyser *p = new NEAnalyser();
	p->init(ne_shared, file);

	Bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_NE_IMAGE"-g");
	AnalyInfoline *head;

	c.y += 2;
	c.h -= 2;
	ht_ne_aviewer *v=new ht_ne_aviewer();
	v->init(&c, DESC_NE_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, ne_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

/* search for lowest/highest */
	NEAddress l=(NEAddress)-1, h=0;
	NE_SEGMENT *s = ne_shared->segments.segments;
	for (uint i=0; i<ne_shared->segments.segment_count; i++) {
		NEAddress base = NE_MAKE_ADDR(i+1, 0);
		uint evsize = MAX(NE_get_seg_vsize(ne_shared, i), NE_get_seg_psize(ne_shared, i));
		if (base < l) l = base;
		if ((base + evsize > h) && (evsize)) h = base + evsize - 1;
		s++;
	}
/**/
	Address *low = p->createAddress1616(NE_ADDR_SEG(l), NE_ADDR_OFS(l));
	Address *high = p->createAddress1616(NE_ADDR_SEG(h), NE_ADDR_OFS(h));
	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, v, p, low, high);
	v->analy_sub = analy;
	v->insertsub(analy);
	delete high;
	delete low;

	v->sendmsg(msg_complete_init, 0);

	if (!(ne_shared->hdr.flags & NE_FLAGS_SELFLOAD)) {
		Address *tmpaddr = p->createAddress1616(NE_ADDR_SEG(ne_shared->hdr.csip), NE_ADDR_OFS(ne_shared->hdr.csip));
		v->gotoAddress(tmpaddr, NULL);
		delete tmpaddr;
	} /* FIXME: else what ? */

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

/*
 *	CLASS ht_ne_aviewer
 */

void ht_ne_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_ne_shared_data *NE_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	ne_shared = NE_shared;
	file = File;
}

const char *ht_ne_aviewer::func(uint i, bool execute)
{
	switch (i) {
		case 3: {
			bool e = false;
			file->cntl(FCNTL_GET_RELOC, &e);
			if (execute) {
				file->cntl(FCNTL_SET_RELOC, !e);
			}
			return e ? (char*)"unrelocate" : (char*)"relocate";
		}
	}
	return ht_aviewer::func(i, execute);
}

void ht_ne_aviewer::setAnalyser(Analyser *a)
{
	((NEAnalyser*)a)->ne_shared = ne_shared;
	((NEAnalyser*)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
