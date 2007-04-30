/* 
 *	HT Editor
 *	htxbeimp.h
 *
 *	Copyright (C) 2003 Stefan Esser
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

#ifndef __HTXBEIMP_H__
#define __HTXBEIMP_H__

#include "formats.h"
#include "xbestruct.h"
#include "htdialog.h"

extern format_viewer_if htxbeimports_if;

/*
 *	ht_xbe_import_function
 */
class ht_xbe_import_function: public Object {
public:
	uint libidx;
	bool byname;
	union {
		uint ordinal;
		struct {
			char *name;
			uint hint;
		} name;
	};
	RVA address;

	ht_xbe_import_function(RVA address, uint ordinal);
	ht_xbe_import_function(RVA address, char *name, uint hint);
	~ht_xbe_import_function();
};

struct ht_xbe_import {
	Container *funcs;
	Container *libs;
};

/*
 *	ht_xbe_import_viewer
 */
class ht_xbe_import_viewer: public ht_itext_listbox {
protected:
	ht_format_group *format_group;
	bool grouplib;
	uint sortby;
	/* new */
		void dosort();
public:
		void init(Bounds *b, const char *desc, ht_format_group *fg);
	virtual	void done();
	/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	bool select_entry(void *entry);
	/* new */
		const char *func(uint i, bool execute);
};

#endif /* !__HTXBEIMP_H__ */
