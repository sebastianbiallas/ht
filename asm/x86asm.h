/* 
 *	The HT Editor
 *	x86asm.h
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

#ifndef __X86ASM_H__
#define __X86ASM_H__

#include "asm.h"
#include "x86opc.h"

struct x86asm_insn {
	char lockprefix;
	char repprefix;
	char segprefix;
	char n[16];
	char *name;
	x86_insn_op op[3];
};

struct x86addrcoding {
	int reg1;
	int reg2;
	int dispsize;
};

/*
 *	CLASS x86asm
 */

#define X86ASM_NULL     				0x00000000
#define X86ASM_ALLOW_AMBIGUOUS		0x00000001		/* IF SET: allow "mov [0], 1" 	ELSE: deny "mov [0], 1" (ambiguous) */

class x86asm: public Assembler {
public:
	int opsize;
	int addrsize;
protected:
	int esizes[3];

	int modrmv;
	int sibv;
	int disp;
	int dispsize;
	int imm;
	int imm2;
	int immsize;
	int address;
	bool ambiguous;
	bool namefound;
	bool addrsize_depend;

	void delete_nonsense();
	int delete_nonsense_insn(asm_code *c);
	void emitdisp(dword disp, int size);
	void emitfarptr(dword s, dword o, bool big);
	void emitimm(dword imm, int size);
	void emitmodrm(int modrm);
	void emitmodrm_mod(int mod);
	void emitmodrm_reg(int reg);
	void emitmodrm_rm(int rm);
	void emitsib_base(int base);
	void emitsib_index(int index);
	void emitsib_scale(int scale);
	int encode_insn(x86asm_insn *insn, x86opc_insn *opcode, int opcodeb, int additional_opcode, int prefix, int eopsize, int eaddrsize);
	int encode_modrm(x86_insn_op *op, char size, int allow_reg, int allow_mem, int eopsize, int eaddrsize);
	int encode_modrm_v(x86addrcoding (*modrmc)[3][8], x86_insn_op *op, int mindispsize, int *mod, int *rm, int *dispsize);
	int encode_op(x86_insn_op *op, x86opc_insn_op *xop, int *esize, int eopsize, int eaddrsize);
	int encode_sib_v(x86_insn_op *op, int mindispsize, int *ss, int *index, int *base, int *mod, int *dispsize, int *disp);
	int esizeaddr(char c, int size);
	int esizeop(char c, int size);
	int fetch_number(char **s, dword *value);
	char flsz2hsz(int size);
	char *immlsz2hsz(int size, int opsize);
	char *lsz2hsz(int size, int opsize);
	bool match_allops(x86asm_insn *insn, x86opc_insn *xinsn, int opsize, int addrsize);
	void match_fopcodes(x86asm_insn *insn);
	void match_opcode(x86opc_insn *opcode, x86asm_insn *insn, int prefix, byte opcodebyte, int additional_opcode);
	int match_opcode_name(char *input_name, const char *opcodelist_name);
	int match_opcode_final(x86opc_insn *opcode, x86asm_insn *insn, int prefix, byte opcodebyte, int additional_opcode, int opsize, int addrsize);
	void match_opcodes(x86opc_insn *opcodes, x86asm_insn *insn, int prefix);
	bool match_size(x86_insn_op *op, x86opc_insn_op *xop, int opsize);
	bool match_type(x86_insn_op *op, x86opc_insn_op *xop, int addrsize);
	bool opfarptr(x86_insn_op *op, char *xop);
	bool opimm(x86_insn_op *op, char *xop);
	bool opplugimm(x86_insn_op *op, char *xop);
	bool opmem(x86asm_insn *asm_insn, x86_insn_op *op, char *xop);
	bool opreg(x86_insn_op *op, char *xop);
	bool opmmx(x86_insn_op *op, char *xop);
	bool opxmm(x86_insn_op *op, char *xop);
	bool opseg(x86_insn_op *op, char *xop);
	bool opspecialregs(x86_insn_op *op, char *xop);
	int simmsize(dword imm, int immsize);
	void splitstr(const char *s, char *name, char *op[3]);
public:
			x86asm(int opsize, int addrsize);
	virtual	~x86asm();

	virtual	asm_insn *alloc_insn();
	virtual	asm_code *encode(asm_insn *asm_insn, int options, CPU_ADDR cur_address);
	virtual	char *get_name();
	virtual	int translate_str(asm_insn *asm_insn, const char *s);
};

#endif /* __X86ASM_H__ */
