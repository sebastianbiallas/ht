/*
 *	HT Editor
 *	stddata.cc
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

#include <stdlib.h>
#include "stddata.h"
#include "tools.h"
#include "htatom.h"

/****************************************************************************/
void queue::init()
{
	first = last = NULL;
}

int	queue::load(ht_object_stream *st)
{
	int Count;
	Count = st->get_int_dec(4, "count");
	for (int i=0; i<Count; i++) {
		void *data;
		if (!load_element(st, &data)) {
			return 1;
		}
		// FIXME: does this reverse the order?
		enqueue(data);
	}
	return st->get_error();
}

int	queue::load_element(ht_object_stream *f, void **data)
{
	return 0;
}

void queue::done()
{
	clear();
}

void queue::clear()
{
	while (!empty()) {
		free_element(dequeue());
	}
}

int	queue::count()
{
	queueitem	*tmp = first;
	int			result = 0;
	while (tmp) {
		result++;
		tmp = tmp->next;
	}
	return result;
}

bool queue::empty()
{
	return (first==NULL);
}

void queue::free_element(void *data)
{
	free(data);
}

void queue::enqueue(void *p)
{
	queueitem *tmp = (queueitem *) smalloc(sizeof(queueitem));
	tmp->next = NULL;
	if (first == NULL) {
		first = last = tmp;
	} else {
		last->next = tmp;
		last = tmp;
	}
	last->data = p;
}

void *queue::dequeue()
{
	void	*ret = top();
	if (!empty()) {
		queueitem *tmp = first;
		first = first->next;
		free(tmp);
		if (!first) last = NULL;
	}
	return ret;
}

void	queue::store(ht_object_stream *f)
{
	int Count = count();
	PUT_INT_DEC(f, Count);
	queueitem *tmp = first;
	while (tmp) {
		store_element(f, tmp->data);
		tmp = tmp->next;
	}
}

void	queue::store_element(ht_object_stream *f, void *data)
{
}

void *queue::top()
{
	return empty() ? NULL : first->data;
}

/*
 *
 */

void tstack::init()
{
	stop = NULL;
}

void tstack::done()
{
	clear();
}

bool tstack::empty()
{
	return (stop==NULL);
}

void tstack::clear()
{
	while (!empty()) pop();
}

void *tstack::top()
{
	return empty() ? NULL : stop->data;
}

void tstack::push(void *p)
{
//	tstackitem *tmp = (tstackitem *) smalloc(sizeof(tstackitem));
}

void *tstack::pop()
{
	void	*ret = top();
	if (!empty()) {
		tstackitem *tmp = stop;
		stop = stop->next;
		free(tmp);
	}
	return ret;
}

/*
 *   area
 */
void area::init()
{
	a = NULL;
}

void areaload(ht_object_stream *st, area_s *&p, int level, int &left)
{
	if (left<=0) {
		p = NULL;
		return;
	}
	p = (area_s *) smalloc0(sizeof(area_s));
	if ((level<=1) || (left<=1)) {
		p->start = st->get_int_hex(sizeof (dword), "start");
		p->end = st->get_int_hex(sizeof (dword), "end");
		p->left = p->right = NULL;
		left--;
	} else {
		areaload(st, p->left, level / 2, left);
		p->start = st->get_int_hex(sizeof (dword), "start");
		p->end = st->get_int_hex(sizeof (dword), "end");
		left--;
		areaload(st, p->right, level / 2 -1, left);
	}
}

int	area::load(ht_object_stream *st)
{
	int count;
	count = st->get_int_dec(sizeof count, "count");
	areaload(st, a, count, count);
	return st->get_error();
}

void area::done()
{
	free_recursive(a);
}

OBJECT_ID	area::object_id()
{
	return ATOM_AREA;
}

area_s *areaget(area_s *p, dword V)
{
	if (p) {
		if (V < (p->start)) return areaget(p->left, V);
		if (V >= (p->end)) return areaget(p->right, V);
		/*if ((V >= (p->start)) && (V < (p->end)))*/ return p;
	} else return NULL;
}

area_s *area::get_area(dword at)
{
	return areaget(a, at);
}

