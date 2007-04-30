/* 
 *	HT Editor
 *	javaopc.cc
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

#include "javaopc.h"

#include <stdio.h>

#define A	JOPC_TYPE_ATYPE | JOPC_SIZE_SMALL
#define B	JOPC_TYPE_CHAR  | JOPC_SIZE_SMALL
#define U	JOPC_TYPE_BYTE  | JOPC_SIZE_SMALL
#define S	JOPC_TYPE_SHORT | JOPC_SIZE_SMALL
#define Sw	JOPC_TYPE_SIMM  | JOPC_SIZE_VAR
#define Cs	JOPC_TYPE_CONST | JOPC_SIZE_SMALL
#define Cw	JOPC_TYPE_CONST | JOPC_SIZE_WIDE
#define O 	JOPC_TYPE_LOCAL | JOPC_SIZE_VAR
#define Ls	JOPC_TYPE_LABEL | JOPC_SIZE_SMALL
#define Lw	JOPC_TYPE_LABEL | JOPC_SIZE_WIDE

javaopc_insn java_insns[256] = {
// 0x0 - 0xf
{"nop"},
{"aconst_null"},
{"iconst_m1"},
{"iconst_0"},
{"iconst_1"},
{"iconst_2"},
{"iconst_3"},
{"iconst_4"},
{"iconst_5"},
{"lconst_0"},
{"lconst_1"},
{"fconst_0"},
{"fconst_1"},
{"fconst_2"},
{"dconst_0"},
{"dconst_1"},
// 0x10-0x1f
{"bipush", {B}},
{"sipush", {S}},
{"ldc", {Cs}},
{"ldc_w", {Cw}},
{"ldc2_w", {Cw}},
{"iload", {O}},
{"lload", {O}},
{"fload", {O}},
{"dload", {O}},
{"aload", {O}},
{"iload_0"},
{"iload_1"},
{"iload_2"},
{"iload_3"},
{"lload_0"},
{"lload_1"},
// 0x20-0x2f
{"lload_2"},
{"lload_3"},
{"fload_0"},
{"fload_1"},
{"fload_2"},  
{"fload_3"},
{"dload_0"},
{"dload_1"},
{"dload_2"},
{"dload_3"},
{"aload_0"},
{"aload_1"},
{"aload_2"},
{"aload_3"},
{"iaload"},
{"laload"},
// 0x30-0x3f
{"faload"},
{"daload"},
{"aaload"},
{"baload"},
{"caload"},
{"saload"},
{"istore", {O}},
{"lstore", {O}},
{"fstore", {O}},
{"dstore", {O}},
{"astore", {O}},
{"istore_0"},
{"istore_1"},
{"istore_2"},
{"istore_3"},
{"lstore_0"},
// 0x40-0x4f
{"lstore_1"},
{"lstore_2"},
{"lstore_3"},
{"fstore_0"},
{"fstore_1"},
{"fstore_2"},
{"fstore_3"},
{"dstore_0"},
{"dstore_1"},
{"dstore_2"},
{"dstore_3"},
{"astore_0"},
{"astore_1"},
{"astore_2"},
{"astore_3"},
{"iastore"},
// 0x50-0x5f
{"lastore"},
{"fastore"},
{"dastore"},
{"aastore"},
{"bastore"},
{"castore"},
{"sastore"},
{"pop"},
{"pop2"},
{"dup"},
{"dup_x1"},
{"dup_x2"},
{"dup2"},
{"dup2_x1"},
{"dup2_x2"},
{"swap"},
// 0x60-0x6f
{"iadd"},
{"ladd"},
{"fadd"},
{"dadd"},
{"isub"},
{"lsub"},
{"fsub"},
{"dsub"},
{"imul"},
{"lmul"},
{"fmul"},
{"dmul"},
{"idiv"},
{"ldiv"},
{"fdiv"},
{"ddiv"},
// 0x70-0x7f
{"irem"},
{"lrem"},
{"frem"},
{"drem"},
{"ineg"},
{"lneg"},
{"fneg"},
{"dneg"},
{"ishl"},
{"lshl"},
{"ishr"},
{"lshr"},
{"iushr"},
{"lushr"},
{"iand"},
{"land"},
// 0x80-0x8f
{"ior"},
{"lor"},
{"ixor"},
{"lxor"},
{"iinc", {O, Sw}},
{"i2l"},
{"i2f"},
{"i2d"},
{"l2i"},
{"l2f"},
{"l2d"},
{"f2i"},
{"f2l"},
{"f2d"},
{"d2i"},
{"d2l"},
// 0x90-0x9f
{"d2f"},
{"i2b"},
{"i2c"},
{"i2s"},
{"lcmp"},
{"fcmpl"},
{"fcmpg"},
{"dcmpl"},
{"dcmpg"},
{"ifeq", {Ls}},
{"ifne", {Ls}},
{"iflt", {Ls}},
{"ifge", {Ls}},
{"ifgt", {Ls}},
{"ifle", {Ls}},
{"if_icmpeq", {Ls}},
// 0xa0-0xaf
{"if_icmpne", {Ls}},
{"if_icmplt", {Ls}},
{"if_icmpge", {Ls}},
{"if_icmpgt", {Ls}},
{"if_icmple", {Ls}},
{"if_acmpeq", {Ls}},
{"if_acmpne", {Ls}},
{"goto", {Ls}},
{"jsr", {Ls}},
{"ret", {O}},
{"tableswitch"},
{"lookupswitch"},
{"ireturn"},
{"lreturn"},
{"freturn"},
{"dreturn"},
// 0xb0-0xbf
{"areturn"},
{"return"},
{"getstatic", {Cw}},
{"putstatic", {Cw}},
{"getfield", {Cw}},
{"putfield", {Cw}},
{"invokevirtual", {Cw}},
{"invokespecial", {Cw}},
{"invokestatic", {Cw}},
{"invokeinterface", {Cw, B}},
{"xxxunusedxxx"},
{"new", {Cw}},
{"newarray", {A}},
{"anewarray", {Cw}},
{"arraylength"},
{"athrow"},
// 0xc0-0xcf
{"checkcast", {Cw}},
{"instanceof", {Cw}},
{"monitorenter"},
{"monitorexit"},
{"wide"},
{"multianewarray", {Cw, B}},    //?
{"ifnull", {Ls}},
{"ifnonnull", {Ls}},
{"goto_w", {Lw}},
{"jsr_w", {Lw}},
{"breakpoint"},
{"ldc_quick"},
{"ldc_w_quick"},
{"ldc2_w_quick"},
{"getfield_quick"},
{"putfield_quick"},
// 0xd0-0xdf
{"getfield2_quick"},
{"putfield2_quick"},
{"getstatic_quick"},
{"putstatic_quick"},
{"getstatic2_quick"},
{"putstatic2_quick"},
{"invokevirtual_quick"},
{"invokenonvirtual_quick"},
{"invokesuper_quick"},
{"invokestatic_quick"},
{"invokeinterface_quick"},
{"invokevirtualobject_quick"},
{NULL},
{"new_quick"},
{"anewarray_quick"},
{"multianewarray_quick"},
// 0xe0-0xef

// e0-e3
{"checkcast_quick"},
{"instanceof_quick"},
{"invokevirtual_quick_w"},
{"getfield_quick_w"},
// e4-e7
{"putfield_quick_w"}, {NULL}, {NULL}, {NULL},
// e8-eb
{NULL}, {NULL}, {NULL}, {NULL},
// ec-ef
{NULL}, {NULL}, {NULL}, {NULL},

// f0-f3
{NULL}, {NULL}, {NULL}, {NULL},
// f4-f7
{NULL}, {NULL}, {NULL}, {NULL},
// f8-fb
{NULL}, {NULL}, {NULL}, {NULL},
// fc-ff
{NULL}, {NULL},
{"impdep1"},
{"impdep2"}
};

