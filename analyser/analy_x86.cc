/* 
 *	HT Editor
 *	analy_x86.cc
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

#include "analy_register.h"
#include "analy_x86.h"
#include "htdebug.h"
#include "snprintf.h"
#include "x86dis.h"

AddressX86Flat32::AddressX86Flat32()
{
}

AddressX86Flat32::AddressX86Flat32(dword Addr)
{
	addr = Addr;
}

bool AddressX86Flat32::add(int offset)
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

int AddressX86Flat32::byteSize()
{
	return 4;
}

int AddressX86Flat32::compareTo(const Object *obj) const
{
/*	if (object_id() != to->object_id()) {
		int as=1;
	}*/
	assert(object_id() == obj->object_id());
	if (addr > ((AddressX86Flat32 *)obj)->addr) return 1;
	if (addr < ((AddressX86Flat32 *)obj)->addr) return -1;
	return 0;
}

int AddressX86Flat32::compareDelinear(Address *to)
{
	assert(object_id() == to->object_id());
	dword da = delinearize(addr);
	dword db = delinearize(((AddressFlat32 *)to)->addr);
	if (da > db) return 1;
	if (da < db) return -1;
	return 0;
}

bool AddressX86Flat32::difference(int &result, Address *to)
{
	if (object_id() == to->object_id()) {
		result = addr-((AddressX86Flat32 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

Object *AddressX86Flat32::duplicate()
{
	return new AddressX86Flat32(addr);
}

void AddressX86Flat32::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(dword*)array);
}

void AddressX86Flat32::getFromCPUAddress(CPU_ADDR *ca)
{
	addr = ca->addr32.offset;
}

int AddressX86Flat32::load(ht_object_stream *s)
{
	addr = s->getIntHex(4, NULL);
	return s->get_error();
}

OBJECT_ID AddressX86Flat32::object_id() const
{
	return ATOM_ADDRESS_X86_FLAT_32;
}

int AddressX86Flat32::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressX86Flat32::putIntoArray(byte *array)
{
	UNALIGNED_MOVE(*(dword*)array, addr);
}

void AddressX86Flat32::putIntoCPUAddress(CPU_ADDR *ca)
{
	ca->addr32.offset = addr;
}

void AddressX86Flat32::store(ht_object_stream *s)
{
	s->putIntHex(addr, 4, NULL);
}

int AddressX86Flat32::stringify(char *s, int max_length, int format)
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

int AddressX86Flat32::stringSize()
{
	return 8;
}


AddressX86_1632::AddressX86_1632()
{
}

AddressX86_1632::AddressX86_1632(word Seg, dword Addr)
{
	seg = Seg;
	addr = Addr;
}

bool AddressX86_1632::add(int offset)
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

int AddressX86_1632::byteSize()
{
	return 6;
}

int AddressX86_1632::compareTo(const Object *obj) const
{
	assert(object_id() == obj->object_id());
	if (seg > ((AddressX86_1632 *)obj)->seg) return 1;
	if (seg < ((AddressX86_1632 *)obj)->seg) return -1;
	if (addr > ((AddressX86_1632 *)obj)->addr) return 1;
	if (addr < ((AddressX86_1632 *)obj)->addr) return -1;
	return 0;
}

int AddressX86_1632::compareDelinear(Address *to)
{
	assert(object_id() == to->object_id());
	dword s1 = delinearize(seg);
	dword s2 = delinearize(((AddressX86_1632 *)to)->seg);
	if (s1 > s2) return 1;
	if (s1 < s2) return -1;
	dword a1 = delinearize(addr);
	dword a2 = delinearize(((AddressX86_1632 *)to)->addr);
	if (a1 > a2) return 1;
	if (a1 < a2) return -1;
	return 0;
}

bool AddressX86_1632::difference(int &result, Address *to)
{
	if ((object_id() == to->object_id()) && (seg == ((AddressX86_1632 *)to)->seg)) {
		result = addr-((AddressX86_1632 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

Object *AddressX86_1632::duplicate()
{
	return new AddressX86_1632(seg, addr);
}

void AddressX86_1632::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(dword*)array);
	UNALIGNED_MOVE(seg, *(word*)(array+sizeof addr));
}

void AddressX86_1632::getFromCPUAddress(CPU_ADDR *ca)
{
	seg = ca->addr32.seg;
	addr = ca->addr32.offset;
}

int AddressX86_1632::load(ht_object_stream *s)
{
	seg = s->getIntHex(2, NULL);
	addr = s->getIntHex(4, NULL);
	return s->get_error();
}

OBJECT_ID AddressX86_1632::object_id() const
{
	return ATOM_ADDRESS_X86_1632;
}

int AddressX86_1632::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressX86_1632::putIntoArray(byte *array)
{
	UNALIGNED_MOVE(*(dword*)array, addr);
	UNALIGNED_MOVE(*(word*)(array+sizeof addr), seg);
}

void AddressX86_1632::putIntoCPUAddress(CPU_ADDR *ca)
{
	ca->addr32.seg = seg;
	ca->addr32.offset = addr;
}

void AddressX86_1632::store(ht_object_stream *s)
{
	s->putIntHex(seg, 2, NULL);
	s->putIntHex(addr, 4, NULL);
}

int AddressX86_1632::stringify(char *s, int max_length, int format)
{
	char *formats[] = {
		"%s%x%s:%s%x%s",
		"%s%4x%s:%s%08x%s",
		"%s%04x%s:%s%08x%s",
		"",
		"%s%X%s:%s%X%s",
		"%s%4X%s:%s%08X%s",
		"%s%04X%s:%s%08X%s",
		"",
	};
	return ht_snprintf(s, max_length, formats[format&7],
	(format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", seg, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"",
	(format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", addr, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"");
}

int AddressX86_1632::stringSize()
{
	return 14;
}

/*
 *
 */
AddressX86_1616::AddressX86_1616()
{
}

AddressX86_1616::AddressX86_1616(word Seg, word Addr)
{
	seg = Seg;
	addr = Addr;
}

bool AddressX86_1616::add(int offset)
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

int AddressX86_1616::byteSize()
{
	return 4;
}

int AddressX86_1616::compareTo(const Object *obj) const
{
	assert(object_id() == obj->object_id());
	if (seg > ((AddressX86_1616 *)obj)->seg) return 1;
	if (seg < ((AddressX86_1616 *)obj)->seg) return -1;
	if (addr > ((AddressX86_1616 *)obj)->addr) return 1;
	if (addr < ((AddressX86_1616 *)obj)->addr) return -1;
	return 0;
}

int AddressX86_1616::compareDelinear(Address *to)
{
	assert(object_id() == to->object_id());
	dword s1 = delinearize(seg);
	dword s2 = delinearize(((AddressX86_1616 *)to)->seg);
	if (s1 > s2) return 1;
	if (s1 < s2) return -1;
	dword a1 = delinearize(addr);
	dword a2 = delinearize(((AddressX86_1616 *)to)->addr);
	if (a1 > a2) return 1;
	if (a1 < a2) return -1;
	return 0;
}

bool AddressX86_1616::difference(int &result, Address *to)
{
	if ((object_id() == to->object_id()) && (seg == ((AddressX86_1616 *)to)->seg)) {
		result = (int)addr-(int)((AddressX86_1616 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

Object *AddressX86_1616::duplicate()
{
	return new AddressX86_1616(seg, addr);
}

void AddressX86_1616::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(word*)array);
	UNALIGNED_MOVE(seg, *(word*)(array+sizeof addr));
}

void AddressX86_1616::getFromCPUAddress(CPU_ADDR *ca)
{
	seg = ca->addr32.seg;
	addr = ca->addr32.offset;
}

int AddressX86_1616::load(ht_object_stream *s)
{
	seg = s->getIntHex(2, NULL);
	addr = s->getIntHex(2, NULL);
	return s->get_error();
}

OBJECT_ID AddressX86_1616::object_id() const
{
	return ATOM_ADDRESS_X86_1616;
}

int AddressX86_1616::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressX86_1616::putIntoArray(byte *array)
{
	UNALIGNED_MOVE(*(word*)array, addr);
	UNALIGNED_MOVE(*(word*)(array+sizeof seg), seg);
}

void AddressX86_1616::putIntoCPUAddress(CPU_ADDR *ca)
{
	ca->addr32.seg = seg;
	ca->addr32.offset = addr;
}

void AddressX86_1616::store(ht_object_stream *s)
{
	s->putIntHex(seg, 2, NULL);
	s->putIntHex(addr, 2, NULL);
}

int AddressX86_1616::stringify(char *s, int max_length, int format)
{
	char *formats[] = {
		"%s%x%s:%s%04x%s",
		"%s%4x%s:%s%04x%s",
		"%s%04x%s:%s%04x%s",
		"",
		"%s%X%s:%s%04X%s",
		"%s%4X%s:%s%04X%s",
		"%s%04X%s:%s%04X%s",
		"",
	};
	return ht_snprintf(s, max_length, formats[format&7],
	(format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", seg, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"",
	(format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", addr, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"");
}

int AddressX86_1616::stringSize()
{
	return 9;
}

/*
 *
 */
void AnalyX86Disassembler::init(Analyser *A, int f)
{
	flags = f;
	createUnasm();
	AnalyDisassembler::init(A);
}

/*
 *
 */
int  AnalyX86Disassembler::load(ht_object_stream *f)
{
	GET_INT_HEX(f, flags);
	return AnalyDisassembler::load(f);
}

/*
 *
 */
void AnalyX86Disassembler::done()
{
	AnalyDisassembler::done();
}

/*
 *
 */
OBJECT_ID	AnalyX86Disassembler::object_id() const
{
	return ATOM_ANALY_X86;
}

Address *AnalyX86Disassembler::createAddress(word segment, dword offset)
{
	if (flags & ANALYX86DISASSEMBLER_FLAGS_FLAT64) {
		return new AddressFlat64(to_qword(offset));
	} else if (flags & ANALYX86DISASSEMBLER_FLAGS_SEGMENTED) {
		if (offset <= 0xffff) {
			return new AddressX86_1616(segment, offset);
		} else {
			// FIXME
//			return new AddressX86_1632(segment, offset);
			return new AddressX86_1616(segment, offset);
		}
	} else {
		return new AddressX86Flat32(offset);
	}
}

void AnalyX86Disassembler::createUnasm()
{
	if (flags & ANALYX86DISASSEMBLER_FLAGS_VXD_X86DIS) {
		if (flags & ANALYX86DISASSEMBLER_FLAGS_16BIT) {
			disasm = new x86dis_vxd(X86_OPSIZE16, X86_ADDRSIZE16);
		} else {
			disasm = new x86dis_vxd(X86_OPSIZE32, X86_ADDRSIZE32);
		}
	} else {
		if (flags & ANALYX86DISASSEMBLER_FLAGS_16BIT) {
			disasm = new x86dis(X86_OPSIZE16, X86_ADDRSIZE16);
		} else {
			disasm = new x86dis(X86_OPSIZE32, X86_ADDRSIZE32);
		}
	}
}
word AnalyX86Disassembler::getSegment(Address *addr)
{
	if (addr->object_id() == ATOM_ADDRESS_X86_1616) {
		return ((AddressX86_1616*)addr)->seg;
	} else if (addr->object_id() == ATOM_ADDRESS_X86_1632) {
		return ((AddressX86_1632*)addr)->seg;
	} else {
		assert(0);
		return 0;
	}
}

/*
 *
 */
Address *AnalyX86Disassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
	Address *addr;
	x86dis_insn *o = (x86dis_insn*)opcode;
	assert(o->op[1].type == X86_OPTYPE_EMPTY);
	switch (o->op[0].type) {
		case X86_OPTYPE_IMM: {
/*          	if (o->op[0].imm == 0x1012c0f) {
				int as=0;
			}*/
			
			word seg = 0;
			if (flags & ANALYX86DISASSEMBLER_FLAGS_SEGMENTED) {
				seg = getSegment(analy->addr);
			}
			addr = createAddress(seg, o->op[0].imm);
			return addr;
		}
		case X86_OPTYPE_FARPTR:
			if (flags & ANALYX86DISASSEMBLER_FLAGS_SEGMENTED) {
				addr = createAddress(o->op[0].farptr.seg, o->op[0].farptr.offset);
			} else {
				break;
			}
			return addr;
		case X86_OPTYPE_MEM: {
			taccess access;
			addr = NULL;
			if (o->op[0].mem.hasdisp) {
				addr = createAddress(0, o->op[0].mem.disp);
				access.type = acread;
				access.indexed = (o->op[0].mem.base != X86_REG_NO) || (o->op[0].mem.index != X86_REG_NO);
				access.size = o->op[0].size;
			} else {
				break;
			}
			if (examine && analy->validAddress(addr, scvalid)) {
				analy->dataAccess(addr, access);
				xref_enum_t xref;
				switch (branchtype) {
					case br_jXX:
					case br_jump:
						xref = xrefijump;
						break;
					case br_call:
						xref = xreficall;
						break;
					default: {assert(0);}
				}
				analy->addXRef(addr, analy->addr, xref);
			}
			if (examine) {
				if (addr) delete addr;
				break;
			} else {
				return addr;
			}
		}
	}
	return new InvalidAddress();
}

/*
 *
 */
void	AnalyX86Disassembler::examineOpcode(OPCODE *opcode)
{
	x86dis_insn *o = (x86dis_insn*)opcode;
	for (int i=0; i<3; i++) {
		x86_insn_op *op = &o->op[i];
		Address *addr = NULL;
		taccess access;
		xref_enum_t xref = xrefoffset;
		switch (op->type) {
			case X86_OPTYPE_IMM:
				access.type = acoffset;
				access.indexed = false;
				addr = createAddress(0, op->imm);
				break;
			case X86_OPTYPE_FARPTR:
				if (flags & ANALYX86DISASSEMBLER_FLAGS_SEGMENTED) {
					addr = createAddress(op->farptr.seg, op->farptr.offset);
				}
				access.type = acoffset;
				access.indexed = false;
				break;
			case X86_OPTYPE_MEM:
				if (op->mem.hasdisp) {
					addr = createAddress(0, op->mem.disp);
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
		if (addr) {
			if (analy->validAddress(addr, scvalid)) {
				analy->dataAccess(addr, access);
				analy->addXRef(addr, analy->addr, xref);
			}
			delete addr;
		}
	}
}

/*
 *
 */
branch_enum_t AnalyX86Disassembler::isBranch(OPCODE *opcode)
{
	x86dis_insn *o = (x86dis_insn*)opcode;
	char *opcode_str = o->name;
	if (opcode_str[0]=='j') {
		if (opcode_str[1]=='m') return br_jump; else return br_jXX;
	} else if ((opcode_str[0]=='l') && (opcode_str[1]=='o')  && (opcode_str[2]=='o')) {
		// loop opcode will be threated like a jXX
		return br_jXX;
	} else if ((opcode_str[0]=='c') && (opcode_str[1]=='a')) {
		return br_call;
	} else if ((opcode_str[0]=='r') && (opcode_str[1]=='e')) {
		return br_return;
	} else return br_nobranch;
}

/*
 *
 */
void AnalyX86Disassembler::store(ht_object_stream *f)
{
	PUT_INT_HEX(f, flags);
	AnalyDisassembler::store(f);
}


