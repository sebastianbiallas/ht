/*
 *	HT Editor
 *	stddata.cc
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

#include <stdlib.h>

#include "analy.h"

#include "atom.h"
#include "stddata.h"
#include "stream.h"
#include "tools.h"

/*
 *   Area
 */
void Area::init()
{
	a = NULL;
}

static void areaload(ObjectStream &st, area_s *&p, int level, int &left)
{
	if (left <= 0) {
		p = NULL;
		return;
	}
	p = new area_s;
	if (level <= 1 || left <= 1) {
		p->start = st.getObject("start");
		p->end = st.getObject("end");
		p->left = p->right = NULL;
		left--;
	} else {
		areaload(st, p->left, level / 2, left);
		p->start = st.getObject("start");
		p->end = st.getObject("end");
		left--;
		areaload(st, p->right, level / 2 -1, left);
	}
}

void	Area::load(ObjectStream &st)
{
	int count;
	GET_INT32D(st, count);
	areaload(st, a, count, count);
}

void Area::done()
{
	freeRecursive(a);
}

ObjectID Area::getObjectID() const
{
	return ATOM_AREA;
}

static area_s *do_clone(area_s *p)
{
	if (p) {
		area_s *q = new area_s;
		q->left = do_clone(p->left);
		q->right = do_clone(p->right);
		q->start = p->start->clone();
		q->end = p->end->clone();
		return q;
	} else {
		return NULL;
	}
}

Area *Area::clone() const
{
	Area *area = new Area();
	area->init();
	area->a = do_clone(a);
	return area;
}

static area_s *areaget(area_s *p, Object *V)
{
	if (p) {
		if (V->compareTo(p->start) < 0) return areaget(p->left, V);
		if (V->compareTo(p->end) >= 0) return areaget(p->right, V);
		/*if ((V >= (p->start)) && (V < (p->end)))*/ return p;
	} else return NULL;
}

area_s *Area::getArea(Object *at)
{
	return areaget(a, at);
}

static void areaadd(area_s *&p, Object *Start, Object *End)
{
	if (p) {
		if (Start->compareTo(p->start) >= 0 && Start->compareTo(p->end)<=0) {
			if (p->end->compareTo(End) < 0) {
				delete p->end;
				p->end = End->clone();
			}
			if ((End->compareTo(p->start) >= 0) && (End->compareTo(p->end)<=0)) {
				if (p->start->compareTo(Start) > 0) {
					delete p->start;
					p->start = Start->clone();
				}
			}
			return;
		}
		if (End->compareTo(p->start) >= 0 && End->compareTo(p->end) <= 0) {
			if (p->start->compareTo(Start) > 0) {
				delete p->start;
				p->start = Start->clone();
			}
			return;
		}
		if (Start->compareTo(p->end) > 0) areaadd(p->right, Start, End);
					  else  areaadd(p->left, Start, End);
	} else {
		// new p
		area_s *tmp = new area_s;
		p = tmp;
		p->start = Start->clone();
		p->end = End->clone();
		p->left = NULL;
		p->right = NULL;
	}
}

void Area::add(Object *Start, Object *End)
{
	areaadd(a, Start, End);
}

static bool areacontains(area_s *p, Object *V)
{
	if (p) {
		if (V->compareTo(p->start) < 0) return areacontains(p->left, V);
		if (V->compareTo(p->end) >= 0) return areacontains(p->right, V);
		/*if ((V >= (p->start)) && (V < (p->end)))*/ return true;
	} else return false;
}

bool Area::contains(Object *v)
{
	if (instanceOf<InvalidAddress>(v)) return false;
	return areacontains(a, v);
}

static void areafindnext(area_s *p, Object *from, Object **res)
{
	if (!from || from->compareTo(p->start) < 0) {
		*res = p->start;
		if (p->left) areafindnext(p->left, from, res);
	} else if (from->compareTo(p->end) >= 0) {
		if (p->right) areafindnext(p->right, from, res);
	} else *res = from;
}

Object *Area::findNext(Object *from)
{
	Object *res = NULL;
	if (a) areafindnext(a, from, &res);
	return res;
}

static void areafindprev(area_s *p, Object *from, Object **res)
{
//FIXME ??:
	if (p) {
		if (!from || from->compareTo(p->end) >= 0) {
			*res = p->end;
			areafindprev(p->right, from, res);
		} else if (from->compareTo(p->start) < 0) {
			areafindprev(p->left, from, res);
		} else *res = from;
	};
}

Object *Area::findPrev(Object *from)
{
	Object *res = NULL;
	areafindprev(a, from, &res);
	return res;
}

void Area::freeRecursive(area_s *p)
{
	if (p) {
		freeRecursive(p->left);
		freeRecursive(p->right);
		delete p->start;
		delete p->end;
		delete p;
	}
}

static void areacount(area_s *p, int &c, Object **startend)
{
	if (p) {
		areacount(p->left, c, startend);
		if (!*startend || p->start->compareTo(*startend) != 0) c++;
		*startend = p->end;
		areacount(p->right, c, startend);
	}
}

static void areastore(ObjectStream &f, area_s *p, Object **startend)
{
	if (p) {
		areastore(f, p->left, startend);
		if (!*startend) {
			f.putObject(p->start, "start");
		} else {
			if ((*startend)->compareTo(p->start) != 0) {
				f.putObject(*startend, "end");
				f.putObject(p->start, "start");
			}
		}
		*startend = p->end;
		areastore(f, p->right, startend);
	}
}

void Area::store(ObjectStream &f) const
{
	int count = 0;
	Object *start = NULL;
	areacount(a, count, &start);
	PUT_INT32D(f, count);
	start = NULL;
	areastore(f, a, &start);
	if (start != NULL) f.putObject(start, "end");
}

#ifdef DEBUG_FIXNEW
static void areadump(int sp, area_s *A)
{
	if (A) {
		for (int i=0; i<sp; i++) printf(" ");
		char buf[1024];
		ht_snprintf(buf, sizeof buf, "%y %y\n", A->start, A->end);
		printf(buf);
		++sp;++sp;
		areadump(sp, A->left);
		areadump(sp, A->right);
	}
}

void Area::dump()
{
	areadump(1, a);
}
#endif

BUILDER(ATOM_AREA, Area, Object)

bool init_stddata()
{
	REGISTER(ATOM_AREA, Area)
	return true;
}

void done_stddata()
{
	UNREGISTER(ATOM_AREA, Area)
}
