/*
 *	HT Editor
 *	x86opc.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
 *	Copyright (C) 2005-2011 Sebastian Biallas (sb@biallas.net)
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

#include "x86opc.h"


x86opc_insn_op x86_op_type[] = {

#define _0	0
{TYPE_0, 0, 0, SIZE_0},
#define _064	_0+1
{TYPE_0, 0, INFO_DEFAULT_64, SIZE_0},

#define Ap	_064+1
{TYPE_A, 0, 0, SIZE_P},
#define Cd	Ap+1
{TYPE_C, 0, INFO_DEFAULT_64, SIZE_D},
#define Dd	Cd+1
{TYPE_D, 0, 0, SIZE_D},
#define Eb	Dd+1
{TYPE_E, 0, 0, SIZE_B},
#define Ew	Eb+1
{TYPE_E, 0, 0, SIZE_W},
#define Ed	Ew+1
{TYPE_E, 0, 0, SIZE_D},
#define Eq	Ed+1
{TYPE_E, 0, 0, SIZE_Q},
#define Er	Eq+1
{TYPE_E, 0, 0, SIZE_R},
#define Ev	Er+1
{TYPE_E, 0, 0, SIZE_V},
#define Ev64	Ev+1
{TYPE_E, 0, INFO_DEFAULT_64, SIZE_V},
#define Gb	Ev64+1
{TYPE_G, 0, 0, SIZE_B},
#define Gw	Gb+1
{TYPE_G, 0, 0, SIZE_W},
#define Gv	Gw+1
{TYPE_G, 0, 0, SIZE_V},
#define Gv64	Gv+1
{TYPE_G, 0, INFO_DEFAULT_64, SIZE_V},
#define Gd	Gv64+1
{TYPE_G, 0, 0, SIZE_D},
#define Gr	Gd+1
{TYPE_G, 0, 0, SIZE_R},
#define Gq	Gr+1
{TYPE_G, 0, 0, SIZE_Q},
#define Ib	Gq+1
{TYPE_I, 0, 0, SIZE_B},
#define Iw	Ib+1
{TYPE_I, 0, 0, SIZE_W},
#define Id	Iw+1
{TYPE_I, 0, 0, SIZE_D},
#define Iv	Id+1
{TYPE_I, 0, 0, SIZE_VV},
#define Iv64	Iv+1
{TYPE_I, 0, INFO_DEFAULT_64, SIZE_VV},
#define Ivq	Iv64+1
{TYPE_I, 0, 0, SIZE_V},
#define Ibv	Ivq+1
{TYPE_I, 0, 0, SIZE_BV},
#define sIbv	Ibv+1
{TYPE_Is,0, 0, SIZE_BV},
#define sIbv64	sIbv+1
{TYPE_Is,0, INFO_DEFAULT_64, SIZE_BV},
#define I4	sIbv64+1
{TYPE_I4, 0, 0, SIZE_B},
#define Jb	I4+1
{TYPE_J, 0, 0, SIZE_B},
#define Jv	Jb+1
{TYPE_J, 0, 0, SIZE_VV},
#define M	Jv+1
{TYPE_M, 0, 0, SIZE_0},
#define Mw	M+1
{TYPE_M, 0, 0, SIZE_W},
#define Md	Mw+1
{TYPE_M, 0, 0, SIZE_D},
#define Mp	Md+1
{TYPE_M, 0, 0, SIZE_P},
#define Mq	Mp+1
{TYPE_M, 0, 0, SIZE_Q},
#define Mv	Mq+1
{TYPE_M, 0, 0, SIZE_V},
#define Mr	Mv+1
{TYPE_M, 0, 0, SIZE_R},
#define Mo	Mr+1
{TYPE_M, 0, 0, SIZE_O},
#define Ms	Mo+1
{TYPE_M, 0, 0, SIZE_S},
#define Ml	Ms+1
{TYPE_M, 0, 0, SIZE_L},
#define Mt	Ml+1
{TYPE_M, 0, 0, SIZE_T},
#define Ma	Mt+1
{TYPE_M, 0, 0, SIZE_A},
#define Mu	Ma+1
{TYPE_M, 0, 0, SIZE_U},
#define My	Mu+1
{TYPE_M, 0, 0, SIZE_Y},
#define MRbr	My+1
{TYPE_MR, SIZE_R, 0, SIZE_B},
#define MRwr	MRbr+1
{TYPE_MR, SIZE_R, 0, SIZE_W},
#define MRwv	MRwr+1
{TYPE_MR, SIZE_V, 0, SIZE_W},
#define MRdr	MRwv+1
{TYPE_MR, SIZE_R, 0, SIZE_D},
#define MRbd	MRdr+1
{TYPE_MR, SIZE_D, 0, SIZE_B},
#define Ob	MRbd+1
{TYPE_O, 0, 0, SIZE_B},
#define Ov	Ob+1
{TYPE_O, 0, 0, SIZE_V},
#define Pd	Ov+1
{TYPE_P, 0, 0, SIZE_D},
#define Pq	Pd+1
{TYPE_P, 0, 0, SIZE_Q},
#define Pu	Pq+1
{TYPE_P, 0, 0, SIZE_U},
#define PRq	Pu+1
{TYPE_PR, 0, 0, SIZE_Q},
#define PRu	PRq+1
{TYPE_PR, 0, 0, SIZE_U},
#define Qd	PRu+1
{TYPE_Q, 0, 0, SIZE_D},
#define Qq	Qd+1
{TYPE_Q, 0, 0, SIZE_Q},
#define Qu	Qq+1
{TYPE_Q, 0, 0, SIZE_U},
#define Qz	Qu+1
{TYPE_Q, 0, 0, SIZE_Z},
#define Rw	Qz+1
{TYPE_R, 0, 0, SIZE_W},
#define Rd	Rw+1
{TYPE_R, 0, 0, SIZE_D},
#define Rq	Rd+1
{TYPE_R, 0, 0, SIZE_Q},
#define Rr	Rq+1
{TYPE_R, 0, 0, SIZE_R},
#define Rr64	Rr+1
{TYPE_R, 0, INFO_DEFAULT_64, SIZE_R},
#define Rv	Rr64+1
{TYPE_R, 0, 0, SIZE_V},
#define RVw	Rv+1
{TYPE_RV, 0, 0, SIZE_W},
#define RVd	RVw+1
{TYPE_RV, 0, 0, SIZE_D},
#define RVq	RVd+1
{TYPE_RV, 0, 0, SIZE_Q},
#define Sw	RVq+1
{TYPE_S, 0, 0, SIZE_W},

#define Vd	Sw+1
{TYPE_V, 0, 0, SIZE_D},
#define Vq	Vd+1
{TYPE_V, 0, 0, SIZE_Q},
#define Vo	Vq+1
{TYPE_V, 0, 0, SIZE_O},
#define Vu	Vo+1
{TYPE_V, 0, 0, SIZE_U},
#define Vz	Vu+1
{TYPE_V, 0, 0, SIZE_Z},
#define VRq	Vz+1
{TYPE_VR, 0, 0, SIZE_Q},
#define VRo	VRq+1
{TYPE_VR, 0, 0, SIZE_O},
#define VVo	VRo+1
{TYPE_VV, 0, 0, SIZE_O},
#define VIo	VVo+1
{TYPE_VI, 0, 0, SIZE_O},

#define Wb	VIo+1
{TYPE_W, 0, 0, SIZE_W},
#define Ww	Wb+1
{TYPE_W, 0, 0, SIZE_W},
#define Wd	Ww+1
{TYPE_W, 0, 0, SIZE_D},
#define Wq	Wd+1
{TYPE_W, 0, 0, SIZE_Q},
#define Wo	Wq+1
{TYPE_W, 0, 0, SIZE_O},
#define Wu	Wo+1
{TYPE_W, 0, 0, SIZE_U},
#define Wz	Wu+1
{TYPE_W, 0, 0, SIZE_Z},

#define Yy	Wz+1
{TYPE_Y, 0, 0, SIZE_Y},
#define YVy	Yy+1
{TYPE_YV, 0, 0, SIZE_Y},
#define YIy	YVy+1
{TYPE_YI, 0, 0, SIZE_Y},
#define YRy	YIy+1
{TYPE_YR, 0, 0, SIZE_Y},
#define Xy	YRy+1
{TYPE_X, 0, 0, SIZE_Y},
#define Xd	Xy+1
{TYPE_X, 0, 0, SIZE_D},
#define Xq	Xd+1
{TYPE_X, 0, 0, SIZE_Q},
#define Xo	Xq+1
{TYPE_X, 0, 0, SIZE_O},

#define VD	Xo+1
{TYPE_VD, 0, 0, SIZE_O},
#define VS0d	VD+1
{TYPE_VS, 0, 0, SIZE_D},
#define VS0q	VS0d+1
{TYPE_VS, 0, 0, SIZE_Q},
#define VS0o	VS0q+1
{TYPE_VS, 0, 0, SIZE_Q},
#define VS00d	VS0o+1
{TYPE_VS, 0, 1, SIZE_Q},
#define VS00q	VS00d+1
{TYPE_VS, 0, 1, SIZE_Q},
#define VS00o	VS00q+1
{TYPE_VS, 0, 1, SIZE_O},
#define VS1d	VS00o+1
{TYPE_VS, 1, 0, SIZE_D},
#define VS1q	VS1d+1
{TYPE_VS, 1, 0, SIZE_Q},
#define VS1o	VS1q+1
{TYPE_VS, 1, 0, SIZE_O},

#define Ft	VS1o+1
{TYPE_F, 0, 0, SIZE_T},

#define __st	Ft+1
{TYPE_Fx, 0, 0, SIZE_T},

#define __1	__st+1
{TYPE_Ix, 1, 0, SIZE_B},
#define __3	__1+1
{TYPE_Ix, 3, 0, SIZE_B},

#define X__al	__3+1
{TYPE_RXx, 0, 0, SIZE_B},
#define X__cl	X__al+1
{TYPE_RXx, 1, 0, SIZE_B},

#define __al	X__cl+1
{TYPE_Rx, 0, 0, SIZE_B},
#define __cl	__al+1
{TYPE_Rx, 1, 0, SIZE_B},
#define __dl	__cl+1
{TYPE_Rx, 2, 0, SIZE_B},
#define __bl	__dl+1
{TYPE_Rx, 3, 0, SIZE_B},
#define __ah	__bl+1
{TYPE_Rx, 4, 0, SIZE_B},
#define __ch	__ah+1
{TYPE_Rx, 5, 0, SIZE_B},
#define __dh	__ch+1
{TYPE_Rx, 6, 0, SIZE_B},
#define __bh	__dh+1
{TYPE_Rx, 7, 0, SIZE_B},

#define X__ax	__bh+1
{TYPE_RXx, 0, 0, SIZE_V},

#define __ax	X__ax+1
{TYPE_Rx, 0, 0, SIZE_V},
#define __cx	__ax+1
{TYPE_Rx, 1, 0, SIZE_V},
#define __dx	__cx+1
{TYPE_Rx, 2, 0, SIZE_V},
#define __bx	__dx+1
{TYPE_Rx, 3, 0, SIZE_V},
#define __sp	__bx+1
{TYPE_Rx, 4, 0, SIZE_V},
#define __bp	__sp+1
{TYPE_Rx, 5, 0, SIZE_V},
#define __si	__bp+1
{TYPE_Rx, 6, 0, SIZE_V},
#define __di	__si+1
{TYPE_Rx, 7, 0, SIZE_V},

#define __ax64	__di+1
{TYPE_Rx, 0, INFO_DEFAULT_64, SIZE_V},
#define __cx64	__ax64+1
{TYPE_Rx, 1, INFO_DEFAULT_64, SIZE_V},
#define __dx64	__cx64+1
{TYPE_Rx, 2, INFO_DEFAULT_64, SIZE_V},
#define __bx64	__dx64+1
{TYPE_Rx, 3, INFO_DEFAULT_64, SIZE_V},
#define __sp64	__bx64+1
{TYPE_Rx, 4, INFO_DEFAULT_64, SIZE_V},
#define __bp64	__sp64+1
{TYPE_Rx, 5, INFO_DEFAULT_64, SIZE_V},
#define __si64	__bp64+1
{TYPE_Rx, 6, INFO_DEFAULT_64, SIZE_V},
#define __di64	__si64+1
{TYPE_Rx, 7, INFO_DEFAULT_64, SIZE_V},

#define X__axw	__di64+1
{TYPE_RXx, 0, 0, SIZE_W},
#define X__dxw	X__axw+1
{TYPE_RXx, 2, 0, SIZE_W},

#define __axdq	X__dxw+1
{TYPE_Rx, 0, 0, SIZE_R},
#define __cxdq	__axdq+1
{TYPE_Rx, 1, 0, SIZE_R},
#define __dxdq	__cxdq+1
{TYPE_Rx, 2, 0, SIZE_R},
#define __bxdq	__dxdq+1
{TYPE_Rx, 3, 0, SIZE_R},
#define __spdq	__bxdq+1
{TYPE_Rx, 4, 0, SIZE_R},
#define __bpdq	__spdq+1
{TYPE_Rx, 5, 0, SIZE_R},
#define __sidq	__bpdq+1
{TYPE_Rx, 6, 0, SIZE_R},
#define __didq	__sidq+1
{TYPE_Rx, 7, 0, SIZE_R},

#define __es	__didq+1
{TYPE_Sx, 0, 0, SIZE_W},
#define __cs	__es+1
{TYPE_Sx, 1, 0, SIZE_W},
#define __ss	__cs+1
{TYPE_Sx, 2, 0, SIZE_W},
#define __ds	__ss+1
{TYPE_Sx, 3, 0, SIZE_W},
#define __fs	__ds+1
{TYPE_Sx, 4, 0, SIZE_W},
#define __gs	__fs+1
{TYPE_Sx, 5, 0, SIZE_W},

#define __st0	__gs+1
{TYPE_F, 0, 0, SIZE_T},
#define __st1	__st0+1
{TYPE_F, 1, 0, SIZE_T},
#define __st2	__st1+1
{TYPE_F, 2, 0, SIZE_T},
#define __st3	__st2+1
{TYPE_F, 3, 0, SIZE_T},
#define __st4	__st3+1
{TYPE_F, 4, 0, SIZE_T},
#define __st5	__st4+1
{TYPE_F, 5, 0, SIZE_T},
#define __st6	__st5+1
{TYPE_F, 6, 0, SIZE_T},
#define __st7	__st6+1
{TYPE_F, 7, 0, SIZE_T},

#define XMM0	__st7+1
{TYPE_Vx, 0, 0, SIZE_O},
};

const char *x86_regs[4][8] = {
{"al",  "cl",  "dl",  "bl",  "ah",  "ch",  "dh",  "bh"},
{"ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di"},
{"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"},
{"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"},
};

const char *x86_64regs[4][16] = {
{"al",  "cl",  "dl",   "bl",   "spl",  "bpl",  "sil",  "dil",
 "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"},
{"ax",  "cx",  "dx",   "bx",   "sp",   "bp",   "si",   "di",
 "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"},
{"eax", "ecx", "edx",  "ebx",  "esp",  "ebp",  "esi",  "edi",
 "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"},
{"rax", "rcx", "rdx",  "rbx",  "rsp",  "rbp",  "rsi",  "rdi",
 "r8",  "r9",  "r10",  "r11",  "r12",  "r13",  "r14",  "r15"},
};

const char *x86_ipregs[4] = {
 "", "ip", "eip", "rip",
};

const char *x86_segs[8] = {
"es", "cs", "ss", "ds", "fs", "gs", 0, 0
};

#define GROUP_OPC_0F38		0
#define GROUP_OPC_660F38	1
#define GROUP_OPC_F20F38	2
#define GROUP_OPC_0F3A		3
#define GROUP_OPC_660F3A	4
#define GROUP_OPC_0F7A		5
#define GROUP_OPC_0F7B		6
#define GROUP_OPC_0F24		7
#define GROUP_OPC_0F25		8

#define GROUP_80		0
#define GROUP_81		1
#define GROUP_83		2
#define GROUP_8F		3
#define GROUP_C0		4
#define GROUP_C1		5
#define GROUP_C6		6
#define GROUP_C7                7
#define GROUP_D0		8
#define GROUP_D1		9
#define GROUP_D2		10
#define GROUP_D3		11
#define GROUP_F6		12
#define GROUP_F7		13
#define GROUP_FE		14
#define GROUP_FF		15
#define GROUP_EXT_00		16
#define GROUP_EXT_01		17
#define GROUP_EXT_18		18
#define GROUP_EXT_71		19
#define GROUP_EXT_72		20
#define GROUP_EXT_73		21
#define GROUP_EXT_AE		22
#define GROUP_EXT_F3_AE		23
#define GROUP_EXT_BA		24
#define GROUP_EXT_C7		25
#define GROUP_EXT_66_C7		26
#define GROUP_EXT_F3_C7		27

#define GROUP_SPECIAL_0F01_0	0
#define GROUP_SPECIAL_0F01_1	1
#define GROUP_SPECIAL_0F01_2	2
#define GROUP_SPECIAL_0F01_3	3
#define GROUP_SPECIAL_0F01_7	4
#define GROUP_SPECIAL_0FAE_0	5
#define GROUP_SPECIAL_0FAE_1	6
#define GROUP_SPECIAL_0FAE_2	7
#define GROUP_SPECIAL_0FAE_3	8
#define GROUP_SPECIAL_0FAE_4	9
#define GROUP_SPECIAL_0FAE_5	10
#define GROUP_SPECIAL_0FAE_6	11
#define GROUP_SPECIAL_0FAE_7	12
#define GROUP_SPECIAL_0FC7_6	13

#define GROUP_0FAE		0
#define GROUP_660F71		1
#define GROUP_660F72		2
#define GROUP_660F73		3
#define GROUP_0F25_12		4
#define GROUP_0F25_12_L		5
#define GROUP_0F25_12_W		6
#define GROUP_0FA_12		7
#define GROUP_0FA_12_L		8
#define GROUP_0FA_12_W		9
#define GROUP_0F38_F3		10
#define GROUP_0F38_F3_W		11

/*

	Opcode name modifiers 
	~ ambigous size (need explicit "size ptr")
	
	first char after ~:
	|    alternative mnemonics with same semantic (|je|jz)
	?    different name depending on opsize    (?16bit|32bit|64bit)
	*    different name depending on addrsize  (*16bit|32bit|64bit)
	&    same as '?' for disassembler and '|' for assembler
 */

