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
 *	class ht_xbe_import_function
 */

class ht_xbe_import_function: public ht_data {
public:
	UINT libidx;
	bool byname;
	union {
		UINT ordinal;
		struct {
			char *name;
			UINT hint;
		} name;
	};
	RVA address;

	ht_xbe_import_function(RVA address, UINT ordinal);
	ht_xbe_import_function(RVA address, char *name, UINT hint);
	~ht_xbe_import_function();
};

struct ht_xbe_import {
	ht_clist *funcs;
	ht_clist *libs;
};

/*
 *	CLASS ht_xbe_import_viewer
 */

class ht_xbe_import_viewer: public ht_itext_listbox {
protected:
	ht_format_group *format_group;
	bool grouplib;
	UINT sortby;
/* new */
			void dosort();
public:
			void	init(bounds *b, char *desc, ht_format_group *fg);
	virtual	void	done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	bool select_entry(void *entry);
/* new */
			char *func(UINT i, bool execute);
};

#endif /* !__HTXBEIMP_H__ */
