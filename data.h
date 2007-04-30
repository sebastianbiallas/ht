/*
 *	HT Editor
 *	data.h
 *
 *	Copyright (C) 2002, 2003 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __DATA_H__
#define __DATA_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "io/types.h"
#include <cstdlib>

typedef uint32 ObjectID;
typedef uint32 ID;

class ObjectStream;

struct BuildCtorArg {
};


template <typename T1, typename T2>
inline bool instanceOf(const T2 *o)
{
	return (dynamic_cast<const T1*>(o) != NULL);
} 

/*
 *	C style malloc support
 */

class HTMallocRes;
HTMallocRes ht_malloc(size_t);

class HTMallocRes
{
private:
	friend HTMallocRes ht_malloc(size_t);
	const size_t mSize;

	HTMallocRes(size_t size)
		: mSize(size)
	{
	}

	HTMallocRes operator=(const HTMallocRes &); // not implemented

public:
	template <typename T> operator T* () const
	{
		return static_cast<T*>(::malloc(mSize));
	}
};

inline HTMallocRes ht_malloc(size_t size)
{
	return HTMallocRes(size);
}


/**
 *	Macro for creating object build functions
 */
#define BUILDER(reg, obj, parent) Object *build_##obj(){BuildCtorArg a;return new obj(a);}
#define BUILDER2(reg, obj) Object *build_##obj(){BuildCtorArg a;return new obj(a);}

/**
 *	Registers builder function by object id.
 */
#define REGISTER(reg, obj) registerAtom(reg, (void*)build_##obj);

/**
 *	Unregisters builder function by object id.
 */
#define UNREGISTER(reg, obj) unregisterAtom(reg);

/* actually a str => bigendian-int */
/** used to define ObjectIDs */
#define MAGIC32(magic) (unsigned long)(((unsigned char)magic[0]<<24) | ((unsigned char)magic[1]<<16) | ((unsigned char)magic[2]<<8) | (unsigned char)magic[3])

/** No/invalid object */
#define OBJID_INVALID			((ObjectID)0)
/** A placeholder object id */
#define OBJID_TEMP			((ObjectID)-1)

#define OBJID_OBJECT			MAGIC32("DAT\x00")

#define OBJID_ARRAY			MAGIC32("DAT\x10")
#define OBJID_STACK			MAGIC32("DAT\x11")

#define OBJID_BINARY_TREE		MAGIC32("DAT\x20")
#define OBJID_AVL_TREE			MAGIC32("DAT\x21")
#define OBJID_SET			MAGIC32("DAT\x22")

#define OBJID_SLINKED_LIST		MAGIC32("DAT\x30")
#define OBJID_QUEUE			MAGIC32("DAT\x31")
#define OBJID_DLINKED_LIST		MAGIC32("DAT\x32")

#define OBJID_KEYVALUE			MAGIC32("DAT\x40")
#define OBJID_SINT			MAGIC32("DAT\x41")
#define OBJID_SINT64			MAGIC32("DAT\x42")
#define OBJID_UINT			MAGIC32("DAT\x43")
#define OBJID_UINT64			MAGIC32("DAT\x44")
#define OBJID_FLOAT			MAGIC32("DAT\x45")

#define OBJID_MEMAREA			MAGIC32("DAT\x48")

#define OBJID_STRING			MAGIC32("DAT\x50")
#define OBJID_ISTRING			MAGIC32("DAT\x51")

#define OBJID_AUTO_COMPARE		MAGIC32("DAT\xc0")

/**
 *	This is THE base class.
 */
class Object {
public:
				Object(BuildCtorArg&) {};
				Object() {};

	virtual			~Object() {};
		void		init() {};
	virtual	void		done() {};
/* new */

/**
 *	Standard object duplicator.
 *	@returns copy of object
 */
	virtual	Object *	clone() const;
/**
 *	Standard Object comparator.
 *	@param obj object to compare to
 *	@returns 0 for equality, negative number if |this<obj| and positive number if |this>obj|
 */
	virtual	int		compareTo(const Object *obj) const;
/**
 *	Stringify object.
 *	Stringify object in string-buffer <i>s</i>. Never writes more than
 *	<i>maxlen</i> characters to <i>s</i>. If <i>maxlen</i> is > 0, a
 *	trailing zero-character is written.
 *
 *	@param buf pointer to buffer that receives object stringification
 *	@param buflen size of buffer that receives object stringification
 *	@returns number of characters written to <i>s</i>, not including the trailing zero
 */
	virtual	int		toString(char *buf, int buflen) const;
/**
 *	Standard Object idle function.
 *	Overwrite and register with htidle.cc::register_idle()
 *	(FIXME)
 *
 *	@returns true if working, false if really idle
 */
	virtual	bool		idle();
/**
 *	Load object from object stream.
 *
 *	@param s object stream to load this object from
 */
	virtual	void		load(ObjectStream &s);
/**
 *	@returns unique object id.
 */
	virtual	ObjectID	getObjectID() const;
/**
 *	stores object.
 *
 *	@param s object stream to store this object into
 */
	virtual	void		store(ObjectStream &s) const;
};

