/* 
 *	HT Editor
 *	analy_x86.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation.
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

#include "analy_register.h"
#include "analy_x86.h"
#include "htdebug.h"
#include "x86dis.h"

#include <string.h>

/*
 *
 */
void analy_x86_disassembler::init(analyser *A, bool __16bit)
{
	_16bit = __16bit;
	analy_disassembler::init(A);
}

/*
 *
 */
int  analy_x86_disassembler::load(ht_object_stream *f)
{
	return analy_disassembler::load(f);
}

/*
 *
 */
void analy_x86_disassembler::done()
{
	analy_disassembler::done();
}

/*
 *
 */
OBJECT_ID	analy_x86_disassembler::object_id()
{
	return ATOM_ANALY_X86;
}

/*
 *
 */
ADDR	analy_x86_disassembler::branch_addr(OPCODE *opcode, tbranchtype branchtype, bool examine)
{
	ADDR Addr = INVALID_ADDR;
	x86dis_insn *o = (x86dis_insn*)opcode;
	assert(o->op[1].type == X86_OPTYPE_EMPTY);
	switch (o->op[0].type) {
		case X86_OPTYPE_IMM:
			Addr = o->op[0].imm;
			break;
		case X86_OPTYPE_FARPTR:
			Addr = o->op[0].farptr.seg * 0x10000 + o->op[0].farptr.offset;
			break;
		case X86_OPTYPE_MEM: {
			taccess access;
			if (o->op[0].mem.hasdisp) {
				Addr = o->op[0].mem.disp;
				access.type = acread;
				access.indexed = (o->op[0].mem.base != X86_REG_NO) || (o->op[0].mem.index != X86_REG_NO);
				access.size = o->op[0].size;
			}
			if (examine && analy->valid_addr(Addr, scvalid)) {
				analy->data_access(Addr, access);
				xref_type_t xref;
				switch (branchtype) {
					case brjXX:
					case brjump:
						xref = xrefijump;
						break;
					case brcall:
						xref = xreficall;
						break;
					default: {assert(0);}
				}
				analy->add_xref(Addr, analy->addr, xref);
			}
			if (examine) {
				return INVALID_ADDR;
			} else {
				return Addr;
			}
		}
	}
	return Addr;
}

/*
 *
 */
void	analy_x86_disassembler::examine_opcode(OPCODE *opcode)
{
	x86dis_insn *o = (x86dis_insn*)opcode;
	for (int i=0; i<3; i++) {
		x86_insn_op *op = &o->op[i];
		ADDR Addr = INVALID_ADDR;
		taccess access;
		xref_type_t xref = xrefoffset;
		switch (op->type) {
			case X86_OPTYPE_IMM:
				Addr = op->imm;
				access.type = acoffset;
				access.indexed = false;
				break;
			case X86_OPTYPE_FARPTR:
				Addr = op->farptr.seg * 0x10000 + op->farptr.offset;
				access.type = acoffset;
				access.indexed = false;
				break;
			case X86_OPTYPE_MEM:
				if (op->mem.hasdisp) {
					Addr = op->mem.disp;
					access.type = acread;
					access.indexed = (op->mem.base != X86_REG_NO) || (op->mem.index != X86_REG_NO);
					access.size = op->size;
					if (strcmp(o->name, "cmp")==0 || strcmp(o->name, "test")==0 || strcmp(o->name, "push")==0) {
						xref = xrefread;
					} else {
						xref = (i==0) ? xrefwrite : xrefread;
					}
				}
				break;
		}
		if (Addr != INVALID_ADDR) {
			if (analy->valid_addr(Addr, scvalid)) {
				analy->data_access(Addr, access);
				analy->add_xref(Addr, analy->addr, xref);
			}
		}
	}
}
/*
 *
 */
void analy_x86_disassembler::init_disasm()
{
	DPRINTF("analy_x86_disassembler: initing x86dis\n");
	// FIXME: better impl ?
	if (_16bit) {
		disasm = new x86dis(X86_OPSIZE16, X86_ADDRSIZE16);
	} else {
		disasm = new x86dis(X86_OPSIZE32, X86_ADDRSIZE32);
	}
	if (analy) analy->set_disasm(disasm);
}

/*
 *
 */
tbranchtype analy_x86_disassembler::is_branch(OPCODE *opcode)
{
	x86dis_insn *o = (x86dis_insn*)opcode;
	char *opcode_str = o->name;
	if (opcode_str[0]=='j') {
		if (opcode_str[1]=='m') return brjump; else return brjXX;
	} else if ((opcode_str[0]=='l') && (opcode_str[1]=='o')  && (opcode_str[2]=='o')) {
		// loop opcode will be threated like a jXX
		return brjXX;
	} else if (strncmp("call", opcode_str, 4)==0) {
		return brcall;
	} else if (strncmp("ret", opcode_str, 3)==0) {
		return brreturn;
	} else return brnobranch;
}

/*
 *
 */
void analy_x86_disassembler::store(ht_object_stream *f)
{
	analy_disassembler::store(f);
}