void areaadd(area_s *&p, dword Start, dword End)
{
	if (p) {
		if ((Start >= p->start) && (Start <= p->end)) {
			if (p->end < End) p->end = End;
			if ((End >= p->start) && (End <= p->end)) {
				if (p->start > Start) p->start = Start;
			}
			return;
		}
		if ((End >= p->start) && (End <= p->end)) {
			if (p->start > Start) p->start = Start;
			return;
		}
		if (Start > p->end) areaadd(p->right, Start, End);
					  else  areaadd(p->left, Start, End);
	} else {
		// new p
		area_s *tmp = (area_s *) smalloc(sizeof(area_s));
		p = tmp;
		p->start = Start;
		p->end = End;
		p->left = NULL;
		p->right = NULL;
	}
}

void area::add(dword Start, dword End)
{
	areaadd(a, Start, End);
}

bool areacontains(area_s *p, dword V)
{
	if (p) {
		if (V < (p->start)) return areacontains(p->left, V);
		if (V >= (p->end)) return areacontains(p->right, V);
		/*if ((V >= (p->start)) && (V < (p->end)))*/ return true;
	} else return false;
}

bool area::contains(dword V)
{
	return areacontains(a, V);
}

void areafindnext(area_s *p, dword From, dword *res)
{
	if (From < p->start) {
		*res = p->start;
		if (p->left) areafindnext(p->left, From, res);
	} else if (From >= p->end) {
		if (p->right) areafindnext(p->right, From, res);
	} else *res = From;
}

dword  area::find_next(dword From)
{
	dword res = 0xffffffff;
	if (a) areafindnext(a, From, &res);
	return res;
}

void areafindprev(area_s *p, dword From, dword *res)
{
//FIXME ??:
	if (p) {
		if (From < p->start) {
			if (p->left) areafindprev(p->left, From, res);
		} else if (From >= p->end) {
			*res = p->start;
			areafindprev(p->right, From, res);
		} else *res = From;
	} else *res = 0x0;
}

dword area::find_prev(dword From)
{
	dword res;
	areafindprev(a, From, &res);
	return res;
}

void area::free_recursive(area_s *p)
{
	if (p) {
		free_recursive(p->left);
		free_recursive(p->right);
		free(p);
	}
}

void areacount(area_s *p, int &c, dword &startend)
{
	if (p) {
		areacount(p->left, c, startend);
		if (p->start!=startend) c++;
		startend = p->end;
		areacount(p->right, c, startend);
	}
}

void areastore(ht_object_stream *f, area_s *p, dword &startend)
{
	if (p) {
		areastore(f, p->left, startend);
		if (startend == 0xffffffff) {
			f->put_int_hex(p->start, sizeof(dword), "start");
		} else {
			if (startend!=p->start) {
				f->put_int_hex(startend, sizeof(dword), "end");
				f->put_int_hex(p->start, sizeof(dword), "start");
			}
		}
		startend = p->end;
		areastore(f, p->right, startend);
	}
}

void area::store(ht_object_stream *f)
{
	int count = 0;
	dword start = 0xffffffff;
	areacount(a, count, start);
	PUT_INT_DEC(f, count);
	start = 0xffffffff;
	areastore(f, a, start);
	if (start!=0xffffffff) f->put_int_hex(start, sizeof(dword), "end");
}

#ifdef DEBUG
void areadump(int sp, area_s *A)
{
	if (A) {
		for (int i=0; i<sp; i++) printf(" ");
		printf("%08x %08x\n", A->start, A->end);
		++sp;++sp;
		areadump(sp, A->left);
		areadump(sp, A->right);
	}
}

void area::dump()
{
	areadump(1, a);
}
#endif

/*
 *  BUILDER etc.
 */

BUILDER(ATOM_QUEUE, queue)
BUILDER(ATOM_AREA, area)

bool init_stddata()
{
	REGISTER(ATOM_QUEUE, queue)
	REGISTER(ATOM_AREA, area)
	return true;
}

void done_stddata()
{
	UNREGISTER(ATOM_QUEUE, queue)
	UNREGISTER(ATOM_AREA, area)
}
