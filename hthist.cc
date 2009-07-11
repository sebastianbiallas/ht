/* 
 *	HT Editor
 *	hthist.cc
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

#include "atom.h"
#include "data.h"
#include "htdebug.h"
#include "hthist.h"
#include "strtools.h"
#include "tools.h"

#include <string.h>

#define ATOM_HT_HISTORY_ENTRY			MAGIC32("HIS\0")
#define ATOM_COMPARE_KEYS_HISTORY_ENTRY		MAGIC32("HIS\1")

#define MAX_HISTORY_ENTRY_COUNT			40

bool insert_history_entry(List *history, const char *name, ht_view *view)
{
	if (name && *name) {
		ObjectStreamBin *os = NULL;
		MemoryFile *file = NULL;
		if (view) {
			file = new MemoryFile();
			os = new ObjectStreamBin(file, false);
			view->getdata(*os);
		}

		ht_history_entry *e = new ht_history_entry(name, os, file);
		ObjHandle li = history->find(e);
		if (li == invObjHandle) {
			history->prepend(e);
		} else {
			delete e;
			history->moveTo(li, history->findFirst());
		}
		/* limit number of history entries to MAX_HISTORY_ENTRY_COUNT */
		while (history->count() > MAX_HISTORY_ENTRY_COUNT) {
			history->del(history->findLast());			
		}
		return true;
	}
	return false;
}

/*
 *	CLASS ht_history_entry
 */

ht_history_entry::ht_history_entry(const char *s, ObjectStreamBin *d, MemoryFile *file)
{
	desc = ht_strdup(s);
	assert(desc);
	data = d;
	datafile = file;
}

ht_history_entry::~ht_history_entry()
{
	free(desc);
	delete data;
	delete datafile;
}

int ht_history_entry::compareTo(const Object *o) const
{
	return strcmp(desc, ((ht_history_entry*)o)->desc);
}

void ht_history_entry::load(ObjectStream &s)
{
	GET_STRING(s, desc);
	uint size;
	GET_INT32D(s, size);

	if (size) {
		datafile = new MemoryFile();
		data = new ObjectStreamBin(datafile, false);

		byte d[size];
		GETX_BINARY(s, d, size, "data");
		datafile->write(d, size);
	} else {
		datafile = NULL;
		data = NULL;
	}
}

void ht_history_entry::store(ObjectStream &s) const
{
	PUT_STRING(s, desc);
	if (datafile) {
		uint size = datafile->getSize();
		PUT_INT32D(s, size);
		PUTX_BINARY(s, datafile->getBufPtr(), size, "data");
	} else {
		PUTX_INT32D(s, 0, "size");
	}
}

ObjectID ht_history_entry::getObjectID() const
{
	return ATOM_HT_HISTORY_ENTRY;
}

/*
 *	ATOMS
 */

static int hist_atoms[]={
	HISTATOM_GOTO,
	HISTATOM_FILE,
	HISTATOM_SEARCH_BIN,
	HISTATOM_SEARCH_EVALSTR,
	HISTATOM_SEARCH_VREGEX,
	HISTATOM_SEARCH_EXPR,
	HISTATOM_ASSEMBLER,
	HISTATOM_NAME_ADDR,
	HISTATOM_EVAL_EXPR
};

void create_hist_atom(uint atom)
{
	List *c = new Array(true);
	registerAtom(atom, c);
}

void destroy_hist_atom(uint atom)
{
	Object *c = (Object*)getAtomValue(atom);
	if (c) {
		unregisterAtom(atom);
		delete c;
	}
}

void store_history(ObjectStream &s)
{
	uint count = sizeof hist_atoms / sizeof hist_atoms[0];
	PUT_INT32D(s, count);
	for (uint i=0; i < count; i++) {
		putIDComment(s, hist_atoms[i]);
		PUTX_INT32X(s, hist_atoms[i], "atom");
		List *c = (List*)getAtomValue(hist_atoms[i]);
		PUTX_OBJECT(s, c, "list");
	}
}

bool load_history(ObjectStream &s)
{
	uint count;
	GET_INT32D(s, count);
	for (uint i=0; i < count; i++) {
		int atom;
		GET_INT32X(s, atom);
		destroy_hist_atom(atom);
		List *c = GETX_OBJECT(s, "list");
		registerAtom(atom, c);
	}
	return true;
}

/*
 *	INIT
 */

BUILDER(ATOM_HT_HISTORY_ENTRY, ht_history_entry, Object);

bool init_hist()
{
	for (uint i=0; i < sizeof hist_atoms / sizeof hist_atoms[0]; i++) {
		create_hist_atom(hist_atoms[i]);
	}

	REGISTER(ATOM_HT_HISTORY_ENTRY, ht_history_entry);

	return true;
}

/*
 *	DONE
 */

void done_hist()
{	
	UNREGISTER(ATOM_HT_HISTORY_ENTRY, ht_history_entry);

	for (uint i=0; i<sizeof hist_atoms / sizeof hist_atoms[0]; i++) {
		destroy_hist_atom(hist_atoms[i]);
	}
}

