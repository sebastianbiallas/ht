/* 
 *	HT Editor
 *	analy_il.cc
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

#include "analy_il.h"
#include "analy_register.h"
#include "ildis.h"
#include "htiobox.h"
#include "snprintf.h"

/*AddressAlphaFlat32::AddressAlphaFlat32()
{
}

AddressAlphaFlat32::AddressAlphaFlat32(dword Addr)
{
	addr = Addr;
}

bool AddressAlphaFlat32::add(int offset)
{
	// check for overflow
	if ((int)offset < 0) {
		if (addr+offset > addr) return false;
	} else {
		if (addr+offset < addr) return false;
	}
	addr+=offset;
	return true;
}

int AddressAlphaFlat32::byteSize()
{
	return 4;
}

int AddressAlphaFlat32::compareTo(Object *to)
{
	assert(object_id() == to->object_id());
	return addr-((AddressAlphaFlat32 *)to)->addr;
}

int AddressAlphaFlat32::compareDelinear(Address *to)
{
	assert(object_id() == to->object_id());
	return delinearize(addr)-delinearize(((AddressFlat32 *)to)->addr);
}

bool AddressAlphaFlat32::difference(int &result, Address *to)
{
	if (object_id() == to->object_id()) {
		result = addr-((AddressAlphaFlat32 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

Object *AddressAlphaFlat32::duplicate()
{
	return new AddressAlphaFlat32(addr);
}

void AddressAlphaFlat32::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(dword*)array);
}

void AddressAlphaFlat32::getFromCPUAddress(CPU_ADDR *ca)
{
	addr = ca->addr32.offset;
}

int AddressAlphaFlat32::load(ht_object_stream *s)
{
	addr = s->getIntHex(4, NULL);
	return s->get_error();
}

OBJECT_ID AddressAlphaFlat32::object_id()
{
	return ATOM_ADDRESS_ALPHA_FLAT_32;
}

int AddressAlphaFlat32::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressAlphaFlat32::putIntoArray(byte *array)
{
	UNALIGNED_MOVE(*(dword*)array, addr);
}

void AddressAlphaFlat32::putIntoCPUAddress(CPU_ADDR *ca)
{
	ca->addr32.offset = addr;
}

void AddressAlphaFlat32::store(ht_object_stream *s)
{
	s->putIntHex(addr, 4, NULL);
}

int AddressAlphaFlat32::stringify(char *s, int max_length, int format)
{
	char *formats[] = {
		"%s%x%s",
		"%s%8x%s",
		"%s%08x%s",
		"",
		"%s%X%s",
		"%s%8X%s",
		"%s%08X%s",
		"",
	};
	return ht_snprintf(s, max_length, formats[format&7], (format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", addr, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"");
}

int AddressAlphaFlat32::stringSize()
{
	return 8;
} */

/*
 *
 */
void AnalyILDisassembler::init(Analyser *A, char* (*string_func)(dword string_ofs, void *context), char* (*token_func)(dword token, void *context), void *context)
{
	disasm = new ILDisassembler(string_func, token_func, context);
	AnalyDisassembler::init(A);
}

/*
 *
 */
void AnalyILDisassembler::done()
{
	AnalyDisassembler::done();
}

OBJECT_ID AnalyILDisassembler::object_id()
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

Address *AnalyILDisassembler::createAddress(dword offset)
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
