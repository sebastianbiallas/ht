/* 
 *	HT Editor
 *	analy_ia64.cc
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

#include <string.h>

#include "analy_ia64.h"
#include "analy_register.h"
#include "ia64dis.h"
#include "htiobox.h"
#include "snprintf.h"

void AnalyIA64Disassembler::init(Analyser *A)
{
	disasm = new IA64Disassembler();
	AnalyDisassembler::init(A);
}

ObjectID AnalyIA64Disassembler::getObjectID() const
{
	return ATOM_ANALY_IA64;
}

/*
 *
 */
Address *AnalyIA64Disassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	IA64DisInsn *dis_insn = (IA64DisInsn*)opcode;
	IA64SlotDisInsn *slot = &dis_insn->slot[dis_insn->selected];
	for (int j=0; j<7; j++) {
		if (slot->op[j].type == IA64_OPERAND_ADDRESS) {
			Address *addr = new AddressFlat64(slot->op[j].imm);
			return addr;
		}
	}
	return new InvalidAddress();
}

Address *AnalyIA64Disassembler::createAddress(uint32 offset)
{
	return new AddressFlat32(offset);
}

/*
 *
 */
void AnalyIA64Disassembler::examineOpcode(OPCODE *opcode)
{
/*     IA64DisInsn *dis_insn = (IA64DisInsn*)opcode;
	IA64SlotDisInsn *slot = &dis_insn->slot[dis_insn->selected];
	for (int j=0; j<7; j++) {
		if (slot->op[j].type == IA64_OPERAND_ADDRESS) {
			Address *addr = new AddressFlat64(slot->op[j].imm);
			if (analy->validAddress(addr, scvalid)) {
				taccess access;
				xref_enum_t xref = xrefoffset;
				access.type = acoffset;
				access.indexed = false;
				analy->dataAccess(addr, access);
				analy->addXRef(addr, analy->addr, xref);
			}
		}
	}*/
}

/*
 *
 */
branch_enum_t AnalyIA64Disassembler::isBranch(OPCODE *opcode)
{
	IA64DisInsn *dis_insn = (IA64DisInsn *) opcode;
	IA64SlotDisInsn *slot = &dis_insn->slot[dis_insn->selected];
	if (ht_strncmp(slot->opcode->name, "br.", 3)==0) {
		if (ht_strncmp(slot->opcode->name+3, "call", 4)==0) {
			return br_call;
		} else if (ht_strncmp(slot->opcode->name+3, "cond", 4)==0) {
			if (slot->qp) {
				return br_jXX;
			} else {
				return br_jump;
			}
		} else if (ht_strncmp(slot->opcode->name+3, "cloop", 5)==0) {
			return br_jXX;
		} else if (ht_strncmp(slot->opcode->name+3, "ret", 3)==0) {
			if (!slot->qp) return br_return;
		}
	}
	return br_nobranch;
}
