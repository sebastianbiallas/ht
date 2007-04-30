/* 
 *	HT Editor
 *	analy_java.h
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

#ifndef ANALY_JAVA_H
#define ANALY_JAVA_H

#include "analy.h"
#include "javadis.h"

class AnalyJavaDisassembler: public AnalyDisassembler {
public:
					AnalyJavaDisassembler() {};
					AnalyJavaDisassembler(BuildCtorArg&a): AnalyDisassembler(a) {};
		void			init(Analyser *A, java_token_func token_func, void *context);
	virtual	ObjectID		getObjectID() const;

	virtual	Address			*branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine);
	virtual	void			examineOpcode(OPCODE *opcode);
	virtual	branch_enum_t	 	isBranch(OPCODE *opcode);
};

#endif /* ANALY_JAVA_H */

