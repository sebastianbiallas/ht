/* 
 *	HT Editor
 *	analy_java.cc
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

#include "analy_register.h"
#include "analy_java.h"
#include "htdebug.h"
#include "javadis.h"

#include <string.h>

/*
 *
 */
void AnalyJavaDisassembler::init(Analyser *A, java_token_func token_func, void *context)
{
	disasm = new javadis(token_func, context);
	AnalyDisassembler::init(A);
}

/*
 *
 */
int  AnalyJavaDisassembler::load(ht_object_stream *f)
{
	return AnalyDisassembler::load(f);
}

/*
 *
 */
void AnalyJavaDisassembler::done()
{
	AnalyDisassembler::done();
}

/*
 *
 */
OBJECT_ID	AnalyJavaDisassembler::object_id() const
{
	return ATOM_ANALY_JAVA;
}

/*
 *
 */

Address *AnalyJavaDisassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	javadis_insn *o = (javadis_insn*)opcode;
	switch (o->op[0].type) {
		case JAVA_OPTYPE_LABEL:
			return new AddressFlat32(o->op[0].label);
	}
	return new InvalidAddress();
}

/*
 *
 */
void AnalyJavaDisassembler::examineOpcode(OPCODE *opcode)
{
/*	javadis_insn *o = (javadis_insn*)opcode;
	for (int i=0; i<JAVAINSN_MAX_PARAM_COUNT; i++) {
		java_insn_op *op = &o->op[i];
		ADDR Addr = INVALID_ADDR;
		taccess access;
		xref_type_t xref = xrefoffset;
		switch (op->type) {
			case JAVA_OPTYPE_LABEL:
				Addr = op->label;
				access.type = acoffset;
				access.indexed = false;
				break;
		}
		if (Addr != INVALID_ADDR) {
			if (analy->valid_addr(Addr, scvalid)) {
				analy->data_access(Addr, access);
				analy->add_xref(Addr, analy->addr, xref);
			}
		}
	}*/
}

/*
 *
 */
branch_enum_t AnalyJavaDisassembler::isBranch(OPCODE *opcode)
{
	javadis_insn *o = (javadis_insn*)opcode;
	char *opcode_str = o->name;
	if ((opcode_str[0]=='i') && (opcode_str[1]=='f')) {
		return br_jXX;
	} else if ((strcmp("tableswitch", opcode_str)==0)
	|| (strcmp("lookupswitch", opcode_str)==0)) {
		return br_jXX;
	} else if (strncmp("ret", opcode_str, 3)==0
	|| strncmp("ret", opcode_str+1, 3)==0
	|| strncmp("athrow", opcode_str, 6)==0) {
		return br_return;
	} else if (strncmp("goto", opcode_str, 4)==0) {
		return br_jump;
	} else if (strncmp("jsr", opcode_str, 3)==0) {
		return br_call;
	} else return br_nobranch;
}

/*
 *
 */
void AnalyJavaDisassembler::store(ht_object_stream *f)
{
	AnalyDisassembler::store(f);
}


