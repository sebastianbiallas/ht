/*
 *	HT Editor
 *	ia64dis.cc
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

#include <string.h>

#include "global.h"
#include "htendian.h"
#include "ia64dis.h"
#include "ia64opc.h"
#include "snprintf.h"

IA64Disassembler::IA64Disassembler()
{
}

IA64Disassembler::~IA64Disassembler()
{
}

bool IA64Disassembler::selectNext(dis_insn *disasm_insn)
{
	IA64DisInsn *insn = (IA64DisInsn *)disasm_insn;
	if (!insn->valid) return false;
	insn->selected += insn->slot[insn->selected].next;
	if (insn->selected > 2) {
		insn->selected = 0;
		return false;
	}
	return true;
}

qword IA64Disassembler::signExtend(qword a, int length)
{
	qword sign = (to_qword(1) << (length-1));
	if ((a & sign) != to_qword(0)) {
		sign <<= 1;
		sign -= to_qword(1);
		sign = ~sign;
		a |= sign;
	}
	return a;
}

void IA64Disassembler::decodeSlot(int slot_nb)
{
	IA64SlotDisInsn *slot = &insn.slot[slot_nb];
	byte role = (insn.tmplt->slot[slot_nb] & 0xf0);
	if (role==IA64_INST_ROLE_LONG) {
		qword tmp = insn.slot[slot_nb].data;
		insn.slot[slot_nb].data = insn.slot[slot_nb+1].data;
		insn.slot[slot_nb+1].data = tmp;
		slot->next = 2;
	} else {
		slot->next = 1;
	}
	role >>= 4;
	
	dword major_opcode = QWORD_GET_LO((slot->data >> 37) & to_qword(0x0f));
	IA64DecisionTreeEntry dtree_entry = IA64DecisionTree[major_opcode * IA64_INST_ROLE_COUNT + role];

	while (!IA64_DECISION_TREE_LEAF_NODE(dtree_entry)) {
		char pos = dtree_entry.pos - 6;
		char size  = dtree_entry.size;
		dword value;

		if (pos < 0) {
			/* extensions in bits 0-5 */
			// FIXME:redundant?
			pos += 6;
			value = QWORD_GET_LO(slot->data >> pos) & ((1 << size)-1);
		} else {
			value = QWORD_GET_LO(slot->data >> (pos+6)) & ((1 << size)-1);
		}
		word next = dtree_entry.next_node + value;
		dtree_entry = IA64DecisionTree[next];
	}
	word inst_id = dtree_entry.next_node;
	
	if ((inst_id >= IA64_OPCODE_INST_LAST) || inst_id == IA64_OPCODE_ILLOP) {
		// FIXME: ..
		slot->valid = false;
	} else {
		slot->valid = true;
		slot->opcode = &IA64OpcodeTable[inst_id];
		slot->qp = QWORD_GET_LO(slot->data) & 0x3f;
		for (int i=0; i<7; i++) slot->op[i].type = IA64_OPERAND_NO;
		int dest = 0;
		if (slot->opcode->op1.role == IA64_OPROLE_DST) {
			if (slot->opcode->op2.role == IA64_OPROLE_SRC) {
				dest = 2;
			} else {
				if (slot->opcode->op3.role == IA64_OPROLE_SRC) {
					dest = 3;
				} else {
					if (slot->opcode->op4.role == IA64_OPROLE_SRC) {
						dest = 4;
					} else {
						// ...
					}
				}
			}
		}
		if (dest) {
			slot->op[dest-1].type = IA64_OPERAND_EQUALS;
		}
/*		if (strcmp(slot->opcode->name, "adds")==0) {
			int as=0;
		}*/
		switch (slot->opcode->format) {
			case IA64_FORMAT_A1:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				slot->op[3].type = IA64_OPERAND_REG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				if (slot->opcode->op1.type == IA64_OPTYPE_ONE) {
					slot->op[4].type = IA64_OPERAND_1;
				}
				break;
			case IA64_FORMAT_A3:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_IMM;
				slot->op[2].imm = ((slot->data >> 13) & to_qword(0x7f))
							|(((slot->data >> 36) & to_qword(1)) << 7);
				slot->op[2].imm = signExtend(slot->op[2].imm, 8);
				slot->op[3].type = IA64_OPERAND_REG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_A4:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_IMM;
				slot->op[2].imm = ((slot->data >> 13) & to_qword(0x7f))
							| (((slot->data >> 27) & to_qword(0x3f)) << 7)
							|(((slot->data >> 36) & to_qword(1)) << 13);
				slot->op[2].imm = signExtend(slot->op[2].imm, 14);
				slot->op[3].type = IA64_OPERAND_REG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_A5:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_IMM;
				slot->op[2].imm = ((slot->data >> 13) & to_qword(0x7f))
							| (((slot->data >> 22) & to_qword(0x7fff)) << 7);
				slot->op[2].imm = signExtend(slot->op[2].imm, 22);
				slot->op[3].type = IA64_OPERAND_REG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x3));
				break;
			case IA64_FORMAT_A6:
			case IA64_FORMAT_A7:
				slot->op[0].type = IA64_OPERAND_PREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x3f));
				slot->op[1].type = IA64_OPERAND_PREG;
				slot->op[1].reg = QWORD_GET_LO((slot->data >> 27) & to_qword(0x3f));
				slot->op[3].type = IA64_OPERAND_REG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				slot->op[4].type = IA64_OPERAND_REG;
				slot->op[4].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_A8:
				slot->op[0].type = IA64_OPERAND_PREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x3f));
				slot->op[1].type = IA64_OPERAND_PREG;
				slot->op[1].reg = QWORD_GET_LO((slot->data >> 27) & to_qword(0x3f));
				slot->op[3].type = IA64_OPERAND_IMM;
				slot->op[3].imm = (slot->data >> 13) & to_qword(0x7f)
							|(((slot->data >> 36) & to_qword(1)) << 7);
				slot->op[3].imm = signExtend(slot->op[3].imm, 8);
				slot->op[4].type = IA64_OPERAND_REG;
				slot->op[4].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_B1:
			case IA64_FORMAT_B2:
				slot->op[0].type = IA64_OPERAND_ADDRESS;
				slot->op[0].ofs = ((slot->data >> 13) & to_qword((1<<20)-1))
							  |(((slot->data >> 36) & to_qword(1)) << 20);
				slot->op[0].ofs = signExtend(slot->op[0].ofs, 21);
				slot->op[0].ofs <<= 4;
				slot->op[0].ofs += cpu_addr.flat64.addr;
				break;
			case IA64_FORMAT_B3:
				slot->op[0].type = IA64_OPERAND_BREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7));
				slot->op[2].type = IA64_OPERAND_ADDRESS;
				slot->op[2].ofs = ((slot->data >> 13) & to_qword((1<<20)-1))
							  |(((slot->data >> 36) & to_qword(1)) << 20);
				slot->op[2].ofs = signExtend(slot->op[2].ofs, 21);
				slot->op[2].ofs <<= 4;
				slot->op[2].ofs += cpu_addr.flat64.addr;
				break;
			case IA64_FORMAT_B4:
				slot->op[0].type = IA64_OPERAND_BREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7));
				break;
			case IA64_FORMAT_B5:
				slot->op[0].type = IA64_OPERAND_BREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7));
				slot->op[2].type = IA64_OPERAND_BREG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7));
				break;
			case IA64_FORMAT_B9:
			case IA64_FORMAT_F15:
			case IA64_FORMAT_I19:
			case IA64_FORMAT_M37:
				slot->op[0].type = IA64_OPERAND_IMM;
				slot->op[0].imm = ((slot->data >> 6) & to_qword((1<<20)-1))
							  |(((slot->data >> 36) & to_qword(1)) << 20);
				break;
			case IA64_FORMAT_F2:               
				slot->op[0].type = IA64_OPERAND_FREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_FREG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				slot->op[3].type = IA64_OPERAND_FREG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 27) & to_qword(0x7f));
				slot->op[4].type = IA64_OPERAND_FREG;
				slot->op[4].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				break;
			case IA64_FORMAT_I21:
				slot->op[0].type = IA64_OPERAND_BREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7));
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				slot->op[3].type = IA64_OPERAND_ADDRESS;
				slot->op[3].imm = (slot->data >> 24) & to_qword(0x1ff);
				slot->op[3].imm = (signExtend(slot->op[3].imm, 9)<<4)+cpu_addr.flat64.addr;
				break;
			case IA64_FORMAT_I22:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_BREG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7));
				break;
			case IA64_FORMAT_I23:
				slot->op[0].type = IA64_OPERAND_PRALL;
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				slot->op[3].type = IA64_OPERAND_IMM;
				slot->op[3].imm = (((slot->data >> 6) & to_qword(0x7f)) << 1)
							|(((slot->data >> 24) & to_qword(0xff)) << 8)
							|(((slot->data >> 36) & to_qword(1)) << 16);
				slot->op[3].imm = signExtend(slot->op[3].imm, 17);
				break;
			case IA64_FORMAT_I24:
				slot->op[0].type = IA64_OPERAND_PRROT;
				slot->op[2].type = IA64_OPERAND_IMM;
				slot->op[2].imm = (((slot->data >> 6) & to_qword(0x7ffffff)) << 16)
							|(((slot->data >> 36) & to_qword(1)) << 43);
				slot->op[2].imm = signExtend(slot->op[2].imm, 28);
				break;
			case IA64_FORMAT_I25:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				if (slot->opcode->op2.type == IA64_OPTYPE_IP) {
					slot->op[2].type = IA64_OPERAND_IP;
				} else {
					slot->op[2].type = IA64_OPERAND_PRALL;
				}
				break;
			case IA64_FORMAT_I26:
			case IA64_FORMAT_M29:
				slot->op[0].type = IA64_OPERAND_AREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				break;
			case IA64_FORMAT_I27:
			case IA64_FORMAT_M31:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_AREG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_I28:
				break;
			case IA64_FORMAT_I29:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_M3:
				slot->op[3].type = IA64_OPERAND_IMM;
				slot->op[3].imm = ((slot->data >> 13) & to_qword(0x7f))
							|(((slot->data >> 27) & to_qword(1))<<7)
							|(((slot->data >> 36) & to_qword(1))<<8);
				slot->op[3].imm = signExtend(slot->op[3].imm, 9);
				goto m1;
			case IA64_FORMAT_M2:
				slot->op[3].type = IA64_OPERAND_REG;
				slot->op[3].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				// fall-through
			case IA64_FORMAT_M1:
				m1:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_MEM_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_M5:
				slot->op[3].type = IA64_OPERAND_IMM;
				slot->op[3].imm = ((slot->data >> 6) & to_qword(0x7f))
							|(((slot->data >> 27) & to_qword(1))<<7)
							|(((slot->data >> 36) & to_qword(1))<<8);
				slot->op[3].imm = signExtend(slot->op[3].imm, 9);
				goto m4;
			case IA64_FORMAT_M4:
				m4:
				slot->op[0].type = IA64_OPERAND_MEM_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				break;
			case IA64_FORMAT_M18:
				slot->op[0].type = IA64_OPERAND_FREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				break;
			case IA64_FORMAT_M19:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_FREG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				break;
			case IA64_FORMAT_M30:
				slot->op[0].type = IA64_OPERAND_AREG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_IMM;
				slot->op[2].imm = (slot->data >> 13) & to_qword(0x7f)
							|(((slot->data >> 36) & to_qword(1)) << 7);
				slot->op[2].imm = signExtend(slot->op[2].imm, 8);
				break;
			case IA64_FORMAT_M32:
				break;
			case IA64_FORMAT_M33:
				break;
			case IA64_FORMAT_M34:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_AR_PFS;
				slot->op[3].type = IA64_OPERAND_IMM;
				slot->op[3].imm = (slot->data >> 20) & to_qword(0x7f);
				slot->op[4].type = IA64_OPERAND_IMM;
				slot->op[4].imm = to_qword(0);
				slot->op[5].type = IA64_OPERAND_IMM;
				slot->op[5].imm = (slot->data >> 13) & to_qword(0x7f)-slot->op[3].imm;
				slot->op[6].type = IA64_OPERAND_IMM;
				slot->op[6].imm = ((slot->data >> 27) & to_qword(0xf))<<3;
				break;
			case IA64_FORMAT_M35:
				break;
			case IA64_FORMAT_M36:
				break;
			case IA64_FORMAT_M42:
				slot->op[0].type = IA64_OPERAND_REG_FILE;
				slot->op[0].regfile.db = slot->opcode->op1.type - IA64_OPTYPE_PMC;
				slot->op[0].regfile.idx = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				
				slot->op[2].type = IA64_OPERAND_REG;
				slot->op[2].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				break;
			case IA64_FORMAT_M43:
				break;
			case IA64_FORMAT_M45:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 13) & to_qword(0x7f));
				slot->op[1].type = IA64_OPERAND_REG;
				slot->op[1].reg = QWORD_GET_LO((slot->data >> 20) & to_qword(0x7f));
				break;
			case IA64_FORMAT_X2:
				slot->op[0].type = IA64_OPERAND_REG;
				slot->op[0].reg = QWORD_GET_LO((slot->data >> 6) & to_qword(0x7f));
				slot->op[2].type = IA64_OPERAND_IMM;
				slot->op[2].imm = ((slot->data >> 13) & to_qword(0x7f))
							|(((slot->data >> 27) & to_qword(0x1ff)) << 7)
							|(((slot->data >> 22) & to_qword(0x1f)) << 16)
							|(((slot->data >> 21) & to_qword(1)) << 21)
							|(insn.slot[slot_nb+1].data << 22)
							|(((slot->data >> 36) & to_qword(1)) << 63);
				break;
			default:
				break;
		}
	}
	
}

