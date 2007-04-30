/*
 *	HT Editor
 *	htcoffimg.cc
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
#include "htcoffimg.h"
#include "strtools.h"
#include "formats.h"

#include "coff_s.h"

#include "htanaly.h"
#include "coff_analy.h"

static ht_view *htcoffimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_coff_shared_data *coff_shared = (ht_coff_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: COFF: loading image (starting analyser)...", &file->getFilename(fn));
	CoffAnalyser *p = new CoffAnalyser();
	p->init(coff_shared, file);

	Bounds c = *b;
	ht_group *g = new ht_group();
	g->init(&c, VO_RESIZE, DESC_COFF_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_coff_aviewer *v = new ht_coff_aviewer();
	v->init(&c, DESC_COFF_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, coff_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

/* search for lowest/highest */
	RVA l=(RVA)-1, h=0;
	COFF_SECTION_HEADER *s=coff_shared->sections.sections;
	for (uint i=0; i<coff_shared->sections.section_count; i++) {
		if (s->data_address < l) l = s->data_address;
		if ((s->data_address + s->data_size > h) && s->data_size) {
			h = s->data_address + s->data_size - 1;
		}
		s++;
	}
/**/
	Address *low = p->createAddress32(l);
	Address *high = p->createAddress32(h);

	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, v, p, low, high);
	v->analy_sub = analy;
	v->insertsub(analy);

	delete high;
	delete low;

	v->sendmsg(msg_complete_init, 0);

	// entrypoint
	switch (coff_shared->opt_magic) {
		case COFF_OPTMAGIC_COFF32:
			Address *tmpaddr = p->createAddress32(coff_shared->coff32header.entrypoint_address);
			v->gotoAddress(tmpaddr, NULL);
			delete tmpaddr;
			break;
	}


	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	coff_shared->v_image=v;
	return g;
}

format_viewer_if htcoffimage_if = {
	htcoffimage_init,
	0
};

/*
 *	CLASS ht_coff_aviewer
 */

void ht_coff_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_coff_shared_data *Coff_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	coff_shared = Coff_shared;
	file = File;
}

void ht_coff_aviewer::setAnalyser(Analyser *a)
{
	((CoffAnalyser *)a)->coff_shared = coff_shared;
	((CoffAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
