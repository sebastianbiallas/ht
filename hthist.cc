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

#include "htatom.h"
#include "htdata.h"
#include "hthist.h"
#include "htstring.h"
#include "tools.h"

#include <string.h>

#define ATOM_HT_HISTORY_ENTRY				MAGICD("HIS\0")
#define ATOM_COMPARE_KEYS_HISTORY_ENTRY		MAGICD("HIS\1")

#define MAX_HISTORY_ENTRY_COUNT		40

bool insert_history_entry(ht_list *history, char *name, ht_view *view)
{
	if (name && *name) {
		ht_object_stream_bin *os=0;
		ht_mem_file *file=0;
		if (view) {
				file=new ht_mem_file();
				file->init();

				os=new ht_object_stream_bin();
				os->init(file);

				view->getdata(os);
		}

		ht_history_entry *e=new ht_history_entry(name, os, file);
		uint li=history->find(e);
		int r=0;
		if (li==LIST_UNDEFINED) {
			history->prepend(e);
			r=1;
		} else {
			delete e;
			history->move(li, 0);
		}
/* limit number of history entries to MAX_HISTORY_ENTRY_COUNT */
		if (history->count() > MAX_HISTORY_ENTRY_COUNT) {
			history->del_multiple(MAX_HISTORY_ENTRY_COUNT, history->count() - MAX_HISTORY_ENTRY_COUNT);
		}
		return 1;
	}
	return 0;
}

/*
 *	CLASS ht_history_entry
 */

ht_history_entry::ht_history_entry(char *s, ht_object_stream_bin *d, ht_mem_file *df)
{
	if (s) desc=ht_strdup(s);
	data=d;
	datafile=df;
}

ht_history_entry::~ht_history_entry()
{
	if (desc) free(desc);
	if (data) {
		data->done();
		delete data;
	}
	if (datafile) {
		datafile->done();
		delete datafile;
	}
}

int ht_history_entry::load(ObjectStream &s)
{
	desc=s->getString(NULL);

	uint size=s->getInt(4, NULL);

	if (size) {
		datafile=new ht_mem_file();
		datafile->init();

		data=new ht_object_stream_bin();
		data->init(datafile);

		void *d=s->getBinary(size, NULL);
		datafile->write(d, size);
		free(d);
	} else {
		datafile=0;
		data=0;
	}

	return 0;
}

void ht_history_entry::store(ObjectStream &s)
{
	s->putString(desc, NULL);

	if (datafile) {
		uint size=datafile->get_size();
	
		s->putInt(size, 4, NULL);
		s->putBinary(datafile->bufptr(), size, NULL);
	} else {
		s->putInt(0, 4, NULL);
	}
}

ObjectID ht_history_entry::getObjectID() const
{
	return ATOM_HT_HISTORY_ENTRY;
}

int compare_keys_history_entry(ht_data *key_a, ht_data *key_b)
{
    return strcmp(((ht_history_entry*)key_a)->desc, ((ht_history_entry*)key_b)->desc);
}

/*
 *	ATOMS
 */

int hist_atoms[]={
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

void create_hist_atom(UINT atom)
{
	ht_clist *c=new ht_clist();
	c->init(compare_keys_history_entry);
	register_atom(atom, c);
}

void destroy_hist_atom(UINT atom)
{
	ht_clist *c=(ht_clist*)find_atom(atom);
	if (c) {
		unregister_atom(atom);
		c->destroy();
		delete c;
	}
}

void store_history(ObjectStream &s)
{
	uint count=sizeof hist_atoms / sizeof hist_atoms[0];
	s->putIntDec(count, 4, NULL);
	for (UINT i=0; i<count; i++) {
		s->putIntHex(hist_atoms[i], 4, NULL);
		ht_clist *c=(ht_clist*)find_atom(hist_atoms[i]);
		s->putObject(c, NULL);
	}
}

bool load_history(ObjectStream &s)
{
	uint count=s->getIntDec(4, NULL);
	for (UINT i=0; i<count; i++) {
		int atom=s->getIntHex(4, NULL);
		destroy_hist_atom(atom);
		ht_clist *c=(ht_clist*)s->getObject(NULL);
		register_atom(atom, c);
	}
	return 1;
}

/*
 *	INIT
 */

BUILDER(ATOM_HT_HISTORY_ENTRY, ht_history_entry);

bool init_hist()
{
	for (UINT i=0; i<sizeof hist_atoms / sizeof hist_atoms[0]; i++) {
		create_hist_atom(hist_atoms[i]);
	}
	
	REGISTER(ATOM_HT_HISTORY_ENTRY, ht_history_entry);
	
	register_atom(ATOM_COMPARE_KEYS_HISTORY_ENTRY, (void*)compare_keys_history_entry);

	return true;
}

/*
 *	DONE
 */

void done_hist()
{

	unregister_atom(ATOM_COMPARE_KEYS_HISTORY_ENTRY);
	
	UNREGISTER(ATOM_HT_HISTORY_ENTRY, ht_history_entry);

	for (UINT i=0; i<sizeof hist_atoms / sizeof hist_atoms[0]; i++) {
		destroy_hist_atom(hist_atoms[i]);
	}
}