x86opc_insn x86_les = {"les", {Gv, Mp}};
x86opc_insn x86_lds = {"lds", {Gv, Mp}};
x86opc_insn x86_pop_group = {0, {SPECIAL_TYPE_GROUP, GROUP_8F}};

x86opc_insn x86_32_insns[256] = {
/* 00 */
{"add", {Eb, Gb}},
{"add", {Ev, Gv}},
{"add", {Gb, Eb}},
{"add", {Gv, Ev}},
{"add", {X__al, Ib}},
{"add", {X__ax, Iv}},
{"push", {__es}},
{"pop", {__es}},
/* 08 */
{"or", {Eb, Gb}},
{"or", {Ev, Gv}},
{"or", {Gb, Eb}},
{"or", {Gv, Ev}},
{"or", {X__al, Ib}},
{"or", {X__ax, Iv}},
{"push", {__cs}},
{0, {SPECIAL_TYPE_PREFIX}},
/* 10 */
{"adc", {Eb, Gb}},
{"adc", {Ev, Gv}},
{"adc", {Gb, Eb}},
{"adc", {Gv, Ev}},
{"adc", {X__al, Ib}},
{"adc", {X__ax, Iv}},
{"push", {__ss}},
{"pop", {__ss}},
/* 18 */
{"sbb", {Eb, Gb}},
{"sbb", {Ev, Gv}},
{"sbb", {Gb, Eb}},
{"sbb", {Gv, Ev}},
{"sbb", {X__al, Ib}},
{"sbb", {X__ax, Iv}},
{"push", {__ds}},
{"pop", {__ds}},
/* 20 */
{"and", {Eb, Gb}},
{"and", {Ev, Gv}},
{"and", {Gb, Eb}},
{"and", {Gv, Ev}},
{"and", {X__al, Ib}},
{"and", {X__ax, Iv}},
{0, {SPECIAL_TYPE_PREFIX}},		/* es-prefix */
{"daa"},
/* 28 */
{"sub", {Eb, Gb}},
{"sub", {Ev, Gv}},
{"sub", {Gb, Eb}},
{"sub", {Gv, Ev}},
{"sub", {X__al, Ib}},
{"sub", {X__ax, Iv}},
{0, {SPECIAL_TYPE_PREFIX}},		/* cs-prefix */
{"das"},
/* 30 */
{"xor", {Eb, Gb}},
{"xor", {Ev, Gv}},
{"xor", {Gb, Eb}},
{"xor", {Gv, Ev}},
{"xor", {X__al, Ib}},
{"xor", {X__ax, Iv}},
{0, {SPECIAL_TYPE_PREFIX}},		/* ss-prefix */
{"aaa"},
/* 38 */
{"cmp", {Eb, Gb}},
{"cmp", {Ev, Gv}},
{"cmp", {Gb, Eb}},
{"cmp", {Gv, Ev}},
{"cmp", {X__al, Ib}},
{"cmp", {X__ax, Iv}},
{0, {SPECIAL_TYPE_PREFIX}},		/* ds-prefix */
{"aas"},
/* 40 */
{"inc", {__ax}},
{"inc", {__cx}},
{"inc", {__dx}},
{"inc", {__bx}},
{"inc", {__sp}},
{"inc", {__bp}},
{"inc", {__si}},
{"inc", {__di}},
/* 48 */
{"dec", {__ax}},
{"dec", {__cx}},
{"dec", {__dx}},
{"dec", {__bx}},
{"dec", {__sp}},
{"dec", {__bp}},
{"dec", {__si}},
{"dec", {__di}},
/* 50 */
{"push", {__ax64}},
{"push", {__cx64}},
{"push", {__dx64}},
{"push", {__bx64}},
{"push", {__sp64}},
{"push", {__bp64}},
{"push", {__si64}},
{"push", {__di64}},
/* 58 */
{"pop", {__ax64}},
{"pop", {__cx64}},
{"pop", {__dx64}},
{"pop", {__bx64}},
{"pop", {__sp64}},
{"pop", {__bp64}},
{"pop", {__si64}},
{"pop", {__di64}},
/* 60 */
{"?pusha|pushad| x"},
{"?popa|popad| x"},
{"bound", {Gv, Mq}},
{"arpl", {Ew, Gw}},			//{"movsxd", {Gv, Ed}},
{0, {SPECIAL_TYPE_PREFIX}},		/* fs-prefix */
{0, {SPECIAL_TYPE_PREFIX}},		/* gs-prefix */
{0, {SPECIAL_TYPE_PREFIX}},		/* op-size prefix */
{0, {SPECIAL_TYPE_PREFIX}},		/* addr-size prefix */
/* 68 */
{"push", {Iv64}},
{"imul", {Gv, Ev, Iv}},
{"push", {sIbv64}},
{"imul", {Gv, Ev, sIbv}},
{"insb"},
{"?insw|insd|insd"},
{"outsb"},
{"?outsw|outsd|outsd"},
/* 70 */
{"jo", {Jb}},
{"jno", {Jb}},
{"|jc|jb|jnae", {Jb}},
{"|jnc|jnb|jae", {Jb}},
{"|jz|je", {Jb}},
{"|jnz|jne", {Jb}},
{"|jna|jbe", {Jb}},
{"|ja|jnbe", {Jb}},
/* 78 */
{"js", {Jb}},
{"jns", {Jb}},
{"|jp|jpe", {Jb}},
{"|jnp|jpo", {Jb}},
{"|jl|jnge", {Jb}},
{"|jnl|jge", {Jb}},
{"|jng|jle", {Jb}},
{"|jg|jnle", {Jb}},
/* 80 */
{0, {SPECIAL_TYPE_GROUP, GROUP_80}},
{0, {SPECIAL_TYPE_GROUP, GROUP_81}},
{0},
{0, {SPECIAL_TYPE_GROUP, GROUP_83}},
{"test", {Eb, Gb}},
{"test", {Ev, Gv}},
{"xchg", {Eb, Gb}},
{"xchg", {Ev, Gv}},
/* 88 */
{"mov", {Eb, Gb}},
{"mov", {Ev, Gv}},
{"mov", {Gb, Eb}},
{"mov", {Gv, Ev}},
{"mov", {MRwv, Sw}},
{"lea", {Gv, M}},
{"mov", {Sw, MRwv}},
{0, {SPECIAL_TYPE_PREFIX}}, // XOP prefix {0, {SPECIAL_TYPE_GROUP, GROUP_8F}},
/* 90 */
{"nop"},		/* same as xchg (e)ax, (e)ax */
{"xchg", {X__ax, __cx}},
{"xchg", {X__ax, __dx}},
{"xchg", {X__ax, __bx}},
{"xchg", {X__ax, __sp}},
{"xchg", {X__ax, __bp}},
{"xchg", {X__ax, __si}},
{"xchg", {X__ax, __di}},
/* 98 */
{"?cbw|cwde|cdqe"},
{"?cwd|cdq|cqo"},
{"~call", {Ap}},
{"fwait"},
{"?pushf|pushfd|pushfq", {_064}},
{"?popf|popfd|popfq", {_064}},
{"sahf"},
{"lahf"},
/* A0 */
{"mov", {X__al, Ob}},
{"mov", {X__ax, Ov}},
{"mov", {Ob, X__al}},
{"mov", {Ov, X__ax}},
{"movsb"},
{"?movsw|movsd|movsq"},
{"cmpsb"},
{"?cmpsw|cmpsd|cmpsq"},
/* A8 */
{"test", {X__al, Ib}},
{"test", {X__ax, Iv}},
{"stosb"},
{"?stosw|stosd|stosq"},
{"lodsb"},
{"?lodsw|lodsd|lodsq"},
{"scasb"},
{"?scasw|scasd|scasq"},
/* B0 */
{"mov", {__al, Ib}},
{"mov", {__cl, Ib}},
{"mov", {__dl, Ib}},
{"mov", {__bl, Ib}},
{"mov", {__ah, Ib}},
{"mov", {__ch, Ib}},
{"mov", {__dh, Ib}},
{"mov", {__bh, Ib}},
/* B8 */
{"mov", {__ax, Ivq}},
{"mov", {__cx, Ivq}},
{"mov", {__dx, Ivq}},
{"mov", {__bx, Ivq}},
{"mov", {__sp, Ivq}},
{"mov", {__bp, Ivq}},
{"mov", {__si, Ivq}},
{"mov", {__di, Ivq}},
/* C0 */
{0, {SPECIAL_TYPE_GROUP, GROUP_C0}},
{0, {SPECIAL_TYPE_GROUP, GROUP_C1}},
{"ret", {Iw}},
{"ret"},
{0, {SPECIAL_TYPE_PREFIX}}, // VEX prefix {"les", {Gv, Mp}}
{0, {SPECIAL_TYPE_PREFIX}}, // VEX prefix {"lds", {Gv, Mp}}
{0, {SPECIAL_TYPE_GROUP, GROUP_C6}},
{0, {SPECIAL_TYPE_GROUP, GROUP_C7}},
/* C8 */
{"enter", {Iw, Ib}},
{"leave"},
{"retf", {Iw}},
{"retf"},
{"int", {__3}},
{"int", {Ib}},
{"into"},
{"?iret|iretd|iretq"},
/* D0 */
{0, {SPECIAL_TYPE_GROUP, GROUP_D0}},
{0, {SPECIAL_TYPE_GROUP, GROUP_D1}},
{0, {SPECIAL_TYPE_GROUP, GROUP_D2}},
{0, {SPECIAL_TYPE_GROUP, GROUP_D3}},
{"aam", {Ib}},
{"aad", {Ib}},
{"setalc"},
{"xlat"},
/* D8 */
{0, {SPECIAL_TYPE_FGROUP, 0}},
{0, {SPECIAL_TYPE_FGROUP, 1}},
{0, {SPECIAL_TYPE_FGROUP, 2}},
{0, {SPECIAL_TYPE_FGROUP, 3}},
{0, {SPECIAL_TYPE_FGROUP, 4}},
{0, {SPECIAL_TYPE_FGROUP, 5}},
{0, {SPECIAL_TYPE_FGROUP, 6}},
{0, {SPECIAL_TYPE_FGROUP, 7}},
/* E0 */
{"|loopnz|loopne", {Jb}},
{"|loopz|loope", {Jb}},
{"loop", {Jb}},
{"*jcxz|jecxz|jrcxz", {Jb}},
{"in", {X__al, Ib}},
{"in", {X__ax, Ib}},
{"out", {Ib, X__al}},
{"out", {Ib, X__ax}},
/* E8 */
{"call", {Jv}},
{"jmp", {Jv}},
{"~jmp", {Ap}},
{"jmp", {Jb}},
{"in", {X__al, X__dxw}},
{"in", {X__ax, X__dxw}},
{"out", {X__dxw, X__al}},
{"out", {X__dxw, X__ax}},
/* F0 */
{0, {SPECIAL_TYPE_PREFIX}},		/* lock-prefix */
{"smi"},
{0, {SPECIAL_TYPE_PREFIX}},		/* repnz-prefix */
{0, {SPECIAL_TYPE_PREFIX}},		/* rep-prefix */
{"hlt"},
{"cmc"},
{0, {SPECIAL_TYPE_GROUP, GROUP_F6}},
{0, {SPECIAL_TYPE_GROUP, GROUP_F7}},
/* F8 */
{"clc"},
{"stc"},
{"cli"},
{"sti"},
{"cld"},
{"std"},
{0, {SPECIAL_TYPE_GROUP, GROUP_FE}},
{0, {SPECIAL_TYPE_GROUP, GROUP_FF}},
};

x86_64_insn_patch x86_64_insn_patches[] = {
{0x06, {0}}, // push es
{0x07, {0}}, // pop es
{0x0e, {0}}, // push cs
{0x16, {0}}, // push ss
{0x17, {0}}, // pop ss
{0x1e, {0}}, // push ds
{0x1f, {0}}, // pop ds
{0x27, {0}}, // daa
{0x2f, {0}}, // das
{0x37, {0}}, // aaa
{0x3f, {0}}, // aas
{0x40, {0}}, // REX prefixes
{0x41, {0}},
{0x42, {0}},
{0x43, {0}},
{0x44, {0}},
{0x45, {0}},
{0x46, {0}},
{0x47, {0}},
{0x48, {0}},
{0x49, {0}},
{0x4a, {0}},
{0x4b, {0}},
{0x4c, {0}},
{0x4d, {0}},
{0x4e, {0}},
{0x4f, {0}}, // ..
{0x60, {0}}, // pusha
{0x61, {0}}, // popa
{0x62, {0}}, // bound
{0x63, {"movsxd", {Gv, Ed}}},
{0x9a, {0}}, // call Ap
{0xce, {0}}, // into
{0xd4, {0}}, // aam
{0xd5, {0}}, // aad
{0xd6, {0}}, // setalc
{0xea, {0}}, // jmp Ap
{-1, {0}},
};

