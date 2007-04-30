/* 
 *	HT Editor
 *	common.cc
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

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "snprintf.h"

Object::Object()
{
#ifdef HTDEBUG
	initialized = false;
	destroyed = false;
#endif
}

Object::~Object()
{
#ifdef HTDEBUG
	if (initialized && !destroyed) {
		printf("shit !!! object not destroyed\n");
		exit(66);
//		int exit=1;
	}
#endif
}

void	Object::init()
{
#ifdef HTDEBUG
	initialized = true;
#endif
}

void Object::done()
{
#ifdef HTDEBUG
	destroyed = true;
#endif
}

/**
 *	Standard Object comparator.
 *	@param obj object to compare to
 *	@returns 0 for equality, negative number if |this<obj| and positive number if |this>obj|
 */
int Object::compareTo(const Object *obj) const
{
	assert(0);
	return 0;
}

/**
 *	Standard Object duplicator.
 *   @returns copy of object
 */
Object *Object::duplicate()
{
	Object *obj = new Object();
	obj->init();
	return obj;
}

/**
 *	Standard Object idle function.
 *	Overwrite and register with htidle.cc:register_idle()
 *	@returns true if working, false if really idle
 */
bool Object::idle()
{
	return false;
}

/**
 *	@returns true if |this| is an object or derived of an object of type |id|.
 */
bool Object::instanceOf(OBJECT_ID id)
{
	return id == object_id();
}

bool Object::instanceOf(Object *o)
{
	return instanceOf(o->object_id());
}

/**
 *	loads object from object stream.
 *	@returns ht_stream errorcode
 */
int Object::load(ht_object_stream *s)
{
	return 0;
}

/**
 *	@returns unique object id.
 */
OBJECT_ID Object::object_id() const
{
	return ATOM_OBJECT;
}

/**
 *	stores object.
 */
void Object::store(ht_object_stream *s)
{
}

/**
 *   Prints object in string |s|
 *	@returns number of characters written in s
 */
int Object::toString(char *s, int maxlen)
{
	OBJECT_ID oid = object_id();
	unsigned char c1 = (oid >> 24);
	unsigned char c2 = (oid >> 16) & 0xff;
	unsigned char c3 = (oid >>  8) & 0xff;
	unsigned char c4 = oid & 0xff;
	return ht_snprintf(s, maxlen, "[OBJECT (%c%c%c%c)]", c1?c1:'0', c2?c2:'0', c3?c3:'0', c4?c4:'0');
}