dis_insn *IA64Disassembler::decode(byte *code, byte maxlen, CPU_ADDR addr)
{
/*	byte data[] = {0x19, 0x18, 0x00, 0x18, 0x1e, 0x10, 0x00, 0x09, 0x79, 0xb0, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x20};
     code = data;*/
	cpu_addr = addr;
	insn.selected = 0;
	if (maxlen < 16) {
		// invalid
		insn.valid = false;
		insn.size = maxlen;
		if (maxlen) memcpy(insn.data, code, maxlen);
	} else {
		insn.valid = true;
		insn.size = 16;
		if (maxlen) memcpy(insn.data, code, maxlen);

		insn.tmplt_idx = code[0] & 0x1f;
		insn.tmplt = &IA64Templates[insn.tmplt_idx];

		if (insn.tmplt->slot[0] == IA64_SLOT_INVALID) {
			insn.valid = false;
		} else {
/*

01101011 11101011 10110111 00000000
00100000 01011111 11111111 11111111
11111111 11111111 11100001 00000000
00011000 01000001 00010000 00000101





orig:
00000010 01100000 10000000 00011001
00111111 00100011 00100000 00000010
10000000 00000000 01000010 11000000
00000001 00100000 00000001 10000100

rev:
10000100 00000001 00100000 00000001
11000000 0.1000010 00000000 10000000
00000010 00100000 00.100011 00111111
00011001 10000000 01100000 000.00010

10001100111111000110011000000001100000000
10001100111111000110011000000001100000000
*/
			QWORD_SET_LO(insn.slot[0].data,
               	  ((dword)(code[0] >> 5))
				| (((dword)code[1]) << 3)
				| (((dword)code[2]) << 11)
				| (((dword)code[3]) << 19)
				| (((dword)code[4] & 0x1f) << 27));    // 32 bits
			QWORD_SET_HI(insn.slot[0].data,
               	  ((dword)(code[4] >> 5))
				| (((dword)code[5] & 0x3f) << 3));     // +9 = 41 bits

			QWORD_SET_LO(insn.slot[1].data,
               	  ((dword)(code[5] >> 6))
				| (((dword)code[6]) << 2)
				| (((dword)code[7]) << 10)
				| (((dword)code[8]) << 18)
				| (((dword)code[9] & 0x3f) << 26));    // 32 bits
			QWORD_SET_HI(insn.slot[1].data,
               	  ((dword)(code[9] >> 6))
				| (((dword)code[10] & 0x7f) << 2));    // +9 = 41 bits

			QWORD_SET_LO(insn.slot[2].data,
               	  ((dword)(code[10] >> 7))
				| (((dword)code[11]) << 1)
				| (((dword)code[12]) << 9)
				| (((dword)code[13]) << 17)
				| (((dword)code[14] & 0x7f) << 25));   // 32 bits
			QWORD_SET_HI(insn.slot[2].data,
               	  ((dword)(code[14] >> 7))
				| (((dword)code[15]) << 1));           // +9 = 41 bits
		}
		for (int i=0; i<3; ) {
			insn.slot[i].valid = false;
			decodeSlot(i);
			i += insn.slot[i].next;
		}
	}
	return (dis_insn*)&insn;
}