typedef int (*Comparator)(const Object *a, const Object *b);

int autoCompare(const Object *a, const Object *b);

typedef void* ObjHandle;
const ObjHandle invObjHandle = NULL;
const uint invIdx = ((uint)-1);

/**
 *	An Enumerator.
 */
class Enumerator: public Object {
public:
				Enumerator(BuildCtorArg&a): Object(a) {};
				Enumerator() {};
	/* extends Object */
	virtual Enumerator *	clone() const = 0;
	virtual	int		toString(char *buf, int buflen) const;
	/* new */

/**
 *	Count elements.
 *
 *	@returns number of objects contained in this structure
 *	@throws NotImplementedException if counting of elements is not supported
 */
	virtual	uint		count() const = 0;

/**
 *	Compare objects.
 *	Compare objects <i>a</i> and <i>b</i> and determine their (logical)
 *	linear order.
 *
 *	@param a object a
 *	@param b object b
 *	@returns 0 if <i>a</i> equals <i>b</i>,
 *	a value >0 if <i>a</i> is greater than <i>b</i> and
 *	a value <0 if <i>a</i> is less than <i>b</i>
 */
	virtual	int		compareObjects(const Object *a, const Object *b) const = 0;
/**
 *	Test if contained.
 *	Test if an object like <i>obj</i> is contained in this structure
 *
 *	@param obj signature of object to find
 *	@returns true if an object like <i>obj</i> is contained, false otherwise
 */
	inline	bool		contains(const Object *obj) const
	{
		return find(obj) != invObjHandle;
	}
/**
 *	Test if empty.
 *	@returns true if empty
 */
	inline bool		isEmpty() const
	{
		return count() == 0;
	}
/**
 *	Find equal object.
 *	Find first object equaling <i>obj</i> in this structure
 *	and if found return it's object handle.
 *
 *	@param obj signature of object to find
 *	@returns object handle of found object or <i>invObjHandle</i> if not found
 */
	virtual	ObjHandle	find(const Object *obj) const;
/**
 *	Find greater-or-equal object.
 *	Find lowest object being greater or equal compared to <i>obj</i> in this structure
 *	and if found return it's object handle. (lowest, greater and equal are
 *	defined via the compareTo method)
 *
 *	@param obj signature of object to find
 *	@returns object handle of found object or <i>invObjHandle</i> if not found
 */
	virtual	ObjHandle	findGE(const Object *obj) const;
/**
 *	Find greater object.
 *	Find lowest object being greater compared to <i>obj</i> in this structure
 *	and if found return it's object handle. (lowest and greater are
 *	defined via the compareTo method)
 *
 *	@param obj signature of object to find
 *	@returns object handle of found object or <i>invObjHandle</i> if not found
 */
	virtual	ObjHandle	findG(const Object *obj) const;
/**
 *	Find lower-or-equal object.
 *	Find greatest object being lower or equal compared to <i>obj</i> in this structure
 *	and if found return it's object handle. (greatest, lower and equal are
 *	defined via the compareTo method)
 *
 *	@param obj signature of object to find
 *	@returns object handle of found object or <i>invObjHandle</i> if not found
 */
	virtual	ObjHandle	findLE(const Object *obj) const;
/**
 *	Find lower object.
 *	Find greatest object being lower compared to <i>obj</i> in this structure
 *	and if found return it's object handle. (greatest and lower are
 *	defined via the compareTo method)
 *
 *	@param obj signature of object to find
 *	@returns object handle of found object or <i>invObjHandle</i> if not found
 */
	virtual	ObjHandle	findL(const Object *obj) const;
/**
 *	Find object's handle by index.
 *
 *	@param i index of object to find
 *	@returns object handle of found object or <i>invObjHandle</i> if not found
 */	
	virtual	ObjHandle	findByIdx(int i) const = 0;
/**
 *	Find (logically) last element's object handle.
 *
 *	@returns object handle of the last element or <i>invObjHandle</i>
 *	if the structure is empty
 */
	virtual	ObjHandle	findLast() const = 0;
/**
 *	Find (logically) previous element's object handle.
 *	Find logically previous element (predecessor) of the object identified
 *	by <i>h</i>. Predecessor of "the invalid object" is the last element
 *	in this structure by definition. (ie. <i>findPrev(invObjHandle) :=
 *	findLast()</i>).
 *
 *	@param h object handle to find a predecessor to
 *	@returns object handle of predecessor or <i>invObjHandle</i> if <i>h</i>
 *	identifies the first element.
 */
	virtual	ObjHandle	findPrev(ObjHandle h) const = 0;
/**
 *	Find (logically) first element's object handle.
 *
 *	@returns object handle of the first element or <i>invObjHandle</i>
 *	if this structure is empty
 */
	virtual	ObjHandle	findFirst() const = 0;
/**
 *	Find (logically) next element's object handle.
 *	Find logically next element (successor) of the object, identified
 *	by <i>h</i>. Successor of "the invalid object" is the first element
 *	in this structure by definition. (ie. <i>findNext(invObjHandle) :=
 *	findFirst()</i>).
 *
 *	@param h object handle to find a successor to
 *	@returns object handle of successor or <i>invObjHandle</i> if <i>h</i>
 *	identifies the last element.
 */
	virtual	ObjHandle	findNext(ObjHandle h) const = 0;
/**
 *	Get object pointer from object handle.
 *
 *	@param h object handle
 *	@returns object pointer if <i>h</i> is valid, <i>NULL</i> otherwise
 */
	virtual	Object *	get(ObjHandle h) const = 0;
/**
 *	Get object's index from its handle.
 *
 *	@param h object handle of object
 *	@returns index of object if <i>h</i> is valid, <i>InvIdx</i> otherwise.
 */
	virtual	uint		getObjIdx(ObjHandle h) const = 0;
/**
 *	Get element by index.
 *	Get the element with index <i>idx</i> (if possible).
 *
 *	@param idx index of element to get
 *	@returns pointer to the requested element or <i>NULL</i> if <i>idx</i>
 *	is invalid.
 */
		Object *	operator [] (int idx) const
		{
			return get(findByIdx(idx));
		}
};

