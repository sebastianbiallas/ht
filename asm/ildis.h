/*
 *	HT Editor
 *	ildis.h
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

#ifndef ILDIS_H
#define ILDIS_H

#include "asm.h"
#include "global.h"
#include "ilopc.h"

struct ILDisInsn {
	bool				valid;
	int				size;
	byte                op;
	dword			data;
	dword			data2;
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
	char* (*string_func)(dword string_ofs, void *context);
	char* (*token_func)(dword token, void *context);
public:
			ILDisassembler(char* (*string_func)(dword string_ofs, void *context), char* (*token_func)(dword token, void *context), void *context);
	virtual	~ILDisassembler();

	virtual	dis_insn	*decode(byte *code, byte maxlen, CPU_ADDR addr);
	virtual	dis_insn	*duplicateInsn(dis_insn *disasm_insn);
	virtual	void		getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align);
	virtual	byte		getSize(dis_insn *disasm_insn);
	virtual	char		*getName();
	virtual	char		*str(dis_insn *disasm_insn, int style);
	virtual	char		*strf(dis_insn *disasm_insn, int style, char *format);
	virtual	OBJECT_ID object_id();
	virtual	bool		validInsn(dis_insn *disasm_insn);
};
								
#endif 
