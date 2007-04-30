/* 
 *	HT Editor
 *	analy_alpha.cc
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

#include "analy_alpha.h"
#include "analy_register.h"
#include "alphadis.h"
#include "htiobox.h"
#include "snprintf.h"

/*
 *
 */
AnalyAlphaDisassembler::AnalyAlphaDisassembler()
{
}

void AnalyAlphaDisassembler::init(Analyser *A)
{
	disasm = new Alphadis();
	AnalyDisassembler::init(A);
}

/*
 *
 */
void AnalyAlphaDisassembler::load(ObjectStream &f)
{
	return AnalyDisassembler::load(f);
}

/*
 *
 */
void AnalyAlphaDisassembler::done()
{
	AnalyDisassembler::done();
}

ObjectID AnalyAlphaDisassembler::getObjectID() const
{
	return ATOM_ANALY_ALPHA;
}

/*
 *
 */
Address *AnalyAlphaDisassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	Address *a = createAddress(((alphadis_insn *)opcode)->data);
	if (examine && analy->validAddress(a, scvalid)) {
		return a;
	}
	delete a;
	return new InvalidAddress();
}

Address *AnalyAlphaDisassembler::createAddress(uint32 offset)
{
	return new AddressFlat32(offset);
}

/*
 *
 */
void AnalyAlphaDisassembler::examineOpcode(OPCODE *opcode)
{
}

/*
 *
 */
branch_enum_t AnalyAlphaDisassembler::isBranch(OPCODE *opcode)
{
	// FIXME: needs work!!
	alphadis_insn *alpha_insn = (alphadis_insn *) opcode;
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
	}
	return br_nobranch;
}

/*
 *
 */
void AnalyAlphaDisassembler::store(ObjectStream &f) const
{
	AnalyDisassembler::store(f);
}