x86opc_insn x86_insns_ext[256] = {
/* 00 */
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_00}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_01}},
{"lar", {Gv, Ew}},
{"lsl", {Gv, Ew}},
{0},
{"syscall"},
{"clts"},
{"sysret"},
/* 08 */
{"invd"},
{"wbinvd"},
{0},
{"ud1"},
{0},
{"prefetch", {Eb}},
{"femms"},
{0, {SPECIAL_TYPE_PREFIX}},
/* 10 */
{"movups", {Vo, Wo}},
{"movups", {Wo, Vo}},
{"movlps", {Vq, Wq}},
{"movlps", {Mq, Vq}},
{"unpcklps", {Vo, Wq}},
{"unpckhps", {Vo, Wq}},
{"movhps", {Vq, Wq}},
{"movhps", {Mq, Vq}},
/* 18 */
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_18}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
/* 20 */
{"mov", {Rr64, Cd}},
{"mov", {Rr, Dd}},
{"mov", {Cd, Rr}},
{"mov", {Dd, Rr}},
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_0F24}},
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_0F25}},
{0},
{0},
/* 28 */
{"movaps", {Vo, Wo}},
{"movaps", {Wo, Vo}},
{"cvtpi2ps", {Vu, Qq}},
{"movntps", {Mo, Vo}},
{"cvttps2pi", {Pq, Wq}},
{"cvtps2pi", {Pq, Wq}},
{"ucomiss", {Vz, Wz}},
{"comiss", {Vz, Wz}},
/* 30 */
{"wrmsr"},
{"rdtsc"},
{"rdmsr"},
{"rdpmc"},
{"sysenter"},
{"sysexit"},
{0},
{"getsec"},
/* 38 */
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_0F38}},
{0},
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_0F3A}},
{0},
{0},
{0},
{0},
{0},
/* 40 */
{"cmovo", {Gv, Ev}},
{"cmovno", {Gv, Ev}},
{"|cmovc|cmovb", {Gv, Ev}},
{"|cmovnc|cmovnb", {Gv, Ev}},
{"|cmovz|cmove", {Gv, Ev}},
{"|cmovnz|cmovne", {Gv, Ev}},
{"|cmova|cmovnbe", {Gv, Ev}},
{"|cmovna|cmovbe", {Gv, Ev}},
/* 48 */
{"cmovs", {Gv, Ev}},
{"cmovns", {Gv, Ev}},
{"|cmovp|cmovpe", {Gv, Ev}},
{"|cmovnp|cmovpo", {Gv, Ev}},
{"|cmovl|cmovnge", {Gv, Ev}},
{"|cmovnl|cmovge", {Gv, Ev}},
{"|cmovng|cmovle", {Gv, Ev}},
{"|cmovg|cmovnle", {Gv, Ev}},
/* 50 */
{"movmskps", {Gd, VRo}},
{"sqrtps", {Vo, Wo}},
{"rsqrtps", {Vo, Wo}},
{"rcpps", {Vo, Wo}},
{"andps", {Vo, Wo}},
{"andnps", {Vo, Wo}},
{"orps", {Vo, Wo}},
{"xorps", {Vo, Wo}},
/* 58 */
{"addps", {Vo, Wo}},
{"mulps", {Vo, Wo}},
{"cvtps2pd", {Vo, Wq}},
{"cvtdq2ps", {Vo, Wo}},
{"subps", {Vo, Wo}},
{"minps", {Vo, Wo}},
{"divps", {Vo, Wo}},
{"maxps", {Vo, Wo}},
/* 60 */
{"punpcklbw", {Pu, Qz}},
{"punpcklwd", {Pu, Qz}},
{"punpckldq", {Pu, Qz}},
{"packsswb", {Pu, Qu}},
{"pcmpgtb", {Pu, Qu}},
{"pcmpgtw", {Pu, Qu}},
{"pcmpgtd", {Pu, Qu}},
{"packuswb", {Pu, Qu}},
/* 68 */
{"punpckhbw", {Pu, Qu}},
{"punpckhwd", {Pu, Qu}},
{"punpckhdq", {Pu, Qu}},
{"packssdw", {Pu, Qu}},
{0},
{0},
{"~movd", {Pu, Er}},
{"movq", {Pu, Qu}},
/* 70 */
{"pshufw", {Pu, Qu, Ib}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_71}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_72}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_73}},
{"pcmpeqb", {Pu, Qu}},
{"pcmpeqw", {Pu, Qu}},
{"pcmpewd", {Pu, Qu}},
{"emms"},
/* 78 */
{"vmread", {Ev64, Gv64}},
{"vmwrite", {Gv64, Ev64}},
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_0F7A}},
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_0F7B}},
{0},
{0},
{"~movd", {Er, Pq}},
{"movq", {Qq, Pq}},
/* 80 */
{"jo", {Jv}},
{"jno", {Jv}},
{"|jc|jb|jnae", {Jv}},
{"|jnc|jnb|jae", {Jv}},
{"|jz|je", {Jv}},
{"|jnz|jne", {Jv}},
{"|jna|jbe", {Jv}},
{"|ja|jnbe", {Jv}},
/* 88 */
{"js", {Jv}},
{"jns", {Jv}},
{"|jp|jpe", {Jv}},
{"|jnp|jpo", {Jv}},
{"|jl|jnge", {Jv}},
{"|jnl|jge", {Jv}},
{"|jng|jle", {Jv}},
{"|jg|jnle", {Jv}},
/* 90 */
{"seto", {Eb}},
{"setno", {Eb}},
{"|setc|setb|setnae", {Eb}},
{"|setnc|setnb|setae", {Eb}},
{"|setz|sete", {Eb}},
{"|setnz|setne", {Eb}},
{"|setna|setbe", {Eb}},
{"|seta|setnbe", {Eb}},
/* 98 */
{"sets", {Eb}},
{"setns", {Eb}},
{"|setp|setpe", {Eb}},
{"|setnp|setpo", {Eb}},
{"|setl|setnge", {Eb}},
{"|setnl|setge", {Eb}},
{"|setng|setle", {Eb}},
{"|setg|setnle", {Eb}},
/* A0 */
{"push", {__fs}},
{"pop", {__fs}},
{"cpuid"},
{"bt", {Ev, Gv}},
{"shld", {Ev, Gv, Ib}},
{"shld", {Ev, Gv, X__cl}},
{0},
{0},
/* A8 */
{"push", {__gs}},
{"pop", {__gs}},
{"rsm"},
{"bts", {Ev, Gv}},
{"shrd", {Ev, Gv, Ib}},
{"shrd", {Ev, Gv, X__cl}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_AE}},
{"imul", {Gv, Ev}},
/* B0 */
{"cmpxchg", {Eb, Gb}},
{"cmpxchg", {Ev, Gv}},
{"lss", {Gv, Mp}},
{"btr", {Ev, Gv}},
{"lfs", {Gv, Mp}},
{"lgs", {Gv, Mp}},
{"~movzx", {Gv, Eb}},
{"~movzx", {Gv, Ew}},
/* B8 */
{"jmpe", {Jv}},
{"ud2"},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_BA}},
{"btc", {Ev, Gv}},
{"bsf", {Gv, Ev}},
{"bsr", {Gv, Ev}},
{"~movsx", {Gv, Eb}},
{"~movsx", {Gv, Ew}},
/* C0 */
{"xadd", {Eb, Gb}},
{"xadd", {Ev, Gv}},
{"cmpCCps", {Vo, Wo, Ib}},
{"movnti", {Mr, Gr}},
{"pinsrw", {Pq, MRwr, Ib}},
{"pextrw", {Gr, PRq, Ib}},
{"shufps", {Vo, Wo, Ib}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_C7}},
/* C8 */
{"bswap", {__axdq}},
{"bswap", {__cxdq}},
{"bswap", {__dxdq}},
{"bswap", {__bxdq}},
{"bswap", {__spdq}},
{"bswap", {__bpdq}},
{"bswap", {__sidq}},
{"bswap", {__didq}},
/* D0 */
{0},
{"psrlw", {Pu, Qu}},
{"psrld", {Pu, Qu}},
{"psrlq", {Pu, Qu}},
{"paddq", {Pu, Qu}},
{"pmullw", {Pu, Qu}},
{0},
{"pmovmskb", {Gd, PRu}},
/* D8 */
{"psubusb", {Pu, Qu}},
{"psubusw", {Pu, Qu}},
{"pminub", {Pu, Qu}},
{"pand", {Pu, Qu}},
{"paddusb", {Pu, Qu}},
{"paddusw", {Pu, Qu}},
{"pmaxub", {Pu, Qu}},
{"pandn", {Pu, Qu}},
/* E0 */
{"pavgb", {Pu, Qu}},
{"psraw", {Pu, Qu}},
{"psrad", {Pu, Qu}},
{"pavgw", {Pu, Qu}},
{"pmulhuw", {Pu, Qu}},
{"pmulhw", {Pu, Qu}},
{0},
{"movntq", {Mq, Pq}},
/* E8 */
{"psubsb", {Pu, Qu}},
{"psubsw", {Pu, Qu}},
{"pminsw", {Pu, Qu}},
{"por", {Pu, Qu}},
{"paddsb", {Pu, Qu}},
{"paddsw", {Pu, Qu}},
{"pmaxsw", {Pu, Qu}},
{"pxor", {Pu, Qu}},
/* F0 */
{0},
{"psllw", {Pu, Qu}},
{"pslld", {Pu, Qu}},
{"psllq", {Pu, Qu}},
{"pmuludq", {Pu, Qu}},
{"pmaddwd", {Pu, Qu}},
{"psadbw", {Pu, Qu}},
{"maskmovq", {Pq, PRq}},
/* F8 */
{"psubb", {Pu, Qu}},
{"psubw", {Pu, Qu}},
{"psubd", {Pu, Qu}},
{"psubq", {Pu, Qu}},
{"paddb", {Pu, Qu}},
{"paddw", {Pu, Qu}},
{"paddd", {Pu, Qu}},
{0},
};

x86opc_insn x86_insns_ext_66[256] = {
/* 00 */
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_00}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_01}},
{"lar", {Gv, Ew}},
{"lsl", {Gv, Ew}},
{0},
{"syscall"},
{"clts"},
{"sysret"},
/* 08 */
{"invd"},
{"wbinvd"},
{0},
{"ud1"},
{0},
{"prefetch", {Eb}},
{"femms"},
{0, {SPECIAL_TYPE_PREFIX}},
/* 10 */
{"movupd", {Vo, Wo}},
{"movupd", {Wo, Vo}},
{"movlpd", {Vq, Wq}},
{"movlpd", {Mq, Vq}},
{"unpcklpd", {Vo, Wq}},
{"unpckhpd", {Vo, Wq}},
{"movhpd", {Vq, Wq}},
{"movhpd", {Mq, Vq}},
/* 18 */
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_18}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
/* 20 */
{"mov", {Rr64, Cd}},	// default 64
{"mov", {Rr, Dd}},	// default 32
{"mov", {Cd, Rr}},	// default 64
{"mov", {Dd, Rr}},	// default 32
{0},
{0},
{0},
{0},
/* 28 */
{"movapd", {Vo, Wo}},
{"movapd", {Wo, Vo}},
{"cvtpi2pd", {Vo, Qq}},
{"movntpd", {Mo, Vo}},
{"cvttpd2pi", {Pq, Wo}},
{"cvtpd2pi", {Pq, Wo}},
{"ucomisd", {Vz, Wz}},
{"comisd", {Vz, Wz}},
/* 30 */
{"wrmsr"},
{"rdtsc"},
{"rdmsr"},
{"rdpmc"},
{"sysenter"},
{"sysexit"},
{0},
{"getsec"},
/* 38 */
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_660F38}},
{0},
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_660F3A}},
{0},
{0},
{0},
{0},
{0},
/* 40 */
{"cmovo", {Gv, Ev}},
{"cmovno", {Gv, Ev}},
{"|cmovc|cmovb", {Gv, Ev}},
{"|cmovnc|cmovnb", {Gv, Ev}},
{"|cmovz|cmove", {Gv, Ev}},
{"|cmovnz|cmovne", {Gv, Ev}},
{"|cmova|cmovnbe", {Gv, Ev}},
{"|cmovna|cmovbe", {Gv, Ev}},
/* 48 */
{"cmovs", {Gv, Ev}},
{"cmovns", {Gv, Ev}},
{"|cmovp|cmovpe", {Gv, Ev}},
{"|cmovnp|cmovpo", {Gv, Ev}},
{"|cmovl|cmovnge", {Gv, Ev}},
{"|cmovnl|cmovge", {Gv, Ev}},
{"|cmovng|cmovle", {Gv, Ev}},
{"|cmovg|cmovnle", {Gv, Ev}},
/* 50 */
{"movmskpd", {Gd, VRo}},
{"sqrtpd", {Vo, Wo}},
{0},  // {"rsqrtpd", {Vo, Wo}},
{0},  // {"rcppd", {Vo, Wo}},
{"andpd", {Vo, Wo}},
{"andnpd", {Vo, Wo}},
{"orpd", {Vo, Wo}},
{"xorpd", {Vo, Wo}},
/* 58 */
{"addpd", {Vo, Wo}},
{"mulpd", {Vo, Wo}},
{"cvtpd2ps", {Vo, Wo}},
{"cvtps2dq", {Vo, Wo}},
{"subpd", {Vo, Wo}},
{"minpd", {Vo, Wo}},
{"divpd", {Vo, Wo}},
{"maxpd", {Vo, Wo}},
/* 60 */
{"punpcklbw", {Pu, Qz}},
{"punpcklwd", {Pu, Qz}},
{"punpckldq", {Pu, Qz}},
{"packsswb", {Pu, Qu}},
{"pcmpgtb", {Pu, Qu}},
{"pcmpgtw", {Pu, Qu}},
{"pcmpgtd", {Pu, Qu}},
{"packuswb", {Pu, Qu}},
/* 68 */
{"punpckhbw", {Pu, Qu}},
{"punpckhwd", {Pu, Qu}},
{"punpckhdq", {Pu, Qu}},
{"packssdw", {Pu, Qu}},
{"punpcklqdq", {Vo, Wo}},
{"punpckhqdq", {Vo, Wo}},
{"~movd", {Vo, Er}},
{"movdqa", {Pu, Qu}},
/* 70 */
{"pshufd", {Pu, Qu, Ib}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_71}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_72}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_73}},
{"pcmpeqb", {Pu, Qu}},
{"pcmpeqw", {Pu, Qu}},
{"pcmpewd", {Pu, Qu}},
{0},
/* 78 */
{"extrq", {VRo, Ib, Ib}},
{"extrq", {Vo, VRo}},
{0},
{0},
{"haddpd", {Vo, Wo}},
{"hsubpd", {Vo, Wo}},
{"~movd", {Er, Vo}},
{"movdqa", {Wo, Vo}},
/* 80 */
{"jo", {Jv}},
{"jno", {Jv}},
{"|jc|jb|jnae", {Jv}},
{"|jnc|jnb|jae", {Jv}},
{"|jz|je", {Jv}},
{"|jnz|jne", {Jv}},
{"|jna|jbe", {Jv}},
{"|ja|jnbe", {Jv}},
/* 88 */
{"js", {Jv}},
{"jns", {Jv}},
{"|jp|jpe", {Jv}},
{"|jnp|jpo", {Jv}},
{"|jl|jnge", {Jv}},
{"|jnl|jge", {Jv}},
{"|jng|jle", {Jv}},
{"|jg|jnle", {Jv}},
/* 90 */
{"seto", {Eb}},
{"setno", {Eb}},
{"|setc|setb|setnae", {Eb}},
{"|setnc|setnb|setae", {Eb}},
{"|setz|sete", {Eb}},
{"|setnz|setne", {Eb}},
{"|setna|setbe", {Eb}},
{"|seta|setnbe", {Eb}},
/* 98 */
{"sets", {Eb}},
{"setns", {Eb}},
{"|setp|setpe", {Eb}},
{"|setnp|setpo", {Eb}},
{"|setl|setnge", {Eb}},
{"|setnl|setge", {Eb}},
{"|setng|setle", {Eb}},
{"|setg|setnle", {Eb}},
/* A0 */
{"push", {__fs}},
{"pop", {__fs}},
{"cpuid"},
{"bt", {Ev, Gv}},
{"shld", {Ev, Gv, Ib}},
{"shld", {Ev, Gv, X__cl}},
{0},
{0},
/* A8 */
{"push", {__gs}},
{"pop", {__gs}},
{"rsm"},
{"bts", {Ev, Gv}},
{"shrd", {Ev, Gv, Ib}},
{"shrd", {Ev, Gv, X__cl}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_AE}},
{"imul", {Gv, Ev}},
/* B0 */
{"cmpxchg", {Eb, Gb}},
{"cmpxchg", {Ev, Gv}},
{"lss", {Gv, Mp}},
{"btr", {Ev, Gv}},
{"lfs", {Gv, Mp}},
{"lgs", {Gv, Mp}},
{"~movzx", {Gv, Eb}},
{"~movzx", {Gv, Ew}},
/* B8 */
{"jmpe", {Jv}},
{"ud2"},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_BA}},
{"btc", {Ev, Gv}},
{"bsf", {Gv, Ev}},
{"bsr", {Gv, Ev}},
{"~movsx", {Gv, Eb}},
{"~movsx", {Gv, Ew}},
/* C0 */
{"xadd", {Eb, Gb}},
{"xadd", {Ev, Gv}},
{"cmpCCpd", {Vo, Wo, Ib}},
{0},
{"pinsrw", {Vo, MRwr, Ib}},
{"pextrw", {Gr, VRo, Ib}},
{"shufpd", {Vo, Wo, Ib}},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_66_C7}},
/* C8 */
{"bswap", {__axdq}},
{"bswap", {__cxdq}},
{"bswap", {__dxdq}},
{"bswap", {__bxdq}},
{"bswap", {__spdq}},
{"bswap", {__bpdq}},
{"bswap", {__sidq}},
{"bswap", {__didq}},
/* D0 */
{"addsubpd", {Vo, Wo}},
{"psrlw", {Pu, Qu}},
{"psrld", {Pu, Qu}},
{"psrlq", {Pu, Qu}},
{"paddq", {Pu, Qu}},
{"pmullw", {Pu, Qu}},
{"movq", {Wq, Vq}},
{"pmovmskb", {Gr, PRu}},
/* D8 */
{"psubusb", {Pu, Qu}},
{"psubusw", {Pu, Qu}},
{"pminub", {Pu, Qu}},
{"pand", {Pu, Qu}},
{"paddusb", {Pu, Qu}},
{"paddusw", {Pu, Qu}},
{"pmaxub", {Pu, Qu}},
{"pandn", {Pu, Qu}},
/* E0 */
{"pavgb", {Pu, Qu}},
{"psraw", {Pu, Qu}},
{"psrad", {Pu, Qu}},
{"pavgw", {Pu, Qu}},
{"pmulhuw", {Pu, Qu}},
{"pmulhw", {Pu, Qu}},
{"cvttpd2dq", {Vo, Wo}},
{"movntdq", {Mo, Vo}},
/* E8 */
{"psubsb", {Pu, Qu}},
{"psubsw", {Pu, Qu}},
{"pminsw", {Pu, Qu}},
{"por", {Pu, Qu}},
{"paddsb", {Pu, Qu}},
{"paddsw", {Pu, Qu}},
{"pmaxsw", {Pu, Qu}},
{"pxor", {Pu, Qu}},
/* F0 */
{0},
{"psllw", {Pu, Qu}},
{"pslld", {Pu, Qu}},
{"psllq", {Pu, Qu}},
{"pmuludq", {Pu, Qu}},
{"pmaddwd", {Pu, Qu}},
{"psadbw", {Pu, Qu}},
{"maskmovdqu", {Vo, VRo}},
/* F8 */
{"psubb", {Pu, Qu}},
{"psubw", {Pu, Qu}},
{"psubd", {Pu, Qu}},
{"psubq", {Pu, Qu}},
{"paddb", {Pu, Qu}},
{"paddw", {Pu, Qu}},
{"paddd", {Pu, Qu}},
{0},
};

