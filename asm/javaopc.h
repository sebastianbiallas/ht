/* 
 *	HT Editor
 *	javaopc.h
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

#ifndef __JAVAOPC_H__
#define __JAVAOPC_H__

#include "io/types.h"

#define JAVA_OPTYPE_EMPTY	0
#define JAVA_OPTYPE_IMM		1
#define JAVA_OPTYPE_DBL		2
#define JAVA_OPTYPE_REF		3
#define JAVA_OPTYPE_LABEL	4
#define JAVA_OPTYPE_CONST	5
#define JAVA_OPTYPE_ATYPE	6

#define JOPC_TYPE(t)		((t) & 0x1f)
#define JOPC_TYPE_EMPTY		0
#define JOPC_TYPE_BYTE		1
#define JOPC_TYPE_SHORT		2
#define JOPC_TYPE_SIMM		3
#define JOPC_TYPE_CHAR		4
#define JOPC_TYPE_CONST		5
#define JOPC_TYPE_LOCAL		6
#define JOPC_TYPE_LABEL		7
#define JOPC_TYPE_ATYPE		8

#define JOPC_SIZE(t)		((t) & 0xe0)
#define JOPC_SIZE_VAR		0x00
#define JOPC_SIZE_SMALL		0x40
#define JOPC_SIZE_WIDE		0x80

#define JAVAINSN_MAX_PARAM_COUNT	3

struct java_insn_op {
	int type;
	int size;
	union {
		uint32 imm;
		uint32 label;
		double dbl;
		uint32 ref;
	};
};

struct javaopc_insn {
	const char *name;
	int optype[JAVAINSN_MAX_PARAM_COUNT];
};

#define JAVA_WIDE_OPCODE 0xc4
extern javaopc_insn java_insns[256];

#endif /* __JAVAOPC_H__ */

