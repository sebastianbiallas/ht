/*
 *	HT Editor
 *	ilopc.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#ifndef ILOPC_H
#define ILOPC_H

#include "io/types.h"

#define MAX_IL_OPCODE 256
#define MAX_IL_OPCODE_PREFIX 36

#define	IL_OPCODE_ARGS_INVALID		0
#define	IL_OPCODE_ARGS_NONE		1
#define	IL_OPCODE_ARGS_INT8		2
#define	IL_OPCODE_ARGS_UINT8		3
#define	IL_OPCODE_ARGS_INT16		4
#define	IL_OPCODE_ARGS_UINT16		5
#define	IL_OPCODE_ARGS_INT32		6
#define	IL_OPCODE_ARGS_INT64		7
#define	IL_OPCODE_ARGS_FLOAT32		8
#define	IL_OPCODE_ARGS_FLOAT64		9
#define	IL_OPCODE_ARGS_TOKEN		10
#define	IL_OPCODE_ARGS_SHORT_VAR	11
#define	IL_OPCODE_ARGS_LONG_VAR		12
#define	IL_OPCODE_ARGS_SHORT_ARG	13
#define	IL_OPCODE_ARGS_LONG_ARG		14
#define	IL_OPCODE_ARGS_SHORT_JUMP	15
#define	IL_OPCODE_ARGS_LONG_JUMP	16
#define	IL_OPCODE_ARGS_CALL		17
#define	IL_OPCODE_ARGS_CALLI		18
#define	IL_OPCODE_ARGS_CALLVIRT		19
#define	IL_OPCODE_ARGS_SWITCH		20
#define	IL_OPCODE_ARGS_STRING		21
#define	IL_OPCODE_ARGS_NEW		22
#define	IL_OPCODE_ARGS_ANN_DATA		23
#define	IL_OPCODE_ARGS_ANN_DATA_S	24
#define	IL_OPCODE_ARGS_ANN_DEAD		25
#define	IL_OPCODE_ARGS_ANN_REF		26
#define	IL_OPCODE_ARGS_ANN_REF_S	27
#define	IL_OPCODE_ARGS_ANN_PHI		28
#define	IL_OPCODE_ARGS_ANN_LIVE		29
#define	IL_OPCODE_ARGS_ANN_ARG		30

#define	IL_OPCODE_ARGS_PREFIX		31

// meta
#define IL_META_TOKEN_MASK              0xff000000
#define IL_META_TOKEN_MODULE            0x00000000
#define IL_META_TOKEN_TYPE_REF          0x01000000
#define IL_META_TOKEN_TYPE_DEF          0x02000000
#define IL_META_TOKEN_FIELD_DEF         0x04000000
#define IL_META_TOKEN_METHOD_DEF        0x06000000
#define IL_META_TOKEN_PARAM_DEF         0x08000000
#define IL_META_TOKEN_INTERFACE_IMPL    0x09000000
#define IL_META_TOKEN_MEMBER_REF        0x0a000000
#define IL_META_TOKEN_CONSTANT          0x0b000000
#define IL_META_TOKEN_CUSTOM_ATTRIBUTE  0x0c000000
#define IL_META_TOKEN_CUSTOM_VALUE      IL_META_TOKEN_CUSTOM_ATTRIBUTE
#define IL_META_TOKEN_FIELD_MARSHAL     0x0d000000
#define IL_META_TOKEN_DECL_SECURITY     0x0e000000
#define IL_META_TOKEN_CLASS_LAYOUT      0x0f000000
#define IL_META_TOKEN_FIELD_LAYOUT      0x10000000
#define IL_META_TOKEN_STAND_ALONE_SIG   0x11000000
#define IL_META_TOKEN_EVENT_MAP         0x12000000
#define IL_META_TOKEN_EVENT             0x14000000
#define IL_META_TOKEN_PROPERTY_MAP      0x15000000
#define IL_META_TOKEN_PROPERTY          0x17000000
#define IL_META_TOKEN_METHOD_SEMANTICS  0x18000000
#define IL_META_TOKEN_METHOD_IMPL       0x19000000
#define IL_META_TOKEN_MODULE_REF        0x1a000000
#define IL_META_TOKEN_TYPE_SPEC         0x1b000000
#define IL_META_TOKEN_IMPL_MAP          0x1c000000
#define IL_META_TOKEN_FIELD_RVA         0x1d000000
#define IL_META_TOKEN_ASSEMBLY          0x20000000
#define IL_META_TOKEN_PROCESSOR_DEF     0x21000000
#define IL_META_TOKEN_OS_DEF            0x22000000
#define IL_META_TOKEN_ASSEMBLY_REF      0x23000000
#define IL_META_TOKEN_PROCESSOR_REF     0x24000000
#define IL_META_TOKEN_OS_REF            0x25000000
#define IL_META_TOKEN_FILE              0x26000000
#define IL_META_TOKEN_EXPORTED_TYPE     0x27000000
#define IL_META_TOKEN_MANIFEST_RESOURCE 0x28000000
#define IL_META_TOKEN_NESTED_CLASS      0x29000000
#define IL_META_TOKEN_STRING            0x70000000
#define IL_META_TOKEN_NAME              0x71000000
#define IL_META_TOKEN_BASE_TYPE         0x72000000


struct ILOpcodeTabEntry {
	const char	*name;
	byte		op;
	byte		size;
};
								
extern ILOpcodeTabEntry il_opcode_table[MAX_IL_OPCODE];
extern ILOpcodeTabEntry il_prefix_opcode_table[MAX_IL_OPCODE_PREFIX];

#endif
 
