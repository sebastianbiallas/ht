/*
 *	HT Editor
 *	global.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#ifndef GLOBAL_H
#define GLOBAL_H

/*
 *	packed structures
 */

#ifdef __cplusplus
#ifdef __GNUC__
#define HTPACKED __attribute__ ((packed))
#else
#error "you're not using the GNU C compiler :-) please add the macro and conditionals for your compiler"
#define HTPACKED blabla
#endif /* !__GNUC__ */
#endif /* __cplusplus */

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

#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int

#define sint8 signed char
#define sint16 signed short
#define sint32 signed int

#include "qword.h"

#ifndef NULL
#	define NULL 0
#endif

#ifndef HAVE_FILEOFS
#define HAVE_FILEOFS
typedef UINT FILEOFS;
#endif

/* C++ specific */
#ifdef __cplusplus

#define DDECL_UINT(name)	dword name HTPACKED
#define DDECL_PTR(type, name)	type *name HTPACKED

typedef unsigned int ID;

typedef unsigned int OBJECT_ID;

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
#define DPRINTF(msg...)

#endif /* __cplusplus */

#endif

