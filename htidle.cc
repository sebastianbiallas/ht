/*
 *	HT Editor
 *	htidle.cc
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

#include "htdata.h"
#include "htdebug.h"
#include "htidle.h"
#include "htkeyb.h"
#include "htsys.h"

ht_clist *idle_objs;

int cur_idle=0;
bool any_idles=0;

void register_idle_object(Object *o)
{
	idle_objs->insert(o);
}

void unregister_idle_object(Object *o)
{
	int c=idle_objs->count();
	for (int i=0; i<c; i++) if (idle_objs->get(i)==o) {
		idle_objs->remove(i);
	}
}

void do_idle()
{
	int c=idle_objs->count();
	if (c) {
		if (cur_idle>=c) {
			cur_idle=0;
			if (!any_idles) sys_suspend();
			any_idles=0;
		}
		Object *i=idle_objs->get(cur_idle);
		assert(i);
		any_idles|=i->idle();
		cur_idle++;
	} else {
		sys_suspend();
	}
}

/*
 *	INIT
 */

bool init_idle()
{
	idle_objs=new ht_clist();
	idle_objs->init();
	return true;
}

/*
 *	DONE
 */

void done_idle()
{
	idle_objs->done();
	delete idle_objs;
}
