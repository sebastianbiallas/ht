/*
 *	HT Editor
 *	alphaopc.h
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

#ifndef ALPHAOPC_H
#define ALPHAOPC_H

#include "io/types.h"

struct alpha_opcode_tab_entry {
	uint16	fcode;
	const char *name;
	byte	type;
};

extern const char *alpha_reg_names[];
extern alpha_opcode_tab_entry alpha_instr_tbl[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext10[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext11[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext12[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext13[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext14[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext15[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext16[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext17[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext18[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext1a[];
extern alpha_opcode_tab_entry alpha_instr_tbl_ext1c[];

#define REG_ZERO	0x1f
#define REG_FLOAT	0x20
#define REG_FZERO	0x3f
#define REG_LIT	0x40


/* unknown/undefined/illegal opcode */
#define ALPHA_ERROR 0

/* extensions */
#define ALPHA_EXTENSION_10  100
#define ALPHA_EXTENSION_11  101
#define ALPHA_EXTENSION_12  102
#define ALPHA_EXTENSION_13  103
#define ALPHA_EXTENSION_14  104
#define ALPHA_EXTENSION_15  105
#define ALPHA_EXTENSION_16  106
#define ALPHA_EXTENSION_17  107
#define ALPHA_EXTENSION_18  108
#define ALPHA_EXTENSION_1A  109
#define ALPHA_EXTENSION_1C  110

/* opcode groups somehow without system */
#define ALPHA_GROUP1 200
#define ALPHA_GROUP2 201
#define ALPHA_GROUP3 202
#define ALPHA_GROUP_FLD 203
#define ALPHA_GROUP4 204
#define ALPHA_GROUP_FST 205
#define ALPHA_GROUP_F2I 206
#define ALPHA_GROUP_I2F 207
#define ALPHA_GROUP_BRA 208
#define ALPHA_GROUP_FBR 209
#define ALPHA_GROUP_JMP 210
#define ALPHA_GROUP_PAL 211

#endif