#define foreach(XTYPE, X, E, code...)\
for (ObjHandle temp0815 = (E).findFirst(); temp0815 != invObjHandle;) {\
	XTYPE *X = (XTYPE*)(E).get(temp0815);                          \
	temp0815 = (E).findNext(temp0815);                             \
	{code;}                                                        \
}

#define foreachbwd(XTYPE, X, E, code...)\
for (ObjHandle temp0815 = (E).findLast(); temp0815 != invObjHandle;) {\
	XTYPE *X = (XTYPE*)(E).get(temp0815);                         \
	temp0815 = (E).findPrev(temp0815);                            \
	{code;}                                                       \
}

#define firstThat(XTYPE, X, E, condition) \
{                                         \
	XTYPE *Y = NULL;                  \
	foreach(XTYPE, X, E,              \
		if (condition) {          \
			Y = X;            \
			break;            \
		}                         \
	)                                 \
	X = Y;                            \
}

#define lastThat(XTYPE, X, E, condition)  \
{                                         \
	XTYPE *Y = NULL;                  \
	foreachbwd(XTYPE, X, E,           \
		if (condition) {          \
			Y = X;            \
			break;            \
		}                         \
	)                                 \
	X = Y;                            \
}

/**
 *	A Container.
 */
class Container: public Enumerator {
protected:
	ObjectID	hom_objid;

	virtual	void		notifyInsertOrSet(const Object *o);
public:
				Container(BuildCtorArg&a): Enumerator(a) {};
				Container();
				
	/* extends Enumerator */
	virtual Container *	clone() const = 0;

