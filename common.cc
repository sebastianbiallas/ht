/* 
 *	HT Editor
 *	common.cc
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

#include "common.h"

#include <stdio.h>
#include <stdlib.h>

object::object()
{
	initialized=0;
	destroyed=0;
}

object::~object()
{
	if (initialized && !destroyed) {
		printf("shit !!! object not destroyed\n");
		exit(66);
//		int exit=1;
	}
}

void	object::init()
{
	initialized=1;
}

void object::done()
{
	destroyed=1;
}

object *object::duplicate()
{
	object *o=new object();
	o->init();
	return o;
}

bool object::idle()
{
	return 0;
}

int  object::load(ht_object_stream *s)
{
	return 0;
}

void object::store(ht_object_stream *s)
{
}

OBJECT_ID object::object_id()
{
	return 0;
}

