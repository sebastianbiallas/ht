/* 
 *	HT Editor
 *	stddata.h
 *
 *	Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@web-productions.de)
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

#ifndef stddata_h
#define stddata_h

#include <stdio.h>
#include "common.h"

/*
class tdata {
public:
				tdata() {};
				~tdata() {};
	void			init();
	void			load(FILE *f);
	void	virtual	done();
	void virtual	store();
};
*/


struct queueitem {
	queueitem	*next;
	void			*data;
};

class queue: public object {
public:
	queueitem			*first, *last;
					queue() {};
	virtual	~queue() {};
			void		init();
			int		load(ht_object_stream *f);
	virtual	int 		load_element(ht_object_stream *f, void **data);
	virtual	void		done();
			void		clear();
			int		count();
			bool		empty();
			void		enqueue(void *p);
			void 	*dequeue();
	virtual	void 	free_element(void *data);
	virtual	void		store(ht_object_stream *f);
	virtual	void		store_element(ht_object_stream *f, void *data);
			void		*top();
};

struct tstackitem {
	tstackitem	*next;
	void			*data;
};

class tstack {
public:
	tstackitem		*stop;
					tstack() {};
	virtual			~tstack() {};
			void		init();
			int		load(ht_object_stream *f);
	virtual   int 		load_element(ht_object_stream *f, void **data);
	virtual	void		done();
			void		clear();
			int		count();
			bool		empty();
	virtual 	void 	free_element(void *data);
			void		*top();
			void		push(void *p);
			void		*pop();
	virtual	void		store(ht_stream *f);
	virtual	void		store_element(ht_object_stream *f, void *data);
};

struct area_s {
	area_s	*left, *right;
	dword	start, end;
};

class area: public object {
public:
	area_s			*a;

			void     	init();
			int 		load(ht_object_stream *f);
	virtual	void 	done();
			OBJECT_ID	object_id();

			void		add(dword Start, dword End);
			bool		contains(dword V);
			area_s	*get_area(dword at);
			dword	find_next(dword From);
			dword	find_prev(dword From);
			void		free_recursive(area_s *p);
	virtual	void		store(ht_object_stream *f);
#ifdef DEBUG
			void		dump();
#endif
};

#define ATOM_QUEUE MAGICD("UEUE")
#define ATOM_AREA MAGICD("AREA")

bool init_stddata();
void done_stddata();

#endif
