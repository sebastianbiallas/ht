/*
 *	HT Editor
 *	javadis.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "stream.h"
#include "store.h"
#include "tools.h"
#include "javadis.h"

#include <stdio.h>
#include <string.h>

/*
 *	CLASS javadis
 */

javadis::javadis()
{
}

javadis::javadis(java_token_func tf, void *c)
{
	token_func = tf;
	context = c;
}

javadis::~javadis()
{
}

dis_insn *javadis::decode(byte *code, byte Maxlen, CPU_ADDR Addr)
{
	ocodep = code;
/* initialize */
	codep = ocodep;
	maxlen = Maxlen;
	addr = Addr.addr32.offset;
	memset(&insn, 0, sizeof(insn));
	insn.invalid = false;

	insn.opcode = getbyte();
	if (insn.opcode == JAVA_WIDE_OPCODE) {
		insn.wideopcode = true;
		insn.opcode = getbyte();
	} else {
		insn.wideopcode = false;
	}
	decode_insn(&java_insns[insn.opcode]);

	if (insn.invalid) {
		insn.name = "db";
		insn.size = 1;
		insn.op[0].type = JAVA_OPTYPE_IMM;
		insn.op[0].size = 1;
		insn.op[0].imm = *code;
		for (int i=1; i<JAVAINSN_MAX_PARAM_COUNT; i++) insn.op[i].type=JAVA_OPTYPE_EMPTY;
	} else {
		insn.size=codep-ocodep;
	}
	return &insn;
}

void javadis::decode_insn(javaopc_insn *xinsn)
{
	if (!xinsn->name) {
		invalidate();
		return;
	}
	insn.name = xinsn->name;
	for (int i=0; i<JAVAINSN_MAX_PARAM_COUNT; i++) {
		decode_op(xinsn->optype[i], insn.wideopcode, &insn.op[i]);
	}
}

void javadis::decode_op(int optype, bool wideopc, java_insn_op *op)
{
	bool widesize = wideopc || (JOPC_SIZE(optype) == JOPC_SIZE_WIDE);
	switch (JOPC_TYPE(optype)) {
		case JOPC_TYPE_CHAR:
			op->type = JAVA_OPTYPE_IMM;
			op->size = 2;
			op->imm = getword();
			break;
		case JOPC_TYPE_BYTE:
			op->type = JAVA_OPTYPE_IMM;
			op->size = 1;
			op->imm = getbyte();
			break;
		case JOPC_TYPE_SHORT:
			op->type = JAVA_OPTYPE_IMM;
			op->size = 2;
			op->imm = getword();
			break;
		case JOPC_TYPE_INT:
			op->type = JAVA_OPTYPE_IMM;
			op->size = 4;
			op->imm = getdword();
			break;
		case JOPC_TYPE_CONST:
			op->type = JAVA_OPTYPE_CONST;
			if (widesize) {
				op->size = 2;
				op->imm = getword();
			} else {
				op->size = 1;
				op->imm = getbyte();
			}
			break;
		case JOPC_TYPE_LOCAL:
			op->type = JAVA_OPTYPE_IMM;
			if (widesize) {
				op->size = 2;
				op->imm = getword();
			} else {
				op->size = 1;
				op->imm = getbyte();
			}
			break;
		case JOPC_TYPE_LABEL:
			op->type = JAVA_OPTYPE_LABEL;
			if (widesize) {
				op->size = 4;
				// FIXME: sint32
				op->label = addr + (int)getdword() - op->size - 1;
			} else {
				op->size = 2;
				// FIXME: sint16
				op->label = addr + (short)getword() - op->size - 1;
			}
			break;
		default:
			op->type = JAVA_OPTYPE_EMPTY;
	}
}

dis_insn *javadis::duplicateInsn(dis_insn *disasm_insn)
{
	javadis_insn *insn = (javadis_insn *)malloc(sizeof (javadis_insn));
	*insn = *(javadis_insn *)disasm_insn;
	return insn;
}

byte javadis::getbyte()
{
	if (codep-ocodep+1<=maxlen) {
		addr++;
		return *(codep++);
	} else {
		invalidate();
		return 0;
	}
}

word javadis::getword()
{
	if (codep-ocodep+2<=maxlen) {
		word w;
		addr += 2;
		w = codep[1] | (codep[0]<<8);
		codep += 2;
		return w;
	} else {
		invalidate();
		return 0;
	}
}

dword javadis::getdword()
{
	if (codep-ocodep+4<=maxlen) {
		dword w;
		addr += 4;
		w = codep[3] | (codep[2]<<8) | (codep[1]<<16) | (codep[0]<<24);
		codep += 4;
		return w;
	} else {
		invalidate();
		return 0;
	}
}

void javadis::getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align)
{
	min_length = 1;
	max_length = 6;
	min_look_ahead = 120;
	avg_look_ahead = 24;
	addr_align = 1;
}

char *javadis::getName()
{
	return "Java/Disassembler";
}

byte javadis::getSize(dis_insn *disasm_insn)
{
	return ((javadis_insn*)disasm_insn)->size;
}

void javadis::invalidate()
{
	insn.invalid = true;
}

int javadis::load(ht_object_stream *f)
{
	return f->get_error();
}

OBJECT_ID javadis::object_id() const
{
	return ATOM_DISASM_JAVA;
}

