/* 
 *	HT Editor
 *	javadis.h
 *
 *	Copyright (C) 2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __JAVADIS_H__
#define __JAVADIS_H__

#include "asm.h"
#include "javaopc.h"

/* x86-specific styles */
//#define X86DIS_STYLE_EXPLICIT_MEMSIZE	0x00000001		/* IF SET: mov word ptr [0000], ax 	ELSE: mov [0000], ax */
//#define X86DIS_STYLE_OPTIMIZE_ADDR		0x00000002		/* IF SET: mov [eax*3], ax 			ELSE: mov [eax+eax*2+00000000], ax */
/*#define X86DIS_STYLE_USE16			0x00000004
#define X86DIS_STYLE_USE32			0x00000008*/

#define JAVAINSN_MAX_PARAM_COUNT	2

struct javadis_insn {
	bool invalid;
	byte size;
	int opcode;
	bool wideopcode;
	char *name;
	java_insn_op op[JAVAINSN_MAX_PARAM_COUNT];
};

/*
 *	CLASS x86dis
 */

typedef int (*java_token_func)(char *result, int maxlen, dword token, void *context);

class javadis: public Disassembler {
protected:
	javadis_insn insn;
	char insnstr[1024];
/* initme! */
	unsigned char *codep, *ocodep;
	int addr;
	int maxlen;
	java_token_func token_func;
	void *context;
/* new */
			void decode_insn(javaopc_insn *insn);
			void decode_op(int optype, bool wideopc, java_insn_op *op);
			byte getbyte();
			word getword();
			dword getdword();
			void invalidate();
			void str_format(char **str, char **format, char *p, char *n, char *op[3], int oplen[3], char stopchar, int print);
	virtual	void str_op(char *opstr, int *opstrlen, javadis_insn *insn, java_insn_op *op);
public:
	javadis();
	javadis(java_token_func token_func, void *context);
	virtual ~javadis();

/* overwritten */
	virtual dis_insn *decode(byte *code, byte maxlen, CPU_ADDR addr);
	virtual dis_insn *duplicateInsn(dis_insn *disasm_insn);
	virtual void getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align);
	virtual char *getName();
	virtual byte getSize(dis_insn *disasm_insn);
		   int load(ht_object_stream *f);
	virtual OBJECT_ID object_id() const;
	virtual char *str(dis_insn *disasm_insn, int options);
	virtual char *strf(dis_insn *disasm_insn, int options, char *format);
	virtual void store(ht_object_stream *f);
	virtual bool validInsn(dis_insn *disasm_insn);
};

#endif /* __JAVADIS_H__ */

