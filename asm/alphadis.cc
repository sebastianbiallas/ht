/*
 *	HT Editor
 *	alphadis.cc
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

#include "alphadis.h"
#include "alphaopc.h"
#include "htdebug.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

#define BITS_OPC(opcode)		((opcode >> 26) & 0x3f)
#define BITS_FFUN(opcode)	((opcode >> 5)  & 0x7ff)
#define BITS_IFUN(opcode)	((opcode >> 5)  & 0x7f)
#define BITS_MFUN(opcode)	(opcode  & 0xffff)
#define BITS_JFUN(opcode)	((opcode >> 14) & 0x3)
#define BITS_ISLIT(opcode)	((opcode >> 12) & 0x1)
#define BITS_IMMED(opcode)	((opcode >> 13) & 0xff)
#define BITS_REGA(opcode)	((opcode >> 21) & 0x1f)
#define BITS_REGB(opcode)	((opcode >> 16) & 0x1f)
#define BITS_REGC(opcode)	(opcode & 0x1f)
#define BITS_BDISP(opcode)	(opcode & 0x1fffff)
#define BITS_BSIGN(opcode)	(opcode & 0x100000)
#define BITS_MDISP(opcode)	(opcode & 0xffff)
#define BITS_MSIGN(opcode)	(opcode & 0x8000)
#define BITS_PAL(opcode)		(opcode & 0x3ffffff)
#define BITS_HINT(opcode)	(opcode & 0x3fff)

/* only needed for assembler.. */
#define MAKE_OPC(opcode)		((opcode & 0x3f) << 26)
#define MAKE_FFUN(opcode)	((opcode & 0x7ff) << 5)
#define MAKE_IFUN(opcode)	((opcode & 0x7f) << 5)
#define MAKE_MFUN(opcode)	(opcode & 0xffff)
#define MAKE_JFUN(opcode)	((opcode & 0x3) << 14)
#define MAKE_LIT			(1<<12)
#define MAKE_IMMED(opcode)	((opcode & 0xff)  << 13 )
#define MAKE_REGA(opcode)	((opcode & 0x1f) << 21)
#define MAKE_REGB(opcode)	((opcode & 0x1f) << 16)
#define MAKE_REGC(opcode)	(opcode & 0x1f)
#define MAKE_BDISP(opcode)	(opcode & 0x1fffff)
#define MAKE_MDISP(opcode)	(opcode & 0xffff)
#define MAKE_PAL(opcode)		(opcode & 0x3ffffff)
#define MAKE_HINT(opcode)	(opcode & 0x3fff)

Alphadis::Alphadis()
	:Disassembler()
{
	insn.valid = false;
}

int find_alpha_instruction(alpha_opcode_tab_entry *table, int f)
{
	int i=0;
	while (f > (table+i)->fcode) i++;
	return i;
}

