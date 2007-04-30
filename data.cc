/*
 *	HT Editor
 *	data.cc
 *
 *	Copyright (C) 2002 Stefan Weyergraf (stefan@weyergraf.de)
 *	Copyright (C) 2002, 2003 Sebastian Biallas (sb@biallas.net)
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

#include <new>
#include <cstring>
#include <cstdlib>
#include <typeinfo>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "atom.h"
#include "except.h"

#include "data.h"
#include "htdebug.h"
#include "snprintf.h"
#include "stream.h"
#include "store.h"

int autoCompare(const Object *a, const Object *b)
{
// FIXME: better use instanceOf
// SB: warum auskommentieren?
// SW: weil nicht so gute logik
//     wie gesagt FIXME, aber bin mir unsicher wie genau

/*	uint	ida = a->getObjectID();
	uint idb = b->getObjectID();
	if (ida != idb) return ida-idb;*/
	return a->compareTo(b);
}

/*
 *	Object
 */

int Object::compareTo(const Object *obj) const
{
//	int a=1;
	throw NotImplementedException(HERE);
}

int Object::toString(char *buf, int buflen) const
{
	ObjectID oid = getObjectID();
	unsigned char c[20];
	int l = 4;
	c[0] = (oid >> 24) & 0xff;
	c[1] = (oid >> 16) & 0xff;
	c[2] = (oid >>  8) & 0xff;
	c[3] = oid & 0xff;
	for (int i = 0; i < 4; i++) {
		if (c[i] < 32 || c[i] > 127) {
			c[l] = '\\';
			c[l+1] = "0123456789abcdef"[c[i] >> 4];
			c[l+2] = "0123456789abcdef"[c[i] & 0xf];
			l += 3;
		} else {
			c[l] = c[i];
			l++;
		}
	}
	c[l] = 0;
	return ht_snprintf(buf, buflen, "Object-%s", c+4);
}

Object *Object::clone() const
{
	throw NotImplementedException(HERE);
}

bool	Object::idle()
{
	return false;
}

void Object::load(ObjectStream &s)
{
}

ObjectID Object::getObjectID() const
{
	return OBJID_OBJECT;
}

void	Object::store(ObjectStream &s) const
{
}

/*
 *	Enumerator
 */
/*Object *Enumerator::operator [] (int idx) const
{
	ObjHandle h = findByIdx(idx);
	return (h != invObjHandle) ? get(h) : NULL;
}*/

ObjHandle Enumerator::find(const Object *key) const
{
	ObjHandle h = findFirst();
	while (h != invObjHandle) {
		if (compareObjects(get(h), key) == 0) return h;
		h = findNext(h);
	}
	return invObjHandle;
}

ObjHandle Enumerator::findG(const Object *key) const
{
	ObjHandle h = findFirst();
	ObjHandle best = invObjHandle;
	while (h != invObjHandle) {
		int c = compareObjects(get(h), key);
		if (c > 0) {
			// h is greater than key
			if (best != invObjHandle) {
				if (compareObjects(get(h), get(best)) < 0) best = h;
			} else {
				best = h;
			}
		}
		h = findNext(h);
	}
	return best;
}

ObjHandle Enumerator::findGE(const Object *key) const
{
	ObjHandle h = findFirst();
	ObjHandle best = invObjHandle;
	while (h != invObjHandle) {
		int c = compareObjects(get(h), key);
		if (c == 0) return h;
		if (c > 0) {
			// h is greater than key
			if (best != invObjHandle) {
				if (compareObjects(get(h), get(best)) < 0) best = h;
			} else {
				best = h;
			}
		}
		h = findNext(h);
	}
	return best;
}

ObjHandle Enumerator::findLE(const Object *key) const
{
	ObjHandle h = findFirst();
	ObjHandle best = invObjHandle;
	while (h != invObjHandle) {
		int c = compareObjects(get(h), key);
		if (c == 0) return h;
		if (c < 0) {
			// h is lower than key
			if (best != invObjHandle) {
				if (compareObjects(get(h), get(best)) > 0) best = h;
			} else {
				best = h;
			}
		}
		h = findNext(h);
	}
	return best;
}

ObjHandle Enumerator::findL(const Object *key) const
{
	ObjHandle h = findFirst();
	ObjHandle best = invObjHandle;
	while (h != invObjHandle) {
		int c = compareObjects(get(h), key);
		if (c < 0) {
			// h is lower than key
			if (best != invObjHandle) {
				if (compareObjects(get(h), get(best)) > 0) best = h;
			} else {
				best = h;
			}
		}
		h = findNext(h);
	}
	return best;
}

int Enumerator::toString(char *buf, int buflen) const
{
	ObjHandle h = findFirst();
	int n = 0;
	if (buflen>1) { *buf++ = '('; buflen--; n++; }
	while ((buflen>0) && h) {
		Object *d = get(h);
		int k = d->toString(buf, buflen);
		buflen -= k;
		buf += k;
		n += k;
		bool comma;
		if (buflen>1) { *buf++ = ','; buflen--; n++; comma = true; } else comma = false;
		h = findNext(h);
		if (!h && comma) { buf--; buflen++; n--; }
	}
	if (buflen>1) { *buf++ = ')'; buflen--; n++; }
	if (buflen>0) *buf++ = 0;
	return n;
}

/*
 *	Container
 */
Container::Container()
{
	hom_objid = OBJID_TEMP;
}

bool Container::delObj(Object *sig)
{
	return del(find(sig));
}

ObjHandle Container::findOrInsert(Object *obj, bool &inserted)
{
	ObjHandle h = find(obj);
	if (h == invObjHandle) {
		h = insert(obj);
		inserted = true;
	} else {
		inserted = false;
	}
	return h;
}

void Container::notifyInsertOrSet(const Object *o)
{
	if (!o) return;
	if (hom_objid == OBJID_TEMP) {
		hom_objid = o->getObjectID();
	} else if (hom_objid != OBJID_INVALID) {
		if (hom_objid != o->getObjectID()) hom_objid = OBJID_INVALID;
	}
}

Object *Container::removeObj(const Object *sig)
{
	return remove(find(sig));
}

/*
 *	Array
 */

#define ARRAY_ALLOC_MIN			4

/*
 *	grow array size by factor (ARRAY_ALLOC_GROW_NUM / ARRAY_ALLOC_GROW_DENOM)
 *	but never by more than ARRAY_ALLOC_GROW_ABSMAX.
 */
#define ARRAY_ALLOC_GROW_NUM		3
#define ARRAY_ALLOC_GROW_DENOM	2

#define ARRAY_ALLOC_GROW_ABSMAX	64*1024

Array::Array(bool oo, int prealloc)
{
	own_objects = oo;
	ecount = 0;
	acount = 0;
	elems = NULL;

	realloc(prealloc);
}

Array::~Array()
{
	delAll();
}

void Array::delAll()
{
	// SB: Doppelte ueberpruefung von oo
	if (elems) {
		if (own_objects) {
			for (uint i=0; i<ecount; i++) {
				freeObj(elems[i]);
			}
		}
		free(elems);
	}
	ecount = 0;
	acount = 0;
	elems = NULL;
}

int Array::delRange(int start, int end)
{
	if (!ecount) return 0;
	if (start < 0) start = 0;
	if (start > end) return 0;
	if ((uint)end >= ecount) end = ecount - 1;
	if (own_objects) {
		uint ende = end;
		for (uint i = start; i <= ende; i++) {
			freeObj(elems[i]);
		}
	}
	memmove(elems+start, elems+end+1, sizeof (*elems) * (ecount-end-1));
	ecount -= end-start+1;
	checkShrink();
	return end-start+1;
}

Array *Array::clone() const
{
	Array *a = new Array(own_objects, ecount);
	for (uint i = 0; i<ecount; i++) {
		Object *e = get(findByIdx(i));
		if (own_objects) e = e->clone();
		a->insert(e);
	}
	return a;
}

void Array::load(ObjectStream &s)
{
	own_objects = true;

	GET_INT32D(s, ecount);
	acount = 0;
	elems = NULL;
	realloc(ecount);
	if (ecount) {
		GET_INT32X(s, hom_objid);

		for (uint i=0; i < ecount; i++) {
			Object *obj = s.getObjectInternal("element", hom_objid);
			elems[i] = obj;
		}
	} else {
		hom_objid = OBJID_TEMP;	
	}
}

ObjectID Array::getObjectID() const
{
	return OBJID_ARRAY;
}

void Array::store(ObjectStream &s) const
{
	PUT_INT32D(s, ecount);
	if (ecount) {
		assert(hom_objid != OBJID_TEMP);
		putIDComment(s, hom_objid);
		PUT_INT32X(s, hom_objid);

		for (uint i = 0; i<ecount; i++) {
			s.putObject(elems[i], "element", hom_objid);
		}
	}
}

