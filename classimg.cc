/*
 *	HT Editor
 *	classimg.cc
 *
 *	Copyright (C) 2002 Stefan Weyergraf
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

#include "class.h"
#include "class_analy.h"
#include "classimg.h"
#include "formats.h"
#include "htanaly.h"
#include "htpal.h"
#include "log.h"
#include "snprintf.h"

ht_view *htclassimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_class_shared_data *class_shared=(ht_class_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: JAVA: loading image (starting analyser)...", &file->getFilename(fn));
	ClassAnalyser *a = new ClassAnalyser();
	a->init(class_shared, file);

	Bounds c = *b;
	ht_group *g = new ht_group();
	g->init(&c, VO_RESIZE, DESC_JAVA_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_class_aviewer *v = new ht_class_aviewer();
	v->init(&c, DESC_JAVA_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, a, class_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	ht_analy_sub *analy=new ht_analy_sub();
	Address *low = a->createAddress32(0);

	Address *high = (Address*)class_shared->valid->findPrev(NULL);
	if (!high) {
		high = a->createAddress32(0);
	} else {
		high = high->clone();
		high->add(-1);
	}
	analy->init(file, v, a, low, high);
	delete low;
	delete high;
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);
	
	viewer_pos entrypoint;
	if (v->string_to_pos("entrypoint", &entrypoint)) {
		v->goto_pos(entrypoint, false);
	}

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

//	class_shared->v_image = v;
	return g;
}

format_viewer_if htclassimage_if = {
	htclassimage_init,
	0
};

/*
 *	CLASS ht_class_aviewer
 */
void ht_class_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_class_shared_data *Class_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	class_shared = Class_shared;
	file = File;
}

void ht_class_aviewer::setAnalyser(Analyser *a)
{
	((ClassAnalyser *)a)->reinit(class_shared, file);
	
	analy = a;
	analy_sub->setAnalyser(a);
}

