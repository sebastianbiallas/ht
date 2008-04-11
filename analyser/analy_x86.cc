/* 
 *	HT Editor
 *	analy_x86.cc
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
#include "analy_x86.h"
#include "htdebug.h"
#include "snprintf.h"
#include "x86dis.h"

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
	assert(getObjectID() == obj->getObjectID());
	if (addr > ((AddressX86Flat32 *)obj)->addr) return 1;
	if (addr < ((AddressX86Flat32 *)obj)->addr) return -1;
	return 0;
}

int AddressX86Flat32::compareDelinear(Address *to)
{
	assert(getObjectID() == to->getObjectID());
	uint32 da = delinearize(addr);
	uint32 db = delinearize(((AddressFlat32 *)to)->addr);
	if (da > db) return 1;
	if (da < db) return -1;
	return 0;
}

bool AddressX86Flat32::difference(int &result, Address *to)
{
	if (getObjectID() == to->getObjectID()) {
		result = addr-((AddressX86Flat32 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

AddressX86Flat32 *AddressX86Flat32::clone() const
{
	return new AddressX86Flat32(*this);
}

void AddressX86Flat32::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(uint32*)array);
}

void AddressX86Flat32::getFromCPUAddress(CPU_ADDR *ca)
{
	addr = ca->addr32.offset;
}

bool AddressX86Flat32::getFromUInt64(uint64 u)
{
	if (u <= 0xffffffff) {
		addr = u;
		return true;
	} else {
		return false;
	}
}

void AddressX86Flat32::load(ObjectStream &s)
{
	GET_INT32X(s, addr);
}

ObjectID AddressX86Flat32::getObjectID() const
{
	return ATOM_ADDRESS_X86_FLAT_32;
}

int AddressX86Flat32::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressX86Flat32::putIntoArray(byte *array) const
{
	UNALIGNED_MOVE(*(uint32*)array, addr);
}

void AddressX86Flat32::putIntoCPUAddress(CPU_ADDR *ca) const
{
	ca->addr32.offset = addr;
}

bool AddressX86Flat32::putIntoUInt64(uint64 &u) const
{
	u = addr;
	return true;
}

void AddressX86Flat32::store(ObjectStream &s) const
{
	PUT_INT32X(s, addr);
}

int AddressX86Flat32::stringify(char *s, int max_length, int format) const
{
	const char *formats[] = {
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

int AddressX86Flat32::stringSize() const
{
	return 8;
}


AddressX86_1632::AddressX86_1632(uint16 Seg, uint32 Addr)
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
	assert(getObjectID() == obj->getObjectID());
	if (seg > ((AddressX86_1632 *)obj)->seg) return 1;
	if (seg < ((AddressX86_1632 *)obj)->seg) return -1;
	if (addr > ((AddressX86_1632 *)obj)->addr) return 1;
	if (addr < ((AddressX86_1632 *)obj)->addr) return -1;
	return 0;
}

int AddressX86_1632::compareDelinear(Address *to)
{
	assert(getObjectID() == to->getObjectID());
	uint32 s1 = delinearize(seg);
	uint32 s2 = delinearize(((AddressX86_1632 *)to)->seg);
	if (s1 > s2) return 1;
	if (s1 < s2) return -1;
	uint32 a1 = delinearize(addr);
	uint32 a2 = delinearize(((AddressX86_1632 *)to)->addr);
	if (a1 > a2) return 1;
	if (a1 < a2) return -1;
	return 0;
}

bool AddressX86_1632::difference(int &result, Address *to)
{
	if ((getObjectID() == to->getObjectID()) && (seg == ((AddressX86_1632 *)to)->seg)) {
		result = addr-((AddressX86_1632 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

AddressX86_1632 *AddressX86_1632::clone() const
{
	return new AddressX86_1632(*this);
}

void AddressX86_1632::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(uint32*)array);
	UNALIGNED_MOVE(seg, *(uint16*)(array+sizeof addr));
}

void AddressX86_1632::getFromCPUAddress(CPU_ADDR *ca)
{
	seg = ca->addr32.seg;
	addr = ca->addr32.offset;
}

bool AddressX86_1632::getFromUInt64(uint64 u)
{
	return false;
}

void AddressX86_1632::load(ObjectStream &s)
{
	GET_INT16X(s, seg);
	GET_INT16X(s, addr);
}

ObjectID AddressX86_1632::getObjectID() const
{
	return ATOM_ADDRESS_X86_1632;
}

int AddressX86_1632::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressX86_1632::putIntoArray(byte *array) const
{
	UNALIGNED_MOVE(*(uint32*)array, addr);
	UNALIGNED_MOVE(*(uint16*)(array+sizeof addr), seg);
}

void AddressX86_1632::putIntoCPUAddress(CPU_ADDR *ca) const
{
	ca->addr32.seg = seg;
	ca->addr32.offset = addr;
}

bool AddressX86_1632::putIntoUInt64(uint64 &u) const
{
	return false;
}

void AddressX86_1632::store(ObjectStream &s) const
{
	PUT_INT16X(s, seg);
	PUT_INT16X(s, addr);
}

int AddressX86_1632::stringify(char *s, int max_length, int format) const
{
	const char *formats[] = {
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

int AddressX86_1632::stringSize() const
{
	return 14;
}

/*
 *
 */