int Array::calcNewBufferSize(int curbufsize, int min_newbufsize) const
{
	int n = curbufsize;
	if (n < ARRAY_ALLOC_MIN) n = ARRAY_ALLOC_MIN;
	if (n < min_newbufsize) {
		while (n < min_newbufsize) {
			n *= ARRAY_ALLOC_GROW_NUM;
			n /= ARRAY_ALLOC_GROW_DENOM;
		}
	} else {
		while (n > min_newbufsize) {
			n *= ARRAY_ALLOC_GROW_DENOM;
			n /= ARRAY_ALLOC_GROW_NUM;
		}
	}
	if (n-curbufsize > ARRAY_ALLOC_GROW_ABSMAX) {
		n = curbufsize + ARRAY_ALLOC_GROW_ABSMAX;
	}
	if (n < ARRAY_ALLOC_MIN) n = ARRAY_ALLOC_MIN;
	if (n < min_newbufsize) {
		n = min_newbufsize;
	}

	return n;
}

void Array::checkShrink()
{
	// FIXME: implement automatic shrinking
	memset(elems+ecount, 0, (acount-ecount) * sizeof (*elems));
}

void Array::freeObj(Object *obj)
{              	
	if (own_objects && obj) {
		obj->done();
		delete obj;
	}
}

void Array::realloc(int n)
{
	if (n == 0) n = 1;	/* alloc'ing 0 bytes not allowed */
	assert((uint)n >= ecount);
	elems = (Object**)::realloc(elems, sizeof (*elems) * n);
	acount = n;
	memset(elems+ecount, 0, (acount-ecount)*sizeof(*elems));
}

/**
 *	Prepare a write access
 *	@param i position of planned write access
 */
void Array::prepareWriteAccess(int i)
{
	if (i < 0) throw MsgException("data structure too big (Array)");
	uint n = calcNewBufferSize(acount, i+1);
	if (n > acount) realloc(n);
}

/*uint Array::count() const
{
	return ecount;
}*/

int Array::compareObjects(const Object *a, const Object *b) const
{
	return autoCompare(a, b);
}

void Array::forceSetByIdx(int i, Object *obj)
{
	// FIXME: sanity check, better idea ?
	if (i < 0) assert(0);
	prepareWriteAccess(i);
	freeObj(elems[i]);
	elems[i] = obj;
	notifyInsertOrSet(obj);
	if ((uint)i>=ecount) ecount = i+1;
}

Object *Array::get(ObjHandle h) const
{
	uint i = handleToNative(h);
	return validHandle(h) ? elems[i] : NULL;
}

uint Array::getObjIdx(ObjHandle h) const
{
	return invObjHandle ? invIdx : handleToNative(h);
}

ObjHandle Array::findByIdx(int i) const
{
	return validHandle(nativeToHandle(i)) ? nativeToHandle(i) : invObjHandle;
}

ObjHandle Array::findFirst() const
{
	return ecount ? nativeToHandle(0) : invObjHandle;
}

ObjHandle Array::findLast() const
{
	return ecount ? nativeToHandle(ecount-1) : invObjHandle;
}

ObjHandle Array::findNext(ObjHandle h) const
{
	if (!validHandle(h)) return findFirst();
	uint i = handleToNative(h);
	return (i<ecount-1) ? nativeToHandle(i+1) : invObjHandle;
}

ObjHandle Array::findPrev(ObjHandle h) const
{
	if (!validHandle(h)) return findLast();
	uint i = handleToNative(h);
	return (i>0) ? nativeToHandle(i-1) : invObjHandle;
}

ObjHandle Array::insert(Object *obj)
{
	prepareWriteAccess(ecount);
	elems[ecount++] = obj;
	notifyInsertOrSet(obj);
	return nativeToHandle(ecount-1);
}

bool Array::del(ObjHandle h)
{
	if (!validHandle(h)) return false;
	freeObj(remove(h));
	return true;
}

bool Array::moveTo(ObjHandle from, ObjHandle to)
{
	if (!validHandle(from)) return false;
	if (!validHandle(to)) return false;
	uint i = handleToNative(from);
	uint t = handleToNative(to);
	if (i == t) return true;
	Object *o = elems[i];
	if (i < t) {
		memmove(elems+i, elems+i+1, sizeof (*elems) * (t - i));
	} else {
		memmove(elems+t+1, elems+t, sizeof (*elems) * (i - t));
	}
	elems[t] = o;
	return true;
}

Object *Array::remove(ObjHandle h)
{
	if (!validHandle(h)) return NULL;
	uint i = handleToNative(h);
	Object *o = elems[i];
	if (i < ecount) memmove(elems+i, elems+i+1, sizeof (*elems) * (ecount - i - 1));
	ecount--;
	checkShrink();
	return o;
}

bool Array::set(ObjHandle h, Object *obj)
{
	if (!validHandle(h)) return false;
	uint i = handleToNative(h);
	freeObj(elems[i]);
	elems[i] = obj;
	notifyInsertOrSet(obj);
	return true;
}

bool Array::swap(ObjHandle h, ObjHandle i)
{
	if (!validHandle(h)) return false;
	if (!validHandle(i)) return false;
	uint H = handleToNative(h);
	uint I = handleToNative(i);
	Object *t = elems[H];
	elems[H] = elems[I];
	elems[I] = t;
	return true;
}

void Array::insertAt(ObjHandle h, Object *obj)
{
	if (!validHandle(h)) {
		insert(obj);
	} else {
		uint i = handleToNative(h);
		if (i < ecount) {
			prepareWriteAccess(ecount);
			memmove(elems+i+1, elems+i, sizeof (*elems) * (ecount - i));
			ecount++;
		} else {
			prepareWriteAccess(i);
			memset(elems+ecount, 0, sizeof (*elems) * (i - ecount));
			ecount = i+1;
		}
		elems[i] = obj;
		notifyInsertOrSet(obj);
	}
}

/*
 *	Stack
 */
Stack::Stack(bool own_objects) : Array(own_objects)
{
}

void	Stack::push(Object *obj)
{
	insert(obj);
}

Object *Stack::pop()
{
	return remove(findLast());
}

ObjectID Stack::getObjectID() const
{
	return OBJID_STACK;
}

/*
 *	SLinkedList
 */
SLinkedList::SLinkedList(bool oo)
{
	own_objects = oo;
	ecount = 0;
	first = last = NULL;
}

SLinkedList::~SLinkedList()
{
	delAll();
}

void SLinkedList::delAll()
{
	SLinkedListNode *n = first, *m;
	while (n) {
		m = n->next;
		freeObj(n->obj);
		deleteNode(n);
		n = m;
	}
	ecount = 0;
	first = last = NULL;
}

SLinkedListNode *SLinkedList::allocNode() const
{
	return new SLinkedListNode;
}

void SLinkedList::deleteNode(SLinkedListNode *node) const
{
	delete node;
}

void SLinkedList::freeObj(Object *obj) const
{
	if (own_objects && obj) {
		obj->done();
		delete obj;
	}
}

SLinkedList *SLinkedList::clone() const
{
	SLinkedList *l = new SLinkedList(own_objects);
	SLinkedListNode *n = first, *m;
	while (n) {
		m = n->next;
		Object *o = m->obj;
		if (own_objects) o = o->clone();
		l->insert(o);
		n = m;
	}
	return l;
}

void SLinkedList::load(ObjectStream &s)
{
	own_objects = true;
	first = last = NULL;

	ecount = 0;
	int ecount;
	GET_INT32D(s, ecount);
	if (ecount) {
		GET_INT32X(s, hom_objid);
		for (int i=0; i < ecount; i++) {
			Object *obj = s.getObjectInternal("element", hom_objid);
			insert(obj);
		}
	} else {
		hom_objid = OBJID_TEMP;
	}
}

ObjectID SLinkedList::getObjectID() const
{
	return OBJID_SLINKED_LIST;
}

void SLinkedList::store(ObjectStream &s) const
{
	PUT_INT32D(s, ecount);
	if (ecount) {
		putIDComment(s, hom_objid);
		PUT_INT32X(s, hom_objid);

		ObjHandle h = findFirst();
		assert(h == invObjHandle || hom_objid != OBJID_TEMP);
		while (h != invObjHandle) {
			Object *o = get(h);

			s.putObject(o, "element", hom_objid);
			h = findNext(h);
		}
	}
}

uint SLinkedList::count() const
{
	return ecount;
}

int SLinkedList::compareObjects(const Object *a, const Object *b) const
{
	return autoCompare(a, b);
}

void SLinkedList::forceSetByIdx(int idx, Object *obj)
{
	// FIXME:
	throw NotImplementedException(HERE);
}

