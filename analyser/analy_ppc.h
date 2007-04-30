/* 
 *	HT Editor
 *	analy_ppc.h
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

#ifndef ANALY_PPC_H
#define ANALY_PPC_H

#include "analy.h"
#include "ppcdis.h"

#define ANALY_PPC_32 0
#define ANALY_PPC_64 1

class AnalyPPCDisassembler: public AnalyDisassembler {
	int mode;
public:
				AnalyPPCDisassembler() {};
				AnalyPPCDisassembler(BuildCtorArg&a): AnalyDisassembler(a) {};

		void		init(Analyser *A, int mode);
	virtual void		done();
	virtual	ObjectID	getObjectID() const;
		void 		load(ObjectStream &f);

	virtual	Address		*branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine);
		Address		*createAddress(uint64 offset);
	virtual	void		examineOpcode(OPCODE *opcode);
	virtual	branch_enum_t 	isBranch(OPCODE *opcode);
	virtual	void		store(ObjectStream &f) const;
};

#endif
