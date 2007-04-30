/* 
 *	HT Editor
 *	htpeimp.h
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

#ifndef __HTPEIMP_H__
#define __HTPEIMP_H__

#include "formats.h"

extern format_viewer_if htpeimports_if;

/*
 *	class ht_pe_import_library
 */

class ht_pe_import_library: public Object {
public:
	char *name;

	ht_pe_import_library(const char *name);
	~ht_pe_import_library();
};

/*
 *	class ht_pe_import_function
 */

class ht_pe_import_function: public Object {
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

	ht_pe_import_function(uint libidx, RVA address, uint ordinal);
	ht_pe_import_function(uint libidx, RVA address, const char *name, uint hint);
	~ht_pe_import_function();
};

struct ht_pe_import {
	Container *funcs;
	Container *libs;
};

/*
 *	CLASS ht_pe_import_viewer
 */

class ht_pe_import_viewer: public ht_itext_listbox {
protected:
	ht_format_group *format_group;
	bool grouplib;
	uint sortby;
/* new */
			void dosort();
public:
			void	init(Bounds *b, const char *desc, ht_format_group *fg);
	virtual	void	done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	bool select_entry(void *entry);
/* new */
		const char *func(uint i, bool execute);
};

#endif /* !__HTPEIMP_H__ */