dis_insn *Alphadis::decode(byte *code, int maxlen, CPU_ADDR addr)
{
	// alpha code instructions must be 32 bits long
	if (maxlen < 4) {
		// invalid
		insn.valid = false;
		insn.size = maxlen;
		insn.table = 0;
		// FIXME: this reads to much bytes!
		UNALIGNED_MOVE(insn.data, *(uint32 *)code);
	} else {
		insn.valid = true;
		insn.size = 4;
		insn.table = &alpha_instr_tbl[0];
		uint32 opcode = *((uint32 *)code);
		int idx = BITS_OPC(opcode);
		switch (alpha_instr_tbl[idx].type) {
			case ALPHA_EXTENSION_10:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext10[0], BITS_IFUN(opcode));
				insn.table = &alpha_instr_tbl_ext10[0];
				break;
			case ALPHA_EXTENSION_11:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext11[0], BITS_IFUN(opcode));
				insn.table = &alpha_instr_tbl_ext11[0];
				break;
			case ALPHA_EXTENSION_12:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext12[0], BITS_IFUN(opcode));
				insn.table = &alpha_instr_tbl_ext12[0];
				break;
			case ALPHA_EXTENSION_13:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext13[0], BITS_IFUN(opcode));
				insn.table = &alpha_instr_tbl_ext13[0];
				break;
			case ALPHA_EXTENSION_14:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext14[0], BITS_FFUN(opcode));
				insn.table = &alpha_instr_tbl_ext14[0];
				break;
			case ALPHA_EXTENSION_15:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext15[0], BITS_FFUN(opcode));
				insn.table = &alpha_instr_tbl_ext15[0];
				break;
			case ALPHA_EXTENSION_16:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext16[0], BITS_FFUN(opcode));
				insn.table = &alpha_instr_tbl_ext16[0];
				break;
			case ALPHA_EXTENSION_17:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext17[0], BITS_FFUN(opcode));
				insn.table = &alpha_instr_tbl_ext17[0];
				break;
			case ALPHA_EXTENSION_18:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext18[0], BITS_MFUN(opcode));
				insn.table = &alpha_instr_tbl_ext18[0];
				break;
			case ALPHA_EXTENSION_1A:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext1a[0], BITS_JFUN(opcode));
				insn.table = &alpha_instr_tbl_ext1a[0];
				break;
			case ALPHA_EXTENSION_1C:
				idx = find_alpha_instruction(&alpha_instr_tbl_ext1c[0], BITS_IFUN(opcode));
				insn.table = &alpha_instr_tbl_ext1c[0];
				break;
		}

		insn.code = idx;

		switch ((insn.table+idx)->type) {
			case ALPHA_GROUP1:
				insn.regA = BITS_REGA(opcode);
				insn.regC = BITS_REGC(opcode);
				if (BITS_ISLIT(opcode)) {
					insn.regB = REG_LIT;
					insn.data = BITS_IMMED(opcode);
				} else {
					insn.regB = BITS_REGB(opcode);
					insn.data = 0;
				}
				break;
			case ALPHA_GROUP2:
				insn.regA = BITS_REGA(opcode) + REG_FLOAT;
				insn.regB = BITS_REGB(opcode) + REG_FLOAT;
				insn.regC = BITS_REGC(opcode) + REG_FLOAT;
				insn.data = 0;
				break;
			case ALPHA_GROUP3:
				insn.regA = BITS_REGA(opcode);
				insn.regB = BITS_REGB(opcode);
				insn.regC = REG_ZERO;
				insn.data = BITS_MDISP(opcode);
				if (BITS_MSIGN(insn.data)) insn.data |= -1 ^ 0xffff;
				break;
			case ALPHA_GROUP_FLD:
				insn.regA = BITS_REGA(opcode) + REG_FLOAT;
				insn.regB = BITS_REGB(opcode);
				insn.regC = REG_ZERO;
				insn.data = BITS_MDISP(opcode);
				if (BITS_MSIGN(insn.data)) insn.data |= -1 ^ 0xffff;
				break;
			case ALPHA_GROUP4:
				insn.regA = BITS_REGA(opcode);
				insn.regB = BITS_REGB(opcode);
				insn.regC = REG_ZERO;
				insn.data = BITS_MDISP(opcode);
				if (BITS_MSIGN(insn.data)) insn.data |= -1 ^ 0xffff;
				break;
			case ALPHA_GROUP_FST:
				insn.regA = BITS_REGA(opcode) + REG_FLOAT;
				insn.regB = BITS_REGB(opcode);
				insn.regC = REG_ZERO;
				insn.data = BITS_MDISP(opcode);
				if (BITS_MSIGN(insn.data)) insn.data |= -1 ^ 0xffff;
				break;
			case ALPHA_GROUP_F2I:
				insn.regA = BITS_REGA(opcode) + REG_FLOAT;
				insn.regB = REG_ZERO;
				insn.regC = BITS_REGC(opcode);
				insn.data = 0;
				break;
			case ALPHA_GROUP_I2F:
				insn.regA = BITS_REGA(opcode);
				insn.regB = REG_ZERO;
				insn.regC = BITS_REGC(opcode) + REG_FLOAT;
				insn.data = 0;
				break;
			case ALPHA_GROUP_BRA:
				insn.regA = BITS_REGA(opcode);
				insn.regB = REG_ZERO;
				insn.regC = REG_ZERO;
				insn.data = BITS_BDISP(opcode);
				if (BITS_BSIGN(insn.data)) insn.data |= -1 ^ 0x1fffff;
				insn.data += 1;
				insn.data *= 4;
				insn.data += addr.addr32.offset;
				break;
			case ALPHA_GROUP_FBR:
				insn.regA = BITS_REGA(opcode) + REG_FLOAT;
				insn.regB = REG_ZERO;
				insn.regC = REG_ZERO;
				insn.data = BITS_BDISP(opcode);
				if (BITS_BSIGN(insn.data)) insn.data |= -1 ^ 0x1fffff;
				insn.data += 1;
				insn.data *= 4;
				insn.data += addr.addr32.offset;
				break;
			case ALPHA_GROUP_JMP:
				insn.regA = BITS_REGA(opcode);
				insn.regB = BITS_REGB(opcode);
				insn.regC = REG_ZERO;
				insn.data = BITS_HINT(opcode);
				break;
			case ALPHA_GROUP_PAL:
				insn.regA = REG_ZERO;
				insn.regB = REG_ZERO;
				insn.regC = REG_ZERO;
				insn.data = BITS_PAL(opcode);
				break;
			default:
				insn.valid = false;
				insn.data = *(uint32 *)code;
				break;
		}
	}
	return &insn;
}

dis_insn *Alphadis::duplicateInsn(dis_insn *disasm_insn)
{
	alphadis_insn *insn = ht_malloc(sizeof (alphadis_insn));
	*insn = *(alphadis_insn *)disasm_insn;
	return insn;
}

void Alphadis::getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align)
{
	min_length = 4;
	max_length = 4;
	min_look_ahead = 4;
	avg_look_ahead = 4;
	addr_align = 4;
}

const char *Alphadis::getName()
{
	return "alpha/disassembler";
}

byte	Alphadis::getSize(dis_insn *disasm_insn)
{
	return ((alphadis_insn*)disasm_insn)->size;
}

