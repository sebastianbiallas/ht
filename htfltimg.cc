/*
 *	HT Editor
 *	htfltimg.cc
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

#include "log.h"
#include "htfltimg.h"
#include "htpal.h"
#include "strtools.h"
#include "formats.h"
#include "snprintf.h"
#include "tools.h"

#include "fltstruc.h"
#include "flt_analy.h"

static ht_view *htfltimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_flt_shared_data *flt_shared=(ht_flt_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: FLAT: loading image (starting analyser)...", &file->getFilename(fn));
	FLTAnalyser *p = new FLTAnalyser();
	p->init(flt_shared, file);

	Bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_FLT_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_flt_aviewer *v=new ht_flt_aviewer();
	v->init(&c, DESC_FLT_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, flt_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	/* find lowest/highest address */
	Address *low;
	Address *high;

	low = p->createAddress32(flt_shared->code_start);
	high = p->createAddress32(flt_shared->bss_end-1);

	ht_analy_sub *analy=new ht_analy_sub();

	if (low->compareTo(high) < 0) {
		analy->init(file, v, p, low, high);
		v->analy_sub = analy;
		v->insertsub(analy);
	} else {
		delete analy;
		v->done();
		delete v;
		head->done();          
		delete head;
		g->done();
		delete g;
		delete high;
		delete low;
		return NULL;
	}
	
	delete high;
	delete low;

	v->sendmsg(msg_complete_init, 0);

	Address *tmpaddr = p->createAddress32(flt_shared->header.entry);	
	v->gotoAddress(tmpaddr, NULL);
	delete tmpaddr;

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

//	macho_shared->v_image=v;
	return g;
}

format_viewer_if htfltimage_if = {
	htfltimage_init,
	0
};

/*
 *	CLASS ht_flt_aviewer
 */
void ht_flt_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_flt_shared_data *FLT_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	flt_shared = FLT_shared;
}

void ht_flt_aviewer::setAnalyser(Analyser *a)
{
	((FLTAnalyser *)a)->flt_shared = flt_shared;
	((FLTAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