	/* new */
/**
 *	Delete all objects. (ie. remove and free all objects)
 */
	virtual	void		delAll() = 0;
/**
 *	Delete object.
 *	Delete (ie. remove and free) first object like <i>sig</i> in
 *	this structure (if possible).
 *
 *	@param sig signature of object to delete (may be inserted in the structure)
 *	@returns true if an object has been deleted, false otherwise
 */
	virtual	bool		delObj(Object *sig);
/**
 *	Delete object.
 *	Delete (ie. remove and free) object identified by <i>h</i>.
 *
 *	@param h object handle of the object to delete
 *	@returns true if the object has been deleted, false otherwise
 */
	virtual	bool		del(ObjHandle h) = 0;
/**
 *	Find or insert object.
 *	Find first object like <i>obj</i> and if that fails, insert <i>obj</i>.
 *	Ie. after call of this function it is guaranteed that <i>contains(obj)</i>.
 *
 *	@param obj object to find similar one to or object to insert
 *	@param inserted indicates if <i>obj</i> has been inserted
 *	@returns object handle of existing or inserted object (never <i>invObjHandle</i>)
 */
	virtual	ObjHandle	findOrInsert(Object *obj, bool &inserted);
/**
 *	Insert object.
 *	Insert <i>obj</i>
 *
 *	@param obj object to insert
 *	@returns object handle of inserted object (never <i>invObjHandle</i>)
 */
	virtual	ObjHandle	insert(Object *obj) = 0;
/**
 *	Remove object.
 *	Remove first object like <i>sig</i> from this structure.
 *	Returned object must be freed explicitly.
 *
 *	@param sig signature of object to remove
 *	@returns removed object
 */
	virtual	Object *	removeObj(const Object *sig);
/**
 *	Remove object.
 *	Remove object identified by <i>h</i>.
 *	Returned object must be freed explicitly.
 *
 *	@param h object handle of object to remove
 *	@returns removed object
 */
	virtual	Object *	remove(ObjHandle h) = 0;
/**
 *	Insert object. (operator-form)
 */
	inline	ObjHandle	operator += (Object *obj) { return insert(obj); }
/**
 *	Delete object. (operator-form)
 */
	inline	bool		operator -= (ObjHandle h) { return del(h); }
/**
 *	Delete object. (operator-form)
 */
	inline	bool		operator -= (Object *sig) { return (*this -= find(sig)); }
/**
 *	Delete object by index.
 *
 *	@param idx index of object to delete
 *	@returns true if the object has been deleted, false otherwise
 */
	inline	bool		operator -= (int idx) { return (*this -= findByIdx(idx)); }
};

/**
 *   An abstract list
 */
class List: public Container {
public:
				List(BuildCtorArg&a): Container(a) {};
				List() {};
	/* extends Enumerator */
	virtual List *		clone() const = 0;
	
	/* new */

/**
 *	Insert object at position.
 *	Insert object <i>obj</i> at the position specified by <i>h</i>.
 *	if <i>h</i> does not specify a valid object handle (eg. invObjHandle),
 *	this function works like <i>insert(obj)</i>.
 *
 *	@param h position to insert object at
 *	@param obj pointer to object to insert
 *	@returns true on success, false otherwise
 */
	virtual	void		insertAt(ObjHandle h, Object *obj) = 0;
/**
 *	Move element.
 *	Move element from position <i>from</i> to position <i>to</i>.
 *
 *	@param from position of element to move
 *	@param to position to move element to
 *	@returns true on success, false otherwise
 */
	virtual	bool		moveTo(ObjHandle from, ObjHandle to) = 0;
/**
 *	Prepend object.
 *	Prepend object <i>obj</i>.
 *
 *	@param obj pointer to object to prepend
 *	@returns object handle of inserted object (never <i>invObjHandle</i>)
 */
	inline	ObjHandle	prepend(Object *obj)
	{
		insertAt(findFirst(), obj);
		return findFirst();
	}
/**
 *	Set element.
 *	Replace element at position <i>h</i> with object <i>obj</i>
 *	and delete replaced object.
 *
 *	@param h object handle of element to replace
 *	@param obj object to replace element
 *	@returns true on success, false otherwise
 */
	virtual	bool		set(ObjHandle h, Object *obj) = 0;
/**
 *	Force: Set element by index.
 *	Set element at index <i>i</i> to object <i>obj</i>
 *	and delete object previously located at this index if the index is valid.
 *	If the index <i>i</i> does not specify a valid list-index,
 *	the list is extended, so that <i>obj</i> will be the last element
 *	and the newly created entries in the list will be <i>NULL</i>s.
 *
 *	@param i index at which to set
 *	@param obj object to set
 */
	virtual	void		forceSetByIdx(int idx, Object *obj) = 0;
/**
 *	Swap two element.
 *	Swap element at position <i>h</i> with element at position <i>i</i>.
 *
 *	@param h handle of one object
 *	@param i handle of the other object
 *	@returns true on success, false otherwise
 */
	virtual	bool		swap(ObjHandle h, ObjHandle i) = 0;
};

#define ARRAY_CONSTR_ALLOC_DEFAULT		4

/**
 *   An array
 */
class Array: public List {
private:
	bool own_objects;
	uint ecount;
	uint acount;
	Object **elems;

