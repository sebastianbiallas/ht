/*
 *	HT Editor
 *	alphadis.h
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

#ifndef ALPHADIS_H
#define ALPHADIS_H

#include "global.h"
#include "asm.h"
#include "alphaopc.h"

struct alphadis_insn {
	bool				valid;
	int				size;       /* only useful if invalid (else 4) */
	long				data;       /* must be signed */
	word				code;
	byte				regA;
	byte				regB;
	byte				regC;
	alpha_opcode_tab_entry	*table;
};

/*
 *	CLASS alphadis
 */

class Alphadis: public Disassembler {
protected:
	char insnstr[256];
	alphadis_insn	insn;
public:
			Alphadis();
	virtual	~Alphadis();

	virtual	dis_insn	*createInvalidInsn();
			int 		load(ht_object_stream *f);
	virtual	dis_insn	*decode(byte *code, byte maxlen, CPU_ADDR addr);
	virtual	dis_insn	*duplicateInsn(dis_insn *disasm_insn);
	virtual	int		getMaxOpcodeLength();
	virtual	void		getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align);
	virtual	byte		getSize(dis_insn *disasm_insn);
	virtual	char		*getName();
	virtual	void		store(ht_object_stream *f);
	virtual	char		*str(dis_insn *disasm_insn, int style);
	virtual	char		*strf(dis_insn *disasm_insn, int style, char *format);
	virtual	OBJECT_ID object_id();
	virtual	bool		validInsn(dis_insn *disasm_insn);
};

#endif