Object *SLinkedList::get(ObjHandle h) const
{
	SLinkedListNode *n = handleToNative(h);
	return validHandle(h) ? n->obj : NULL;
}

uint SLinkedList::getObjIdx(ObjHandle g) const
{
	int i = 0;
	ObjHandle h = findFirst();
	Object *obj = get(g);
	while (h != invObjHandle) {
		if (compareObjects(get(h), obj) == 0) return i;
		i++;
		h = findNext(h);
	}
	return invIdx;
}

ObjHandle SLinkedList::findByIdx(int i) const
{
	ObjHandle h = findFirst();
	while (h != invObjHandle) {
		if (!i--) break;
		h = findNext(h);
	}
	return h;
}

ObjHandle SLinkedList::findFirst() const
{
	return nativeToHandle(first);
}

ObjHandle SLinkedList::findLast() const
{
	return nativeToHandle(last);
}

ObjHandle SLinkedList::findNext(ObjHandle h) const
{
	if (!validHandle(h)) return findFirst();
	SLinkedListNode *n = handleToNative(h);
	return nativeToHandle(n->next);
}

ObjHandle SLinkedList::findPrev(ObjHandle g) const
{
	if (!validHandle(g)) return findLast();
	SLinkedListNode *ng = handleToNative(g);
	if (ng == first) return invObjHandle;
	ObjHandle h = findFirst();
	while (h != invObjHandle) {
		SLinkedListNode *nh = handleToNative(h);
		if (nh->next == ng) return nativeToHandle(nh);
		h = findNext(h);
	}
	return invObjHandle;
}

bool SLinkedList::del(ObjHandle h)
{
	if (!h) return false;
	freeObj(remove(h));
	return true;
}

ObjHandle SLinkedList::insert(Object *obj)
{
	SLinkedListNode *n = allocNode();
	n->obj = obj;
	n->next = NULL;
	if (last) {
		last->next = n;
		last = n;
	} else {
		first = last = n;
	}
	ecount++;
	notifyInsertOrSet(obj);
	return nativeToHandle(n);
}

Object *SLinkedList::remove(ObjHandle h)
{
	if (!validHandle(h)) return NULL;
	SLinkedListNode *n = handleToNative(h);
	Object *o = n->obj;
	if (n == first) {
		if (n == last) {
			first = NULL;
			last = NULL;
		} else {
			first = n->next;
		}
	} else {
		SLinkedListNode *p = handleToNative(findPrev(h));
		p->next = n->next;
		if (n == last) last = p;
	}
	deleteNode(n);
	ecount--;
	return o;
}

void SLinkedList::insertAt(ObjHandle h, Object *obj)
{
	// FIXME: nyi
	throw NotImplementedException(HERE);
#if 0
	uint i = ((uint)h)-1;	// WRONG!
	if (i>ecount-1) {
		insert(obj);
		return;
	}
	ObjHandle q = i ? findByIdx(i-1) : invObjHandle;
	SLinkedListNode *n;
	if (q != invObjHandle) {
		n = handleToNative(q);
	} else if (i==0) {
		n = NULL;
	} else {
		insert(obj);
		return;
	}		
	SLinkedListNode *m = allocNode();
	m->obj = obj;
	if (n) {
		m->next = n->next;
		n->next = m;
		if (n == last) last = m;
	} else {
		m->next = first;
		first = m;
		if (!last) last = m;
	}
	ecount++;
	notifyInsertOrSet(obj);
#endif
}

bool SLinkedList::moveTo(ObjHandle from, ObjHandle to)
{
	// FIXME:
	throw NotImplementedException(HERE);
}

bool SLinkedList::set(ObjHandle h, Object *obj)
{
	SLinkedListNode *n = handleToNative(h);
	if (!n) return false;
	freeObj(n->obj);
	n->obj = obj;
	return true;
}

bool SLinkedList::swap(ObjHandle h, ObjHandle i)
{
	SLinkedListNode *H = handleToNative(h);
	SLinkedListNode *I = handleToNative(i);
	Object *t = H->obj;
	H->obj = I->obj;
	I->obj = t;
	return true;
}

bool SLinkedList::validHandle(ObjHandle h) const
{
	return h != invObjHandle;
}

SLinkedListNode* SLinkedList::handleToNative(ObjHandle h) const
{
	return (SLinkedListNode*)h;
}

ObjHandle SLinkedList::nativeToHandle(SLinkedListNode *n) const
{
	return (ObjHandle)n;
}

/*
 *	DLinkedList
 */
DLinkedList::DLinkedList(bool oo)
{
	own_objects = oo;
	ecount = 0;
	first = last = NULL;
}

DLinkedList::~DLinkedList()
{
	delAll();
}

void DLinkedList::delAll()
{
	DLinkedListNode *n = first, *m;
	while (n) {
		m = n->next;
		freeObj(n->obj);
		deleteNode(n);
		n = m;
	}
	ecount = 0;
	first = last = NULL;
}

DLinkedListNode *DLinkedList::allocNode() const
{
	return new DLinkedListNode;
}

void DLinkedList::deleteNode(DLinkedListNode *node) const
{
	delete node;
}

void DLinkedList::freeObj(Object *obj) const
{
	if (own_objects && obj) {
		obj->done();
		delete obj;
	}
}

DLinkedList *DLinkedList::clone() const
{
	DLinkedList *l = new DLinkedList(own_objects);
	DLinkedListNode *n = first, *m;
	while (n) {
		m = n->next;
		Object *o = m->obj;
		if (own_objects) o = o->clone();
		l->insert(o);
		n = m;
	}
	return l;
}

void DLinkedList::load(ObjectStream &s)
{
	own_objects = true;
	first = last = NULL;

	ecount = 0;
	int ecount;
	GET_INT32D(s, ecount);
	if (ecount) {
		GET_INT32X(s, hom_objid);

		for (int i=0; i<ecount; i++) {
			Object *obj = s.getObjectInternal("element", hom_objid);
			insert(obj);
		}
	} else {
		hom_objid = OBJID_TEMP;
	}
}

ObjectID DLinkedList::getObjectID() const
{
	return OBJID_DLINKED_LIST;
}

void DLinkedList::store(ObjectStream &s) const
{
	PUT_INT32D(s, ecount);
	if (ecount) {
		putIDComment(s, hom_objid);
		PUT_INT32X(s, hom_objid);

		ObjHandle h = findFirst();
		assert(h == invObjHandle || hom_objid != OBJID_TEMP);
		while (h != invObjHandle) {
			Object *o = get(h);

			s.putObject(o, "element", hom_objid);
			h = findNext(h);
		}
	}
}

uint DLinkedList::count() const
{
	return ecount;
}

int DLinkedList::compareObjects(const Object *a, const Object *b) const
{
	return autoCompare(a, b);
}

void DLinkedList::forceSetByIdx(int idx, Object *obj)
{
	// FIXME:
	throw NotImplementedException(HERE);
}

Object *DLinkedList::get(ObjHandle h) const
{
	DLinkedListNode *n = handleToNative(h);
	return validHandle(h) ? n->obj : NULL;
}

uint DLinkedList::getObjIdx(ObjHandle g) const
{
	int i = 0;
	ObjHandle h = findFirst();
	Object *obj = get(g);
	while (h != invObjHandle) {
		if (compareObjects(get(h), obj) == 0) return i;
		i++;
		h = findNext(h);
	}
	return invIdx;
}

ObjHandle DLinkedList::findByIdx(int i) const
{
	ObjHandle h = findFirst();
	while (h != invObjHandle) {
		if (!i--) break;
		h = findNext(h);
	}
	return h;
}

ObjHandle DLinkedList::findFirst() const
{
	return nativeToHandle(first);
}

ObjHandle DLinkedList::findLast() const
{
	return nativeToHandle(last);
}

ObjHandle DLinkedList::findNext(ObjHandle h) const
{
	if (!validHandle(h)) return findFirst();
	DLinkedListNode *n = handleToNative(h);
	return nativeToHandle(n->next);
}

ObjHandle DLinkedList::findPrev(ObjHandle h) const
{
	if (!validHandle(h)) return findLast();
	DLinkedListNode *n = handleToNative(h);
	return nativeToHandle(n->prev);
}

bool DLinkedList::del(ObjHandle h)
{
	if (!h) return false;
	freeObj(remove(h));
	return true;
}

ObjHandle DLinkedList::insert(Object *obj)
{
	DLinkedListNode *n = allocNode();
	n->obj = obj;
	n->next = NULL;
	if (last) {
		last->next = n;
		n->prev = last;
		last = n;
	} else {
		first = last = n;
	}
	ecount++;
	notifyInsertOrSet(obj);
	return nativeToHandle(n);
}