	virtual	int		calcNewBufferSize(int curbufsize, int min_newbufsize) const;
	virtual	void		checkShrink();
	virtual	void		freeObj(Object *obj);
		void		prepareWriteAccess(int i);
		void		realloc(int n);
	inline	bool		validHandle(ObjHandle h) const
	{
		return (handleToNative(h) < ecount);
	}
	inline	uint		handleToNative(ObjHandle h) const
	{
		return (Object**)h-elems;
	}
	inline	ObjHandle	nativeToHandle(int i) const
	{
		return elems+i;
	}
public:
				Array(BuildCtorArg &a): List(a) {};
				Array(bool own_objects, int prealloc = ARRAY_CONSTR_ALLOC_DEFAULT);
	virtual			~Array();
	/* extends Object */
	virtual	Array *		clone() const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
	/* extends Enumerator */
	virtual	uint		count() const
	{
		return ecount;
	}
	virtual	int		compareObjects(const Object *a, const Object *b) const;
	virtual	ObjHandle	findByIdx(int i) const;
	virtual	ObjHandle	findFirst() const;
	virtual	ObjHandle	findLast() const;
	virtual	ObjHandle	findNext(ObjHandle h) const;
	virtual	ObjHandle	findPrev(ObjHandle h) const;
	virtual	Object *	get(ObjHandle h) const;
	virtual	uint		getObjIdx(ObjHandle h) const;
	/* extends Container */
	virtual	void		delAll();
	virtual	bool		del(ObjHandle h);
	virtual	ObjHandle	insert(Object *obj);
	virtual	Object *	remove(ObjHandle h);
	/* extends List */
	virtual	void		forceSetByIdx(int idx, Object *obj);
	virtual	void		insertAt(ObjHandle h, Object *obj);
	virtual	bool		moveTo(ObjHandle from, ObjHandle to);
	virtual	bool		set(ObjHandle h, Object *obj);
	virtual	bool		swap(ObjHandle h, ObjHandle i);
	/* new */
/**
 *	Delete range of objects. (ie. remove and free all objects)
 *	@return number of objects deleted
 */
	virtual	int		delRange(int start, int end);

	inline void		insertBefore(int idx, Object *obj)
	{
		insertAt(findByIdx(idx), obj);
	}
};

/**
 *   A stack
 */
class Stack: public Array {
public:
				Stack(BuildCtorArg &a): Array(a) {};
				Stack(bool own_objects);
	/* new */
	virtual Object *	pop();
	virtual void		push(Object *obj);
	virtual	ObjectID	getObjectID() const;
};

/**
 *	SLinkedList's node structure
 */
struct SLinkedListNode {
	Object *obj;
	SLinkedListNode *next;
};

/**
 *	A (simply) linked list
 */
class SLinkedList: public List {
private:
	bool own_objects;
	uint ecount;
	SLinkedListNode *first, *last;

	virtual	SLinkedListNode *allocNode() const;
	virtual	void		deleteNode(SLinkedListNode *node) const;
	virtual	void		freeObj(Object *obj) const;
	inline	bool		validHandle(ObjHandle h) const;
	inline	SLinkedListNode *handleToNative(ObjHandle h) const;
	inline	ObjHandle	nativeToHandle(SLinkedListNode *n) const;
public:
				SLinkedList(BuildCtorArg&a): List(a) {};
				SLinkedList(bool own_objects);
	virtual			~SLinkedList();
	/* extends Object */
	virtual	SLinkedList *	clone() const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
	/* extends Enumerator */
	virtual	uint		count() const;
	virtual	int		compareObjects(const Object *a, const Object *b) const;
	virtual	ObjHandle	findByIdx(int i) const;
	virtual	ObjHandle	findFirst() const;
	virtual	ObjHandle	findLast() const;
	virtual	ObjHandle	findNext(ObjHandle h) const;
	virtual	ObjHandle	findPrev(ObjHandle h) const;
	virtual	Object *	get(ObjHandle h) const;
	virtual	uint		getObjIdx(ObjHandle h) const;
	/* extends Container */
	virtual	void		delAll();
	virtual	bool		del(ObjHandle h);
	virtual	ObjHandle	insert(Object *obj);
	virtual	Object *	remove(ObjHandle h);
	/* extends List */
	virtual	void		forceSetByIdx(int idx, Object *obj);
	virtual	void		insertAt(ObjHandle h, Object *obj);
	virtual	bool		moveTo(ObjHandle from, ObjHandle to);
	virtual	bool		set(ObjHandle h, Object *obj);
	virtual	bool		swap(ObjHandle h, ObjHandle i);
};