x86opc_insn x86_insns_ext_f2[256] = {
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 10 */
{"movsd", {Vq, Wq}},
{"movsd", {Wq, Vq}},
{"movddup", {Vo, Wq}},
{0},
{0},
{0},
{0},
{0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0},
{0},
{"~cvtsi2sd", {Vq, Er}},
{"movntsd", {Mq, Vq}},
{"cvttsd2si", {Gr, Wq}},
{"cvtsd2si", {Gr, Wq}},
{0},
{0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0, {SPECIAL_TYPE_OPC_GROUP, GROUP_OPC_F20F38}},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
/* 40 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0},
{"sqrtsd", {Vq, Wq}},
{0},
{0},
{0},
{0},
{0},
{0},
/* 58 */
{"addsd", {Vq, Wq}},
{"mulsd", {Vq, Wq}},
{"cvtsd2ss", {Vd, Wq}},
{0},
{"subsd", {Vq, Wq}},
{"minsd", {Vq, Wq}},
{"divsd", {Vq, Wq}},
{"maxsd", {Vq, Wq}},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{"pshuflw", {Vo, Wo, Ib}},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
/* 78 */
{"insertq", {Vo, VRo, Iw}},
{"insertq", {Vo, VRo}},
{0},
{0},
{"haddps", {Vo, Wo}},
{"hsubps", {Vo, Wo}},
{0},
{0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0},
{0},
{"cmpCCsd", {Vq, Wq, Ib}},
{0},
{0},
{0},
{0},
{0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{"addsubps", {Vo, Wo}},
{0},
{0},
{0},
{0},
{0},
{"movdq2q", {Pq, VRq}},
{0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0},
{0},
{0},
{0},
{0},
{0},
{"cvtpd2dq", {Vo, Wo}},
{0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{"lddqu", {Vo, Mo}},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
};

x86opc_insn x86_insns_ext_f3[256] = {
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 10 */
{"movss", {Vd, Wd}},
{"movss", {Wd, Vd}},
{"movsldup", {Vo, Wo}},
{0},
{0},
{0},
{"movshdup", {Vo, Wo}},
{0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0},
{0},
{"~cvtsi2ss", {Vd, Er}},
{"movntss", {Md, Vd}},
{"cvttss2si", {Gr, Wd}},
{"cvtss2si", {Gr, Wd}},
{0},
{0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0},
{"sqrtss", {Vd, Wd}},
{"rsqrtss", {Vd, Wd}},
{"rcpss", {Vd, Wd}},
{0},
{0},
{0},
{0},
/* 58 */
{"addss", {Vd, Wd}},
{"mulss", {Vd, Wd}},
{"cvtss2sd", {Vq, Wd}},
{"cvttps2dq", {Vo, Wo}},
{"subss", {Vd, Wd}},
{"minss", {Vd, Wd}},
{"divss", {Vd, Wd}},
{"maxss", {Vd, Wd}},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{"movdqu", {Vo, Wo}},
/* 70 */
{"pshufhw", {Vo, Wo, Ib}},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
/* 78 */
{0},
{0},
{0},
{0},
{0},
{0},
{"movq", {Vo, Wq}},
{"movdqu", {Wo, Vo}},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0},
{0},
{0},
{0},
{0},
{0},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_F3_AE}},
{0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{"popcnt", {Gv, Ev}},
{0},
{0},
{0},
{"tzcnt", {Gv, Ev}},
{"lzcnt", {Gv, Ev}},
{0},
{0},
/* c0 */
{0},
{0},
{"cmpCCss", {Vd, Wd, Ib}},
{0},
{0},
{0},
{0},
{0, {SPECIAL_TYPE_GROUP, GROUP_EXT_F3_C7}},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0},
{0},
{0},
{0},
{0},
{0},
{"movq2dq", {Vo, PRq}},
{0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0},
{0},
{0},
{0},
{0},
{0},
{"cvtdq2pd", {Vo, Wq}},
{0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
};

