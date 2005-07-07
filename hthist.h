/* 
 *	HT Editor
 *	hthist.h
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

#ifndef HTHIST_H
#define HTHIST_H

#include "htobj.h"
#include "store.h"
#include "tools.h"

/*
 *	histories
 */

#define HISTATOM_GOTO			MAGICD("HIS\x80")
#define HISTATOM_FILE			MAGICD("HIS\x81")
#define HISTATOM_SEARCH_BIN		MAGICD("HIS\x82")
#define HISTATOM_SEARCH_EVALSTR	MAGICD("HIS\x83")
#define HISTATOM_SEARCH_VREGEX	MAGICD("HIS\x84")
#define HISTATOM_SEARCH_EXPR		MAGICD("HIS\x85")
#define HISTATOM_ASSEMBLER		MAGICD("HIS\x86")
#define HISTATOM_NAME_ADDR		MAGICD("HIS\x87")
#define HISTATOM_EVAL_EXPR		MAGICD("HIS\x88")

/*
 *	CLASS ht_history_entry
 */

class ht_history_entry: public ht_data {
public:
	char *desc;
	ht_object_stream_bin *data;
	ht_mem_file *datafile;
	
	ht_history_entry(char *str=0, ht_object_stream_bin *data=0, ht_mem_file *datafile=0);
	~ht_history_entry();
/* overwritten */
	virtual int	load(ObjectStream &s);
	virtual void	store(ObjectStream &s);
	virtual ObjectID getObjectID() const;
};

bool insert_history_entry(ht_list *history, char *name, ht_view *view);

void store_history(ObjectStream &s);
bool load_history(ObjectStream &s);

/*
 *	INIT
 */

bool init_hist();

/*
 *	DONE
 */

void done_hist();

#endif /* HTHIST_H */