/**
 *   A queue
 */
class Queue: public SLinkedList {
public:
				Queue(BuildCtorArg&a): SLinkedList(a) {};
				Queue(bool own_objects);
/* new */

/**
 *	De-queue element.
 *	Remove and return next element of the queue.
 *
 *	@returns next element of the queue
 */
	inline	Object *	deQueue()
	{
		return remove(findFirst());
	}

/**
 *	En-queue element.
 *	Append element <i>obj</i> to the queue.
 *
 *	@param obj pointer to object to en-queue
 */
	inline	void		enQueue(Object *obj)
	{
		insert(obj);
	}

	virtual	ObjectID	getObjectID() const;
};

/**
 *	DLinkedList's node structure
 */
struct DLinkedListNode {
	Object *obj;
	DLinkedListNode *prev;
	DLinkedListNode *next;
};

/**
 *	A (doubly) linked list
 */
class DLinkedList: public List {
private:
	bool own_objects;
	uint ecount;
	DLinkedListNode *first, *last;

	virtual	DLinkedListNode *allocNode() const;
	virtual	void		deleteNode(DLinkedListNode *node) const;
	virtual	void		freeObj(Object *obj) const;
	inline	bool		validHandle(ObjHandle h) const;
	inline	DLinkedListNode *handleToNative(ObjHandle h) const;
	inline	ObjHandle	nativeToHandle(DLinkedListNode *n) const;
public:
				DLinkedList(BuildCtorArg&a): List(a) {};
				DLinkedList(bool own_objects);
	virtual			~DLinkedList();
	/* extends Object */
	virtual	DLinkedList *	clone() const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
	/* extends Enumerator */
	virtual	uint		count() const;
	virtual	int		compareObjects(const Object *a, const Object *b) const;
	virtual	ObjHandle	findByIdx(int i) const;
	virtual	ObjHandle	findFirst() const;
	virtual	ObjHandle	findLast() const;
	virtual	ObjHandle	findNext(ObjHandle h) const;
	virtual	ObjHandle	findPrev(ObjHandle h) const;
	virtual	Object *	get(ObjHandle h) const;
	virtual	uint		getObjIdx(ObjHandle h) const;
	/* extends Container */
	virtual	void		delAll();
	virtual	bool		del(ObjHandle h);
	virtual	ObjHandle	insert(Object *obj);
	virtual	Object *	remove(ObjHandle h);
	/* extends List */
	virtual	void		forceSetByIdx(int idx, Object *obj);
	virtual	void		insertAt(ObjHandle h, Object *obj);
	virtual	bool		moveTo(ObjHandle from, ObjHandle to);
	virtual	bool		set(ObjHandle h, Object *obj);
	virtual	bool		swap(ObjHandle h, ObjHandle i);
};

/**
 *   BinaryTree's node structure
 */
struct BinTreeNode {
	Object *key;
	BinTreeNode *left, *right;
	int unbalance;
};

/**
 *   A simple binary tree
 */
class BinaryTree: public Container {
protected:
	bool own_objects;
	uint ecount;
	BinTreeNode *root;
	Comparator compare;

