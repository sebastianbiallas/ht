/* 
 *	HT Editor
 *	htatom.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTATOM_H__
#define __HTATOM_H__

#include "htdata.h"

typedef unsigned int HT_ATOM;

void *find_atom(HT_ATOM atom);
HT_ATOM find_atom_rev(const void *data);
bool register_atom(HT_ATOM atom, const void *data);
bool unregister_atom(HT_ATOM atom);

/*
 *	INIT
 */
 
bool init_atom();

/*
 *	DONE
 */

void done_atom();

#endif /* !__HTATOM_H__ */
