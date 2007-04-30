/* 
 *	HT Editor
 *	analy_java.cc
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

#include "analy_register.h"
#include "analy_java.h"
#include "htdebug.h"
#include "javadis.h"
#include "strtools.h"
#include "snprintf.h"

void AnalyJavaDisassembler::init(Analyser *A, java_token_func token_func, void *context)
{
	disasm = new javadis(token_func, context);
	AnalyDisassembler::init(A);
}

/*
 *
 */
ObjectID AnalyJavaDisassembler::getObjectID() const
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

static uint32 read4(Analyser *a, Address &addr, byte *b)
{
	if (a->bufPtr(&addr, b, 4) != 4) throw 1;
	addr.add(4);
	return (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3];
}

void AnalyJavaDisassembler::examineOpcode(OPCODE *opcode)
{
	javadis_insn *o = (javadis_insn*)opcode;
	const char *opcode_str = o->name;
	if (strcmp("lookupswitch", opcode_str) == 0) {
		uint32 ofs = o->addr;
		AddressFlat32 c(ofs);
		Location *loc = analy->getFunctionByAddress(&c);
		if (loc) {
			AddressFlat32 *func = (AddressFlat32 *)loc->addr;
			uint32 f = func->addr;
			ofs = ofs+1 - f;
			while (ofs % 4) {
				AddressFlat32 iofs(f+ofs);
				analy->data->setIntAddressType(&iofs, dst_ibyte, 1);
				ofs++;
			}
			AddressFlat32 iofs(f + ofs);
			byte buf[4];
			try {
				analy->data->setIntAddressType(&iofs, dst_idword, 4);
				uint32 default_ofs = read4(analy, iofs, buf);
				analy->data->setIntAddressType(&iofs, dst_idword, 4);
				uint32 n = read4(analy, iofs, buf);
			
				for (uint32 i=0; i < n; i++) {
					char buffer[100];
					analy->data->setIntAddressType(&iofs, dst_idword, 4);
					uint32 value = read4(analy, iofs, buf);
					analy->data->setIntAddressType(&iofs, dst_idword, 4);
					uint32 rel_ofs = read4(analy, iofs, buf);
					AddressFlat32 a(o->addr + rel_ofs);
					if (!analy->getLocationByAddress(&a)) analy->addComment(&a, 0, "");
					ht_snprintf(buffer, sizeof buffer, "lookupswitch value: %d", value);
					analy->addComment(&a, 0, buffer);
					analy->addAddressSymbol(&a, "lookup", label_loc, loc);
					analy->pushAddress(&a, func);
					analy->addXRef(&a, &c, xrefijump);
				}
				AddressFlat32 a(o->addr + default_ofs);
				analy->addComment(&a, 0, "");
				analy->addComment(&a, 0, "lookupswitch default");
				analy->addAddressSymbol(&a, "lookup", label_loc, loc);
				analy->pushAddress(&a, func);
				analy->addXRef(&a, &c, xrefijump);
			} catch (...) {
				return;
			}
		}
	} else if (strcmp("tableswitch", opcode_str) == 0) {
		uint32 ofs = o->addr;
		AddressFlat32 c(ofs);
		Location *loc = analy->getFunctionByAddress(&c);
		if (loc) {
			AddressFlat32 *func = (AddressFlat32 *)loc->addr;
			uint32 f = func->addr;
			ofs = ofs+1 - f;
			while (ofs % 4) {
				AddressFlat32 iofs(f+ofs);
				analy->data->setIntAddressType(&iofs, dst_ibyte, 1);
				ofs++;
			}
			AddressFlat32 iofs(f + ofs);
			byte buf[4];
			try {
				analy->data->setIntAddressType(&iofs, dst_idword, 4);
				uint32 default_ofs = read4(analy, iofs, buf);
				analy->data->setIntAddressType(&iofs, dst_idword, 4);
				sint32 low = read4(analy, iofs, buf);
				analy->data->setIntAddressType(&iofs, dst_idword, 4);
				sint32 high = read4(analy, iofs, buf);
			
				for (sint32 i=low; i <= high; i++) {
					char buffer[100];
					analy->data->setIntAddressType(&iofs, dst_idword, 4);
					uint32 rel_ofs = read4(analy, iofs, buf);
					AddressFlat32 a(o->addr + rel_ofs);
					if (!analy->getLocationByAddress(&a)) analy->addComment(&a, 0, "");
					ht_snprintf(buffer, sizeof buffer, "tableswitch value: %d", i);
					analy->addComment(&a, 0, buffer);
					analy->addAddressSymbol(&a, "table", label_loc, loc);
					analy->pushAddress(&a, func);
					analy->addXRef(&a, &c, xrefijump);
				}
				AddressFlat32 a(o->addr + default_ofs);
				analy->addComment(&a, 0, "");
				analy->addComment(&a, 0, "tableswitch default");
				analy->addAddressSymbol(&a, "table", label_loc, loc);
				analy->pushAddress(&a, func);
				analy->addXRef(&a, &c, xrefijump);
			} catch (...) {
				return;
			}
		}
	}
}

/*
 *
 */
branch_enum_t AnalyJavaDisassembler::isBranch(OPCODE *opcode)
{
	javadis_insn *o = (javadis_insn*)opcode;
	const char *opcode_str = o->name;
	if ((opcode_str[0]=='i') && (opcode_str[1]=='f')) {
		return br_jXX;
	} else if ((strcmp("tableswitch", opcode_str)==0)
	|| (strcmp("lookupswitch", opcode_str)==0)) {
		examineOpcode(opcode);
		return br_jump;
	} else if (ht_strncmp("ret", opcode_str, 3)==0
	|| ht_strncmp("ret", opcode_str+1, 3)==0
	|| ht_strncmp("athrow", opcode_str, 6)==0) {
		return br_return;
	} else if (ht_strncmp("goto", opcode_str, 4)==0) {
		return br_jump;
	} else if (ht_strncmp("jsr", opcode_str, 3)==0) {
		return br_call;
	} else return br_nobranch;
}