AddressX86_1616::AddressX86_1616(uint16 Seg, uint16 Addr)
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
	assert(getObjectID() == obj->getObjectID());
	if (seg > ((AddressX86_1616 *)obj)->seg) return 1;
	if (seg < ((AddressX86_1616 *)obj)->seg) return -1;
	if (addr > ((AddressX86_1616 *)obj)->addr) return 1;
	if (addr < ((AddressX86_1616 *)obj)->addr) return -1;
	return 0;
}

int AddressX86_1616::compareDelinear(Address *to)
{
	assert(getObjectID() == to->getObjectID());
	uint32 s1 = delinearize(seg);
	uint32 s2 = delinearize(((AddressX86_1616 *)to)->seg);
	if (s1 > s2) return 1;
	if (s1 < s2) return -1;
	uint32 a1 = delinearize(addr);
	uint32 a2 = delinearize(((AddressX86_1616 *)to)->addr);
	if (a1 > a2) return 1;
	if (a1 < a2) return -1;
	return 0;
}

bool AddressX86_1616::difference(int &result, Address *to)
{
	if ((getObjectID() == to->getObjectID()) && (seg == ((AddressX86_1616 *)to)->seg)) {
		result = (int)addr-(int)((AddressX86_1616 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

AddressX86_1616 *AddressX86_1616::clone() const
{
	return new AddressX86_1616(*this);
}

void AddressX86_1616::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(uint16*)array);
	UNALIGNED_MOVE(seg, *(uint16*)(array+sizeof addr));
}

bool AddressX86_1616::getFromUInt64(uint64 u)
{
	return false;
}

void AddressX86_1616::getFromCPUAddress(CPU_ADDR *ca)
{
	seg = ca->addr32.seg;
	addr = ca->addr32.offset;
}

void AddressX86_1616::load(ObjectStream &s)
{
	GET_INT16X(s, seg);
	GET_INT16X(s, addr);
}

ObjectID AddressX86_1616::getObjectID() const
{
	return ATOM_ADDRESS_X86_1616;
}

int AddressX86_1616::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

void AddressX86_1616::putIntoArray(byte *array) const
{
	UNALIGNED_MOVE(*(uint16*)array, addr);
	UNALIGNED_MOVE(*(uint16*)(array+sizeof seg), seg);
}

void AddressX86_1616::putIntoCPUAddress(CPU_ADDR *ca) const
{
	ca->addr32.seg = seg;
	ca->addr32.offset = addr;
}

bool AddressX86_1616::putIntoUInt64(uint64 &u) const
{
	return false;
}

void AddressX86_1616::store(ObjectStream &s) const
{
	PUT_INT16X(s, seg);
	PUT_INT16X(s, addr);
}

int AddressX86_1616::stringify(char *s, int max_length, int format) const
{
	const char *formats[] = {
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

int AddressX86_1616::stringSize() const
{
	return 9;
}

void AnalyX86Disassembler::init(Analyser *A, int f)
{
	flags = f;
	createUnasm();
	AnalyDisassembler::init(A);
}

/*
 *
 */
void AnalyX86Disassembler::load(ObjectStream &f)
{
	GET_INT32X(f, flags);
	AnalyDisassembler::load(f);
}

/*
 *
 */
ObjectID AnalyX86Disassembler::getObjectID() const
{
	return ATOM_ANALY_X86;
}

Address *AnalyX86Disassembler::createAddress(uint16 segment, uint64 offset)
{
	if (flags & (ANALYX86DISASSEMBLER_FLAGS_FLAT64 | ANALYX86DISASSEMBLER_FLAGS_AMD64)) {
		return new AddressFlat64(offset);
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
	if (flags & ANALYX86DISASSEMBLER_FLAGS_AMD64) {
		disasm = new x86_64dis();
	} else {
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
}
uint16 AnalyX86Disassembler::getSegment(Address *addr)
{
	if (addr->getObjectID() == ATOM_ADDRESS_X86_1616) {
		return ((AddressX86_1616*)addr)->seg;
	} else if (addr->getObjectID() == ATOM_ADDRESS_X86_1632) {
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
			uint16 seg = 0;
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
				delete addr;
				break;
			} else {
				return addr;
			}
		}
		default: break;
	}
	return new InvalidAddress();
}

/*
 *
 */
void	AnalyX86Disassembler::examineOpcode(OPCODE *opcode)
{
	x86dis_insn *o = (x86dis_insn*)opcode;
	for (int i = 0; i < 5; i++) {
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
		default: continue;
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
	const char *opcode_str = o->name;
	if (opcode_str[0] == '~') {
		opcode_str++;
	}
	if (opcode_str[0] == '|') {
		opcode_str++;
	}

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
void AnalyX86Disassembler::store(ObjectStream &f) const
{
	PUT_INT32X(f, flags);
	AnalyDisassembler::store(f);
}
