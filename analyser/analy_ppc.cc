/* 
 *	HT Editor
 *	analy_ppc.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include "analy_ppc.h"
#include "analy_register.h"
#include "ppcdis.h"
#include "htiobox.h"
#include "snprintf.h"

/*
 *
 */
void AnalyPPCDisassembler::init(Analyser *A)
{
	disasm = new PPCDisassembler();
	AnalyDisassembler::init(A);
}

/*
 *
 */
void AnalyPPCDisassembler::done()
{
	AnalyDisassembler::done();
}

OBJECT_ID AnalyPPCDisassembler::object_id() const
{
	return ATOM_ANALY_PPC;
}

/*
 *
 */
Address *AnalyPPCDisassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	Address *a;
	a = createAddress(((ppcdis_insn *)opcode)->op[((ppcdis_insn *)opcode)->ops-1].rel.mem);
	if (/*examine &&*/ analy->validAddress(a, scvalid)) {
		return a;
	}
	delete a;
	return new InvalidAddress();
}

Address *AnalyPPCDisassembler::createAddress(dword offset)
{
	return new AddressFlat32(offset);
}

/*
 *
 */
void AnalyPPCDisassembler::examineOpcode(OPCODE *opcode)
{
}

/*
 *
 */
branch_enum_t AnalyPPCDisassembler::isBranch(OPCODE *opcode)
{
	// FIXME: needs work!!
	ppcdis_insn *ppc_insn = (ppcdis_insn *) opcode;
	if (ppc_insn->name[0]=='b') {
		if (ppc_insn->name[1]=='l') {
			if (ppc_insn->name[2]==0) {
				return br_call;
			}
			if (ppc_insn->name[2]=='r') {
				if (ppc_insn->name[3]=='l') {
					return br_call;
				} else {
					return br_return;
				}
			}
			return br_jXX;
		}
		if (ppc_insn->name[strlen(ppc_insn->name)] == 'l') {
				return br_call;
		}
		if (ppc_insn->name[1]==0) {
				return br_jump;
		}
		return br_jXX;
	}
/*	alphadis_insn *alpha_insn = (alphadis_insn *) opcode;
	if (alpha_insn->valid) {
		switch ((alpha_insn->table+alpha_insn->code)->type) {
			case ALPHA_GROUP_BRA:
				if (alpha_insn->table == alpha_instr_tbl) {
					switch (alpha_insn->code) {
						case 0x30:
							return br_jump;
						case 0x34:
							return br_call;
						default:
							if (alpha_insn->code > 0x30) return br_jXX;
					}
				}
				return br_nobranch;
			case ALPHA_GROUP_JMP: {
				switch (alpha_insn->code) {
					case 0:
					case 3:
					case 1:
						return br_call;
					case 2:
						return br_return;
				}
			}
		}
	}*/
	return br_nobranch;
}