dis_insn *IA64Disassembler::duplicateInsn(dis_insn *disasm_insn)
{
	IA64DisInsn *insn = (IA64DisInsn *)malloc(sizeof (IA64DisInsn));
	*insn = *(IA64DisInsn *)disasm_insn;
	return insn;
}

int IA64Disassembler::getMaxOpcodeLength()
{
	// ia64 instructions are all 128 bits long
	return 16;
}

void IA64Disassembler::getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align)
{
	min_length = 16;
	max_length = 16;
	min_look_ahead = 16;
	avg_look_ahead = 16;
	addr_align = 16;
}

byte IA64Disassembler::getSize(dis_insn *disasm_insn)
{
	return ((IA64DisInsn*)disasm_insn)->size;
}

char *IA64Disassembler::getName()
{
	return "IA64/Disassembler";
}

char *IA64Disassembler::str(dis_insn *disasm_insn, int style)
{
	return strf(disasm_insn, style, "");
}

char *IA64Disassembler::strf(dis_insn *disasm_insn, int style, char *format)
{
	if (style & DIS_STYLE_HIGHLIGHT) enable_highlighting();

	const char *cs_default = get_cs(e_cs_default);
	const char *cs_number = get_cs(e_cs_number);
	const char *cs_symbol = get_cs(e_cs_symbol);
//	const char *cs_string = get_cs(e_cs_string);
	const char *cs_comment = get_cs(e_cs_comment);

	IA64DisInsn *dis_insn = (IA64DisInsn *) disasm_insn;
	if (!dis_insn->valid) {
//		is_invalid:
//          assert(dis_insn->size==1);
		char *is = insnstr + sprintf(insnstr, "db              ");
		for (int i=0; i < dis_insn->size; i++) {
			is += sprintf(is, "%s%02x", cs_number, dis_insn->data[i]);
			if (i==7) is += sprintf(is, "-");
//               if (i+1 != dis_insn->size) is += sprintf(is, "%s, ", cs_symbol);
		}
	} else {
		char *is = insnstr;
		IA64SlotDisInsn *slot = &dis_insn->slot[dis_insn->selected];
		is[0] = 0;
//          char c[] = ".#'";
		if (slot->valid) {
			char qp[6];
			if (slot->qp) {
				sprintf(qp, "(p%d)", slot->qp);
			} else {
				qp[0]=0;
			}
			is += ht_snprintf(is, 256, "%s%d %5s %s%-20s", cs_comment, dis_insn->selected, qp, cs_default, slot->opcode->name);
			for (int i=0; i<7; i++) {
				if (slot->op[i].type==IA64_OPERAND_NO) break;
				if (slot->op[i].type==IA64_OPERAND_EQUALS) {
					is += ht_snprintf(is, 256, " %s= ", cs_symbol);
					i++;
					goto w;
				} else {
					if (i!=0) is += ht_snprintf(is, 256, "%s, ", cs_symbol);
				}
				w:
				switch (slot->op[i].type) {
					case IA64_OPERAND_1:
						is += ht_snprintf(is, 256, "%s1", cs_number);
						break;
					case IA64_OPERAND_REG:
						is += ht_snprintf(is, 256, "%sr%d", cs_default, slot->op[i].reg);
						break;                              
					case IA64_OPERAND_BREG:
						is += ht_snprintf(is, 256, "%sbr%d", cs_default, slot->op[i].reg);
						break;                              
					case IA64_OPERAND_FREG:
						is += ht_snprintf(is, 256, "%sf%d", cs_default, slot->op[i].reg);
						break;                              
					case IA64_OPERAND_PREG:
						is += ht_snprintf(is, 256, "%sp%d", cs_default, slot->op[i].reg);
						break;
					case IA64_OPERAND_AREG:
						is += ht_snprintf(is, 256, "%sar%d", cs_default, slot->op[i].reg);
						break;
					case IA64_OPERAND_PRALL:
						is += ht_snprintf(is, 256, "%spr", cs_default);
						break;
					case IA64_OPERAND_PRROT:
						is += ht_snprintf(is, 256, "%spr.rot", cs_default);
						break;
					case IA64_OPERAND_AR_PFS:
						is += ht_snprintf(is, 256, "%sar.pfs", cs_default);
						break;
					case IA64_OPERAND_IP:
						is += ht_snprintf(is, 256, "%sip", cs_default);
						break;
					case IA64_OPERAND_MEM_REG:
						is += ht_snprintf(is, 256, "%s[%sr%d%s]", cs_symbol, cs_default, slot->op[i].reg, cs_symbol);
						break;                              
					case IA64_OPERAND_IMM:
						is += ht_snprintf(is, 256, "%s%qx", cs_number, &slot->op[i].imm);
						break;                              
					case IA64_OPERAND_ADDRESS: {
						CPU_ADDR caddr;
						caddr.flat64.addr = slot->op[i].ofs;
						int slen;
						char *s = (addr_sym_func) ? addr_sym_func(caddr, &slen, addr_sym_func_context) : NULL;
						if (s) {
							char *p = is;
							memmove(p, s, slen);
							p[slen] = 0;
							is += slen;
						} else {
							is += ht_snprintf(is, 256, "%s0x%qx", cs_number, &slot->op[i].ofs);
						}
						break;
					}
					case IA64_OPERAND_REG_FILE: {
						char *dbs[] = {"pmc", "pmd", "pkr", "rr", "ibr", "dbr", "itr", "dtr", "msr"};
						is += ht_snprintf(is, 256, "%s%s[%sr%d%s]", dbs[slot->op[i].regfile.db], cs_symbol, cs_default, slot->op[i].regfile.idx, cs_symbol);
					}
				}
			}
		} else {
			is += ht_snprintf(is, 256, "%s%d       %-20s", cs_comment, dis_insn->selected, "invalid");
		}
//		line += insn.slot[line].next;
//		if (line > 2) line = 0;
		//
		
		char tmplt_str[100];
		tmplt_str[0] = 0;
		char *t = tmplt_str;
		for (int i=0; i<3; i++) {
		switch (insn.tmplt->slot[i] & 0x0f) {
			case IA64_SLOT_INVALID:
				t+=sprintf(t, "*");
				goto e;
				break;
			case IA64_SLOT_M_UNIT:
				t+=sprintf(t, "M");
				break;
			case IA64_SLOT_I_UNIT:
				t+=sprintf(t, "I");
				break;
			case IA64_SLOT_L_UNIT:
				t+=sprintf(t, "L");
				break;
			case IA64_SLOT_X_UNIT:
				t+=sprintf(t, "X");
				break;
			case IA64_SLOT_F_UNIT:
				t+=sprintf(t, "F");
				break;
			case IA64_SLOT_B_UNIT:
				t+=sprintf(t, "B");
				break;
		}
		}
		e:;
//		is += ht_snprintf(is, 256, "                   t=%02x(%s) s0=%013Q s1=%013Q s2=%013Q", insn.tmplt_idx, tmplt_str, &insn.slot[0].data, &insn.slot[1].data, &insn.slot[2].data);
/*          for (int i=0; i < dis_insn->size; i++) {
			is += sprintf(is, "%s%02x", cs_number, dis_insn->data[i]);
			if (i==7) is += sprintf(is, "-");
		}*/
	}
	
	disable_highlighting();
	return insnstr;     
}

OBJECT_ID IA64Disassembler::object_id()
{
	return ATOM_DISASM_IA64;
}

bool IA64Disassembler::validInsn(dis_insn *disasm_insn)
{
	return ((IA64DisInsn *)disasm_insn)->valid;
}

	