Object *DLinkedList::remove(ObjHandle h)
{
	if (!validHandle(h)) return NULL;
	DLinkedListNode *n = handleToNative(h);
	Object *o = n->obj;
	if (n == first) {
		if (n == last) {
			first = NULL;
			last = NULL;
		} else {
			first = n->next;
			first->prev = NULL;
		}
	} else if (n == last) {
		last = n->prev;
		last->next = NULL;
	} else {
		n->prev->next = n->next;
		n->next->prev = n->prev;
	}
	deleteNode(n);
	ecount--;
	return o;
}

void DLinkedList::insertAt(ObjHandle h, Object *obj)
{
	// FIXME: nyi
	throw NotImplementedException(HERE);
}

bool DLinkedList::moveTo(ObjHandle from, ObjHandle to)
{
	// FIXME:
	throw NotImplementedException(HERE);
}

bool DLinkedList::set(ObjHandle h, Object *obj)
{
	DLinkedListNode *n = handleToNative(h);
	if (!n) return false;
	freeObj(n->obj);
	n->obj = obj;
	return true;
}

bool DLinkedList::swap(ObjHandle h, ObjHandle i)
{
	DLinkedListNode *H = handleToNative(h);
	DLinkedListNode *I = handleToNative(i);
	Object *t = H->obj;
	H->obj = I->obj;
	I->obj = t;
	return true;
}

bool DLinkedList::validHandle(ObjHandle h) const
{
	return h != invObjHandle;
}

DLinkedListNode* DLinkedList::handleToNative(ObjHandle h) const
{
	return (DLinkedListNode*)h;
}

ObjHandle DLinkedList::nativeToHandle(DLinkedListNode *n) const
{
	return (ObjHandle)n;
}

/*
 *	Queue
 */
Queue::Queue(bool own_objects) : SLinkedList(own_objects)
{
}


ObjectID Queue::getObjectID() const
{
	return OBJID_QUEUE;
}

/*
 *	BinaryTree
 */
BinaryTree::BinaryTree(bool oo, Comparator comp)
{
	root = NULL;
	own_objects = oo;
	compare = comp;
	ecount = 0;
}

BinaryTree::~BinaryTree()
{
	delAll();
}

void BinaryTree::delAll()
{
	freeAll(root);
	root = NULL;
	ecount = 0;
}

BinTreeNode *BinaryTree::allocNode() const
{
	return new BinTreeNode;
}

void BinaryTree::deleteNode(BinTreeNode *node) const
{
	delete node;
}

BinTreeNode **BinaryTree::findNodePtr(BinTreeNode **nodeptr, const Object *obj) const
{
	BinTreeNode **x = nodeptr;
	while (x) {
		int c = compareObjects((*x)->key, obj);
		if (c < 0) {
			x = &(*x)->right;
		} else if (c > 0) {
			x = &(*x)->left;
		} else break;
	}
	return x;
}

BinTreeNode *BinaryTree::findNode(BinTreeNode *node, const Object *obj) const
{
	while (node) {
		int c = compareObjects(node->key, obj);
		if (c < 0) {
			node = node->right;
		} else if (c > 0) {
			node = node->left;
		} else break;
	}
	return node;
}

BinTreeNode *BinaryTree::findNodeG(BinTreeNode *node, const Object *obj) const
{
	if (!node) return NULL;
	BinTreeNode *lastGreater = NULL;
	while (true) {
		int c = compareObjects(obj, node->key);
		if (c < 0) {
			if (!node->left) return node;
			lastGreater = node;
			node = node->left;
		} else {
			if (!node->right) return lastGreater;
			node = node->right;
		}
	}
}

BinTreeNode *BinaryTree::findNodeGE(BinTreeNode *node, const Object *obj) const
{
	if (!node) return NULL;
	BinTreeNode *lastGreater = NULL;
	while (true) {
		int c = compareObjects(obj, node->key);
		if (c < 0) {
			if (!node->left) return node;
			lastGreater = node;
			node = node->left;
		} else if (c > 0) {
			if (!node->right) return lastGreater;
			node = node->right;
		} else {
			return node;
		}
	}
}

BinTreeNode *BinaryTree::findNodeL(BinTreeNode *node, const Object *obj) const
{
	if (!node) return NULL;
	BinTreeNode *lastLower = NULL;
	while (true) {
		int c = compareObjects(obj, node->key);
		if (c <= 0) {
			if (!node->left) return lastLower;
			node = node->left;
		} else {
			if (!node->right) return node;
			lastLower = node;
			node = node->right;
		}
	}
}

BinTreeNode *BinaryTree::findNodeLE(BinTreeNode *node, const Object *obj) const
{
	if (!node) return NULL;
	BinTreeNode *lastLower = NULL;
	while (true) {
		int c = compareObjects(obj, node->key);
		if (c < 0) {
			if (!node->left) return lastLower;
			node = node->left;
		} else if (c > 0) {
			if (!node->right) return node;
			lastLower = node;
			node = node->right;
		} else {
			return node;
		}
	}
}

void BinaryTree::freeAll(BinTreeNode *n)
{
	if (!n) return;
	freeAll(n->left);
	freeObj(n->key);
	freeAll(n->right);
	deleteNode(n);
}

void BinaryTree::freeObj(Object *obj) const
{
	if (own_objects && obj) {
		obj->done();
		delete obj;
	}
}

ObjHandle BinaryTree::findByIdxR(BinTreeNode *n, int &i) const
{
	if (!n) return invObjHandle;
	ObjHandle h;
	if ((h = findByIdxR(n->left, i))) return h;
	if (i == 0) return nativeToHandle(n);
	i--;
	if ((h = findByIdxR(n->right, i))) return h;
	return invObjHandle;
}

BinTreeNode *BinaryTree::getLeftmost(BinTreeNode *node) const
{
	if (node) while (node->left) node = node->left;
	return node;
}

BinTreeNode *BinaryTree::getRightmost(BinTreeNode *node) const
{
	if (node) while (node->right) node = node->right;
	return node;
}

BinTreeNode **BinaryTree::getLeftmostPtr(BinTreeNode **p) const
{
	if (*p) while ((*p)->left) p = &(*p)->left;
	return p;
}

BinTreeNode **BinaryTree::getRightmostPtr(BinTreeNode **p) const
{
	if (*p) while ((*p)->right) p = &(*p)->right;
	return p;
}

void BinaryTree::cloneR(BinTreeNode *node)
{
	if (!node) return;
	Object *o = own_objects ? node->key->clone() : node->key;
	// SB: nicht gut: (unnoetige compares)
	insert(o);

	cloneR(node->left);
	cloneR(node->right);
}

BinaryTree *BinaryTree::clone() const
{
	BinaryTree *c = new BinaryTree(own_objects, compare);
	c->cloneR(root);
	return c;
}

void BinaryTree::loadR(ObjectStream &s, BinTreeNode **n, int l, int r)
{
	if (l > r) {
		*n = NULL;
		return;
	}
	*n = allocNode();
	uint m = (l+r)/2;
	loadR(s, &(*n)->left, l, m-1);

	(*n)->key = s.getObjectInternal("element", hom_objid);

	loadR(s, &(*n)->right, m+1, r);
}

void BinaryTree::load(ObjectStream &s)
{
	const void *m = getAtomValue(GETX_INT32(s, "comparator"));
	if (!m) throw MsgException("BinaryTree::load(): invalid comparator!");
	compare = (Comparator)m;

	GET_INT32D(s, ecount);
	root = NULL;
	own_objects = true;
	if (ecount) {
		GET_INT32X(s, hom_objid);
		loadR(s, &root, 0, ecount-1);
	} else {
		hom_objid = OBJID_TEMP;
	}
}

ObjectID BinaryTree::getObjectID() const
{
	return OBJID_BINARY_TREE;
}

void BinaryTree::storeR(ObjectStream &s, BinTreeNode *n) const
{
	if (!n) return;
	storeR(s, n->left);

	s.putObject(n->key, "element", hom_objid);

	storeR(s, n->right);
}

void	BinaryTree::store(ObjectStream &s) const
{
	int aId = getAtomId((void*)compare);
	if (!aId) throw MsgException("BinaryTree::store() : comparator not registered!");
	putIDComment(s, aId);
	PUTX_INT32X(s, aId, "comparator");

	PUT_INT32D(s, ecount);
	if (ecount) {
		assert(hom_objid != OBJID_TEMP);
		putIDComment(s, hom_objid);
		PUT_INT32X(s, hom_objid);
		storeR(s, root);
	}
}

uint BinaryTree::count() const
{
	return ecount;
}

int BinaryTree::compareObjects(const Object *a, const Object *b) const
{
	return compare(a, b);
}

