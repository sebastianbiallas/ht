/* 
 *	HT Editor
 *	analy_il.cc
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

#include "analy_il.h"
#include "analy_register.h"
#include "ildis.h"
#include "htiobox.h"
#include "snprintf.h"

void AnalyILDisassembler::init(Analyser *A, char* (*string_func)(uint32 string_ofs, void *context), char* (*token_func)(uint32 token, void *context), void *context)
{
	disasm = new ILDisassembler(string_func, token_func, context);
	AnalyDisassembler::init(A);
}

ObjectID AnalyILDisassembler::getObjectID() const
{
	return ATOM_ANALY_IL;
}

/*
 *
 */
Address *AnalyILDisassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	return new InvalidAddress();
}

Address *AnalyILDisassembler::createAddress(uint32 offset)
{
	return new AddressFlat32(offset);
}

/*
 *
 */
void AnalyILDisassembler::examineOpcode(OPCODE *opcode)
{
}

/*
 *
 */
branch_enum_t AnalyILDisassembler::isBranch(OPCODE *opcode)
{
	return br_nobranch;
}
