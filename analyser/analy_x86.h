/*
 *	HT Editor
 *	analy_x86.h
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

#ifndef ANALY_X86_H
#define ANALY_X86_H

#include "analy.h"

class AddressX86Flat32: public Address {
public:
	uint32 addr;
	AddressX86Flat32(BuildCtorArg&a): Address(a) {};
	AddressX86Flat32(uint32 a=0): addr(a) {};
	virtual bool add(int offset);
	virtual int byteSize();
	virtual int compareTo(const Object *obj) const;
	virtual int compareDelinear(Address *to);
	virtual bool difference(int &result, Address *to);
	virtual AddressX86Flat32 *clone() const;
	virtual void getFromArray(const byte *array);
	virtual void getFromCPUAddress(CPU_ADDR *ca);
	virtual	bool getFromUInt64(uint64 u);
	virtual void load(ObjectStream &s);
	virtual ObjectID getObjectID() const;
	virtual int parseString(const char *s, int length, Analyser *a);
	virtual void putIntoArray(byte *array) const;
	virtual void putIntoCPUAddress(CPU_ADDR *ca) const;
	virtual	bool putIntoUInt64(uint64 &u) const;
	virtual void store(ObjectStream &s) const;
	virtual int stringify(char *s, int max_length, int format) const;
	virtual int stringSize() const;
};

class AddressX86_1616: public Address {
public:
	uint16 seg;
	uint16 addr;
public:
	AddressX86_1616(BuildCtorArg&a): Address(a) {};
	AddressX86_1616(uint16 seg=0, uint16 addr=0);
	virtual bool add(int offset);
	virtual int byteSize();
	virtual int compareTo(const Object *obj) const;
	virtual int compareDelinear(Address *to);
	virtual bool difference(int &result, Address *to);
	virtual AddressX86_1616 *clone() const;
	virtual void getFromArray(const byte *array);
	virtual void getFromCPUAddress(CPU_ADDR *ca);
	virtual	bool getFromUInt64(uint64 u);
	virtual void load(ObjectStream &s);
	virtual ObjectID getObjectID() const;
	virtual int parseString(const char *s, int length, Analyser *a);
	virtual void putIntoArray(byte *array) const;
	virtual void putIntoCPUAddress(CPU_ADDR *ca) const;
	virtual	bool putIntoUInt64(uint64 &u) const;
	virtual void store(ObjectStream &s) const;
	virtual int stringify(char *s, int max_length, int format) const;
	virtual int stringSize() const;
};

class AddressX86_1632: public Address {
public:
	uint16 seg;
	uint32 addr;
public:
	AddressX86_1632(BuildCtorArg&a): Address(a) {};
	AddressX86_1632(uint16 seg=0, uint32 addr=0);
	virtual bool add(int offset);
	virtual int byteSize();
	virtual int compareTo(const Object *obj) const;
	virtual int compareDelinear(Address *obj);
	virtual bool difference(int &result, Address *to);
	virtual AddressX86_1632 *clone() const;
	virtual void getFromArray(const byte *array);
	virtual void getFromCPUAddress(CPU_ADDR *ca);
	virtual	bool getFromUInt64(uint64 u);
	virtual void load(ObjectStream &s);
	virtual ObjectID getObjectID() const;
	virtual int parseString(const char *s, int length, Analyser *a);
	virtual void putIntoArray(byte *array) const;
	virtual void putIntoCPUAddress(CPU_ADDR *ca) const;
	virtual	bool putIntoUInt64(uint64 &u) const;
	virtual void store(ObjectStream &s) const;
	virtual int stringify(char *s, int max_length, int format) const;
	virtual int stringSize() const;
};

#define ANALYX86DISASSEMBLER_FLAGS_16BIT		1
#define ANALYX86DISASSEMBLER_FLAGS_SEGMENTED		2
#define ANALYX86DISASSEMBLER_FLAGS_FLAT64		4
#define ANALYX86DISASSEMBLER_FLAGS_VXD_X86DIS		8
#define ANALYX86DISASSEMBLER_FLAGS_AMD64		16

class AnalyX86Disassembler: public AnalyDisassembler {
protected:
		void		createUnasm();
public:
	int flags;

				AnalyX86Disassembler() {};
				AnalyX86Disassembler(BuildCtorArg&a): AnalyDisassembler(a) {};
		void		init(Analyser *A, int flags);
		void 		load(ObjectStream &f);
	virtual	ObjectID	getObjectID() const;

	virtual	Address		*branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine);
		Address		*createAddress(uint16 segment, uint64 offset);
		uint16		getSegment(Address *addr);
	virtual	void		examineOpcode(OPCODE *opcode);
	virtual	branch_enum_t 	isBranch(OPCODE *opcode);
	virtual	void		store(ObjectStream &f) const;
};

#endif
