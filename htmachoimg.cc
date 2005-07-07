/*
 *	HT Editor
 *	htmachoimg.cc
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
#include "htmachoimg.h"
#include "htpal.h"
#include "htstring.h"
#include "formats.h"
#include "snprintf.h"
#include "tools.h"

#include "machostruc.h"
#include "macho_analy.h"

static ht_view *htmachoimage_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_macho_shared_data *macho_shared=(ht_macho_shared_data *)group->get_shared_data();

//	if (macho_shared->ident.e_ident[MACHO_EI_CLASS]!=MACHOCLASS32) return 0;

	LOG("%s: Mach-O: loading image (starting analyser)...", file->get_filename());
	MachoAnalyser *p = new MachoAnalyser();
	p->init(macho_shared, file);

	bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_MACHO_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_macho_aviewer *v=new ht_macho_aviewer();
	v->init(&c, DESC_MACHO_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, macho_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

/* find lowest/highest address */
	Address *low = NULL;
	Address *high = NULL;

	MACHOAddress l, h;
	l = (uint32)-1;
	h = 0;
	MACHO_SECTION *s = macho_shared->sections.sections;
	for (UINT i=0; i < macho_shared->sections.count; i++) {
		if (macho_valid_section(s, 0)) {
			if (s->vmaddr < l) l = s->vmaddr;
			if ((s->vmaddr + s->vmsize > h) && s->vmsize) h=s->vmaddr + s->vmsize - 1;
		}
		s++;
	}
	low = p->createAddress32(l);
	high = p->createAddress32(h);

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

	Address *tmpaddr = NULL;
/*	switch (macho_shared->ident.e_ident[MACHO_EI_CLASS]) {
		case MACHOCLASS32: {
			tmpaddr = p->createAddress32(macho_shared->header32.e_entry);
			break;
		}
		case MACHOCLASS64: {
			tmpaddr = p->createAddress64(macho_shared->header64.e_entry);
			break;
		}
	}*/
//	tmpaddr = p->createAddress32(macho_shared->header32.e_entry);
	
//	v->gotoAddress(tmpaddr, NULL);
//	delete tmpaddr;
	MACHO_COMMAND_U **pp = macho_shared->cmds.cmds;
	for (UINT i=0; i < macho_shared->cmds.count; i++) {
		if (((*pp)->cmd.cmd == LC_UNIXTHREAD) || ((*pp)->cmd.cmd == LC_THREAD)) {
			MACHO_THREAD_COMMAND *s = (MACHO_THREAD_COMMAND*)*pp;
			Address *entry;
			switch (macho_shared->header.cputype) {
			case MACHO_CPU_TYPE_POWERPC:
				entry = p->createAddress32(s->state.state_ppc.srr0);
				break;
			case MACHO_CPU_TYPE_I386:
				entry = p->createAddress32(s->state.state_i386.eip);
				break;
			default: assert(0);
			}
			v->gotoAddress(entry, NULL);
			delete entry;
			break;
		}
		pp++;
	}

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

//	macho_shared->v_image=v;
	return g;
}

format_viewer_if htmachoimage_if = {
	htmachoimage_init,
	0
};

/*
 *	CLASS ht_macho_aviewer
 */
void ht_macho_aviewer::init(bounds *b, char *desc, int caps, ht_streamfile *File, ht_format_group *format_group, Analyser *Analy, ht_macho_shared_data *MACHO_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	macho_shared = MACHO_shared;
}

void ht_macho_aviewer::setAnalyser(Analyser *a)
{
	((MachoAnalyser *)a)->macho_shared = macho_shared;
	((MachoAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