ObjectID Alphadis::getObjectID() const
{
	return ATOM_DISASM_ALPHA;
}

const char *Alphadis::str(dis_insn *disasm_insn, int style)
{
	return strf(disasm_insn, style, "");
}

#define A_REG_A alpha_reg_names[alpha_insn->regA]
#define A_REG_B alpha_reg_names[alpha_insn->regB]
#define A_REG_C alpha_reg_names[alpha_insn->regC]
#define A_NAME (alpha_insn->table+alpha_insn->code)->name

const char *Alphadis::strf(dis_insn *disasm_insn, int style, const char *format)
{
	if (style & DIS_STYLE_HIGHLIGHT) enable_highlighting();
	
	const char *cs_default = get_cs(e_cs_default);
	const char *cs_number = get_cs(e_cs_number);
	const char *cs_symbol = get_cs(e_cs_symbol);

	alphadis_insn *alpha_insn = (alphadis_insn *) disasm_insn;
	
	if (!alpha_insn->valid) {
		is_invalid:
		switch (alpha_insn->size) {
			case 1:
				strcpy(insnstr, "db         ?");
				break;
			case 2:
				strcpy(insnstr, "dw         ?");
				break;
			case 3:
				strcpy(insnstr, "db         ? * 3");
				break;
			case 4:
				sprintf(insnstr, "dd         %s0x%08x", cs_number, alpha_insn->data);
				break;
			default: { /* braces for empty assert */
				assert(0);
			}
		}
	} else switch ((alpha_insn->table+alpha_insn->code)->type) {
		case ALPHA_GROUP1:
		case ALPHA_GROUP2:
			if (alpha_insn->regB != REG_LIT)
				sprintf(insnstr, "%-10s %s%s,%s %s%s,%s %s", A_NAME, A_REG_A, cs_symbol, cs_default, A_REG_B, cs_symbol, cs_default, A_REG_C);
			else
				sprintf(insnstr, "%-10s %s%s,%s %s0x%x%s,%s %s", A_NAME, A_REG_A, cs_symbol, cs_default, cs_number, alpha_insn->data, cs_symbol, cs_default, A_REG_C);
			break;
		case ALPHA_GROUP3:
		case ALPHA_GROUP_FLD:
		case ALPHA_GROUP4:
		case ALPHA_GROUP_FST: {
			short sdata = (short)(alpha_insn->data&0xffff);
			char c;
			if (sdata<0) {
				c = '-';
				sdata = -sdata;
			} else {
				c = '+';
			}
			sprintf(insnstr, "%-10s %s%s, [%s%s%s%c%s0x%x%s]", A_NAME, A_REG_A, cs_symbol, cs_default, A_REG_B, cs_symbol, c, cs_number, sdata, cs_symbol);
			break;
		}
		case ALPHA_GROUP_I2F:
		case ALPHA_GROUP_F2I:
			sprintf(insnstr, "%-10s %s%s,%s %s", A_NAME, A_REG_A, cs_symbol, cs_default, A_REG_C);
			break;
		case ALPHA_GROUP_BRA:
		case ALPHA_GROUP_FBR: {
			CPU_ADDR caddr;
			caddr.addr32.offset = (uint32)alpha_insn->data;
			int slen;
			char *p;
			char *s = (addr_sym_func) ? addr_sym_func(caddr, &slen, addr_sym_func_context) : 0;
			if (s) {
				p = insnstr+sprintf(insnstr, "%-10s %s%s,%s ", A_NAME, A_REG_A, cs_symbol, cs_default);
				memmove(p, s, slen);
				p[slen] = 0;
			} else {
				sprintf(insnstr, "%-10s %s%s, %s0x%x", A_NAME, A_REG_A, cs_symbol, cs_number, (uint32)alpha_insn->data);
			}
			break;
		}
		case ALPHA_GROUP_JMP: {
			CPU_ADDR caddr;
			caddr.addr32.offset = (uint32)alpha_insn->data;
			int slen;
			char *s = (addr_sym_func) ? addr_sym_func(caddr, &slen, addr_sym_func_context) : 0;
			if (s) {
				char *p = insnstr + sprintf(insnstr, "%-10s %s %s(%s%s%s),%s ", A_NAME, A_REG_A, cs_symbol, cs_default, A_REG_B, cs_symbol, cs_default);
				memmove(p, s, slen);
				p[slen] = 0;
			} else {
				sprintf(insnstr, "%-10s %s %s(%s%s%s), %s0x%x", A_NAME, A_REG_A, cs_symbol, cs_default, A_REG_B, cs_symbol, cs_number, (uint32)alpha_insn->data);
			}
			break;
		}
		case ALPHA_GROUP_PAL:
			sprintf(insnstr, "%-10s %s0x%08x", A_NAME, cs_number, alpha_insn->data);
			break;
		default:
			goto is_invalid;
	}
	disable_highlighting();
	return insnstr;     
}

bool	Alphadis::validInsn(dis_insn *disasm_insn)
{
	return ((alphadis_insn *)disasm_insn)->valid;
}

