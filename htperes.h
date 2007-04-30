/* 
 *	HT Editor
 *	htperes.h
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

#ifndef __HTPERES_H__
#define __HTPERES_H__

#include "httree.h"

/*
 *	CLASS ht_pe_resource_viewer
 */

class ht_pe_resource_viewer: public ht_static_treeview {
private:
/* new */
	virtual	Object *vstate_create();
		bool vstate_save();
	virtual	void vstate_restore(Object *d);
public:
		void init(Bounds *b, const char *desc);
	virtual	void done();
/* overwritten */
	virtual	void	handlemsg(htmsg *msg);
	virtual	void	select_node(void *node);
};

extern format_viewer_if htperesources_if;

#endif /* __HTPERES_H__ */