ObjHandle BinaryTree::find(const Object *key) const
{
	return findNode(root, key);
}

ObjHandle BinaryTree::findG(const Object *key) const
{
	return findNodeG(root, key);
}

ObjHandle BinaryTree::findGE(const Object *key) const
{
	return findNodeGE(root, key);
}

ObjHandle BinaryTree::findL(const Object *key) const
{
	return findNodeL(root, key);
}

ObjHandle BinaryTree::findLE(const Object *key) const
{
	return findNodeLE(root, key);
}

Object *BinaryTree::get(ObjHandle h) const
{
	BinTreeNode *n = handleToNative(h);
	return validHandle(h) ? n->key : NULL;
}

uint BinaryTree::getObjIdx(ObjHandle h) const
{
	// FIXME: implement it
	throw NotImplementedException(HERE);
}

ObjHandle BinaryTree::findByIdx(int i) const
{
	return findByIdxR(root, i);
}

ObjHandle BinaryTree::findFirst() const
{
	return nativeToHandle(getLeftmost(root));
}

ObjHandle BinaryTree::findLast() const
{
	return nativeToHandle(getRightmost(root));
}

ObjHandle BinaryTree::findNext(ObjHandle h) const
{
	if (!validHandle(h)) return findFirst();
	BinTreeNode *n = handleToNative(h);
	if (n->right) return nativeToHandle(getLeftmost(n->right));
	BinTreeNode *x = root, *result = NULL;
	while (x) {
		int c = compareObjects(x->key, n->key);
		if (c > 0) {
			result = x;
			x = x->left;
		} else {
			x = x->right;
		}
	}
	return nativeToHandle(result);
}

ObjHandle BinaryTree::findPrev(ObjHandle h) const
{
	if (!validHandle(h)) return findLast();
	BinTreeNode *n = handleToNative(h);
	if (n->left) return nativeToHandle(getRightmost(n->left));
	BinTreeNode *x = root, *result = NULL;
	while (x) {
		int c = compareObjects(x->key, n->key);
		if (c < 0) {
			result = x;
			x = x->right;
		} else {
			x = x->left;
		}
	}
	return nativeToHandle(result);
}

bool BinaryTree::del(ObjHandle h)
{
	if (!validHandle(h)) return false;
	BinTreeNode *n = handleToNative(h);
	Object *obj = n->key;
	bool r = remove(h);
	freeObj(obj);
	return r;
}

// SB: ich haette gerne noch ein findOrInsert (besserer Name noetig),
//     das entweder einfuegt oder - wenns das schon gibt -
//     das ObjHandle zurueckgibt (bzw immer das objHandle zurueckgibt)
// SW: interface + naive implementierung in Container sind da
ObjHandle BinaryTree::insert(Object *obj)
{
	return insertR(root, obj);
}

ObjHandle BinaryTree::insertR(BinTreeNode *&node, Object *obj)
{
	if (!node) {
		node = allocNode();
		node->key = obj;
		node->left = NULL;
		node->right = NULL;
		ecount++;
		notifyInsertOrSet(obj);
		return nativeToHandle(node);
	}
	int c = compareObjects(obj, node->key);
	if (c > 0) {
		return insertR(node->right, obj);
	} else if (c < 0) {
		return insertR(node->left, obj);
	} else return invObjHandle;
}

Object *BinaryTree::remove(ObjHandle h)
{
	if (!validHandle(h)) return NULL;
	/* n is the node, whose key has to be removed */
	BinTreeNode *n = handleToNative(h);
	/* d is node that is to be removed - not necessarily n. */
	BinTreeNode *d;

	Object *o = n->key;
	if (n->left && n->right) {
		/* p is pointer to left/right inside Parent(d) with: *p = d */
		BinTreeNode **p = getLeftmostPtr(&n->right);
		d = *p;
		*p = (*p)->right;
	} else if (n->left || n->right) {
		d = n->left ? n->left : n->right;
		n->left = d->left;
		n->right = d->right;
	} else {
		// SB: hier wuerde ein remove(Object *o) mit integriertem find()
		//     auch (etwas) schneller sein, da man kein findNodePtr mehr braucht
		// SW: interface ist da: remove(Object *o)
		BinTreeNode **p = findNodePtr(&root, n->key);
		d = *p;
		*p = NULL;
	}

	n->key = d->key;
	deleteNode(d);
	ecount--;
	return o;
}

void BinaryTree::setNodeIdentity(BinTreeNode *node, BinTreeNode *newident)
{
	node->key = newident->key;
}

/*
 *	AVLTree
 */
AVLTree::AVLTree(bool aOwnObjects, Comparator aComparator)
 : BinaryTree(aOwnObjects, aComparator)
{

}

void debugOutNode(FILE *f, BinTreeNode *n, BinTreeNode *p)
{
	if (n) {
		char b[1024];
		ht_snprintf(b, sizeof b, "node: { title: \"%y\" label: \"%y (%d)\" }\n", n->key, n->key, n->unbalance);
		fputs(b, f);
		if (p) {
			ht_snprintf(b, sizeof b, "edge: { sourcename: \"%y\" targetname: \"%y\" }\n", p->key, n->key);
			fputs(b, f);
		}
		debugOutNode(f, n->right, n);
		debugOutNode(f, n->left, n);
	}
}

void AVLTree::debugOut()
{
	FILE *f = fopen("test.vcg", "wb");
	fputs("graph: {\nlayoutalgorithm: tree\n", f);
	debugOutNode(f, root, NULL);
	fputs("}\n", f);
	fclose(f);
}

bool AVLTree__expensiveCheck(BinTreeNode *n, int &height)
{
	if (n) {
		int left, right;
		if (!AVLTree__expensiveCheck(n->left, left)) return false;
		if (!AVLTree__expensiveCheck(n->right, right)) return false;
		height = MAX(left, right)+1;
		if (left < right) {
			return n->unbalance == 1;
		} else if (left > right) {
			return n->unbalance == -1;
		} else {
			return n->unbalance == 0;
		}
	} else {
		height = 0;
		return true;
	}
}

bool AVLTree::expensiveCheck() const
{
	int dummy;
	return AVLTree__expensiveCheck(root, dummy);
}

void AVLTree::cloneR(BinTreeNode *node)
{
	if (!node) return;
	Object *o = own_objects ? node->key->clone() : node->key;
	// SB: nicht gut: (unnoetige compares)
	insert(o);

	cloneR(node->left);
	cloneR(node->right);
}

AVLTree *AVLTree::clone() const
{
	AVLTree *c = new AVLTree(own_objects, compare);
	c->cloneR(root);
	return c;
}

int AVLTree::loadR(ObjectStream &s, BinTreeNode *&n, int l, int r)
{
	if (l > r) {
		n = NULL;
		return 0;
	}
	n = allocNode();
	uint m = (l+r)/2;

	int L = loadR(s, n->left, l, m-1);

	n->key = s.getObject("element", hom_objid);

	int R = loadR(s, n->right, m+1, r);

	if (L < R) {
		n->unbalance = +1;
	} else if (L > R) {
		n->unbalance = -1;
	} else {
		n->unbalance = 0;
	}
	return MAX(L, R)+1;
}

void AVLTree::load(ObjectStream &s)
{
	const void *m = getAtomValue(GETX_INT32X(s, "comparator"));
	if (!m) throw MsgException("AVLTree::load(): invalid 'comparator'!");
	compare = (Comparator)m;

	GET_INT32D(s, ecount);
	root = NULL;
	own_objects = true;
	if (ecount) {
		GET_INT32X(s, hom_objid);
		loadR(s, root, 0, ecount-1);
	} else {
		hom_objid = OBJID_TEMP;
	}
}

ObjectID AVLTree::getObjectID() const
{
	return OBJID_AVL_TREE;
}

