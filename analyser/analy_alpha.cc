/* 
 *	HT Editor
 *	analy_alpha.cc
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

#include "analy_alpha.h"
#include "analy_register.h"
#include "alphadis.h"
#include "htiobox.h"

#include <string.h>

/*
 *
 */
void analy_alpha_disassembler::init(analyser *A)
{
	analy_disassembler::init(A);
}

/*
 *
 */
int  analy_alpha_disassembler::load(ht_object_stream *f)
{
	return analy_disassembler::load(f);
}

/*
 *
 */
void analy_alpha_disassembler::done()
{
	analy_disassembler::done();
}

OBJECT_ID analy_alpha_disassembler::object_id()
{
	return ATOM_ANALY_ALPHA;
}

/*
 *
 */
ADDR	analy_alpha_disassembler::branch_addr(OPCODE *opcode, tbranchtype branchtype, bool examine)
{
	if (examine && analy->valid_addr(((alphadis_insn *)opcode)->data, scvalid)) {
	}
	return ((alphadis_insn *)opcode)->data;
}

/*
 *
 */
void analy_alpha_disassembler::examine_opcode(OPCODE *opcode)
{
}

/*
 *
 */
void analy_alpha_disassembler::init_disasm()
{
	DPRINTF("analy_alpha_disassembler: initing alphadis\n");
	disasm = new alphadis();
	if (analy) analy->set_disasm(disasm);
}

/*
 *
 */
tbranchtype analy_alpha_disassembler::is_branch(OPCODE *opcode)
{
	// FIXME: needs work!!
	alphadis_insn *alpha_insn = (alphadis_insn *) opcode;
	if (alpha_insn->valid) {
		switch ((alpha_insn->table+alpha_insn->code)->type) {
			case ALPHA_GROUP_BRA:
				if (alpha_insn->table == alpha_instr_tbl) {
					switch (alpha_insn->code) {
						case 0x30:
							return brjump;
						case 0x34:
							return brcall;
						default:
							if (alpha_insn->code > 0x30) return brjXX;
					}
				}
				return brnobranch;
			case ALPHA_GROUP_JMP: {
				switch (alpha_insn->code) {
					case 0:
					case 3:
					case 1:
						return brcall;
					case 2:
						return brreturn;
				}
			}
		}
	}
	return brnobranch;
}

/*
 *
 */
void analy_alpha_disassembler::store(ht_object_stream *f)
{
	analy_disassembler::store(f);
}

