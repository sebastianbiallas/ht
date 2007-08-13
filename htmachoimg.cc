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
#include "strtools.h"
#include "formats.h"
#include "snprintf.h"
#include "tools.h"

#include "machostruc.h"
#include "macho_analy.h"

static ht_view *htmachoimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_macho_shared_data &macho_shared = *(ht_macho_shared_data *)group->get_shared_data();

	String fn;
	file->getFilename(fn);
	LOG("%y: Mach-O: loading image (starting analyser)...", &fn);
	MachoAnalyser *p = new MachoAnalyser();
	p->init(&macho_shared, file);

	Bounds c = *b;
	ht_group *g = new ht_group();
	g->init(&c, VO_RESIZE, DESC_MACHO_IMAGE"-g");

	c.y += 2;
	c.h -= 2;
	ht_macho_aviewer *v = new ht_macho_aviewer();
	v->init(&c, DESC_MACHO_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, &macho_shared);

	c.y -= 2;
	c.h = 2;
	AnalyInfoline *head = new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	/* find lowest/highest address */
	Address *low = NULL;
	Address *high = NULL;

	MACHOAddress l, h;
	l = -1ULL;
	h = 0;
	MACHO_SECTION_U *s = macho_shared.sections;
	for (uint i=0; i < macho_shared.section_count; i++) {
		if (macho_valid_section(s)) {
			if (s->_64) {
				if (s->s64.vmaddr < l) l = s->s64.vmaddr;
				if (s->s64.vmaddr+s->s64.vmsize > h && s->s64.vmsize) h = s->s64.vmaddr + s->s64.vmsize - 1;
			} else {
				if (s->s.vmaddr < l) l = s->s.vmaddr;
				if (s->s.vmaddr+s->s.vmsize > h && s->s.vmsize) h = s->s.vmaddr + s->s.vmsize - 1;
			}
		}
		s++;
	}
	if (macho_shared._64) {
		low = p->createAddress64(l);
		high = p->createAddress64(h);
	} else {
		low = p->createAddress32(l);
		high = p->createAddress32(h);
	}

	ht_analy_sub *analy = new ht_analy_sub();

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

	MACHO_COMMAND_U **pp = macho_shared.cmds.cmds;
	for (uint i=0; i < macho_shared.cmds.count; i++) {
		if ((*pp)->cmd.cmd == LC_UNIXTHREAD || (*pp)->cmd.cmd == LC_THREAD) {
			MACHO_THREAD_COMMAND *s = (MACHO_THREAD_COMMAND*)*pp;
			uint64 e = 0;
			switch (macho_shared.header.cputype) {
			case MACHO_CPU_TYPE_ARM:
				e = s->state.state_arm.pc;
				break;
			case MACHO_CPU_TYPE_POWERPC:
				e = s->state.state_ppc.srr[0];
				break;
			case MACHO_CPU_TYPE_I386:
				e = s->state.state_i386.eip;
				break;
			case MACHO_CPU_TYPE_X86_64:
				e = s->state.state_x86_64.rip;
				break;
			case MACHO_CPU_TYPE_POWERPC64:
				e = s->state.state_ppc64.srr[0];
				break;
			default: assert(0);
			}
			Address *entry;
			if (macho_shared._64) {
				entry = p->createAddress64(e);
			} else {
				entry = p->createAddress32(e);
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

	return g;
}

format_viewer_if htmachoimage_if = {
	htmachoimage_init,
	0
};

/*
 *	CLASS ht_macho_aviewer
 */
void ht_macho_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_macho_shared_data *MACHO_shared)
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
