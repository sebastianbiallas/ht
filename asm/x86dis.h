/* 
 *	The HT Editor
 *	x86dis.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __X86DIS_H__
#define __X86DIS_H__

#include "asm.h"
#include "x86opc.h"

#define X86DIS_OPCODE_CLASS_STD		0		/* no prefix */
#define X86DIS_OPCODE_CLASS_EXT		1		/* 0F */
#define X86DIS_OPCODE_CLASS_EXTEXT		2		/* 0F0F */

/* x86-specific styles */
#define X86DIS_STYLE_EXPLICIT_MEMSIZE	0x00000001		/* IF SET: mov word ptr [0000], ax 	ELSE: mov [0000], ax */
#define X86DIS_STYLE_OPTIMIZE_ADDR		0x00000002		/* IF SET: mov [eax*3], ax 			ELSE: mov [eax+eax*2+00000000], ax */
/*#define X86DIS_STYLE_USE16			0x00000004
#define X86DIS_STYLE_USE32			0x00000008*/

struct x86dis_insn {
	bool invalid;
	char lockprefix;
	char repprefix;
	char segprefix;
	byte size;
	int opcode;
	int opcodeclass;
	int eopsize;
	int eaddrsize;
	char *name;
	x86_insn_op op[3];
};

/*
 *	CLASS x86dis
 */

class x86dis: public disassembler {
protected:
	x86dis_insn insn;
	int opsize, addrsize;
	char insnstr[256];
/* initme! */
	unsigned char *codep, *ocodep;
	int addr;
	byte c;
	int modrm;
	int sib;
	int maxlen;

/* new */
			void decode_insn(x86opc_insn *insn);
			void decode_modrm(x86_insn_op *op, char size, int allow_reg, int allow_mem, int mmx);
			void decode_op(x86_insn_op *op, x86opc_insn_op *xop);
			void decode_sib(x86_insn_op *op, int mod);
			int esizeaddr(char c);
			int esizeop(char c);
			byte getbyte();
			word getword();
			dword getdword();
			int getmodrm();
			int getsib();
			void invalidate();
			int isfloat(char c);
			void prefixes();
			int special_param_ambiguity(x86dis_insn *disasm_insn);
			void str_format(char **str, char **format, char *p, char *n, char *op[3], int oplen[3], char stopchar, int print);
	virtual	void str_op(char *opstr, int *opstrlen, x86dis_insn *insn, x86_insn_op *op, bool explicit_params);
public:
	x86dis();
	x86dis(int opsize, int addrsize);
	virtual ~x86dis();

/* overwritten */
	virtual dis_insn *decode(byte *code, byte maxlen, CPU_ADDR addr);
	virtual int getmaxopcodelength();
	virtual char *get_name();
	virtual byte getsize(dis_insn *disasm_insn);
		   int load(ht_object_stream *f);
	virtual OBJECT_ID object_id();
	virtual char *str(dis_insn *disasm_insn, int options);
	virtual char *strf(dis_insn *disasm_insn, int options, char *format);
	virtual void store(ht_object_stream *f);
	virtual bool valid_insn(dis_insn *disasm_insn);
};

#endif /* __X86DIS_H__ */