ObjHandle AVLTree::insert(Object *obj)
{
	/* t will point to the node where rebalancing may be necessary */
	BinTreeNode **t = &root;
	/* *pp will walk through the tree */
	BinTreeNode **pp = &root;
	// Search
	while (*pp) {
		int c = compareObjects(obj, (*pp)->key);
		if (c < 0) {
			pp = &(*pp)->left;
		} else if (c > 0) {
			pp = &(*pp)->right;
		} else {
			// element found
			return invObjHandle;
		}
		if (*pp && (*pp)->unbalance) {
			t = pp;
		}
	}

	/* s points to the node where rebalancing may be necessary */
	BinTreeNode *s = *t;

	// Insert
	*pp = allocNode();
	BinTreeNode *retval = *pp;
	retval->key = obj;
	retval->left = retval->right = NULL;
	retval->unbalance = 0;
	ecount++;
	notifyInsertOrSet(obj);
	if (!s) return nativeToHandle(retval);

	// Rebalance
	int a;
	BinTreeNode *r;
	BinTreeNode *p;
	if (compareObjects(obj, s->key) < 0) {
		a = -1;
		r = p = s->left;
	} else {
		a = 1;
		r = p = s->right;
	}
	while (p != retval) {
		if (compareObjects(obj, p->key) < 0) {
			p->unbalance = -1;
			p = p->left;
		} else {
			p->unbalance = 1;
			p = p->right;
		}
	}
	if (!s->unbalance) {
		// tree was balanced before insertion
		s->unbalance = a;
	} else if (s->unbalance == -a) {
		// tree has become more balanced
		s->unbalance = 0;
	} else {
		// tree is out of balance
		if (r->unbalance == a) {
			// single rotation
			p = r;
			if (a < 0) {
				s->left = r->right;
				r->right = s;
			} else {
				s->right = r->left;
				r->left = s;
			}
			s->unbalance = r->unbalance = 0;
		} else {
			// double rotation
			if (a < 0) {
				p = r->right;
				r->right = p->left;
				p->left = r;
				s->left = p->right;
				p->right = s;
			} else {
				p = r->left;
				r->left = p->right;
				p->right = r;
				s->right = p->left;
				p->left = s;
			}
			s->unbalance = (p->unbalance == a) ? -a : 0;
			r->unbalance = (p->unbalance == -a) ? a : 0;
			p->unbalance = 0;
		}
		// finalization
		*t = p;
	}
	assert(root);
	return nativeToHandle(retval);
}

BinTreeNode *AVLTree::removeR(Object *key, BinTreeNode *&node, int &change, int cmp)
{
	if (node == NULL) {
		change = 0;
		return NULL;
	}

	BinTreeNode *found = NULL;
	int decrease = 0;

	int result;
	if (!cmp) {
		result = compareObjects(key, node->key);
		if (result < 0) {
			result = -1;
		} else if (result > 0) {
			result = 1;
		}
	} else if (cmp < 0) {
		result = (node->left == NULL) ? 0 : -1;
	} else {
		result = (node->right == NULL) ? 0 : 1;
	}

	if (result) {
		found = removeR(key, (result < 0) ? node->left : node->right, change, cmp);
		if (!found) return NULL;
		decrease = result * change;
	} else {
		found = node;

		/*
		 *	Same logic as in BinaryTree::remove()
		 */
		if (!node->left && !node->right) {
			node = NULL;
			change = 1;
			return found;
		} else if (!node->left || !node->right) {
			node = node->right ? node->right : node->left;
			change = 1;
			return found;
		} else {
			BinTreeNode *n = removeR(key, node->right, decrease, -1);
			setNodeIdentity(node, n);
			found = n;
		}
	}

	node->unbalance -= decrease;

	if (decrease) {
		if (node->unbalance) {
			change = 0;
			int a;
			BinTreeNode *r = NULL;
			if (node->unbalance < -1) {
				a = -1;
				r = node->left;
			} else if (node->unbalance > 1) {
				a = 1;
				r = node->right;
			} else {
				a = 0;
			}
			if (a) {
				/*
				 *	If r->unbalance == 0 do also a single rotation.
				 *	This case cant occure with insert operations.
				 */
				if (r->unbalance == -a) {
					/*
					 *	double rotation.
					 *	See insert
					 */
					BinTreeNode *p;
					if (a > 0) {
						p = r->left;
						r->left = p->right;
						p->right = r;
						node->right = p->left;
						p->left = node;
					} else {
						p = r->right;
						r->right = p->left;
						p->left = r;
						node->left = p->right;
						p->right = node;
					}
					node->unbalance = (p->unbalance == a) ? -a: 0;
					r->unbalance = (p->unbalance == -a) ? a: 0;
					p->unbalance = 0;
					node = p;
					change = 1;
				} else {
					/*
					 *	single rotation
					 *	Height of tree changes if r is/was unbalanced
					 */
					change = r->unbalance?1:0;
					if (a > 0) {
						node->right = r->left;
						r->left = node;
						r->unbalance--;
					} else {
						node->left = r->right;
						r->right = node;
						r->unbalance++;
					}
					node->unbalance = - r->unbalance;
					node = r;
				}
			}
		} else {
			/*
			 *	Tree has become more balanced
			 */
			change = 1;
		}
	} else {
		change = 0;
	}

	return found;
}

Object *AVLTree::remove(ObjHandle h)
{
	if (!validHandle(h)) return NULL;
	BinTreeNode *n = handleToNative(h);
	Object *o = n->key;
	int change;
	BinTreeNode *node = removeR(n->key, root, change, 0);
	if (node) {
		deleteNode(node);
		ecount--;
		return o;
	}
	return NULL;
}

/**
 *	A MRU Cache
 */
MRUCache::MRUCache(bool own_objects, Comparator comparator)
: AVLTree(own_objects, comparator)
{
	mostRU = leastRU = NULL;
}

MRUCacheNode *MRUCache::allocNode() const
{
	MRUCacheNode *a = new MRUCacheNode();
//	((MRUCacheNode*)a)->lessRU = (MRUCacheNode*)0xfffff789;
//	((MRUCacheNode*)a)->moreRU = (MRUCacheNode*)0xfffff888;
	return a;
}

MRUCache *MRUCache::clone() const
{
	throw NotImplementedException(HERE);
}

void MRUCache::delAll()
{
	AVLTree::delAll();
	mostRU = leastRU = NULL;
}

void MRUCache::deleteNode(BinTreeNode *node) const
{
//	((MRUCacheNode*)node)->lessRU = (MRUCacheNode*)0xdeadf0cc;
//	((MRUCacheNode*)node)->moreRU = (MRUCacheNode*)0xdeadf0c2;
	delete node;
}

ObjHandle MRUCache::insert(Object *obj)
{
//	checkList();
	uint count0 = count();
	ObjHandle h = AVLTree::insert(obj);
//	DPRINTF("mru_insert: %08x\n", (int)h);
	if (count0 != count()) {
		// make mostRU in MRU list
		MRUCacheNode *n = handleToNative(h);
		n->moreRU = NULL;
		n->lessRU = mostRU;
		if (mostRU == NULL) {
			mostRU = leastRU = n;
		} else {
			mostRU->moreRU = n;
			mostRU = n;
		}
//		checkList();
	}
	return h;
}

Object *MRUCache::remove(ObjHandle h)
{
/*	static int debug_rc = 0;
	++debug_rc;
	DPRINTF("mru_remove(%d): %08x\n", debug_rc, (int)h);
	if (debug_rc == 19) {
		int sdf=32;
	}
	checkList();*/
	if (h != invObjHandle) {
		MRUCacheNode *n = handleToNative(h);
		// remove from MRU list
		if (n == mostRU) {
			if (n == leastRU) {
//				DPRINTF("   --> removing leastRU entry --> empty list\n");
				mostRU = NULL;
				leastRU = NULL;
			} else {
//				DPRINTF("   --> removing mostRU entry\n");
				mostRU = n->lessRU;
				mostRU->moreRU = NULL;
			}
		} else if (n == leastRU) {
//			DPRINTF("   --> removing leastRU entry\n");
//			DPRINTF("       mostRU was: %08x, leastRU was: %08x\n", mostRU, leastRU);
			leastRU = n->moreRU;
			leastRU->lessRU = NULL;
//			DPRINTF("       mostRU is now: %08x, leastRU is now: %08x\n", mostRU, leastRU);
//			DPRINTF("       mostRU->moreRU: %08x\n", mostRU->moreRU);
		} else {
//			DPRINTF("   --> removing entry\n");
			n->moreRU->lessRU = n->lessRU;
			n->lessRU->moreRU = n->moreRU;
		}
	}
//	checkList();
	Object *o = AVLTree::remove(h);
	if ((h != invObjHandle) && (o == NULL)) assert(0);
//	checkList();
	return o;
}

void MRUCache::setNodeIdentity(BinTreeNode *node, BinTreeNode *newident)
{
	MRUCacheNode *_node = (MRUCacheNode *)node;
	MRUCacheNode *_newident = (MRUCacheNode *)newident;
	_node->key = _newident->key;
	_node->moreRU = _newident->moreRU;
	_node->lessRU = _newident->lessRU;
	if (_newident->moreRU) {
		_newident->moreRU->lessRU = _node;
	} else {
		mostRU = _node;
	}
	if (_newident->lessRU) {
		_newident->lessRU->moreRU = _node;
	} else {
		leastRU = _node;
	}
}

void MRUCache::store(ObjectStream &s) const
{
	throw NotImplementedException(HERE);
}

