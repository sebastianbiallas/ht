/*
 *	HT Editor
 *	ildis.cc
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

#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "htendian.h"
#include "ildis.h"
#include "ilopc.h"


ILDisassembler::ILDisassembler(char* (*sf)(dword string_ofs, void *context), char* (*tf)(dword token, void *context), void *ctx)
{
	string_func = sf;
	token_func = tf;
	context = ctx;
}

ILDisassembler::~ILDisassembler()
{
}

dis_insn *ILDisassembler::decode(byte *code, int maxlen, CPU_ADDR addr)
{
	insn.valid = false;
	insn.prefix = NULL;
	insn.size = 0;
	insn.data = *code;
restart:
	if (!maxlen) {
		return (dis_insn *)&insn;
	}
	insn.opcode = &il_opcode_table[*code];
	insn.size += insn.opcode->size;
	if (insn.size > maxlen) {
		insn.size = 1;
		insn.valid = false;
		return (dis_insn *)&insn;
	}
	insn.op = insn.opcode->op;
	switch (insn.op) {
		case IL_OPCODE_ARGS_INVALID:
			return (dis_insn *)&insn;
		case IL_OPCODE_ARGS_NONE:
			break;
		case IL_OPCODE_ARGS_PREFIX:
			insn.opcode = NULL;
			code++;
			if (*code < MAX_IL_OPCODE_PREFIX) {
				insn.prefix = &il_prefix_opcode_table[*code];
				insn.size--;
				code++;
				goto restart;
			} else {
				return (dis_insn *)&insn;
			}
		case IL_OPCODE_ARGS_SHORT_JUMP: {
			int c = ((char)code[1]);
			insn.data = addr.addr32.offset+2+c;
			break;
		}
		case IL_OPCODE_ARGS_LONG_JUMP: {
			int c = create_host_int(code+1, 4, little_endian);
			insn.data = addr.addr32.offset+5+c;
			break;
		}
		case IL_OPCODE_ARGS_UINT16: {
			word w = create_host_int(code+1, 2, little_endian);
			insn.data = w;
			break;
		}
		case IL_OPCODE_ARGS_INT16: {
			word w = create_host_int(code+1, 2, little_endian);
			insn.data = (int)((short)w);
			break;
		}
		case IL_OPCODE_ARGS_UINT8: {
			insn.data = (byte)code[1];
			break;
		}
		case IL_OPCODE_ARGS_INT8:
		case IL_OPCODE_ARGS_SHORT_VAR:
		case IL_OPCODE_ARGS_SHORT_ARG: {
			insn.data = (int)((char)code[1]);
			break;
		}
		case IL_OPCODE_ARGS_TOKEN:
		case IL_OPCODE_ARGS_NEW:
		case IL_OPCODE_ARGS_CALL:
		case IL_OPCODE_ARGS_CALLVIRT:
		case IL_OPCODE_ARGS_INT32:
		case IL_OPCODE_ARGS_STRING:
			insn.data = create_host_int(code+1, 4, little_endian);
			break;
	}
	insn.valid = true;
	return (dis_insn *)&insn;
}

dis_insn *ILDisassembler::duplicateInsn(dis_insn *disasm_insn)
{
	ILDisInsn *insn = (ILDisInsn *)malloc(sizeof (ILDisInsn));
	*insn = *(ILDisInsn *)disasm_insn;
	return insn;
}

void ILDisassembler::getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align)
{
	// FIXME
	min_length = 1;
	max_length = 10;
	min_look_ahead = 120;
	avg_look_ahead = 24;
	addr_align = 1;
}

byte ILDisassembler::getSize(dis_insn *disasm_insn)
{
	return ((ILDisInsn*)disasm_insn)->size;
}

char *ILDisassembler::getName()
{
	return "IL/Disassembler";
}

char *ILDisassembler::str(dis_insn *disasm_insn, int style)
{
	return strf(disasm_insn, style, "");
}

char *ILDisassembler::strf(dis_insn *disasm_insn, int style, char *format)
{
	if (style & DIS_STYLE_HIGHLIGHT) enable_highlighting();

//	const char *cs_default = get_cs(e_cs_default);
	const char *cs_number = get_cs(e_cs_number);
//	const char *cs_symbol = get_cs(e_cs_symbol);
	const char *cs_string = get_cs(e_cs_string);

	ILDisInsn *dis_insn = (ILDisInsn *) disasm_insn;
	
	if (!dis_insn->valid) {
//		is_invalid:
//          assert(dis_insn->size==1);
		sprintf(insnstr, "db              %s0x%02x", cs_number, dis_insn->data);
	} else {
		switch (dis_insn->op) {
			case IL_OPCODE_ARGS_NONE:
				sprintf(insnstr, "%-15s", dis_insn->opcode->name);
				break;
			case IL_OPCODE_ARGS_UINT8: {
				sprintf(insnstr, "%-15s %s0x%02x", dis_insn->opcode->name, cs_number, dis_insn->data);
				break;
			}
			case IL_OPCODE_ARGS_INT8:
			case IL_OPCODE_ARGS_SHORT_VAR:
			case IL_OPCODE_ARGS_SHORT_ARG: {
				sprintf(insnstr, "%-15s %s0x%x", dis_insn->opcode->name, cs_number, dis_insn->data);
				break;
			}
			case IL_OPCODE_ARGS_INT32:
				sprintf(insnstr, "%-15s %s0x%08x", dis_insn->opcode->name, cs_number, dis_insn->data);
				break;                    
			case IL_OPCODE_ARGS_CALL:
			case IL_OPCODE_ARGS_CALLI:
			case IL_OPCODE_ARGS_CALLVIRT:
			case IL_OPCODE_ARGS_NEW:
			case IL_OPCODE_ARGS_TOKEN: {
				dword token = dis_insn->data;                    
				char *tokenstr = NULL;
				if (token_func) {
					tokenstr = token_func(token, context);
					if (tokenstr) {
						sprintf(insnstr, "%-15s %s", dis_insn->opcode->name, tokenstr);
						break;
					}
				}
				sprintf(insnstr, "%-15s %s0x%08x", dis_insn->opcode->name, cs_number, token);
				break;
			}
			case IL_OPCODE_ARGS_STRING: {
				if ((dis_insn->data & IL_META_TOKEN_MASK) == IL_META_TOKEN_STRING) {
					dword strofs = dis_insn->data & (~IL_META_TOKEN_MASK);
					char *str = NULL;
					if (string_func) {
						str = string_func(strofs, context);
					}
					if (str) {
						sprintf(insnstr, "%-15s %s\"%s\"", dis_insn->opcode->name, cs_string, str);
						break;
					}
				}
				sprintf(insnstr, "%-15s %s0x%08x", dis_insn->opcode->name, cs_number, dis_insn->data);
				break;
			}
			case IL_OPCODE_ARGS_LONG_JUMP:
			case IL_OPCODE_ARGS_SHORT_JUMP: {
				CPU_ADDR caddr;
				caddr.addr32.offset = (dword)dis_insn->data;
				int slen;
				char *s = (addr_sym_func) ? addr_sym_func(caddr, &slen, addr_sym_func_context) : NULL;
				if (s) {
					char *p = insnstr + sprintf(insnstr, "%-15s ", dis_insn->opcode->name);
					memmove(p, s, slen);
					p[slen] = 0;
				} else {
					sprintf(insnstr, "%-15s %s0x%08x", dis_insn->opcode->name, cs_number, dis_insn->data);
				}
				break;
			}
			default:
				sprintf(insnstr, "%-15s [unsupported paramtype]", dis_insn->opcode->name);
		}
	}
	
	disable_highlighting();
	return insnstr;     
}

OBJECT_ID ILDisassembler::object_id() const
{
	// FIXME
	return 0;
}

bool ILDisassembler::validInsn(dis_insn *disasm_insn)
{
	return ((ILDisInsn *)disasm_insn)->valid;
}

	

