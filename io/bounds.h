/* 
 *	HT Editor
 *	bounds.h - Bounds for rectangular objects
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

#ifndef __BOUNDS_H__
#define __BOUNDS_H__

#include "types.h"

/**
 *	Rectangular visual Bounds
 */
struct Bounds {
	int x, y, w, h;

	Bounds() {}

	Bounds(const Bounds &b)
	{ x = b.x; y = b.y; w = b.w; h = b.h; }

	Bounds(int X, int Y, int W, int H)
	{ x = X; y = Y; w = W; h = H; }

	inline void assign(int X, int Y, int W, int H)
	{ x = X; y = Y; w = W; h = H; }

	inline void move(int deltax, int deltay)
	{ x += deltax; y += deltay; }

	inline void resize(int deltaw, int deltah)
	{ w += deltaw; h += deltah; }

	inline void intersectWith(const Bounds &b)
	{
		if (b.x > x) {
			w -= b.x-x;
			x = b.x;
		}
		if (b.y > y) {
			h -= b.y-y;
			y = b.y;
		}
		if (x+w > b.x+b.w) w -= x+w-b.x-b.w;
		if (y+h > b.y+b.h) h -= y+h-b.y-b.h;
		if (w < 0) w = 0;
		if (h < 0) h = 0;
	}

	inline bool containsPoint(int ax, int ay)
	{
		return (ax >= x) && (ax < x+w) && (ay >= y) && (ay < y+h);
	}
};

#endif /* __BOUNDS_H__ */