void MRUCache::propagate(ObjHandle h)
{
	if (h != invObjHandle) {
		MRUCacheNode *n = handleToNative(h);
		// remove from MRU list
		if (n == mostRU) {
			if (n == leastRU) {
				mostRU = NULL;
				leastRU = NULL;
			} else {
				mostRU = n->lessRU;
				mostRU->moreRU = NULL;
			}
		} else if (n == leastRU) {
			leastRU = n->moreRU;
			leastRU->lessRU = NULL;
		} else {
			n->moreRU->lessRU = n->lessRU;
			n->lessRU->moreRU = n->moreRU;
		}
//		checkList();
		// make mostRU in MRU list
		n->moreRU = NULL;
		n->lessRU = mostRU;
		if (mostRU == NULL) {
			mostRU = leastRU = n;
		} else {
			mostRU->moreRU = n;
			mostRU = n;
		}
//		checkList();
	}
}

ObjHandle MRUCache::getLRU()
{
	return nativeToHandle(leastRU);
}

void MRUCache::checkList() const
{
	assert(expensiveCheck());
	DPRINTF("======================================================\n");
	if (!mostRU || !leastRU) {
		DPRINTF("empty\n");
		assert(!mostRU);
		assert(!leastRU);
		return;
	}
	DPRINTF("list: \n");
	MRUCacheNode *i = mostRU;
	while (i) {
		DPRINTF(" moreRU   | this   | lessRU   ");
		if (i->lessRU) DPRINTF("||");
		i = i->lessRU;
	}
	DPRINTF("\n");
	i = mostRU;
	while (i) {
#if 0
		DPRINTF("%08x|%08x|%08x", i->moreRU, i, i->lessRU);
		if (i->lessRU) DPRINTF("||");
#else
		DPRINTF("%08x\n", i->moreRU);
		DPRINTF("%08x\n", i);
		DPRINTF("%08x\n", i->lessRU);
		if (i->lessRU) DPRINTF("       ||\n");
#endif
		i = i->lessRU;
	}
	DPRINTF("\n");
	uint c1 = 1;
	i = mostRU;
	while (i->lessRU) {
		if (i->lessRU->moreRU != i) {
			DPRINTF("ecount =%08x\n", ecount);
			DPRINTF("mostRU: %08x, leastRU: %08x\n", mostRU, leastRU);
			DPRINTF("i: %08x, i->lessRU: %08x, i->np: %08x\n", i, i->lessRU, i->lessRU->moreRU);
		}
		assert(i->lessRU->moreRU == i);
		assert(((i == mostRU) && (i->moreRU == NULL)) || ((i != mostRU) && (i->moreRU != NULL)));
		i = i->lessRU;
		c1++;
		assert(c1<=ecount);
	}
	assert(i == leastRU);
	uint c2 = 1;
	i = leastRU;
	while (i->moreRU) {
		assert(i->moreRU->lessRU == i);
		assert(((i == leastRU) && (i->lessRU == NULL)) || ((i != leastRU) && (i->lessRU != NULL)));
		i = i->moreRU;
		c2++;
		assert(c2<=ecount);
	}
	assert(i == mostRU);
//	assert(c1 == ecount);
//	assert(c2 == ecount);
}

/*
 *	Set
 */
Set::Set(bool oo)
: AVLTree(oo)
{
}

ObjectID Set::getObjectID() const
{
	return OBJID_SET;
}

void Set::intersectWith(Set *b)
{
	foreach(Object, elem, *this,
		if (!b->contains(elem)) delObj(elem);
	);
}

void Set::unionWith(Set *b)
{
	foreach(Object, elem, *b,
		if (!contains(elem)) insert(own_objects ? elem->clone() : elem);
	);
}

/*
 *
 */
KeyValue::~KeyValue()
{
	if (mKey) mKey->done();
	delete mKey;
	if (mValue) mValue->done();
	delete mValue;
}

KeyValue *KeyValue::clone() const
{
	return new KeyValue(mKey->clone(), mValue->clone());
}

int KeyValue::compareTo(const Object *obj) const
{
	return mKey->compareTo(((KeyValue*)obj)->mKey);
}

int KeyValue::toString(char *buf, int buflen) const
{
	return ht_snprintf(buf, buflen, "[Key: %y; Value: %y]", mKey, mValue);
}

void KeyValue::load(ObjectStream &s)
{
	GET_OBJECT(s, mKey);
	GET_OBJECT(s, mValue);
}

ObjectID KeyValue::getObjectID() const
{
	return OBJID_KEYVALUE;
}

void KeyValue::store(ObjectStream &s) const
{
	PUT_OBJECT(s, mKey);
	PUT_OBJECT(s, mValue);
}

/*
 *	SInt
 */
SInt *SInt::clone() const
{
	return new SInt(value);
}

int SInt::compareTo(const Object *obj) const
{
	SInt *s = (SInt*)obj;
	return value - s->value;
}

int SInt::toString(char *buf, int buflen) const
{
	return ht_snprintf(buf, buflen, "%d", value);
}

void SInt::load(ObjectStream &s)
{
	GET_INT32D(s, value);
}

ObjectID SInt::getObjectID() const
{
	return OBJID_SINT;
}

void SInt::store(ObjectStream &s) const
{
	PUT_INT32D(s, value);
}

/*
 *	A signed Integer (64-bit)
 */
SInt64 *SInt64::clone() const
{
	return new SInt64(value);
}

int SInt64::compareTo(const Object *obj) const
{
	SInt64 *u = (SInt64*)obj;

	if (value < u->value) {
		return -1;
	} else if (value > u->value) {
		return 1;
	} else {
		return 0;
	}
}

int SInt64::toString(char *buf, int buflen) const
{
	return ht_snprintf(buf, buflen, "%qd", value);
}

void SInt64::load(ObjectStream &s)
{
	GET_INT64D(s, value);
}

ObjectID SInt64::getObjectID() const
{
	return OBJID_SINT64;
}

void SInt64::store(ObjectStream &s) const
{
	PUT_INT64D(s, value);
}

/*
 *	UInt
 */
UInt *UInt::clone() const
{
	return new UInt(value);
}

int UInt::compareTo(const Object *obj) const
{
	UInt *u = (UInt*)obj;

	if (value < u->value) {
		return -1;
	} else if (value > u->value) {
		return 1;
	} else {
		return 0;
	}
}

int UInt::toString(char *buf, int buflen) const
{
	return ht_snprintf(buf, buflen, "%u", value);
}

void UInt::load(ObjectStream &s)
{
	GET_INT32D(s, value);
}

ObjectID UInt::getObjectID() const
{
	return OBJID_UINT;
}

void UInt::store(ObjectStream &s) const
{
	PUT_INT32D(s, value);
}

/*
 *	A unsigned Integer (64-bit)
 */
UInt64 *UInt64::clone() const
{
	return new UInt64(value);
}

int UInt64::compareTo(const Object *obj) const
{
	UInt64 *u = (UInt64*)obj;

	if (value < u->value) {
		return -1;
	} else if (value > u->value) {
		return 1;
	} else {
		return 0;
	}
}

int UInt64::toString(char *buf, int buflen) const
{
	return ht_snprintf(buf, buflen, "%qu", value);
}

void UInt64::load(ObjectStream &s)
{
	GET_INT64D(s, value);
}

ObjectID UInt64::getObjectID() const
{
	return OBJID_UINT64;
}

void UInt64::store(ObjectStream &s) const
{
	PUT_INT64D(s, value);
}

/*
 *	A floating-point number	(FIXME: no portable storage yet)
 */
Float *Float::clone() const
{
	return new Float(value);
}

int Float::compareTo(const Object *obj) const
{
// FIXME: do we want to compare for equality using some error term epsilon ?
	Float *f = (Float*)obj;

	if (value < f->value) {
		return -1;
	} else if (value > f->value) {
		return 1;
	} else {
		return 0;
	}
}

int Float::toString(char *buf, int buflen) const
{
	return ht_snprintf(buf, buflen, "%f", value);
}

ObjectID Float::getObjectID() const
{
	return OBJID_FLOAT;
}

/**
 *	A memory area.
 */

MemArea::MemArea(const void *p, uint s, bool d)
{
	duplicate = d;
	size = s;
	if (duplicate) {
		ptr = malloc(size);
		if (!ptr) throw std::bad_alloc();
		memcpy(ptr, p, size);
	} else {
		// FIXME: un-const'ing p
		ptr = const_cast<void*>(p);
	}
}

MemArea::~MemArea()
{
	if (duplicate) free(ptr);
}

MemArea *MemArea::clone() const
{
	return new MemArea(ptr, size, true);
}

int MemArea::compareTo(const Object *obj) const
{
	const MemArea *a = this;
	const MemArea *b = (const MemArea*)obj;
	if (a->size != b->size) return a->size - b->size;
	return memcmp(a->ptr, b->ptr, a->size);
}

