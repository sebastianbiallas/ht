/* 
 *	The HT Editor
 *	x86asm.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
 *	Copyright (C) 2005-2007 Sebastian Biallas (sb@biallas.net)
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
#include "x86dis.h"

struct x86asm_insn {
	char lockprefix;
	char repprefix;
	char segprefix;
	char opsizeprefix;
	char n[32];
	char *name;
	x86_insn_op op[5];
};

struct x86addrcoding {
	int reg1;
	int reg2;
	int dispsize;
};

/*
 *	CLASS x86asm
 */

#define X86ASM_NULL			0x00000000
#define X86ASM_ALLOW_AMBIGUOUS		0x00000001		/* IF SET: allow "mov [0], 1" 	ELSE: deny "mov [0], 1" (ambiguous) */

class x86asm: public Assembler {
public:
	X86OpSize opsize;
	X86AddrSize addrsize;
protected:
	int esizes[5];

	int modrmv;
	int sibv;
	int vexvvvv;
	uint64 disp;
	int dispsize;
	int disppos;
	uint64 imm;
	int imm2;
	int immsize;
	uint8 rexprefix; // 0 = no, 0x80 = forbid, 0x40 = yes
	int drexdest;
	int drexoc0;
	uint64 address;
	bool ambiguous;
	bool namefound;
	static x86opc_insn (*x86_32a_insns)[256];
	x86opc_insn (*x86_insns)[256];

		void prepInsns();
	virtual x86dis *createCompatibleDisassembler();
	void delete_nonsense(CPU_ADDR addr);
	bool delete_nonsense_insn(asm_code *c, x86dis *dis, CPU_ADDR addr);
	void emitdisp(uint64 disp, int size);
	void emitfarptr(uint32 s, uint32 o, bool big);
	void emitimm(uint64 imm, int size);
	void emitmodrm(int modrm);
	void emitmodrm_mod(int mod);
	void emitmodrm_reg(int reg);
	void emitmodrm_rm(int rm);
	void emitsib_base(int base);
	void emitsib_index(int index);
	void emitsib_scale(int scale);
	bool encode_insn(x86asm_insn *insn, x86opc_insn *opcode, int opcodeb, int additional_opcode, int prefix, int eopsize, int eaddrsize);
	bool encode_vex_insn(x86asm_insn *insn, x86opc_vex_insn *opcode, int opcodeb, int additional_opcode, int opsize, int addrsize);
	bool encode_modrm(x86_insn_op *op, char size, bool allow_reg, bool allow_mem, int eopsize, int eaddrsize);
	bool encode_modrm_v(const x86addrcoding (*modrmc)[3][8], x86_insn_op *op, int mindispsize, int *mod, int *rm, int *dispsize);
	bool encode_op(x86_insn_op *op, x86opc_insn_op *xop, int *esize, int eopsize, int eaddrsize);
	bool encode_sib_v(x86_insn_op *op, int mindispsize, int *ss, int *index, int *base, int *mod, int *dispsize, int *disp);
	int esizeop(uint c, int size);
	int esizeop_ex(uint c, int size);
	char flsz2hsz(int size);
	const char *immlsz2hsz(int size, int opsize);
	const char *lsz2hsz(int size, int opsize);
	int match_allops(x86asm_insn *insn, byte *xop, int maxops, int opsize, int addrsize);
	void match_fopcodes(x86asm_insn *insn);
	void match_opcode(x86opc_insn *opcode, x86asm_insn *insn, int prefix, byte opcodebyte, int additional_opcode, int def_match);
	void match_vex_opcode(x86opc_vex_insn *opcode, x86asm_insn *insn, byte opcodebyte, int additional_opcode);
	int match_opcode_name(const char *input_name, const char *opcodelist_name, int def_match);
	int match_opcode_final(x86opc_insn *opcode, x86asm_insn *insn, int prefix, byte opcodebyte, int additional_opcode, int opsize, int addrsize, int match);
	int match_vex_opcode_final(x86opc_vex_insn *opcode, x86asm_insn *insn, byte opcodebyte, int additional_opcode, int opsize, int addrsize);
	void match_opcodes(x86opc_insn *opcodes, x86asm_insn *insn, int prefix, int def_match);
	void match_vex_opcodes(x86asm_insn *insn);
	bool match_size(x86_insn_op *op, x86opc_insn_op *xop, int opsize);
	int match_type(x86_insn_op *op, x86opc_insn_op *xop, int addrsize);
	bool opfarptr(x86_insn_op *op, const char *xop);
	bool opimm(x86_insn_op *op, const char *xop);
	bool opplugimm(x86_insn_op *op, const char *xop);
	bool opmem(x86asm_insn *insn, x86_insn_op *op, const char *xop);
	virtual bool opreg(x86_insn_op *op, const char *xop);
	bool opmmx(x86_insn_op *op, const char *xop);
	virtual bool opxmm(x86_insn_op *op, const char *xop);
	virtual bool opymm(x86_insn_op *op, const char *xop);
	bool opseg(x86_insn_op *op, const char *xop);
	bool opspecialregs(x86_insn_op *op, const char *xop);
	int simmsize(uint64 imm, int immsize);
	void splitstr(const char *s, char *name, int size, char *op[5], int opsize);
	void tok(const char **s, char *res, int reslen, const char *sep);
public:
		x86asm(X86OpSize opsize, X86AddrSize addrsize);

	virtual	asm_insn *alloc_insn();
	virtual	asm_code *encode(asm_insn *asm_insn, int options, CPU_ADDR cur_address);
	virtual	const char *get_name();
	virtual	bool translate_str(asm_insn *asm_insn, const char *s);
};


class x86_64asm: public x86asm {
	static x86opc_insn (*x86_64_insns)[256];
public:
	
		x86_64asm();
	virtual bool opreg(x86_insn_op *op, const char *xop);
	virtual bool opxmm(x86_insn_op *op, const char *xop);
	virtual bool opymm(x86_insn_op *op, const char *xop);
		void prepInsns();
protected:
	virtual x86dis *createCompatibleDisassembler();
};


#endif /* __X86ASM_H__ */