		BinTreeNode *	allocNode() const;
		void		cloneR(BinTreeNode *node);
	virtual	void		deleteNode(BinTreeNode *node) const;
		BinTreeNode *	findNode(BinTreeNode *node, const Object *obj) const;
		BinTreeNode *	findNodeG(BinTreeNode *node, const Object *obj) const;
		BinTreeNode *	findNodeGE(BinTreeNode *node, const Object *obj) const;
		BinTreeNode *	findNodeL(BinTreeNode *node, const Object *obj) const;
		BinTreeNode *	findNodeLE(BinTreeNode *node, const Object *obj) const;
		BinTreeNode **	findNodePtr(BinTreeNode **nodeptr, const Object *obj) const;
		void		freeAll(BinTreeNode *n);
		void		freeObj(Object *obj) const;
		BinTreeNode *	getLeftmost(BinTreeNode *node) const;
		BinTreeNode *	getRightmost(BinTreeNode *node) const;
		BinTreeNode **	getLeftmostPtr(BinTreeNode **nodeptr) const;
		BinTreeNode **	getRightmostPtr(BinTreeNode **nodeptr) const;
		ObjHandle	findByIdxR(BinTreeNode *n, int &i) const;
		ObjHandle	insertR(BinTreeNode *&node, Object *obj);
		void 		loadR(ObjectStream &s, BinTreeNode **n, int l, int r);
		void 		storeR(ObjectStream &s, BinTreeNode *n) const;
	virtual	void		setNodeIdentity(BinTreeNode *node, BinTreeNode *newident);
	inline	bool		validHandle(ObjHandle h) const { return (h != invObjHandle); }
	inline	BinTreeNode *	handleToNative(ObjHandle h) const { return (BinTreeNode*)h; }
	inline	ObjHandle	nativeToHandle(BinTreeNode *n) const { return (ObjHandle*)n; }
public:
				BinaryTree(BuildCtorArg&a): Container(a) {};
				BinaryTree(bool own_objects, Comparator comparator = autoCompare);
	virtual			~BinaryTree();
	/* extends Object */
	virtual	BinaryTree *	clone() const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
	/* extends Enumerator */
	virtual	void		delAll();
	virtual	uint		count() const;
	virtual	int		compareObjects(const Object *a, const Object *b) const;
	virtual	ObjHandle	find(const Object *obj) const;
	virtual	ObjHandle	findG(const Object *obj) const;
	virtual	ObjHandle	findGE(const Object *obj) const;
	virtual	ObjHandle	findL(const Object *obj) const;
	virtual	ObjHandle	findLE(const Object *obj) const;
	virtual	ObjHandle	findByIdx(int i) const;
	virtual	ObjHandle	findFirst() const;
	virtual	ObjHandle	findLast() const;
	virtual	ObjHandle	findNext(ObjHandle h) const;
	virtual	ObjHandle	findPrev(ObjHandle h) const;
	virtual	Object *	get(ObjHandle h) const;
	virtual	uint		getObjIdx(ObjHandle h) const;
	/* extends Container */
	virtual	bool		del(ObjHandle h);
	virtual	ObjHandle	insert(Object *obj);
	virtual	Object *	remove(ObjHandle h);
};


/**
 *   A height-balanced binary tree (AVL)
 */
class AVLTree: public BinaryTree {
private:
		void		cloneR(BinTreeNode *node);
		BinTreeNode *	removeR(Object *key, BinTreeNode *&root, int &change, int cmp);
		int		loadR(ObjectStream &s, BinTreeNode *&n, int l, int r);
public:
				AVLTree(BuildCtorArg&a): BinaryTree(a) {};
				AVLTree(bool own_objects, Comparator comparator = autoCompare);

		void		debugOut();
		bool		expensiveCheck() const;
	/* extends Object */
	virtual	AVLTree *	clone() const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	/* extends Container */
	virtual	ObjHandle	insert(Object *obj);
	virtual	Object *	remove(ObjHandle h);
};

/**
 *   MRU Cache's node structure
 */
struct MRUCacheNode: public BinTreeNode {
	MRUCacheNode	*moreRU, *lessRU;
};

/**
 *   A most-recently-used (MRU) cache
 */
class MRUCache: public AVLTree {
private:
	MRUCacheNode *		mostRU;
	MRUCacheNode *		leastRU;

	virtual	MRUCacheNode *	allocNode() const;
		void		checkList() const;
	virtual	void		deleteNode(BinTreeNode *node) const;
	inline	MRUCacheNode *	handleToNative(ObjHandle h) const { return (MRUCacheNode*)h; }
	inline	ObjHandle	nativeToHandle(MRUCacheNode *n) const { return (ObjHandle)n; }
	virtual	void		setNodeIdentity(BinTreeNode *node, BinTreeNode *newident);
public:
				MRUCache(bool own_objects, Comparator comparator = autoCompare);

	/* extends Object */
	virtual	MRUCache *	clone() const;
	virtual	void		store(ObjectStream &s) const;
	/* extends Container */
	virtual	ObjHandle	insert(Object *obj);
	virtual	Object *	remove(ObjHandle h);
	/* extends AVLTree */
	virtual	void		delAll();
	/* new */
		void		propagate(ObjHandle h);
		ObjHandle	getLRU();
};

/**
 *	A finite set
 */
class Set: public AVLTree {
public:
				Set(BuildCtorArg&a):AVLTree(a) {};
				Set(bool own_objects);
	/* new */
			void	intersectWith(Set *b);
			void	unionWith(Set *b);
	inline	bool	operator &(Object *obj) const
	{
		return contains(obj);
	}

	virtual	ObjectID	getObjectID() const;
};

/*
 *	IntSet
 */

