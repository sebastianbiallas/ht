/* 
 *	HT Editor
 *	analy_il.h
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

#ifndef ANALY_IL_H
#define ANALY_IL_H

#include "analy.h"
#include "ildis.h"

/*class AddressAlphaFlat32: public Address {
public:
	dword addr;
	AddressAlphaFlat32();
	AddressAlphaFlat32(dword addr);
	virtual bool add(int offset);
	virtual int byteSize();
	virtual int compareTo(Object *to);
	virtual int compareDelinear(Address *to);
	virtual bool difference(int &result, Address *to);
	virtual Object *duplicate();
	virtual void getFromArray(const byte *array);
	virtual void getFromCPUAddress(CPU_ADDR *ca);
	virtual int load(ht_object_stream *s);
	virtual OBJECT_ID object_id();
	virtual int parseString(const char *s, int length, Analyser *a);
	virtual void putIntoArray(byte *array);
	virtual void putIntoCPUAddress(CPU_ADDR *ca);
	virtual void store(ht_object_stream *s);
	virtual int stringify(char *s, int max_length, int format);
	virtual int stringSize();
};*/

class AnalyILDisassembler: public AnalyDisassembler {
public:
			void			init(Analyser *A, char* (*string_func)(dword string_ofs, void *context), char* (*token_func)(dword token, void *context), void *context);
	virtual   void	     	done();
	virtual	OBJECT_ID		object_id();

	virtual	Address		*branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine);
			Address		*createAddress(dword offset);
	virtual	void			examineOpcode(OPCODE *opcode);
	virtual	branch_enum_t 	isBranch(OPCODE *opcode);
};

#endif