void javadis::str_op(char *opstr, int *opstrlen, javadis_insn *insn, java_insn_op *op)
{
	const char *cs_default = get_cs(e_cs_default);
	const char *cs_number = get_cs(e_cs_number);
//	const char *cs_symbol = get_cs(e_cs_symbol);
	const char *cs_comment = get_cs(e_cs_comment);

	*opstrlen=0;
	switch (op->type) {
		case JAVA_OPTYPE_CONST: {
			char *g=opstr;
			strcpy(g, cs_comment); g += strlen(cs_comment);
			g += token_func(g, 1024, op->imm, context);
			*(g++) = ' ';
			strcpy(g, cs_number); g += strlen(cs_number);
			switch (op->size) {
				case 1:
					hexd(&g, 2, options, op->imm);
					break;
				case 2:
					hexd(&g, 4, options, op->imm);
					break;
				case 4:
					hexd(&g, 8, options, op->imm);
					break;
			}
			break;
		}
		case JAVA_OPTYPE_LABEL: {
			CPU_ADDR a;
			a.addr32.offset=op->imm;
			int slen;
			char *s=(addr_sym_func) ? addr_sym_func(a, &slen, addr_sym_func_context) : NULL;
			if (s) {
				memmove(opstr, s, slen);
				opstr[slen]=0;
				*opstrlen=slen;
			} else {
				char *g=opstr;
				strcpy(g, cs_number); g += strlen(cs_number);
				switch (op->size) {
					case 1:
						hexd(&g, 2, options, op->imm);
						break;
					case 2:
						hexd(&g, 4, options, op->imm);
						break;
					case 4:
						hexd(&g, 8, options, op->imm);
						break;
				}
				strcpy(g, cs_default); g += strlen(cs_default);
			}
			break;
		}
		case JAVA_OPTYPE_IMM: {
			CPU_ADDR a;
			a.addr32.offset=op->imm;
			char *g=opstr;
			strcpy(g, cs_number); g += strlen(cs_number);
			switch (op->size) {
				case 1:
					hexd(&g, 2, options, op->imm);
					break;
				case 2:
					hexd(&g, 4, options, op->imm);
					break;
				case 4:
					hexd(&g, 8, options, op->imm);
					break;
			}
			strcpy(g, cs_default); g += strlen(cs_default);
			break;
		}
		default:
			opstr[0]=0;
	}
}

void javadis::str_format(char **str, char **format, char *p, char *n, char *op[3], int oplen[3], char stopchar, int print)
{
	
	const char *cs_default = get_cs(e_cs_default);
	const char *cs_symbol = get_cs(e_cs_symbol);

	char *f=*format;
	char *s=*str;
	while (*f) {
		if (*f==stopchar) break;
		switch (*f) {
			case '\t':
				if (print) do *(s++)=' '; while ((s-insnstr) % 16);
				break;
			case DISASM_STRF_VAR:
				f++;
				if (print) {
					char *t=0;
					int tl=0;
					switch (*f) {
						case DISASM_STRF_PREFIX:
							t=p;
							break;
						case DISASM_STRF_NAME:
							t=n;
							break;
						case DISASM_STRF_FIRST:
							t=op[0];
							tl=oplen[0];
							break;
						case DISASM_STRF_SECOND:
							t=op[1];
							tl=oplen[1];
							break;
						case DISASM_STRF_THIRD:
							t=op[2];
							tl=oplen[2];
							break;
					}
					if (tl) {
						memmove(s, t, tl);
						s+=tl;
						*s=0;
					} else {
						strcpy(s, t);
						s += strlen(s);
					}
				}
				break;
			case DISASM_STRF_COND: {
				char *t=0;
				f++;
				switch (*f) {
					case DISASM_STRF_PREFIX:
						t=p;
						break;
					case DISASM_STRF_NAME:
						t=n;
						break;
					case DISASM_STRF_FIRST:
						t=op[0];
						break;
					case DISASM_STRF_SECOND:
						t=op[1];
						break;
					case DISASM_STRF_THIRD:
						t=op[2];
						break;
				}
				f+=2;
				if ((t) && (t[0])) {
					str_format(&s, &f, p, n, op, oplen, *(f-1), 1);
				} else {
					str_format(&s, &f, p, n, op, oplen, *(f-1), 0);
				}
				break;
			}
			default:
				if (print) {
					bool x = (strchr(",.-=+-*/[]()", *f) != NULL) && *f;
					if (x) { strcpy(s, cs_symbol); s += strlen(cs_symbol); }
					*(s++) = *f;
					if (x) { strcpy(s, cs_default); s += strlen(cs_default); }
				}
		}
		f++;
	}
	*s=0;
	*format=f;
	*str=s;
}

char *javadis::str(dis_insn *disasm_insn, int options)
{
	return strf(disasm_insn, options, DISASM_STRF_DEFAULT_FORMAT);
}

char *javadis::strf(dis_insn *disasm_insn, int opt, char *format)
{
	javadis_insn *insn = (javadis_insn*)disasm_insn;
	char prefix[64];
	char *p = prefix;
	options = opt;
	*p = 0;

	char ops[3][512];	/* FIXME: possible buffer overflow ! */
	char *op[3];
	int oplen[3];

	if (options & DIS_STYLE_HIGHLIGHT) enable_highlighting();
	for (int i=0; i<3; i++) {
		op[i]=(char*)&ops[i];
		str_op(op[i], &oplen[i], insn, &insn->op[i]);
	}
	char *s=insnstr;
	str_format(&s, &format, prefix, insn->name, op, oplen, 0, 1);
	disable_highlighting();
	return insnstr;
}

void javadis::store(ht_object_stream *f)
{
}

bool javadis::validInsn(dis_insn *disasm_insn)
{
	return !((javadis_insn *)disasm_insn)->invalid;
}
