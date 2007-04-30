/*
 *	HT Editor
 *	htidle.cc
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

#include "data.h"
#include "htdebug.h"
#include "htidle.h"
#include "keyb.h"
#include "sys.h"

static List *idle_objs;

void register_idle_object(Object *o)
{
	idle_objs->insert(o);
}

void unregister_idle_object(Object *o)
{
	int c = idle_objs->count();
	for (int i=0; i < c; i++) if ((*idle_objs)[i] == o) {
		idle_objs->remove(idle_objs->findByIdx(i));
	}
}

void do_idle()
{
	static int cur_idle = 0;
	static bool any_idles = 0;
	int c = idle_objs->count();
	if (c) {
		if (cur_idle >= c) {
			cur_idle = 0;
			if (!any_idles) sys_suspend();
			any_idles = 0;
		}
		Object *i = (*idle_objs)[cur_idle];
		assert(i);
		any_idles |= i->idle();
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
	idle_objs = new Array(false);
	return true;
}

/*
 *	DONE
 */

void done_idle()
{
	delete idle_objs;
}