x86opc_insn x86_opc_group_insns[X86_OPC_GROUPS][256] = {
/* 0 - GROUP_OPC_0F38 */
{
/* 00 */
{"pshufb", {Pu, Qu}},
{"phaddw", {Pu, Qu}},
{"phaddd", {Pu, Qu}},
{"phaddsw", {Pu, Qu}},
{"pmaddubsw", {Pu, Qu}},
{"phsubw", {Pu, Qu}},
{"phsubd", {Pu, Qu}},
{"phsubsw", {Pu, Qu}},
/* 08 */
{"psignb", {Pu, Qu}},
{"psignw", {Pu, Qu}},
{"psignd", {Pu, Qu}},
{"pmulhrsw", {Pu, Qu}},
{0},
{0},
{0},
{0},
/* 10 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 18 */
{0},
{0},
{0},
{0},
{"pabsb", {Pu, Qu}},
{"pabsw", {Pu, Qu}},
{"pabsd", {Pu, Qu}},
{0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{"movbe", {Gv, Mv}},
{"movbe", {Mv, Gv}},
{0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 1 - GROUP_OPC_660F38 */
{
/* 00 */
{"pshufb", {Pu, Qu}},
{"phaddw", {Pu, Qu}},
{"phaddd", {Pu, Qu}},
{"phaddsw", {Pu, Qu}},
{"pmaddubsw", {Pu, Qu}},
{"phsubw", {Pu, Qu}},
{"phsubd", {Pu, Qu}},
{"phsubsw", {Pu, Qu}},
/* 08 */
{"psignb", {Pu, Qu}},
{"psignw", {Pu, Qu}},
{"psignd", {Pu, Qu}},
{"pmulhrsw", {Pu, Qu}},
{0},
{0},
{0},
{0},
/* 10 */
{"pblendvb", {Vo, Wo, XMM0}},
{0},
{0},
{0},
{"blendvps", {Vo, Wo, XMM0}},
{"blendvpd", {Vo, Wo, XMM0}},
{0},
{"ptest", {Vo, Wo}},
/* 18 */
{0},
{0},
{0},
{0},
{"pabsb", {Pu, Qu}},
{"pabsw", {Pu, Qu}},
{"pabsd", {Pu, Qu}},
{0},
/* 20 */
{"pmovsxbw", {Vo, Wq}},
{"pmovsxbd", {Vo, Wd}},
{"pmovsxbq", {Vo, Ww}},
{"pmovsxwd", {Vo, Wq}},
{"pmovsxwq", {Vo, Wd}},
{"pmovsxdq", {Vo, Wq}},
{0},
{0},
/* 28 */
{"pmuldq", {Vo, Wo}},
{"pcmpeqq", {Vo, Wo}},
{"movntdqa", {Vo, Mo}},
{"packusdw", {Vo, Wo}},
{0},
{0},
{0},
{0},
/* 30 */
{"pmovzxbw", {Vo, Wq}},
{"pmovzxbd", {Vo, Wd}},
{"pmovzxbq", {Vo, Ww}},
{"pmovzxwd", {Vo, Wq}},
{"pmovzxwq", {Vo, Wd}},
{"pmovzxdq", {Vo, Wq}},
{0},
{"pcmpgtq", {Vo, Wo}},
/* 38 */
{"pminsb", {Vo, Wo}},
{"pminsd", {Vo, Wo}},
{"pminuw", {Vo, Wo}},
{"pminud", {Vo, Wo}},
{"pmaxsb", {Vo, Wo}},
{"pmaxsd", {Vo, Wo}},
{"pmaxuw", {Vo, Wo}},
{"pmaxud", {Vo, Wo}},
/* 40 */
{"pmulld", {Vo, Wo}},
{"phminposuw", {Vo, Wo}},
{0},
{0},
{0},
{0},
{0},
{0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{"invept", {Gr, Mo}},
{"invvpid", {Gr, Mo}},
{"invpcid", {Gr, Mo}},
{0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0},
{0},
{0},
{"aesimc", {Vo, Wo}},
{"aesenc", {Vo, Wo}},
{"aesenclast", {Vo, Wo}},
{"aesdec", {Vo, Wo}},
{"aesdeclast", {Vo, Wo}},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{"movbe", {Gv, Mv}},
{"movbe", {Mv, Gv}},
{0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},

/* 2 - GROUP_OPC_F20F38 */
{
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 10 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{"~crc32", {Gr, Eb}},
{"~crc32", {Gr, Ev}},
{0},
{0},
{0},
{0},
{0},
{0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 3 - GROUP_OPC_0F3A */
{
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{"palignr", {Pu, Qu, Ib}},
/* 10 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 4 - GROUP_OPC_660F3A */
{
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{"roundps", {Vo, Wo, Ib}},
{"roundpd", {Vo, Wo, Ib}},
{"roundss", {Vo, Wo, Ib}},
{"roundsd", {Vo, Wo, Ib}},
{"blendps", {Vo, Wo, Ib}},
{"blendpd", {Vo, Wo, Ib}},
{"pblendw", {Vo, Wo, Ib}},
{"palignr", {Vo, Wo, Ib}},
/* 10 */
{0},
{0},
{0},
{0},
{"pextrb", {MRbr, Vo, Ib}},
{"pextrw", {MRwr, Vo, Ib}},
{"&pextrd|pextrd|pextrq", {Er, Vo, Ib}},
{"extractps", {MRdr, Vo, Ib}},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{"pinsrb", {Vo, MRbd, Ib}},
{"insertps", {Vo, Wd, Ib}},
{"&pinsrd|pinsrd|pinsrq", {Vo, Er, Ib}},
{0},
{0},
{0},
{0},
{0},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{"dpps", {Vo, Wo, Ib}},
{"dppd", {Vo, Wo, Ib}},
{"mpsadbw", {Vo, Wo, Ib}},
{0},
{"pclmulqdq", {Vo, Wo, Ib}},
{0},
{0},
{0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{"pcmpestrm", {Vo, Wo, Ib}},
{"pcmpestri", {Vo, Wo, Ib}},
{"pcmpistrm", {Vo, Wo, Ib}},
{"pcmpistri", {Vo, Wo, Ib}},
{0},
{0},
{0},
{0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0},
{0},
{0},
{0}, 
{0},
{0},
{0},
{"aeskeygenassist", {Vo, Wo, Ib}},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 5 - GROUP_OPC_0F7A */
{
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 10 */
{"frczps", {Vo, Wo}},
{"frczpd", {Vo, Wo}},
{"frczss", {Vo, Wd}},
{"frczsd", {Vo, Wq}},
{0},
{0},
{0},
{0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{"cvtph2ps", {Vo, Wq}},
{"cvtps2ph", {Wq, Vo}},
{0},
{0},
{0},
{0},
{0},
{0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{0},
{"phaddbw", {Vo, Wo}},
{"phaddbd", {Vo, Wo}},
{"phaddbq", {Vo, Wo}},
{0},
{0},
{"phaddwd", {Vo, Wo}},
{"phaddwq", {Vo, Wo}},
/* 48 */
{0},
{0},
{0},
{"phadddq", {Vo, Wo}},
{0},
{0},
{0},
{0},
/* 50 */
{0},
{"phaddubw", {Vo, Wo}},
{"phaddubd", {Vo, Wo}},
{"phaddubq", {Vo, Wo}},
{0},
{0},
{"phadduwd", {Vo, Wo}},
{"phadduwq", {Vo, Wo}},
/* 58 */
{0},
{0},
{0},
{"phaddudq", {Vo, Wo}},
{0},
{0},
{0},
{0},
/* 60 */
{0},
{"phsubbw", {Vo, Wo}},
{"phsubwd", {Vo, Wo}},
{"phsubdq", {Vo, Wo}},
{0},
{0},
{0},
{0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 6 - GROUP_OPC_0F7B */
{
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 10 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{"protb", {Vo, Wo, Ib}},
{"protw", {Vo, Wo, Ib}},
{"protd", {Vo, Wo, Ib}},
{"protq", {Vo, Wo, Ib}},
{0},
{0},
{0},
{0},
/* 48 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 6 - GROUP_OPC_0F24 */
{
/* 00 */
{"fmaddps", {VD, VD, VS0o, VS1o}},
{"fmaddpd", {VD, VD, VS0o, VS1o}},
{"fmaddss", {VD, VD, VS0d, VS1d}},
{"fmaddsd", {VD, VD, VS0q, VS1q}},
{"fmaddps", {VD, VS0o, VS1o, VD}},
{"fmaddpd", {VD, VS0o, VS1o, VD}},
{"fmaddss", {VD, VS0d, VS1d, VD}},
{"fmaddsd", {VD, VS0q, VS1q, VD}},
/* 08 */
{"fmsubps", {VD, VD, VS0o, VS1o}},
{"fmsubpd", {VD, VD, VS0o, VS1o}},
{"fmsubss", {VD, VD, VS0d, VS1d}},
{"fmsubsd", {VD, VD, VS0q, VS1q}},
{"fmsubps", {VD, VS0o, VS1o, VD}},
{"fmsubpd", {VD, VS0o, VS1o, VD}},
{"fmsubss", {VD, VS0d, VS1d, VD}},
{"fmsubsd", {VD, VS0q, VS1q, VD}},
/* 10 */
{"fnmaddps", {VD, VD, VS0o, VS1o}},
{"fnmaddpd", {VD, VD, VS0o, VS1o}},
{"fnmaddss", {VD, VD, VS0d, VS1d}},
{"fnmaddsd", {VD, VD, VS0q, VS1q}},
{"fnmaddps", {VD, VS0o, VS1o, VD}},
{"fnmaddpd", {VD, VS0o, VS1o, VD}},
{"fnmaddss", {VD, VS0d, VS1d, VD}},
{"fnmaddsd", {VD, VS0q, VS1q, VD}},
/* 18 */
{"fnmsubps", {VD, VD, VS0o, VS1o}},
{"fnmsubpd", {VD, VD, VS0o, VS1o}},
{"fnmsubss", {VD, VD, VS0d, VS1d}},
{"fnmsubsd", {VD, VD, VS0q, VS1q}},
{"fnmsubps", {VD, VS0o, VS1o, VD}},
{"fnmsubpd", {VD, VS0o, VS1o, VD}},
{"fnmsubss", {VD, VS0d, VS1d, VD}},
{"fnmsubsd", {VD, VS0q, VS1q, VD}},
/* 20 */
{"permps", {VD, VD, VS0o, VS1o}},
{"permpd", {VD, VD, VS0o, VS1o}},
{"pcmov", {VD, VD, VS0o, VS1o}},
{"pperm", {VD, VD, VS0o, VS1o}},
{"permps", {VD, VS0o, VS1o, VD}},
{"permpd", {VD, VS0o, VS1o, VD}},
{"pcmov", {VD, VS0o, VS1o, VD}},
{"pperm", {VD, VS0o, VS1o, VD}},
/* 28 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{"protb", {VD, VS0o, VS1o}},
{"protw", {VD, VS0o, VS1o}},
{"protd", {VD, VS0o, VS1o}},
{"protq", {VD, VS0o, VS1o}},
{"pshlb", {VD, VS0o, VS1o}},
{"pshlw", {VD, VS0o, VS1o}},
{"pshld", {VD, VS0o, VS1o}},
{"pshlq", {VD, VS0o, VS1o}},
/* 48 */
{"pshab", {VD, VS0o, VS1o}},
{"pshaw", {VD, VS0o, VS1o}},
{"pshad", {VD, VS0o, VS1o}},
{"pshaq", {VD, VS0o, VS1o}},
{0},
{0},
{0},
{0},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0},
{0},
{0},
{0},
{0},
{"pmacssww", {VD, VS00o, VS1o, VD}},
{"pmacsswd", {VD, VS00o, VS1o, VD}},
{"pmacssdql", {VD, VS00o, VS1o, VD}},
/* 88 */
{0},
{0},
{0},
{0},
{0},
{0},
{"pmacssdd", {VD, VS00o, VS1o, VD}},
{"pmacssdqh", {VD, VS00o, VS1o, VD}},
/* 90 */
{0},
{0},
{0},
{0},
{0},
{"pmacsww", {VD, VS00o, VS1o, VD}},
{"pmacswd", {VD, VS00o, VS1o, VD}},
{"pmacsdql", {VD, VS00o, VS1o, VD}},
/* 98 */                   
{0},
{0},
{0},
{0},
{0},
{0},
{"pmacsdd", {VD, VS00o, VS1o, VD}},
{"pmacsdqh", {VD, VS00o, VS1o, VD}},
/* a0 */
{0},
{0},
{0},
{0},
{0},
{0},
{"pmadcsswd", {VD, VS00o, VS1o, VD}},
{0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0},
{0},
{0},
{0},
{0},
{0},
{"pmadcswd", {VD, VS00o, VS1o, VD}},
{0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
/* 7 - GROUP_OPC_0F25 */
{
/* 00 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 08 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 10 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 18 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 20 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 28 */
{0},
{0},
{0},
{0},
{"comps", {VD, VS00o, VS1o, Ib}},
{"compd", {VD, VS00o, VS1o, Ib}},
{"comss", {VD, VS00d, VS1d, Ib}},
{"comsd", {VD, VS00q, VS1q, Ib}},
/* 30 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 38 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 40 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 48 */
{0},
{0},
{0},
{0},
{"pcomb", {VD, VS00o, VS1o, Ib}},
{"pcomw", {VD, VS00o, VS1o, Ib}},
{"pcomd", {VD, VS00o, VS1o, Ib}},
{"pcomq", {VD, VS00o, VS1o, Ib}},
/* 50 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 58 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 60 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 68 */
{0},
{0},
{0},
{0},
{"pcomub", {VD, VS00o, VS1o, Ib}},
{"pcomuw", {VD, VS00o, VS1o, Ib}},
{"pcomud", {VD, VS00o, VS1o, Ib}},
{"pcomuq", {VD, VS00o, VS1o, Ib}},
/* 70 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 78 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 80 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 88 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 90 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* 98 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* a8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* b8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* c8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* d8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* e8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f0 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
/* f8 */
{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
},
};

x86opc_insn x86_group_insns[][8] = {
/* 0 - GROUP_80 */
{
{"~add", {Eb, Ib}},
{"~or", {Eb, Ib}},
{"~adc", {Eb, Ib}},
{"~sbb", {Eb, Ib}},
{"~and", {Eb, Ib}},
{"~sub", {Eb, Ib}},
{"~xor", {Eb, Ib}},
{"~cmp", {Eb, Ib}},
},
/* 1 - GROUP_81 */
{
{"~add", {Ev, Iv}},
{"~or", {Ev, Iv}},
{"~adc", {Ev, Iv}},
{"~sbb", {Ev, Iv}},
{"~and", {Ev, Iv}},
{"~sub", {Ev, Iv}},
{"~xor", {Ev, Iv}},
{"~cmp", {Ev, Iv}},
},
/* 2 - GROUP_83 */
{
{"~add", {Ev, sIbv}},
{"~or", {Ev, sIbv}},
{"~adc", {Ev, sIbv}},
{"~sbb", {Ev, sIbv}},
{"~and", {Ev, sIbv}},
{"~sub", {Ev, sIbv}},
{"~xor", {Ev, sIbv}},
{"~cmp", {Ev, sIbv}},
},
/* 3 - GROUP_8F */
{
{"~pop", {Ev64}},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 4 - GROUP_C0 */
{
{"~rol", {Eb, Ib}},
{"~ror", {Eb, Ib}},
{"~rcl", {Eb, Ib}},
{"~rcr", {Eb, Ib}},
{"~shl", {Eb, Ib}},
{"~shr", {Eb, Ib}},
{"~sal", {Eb, Ib}},
{"~sar", {Eb, Ib}},
},
/* 5 - GROUP_C1 */
{
{"~rol", {Ev, Ib}},
{"~ror", {Ev, Ib}},
{"~rcl", {Ev, Ib}},
{"~rcr", {Ev, Ib}},
{"~shl", {Ev, Ib}},
{"~shr", {Ev, Ib}},
{"~sal", {Ev, Ib}},
{"~sar", {Ev, Ib}},
},
/* 6 - GROUP_C6 */
{
{"~mov", {Eb, Ib}},
{0},
{0},
{0},
{0},
{0},
{0},
{"xabort", {Ib}},
},
/* 7 - GROUP_C7 */
{
{"~mov", {Ev, Iv}},
{0},
{0},
{0},
{0},
{0},
{0},
{"xbegin", {Jv}},
},
/* 8 - GROUP_D0 */
{
{"~rol", {Eb, __1}},
{"~ror", {Eb, __1}},
{"~rcl", {Eb, __1}},
{"~rcr", {Eb, __1}},
{"~shl", {Eb, __1}},
{"~shr", {Eb, __1}},
{"~sal", {Eb, __1}},
{"~sar", {Eb, __1}},
},
/* 9 - GROUP_D1 */
{
{"~rol", {Ev, __1}},
{"~ror", {Ev, __1}},
{"~rcl", {Ev, __1}},
{"~rcr", {Ev, __1}},
{"~shl", {Ev, __1}},
{"~shr", {Ev, __1}},
{"~sal", {Ev, __1}},
{"~sar", {Ev, __1}},
},
/* 10 - GROUP_D2 */
{
{"~rol", {Eb, X__cl}},
{"~ror", {Eb, X__cl}},
{"~rcl", {Eb, X__cl}},
{"~rcr", {Eb, X__cl}},
{"~shl", {Eb, X__cl}},
{"~shr", {Eb, X__cl}},
{"~sal", {Eb, X__cl}},
{"~sar", {Eb, X__cl}},
},
/* 11 - GROUP_D3 */
{
{"~rol", {Ev, X__cl}},
{"~ror", {Ev, X__cl}},
{"~rcl", {Ev, X__cl}},
{"~rcr", {Ev, X__cl}},
{"~shl", {Ev, X__cl}},
{"~shr", {Ev, X__cl}},
{"~sal", {Ev, X__cl}},
{"~sar", {Ev, X__cl}},
},
/* 12 - GROUP_F6 */
{
{"~test", {Eb, Ib}},
{"~test", {Eb, Ib}},
{"~not", {Eb}},
{"~neg", {Eb}},
{"mul", {X__al, Eb}},
{"imul", {X__al, Eb}},
{"div", {X__al, Eb}},
{"idiv", {X__al, Eb}},
},
/* 13 - GROUP_F7 */
{
{"~test", {Ev, Iv}},
{"~test", {Ev, Iv}},
{"~not", {Ev}},
{"~neg", {Ev}},
{"mul", {X__ax, Ev}},
{"imul", {X__ax, Ev}},
{"div", {X__ax, Ev}},
{"idiv", {X__ax, Ev}},
},
/* 14 - GROUP_FE */
{
{"~inc", {Eb}},
{"~dec", {Eb}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 15 - GROUP_FF */
{
{"~inc", {Ev}},
{"~dec", {Ev}},
{"~call", {Ev64}},
{"~call", {Mp}},
{"~jmp", {Ev64}},
{"~jmp", {Mp}},
{"~push", {Ev64}},
{0},
},
/* 16 - GROUP_EXT_00 */
{
{"sldt", {MRwv}},
{"str", {MRwv}},
{"lldt", {MRwv}},
{"ltr", {MRwv}},
{"verr", {MRwv}},
{"verw", {MRwv}},
{0},
{0},
},
/* 17 - GROUP_EXT_01 */
{
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0F01_0}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0F01_1}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0F01_2}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0F01_3}},
{"smsw", {MRwv}},
{0},
{"lmsw", {MRwv}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0F01_7}},
},
/* 18 - GROUP_EXT_18 */
{
{"prefetchnta", {M}},
{"prefetch0", {M}},
{"prefetch1", {M}},
{"prefetch2", {M}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
{"~nop", {Ev}},
},
/* 16 - GROUP_EXT_71 */
{
{0},
{0},
{"psrlw", {PRu, Ib}},
{0},
{"psraw", {PRu, Ib}},
{0},
{"psllw", {PRu, Ib}},
{0},
},
/* 17 - GROUP_EXT_72 */
{
{0},
{0},
{"psrld", {PRu, Ib}},
{0},
{"psrad", {PRu, Ib}},
{0},
{"pslld", {PRu, Ib}},
{0},
},
/* 18 - GROUP_EXT_73 */
{
{0},
{0},
{"psrlq", {PRu, Ib}},
{"psrldq", {PRu, Ib}},
{0},
{0},
{"psllq", {PRu, Ib}},
{"pslldq", {PRu, Ib}},
},
/* 22 - GROUP_EXT_AE */
{
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_0}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_1}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_2}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_3}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_4}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_5}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_6}},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FAE_7}},
},
/* 22 - GROUP_EXT_F3_AE */
{
{"rdfsbase", {Rr}},
{"rdgsbase", {Rr}},
{"wrfsbase", {Rr}},
{"wrgsbase", {Rr}},
{0},
{0},
{0},
{0},
},
/* 23 - GROUP_EXT_BA */
{
{0},
{0},
{0},
{0},
{"~bt", {Ev, Ib}},
{"~bts", {Ev, Ib}},
{"~btr", {Ev, Ib}},
{"~btc", {Ev, Ib}},
},
/* 24 - GROUP_EXT_C7 */
{
{0},
{"? |cmpxchg8b|cmpxchg16b", {M}},
{0},
{0},
{0},
{0},
{0, {SPECIAL_TYPE_SGROUP, GROUP_SPECIAL_0FC7_6}},
{"vmptrst", {Mq}},
},
/* 25 - GROUP_EXT_66_C7 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{"vmclear", {Mq}},
{0},
},
/* 26 - GROUP_EXT_F3_C7 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{"vmxon", {Mq}},
{0},
},
};

x86opc_insn x86_special_group_insns[X86_SPECIAL_GROUPS][9] = {
/* 0 - GROUP_SPECIAL_0F01_0 */
{
{0},
{"vmcall"},
{"vmlaunch"},
{"vmresume"},
{"vmxoff"},
{0},
{0},
{0},
// with mod!=11:
{"sgdt", {M}},
},
/* 1 - GROUP_SPECIAL_0F01_1 */
{
{"monitor"},
{"mwait"},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"sidt", {M}},
},
/* 2 - GROUP_SPECIAL_0F01_2 */
{
{"xgetbv"},
{"xsetbv"},
{0},
{0},
{"vmfunc"},
{"xend"},
{"xtest"},
{0},
// with mod!=11:
{"lgdt", {M}},
},
/* 3 - GROUP_SPECIAL_0F01_3 */
{
{"vmrun"},
{"vmmcall"},
{"vmload"},
{"vmsave"},
{"stgi"},
{"clgi"},
{"skinit"},
{"invlpga"},
// with mod!=11:
{"lidt", {M}},
},
/* 4 - GROUP_SPECIAL_0F01_7 */
{
{"swapgs"},
{"rdtscp"},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"invlpg", {M}},
},
/* 5 - GROUP_SPECIAL_0FAE_0 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"?fxsave|fxsave|fxsave64", {M}},
},
/* 6 - GROUP_SPECIAL_0FAE_1 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"?fxstor|fxstor|fxstor64", {M}},
},
/* 7 - GROUP_SPECIAL_0FAE_2 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"ldmxcsr", {Md}},
},
/* 8 - GROUP_SPECIAL_0FAE_3 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"stmxcsr", {Md}},
},
/* 9 - GROUP_SPECIAL_0FAE_4 */
{
{0},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"?xsave|xsave|xsave64", {M}},
},
/* 10 - GROUP_SPECIAL_0FAE_5 */
{
{"lfence"},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"?xrstor|xrstor|xrstor64", {M}},
},
/* 11 - GROUP_SPECIAL_0FAE_6 */
{
{"mfence"},
{0},
{0},
{0},
{0},
{0},
{0},
{"?xsaveopt|xsaveopt|xsaveopt64", {M}},
// with mod!=11:
{0},
},
/* 12 - GROUP_SPECIAL_0FAE_7 */
{
{"sfence"},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
// with mod!=11:
{"clflush", {M}},
},
/* 12 - GROUP_SPECIAL_0FC7_6 */
{
{"rdrand", {Rv}},
{"rdrand", {Rv}},
{"rdrand", {Rv}},
{"rdrand", {Rv}},
{"rdrand", {Rv}},
{"rdrand", {Rv}},
{"rdrand", {Rv}},
{"rdrand", {Rv}},
// with mod!=11:
{"vmptrld", {Mq}},
},
};

/*
 *	The ModR/M byte is < 0xC0
 */

x86opc_insn x86_modfloat_group_insns[8][8] = {
/* prefix D8 */
{
{"~fadd", {Ms}},
{"~fmul", {Ms}},
{"~fcom", {Ms}},
{"~fcomp", {Ms}},
{"~fsub", {Ms}},
{"~fsubr", {Ms}},
{"~fdiv", {Ms}},
{"~fdivr", {Ms}},
},
/* prefix D9 */
{
{"~fld", {Ms}},
{0},
{"~fst", {Ms}},
{"~fstp", {Ms}},
{"fldenv", {M}},
{"fldcw", {Mw}},
{"fstenv", {M}},
{"fstcw", {Mw}},
},
/* prefix DA */
{
{"~fiadd", {Md}},
{"~fimul", {Md}},
{"~ficom", {Md}},
{"~ficomp", {Md}},
{"~fisub", {Md}},
{"~fisubr", {Md}},
{"~fidiv", {Md}},
{"~fidivr", {Md}},
},
/* prefix DB */
{
{"~fild", {Md}},
{"~fisttp", {Md}},
{"~fist", {Md}},
{"~fistp", {Md}},
{0},
{"~fld", {Mt}},
{0},
{"~fstp", {Mt}},
},
/* prefix DC */
{
{"~fadd", {Ml}},
{"~fmul", {Ml}},
{"~fcom", {Ml}},
{"~fcomp", {Ml}},
{"~fsub", {Ml}},
{"~fsubr", {Ml}},
{"~fdiv", {Ml}},
{"~fdivr", {Ml}},
},
/* prefix DD */
{
{"~fld", {Ml}},
{"~fisttp", {Mq}},
{"~fst", {Ml}},
{"~fstp", {Ml}},
{"~frstor", {M}},
{0},
{"fsave", {M}},
{"fstsw", {Mw}},
},
/* prefix DE */
{
{"~fiadd", {Mw}},
{"~fimul", {Mw}},
{"~ficom", {Mw}},
{"~ficomp", {Mw}},
{"~fisub", {Mw}},
{"~fisubr", {Mw}},
{"~fidiv", {Mw}},
{"~fidivr", {Mw}},
},
/* prefix DF */
{
{"~fild", {Mw}},
{"~fisttp", {Mw}},
{"~fist", {Mw}},
{"~fistp", {Mw}},
{"~fbld", {Ma}},
{"~fild", {Mq}},
{"~fbstp", {Ma}},
{"~fistp", {Mq}},
}

};

x86opc_insn fgroup_12[8] = {
{"fnop"},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
};

x86opc_insn fgroup_14[8] = {
{"fchs"},
{"fabs"},
{0},
{0},
{"ftst"},
{"fxam"},
{0},
{0},
};

x86opc_insn fgroup_15[8] = {
{"fld1"},
{"fldl2t"},
{"fldl2e"},
{"fldpi"},
{"fldlg2"},
{"fldln2"},
{"fldz"},
{0},
};

x86opc_insn fgroup_16[8] = {
{"f2xm1"},
{"fyl2x"},
{"fptan"},
{"fpatan"},
{"fxtract"},
{"fprem1"},
{"fdecstp"},
{"fincstp"},
};

x86opc_insn fgroup_17[8] = {
{"fprem"},
{"fyl2xp1"},
{"fsqrt"},
{"fsincos"},
{"frndint"},
{"fscale"},
{"fsin"},
{"fcos"},
};

x86opc_insn fgroup_25[8] = {
{0},
{"fucompp"},
{0},
{0},
{0},
{0},
{0},
{0},
};

x86opc_insn fgroup_34[8] = {
{0},
{0},
{"fclex"},
{"finit"},
{0},
{0},
{0},
{0},
};

x86opc_insn fgroup_63[8] = {
{0},
{"fcompp"},
{0},
{0},
{0},
{0},
{0},
{0},
};

x86opc_insn fgroup_74[8] = {
{"fstsw", {X__axw}},
{0},
{0},
{0},
{0},
{0},
{0},
{0},
};

/*
 *	The ModR/M byte is >= 0xC0
 */

x86opc_finsn x86_float_group_insns[8][8] = {
/* prefix D8 */
{
{0, {"fadd", {__st, Ft}}},
{0, {"fmul", {__st, Ft}}},
{0, {"fcom", {__st, Ft}}},
{0, {"fcomp", {__st, Ft}}},
{0, {"fsub", {__st, Ft}}},
{0, {"fsubr", {__st, Ft}}},
{0, {"fdiv", {__st, Ft}}},
{0, {"fdivr", {__st, Ft}}},
},
/* prefix D9 */
{
{0, {"fld", {__st, Ft}}},
{0, {"fxch", {__st, Ft}}},
{(x86opc_insn *)&fgroup_12},
{0},
{(x86opc_insn *)&fgroup_14},
{(x86opc_insn *)&fgroup_15},
{(x86opc_insn *)&fgroup_16},
{(x86opc_insn *)&fgroup_17},
},
/* prefix DA */
{
{0, {"fcmovb", {__st, Ft}}},
{0, {"fcmove", {__st, Ft}}},
{0, {"fcmovbe", {__st, Ft}}},
{0, {"fcmovu", {__st, Ft}}},
{0},
{(x86opc_insn *)&fgroup_25},
{0},
{0},
},
/* prefix DB */
{
{0, {"fcmovnb", {__st, Ft}}},
{0, {"fcmovne", {__st, Ft}}},
{0, {"fcmovnbe", {__st, Ft}}},
{0, {"fcmovnu", {__st, Ft}}},
{(x86opc_insn*)&fgroup_34},
{0, {"fucomi", {__st, Ft}}},
{0, {"fcomi", {__st, Ft}}},
{0},
},
/* prefix DC */
{
{0, {"fadd", {Ft, __st}}},
{0, {"fmul", {Ft, __st}}},
{0},
{0},
{0, {"fsubr", {Ft, __st}}},
{0, {"fsub", {Ft, __st}}},
{0, {"fdivr", {Ft, __st}}},
{0, {"fdiv", {Ft, __st}}},
},
/* prefix DD */
{
{0, {"ffree", {Ft}}},
{0},
{0, {"fst", {Ft}}},
{0, {"fstp", {Ft}}},
{0, {"fucom", {Ft, __st}}},
{0, {"fucomp", {Ft}}},
{0},
{0},
},
/* prefix DE */
{
{0, {"faddp", {Ft, __st}}},
{0, {"fmulp", {Ft, __st}}},
{0},
{(x86opc_insn*)&fgroup_63},
{0, {"fsubrp", {Ft, __st}}},
{0, {"fsubp", {Ft, __st}}},
{0, {"fdivrp", {Ft, __st}}},
{0, {"fdivp", {Ft, __st}}},
},
/* prefix DF */
{
{0, {"ffreep", {Ft}}},
{0},
{0},
{0},
{(x86opc_insn*)&fgroup_74},
{0, {"fucomip", {__st, Ft}}},
{0, {"fcomip", {__st, Ft}}},
{0},
},

};


/*
 * The vex format insns
 */

#define E0(e) static x86opc_vex_insn v##e[] = {
#define E(e) {0}}; static x86opc_vex_insn v##e[] = {
#define Elast {0}}; 
E0(00)
{"vpshufb", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpshufb", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vpermq", _256|_66|_0f3a|W1, {Yy, Xy, Ib}},
E(01)
{"pvhaddw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vphaddw", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vpermpd", _256|_66|_0f3a|W1, {Yy, Xy, Ib}},
E(02)
{"vphaddd", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vphaddd", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vpblendd", _128|_66|_0f3a|W0, {Vo, VVo, Wo, Ib}},
{"vpblendd", _256|_66|_0f3a|W0, {Yy, YVy, Xy, Ib}},
E(03)
{"vphaddsw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vphaddsw", _256|_66|_0f38, {Yy, YVy, Xy}},
E(04)
{"vpermilps", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vpermilps", _256|_66|_0f3a, {Yy, Xy, Ib}},
{"vpmaddubsw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmaddubsw", _256|_66|_0f38, {Yy, YVy, Xy}},
E(05)
{"vphsubw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vphsubw", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vpermilpd", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vpermilpd", _256|_66|_0f3a, {Yy, Xy, Ib}},
E(06)
{"vphsubd", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vphsubd", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vperm2f128", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
E(07)
{"vphsubsw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vphsubsw", _256|_66|_0f38, {Yy, YVy, Xy}},
E(08)
{"vpsignb", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpsignb", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vroundps", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vroundps", _256|_66|_0f3a, {Yy, Xy, Ib}},
E(09)
{"vpsignw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpsignw", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vroundpd", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vroundpd", _256|_66|_0f3a, {Yy, Xy, Ib}},
E(0a)
{"vpsignd", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpsignd", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vroundss", _128|_66|_0f3a, {Vo, Wo, Ib}},
E(0b)
{"vpmulhrsw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmulhrsw", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vroundsd", _128|_66|_0f3a, {Vo, Wo, Ib}},
E(0c)
{"vblendps", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vblendps", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
{"vpermilps", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpermilps", _256|_66|_0f38, {Yy, YVy, Xy}},
E(0d)
{"vpermilpd", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpermilpd", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vblendpd", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vblendpd", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
E(0e)
{"vpblendw", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vpblendw", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
{"vptestps", _128|_66|_0f38, {Vo, Wo}},
{"vptestps", _256|_66|_0f38, {Yy, Xy}},
E(0f)
{"vpalignr", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vpalignr", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
{"vptestpd", _128|_66|_0f38, {Vo, Wo}},
{"vptestpd", _256|_66|_0f38, {Yy, Xy}},
E(10)
//{"vmovss", _128|_f3|_0f, {Vo, VVo, VRo}},
{"vmovss", _128|_f3|_0f, {Vo, VVo, Wd}},
//{"vmovsd", _128|_f2|_0f, {Vo, VVo, VRo}},
{"vmovsd", _128|_f2|_0f, {Vo, VVo, Wq}},
{"vmovups", _128|_0f, {Vo, Wo}},
{"vmovups", _256|_0f, {Yy, Xy}},
{"vmovupd", _128|_66|_0f, {Vo, Wo}},
{"vmovupd", _256|_66|_0f, {Yy, Xy}},
E(11)
//{"vmovss", _128|_f3|_0f, {VRo, VVo, Vo}},
{"vmovss", _128|_f3|_0f, {Wd, VVo, Vo}},
//{"vmovsd", _128|_f2|_0f, {VRo, VVo, Vo}},
{"vmovsd", _128|_f2|_0f, {Wq, VVo, Vo}},
{"vmovups", _128|_0f, {Wo, Vo}},
{"vmovups", _256|_0f, {Xy, Yy}},
{"vmovupd", _128|_66|_0f, {Wo, Vo}},
{"vmovupd", _256|_66|_0f, {Xy, Yy}},
E(12)
{0, _128|_0f25|W0, {SPECIAL_TYPE_GROUP, GROUP_0F25_12}},
{0, _128|_0f25|W1, {SPECIAL_TYPE_GROUP, GROUP_0F25_12_W}},
{0, _256|_0f25|W0, {SPECIAL_TYPE_GROUP, GROUP_0F25_12_L}},
{0, _128|_0fA|W0, {SPECIAL_TYPE_GROUP, GROUP_0FA_12}},
{0, _128|_0fA|W1, {SPECIAL_TYPE_GROUP, GROUP_0FA_12_W}},
{0, _256|_0fA|W0, {SPECIAL_TYPE_GROUP, GROUP_0FA_12_L}},
{"vmovddup", _128|_f2|_0f, {Vo, Wq}},
{"vmovddup", _256|_f2|_0f, {Yy, Xy}},
{"vmovhlps", _128|_0f, {Vo, VVo, VRo}},
{"vmovlps", _128|_0f, {Vo, VVo, Mq}},
{"vmovlpd", _128|_66|_0f, {Vo, VVo, Mq}},
{"vmovsldup", _128|_f3|_0f, {Vo, Wo}},
{"vmovsldup", _256|_f3|_0f, {Yy, Xy}},
E(13)
{"vmovlps", _128|_0f, {Mq, Vo}},
{"vmovlpd", _128|_66|_0f, {Mq, Vo}},
{"vcvtph2ps", _128|_0f, {Vo, Wq}},
{"vcvtph2ps", _256|_66|_0f38, {Yy, Wo}},
E(14)
{"vpextrb", _128|_66|_0f3a, {MRbr, Vo, Ib}},
{"vunpcklps", _128|_0f, {Vo, VVo, Wo}},
{"vunpcklps", _256|_0f, {Yy, YVy, Xy}},
{"vunpcklpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vunpcklpd", _256|_66|_0f, {Yy, YVy, Xy}},
E(15)
{"vpextrw", _128|_66|_0f3a, {MRwr, Vo, Ib}},
{"vunpckhps", _128|_0f, {Vo, VVo, Wo}},
{"vunpckhps", _256|_0f, {Yy, YVy, Xy}},
{"vunpckhpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vunpckhpd", _256|_66|_0f, {Yy, YVy, Xy}},
E(16)
{"vpextrd", _128|_66|_0f3a|W0, {Ed, Vo, Ib}},
{"vpextrq", _128|_66|_0f3a|W1, {Eq, Vo, Ib}},
{"vmovhps", _128|_0f, {Vo, VVo, Mq}},
{"vmovhpd", _128|_66|_0f, {Vo, VVo, Mq}},
{"vmovlhps", _128|_0f, {Vo, VVo, VRo}},
{"vmovshdup", _128|_f3|_0f, {Vo, Wo}},
{"vmovshdup", _256|_f3|_0f, {Yy, Xy}},
{"vpermps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
E(17)
{"vmovhps", _128|_0f, {Mq, Vo}},
{"vmovhpd", _128|_66|_0f, {Mq, Vo}},
{"vextractps", _128|_66|_0f3a, {Ed, Vo, Ib}},
{"vextractps", _128|_66|_0f3a|W1, {Eq, Vo, Ib}},
{"vptest", _128|_66|_0f38, {Vo, Wo}},
{"vptest", _256|_66|_0f38, {Yy, Xy}},
E(18)
{"vbroadcastss", _128|_66|_0f38, {Vo, Wd}},
{"vbroadcastss", _256|_66|_0f38, {Yy, Wd}},
{"vinsertf128", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
E(19)
{"vbroadcastsd", _128|_66|_0f38, {Vo, Wq}},
{"vbroadcastsd", _256|_66|_0f38, {Yy, Wq}},
{"vextractf128", _256|_66|_0f3a, {Xy, Yy, Ib}},
E(1a)
{"vbroadcastf128", _256|_66|_0f38, {Yy, Mo}},
E(1c)
{"vpabsb", _128|_66|_0f38, {Vo, Wo}},
{"vpabsb", _256|_66|_0f38, {Yy, Xy}},
E(1d)
{"vpabsw", _128|_66|_0f38, {Vo, Wo}},
{"vpabsw", _256|_66|_0f38, {Yy, Xy}},
{"vcvtps2ph", _128|_66|_0f3a, {Wq, Vo, Ib}},
{"vcvtps2ph", _256|_66|_0f3a, {Wo, Yy, Ib}},
E(1e)
{"vpabsd", _128|_66|_0f38, {Vo, Wo}},
{"vpabsd", _256|_66|_0f38, {Yy, Xy}},
E(20)
{"vpinsrb", _128|_66|_0f3a, {Vo, VVo, MRbd, Ib}},
{"vpmovsxbw", _128|_66|_0f38, {Vo, Wq}},
{"vpmovsxbw", _256|_66|_0f38, {Yy, Wo}},
E(21)
{"vinsertps", _128|_66|_0f3a, {Vo, VVo, Wd, Ib}},
{"vpmovsxbd", _128|_66|_0f38, {Vo, Wd}},
{"vpmovsxbd", _256|_66|_0f38, {Yy, Wq}},
E(22)
{"vpinsrd", _128|_66|_0f3a|W0, {Vo, VVo, Ed, Ib}},
{"vpinsrq", _128|_66|_0f3a|W1, {Vo, VVo, Eq, Ib}},
{"vpmovsxbq", _128|_66|_0f38, {Vo, Ww}},
{"vpmovsxbq", _256|_66|_0f38, {Yy, Wd}},
E(23)
{"vpmovsxwd", _128|_66|_0f38, {Vo, Wq}},
{"vpmovsxwd", _256|_66|_0f38, {Yy, Wo}},
E(24)
{"vpmovsxwq", _128|_66|_0f38, {Vo, Wd}},
{"vpmovsxwq", _256|_66|_0f38, {Yy, Wq}},
E(25)
{"vpmovsxdq", _128|_66|_0f38, {Vo, Wq}},
{"vpmovsxdq", _256|_66|_0f38, {Yy, Wo}},
E(28)
{"vmovapd", _128|_66|_0f, {Vo, Wo}},
{"vmovapd", _256|_66|_0f, {Yy, Xy}},
{"vpmuldq", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmuldq", _256|_66|_0f38, {Yy, YVy, Xy}},
E(29)
{"vmovapd", _128|_66|_0f, {Wo, Vo}},
{"vmovapd", _256|_66|_0f, {Xy, Yy}},
{"vpcmpeqq", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpcmpeqq", _256|_66|_0f38, {Yy, YVy, Xy}},
E(2a)
{"vmovntdqa", _128|_66|_0f38, {Vo, Mo}},
{"vmovntdqa", _256|_66|_0f38, {Yy, My}},
{"vcvtsi2ss", _128|_f3|_0f|W0, {Vo, VVo, Ed}},
{"vcvtsi2ss", _128|_f3|_0f|W1, {Vo, VVo, Eq}},
{"vcvtsi2sd", _128|_f2|_0f|W0, {Vo, VVo, Ed}},
{"vcvtsi2sd", _128|_f2|_0f|W1, {Vo, VVo, Eq}},
E(2b)
{"vmovntps", _128|_0f, {Mo, Vo}},
{"vmovntps", _256|_0f, {My, Xy}},
{"vmovntpd", _128|_66|_0f, {Mo, Vo}},
{"vmovntpd", _256|_66|_0f, {My, Xy}},
{"vpackusdw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpackusdw", _256|_66|_0f38, {Yy, YVy, Xy}},
E(2c)
{"vmaskmovps", _128|_66|_0f38, {Vo, VVo, Mo}},
{"vmaskmovps", _256|_66|_0f38, {Yy, YVy, My}},
{"vcvttss2si", _128|_f3|_0f|W0, {Gd, Wd}},
{"vcvttss2si", _128|_f3|_0f|W1, {Gq, Wd}},
{"vcvttsd2si", _128|_f2|_0f|W0, {Gd, Wq}},
{"vcvttsd2si", _128|_f2|_0f|W1, {Gq, Wq}},
E(2d)
{"vcvtsd2si", _128|_f2|_0f|W0, {Gd, Wq}},
{"vcvtsd2si", _128|_f2|_0f|W1, {Gq, Wq}},
{"vcvtss2si", _128|_f3|_0f|W0, {Gd, Wd}},
{"vcvtss2si", _128|_f3|_0f|W1, {Gq, Wd}},
{"vmaskmovpd", _128|_66|_0f38, {Vo, VVo, Mo}},
{"vmaskmovpd", _256|_66|_0f38, {Yy, YVy, My}},
E(2e)
{"vucomiss", _128|_0f, {Vo, Wd}},
{"vucomisd", _128|_66|_0f, {Vo, Wq}},
{"vmaskmovps", _128|_66|_0f38, {Mo, VVo, Vo}},
{"vmaskmovps", _256|_66|_0f38, {My, YVy, Yy}},
E(2f)
{"vmaskmovpd", _128|_66|_0f38, {Mo, VVo, Vo}},
{"vmaskmovpd", _256|_66|_0f38, {My, YVy, Yy}},
{"vcomiss", _128|_0f, {Vo, Wd}},
{"vcomisd", _128|_66|_0f, {Vo, Wq}},
E(30)
{"vpmovzxbw", _128|_66|_0f38, {Vo, Wq}},
{"vpmovzxbw", _256|_66|_0f38, {Yy, Wo}},
E(31)
{"vpmovzxbd", _128|_66|_0f38, {Vo, Wd}},
{"vpmovzxbd", _256|_66|_0f38, {Yy, Wq}},
E(32)
{"vpmovzxbq", _128|_66|_0f38, {Vo, Ww}},
{"vpmovzxbq", _256|_66|_0f38, {Yy, Wd}},
E(33)
{"vpmovzxwd", _128|_66|_0f38, {Vo, Wq}},
{"vpmovzxwd", _256|_66|_0f38, {Yy, Wo}},
E(34)
{"vpmovzxwq", _128|_66|_0f38, {Vo, Wd}},
{"vpmovzxwq", _256|_66|_0f38, {Yy, Wq}},
E(35)
{"vpmovzxdq", _128|_66|_0f38, {Vo, Wq}},
{"vpmovzxdq", _256|_66|_0f38, {Yy, Wo}},
E(36)
{"vpermd", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
E(37)
{"vpcmpgtq", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpcmpgtq", _256|_66|_0f38, {Yy, YVy, Xy}},
E(38)
{"vpminsb", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpminsb", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vinserti128", _256|_66|_0f3a, {Yy, YVy, Wo, Ib}},
E(39)
{"vpminsd", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpminsd", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vextracti128", _256|_66|_0f3a, {Wo, Yy, Ib}},
E(3a)
{"vpminuw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpminuw", _256|_66|_0f38, {Yy, YVy, Xy}},
E(3b)
{"vpminud", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpminud", _256|_66|_0f38, {Yy, YVy, Xy}},
E(3c)
{"vpmaxsb", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmaxsb", _256|_66|_0f38, {Yy, YVy, Xy}},
E(3d)
{"vpmaxsd", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmaxsd", _256|_66|_0f38, {Yy, YVy, Xy}},
E(3e)
{"vpmaxuw", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmaxuw", _256|_66|_0f38, {Yy, YVy, Xy}},
E(3f)
{"vpmaxud", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmaxud", _256|_66|_0f38, {Yy, YVy, Xy}},
E(40)
{"vpmulld", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vpmulld", _256|_66|_0f38, {Yy, YVy, Xy}},
{"vdpps", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vdpps", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
E(41)
{"vdppd", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vphminposuw", _128|_66|_0f38, {Vo, Wo}},
E(42)
{"vmpsadbw", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
{"vmpsadbw", _256|_66|_0f3a, {Yy, YVy, Xy, Ib}},
E(44)
{"vpclmulqdq", _128|_66|_0f3a, {Vo, VVo, Wo, Ib}},
E(45)
{"vpsrlvd", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vpsrlvd", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vpsrlvq", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vpsrlvq", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(46)
{"vpsravd", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vpsravd", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vperm2i128", _256|_66|_0f3a|W0, {Yy, YVy, Xy, Ib}},
E(47)
{"vpsllvd", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vpsllvd", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vpsllvq", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vpsllvq", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(48)
{"vpermil2ps", _128|_66|_0f3a|W0, {Vo, VVo, Wo, VIo, I4}},
{"vpermil2ps", _128|_66|_0f3a|W1, {Vo, VVo, VIo, Wo, I4}},
{"vpermil2ps", _256|_66|_0f3a|W0, {Yy, YVy, Xy, YIy, I4}},
{"vpermil2ps", _256|_66|_0f3a|W1, {Yy, YVy, YIy, Xy, I4}},
E(49)
{"vpermil2pd", _128|_66|_0f3a|W0, {Vo, VVo, Wo, VIo, I4}},
{"vpermil2pd", _128|_66|_0f3a|W1, {Vo, VVo, VIo, Wo, I4}},
{"vpermil2pd", _256|_66|_0f3a|W0, {Yy, YVy, Xy, YIy, I4}},
{"vpermil2pd", _256|_66|_0f3a|W1, {Yy, YVy, YIy, Xy, I4}},
E(4a)
{"vblendvps", _128|_66|_0f3a, {Vo, VVo, Wo, VIo}},
{"vblendvps", _256|_66|_0f3a, {Yy, YVy, Xy, YIy}},
E(4b)
{"vblendvpd", _128|_66|_0f3a, {Vo, VVo, Wo, VIo}},
{"vblendvpd", _256|_66|_0f3a, {Yy, YVy, Xy, YIy}},
E(4c)
{"vpblendvb", _128|_66|_0f3a, {Vo, VVo, Wo, VIo}},
{"vpblendvb", _256|_66|_0f3a, {Yy, YVy, Xy, VIo}},
E(50)
{"vmovmskps", _128|_0f, {Gd, VRo}},
{"vmovmskps", _256|_0f, {Gd, YRy}},
{"vmovmskpd", _128|_66|_0f, {Gd, VRo}},
{"vmovmskpd", _256|_66|_0f, {Gd, YRy}},
E(51)
{"vsqrtps", _128|_0f, {Vo, Wo}},
{"vsqrtps", _256|_0f, {Yy, Xy}},
{"vsqrtpd", _128|_66|_0f, {Vo, Wo}},
{"vsqrtpd", _256|_66|_0f, {Yy, Xy}},
{"vsqrtss", _128|_f3|_0f, {Vo, Wo}},
{"vsqrtsd", _128|_f2|_0f, {Vo, Wo}},
E(52)
{"vrsqrtps", _128|_0f, {Vo, Wo}},
{"vrsqrtss", _128|_f3|_0f, {Vo, Wo}},
E(53)
{"vrcpps", _128|_0f, {Vo, Wo}},
{"vrcpss", _128|_f3|_0f, {Vo, Wo}},
E(54)
{"vandps", _128|_0f, {Vo, VVo, Wo}},
{"vandps", _256|_0f, {Yy, YVy, Xy}},
{"vandpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vandpd", _256|_66|_0f, {Yy, YVy, Xy}},
E(55)
{"vandnps", _128|_0f, {Vo, VVo, Wo}},
{"vandnps", _256|_0f, {Yy, YVy, Xy}},
{"vandnpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vandnpd", _256|_66|_0f, {Yy, YVy, Xy}},
E(56)
{"vorps", _128|_0f, {Vo, VVo, Wo}},
{"vorps", _256|_0f, {Yy, YVy, Xy}},
{"vorpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vorpd", _256|_66|_0f, {Yy, YVy, Xy}},
E(57)
{"vxorps", _128|_0f, {Vo, VVo, Wo}},
{"vxorps", _256|_0f, {Yy, YVy, Xy}},
{"vxorpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vxorpd", _256|_66|_0f, {Yy, YVy, Xy}},
E(58)
{"vaddps", _128|_0f, {Vo, VVo, Wo}},
{"vaddps", _256|_0f, {Yy, YVy, Xy}},
{"vaddpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vaddpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaddss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vaddsd", _128|_f3|_0f, {Vo, VVo, Wo}},
{"vpbroadcastd", _128|_66|_0f38|W0, {Vo, Wd}},
{"vpbroadcastd", _256|_66|_0f38|W0, {Yy, Wd}},
E(59)
{"vmulss", _128|_0f, {Vo, VVo, Wo}},
{"vmulps", _256|_0f, {Yy, YVy, Xy}},
{"vmulpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vmulpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vmulss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vmulsd", _128|_f3|_0f, {Vo, VVo, Wo}},
{"vpbroadcastq", _128|_66|_0f38|W0, {Vo, Wq}},
{"vpbroadcastq", _256|_66|_0f38|W0, {Yy, Wq}},
E(5a)
{"vcvtps2pd", _128|_0f, {Vo, Wo}},
{"vcvtps2pd", _256|_0f, {Yy, Xy}},
{"vcvtpd2ps", _128|_66|_0f, {Vo, Wo}},
{"vcvtpd2ps", _256|_66|_0f, {Yy, Xy}},
{"vcvtss2sd", _128|_f3|_0f, {Vo, VVo, Wd}},
{"vcvtsd2ss", _128|_f2|_0f, {Vo, VVo, Wq}},
{"vbroadcasti128", _256|_66|_0f38, {Yy, Mo}},
E(5b)
{"vcvtdq2ps", _128|_0f, {Vo, Wo}},
{"vcvtdq2ps", _256|_0f, {Yy, Xy}},
{"vcvtps2dq", _128|_66|_0f, {Vo, Wo}},
{"vcvtps2dq", _256|_66|_0f, {Yy, Xy}},
{"vcvttps2dq", _128|_f3|_0f, {Vo, Wo}},
{"vcvttps2dq", _256|_f3|_0f, {Yy, Xy}},
E(5c)
{"vsubps", _128|_0f, {Vo, VVo, Wo}},
{"vsubps", _256|_0f, {Yy, YVy, Xy}},
{"vsubpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vsubpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vsubss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vsubsd", _128|_f3|_0f, {Vo, VVo, Wo}},
{"vfmaddsubps", _128|_66|_0f3a|W0, {Vo, VIo, Wo, VVo}},
{"vfmaddsubps", _128|_66|_0f3a|W1, {Vo, VIo, VVo, Wo}},
{"vfmaddsubps", _256|_66|_0f3a|W0, {Yy, YIy, Xy, YVy}},
{"vfmaddsubps", _256|_66|_0f3a|W1, {Yy, YIy, YVy, Xy}},
E(5d)
{"vminps", _128|_0f, {Vo, VVo, Wo}},
{"vminps", _256|_0f, {Yy, YVy, Xy}},
{"vminpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vminpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vminss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vminsd", _128|_f3|_0f, {Vo, VVo, Wo}},
{"vfmaddsubpd", _128|_66|_0f3a|W0, {Vo, VIo, Wo, VVo}},
{"vfmaddsubpd", _128|_66|_0f3a|W1, {Vo, VIo, VVo, Wo}},
{"vfmaddsubpd", _256|_66|_0f3a|W0, {Yy, YIy, Xy, YVy}},
{"vfmaddsubpd", _256|_66|_0f3a|W1, {Yy, YIy, YVy, Xy}},
E(5e)
{"vdivps", _128|_0f, {Vo, VVo, Wo}},
{"vdivps", _256|_0f, {Yy, YVy, Xy}},
{"vdivpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vdivpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vdivss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vdivsd", _128|_f3|_0f, {Vo, VVo, Wo}},
{"vfmsubaddps", _128|_66|_0f3a|W0, {Vo, VIo, Wo, VVo}},
{"vfmsubaddps", _128|_66|_0f3a|W1, {Vo, VIo, VVo, Wo}},
{"vfmsubaddps", _256|_66|_0f3a|W0, {Yy, YIy, Xy, YVy}},
{"vfmsubaddps", _256|_66|_0f3a|W1, {Yy, YIy, YVy, Xy}},
E(5f)
{"vmaxps", _128|_0f, {Vo, VVo, Wo}},
{"vmaxps", _256|_0f, {Yy, YVy, Xy}},
{"vmaxpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vmaxpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vmaxss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vmaxsd", _128|_f3|_0f, {Vo, VVo, Wo}},
{"vfmsubaddpd", _128|_66|_0f3a|W0, {Vo, VIo, Wo, VVo}},
{"vfmsubaddpd", _128|_66|_0f3a|W1, {Vo, VIo, VVo, Wo}},
{"vfmsubaddpd", _256|_66|_0f3a|W0, {Yy, YIy, Xy, YVy}},
{"vfmsubaddpd", _256|_66|_0f3a|W1, {Yy, YIy, YVy, Xy}},
E(60)
{"vpcmpestrm", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vpunpcklbw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpcklbw", _256|_66|_0f, {Yy, YVy, Xy}},
E(61)
{"vpcmpestri", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vpunpcklwd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpcklwd", _256|_66|_0f, {Yy, YVy, Xy}},
E(62)
{"vpcmpistrm", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vpunpckldq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpckldq", _256|_66|_0f, {Yy, YVy, Xy}},
E(63)
{"vpcmpistri", _128|_66|_0f3a, {Vo, Wo, Ib}},
{"vpacksswb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpacksswb", _256|_66|_0f, {Yy, YVy, Xy}},
E(64)
{"vpcmpgtb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpcmpgtb", _256|_66|_0f, {Yy, YVy, Xy}},
E(65)
{"vpcmpgtw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpcmpgtw", _256|_66|_0f, {Yy, YVy, Xy}},
E(66)
{"vpcmpgtd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpcmpgtd", _256|_66|_0f, {Yy, YVy, Xy}},
E(67)
{"vpackuswb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpackuswb", _256|_66|_0f, {Yy, YVy, Xy}},
E(68)
{"vpunpckhbw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpckhbw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vfmaddps", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmaddps", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfmaddps", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfmaddps", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(69)
{"vpunpckhwd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpckhwd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vfmaddpd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmaddpd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfmaddpd", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfmaddpd", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(6a)
{"vpunpckhdq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpckhdq", _256|_66|_0f, {Yy, YVy, Xy}},
{"vfmaddss", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmaddss", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(6b)
{"vpackssdw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpackssdw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vfmaddsd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmaddsd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(6c)
{"vpunpcklqdq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpcklqdq", _256|_66|_0f, {Yy, YVy, Xy}},
{"vfmsubps", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmsubps", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfmsubps", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfmsubps", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(6d)
{"vpunpckhqdq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpunpckhqdq", _256|_66|_0f, {Yy, YVy, Xy}},
{"vfmsubpd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmsubpd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfmsubpd", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfmsubpd", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(6e)
{"vmovd", _128|_66|_0f|W0, {Vo, Ed}},
{"vmovq", _128|_66|_0f|W1, {Vo, Eq}},
{"vfmsubss", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmsubss", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(6f)
{"vmovdqa", _128|_66|_0f, {Vo, Wo}},
{"vmovdqa", _256|_66|_0f, {Yy, Xy}},
{"vmovdqu", _128|_f3|_0f, {Vo, Wo}},
{"vmovdqu", _256|_f3|_0f, {Yy, Xy}},
{"vfmsubsd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfmsubsd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(70)
{"vpshufd", _128|_66|_0f, {Vo, Wo, Ib}},
{"vpshufd", _256|_66|_0f, {Yy, Xy, Ib}},
{"vpshufhw", _128|_f3|_0f, {Vo, Wo, Ib}},
{"vpshufhw", _256|_f3|_0f, {Yy, Xy, Ib}},
{"vpshuflw", _128|_f2|_0f, {Vo, Wo, Ib}},
{"vpshuflw", _256|_f2|_0f, {Yy, Xy, Ib}},
E(71)
{0, _128|_66|_0f, {SPECIAL_TYPE_GROUP, GROUP_660F71}},
E(72)
{0, _128|_66|_0f, {SPECIAL_TYPE_GROUP, GROUP_660F72}},
E(73)
{0, _128|_66|_0f, {SPECIAL_TYPE_GROUP, GROUP_660F73}},
E(74)
{"vpcmpeqb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpcmpeqb", _256|_66|_0f, {Yy, YVy, Xy}},
E(75)
{"vpcmpeqw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpcmpeqw", _256|_66|_0f, {Yy, YVy, Xy}},
E(76)
{"vpcmpeqd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpcmpeqd", _256|_66|_0f, {Yy, YVy, Xy}},
E(77)
{"vzeroupper", _128|_0f},
{"vzeroall", _256|_0f},
E(78)
{"vpbroadcastb", _128|_66|_0f38|W0, {Vo, Wb}},
{"vpbroadcastb", _256|_66|_0f38|W0, {Yy, Wb}},
{"vfnmaddps", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmaddps", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfnmaddps", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfnmaddps", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(79)
{"vpbroadcastw", _128|_66|_0f38|W0, {Vo, Ww}},
{"vpbroadcastw", _256|_66|_0f38|W0, {Yy, Ww}},
{"vfnmaddpd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmaddpd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfnmaddpd", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfnmaddpd", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(7a)
{"vfnmaddss", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmaddss", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(7b)
{"vfnmaddsd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmaddsd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(7c)
{"vhaddpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vhaddpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vhaddss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vhaddss", _256|_f2|_0f, {Yy, YVy, Xy}},
{"vfnmsubps", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmsubps", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfnmsubps", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfnmsubps", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(7d)
{"vhsubpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vhsubpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vhsubss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vhsubss", _256|_f2|_0f, {Yy, YVy, Xy}},
{"vfnmsubpd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmsubpd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
{"vfnmsubpd", _256|_66|_0f3a|W0, {Yy, YIy, YVy, Xy}},
{"vfnmsubpd", _256|_66|_0f3a|W1, {Yy, YIy, Xy, YVy}},
E(7e)
{"vmovd", _128|_66|_0f|W0, {Ed, Vo}},
{"vmovq", _128|_66|_0f|W1, {Eq, Vo}},
{"vmovq", _128|_f3|_0f, {Vo, Wq}},
{"vfnmsubss", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmsubss", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(7f)
{"vmovdqa", _128|_66|_0f, {Wo, Vo}},
{"vmovdqa", _256|_66|_0f, {Xy, Yy}},
{"vmovdqu", _128|_f3|_0f, {Wo, Vo}},
{"vmovdqu", _256|_f3|_0f, {Xy, Yy}},
{"vfnmsubsd", _128|_66|_0f3a|W0, {Vo, VIo, VVo, Wo}},
{"vfnmsubsd", _128|_66|_0f3a|W1, {Vo, VIo, Wo, VVo}},
E(80)
{"vfrczpd", _128|_0f25, {Vo, Wo}},
{"vfrczpd", _256|_0f25, {Yy, Xy}},
E(81)
{"vfrczps", _128|_0f25, {Vo, Wo}},
{"vfrczps", _256|_0f25, {Yy, Xy}},
E(82)
{"vfrczsd", _128|_0f25, {Vo, Wo}},
{"vfrczsd", _256|_0f25, {Yy, Xy}},
E(83)
{"vfrczss", _128|_0f25, {Vo, Wo}},
{"vfrczss", _256|_0f25, {Yy, Xy}},
E(85)
{"vpmacssww", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(86)
{"vpmacsswd", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(87)
{"vpmacssdql", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(8c)
{"vpmaskmovd", _128|_66|_0f38|W0, {Vo, VVo, Mo}},
{"vpmaskmovd", _256|_66|_0f38|W0, {Yy, YVy, My}},
{"vpmaskmovq", _128|_66|_0f38|W1, {Vo, VVo, Mo}},
{"vpmaskmovq", _256|_66|_0f38|W1, {Yy, YVy, My}},
E(8e)
{"vpmacssdd", _128|_0f24, {Vo, VVo, Wo, VIo}},
{"vpmaskmovd", _128|_66|_0f38|W0, {Mo, VVo, Vo}},
{"vpmaskmovd", _256|_66|_0f38|W0, {My, YVy, Yy}},
{"vpmaskmovq", _128|_66|_0f38|W1, {Mo, VVo, Vo}},
{"vpmaskmovq", _256|_66|_0f38|W1, {My, YVy, Yy}},
E(8f)
{"vpmacssdqh", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(90)
{"vprotb", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vprotb", _128|_0f25|W1, {Vo, VVo, Wo}},
{"vpgatherdd", _128|_66|_0f38|W0, {Vo, Wd, VVo}},
{"vpgatherdd", _256|_66|_0f38|W0, {Yy, Xd, YVy}},
{"vpgatherdq", _128|_66|_0f38|W1, {Vo, Wd, VVo}},
{"vpgatherdq", _256|_66|_0f38|W1, {Yy, Xd, YVy}},
E(91)
{"vprotw", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vprotw", _128|_0f25|W1, {Vo, VVo, Wo}},
{"vpgatherqd", _128|_66|_0f38|W0, {Vo, Wq, VVo}},
{"vpgatherqd", _256|_66|_0f38|W0, {Yy, Xq, YVy}},
{"vpgatherqq", _128|_66|_0f38|W1, {Vo, Wq, VVo}},
{"vpgatherqq", _256|_66|_0f38|W1, {Yy, Xq, YVy}},
E(92)
{"vprotd", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vprotd", _128|_0f25|W1, {Vo, VVo, Wo}},
{"vgatherdps", _128|_66|_0f38|W0, {Vo, Wd, VVo}},
{"vgatherdps", _256|_66|_0f38|W0, {Yy, Xd, YVy}},
{"vgatherdpd", _128|_66|_0f38|W1, {Vo, Wd, VVo}},
{"vgatherdpd", _256|_66|_0f38|W1, {Yy, Xd, YVy}},
E(93)
{"vprotq", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vprotq", _128|_0f25|W1, {Vo, VVo, Wo}},
{"vgatherqps", _128|_66|_0f38|W0, {Vo, Wq, VVo}},
{"vgatherqps", _256|_66|_0f38|W0, {Yy, Xq, YVy}},
{"vgatherqpd", _128|_66|_0f38|W1, {Vo, Wq, VVo}},
{"vgatherqpd", _256|_66|_0f38|W1, {Yy, Xq, YVy}},
E(94)
{"vpshlb", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshlb", _128|_0f25|W1, {Vo, VVo, Wo}},
E(95)
{"vpmacsww", _128|_0f24, {Vo, VVo, Wo, VIo}},
{"vpshlw", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshlw", _128|_0f25|W1, {Vo, VVo, Wo}},
E(96)
{"vfmaddsub132ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmaddsub132ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmaddsub132pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmaddsub132pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
{"vpmacswd", _128|_0f24, {Vo, VVo, Wo, VIo}},
{"vpshld", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshld", _128|_0f25|W1, {Vo, VVo, Wo}},
E(97)
{"vfmaddsub132ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmaddsub132sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vpmacsdql", _128|_0f24, {Vo, VVo, Wo, VIo}},
{"vpshlq", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshlq", _128|_0f25|W1, {Vo, VVo, Wo}},
E(98)
{"vfmadd132ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmadd132ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmadd132pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmadd132pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
{"vpshab", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshab", _128|_0f25|W1, {Vo, VVo, Wo}},
E(99)
{"vfmadd132ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmadd132sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vpshaw", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshaw", _128|_0f25|W1, {Vo, VVo, Wo}},
E(9a)
{"vfmsub132ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmsub132ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmsub132pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmsub132pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
{"vpshad", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshad", _128|_0f25|W1, {Vo, VVo, Wo}},
E(9b)
{"vfmsub132ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmsub132sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vpshaq", _128|_0f25|W0, {Vo, Wo, VVo}},
{"vpshaq", _128|_0f25|W1, {Vo, VVo, Wo}},
E(9c)
{"vfnmadd132ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmadd132ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfnmadd132pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfnmadd132pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(9d)
{"vfnmadd132ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmadd132sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(9e)
{"vfnmsub132ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmsub132ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfnmsub132pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfnmsub132pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
{"vpmacsdd", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(9f)
{"vfnmsub132ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmsub132sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vpmacsdqh", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(a0)
{"vcvtph2ps", _128|_0f24, {Vo, Wq, Ib}},
{"vcvtph2ps", _256|_0f24, {Yy, Xo, Ib}},
E(a1)
{"vcvtps2ph", _128|_0f25, {Wq, Vo}},
{"vcvtps2ph", _256|_0f25, {Xo, Yy}},
E(a2)
{"vpcmov", _128|_0f24|W0, {Vo, VVo, Wo, VIo}},
{"vpcmov", _256|_0f24|W0, {Yy, YVy, Xy, YIy}},
{"vpcmov", _128|_0f24|W1, {Vo, VVo, VIo, Wo}},
{"vpcmov", _256|_0f24|W1, {Yy, YVy, YIy, Xy}},
E(a3)
{"vpperm", _128|_0f24|W0, {Vo, VVo, Wo, VIo}},
{"vpperm", _128|_0f24|W1, {Vo, VVo, VIo, Wo}},
E(a6)
{"vfmaddsub213ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmaddsub213ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmaddsub213pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmaddsub213pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
{"vpmadcsswd", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(a7)
{"vfmaddsub213ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmaddsub213sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(a8)
{"vfmadd213ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmadd213ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmadd213pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmadd213pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(a9)
{"vfmadd213ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmadd213sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(aa)
{"vfmsub213ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmsub213ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmsub213pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmsub213pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(ab)
{"vfmsub213ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmsub213sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(ac)
{"vfnmadd213ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmadd213ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfnmadd213pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfnmadd213pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(ad)
{"vfnmadd213ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmadd213sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(ae)
{0, _0f, {SPECIAL_TYPE_GROUP, GROUP_0FAE}},
{"vfnmsub213ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmsub213ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfnmsub213pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfnmsub213pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(af)
{"vfnmsub213ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmsub213sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(b6)
{"vfmaddsub231ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmaddsub231ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmaddsub231pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmaddsub231pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
{"vpmadcswd", _128|_0f24, {Vo, VVo, Wo, VIo}},
E(b7)
{"vfmaddsub231ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmaddsub231sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(b8)
{"vfmadd231ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmadd231ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmadd231pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmadd231pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(b9)
{"vfmadd231ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmadd231sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(ba)
{"vfmsub231ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmsub231ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfmsub231pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfmsub231pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(bb)
{"vfmsub231ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfmsub231sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(bc)
{"vfnmadd231ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmadd231ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfnmadd231pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfnmadd231pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(bd)
{"vfnmadd231ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmadd231sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(be)
{"vfnmsub231ps", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmsub231ps", _256|_66|_0f38|W0, {Yy, YVy, Xy}},
{"vfnmsub231pd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
{"vfnmsub231pd", _256|_66|_0f38|W1, {Yy, YVy, Xy}},
E(bf)
{"vfnmsub231ss", _128|_66|_0f38|W0, {Vo, VVo, Wo}},
{"vfnmsub231sd", _128|_66|_0f38|W1, {Vo, VVo, Wo}},
E(c0)
{"vprotb", _128|_0f24, {Vo, Wo, Ib}},
E(c1)
{"vprotw", _128|_0f24, {Vo, Wo, Ib}},
{"vphaddbw", _128|_0f25, {Vo, Wo}},
E(c2)
{"vcmpps", _128|_0f, {Vo, VVo, Wo, Ib}},
{"vcmpps", _256|_0f, {Yy, YVy, Xy, Ib}},
{"vcmppd", _128|_66|_0f, {Vo, VVo, Wo, Ib}},
{"vcmppd", _256|_66|_0f, {Yy, YVy, Xy, Ib}},
{"vcmpss", _128|_f2|_0f, {Vo, VVo, Wo, Ib}},
{"vcmpsd", _128|_f3|_0f, {Vo, VVo, Wo, Ib}},
{"vprotd", _128|_0f24, {Vo, Wo, Ib}},
{"vphaddbd", _128|_0f25, {Vo, Wo}},
E(c3)
{"vprotq", _128|_0f24, {Vo, Wo, Ib}},
{"vphaddbd", _128|_0f25, {Vo, Wo}},
E(c4)
{"pinsrw", _128|_66|_0f, {Vo, MRwr, Ib}},
E(c5)
{"vpextrw", _128|_66|_0f, {Gr, VRo, Ib}},
E(c6)
{"vshufps", _128|_0f, {Vo, VVo, Wo, Ib}},
{"vshufps", _256|_0f, {Yy, YVy, Xy, Ib}},
{"vshufpd", _128|_66|_0f, {Vo, VVo, Wo, Ib}},
{"vshufpd", _256|_66|_0f, {Yy, YVy, Xy, Ib}},
{"vphadddwd", _128|_0f25, {Vo, Wo}},
E(c7)
{"vphadddwq", _128|_0f25, {Vo, Wo}},
E(cb)
{"vphadddq", _128|_0f25, {Vo, Wo}},
E(cc)
{"vpcomb", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(cd)
{"vpcomw", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(ce)
{"vpcomd", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(cf)
{"vpcomq", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(d0)
{"vaddsubpd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vaddsubpd", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaddsubss", _128|_f2|_0f, {Vo, VVo, Wo}},
{"vaddsubss", _256|_f2|_0f, {Yy, YVy, Xy}},
E(d1)
{"vpsrlw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsrlw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vphaddubw", _128|_0f25, {Vo, Wo}},
E(d2)
{"vpsrld", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsrld", _256|_66|_0f, {Yy, YVy, Xy}},
{"vphaddubd", _128|_0f25, {Vo, Wo}},
E(d3)
{"vpsrlq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsrlq", _256|_66|_0f, {Yy, YVy, Xy}},
{"vphaddubq", _128|_0f25, {Vo, Wo}},
E(d4)
{"vpaddq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddq", _256|_66|_0f, {Yy, YVy, Xy}},
E(d5)
{"vpmullw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmullw", _256|_66|_0f, {Yy, YVy, Xy}},
E(d6)
{"vmovq", _128|_66|_0f, {Wq, Vo}},
{"vphadduwd", _128|_0f25, {Vo, Wo}},
E(d7)
{"vpmovmskb", _128|_66|_0f, {Gd, VRo}},
{"vpmovmskb", _256|_66|_0f, {Gq, YRy}},
{"vphadduwq", _128|_0f25, {Vo, Wo}},
{"vphaddwq", _128|_0f25, {Vo, Wo}}, // FIXME
E(d8)
{"vpsubusb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubusb", _256|_66|_0f, {Yy, YVy, Xy}},
E(d9)
{"vpsubusw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubusw", _256|_66|_0f, {Yy, YVy, Xy}},
E(da)
{"vpminub", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpminub", _256|_66|_0f, {Yy, YVy, Xy}},
E(db)
{"vpand", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpand", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaesimc", _128|_66|_0f38, {Vo, Wo}},
{"vphaddudq", _128|_0f25, {Vo, Wo}},
E(dc)
{"vpaddusb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddusb", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaesenc", _128|_66|_0f38, {Vo, VVo, Wo}},
E(dd)
{"vpaddusw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddusw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaesenclast", _128|_66|_0f38, {Vo, VVo, Wo}},
E(de)
{"vpmaxub", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmaxub", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaesdec", _128|_66|_0f38, {Vo, VVo, Wo}},
E(df)
{"vpandn", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpandn", _256|_66|_0f, {Yy, YVy, Xy}},
{"vaesdeclast", _128|_66|_0f38, {Vo, VVo, Wo}},
{"vaeskeygenassist", _128|_66|_0f3a, {Vo, Wo, Ib}},
E(e0)
{"vpavgb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpavgb", _256|_66|_0f, {Yy, YVy, Xy}},
E(e1)
{"vpsraw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vphsubbw", _128|_0f25, {Vo, Wo}},
E(e2)
{"vpsrad", _128|_66|_0f, {Vo, VVo, Wo}},
{"vphsubwd", _128|_0f25, {Vo, Wo}},
E(e3)
{"vpavgw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpavgw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vphsubdq", _128|_0f25, {Vo, Wo}},
E(e4)
{"vpmulhuw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmulhuw", _256|_66|_0f, {Yy, YVy, Xy}},
E(e5)
{"vpmulhw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmulhw", _256|_66|_0f, {Yy, YVy, Xy}},
E(e6)
{"vcvtdq2pd", _128|_f3|_0f, {Vo, Wo}},
{"vcvtdq2pd", _256|_f3|_0f, {Yy, Xy}},
{"vcvtpd2dq", _128|_f2|_0f, {Vo, Wo}},
{"vcvtpd2dq", _256|_f2|_0f, {Yy, Xy}},
{"vcvttpd2dq", _128|_66|_0f, {Vo, Wo}},
{"vcvttpd2dq", _256|_66|_0f, {Yy, Xy}},
E(e7)
{"vmovntdq", _128|_66|_0f, {Mo, Vo}},
{"vmovntdq", _256|_66|_0f, {My, Yy}},
E(e8)
{"vpsubsb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubsb", _256|_66|_0f, {Yy, YVy, Xy}},
E(e9)
{"vpsubsw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubsw", _256|_66|_0f, {Yy, YVy, Xy}},
E(ea)
{"vpminsw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpminsw", _256|_66|_0f, {Yy, YVy, Xy}},
E(eb)
{"vpor", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpor", _256|_66|_0f, {Yy, YVy, Xy}},
E(ec)
{"vpaddsb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddsb", _256|_66|_0f, {Yy, YVy, Xy}},
{"vpcomub", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(ed)
{"vpaddsw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddsw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vpcomuw", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(ee)
{"vpmaxsw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmaxsw", _256|_66|_0f, {Yy, YVy, Xy}},
{"vpcomud", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(ef)
{"vpxor", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpxor", _256|_66|_0f, {Yy, YVy, Xy}},
{"vpcomuq", _128|_0f24, {Vo, VVo, Wo, Ib}},
E(f0)
{"vlddqu", _128|_f2|_0f, {Vo, Mo}},
{"vlddqu", _256|_f2|_0f, {Yy, My}},
{"rorx", _128|_f2|_0f3a|W0, {Gd, Ed, Ib}},
{"rorx", _128|_f2|_0f3a|W1, {Gq, Eq, Ib}},
E(f1)
{"vpsllw", _128|_66|_0f, {Vo, VVo, Wo}},
E(f2)
{"vpslld", _128|_66|_0f, {Vo, VVo, Wo}},
{"andn", _128|_0f38|W0, {Gd, RVd, Ed}},
{"andn", _128|_0f38|W1, {Gq, RVq, Eq}},
E(f3)
{0, _128|_0f38|W0, {SPECIAL_TYPE_GROUP, GROUP_0F38_F3}},
{0, _128|_0f38|W1, {SPECIAL_TYPE_GROUP, GROUP_0F38_F3_W}},
{"vpsllq", _128|_66|_0f, {Vo, VVo, Wo}},
E(f4)
{"vpmuludq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmuludq", _256|_66|_0f, {Yy, YVy, Xy}},
E(f5)
{"vpmaddwd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpmaddwd", _256|_66|_0f, {Yy, YVy, Xy}},
{"bzhi", _128|_0f38|W0, {Gd, RVd, Ed}},
{"bzhi", _128|_0f38|W1, {Gq, RVq, Eq}},
{"pdep", _128|_f2|_0f38|W0, {Gd, RVd, Ed}},
{"pdep", _128|_f2|_0f38|W1, {Gq, RVq, Eq}},
{"pext", _128|_f3|_0f38|W0, {Gd, RVd, Ed}},
{"pext", _128|_f3|_0f38|W1, {Gq, RVq, Eq}},
E(f6)
{"vpsadbw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsadbw", _256|_66|_0f, {Yy, YVy, Xy}},
{"mulx", _128|_f2|_0f38|W0, {Gd, RVd, Ed}},
{"mulx", _128|_f2|_0f38|W1, {Gq, RVq, Eq}},
E(f7)
{"vmaskmovdqu", _128|_66|_0f, {Vo, VRo}},
{"bextr", _128|_0f38|W0, {Gd, RVd, Ed}},
{"bextr", _128|_0f38|W1, {Gq, RVq, Eq}},
{"sarx", _128|_f3|_0f38|W0, {Gd, RVd, Ed}},
{"sarx", _128|_f3|_0f38|W1, {Gq, RVq, Eq}},
{"shlx", _128|_66|_0f38|W0, {Gd, RVd, Ed}},
{"shlx", _128|_66|_0f38|W1, {Gq, RVq, Eq}},
{"shlx", _128|_f2|_0f38|W0, {Gd, RVd, Ed}},
{"shlx", _128|_f2|_0f38|W1, {Gq, RVq, Eq}},
E(f8)
{"vpsubb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubb", _256|_66|_0f, {Yy, YVy, Xy}},
E(f9)
{"vpsubw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubw", _256|_66|_0f, {Yy, YVy, Xy}},
E(fa)
{"vpsubd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubd", _256|_66|_0f, {Yy, YVy, Xy}},
E(fb)
{"vpsubq", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpsubq", _256|_66|_0f, {Yy, YVy, Xy}},
E(fc)
{"vpaddb", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddb", _256|_66|_0f, {Yy, YVy, Xy}},
E(fd)
{"vpaddw", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddw", _256|_66|_0f, {Yy, YVy, Xy}},
E(fe)
{"vpaddd", _128|_66|_0f, {Vo, VVo, Wo}},
{"vpaddd", _256|_66|_0f, {Yy, YVy, Xy}},
Elast

x86opc_vex_insn x86_group_vex_insns[][8] = {
/* 0 - GROUP_0FAE */
{
{0},
{0},
{"vldmxcsr", _0f, {Md}},
{"vstmxcsr", _0f, {Md}},
{0},
{0},
{0},
{0},
},
/* 1 - GROUP_660F71 */
{
{0},
{0},
{"vpsrlw", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
{"vpsraw", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
{"vpsllw", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
},
/* 2 - GROUP_660F72 */
{
{0},
{0},
{"vpsrld", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
{"vpsrad", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
{"vpslld", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
},
/* 3 - GROUP_660F73 */
{
{0},
{0},
{"vpsrlq", _128|_66|_0f, {VVo, VRo, Ib}},
{"vpsrldq", _128|_66|_0f, {VVo, VRo, Ib}},
{0},
{0},
{"vpsllq", _128|_66|_0f, {VVo, VRo, Ib}},
{"vpslldq", _128|_66|_0f, {VVo, VRo, Ib}},
},
/* 4 - GROUP_0F25_12 */
{
{"llwpcb", _128|_0f25|W0, {Rw}},
{"slwpcb", _128|_0f25|W0, {Rw}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 5 - GROUP_0F25_12_L */
{
{"llwpcb", _256|_0f25|W0, {Rd}},
{"slwpcb", _256|_0f25|W0, {Rd}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 6 - GROUP_0F25_12_W */
{
{"llwpcb", _128|_0f25|W1, {Rq}},
{"slwpcb", _128|_0f25|W1, {Rq}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 7 - GROUP_0FA_12 */
{
{"lwpins", _128|_0fA, {RVw, Ed, Iw}},
{"lwpval", _128|_0fA, {RVw, Ed, Iw}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 8 - GROUP_0FA_12_L */
{
{"lwpins", _256|_0fA, {RVd, Ed, Id}},
{"lwpval", _256|_0fA, {RVd, Ed, Id}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 9 - GROUP_0FA_12_W */
{
{"lwpins", _128|_0fA|W1, {RVq, Ed, Id}},
{"lwpval", _128|_0fA|W1, {RVq, Ed, Id}},
{0},
{0},
{0},
{0},
{0},
{0},
},
/* 12 - GROUP_0F38_F3 */
{
{0},
{"blsr", _128|_0f38|W0, {RVd, Ed}},
{"blsmsk", _128|_0f38|W0, {RVd, Ed}},
{"blsi", _128|_0f38|W0, {RVd, Ed}},
{0},
{0},
{0},
{0},
},
/* 11 - GROUP_0F38_F3_W */
{
{0},
{"blsr", _128|_0f38|W1, {RVq, Eq}},
{"blsmsk", _128|_0f38|W1, {RVq, Eq}},
{"blsi", _128|_0f38|W1, {RVq, Eq}},
{0},
{0},
{0},
{0},
},
};

x86opc_vex_insn *x86_vex_insns[256] = {
v00, v01, v02, v03, v04, v05, v06, v07,
v08, v09, v0a, v0b, v0c, v0d, v0e, v0f,
v10, v11, v12, v13, v14, v15, v16, v17,
v18, v19, v1a,   0, v1c, v1d, v1e,   0,
v20, v21, v22, v23, v24, v25,   0,   0,
v28, v29, v2a, v2b, v2c, v2d, v2e, v2f,
v30, v31, v32, v33, v34, v35, v36, v37,
v38, v39, v3a, v3b, v3c, v3d, v3e, v3f,
v40, v41, v42,   0, v44, v45, v46, v47,
v48, v49, v4a, v4b, v4c,   0,   0,   0,
v50, v51, v52, v53, v54, v55, v56, v57,
v58, v59, v5a, v5b, v5c, v5d, v5e, v5f,
v60, v61, v62, v63, v64, v65, v66, v67,
v68, v69, v6a, v6b, v6c, v6d, v6e, v6f,
v70, v71, v72, v73, v74, v75, v76, v77,
v78, v79, v7a, v7b, v7c, v7d, v7e, v7f,
v80, v81, v82, v83,   0, v85, v86, v87,
  0,   0,   0,   0, v8c,   0, v8e, v8f,
v90, v91, v92, v93, v94, v95, v96, v97,
v98, v99, v9a, v9b, v9c, v9d, v9e, v9f,
va0, va1, va2, va3,   0,   0, va6, va7,
va8, va9, vaa, vab, vac, vad, vae, vaf,
  0,   0,   0,   0,   0,   0, vb6, vb7,
vb8, vb9, vba, vbb, vbc, vbd, vbe, vbf,
vc0, vc1, vc2, vc3, vc4, vc5, vc6, vc7,
  0,   0,   0, vcb, vcc, vcd, vce, vcf,
vd0, vd1, vd2, vd3, vd4, vd5, vd6, vd7,
vd8, vd9, vda, vdb, vdc, vdd, vde, vdf,
ve0, ve1, ve2, ve3, ve4, ve5, ve6, ve7,
ve8, ve9, vea, veb, vec, ved, vee, vef,
vf0, vf1, vf2, vf3, vf4, vf5, vf6, vf7,
vf8, vf9, vfa, vfb, vfc, vfd, vfe,   0,
};
