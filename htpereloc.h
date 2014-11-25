/*
 *	HT Editor
 *	htpereloc.h
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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

#ifndef __HTPERELOC_H__
#define __HTPERELOC_H__

#include "data.h"
#include "formats.h"
#include "ilstruct.h"

extern format_viewer_if htpereloc_if;

/*
 *	CLASS ht_pe_reloc_viewer
 */

class ht_pe_reloc_viewer: public ht_itext_listbox {
protected:
	ht_format_group *format_group;

public:
			void	init(Bounds *b, const char *desc, ht_format_group *fg);
//	virtual	void	done();

	virtual	void handlemsg(htmsg *msg);
	virtual	bool select_entry(void *entry);
	const char *func(uint i, bool execute);
};

#endif
