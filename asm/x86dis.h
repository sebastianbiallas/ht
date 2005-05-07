/* 
 *	HT Editor
 *	x86dis.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
 *	Copyright (C) 2005 Sebastian Biallas (sb@biallas.net)
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
#define X86DIS_OPCODE_CLASS_EXT_F2	2		/* F2 0F */
#define X86DIS_OPCODE_CLASS_EXT_F3	3		/* F3 0F */
#define X86DIS_OPCODE_CLASS_EXTEXT	4		/* 0F 0F */

/* x86-specific styles */
#define X86DIS_STYLE_EXPLICIT_MEMSIZE	0x00000001	/* IF SET: mov word ptr [0000], ax 	ELSE: mov [0000], ax */
#define X86DIS_STYLE_OPTIMIZE_ADDR	0x00000002	/* IF SET: mov [eax*3], ax 		ELSE: mov [eax+eax*2+00000000], ax */
/*#define X86DIS_STYLE_USE16		0x00000004
#define X86DIS_STYLE_USE32		0x00000008*/

struct x86dis_insn {
	bool invalid;
	char opsizeprefix;
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

class x86dis: public Disassembler {
public:
	int opsize, addrsize;
protected:
	x86dis_insn insn;
	char insnstr[256];
/* initme! */
	unsigned char *codep, *ocodep;
	int seg;
	int addr; // FIXME: int??
	byte c;
	int modrm;
	int sib;
	int maxlen;

/* new */
			void	decode_insn(x86opc_insn *insn);
			void	decode_modrm(x86_insn_op *op, char size, bool allow_reg, bool allow_mem, bool mmx, bool xmm);
			void	decode_op(x86_insn_op *op, x86opc_insn_op *xop);
			void	decode_sib(x86_insn_op *op, int mod);
			int	esizeaddr(char c);
			int	esizeop(char c);
			byte	getbyte();
			word	getword();
			dword	getdword();
			int	getmodrm();
			int	getsib();
			void	invalidate();
			bool	isfloat(char c);
			void	prefixes();
			int	special_param_ambiguity(x86dis_insn *disasm_insn);
			void	str_format(char **str, char **format, char *p, char *n, char *op[3], int oplen[3], char stopchar, int print);
	virtual		void	str_op(char *opstr, int *opstrlen, x86dis_insn *insn, x86_insn_op *op, bool explicit_params);
public:
				x86dis();
				x86dis(int opsize, int addrsize);
	virtual			~x86dis();

/* overwritten */
	virtual	dis_insn *	decode(byte *code, int maxlen, CPU_ADDR addr);
	virtual	dis_insn *	duplicateInsn(dis_insn *disasm_insn);
	virtual	void		getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align);
	virtual	char *		getName();
	virtual	byte		getSize(dis_insn *disasm_insn);
		int		load(ht_object_stream *f);
	virtual OBJECT_ID	object_id() const;
	virtual char *		str(dis_insn *disasm_insn, int options);
	virtual char *		strf(dis_insn *disasm_insn, int options, char *format);
	virtual void		store(ht_object_stream *f);
	virtual bool		validInsn(dis_insn *disasm_insn);
};

class x86dis_vxd: public x86dis {
protected:
	virtual void str_op(char *opstr, int *opstrlen, x86dis_insn *insn, x86_insn_op *op, bool explicit_params);
public:
				x86dis_vxd();
				x86dis_vxd(int opsize, int addrsize);
	virtual			~x86dis_vxd();

	virtual dis_insn *	decode(byte *code, byte maxlen, CPU_ADDR addr);
	virtual OBJECT_ID	object_id() const;
};

#endif /* __X86DIS_H__ */
