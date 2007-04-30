/*
 *	HT Editor
 *	ia64dis.h
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

#ifndef IA64DIS_H
#define IA64DIS_H

#include "asm.h"
#include "io/types.h"
#include "ia64opc.h"

#define IA64_OPERAND_NO		0
#define IA64_OPERAND_EQUALS	1
#define IA64_OPERAND_1		2
#define IA64_OPERAND_REG	3
#define IA64_OPERAND_AREG	4
#define IA64_OPERAND_BREG	5
#define IA64_OPERAND_FREG	6
#define IA64_OPERAND_PREG	7
#define IA64_OPERAND_AR_PFS	8
#define IA64_OPERAND_AR_CCV	9
#define IA64_OPERAND_MEM_REG	10
#define IA64_OPERAND_IMM	11
#define IA64_OPERAND_ADDRESS	12
#define IA64_OPERAND_REG_FILE	13
#define IA64_OPERAND_PRALL	14
#define IA64_OPERAND_PRROT	15
#define IA64_OPERAND_IP		16

struct IA64Op {
	int type;
	union {
		int reg;
		uint64 imm;
		uint64 ofs;
		struct {
			int db;
			int idx;
		} regfile;
	};
};

struct IA64SlotDisInsn {
	bool			valid;
	int			next;
	uint64			data;
	uint32			qp;
	IA64OpcodeEntry *	opcode;
	IA64Op			op[7];
};

struct IA64DisInsn {
	bool			valid;
	int			size;
	byte			data[16];
	IA64Template		*tmplt;
	byte			tmplt_idx;       /* template bits */

	int			selected;
	IA64SlotDisInsn		slot[3];
};

/*
 *	CLASS alphadis
 */

class IA64Disassembler: public Disassembler {
protected:
	CPU_ADDR cpu_addr;
	char insnstr[256];
	IA64DisInsn	insn;
public:
			IA64Disassembler() {};
			IA64Disassembler(BuildCtorArg&a): Disassembler(a) {};

	virtual	dis_insn	*decode(byte *code, int maxlen, CPU_ADDR addr);
	virtual	dis_insn	*duplicateInsn(dis_insn *disasm_insn);
	virtual	void		getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align);
	virtual	byte		getSize(dis_insn *disasm_insn);
	virtual	const char	*getName();
	virtual	bool		selectNext(dis_insn *disasm_insn);
	virtual	const char	*str(dis_insn *disasm_insn, int style);
	virtual	const char	*strf(dis_insn *disasm_insn, int style, const char *format);
	virtual	ObjectID	getObjectID() const;
	virtual	bool		validInsn(dis_insn *disasm_insn);
private:
		void		decodeSlot(int slot_nb);
		uint64		signExtend(uint64 a, int length);
};

#endif 
