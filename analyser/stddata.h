/* 
 *	HT Editor
 *	stddata.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#ifndef STDDATA_H
#define STDDATA_H

#include "common.h"

struct area_s {
	area_s	*left, *right;
	Object	*start, *end;
};

class Area: public Object {
public:
	area_s			*a;

			void     	init();
			int 		load(ht_object_stream *f);
	virtual	void 	done();
			OBJECT_ID	object_id() const;

			void		add(Object *Start, Object *End);
			bool		contains(Object *V);
			area_s	*getArea(Object *at);
			Object	*findNext(Object *From);
			Object	*findPrev(Object *From);
			void		freeRecursive(area_s *p);
	virtual	void		store(ht_object_stream *f);
#ifdef DEBUG_FIXNEW
			void		dump();
#endif
};

#define ATOM_AREA MAGICD("AREA")

bool init_stddata();
void done_stddata();

#endif
