/* 
 *	HT Editor
 *	htkeyb.h (DJGPP implementation)
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

#ifndef __HTKEYB_H__
#define __HTKEYB_H__

#include "htio.h"

bool ht_keypressed();

ht_key ht_getkey();
int ht_raw_getkey();

/*
 *	INIT
 */
 
bool init_keyb();

/*
 *	DONE
 */
 
void done_keyb();

#endif /* !__HTKEYB_H__ */
