/* 
 *	HT Editor
 *	asm.cc
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

#include "asm.h"
#include "common.h"
#include "global.h"
#include "htatom.h"
#include "htdebug.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "alphadis.h"
#include "x86dis.h"

/*
 *	CLASS assembler
 */

assembler::assembler(bool b)
{
	codes = 0;
	bigendian = b;
}

assembler::~assembler()
{
}

asm_insn *assembler::alloc_insn()
{
	return NULL;
}

void assembler::deletecode(asm_code *code)
{
	asm_code **p=&codes, *c=codes;
	while (c) {
		if (c==code) {
			*p=c->next;
			delete c;
			return;
		}
		c=c->next;
		p=&(*p)->next;
	}
}

asm_code *assembler::encode(asm_insn *asm_insn, int _options, CPU_ADDR cur_address)
{
	free_asm_codes();
	error=0;
	options=_options;
	return 0;
}

void assembler::clearcode()
{
	code.size=0;
}

void assembler::emitbyte(byte b)
{
	code.data[code.size] = b;
	code.size++;
}

void assembler::emitword(word w)
{
	if (bigendian) {
		code.data[code.size+1] = (byte)w;
		code.data[code.size+0] = (byte)(w>>8);
	} else {
		code.data[code.size+0] = (byte)w;
		code.data[code.size+1] = (byte)(w>>8);
	}
	code.size += 2;
}

void assembler::emitdword(dword d)
{
	if (bigendian) {
		code.data[code.size+3] = (byte)d;
		code.data[code.size+2] = (byte)(d>>8);
		code.data[code.size+1] = (byte)(d>>16);
		code.data[code.size+0] = (byte)(d>>24);
	} else {
		code.data[code.size+0] = (byte)d;
		code.data[code.size+1] = (byte)(d>>8);
		code.data[code.size+2] = (byte)(d>>16);
		code.data[code.size+3] = (byte)(d>>24);
	}
	code.size += 4;
}

void assembler::free_asm_codes()
{
	while (codes) {
		asm_code *t=codes->next;
		delete codes;
		codes=t;
	}
}

char *assembler::get_error_msg()
{
	return error_msg;
}

char *assembler::get_name()
{
	return "generic asm";
}

void assembler::newcode()
{
	code.size=0;
}

asm_code *assembler::shortest(asm_code *codes)
{
	asm_code *best=0;
	dword bestv=0xffffffff;
	while (codes) {
		if (codes->size<bestv) {
			best=codes;
			bestv=codes->size;
		}
		codes=codes->next;
	};
	return best;
}

void assembler::pushcode()
{
	asm_code **t=&codes;
	while (*t) {
		t=&(*t)->next;
	}
	*t=new asm_code;

	memmove(*t, &code, sizeof code);
	(*t)->next=0;
}

int assembler::prepare_str(asm_insn *asm_insn, char *s)
{
	return 0;
}

void assembler::set_error_msg(char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vsprintf(error_msg, format, arg);
	va_end(arg);
	error=1;
}

void assembler::set_imm_eval_proc(int (*p)(void *context, char **s, dword *v), void *c)
{
	imm_eval_proc=p;
	imm_eval_context=c;
}

/*
 *	CLASS disassembler
 */

disassembler::disassembler()
{
	disable_highlighting();
}

disassembler::~disassembler()
{
}

char* (*addr_sym_func)(CPU_ADDR addr, int *symstrlen, void *context) = NULL;
void* addr_sym_func_context = NULL;

dis_insn *disassembler::create_invalid_insn()
{
	return 0;
}

dis_insn *disassembler::decode(byte *code, byte maxlen, CPU_ADDR cur_address)
{
	return 0;
}

int disassembler::getmaxopcodelength()
{
	return 0;
}

byte disassembler::getsize(dis_insn *disasm_insn)
{
	return 0;
}

void disassembler::hexd(char **s, int size, int options, int imm)
{
	char ff[16];
	char *f = (char*)&ff;
	char *t = *s;
	*f++ = '%';
	if ((imm>=0) && (imm<=9)) {
		*s += sprintf(*s, "%d", imm);
	} else if (options & DIS_STYLE_SIGNED) {
		if (!(options & DIS_STYLE_HEX_NOZEROPAD)) f += sprintf(f, "0%d", size);
		*f++ = 'd';
		*f = 0;
		*s += sprintf(*s, ff, imm);
	} else {
		if (options & DIS_STYLE_HEX_CSTYLE) *f++ = '#';
		if (!(options & DIS_STYLE_HEX_NOZEROPAD)) f += sprintf(f, "0%d", size);
		if (options & DIS_STYLE_HEX_UPPERCASE) *f++ = 'X'; else
			*f++ = 'x';
		if (options & DIS_STYLE_HEX_ASMSTYLE) *f++ = 'h';
		*f = 0;
		*s += sprintf(*s, ff, imm);
		if ((options & DIS_STYLE_HEX_NOZEROPAD) && (*t-'0'>9)) {
			memmove(t+1, t, strlen(t)+1);
			*t = '0';
			(*s)++;
		}
	}
}

char *disassembler::str(dis_insn *disasm_insn, int style)
{
	return strf(disasm_insn, style, DISASM_STRF_DEFAULT_FORMAT);
}

char *disassembler::strf(dis_insn *disasm_insn, int style, char *format)
{
	return 0;
}

bool disassembler::valid_insn(dis_insn *disasm_insn)
{
	return 0;
}

const char *disassembler::get_cs_default()
{
	return (highlight) ? ASM_SYNTAX_DEFAULT : "";
}

const char *disassembler::get_cs_number()
{
	return (highlight) ? ASM_SYNTAX_NUMBER : "";
}

const char *disassembler::get_cs_symbol()
{
	return (highlight) ? ASM_SYNTAX_SYMBOL : "";
}

const char *disassembler::get_cs_string()
{
	return (highlight) ? ASM_SYNTAX_STRING : "";
}

void disassembler::enable_highlighting()
{
	highlight = true;
}

void disassembler::disable_highlighting()
{
	highlight = false;
}

BUILDER(ATOM_DISASM_ALPHA, alphadis)
BUILDER(ATOM_DISASM_X86, x86dis)

bool init_asm()
{
	REGISTER(ATOM_DISASM_ALPHA, alphadis)
	REGISTER(ATOM_DISASM_X86, x86dis)
	return true;
}

void done_asm()
{
	UNREGISTER(ATOM_DISASM_ALPHA, alphadis)
	UNREGISTER(ATOM_DISASM_X86, x86dis)
}
