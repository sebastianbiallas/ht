/*
 *	HT Editor
 *	global.h
 *
 *	Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@web-productions.de)
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

#ifndef GLOBAL_H
#define GLOBAL_H

/*
 *	packed structures
 */

#ifdef __GNUC__
#define HTPACKED __attribute__ ((packed))
#else
#error this is not the GNU C compiler :-) so you must add your definition of HTPACKED here (global.h)
#define HTPACKED blabla
#endif

/*
 *	types
 */

#ifdef byte
#	undef byte
#endif
#ifdef word
#	undef word
#endif
#ifdef dword
#	undef dword
#endif

typedef unsigned int UINT;

#define byte unsigned char
#define word unsigned short
#define dword unsigned int

#include "qword.h"

#define DDECL_UINT(name)	dword name HTPACKED
#define DDECL_PTR(type, name)	type *name HTPACKED

typedef unsigned int ID;

typedef unsigned int OBJECT_ID;

#ifndef NULL
#	define NULL 0
#endif

#ifndef HAVE_FILEOFS
#define HAVE_FILEOFS
typedef UINT FILEOFS;
#endif

#define STUB
#define ABSTRACT

/*
 *	steves strucs
 */

struct bounds {
	int x, y, w, h;
};

#define BOUNDS_ASSIGN(b, X, Y, W, H) b.x=X; b.y=Y; b.w=W; b.h=H;

union htmsg_param {
	int integer;
	void *ptr;
	char *str;
};

struct htmsg {
	int msg;
	int type;
	htmsg_param data1;
	htmsg_param data2;
};

/*
 *	macros
 */

#define NEW_PTR(type, var) type	*var = (type *) malloc(sizeof(type))
#define NEW_OBJECT(instance, class, params...) \
((class *)(instance = new class()))->init(params)
#define DELETE_OBJECT(obj) obj->done(); delete obj;

#ifdef DEBUG
#define DPRINTF(msg...) printf(msg)
#else
#define DPRINTF(msg...)
#endif

/*
 *	compile time formats
 */

// #define HEX8FORMAT8 "%08x"
// #define HEX8FORMAT "%08x"
#define HEX8FORMAT8 "%8x"
#define HEX8FORMAT "%x"

#endif
