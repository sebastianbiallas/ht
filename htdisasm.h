/* 
 *	HT Editor
 *	htdisasm.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#ifndef __HTDISASM_H__
#define __HTDISASM_H__

#include "asm.h"
#include "htobj.h"
#include "htformat.h"

#define DESC_DISASM "disasm/x86"

extern format_viewer_if htdisasm_if;

void dialog_assemble(ht_format_viewer *f, viewer_pos vaddr, CPU_ADDR cpuaddr, Assembler *a, Disassembler *disasm, const char *default_str, int want_length);

/*
 *	CLASS ht_disasm_viewer
 */

class ht_disasm_sub;

class ht_disasm_viewer: public ht_uformat_viewer {
protected:
	Assembler *assem;
	Disassembler *disasm;
	int op1632;

/* new */
	virtual	ht_disasm_sub *get_disasm_sub();
public:
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Assembler *a, Disassembler *d, int t);
	virtual void done();
/* overwritten */
	virtual	const char *func(uint i, bool execute);
	virtual	int  get_pindicator_str(char *buf, int max_len);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	virtual	void handlemsg(htmsg *msg);
	virtual	bool offset_to_pos(FileOfs ofs, viewer_pos *addr);
	virtual	bool pos_to_offset(viewer_pos addr, FileOfs *ofs);
	virtual	bool qword_to_pos(uint64 q, viewer_pos *pos);
	virtual	bool ref_sel(LINE_ID *id);
	virtual	bool symbol_handler(eval_scalar *result, char *name);
};

/*
 *	CLASS ht_disasm_sub
 */

class ht_disasm_sub: public ht_linear_sub {
private:
	Disassembler *disasm;
	bool own_disasm;
	int display_style;
public:
			void init(File *file, FileOfs ofs, int size, Disassembler *disasm, bool own_disasm, int display_style);
	virtual 	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
};

#endif /* !__HTDISASM_H__ */

