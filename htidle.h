/* 
 *	HT Editor
 *	htidle.h
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

#ifndef __HTIDLE_H__
#define __HTIDLE_H__

#include "data.h"
void register_idle_object(Object *o);
void unregister_idle_object(Object *o);
void do_idle();

/*
 *	INIT
 */

bool init_idle();

/*
 *	DONE
 */

void done_idle();

#endif