int MemArea::toString(char *buf, int buflen) const
{
	throw NotImplementedException(HERE);
}

void MemArea::load(ObjectStream &s)
{
	GET_INT32D(s, size);
	ptr = malloc(size);
	if (!ptr) throw std::bad_alloc();
	GET_BINARY(s, ptr, size);
}

ObjectID MemArea::getObjectID() const
{
	return OBJID_MEMAREA;
}

void MemArea::store(ObjectStream &s) const
{
	PUT_INT32D(s, size),
	PUT_BINARY(s, ptr, size);
}

/*
 *	IntSet
 */

IntSet::IntSet(uint aMaxSetSize)
{
	mMaxSetSize = aMaxSetSize;
	mSetSize = 0;
	mSet = NULL;
}

IntSet::~IntSet()
{
	free(mSet);
}

void IntSet::assign(const IntSet &from)
{
	delAll();
	if (from.mSetSize) makeAccessible(from.mSetSize-1);
	uint s = from.mSetSize ? idx2ByteOfs(from.mSetSize-1)+1 : 0;
	if (s) memcpy(mSet, from.mSet, s);
}

IntSet *IntSet::clone() const
{
	IntSet *s = new IntSet(mMaxSetSize);
	s->assign(*this);
	return s;
}

int IntSet::compareTo(const Object *obj) const
{
	const IntSet *a = (const IntSet*)this;
	const IntSet *b = (const IntSet*)obj;
	if (a->mSetSize == b->mSetSize) {
		uint s = a->mSetSize ? idx2ByteOfs(a->mSetSize-1)+1 : 0;
		return memcmp(a->mSet, b->mSet, s);
	}
	return a->mSetSize - b->mSetSize;
}

inline uint IntSet::idx2ByteOfs(uint i) const
{
	return i >> 3;
}

inline uint IntSet::idx2BitMask(uint i) const
{
	return 1 << (i & 7);
}

void IntSet::makeAccessible(uint i)
{
	if (!isAccessible(i)) {
		if (i+1 > mMaxSetSize) throw IndexOutOfBoundsException(HERE);
		uint oldByteSize = mSetSize ? idx2ByteOfs(mSetSize-1)+1 : 0;
		uint newByteSize = idx2ByteOfs(i)+1;
		// grow exponentially
		if (newByteSize < 3*oldByteSize/2) newByteSize = 3*oldByteSize/2;
		mSet = (byte*)realloc(mSet, newByteSize);
		if (!mSet) throw std::bad_alloc();
		memset(mSet+oldByteSize, 0, newByteSize-oldByteSize);
		mSetSize = i+1;
	}
}

inline bool IntSet::isAccessible(uint i) const
{
	return i < mSetSize;
}

bool IntSet::contains(uint i) const
{
	if (!isAccessible(i)) return false;
	return mSet[idx2ByteOfs(i)] & idx2BitMask(i);
}

void IntSet::del(uint i)
{
	makeAccessible(i);
	mSet[idx2ByteOfs(i)] &= ~idx2BitMask(i);
}

void IntSet::delAll()
{
	free(mSet);
	mSet = NULL;
	mSetSize = 0;
}

bool IntSet::findFirst(uint &i, bool set) const
{
	i = (uint)-1;
	return findNext(i, set);
}

bool IntSet::findNext(uint &i, bool set) const
{
	// FIXME: naive impl
	for (uint j=i+1; j < mSetSize; j++) {
		if (contains(j) == set) {
			i = j;
			return true;
		}
	}
	if (i+1 >= mSetSize && i+1 < mMaxSetSize && !set) {
		i++;
		return true;
	}
	return false;
}

bool IntSet::findPrev(uint &i, bool set) const
{
	// FIXME: naive impl
	if (i == 0) return false;
	if (i-1 >= mSetSize && i-1 < mMaxSetSize && !set) {
		i--;
		return true;
	}
	for (uint j=i-1; j >= 0; j--) {
		if (contains(j) == set) {
			i = j;
			return true;
		}
	}
	return false;
}

void IntSet::insert(uint i)
{
	makeAccessible(i);
	mSet[idx2ByteOfs(i)] |= idx2BitMask(i);
}

int IntSet::toString(char *buf, int buflen) const
{
	int w = 0;
	uint s;
	if (!findFirst(s, true)) return 0;
	w += ht_snprintf(buf+w, buflen-w, "(");
	while (buflen-w > 0) {
		w += ht_snprintf(buf+w, buflen-w, "%d", s);
		if (!findNext(s, true)) break;
		w += ht_snprintf(buf+w, buflen-w, ",");
	}
	w += ht_snprintf(buf+w, buflen-w, ")");
	return w;
}

/*
 *	sorter
 */

static void quickSortR(List &list, int l, int r)
{
	int m = (l+r)/2;
	int L = l;
	int R = r;
	Object *c = list[m];
	do {
		while ((l<=r) && (list.compareObjects(list[l], c)<0)) l++;
		while ((l<=r) && (list.compareObjects(list[r], c)>0)) r--;
		if (l<=r) {
			list.swap(list.findByIdx(l), list.findByIdx(r));
			l++;
			r--;
		}
	} while (l<r);
	if (L<r) quickSortR(list, L, r);
	if (l<R) quickSortR(list, l, R);
}

bool quickSort(List &l)
{
	int c = l.count();
	if (c) quickSortR(l, 0, c-1);
	return true;
}

/*
 *   matchhash
 */

const char *matchhash(int value, int_hash *hash_table)
{
	if (hash_table) {
		while (hash_table->desc) {
			if (hash_table->value==value) return hash_table->desc;
			hash_table++;
		}
	}
	return NULL;
}

/*
 *	Module Init/Done
 */

BUILDER2(OBJID_OBJECT, Object);

BUILDER(OBJID_ARRAY, Array, List);
BUILDER(OBJID_STACK, Stack, Array);

BUILDER(OBJID_BINARY_TREE, BinaryTree, Container);
BUILDER(OBJID_AVL_TREE, AVLTree, BinaryTree);
BUILDER(OBJID_SET, Set, AVLTree);

BUILDER(OBJID_SLINKED_LIST, SLinkedList, List);
BUILDER(OBJID_QUEUE, Queue, SLinkedList);
BUILDER(OBJID_DLINKED_LIST, DLinkedList, List);

BUILDER(OBJID_KEYVALUE, KeyValue, Object);
BUILDER(OBJID_SINT, SInt, Object);
BUILDER(OBJID_UINT, UInt, Object);
BUILDER(OBJID_MEMAREA, MemArea, Object);

BUILDER(OBJID_STRING, String, Object);
BUILDER(OBJID_ISTRING, IString, String);

bool init_data()
{
	registerAtom(OBJID_AUTO_COMPARE, (void*)&autoCompare);

	REGISTER(OBJID_OBJECT, Object);

	REGISTER(OBJID_ARRAY, Array);
	REGISTER(OBJID_STACK, Stack);

	REGISTER(OBJID_BINARY_TREE, BinaryTree);
	REGISTER(OBJID_AVL_TREE, AVLTree);
	REGISTER(OBJID_SET, Set);

	REGISTER(OBJID_SLINKED_LIST, SLinkedList);
	REGISTER(OBJID_QUEUE, Queue);
	REGISTER(OBJID_DLINKED_LIST, DLinkedList);

	REGISTER(OBJID_KEYVALUE, KeyValue);
	REGISTER(OBJID_SINT, SInt);
	REGISTER(OBJID_UINT, UInt);
	REGISTER(OBJID_MEMAREA, MemArea);

	REGISTER(OBJID_STRING, String);
	REGISTER(OBJID_ISTRING, IString);
	return true;
} 
 
void done_data()
{
	unregisterAtom(OBJID_AUTO_COMPARE);

	UNREGISTER(OBJID_OBJECT, Object);

	UNREGISTER(OBJID_ARRAY, Array);
	UNREGISTER(OBJID_STACK, Stack);

	UNREGISTER(OBJID_BINARY_TREE, BinaryTree);
	UNREGISTER(OBJID_AVL_TREE, AVLTree);
	UNREGISTER(OBJID_SET, Set);

	UNREGISTER(OBJID_SLINKED_LIST, SLinkedList);
	UNREGISTER(OBJID_QUEUE, Queue);
	UNREGISTER(OBJID_DLINKED_LIST, DLinkedList);

	UNREGISTER(OBJID_KEYVALUE, KeyValue);
	UNREGISTER(OBJID_SINT, SInt);
	UNREGISTER(OBJID_UINT, UInt);
	UNREGISTER(OBJID_MEMAREA, MemArea);

	UNREGISTER(OBJID_STRING, String);
	UNREGISTER(OBJID_ISTRING, IString);
}
