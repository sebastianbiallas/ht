/* 
 *	HT Editor
 *	analy_avr.cc
 *
 *	Copyright (C) 2008 Sebastian Biallas (sb@biallas.net)
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

#include "analy_avr.h"
#include "analy_register.h"
#include "avrdis.h"
#include "htiobox.h"
#include "snprintf.h"

void AnalyAVRDisassembler::init(Analyser *A)
{
	disasm = new AVRDisassembler();
	AnalyDisassembler::init(A);
}

ObjectID AnalyAVRDisassembler::getObjectID() const
{
	return ATOM_ANALY_AVR;
}

/*
 *
 */
Address *AnalyAVRDisassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	Address *a;
	a = createAddress(((avrdis_insn *)opcode)->op[0].imm);
	if (/*examine &&*/ analy->validAddress(a, scvalid)) {
		return a;
	}
	delete a;
	return new InvalidAddress();
}

Address *AnalyAVRDisassembler::createAddress(uint64 offset)
{
	return new AddressFlat32(offset);
}

/*
 *
 */
void AnalyAVRDisassembler::examineOpcode(OPCODE *opcode)
{
}

/*
 *
 */
branch_enum_t AnalyAVRDisassembler::isBranch(OPCODE *opcode)
{
	avrdis_insn *avr_insn = (avrdis_insn *) opcode;
	const char *name = avr_insn->name;
	if (strcmp(name, "call") == 0
	 || strcmp(name, "rcall") == 0) {
	    return br_call;
	}
	if (strcmp(name, "jmp") == 0
	 || strcmp(name, "rjmp") == 0) {
	    return br_jump;
	}
	if (strcmp(name, "ret") == 0
	 || strcmp(name, "reti") == 0) {
	    return br_return;
	}

	if (strcmp(name, "brcc") == 0
	 || strcmp(name, "brcs") == 0
	 || strcmp(name, "breq") == 0
	 || strcmp(name, "brge") == 0
	 || strcmp(name, "brhc") == 0
	 || strcmp(name, "brhs") == 0
	 || strcmp(name, "brid") == 0
	 || strcmp(name, "brie") == 0
	 || strcmp(name, "brlo") == 0
	 || strcmp(name, "brlt") == 0
	 || strcmp(name, "brmi") == 0
	 || strcmp(name, "brne") == 0
	 || strcmp(name, "brpl") == 0
	 || strcmp(name, "brsh") == 0
	 || strcmp(name, "brtc") == 0
	 || strcmp(name, "brts") == 0
	 || strcmp(name, "brvc") == 0
	 || strcmp(name, "brvs") == 0) {
	    return br_jXX;
	}
#if 0
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
		if (ppc_insn->name[1] == 0 || strcmp(ppc_insn->name, "bctr") == 0) {
				return br_jump;
		}
		return br_jXX;
	}
#endif
	return br_nobranch;
}

