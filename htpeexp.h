/* 
 *	HT Editor
 *	htpeexp.h
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

#ifndef __HTPEEXP_H__
#define __HTPEEXP_H__

#include "data.h"
#include "htdialog.h"
#include "formats.h"

extern format_viewer_if htpeexports_if;

/*
 *	CLASS ht_pe_export_viewer
 */

class ht_pe_export_viewer: public ht_itext_listbox {
protected:
	ht_format_group *format_group;
public:
			void	init(Bounds *b, ht_format_group *fg);
	virtual	void	done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	bool select_entry(void *entry);
/* new */
		const char *func(uint i, bool execute);
};

/*
 *	CLASS ht_pe_export_function
 */

class ht_pe_export_function: public Object {
public:
	uint ordinal;

	bool byname;
	char *name;
	RVA address;

	ht_pe_export_function(RVA address, uint ordinal);
	ht_pe_export_function(RVA address, uint ordinal, const char *name);
	~ht_pe_export_function();
};

struct ht_pe_export {
	Container *funcs;
};

#endif /* !__HTPEEXP_H__ */
