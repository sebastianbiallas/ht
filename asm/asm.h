/* 
 *	The HT Editor
 *	asm.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __ASM_H__
#define __ASM_H__

#include "common.h"

#define CPU_X86	1

#define MAX_INSN_SIZE	16

struct CPU_ADDR {
	union {
		struct {
			word seg;
			dword offset;
		} addr32;
		struct {
			dword lo, hi;
		} flat64;
	};
};

struct asm_code {
	asm_code *next;
	dword size;
	byte data[MAX_INSN_SIZE];
	void *context;
};

typedef void dis_insn;
typedef void asm_insn;

/*
 *	CLASS assembler
 */

class assembler: public object {
protected:
	int (*imm_eval_proc)(void *context, char **s, dword *v);
	void *imm_eval_context;
	
	asm_code *codes;
	asm_code code;
	char error_msg[256];
	bool error;
	int options;
	bool bigendian;

			void emitbyte(byte b);
			void emitword(word w);
			void emitdword(dword d);
			void free_asm_codes();
			void deletecode(asm_code *c);
			void clearcode();
			void newcode();
			void pushcode();
public:
			assembler(bool bigendian);
			~assembler();
/* new */
	virtual	asm_insn *alloc_insn();
	virtual	asm_code *encode(asm_insn *asm_insn, int options, CPU_ADDR cur_address);
			char *get_error_msg();
	virtual	char *get_name();
	virtual	int prepare_str(asm_insn *asm_insn, char *s);
			void set_error_msg(char *format, ...);
			void set_imm_eval_proc(int (*imm_eval_proc)(void *context, char **s, dword *v), void *imm_eval_context);
			asm_code *shortest(asm_code *codes);
};

/*
 *	CLASS disassembler
 */

/* generic disassembler styles */
#define DIS_STYLE_HIGHLIGHT			0x80000000		/* create highlighting information in strf() */
#define DIS_STYLE_HEX_CSTYLE			0x40000000		/* IF SET: mov eax, 0x12345678 		ELSE: mov eax, 12345678 */
#define DIS_STYLE_HEX_ASMSTYLE		0x20000000		/* IF SET: mov eax, 12345678h 		ELSE: mov eax, 12345678 */
#define DIS_STYLE_HEX_UPPERCASE		0x10000000		/* IF SET: mov eax, 5678ABCD	 		ELSE: mov eax, 5678abcd */
#define DIS_STYLE_HEX_NOZEROPAD		0x08000000		/* IF SET: mov eax, 8002344	 		ELSE: mov eax, 008002344 */
#define DIS_STYLE_SIGNED				0x04000000		/* IF SET: mov eax, -1	 			ELSE: mov eax, 0ffffffffh */

#define DIS_STYLE_TABSIZE			8

extern char* (*addr_sym_func)(CPU_ADDR addr, int *symstrlen, void *context);
extern void* addr_sym_func_context;

class disassembler: public object {
protected:
	int options;
	bool highlight;
	
			const char *get_cs_default();
			const char *get_cs_number();
			const char *get_cs_symbol();
			const char *get_cs_string();
			void hexd(char **s, int size, int options, int imm);
			void enable_highlighting();
			void disable_highlighting();
public:
			disassembler();
			~disassembler();
/* new */
	virtual	dis_insn *create_invalid_insn();
	virtual	dis_insn *decode(byte *code, byte maxlen, CPU_ADDR cur_address);
	virtual	int  getmaxopcodelength();
	virtual	byte getsize(dis_insn *disasm_insn);
	virtual	char *get_name()=0;
	virtual	char *str(dis_insn *disasm_insn, int style);
	virtual	char *strf(dis_insn *disasm_insn, int style, char *format);
	virtual	bool valid_insn(dis_insn *disasm_insn);
};

/*****************************************************************************
 *	The strf() format                                                       *
 *****************************************************************************
	String	Action
    --------------------------------------------------
	%x		substitute expression with symbol "x"
	?xy...y	if symbol "x" is undefined leave out the whole expression,
			otherwise subsitute expression with string between the two "y"s

	Symbol	Desc
    --------------------------------------------------
	p 		prefix
	n 		name
	1 		first operand
	2 		second operand
	3 		third operand
*/

#define DISASM_STRF_VAR			'%'
#define DISASM_STRF_COND			'?'

#define DISASM_STRF_PREFIX		'p'
#define DISASM_STRF_NAME			'n'
#define DISASM_STRF_FIRST		'1'
#define DISASM_STRF_SECOND		'2'
#define DISASM_STRF_THIRD		'3'

#define DISASM_STRF_DEFAULT_FORMAT	"?p#%p #%n\t%1?2#, %2?3/, %3/#"
#define DISASM_STRF_SMALL_FORMAT	"?p#%p #%n?1- %1?2#,%2?3/,%3/#-"

#define ATOM_DISASM_X86 MAGICD("DIS\x01")
#define ATOM_DISASM_ALPHA MAGICD("DIS\x02")

#define ASM_SYNTAX_DEFAULT "\\@d"
#define ASM_SYNTAX_NUMBER "\\@n"
#define ASM_SYNTAX_SYMBOL "\\@c"
#define ASM_SYNTAX_STRING "\\@s"

bool init_asm();
void done_asm();

#endif /* __ASM_H__ */
