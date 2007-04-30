/*
 *	HT Editor
 *	ilopc.cc
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

#include <cstdio>
#include "io/types.h"
#include "ilopc.h"

ILOpcodeTabEntry il_opcode_table[MAX_IL_OPCODE] = {

	{"nop", IL_OPCODE_ARGS_NONE, 1},
	{"break", IL_OPCODE_ARGS_NONE, 1},

	{"ldarg.0", IL_OPCODE_ARGS_NONE, 1},
	{"ldarg.1", IL_OPCODE_ARGS_NONE, 1},
	{"ldarg.2", IL_OPCODE_ARGS_NONE, 1},
	{"ldarg.3", IL_OPCODE_ARGS_NONE, 1},
	{"ldloc.0", IL_OPCODE_ARGS_NONE, 1},
	{"ldloc.1", IL_OPCODE_ARGS_NONE, 1},
	{"ldloc.2", IL_OPCODE_ARGS_NONE, 1},
	{"ldloc.3", IL_OPCODE_ARGS_NONE, 1},
	{"stloc.0", IL_OPCODE_ARGS_NONE, 1},
	{"stloc.1", IL_OPCODE_ARGS_NONE, 1},
	{"stloc.2", IL_OPCODE_ARGS_NONE, 1},
	{"stloc.3", IL_OPCODE_ARGS_NONE, 1},

	{"ldarg.s", IL_OPCODE_ARGS_SHORT_ARG, 2},
	{"ldarga.s", IL_OPCODE_ARGS_SHORT_ARG, 2},
	{"starg.s", IL_OPCODE_ARGS_SHORT_ARG, 2},
	{"ldloc.s", IL_OPCODE_ARGS_SHORT_VAR, 2},
	{"ldloca.s", IL_OPCODE_ARGS_SHORT_VAR, 2},
	{"stloc.s", IL_OPCODE_ARGS_SHORT_VAR, 2},

	{"ldnull", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.m1", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.0", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.1", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.2", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.3", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.4", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.5", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.6", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.7", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.8", IL_OPCODE_ARGS_NONE, 1},
	{"ldc.i4.s", IL_OPCODE_ARGS_INT8, 2},
	{"ldc.i4", IL_OPCODE_ARGS_INT32, 5},
	{"ldc.i8", IL_OPCODE_ARGS_INT64, 9},
	{"ldc.r4", IL_OPCODE_ARGS_FLOAT32, 5},
	{"ldc.r8", IL_OPCODE_ARGS_FLOAT64, 9},

	{"ldptr", IL_OPCODE_ARGS_INT32, 5},

	{"dup", IL_OPCODE_ARGS_NONE, 1},
	{"pop", IL_OPCODE_ARGS_NONE, 1},

	{"jmp", IL_OPCODE_ARGS_CALL, 5},
	{"call", IL_OPCODE_ARGS_CALL, 5},
	{"calli", IL_OPCODE_ARGS_CALLI, 5},
	{"ret", IL_OPCODE_ARGS_NONE, 1},

	{"br.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"brfalse.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"brtrue.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"beq.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"bge.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"bgt.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"ble.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"blt.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"bne.un.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"bge.un.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"bgt.un.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"ble.un.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"blt.un.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},
	{"br", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"brfalse", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"brtrue", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"beq", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"bge", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"bgt", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"ble", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"blt", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"bne.un", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"bge.un", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"bgt.un", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"ble.un", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"blt.un", IL_OPCODE_ARGS_LONG_JUMP, 5},

	{"switch", IL_OPCODE_ARGS_SWITCH, 5}, // must be at least 5 bytes long

	{"ldind.i1", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.u1", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.i2", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.u2", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.i4", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.u4", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.i8", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.i", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.r4", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.r8", IL_OPCODE_ARGS_NONE, 1},
	{"ldind.ref", IL_OPCODE_ARGS_NONE, 1},
	{"stind.ref", IL_OPCODE_ARGS_NONE, 1},
	{"stind.i1", IL_OPCODE_ARGS_NONE, 1},
	{"stind.i2", IL_OPCODE_ARGS_NONE, 1},
	{"stind.i4", IL_OPCODE_ARGS_NONE, 1},
	{"stind.i8", IL_OPCODE_ARGS_NONE, 1},
	{"stind.r4", IL_OPCODE_ARGS_NONE, 1},
	{"stind.r8", IL_OPCODE_ARGS_NONE, 1},

	{"add", IL_OPCODE_ARGS_NONE, 1},
	{"sub", IL_OPCODE_ARGS_NONE, 1},
	{"mul", IL_OPCODE_ARGS_NONE, 1},
	{"div", IL_OPCODE_ARGS_NONE, 1},
	{"div.un", IL_OPCODE_ARGS_NONE, 1},
	{"rem", IL_OPCODE_ARGS_NONE, 1},
	{"rem.un", IL_OPCODE_ARGS_NONE, 1},
	{"and", IL_OPCODE_ARGS_NONE, 1},
	{"or", IL_OPCODE_ARGS_NONE, 1},
	{"xor", IL_OPCODE_ARGS_NONE, 1},
	{"shl", IL_OPCODE_ARGS_NONE, 1},
	{"shr", IL_OPCODE_ARGS_NONE, 1},
	{"shr.un", IL_OPCODE_ARGS_NONE, 1},
	{"neg", IL_OPCODE_ARGS_NONE, 1},
	{"not", IL_OPCODE_ARGS_NONE, 1},

	{"conv.i1", IL_OPCODE_ARGS_NONE, 1},
	{"conv.i2", IL_OPCODE_ARGS_NONE, 1},
	{"conv.i4", IL_OPCODE_ARGS_NONE, 1},
	{"conv.i8", IL_OPCODE_ARGS_NONE, 1},
	{"conv.r4", IL_OPCODE_ARGS_NONE, 1},
	{"conv.r8", IL_OPCODE_ARGS_NONE, 1},
	{"conv.u4", IL_OPCODE_ARGS_NONE, 1},
	{"conv.u8", IL_OPCODE_ARGS_NONE, 1},

	{"callvirt", IL_OPCODE_ARGS_CALLVIRT, 5},
	{"cpobj", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldobj", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldstr", IL_OPCODE_ARGS_STRING, 5},

	{"newobj", IL_OPCODE_ARGS_NEW, 5},
	{"castclass", IL_OPCODE_ARGS_TOKEN, 5},
	{"isinst", IL_OPCODE_ARGS_TOKEN, 5},

	{"conv.r.un", IL_OPCODE_ARGS_NONE, 1},
	{"ann.data.s", IL_OPCODE_ARGS_ANN_DATA_S, 2}, // // must be at least 2 bytes long

	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0x78

	{"unbox", IL_OPCODE_ARGS_TOKEN, 5},

	{"throw", IL_OPCODE_ARGS_NONE, 1},

	{"ldfld", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldflda", IL_OPCODE_ARGS_TOKEN, 5},
	{"stfld", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldsfld", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldsflda", IL_OPCODE_ARGS_TOKEN, 5},
	{"stsfld", IL_OPCODE_ARGS_TOKEN, 5},
	{"stobj", IL_OPCODE_ARGS_TOKEN, 5},

	{"conv.ovf.i1.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i2.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i4.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i8.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u1.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u2.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u4.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u8.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i.un", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u.un", IL_OPCODE_ARGS_NONE, 1},

	{"box", IL_OPCODE_ARGS_TOKEN, 5},

	{"newarr", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldlen", IL_OPCODE_ARGS_NONE, 1},
	{"ldelema", IL_OPCODE_ARGS_TOKEN, 5},
	{"ldelem.i1", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.u1", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.i2", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.u2", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.i4", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.u4", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.i8", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.i", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.r4", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.r8", IL_OPCODE_ARGS_NONE, 1},
	{"ldelem.ref", IL_OPCODE_ARGS_NONE, 1},

	{"stelem.i", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.i1", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.i2", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.i4", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.i8", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.r4", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.r8", IL_OPCODE_ARGS_NONE, 1},
	{"stelem.ref", IL_OPCODE_ARGS_NONE, 1},

	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xa3
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xb2

	{"conv.ovf.i1", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u1", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i2", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u2", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i4", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u4", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i8", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u8", IL_OPCODE_ARGS_NONE, 1},

	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xbb
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xc1

	{"refanyval", IL_OPCODE_ARGS_TOKEN, 5},
	{"ckfinite", IL_OPCODE_ARGS_NONE, 1},

	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xc4
	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xc5

	{"mkrefany", IL_OPCODE_ARGS_TOKEN, 5},

	{"ann.call", IL_OPCODE_ARGS_TOKEN, 5},
	{"ann.catch", IL_OPCODE_ARGS_NONE, 1},
	{"ann.dead", IL_OPCODE_ARGS_ANN_DEAD, 3},
	{"ann.hoisted", IL_OPCODE_ARGS_NONE, 1},
	{"ann.hoisted_call", IL_OPCODE_ARGS_TOKEN, 5},
	{"ann.lab", IL_OPCODE_ARGS_NONE, 1},
	{"ann.def", IL_OPCODE_ARGS_NONE, 1},
	{"ann.ref.s", IL_OPCODE_ARGS_ANN_REF_S, 2},
	{"ann.phi", IL_OPCODE_ARGS_ANN_PHI, 3}, // must be at least 3 bytes long

	{"ldtoken", IL_OPCODE_ARGS_TOKEN, 5},

	{"conv.u2", IL_OPCODE_ARGS_NONE, 1},
	{"conv.u1", IL_OPCODE_ARGS_NONE, 1},
	{"conv.i", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.i", IL_OPCODE_ARGS_NONE, 1},
	{"conv.ovf.u", IL_OPCODE_ARGS_NONE, 1},

	{"add.ovf", IL_OPCODE_ARGS_NONE, 1},
	{"add.ovf.un", IL_OPCODE_ARGS_NONE, 1},
	{"mul.ovf", IL_OPCODE_ARGS_NONE, 1},
	{"mul.ovf.un", IL_OPCODE_ARGS_NONE, 1},
	{"sub.ovf", IL_OPCODE_ARGS_NONE, 1},
	{"sub.ovf.un", IL_OPCODE_ARGS_NONE, 1},

	{"endfinally", IL_OPCODE_ARGS_NONE, 1},

	{"leave", IL_OPCODE_ARGS_LONG_JUMP, 5},
	{"leave.s", IL_OPCODE_ARGS_SHORT_JUMP, 2},

	{"stdind.i", IL_OPCODE_ARGS_NONE, 1},

	{"conv.u", IL_OPCODE_ARGS_NONE, 1},

	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xe1
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xef
	
	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xf0
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1},
	{NULL, IL_OPCODE_ARGS_INVALID, 1}, // 0xfd

	{NULL, IL_OPCODE_ARGS_PREFIX, 3}, // special: prefix

	{NULL, IL_OPCODE_ARGS_INVALID, 1},
};

ILOpcodeTabEntry il_prefix_opcode_table[MAX_IL_OPCODE_PREFIX] = {
	{"arglist", IL_OPCODE_ARGS_NONE, 2},

	{"ceq", IL_OPCODE_ARGS_NONE, 2},
	{"cgt", IL_OPCODE_ARGS_NONE, 2},
	{"cgt.un", IL_OPCODE_ARGS_NONE, 2},
	{"clt", IL_OPCODE_ARGS_NONE, 2},
	{"clt.un", IL_OPCODE_ARGS_NONE, 2},

	{"ldftn", IL_OPCODE_ARGS_TOKEN, 6},
	{"ldvirtftn", IL_OPCODE_ARGS_TOKEN, 6},

	{"jmpi", IL_OPCODE_ARGS_CALLI, 6},

	{"ldarg", IL_OPCODE_ARGS_LONG_VAR, 4},
	{"ldarga", IL_OPCODE_ARGS_LONG_VAR, 4},
	{"starg", IL_OPCODE_ARGS_LONG_VAR, 4},
	{"ldloc", IL_OPCODE_ARGS_LONG_VAR, 4},
	{"ldloca", IL_OPCODE_ARGS_LONG_VAR, 4},
	{"stloc", IL_OPCODE_ARGS_LONG_VAR, 4},

	{"localloc", IL_OPCODE_ARGS_NONE, 2},

	{NULL, IL_OPCODE_ARGS_INVALID, 2}, // 0x10

	{"endfilter", IL_OPCODE_ARGS_NONE, 2},
	{"unaligned.", IL_OPCODE_ARGS_UINT8, 3},
	{"volatile.", IL_OPCODE_ARGS_NONE, 2},
	{"tail.", IL_OPCODE_ARGS_NONE, 2},
	{"initobj", IL_OPCODE_ARGS_TOKEN, 6},
	{"ann.live", IL_OPCODE_ARGS_ANN_LIVE, 4},
	{"cpblk", IL_OPCODE_ARGS_NONE, 2},
	{"initblk", IL_OPCODE_ARGS_NONE, 2},
	{"ann.ref", IL_OPCODE_ARGS_ANN_REF, 4},
	{"rethrow", IL_OPCODE_ARGS_NONE, 2},

	{NULL, IL_OPCODE_ARGS_INVALID, 2}, // 0x1b

	{"sizeof", IL_OPCODE_ARGS_TOKEN, 6},
	{"refanytype", IL_OPCODE_ARGS_NONE, 2},

	{NULL, IL_OPCODE_ARGS_INVALID, 2}, // 0x1e
	{NULL, IL_OPCODE_ARGS_INVALID, 2}, // 0x1f
	{NULL, IL_OPCODE_ARGS_INVALID, 2}, // 0x20
	{NULL, IL_OPCODE_ARGS_INVALID, 2}, // 0x21

	{"ann.data", IL_OPCODE_ARGS_ANN_DATA, 6}, // must be at least 6 bytes long
	{"ann.arg", IL_OPCODE_ARGS_ANN_ARG, 4},
};
