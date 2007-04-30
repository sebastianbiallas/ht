/*
 *	HT Editor
 *	types.h
 *
 *	Copyright (C) 1999-2003 Sebastian Biallas (sb@biallas.net)
 *	Copyright (C) 1999-2003 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 *	structure packing
 */

#ifdef __cplusplus
#ifdef __GNUC__

#	define FUNCTION_CONST	__attribute__((const))
#	define PACKED		__attribute__((packed))
#	define UNUSED		__attribute__((unused))
#	define DEPRECATED	__attribute__((deprecated))
#	define NORETURN		__attribute__((noreturn))
#	define ALIGN_STRUCT(n)	__attribute__((aligned(n)))

#else
#error "You are not using the GNU C compiler collection (GCC). Please add the __htpacked macro and conditionals for your compiler"
#define __htpacked
#endif /* !__GNUC__ */
#endif /* __cplusplus */

/*
 *	integers
 */

#include SYSTEM_OSAPI_SPECIFIC_TYPES_HDR

/*
 *	C++ specific
 */

#ifdef __cplusplus

#define DDECL_UINT(name)	uint32 name PACKED
#define DDECL_PTR(type, name)	type *name PACKED

#define NEW_PTR(type, var) type	*var = (type *) malloc(sizeof(type))
#define NEW_OBJECT(instance, class, params...) \
((class *)(instance = new class()))->init(params)
#define DELETE_OBJECT(obj) obj->done(); delete obj;

#define DPRINTF(msg...)


/*
 *	steves strucs
 */

//#define BOUNDS_ASSIGN(b, X, Y, W, H) b.x=X; b.y=Y; b.w=W; b.h=H;

union htmsg_param {
	int integer;
	uint64 q;
	void *ptr;
	char *str;
	const char *cstr;
};

struct htmsg {
	int msg;
	int type;
	htmsg_param data1;
	htmsg_param data2;
};
#endif /* __cplusplus */

#endif
