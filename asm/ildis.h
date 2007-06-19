/*
 *	HT Editor
 *	ildis.h
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

#ifndef ILDIS_H
#define ILDIS_H

#include "asm.h"
#include "data.h"
#include "ilopc.h"

struct ILDisInsn {
	bool	valid;
	int	size;
	byte	op;
	union {
		uint32	ui;
		uint64	q;
		float	f;
		double	df;
		int	i;
	} data;
	ILOpcodeTabEntry	*prefix;
	ILOpcodeTabEntry	*opcode;
};

/*
 *	CLASS alphadis
 */

class ILDisassembler: public Disassembler {
protected:
	char insnstr[256];
	ILDisInsn	insn;
	void *context;
	char* (*string_func)(uint32 string_ofs, void *context);
	char* (*token_func)(uint32 token, void *context);
public:
				ILDisassembler(char* (*string_func)(uint32 string_ofs, void *context), char* (*token_func)(uint32 token, void *context), void *context);
				ILDisassembler(BuildCtorArg&a): Disassembler(a) {};

	virtual	dis_insn	*decode(byte *code, int maxlen, CPU_ADDR addr);
	virtual	dis_insn	*duplicateInsn(dis_insn *disasm_insn);
	virtual	void		getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align);
	virtual	byte		getSize(dis_insn *disasm_insn);
	virtual	const char	*getName();
		void		initialize(char* (*string_func)(uint32 string_ofs, void *context), char* (*token_func)(uint32 token, void *context), void *context);
	virtual	const char	*str(dis_insn *disasm_insn, int style);
	virtual	const char	*strf(dis_insn *disasm_insn, int style, const char *format);
	virtual	ObjectID	getObjectID() const;
	virtual	bool		validInsn(dis_insn *disasm_insn);
};
								
#endif 
