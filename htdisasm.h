/* 
 *	HT Editor
 *	htdisasm.h
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

#ifndef __HTDISASM_H__
#define __HTDISASM_H__

#include "asm.h"
#include "htobj.h"
#include "htformat.h"

#define DESC_DISASM "disasm/x86"

extern format_viewer_if htdisasm_if;

void dialog_assemble(ht_format_viewer *f, viewer_pos vaddr, CPU_ADDR cpuaddr, Assembler *a, Disassembler *disasm, char *default_str, UINT want_length);

/*
 *	CLASS ht_disasm_viewer
 */

class ht_disasm_viewer: public ht_uformat_viewer {
protected:
	Assembler *assem;
	Disassembler *disasm;
	int op1632;
public:
			void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, Assembler *a, Disassembler *d, int t);
	virtual 	void done();
/* overwritten */
	virtual	bool pos_to_offset(viewer_pos addr, FILEOFS *ofs);
	virtual	bool string_to_pos(char *string, viewer_pos *addr);
	virtual	void get_pindicator_str(char *buf);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	virtual	void handlemsg(htmsg *msg);
	virtual	bool offset_to_pos(FILEOFS ofs, viewer_pos *addr);
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
			void init(ht_streamfile *file, FILEOFS ofs, int size, Disassembler *disasm, bool own_disasm, int display_style);
	virtual 	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FILEOFS *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
};

#endif /* !__HTDISASM_H__ */