class IntSet: public Object {
protected:
	uint mMaxSetSize;	// in bits
	uint mSetSize;		// in bits
	byte *mSet;
	/* new */
	inline	uint	idx2ByteOfs(uint i) const;
	inline	uint	idx2BitMask(uint i) const;
		void	makeAccessible(uint i);
	inline	bool	isAccessible(uint i) const;
public:
			IntSet(uint aMaxSetSize);
	virtual		~IntSet();
	/* extends Object */
	virtual	IntSet *clone() const;
	virtual	int	compareTo(const Object *obj) const;
	virtual	int	toString(char *buf, int buflen) const;
	/* new */
		void	assign(const IntSet &from);
		bool	contains(uint i) const;
		void	del(uint i);
		void	delAll();
		bool	findFirst(uint &i, bool set) const;
		bool	findNext(uint &i, bool set) const;
		bool	findPrev(uint &i, bool set) const;
		void	insert(uint i);
};

/**
 *	Maintains a key-value pair for easy inserting objects with "simple" keys
 *	into Containers.
 *	Key and Value will be <code>delete</code>d in the destructor.
 */
class KeyValue: public Object {
public:
	Object		*mKey;
	Object		*mValue;

				KeyValue(BuildCtorArg&a): Object(a) {};
				KeyValue(Object *aKey, Object *aValue): mKey(aKey), mValue(aValue) {};
	virtual			~KeyValue();

	virtual	KeyValue *	clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

/**
 *	A signed Integer
 */
class SInt: public Object {
public:
	signed int value;

				SInt(BuildCtorArg&a): Object(a) {};
				SInt(signed int i): value(i) {};
	/* extends Object */
	virtual	SInt *		clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

typedef SInt Integer;

/**
 *	A signed Integer (64-bit)
 */
class SInt64: public Object {
public:
	sint64 value;

				SInt64(BuildCtorArg&a): Object(a) {};
				SInt64(sint64 i): value(i) {};
	/* extends Object */
	virtual	SInt64 *	clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

/**
 *	An unsigned Integer
 */
class UInt: public Object {
public:
	unsigned int value;

				UInt(BuildCtorArg&a): Object(a) {};
				UInt(unsigned int i): value(i) {};
	/* extends Object */
	virtual	UInt *		clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

/**
 *	An unsigned Integer (64-bit)
 */
class UInt64: public Object {
public:
	uint64 value;

				UInt64(BuildCtorArg&a): Object(a) {};
				UInt64(uint64 i): value(i) {};
	/* extends Object */
	virtual UInt64 *	clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

/**
 *	A floating-point number	(FIXME: no portable storage yet)
 */
class Float: public Object {
public:
	double value;

				Float(BuildCtorArg&a): Object(a) {};
				Float(double d): value(d) {};
	/* extends Object */
	virtual	Float *		clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
//	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
//	virtual	void		store(ObjectStream &s) const;
};

/**
 *	A pointer. Cannot be stored.
 */
class Pointer: public Object {
public:
	void *value;

	Pointer(void *p): value(p) {};
};

/**
 *	A memory area.
 */
class MemArea: public Object {
private:
	bool duplicate;
public:
	void *ptr;
	uint size;

				MemArea(BuildCtorArg&a): Object(a) {};
				MemArea(const void *p, uint size, bool duplicate = false);
				~MemArea();
	/* extends Object */
	virtual	MemArea *	clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		toString(char *buf, int buflen) const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

/*
 *	sorter
 */
bool quickSort(List &l);


/*
 *	char_set
 */

#define CS_SETSIZE 256

typedef struct char_set {
  unsigned char char_bits [((CS_SETSIZE) + 7) / 8];
} char_set;

#define CS_SET(n, p)    ((p)->char_bits[(n) / 8] |= (1 << ((n) & 7)))
#define CS_CLR(n, p)	((p)->char_bits[(n) / 8] &= ~(1 << ((n) & 7)))
#define CS_ISSET(n, p)	((p)->char_bits[(n) / 8] & (1 << ((n) & 7)))
#define CS_ZERO(p)	memset ((void *)(p), 0, sizeof (*(p)))

/*
 *
 */

#define BITMAP(a0, a1, a2, a3, a4, a5, a6, a7) (((a0)<<0) | ((a1)<<1) | ((a2)<<2) | ((a3)<<3) | ((a4)<<4) | ((a5)<<5) | ((a6)<<6) | ((a7)<<7))

#define BITBIT(bitmap, p) ((bitmap)>>(p)&1)

/*
 *	simple int hash
 */

struct int_hash {
	int value;
	const char *desc;
};

const char *matchhash(int value, int_hash *hash_table);

#include "stream.h"			// load/store need ObjectStream


/*
 *	Module Init/Done
 */

bool init_data();
void done_data();

#endif /* __DATA_H__ */
