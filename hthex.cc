/* 
 *	HT Editor
 *	hthex.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
 *	Copyright (C) 2007 Sebastian Biallas (sb@biallas.net)
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

#include "cmds.h"
#include "endianess.h"
#include "htctrl.h"
#include "htiobox.h"
#include "hthex.h"
#include "htmenu.h"
#include "htsearch.h"
#include "snprintf.h"
#include "stream.h"
#include "tools.h"

extern "C" {
#include "evalx.h"
#include "regex.h"
}

ht_view *hthex_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_hex_viewer *v = new ht_hex_viewer();
	v->init(b, DESC_HEX, VC_EDIT | VC_GOTO | VC_SEARCH | VC_REPLACE | VC_RESIZE, file, group);

	v->search_caps |= SEARCHMODE_BIN | SEARCHMODE_EVALSTR | SEARCHMODE_EXPR;

	v->h = new ht_hex_file_sub();
	v->h->init(file, 0, file->getSize(), 16, 0);

	v->insertsub(v->h);
	return v;
}

format_viewer_if hthex_if = {
	hthex_init,
	0
};

/*
 *	CLASS ht_hex_viewer
 */

int ht_hex_viewer::get_pindicator_str(char *buf, int max_len)
{
	FileOfs o;
	if (get_current_offset(&o)) {
		FileOfs sel_start, sel_end;
		pselect_get(&sel_start, &sel_end);
		char ttemp[1024];
		if (sel_end-sel_start > 0) {
			ht_snprintf(ttemp, sizeof ttemp, "selection %qxh-%qxh (%qd byte%s) ", sel_start, sel_end-1, sel_end-sel_start, sel_end-sel_start==1?"":"s");
		} else {
			ttemp[0] = 0;
		}
		return ht_snprintf(buf, max_len, " %s %qxh/%qu %s", edit() ? "edit" : "view", o, o, ttemp);
	} else {
		return ht_snprintf(buf, max_len, " ? ");
	}
}
	
bool ht_hex_viewer::get_vscrollbar_pos(int *pstart, int *psize)
{
	FileOfs s = file->getSize();
	if (s) {
		uint ll = h->get_line_length();
		FileOfs o = top.line_id.id2 + (uint64(top.line_id.id1) << 32);
		sint64 z = MIN(size.h * ll, s - o);
		return scrollbar_pos(o, z, s, pstart, psize);
	}
	return false;
}

void ht_hex_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_filesize_changed:
		htmsg m;
		m.msg = msg_filesize_changed;
		m.type = mt_broadcast;
		sendsubmsg(&m);

		uf_initialized = false;
		complete_init();

		dirtyview();
		return;
	case cmd_hex_entropy: {
		FileOfs ofs;
		if (get_current_offset(&ofs)) {
			byte buf[64];
			if (pread(ofs, buf, 64)==64) {
				int e = calc_entropy2(buf, 64);
				infobox("64-byte entropy at offset 0x%08qx: %d %%", ofs, e);
			}
		}
		clearmsg(msg);
		return;
	}
	case cmd_hex_display_bytes: {
		char result[256];
		sprintf(result, "%d", h->get_line_length());
		if (inputbox("Change display width", "~Line length (Bytes)", result, 256)) {
			int ll = strtoul(result, NULL, 10);
			if (ll > 0 && ll <= 32) {
				FileOfs ofs = -1ULL;
				get_current_offset(&ofs);
				int disp = h->get_disp();
				if (disp >= ll) {
					h->set_disp(0);
				}
				h->set_line_length(ll);
				viewer_pos p;
				if (ofs != -1ULL && offset_to_pos(ofs, &p) && goto_pos(p, this)) {
					focus_cursor();
				}
			} else {
				errorbox("Line length must be > 0 and <= 32!");
			}
		}
		clearmsg(msg);
		return;
	}
	case cmd_hex_display_disp: {
		char result[256];
		sprintf(result, "%d", h->get_disp());
		if (inputbox("Change display displacement", "~Displacement (Bytes)", result, 256)) {
			int ll = h->get_line_length();
			int disp = strtoul(result, NULL, 10);
			if (disp >= 0 && disp < ll) {
				FileOfs ofs = -1ULL;
				get_current_offset(&ofs);
				h->set_disp(disp);
				viewer_pos p;
				if (ofs != -1ULL && offset_to_pos(ofs, &p) && goto_pos(p, this)) {
					focus_cursor();
				}
			} else {
				errorbox("Displacement must be >= 0 and < line length (%d)!", ll);
			}
		}
		clearmsg(msg);
		return;
	}
	case msg_contextmenuquery: {
		ht_static_context_menu *m = new ht_static_context_menu();
		m->init("~Local-Hex");
		m->insert_entry("~Block operations", "Ctrl+B", cmd_file_blockop, K_Control_B, 1);
		m->insert_entry("~Replace", "Ctrl+E", cmd_file_replace, K_Control_E, 1);
		m->insert_entry("~Entropy", "Ctrl+T", cmd_hex_entropy, K_Control_T, 1);
		m->insert_entry("Change display ~width...", "Ctrl+O", cmd_hex_display_bytes, K_Control_O, 1);
		m->insert_entry("Change display ~displacement...", "Ctrl+U", cmd_hex_display_disp, K_Control_U, 1);

		msg->msg = msg_retval;
		msg->data1.ptr = m;
		return;
	}
	}
	ht_uformat_viewer::handlemsg(msg);
}

bool ht_hex_viewer::pos_to_offset(viewer_pos p, FileOfs *ofs)
{
	*ofs = (uint64(p.u.line_id.id1) << 32) + p.u.line_id.id2 + p.u.tag_idx;
	return true;
}

bool ht_hex_viewer::offset_to_pos(FileOfs ofs, viewer_pos *p)
{
	uint ll = h->get_line_length();
	uint disp = h->get_disp();
	clear_viewer_pos(p);
	p->u.sub = first_sub;
	FileOfs id;
	if (ofs < disp) {
		id = 0;
		p->u.tag_idx = ofs;
	} else {
		id = ofs - ((ofs - disp) % ll);
		p->u.tag_idx = (ofs - disp) % ll;
	}
	p->u.line_id.id1 = id >> 32;
	p->u.line_id.id2 = id;
	return true;
}

bool ht_hex_viewer::qword_to_pos(uint64 q, viewer_pos *p)
{
	uint ll = h->get_line_length();
	uint disp = h->get_disp();
	ht_linear_sub *s = (ht_linear_sub*)cursor.sub;
	FileOfs ofs = q;
	clear_viewer_pos(p);
	p->u.sub = s;
	if (ofs < disp) {
		p->u.tag_idx = ofs;
	} else {
		p->u.tag_idx = (ofs - disp) % ll;
	}
	return s->convert_ofs_to_id(ofs, &p->u.line_id);
}

static bool readao(eval_scalar *result, ht_hex_viewer *h, int size, Endianess e, bool sign, FileOfs ofs)
{
	byte buf[8];
	File *file = h->get_file();
	try {
		file->seek(ofs);
		file->readx(&buf, size);
	} catch (const IOException&) {
		set_eval_error("i/o error (couldn't read %d bytes from ofs %qd (0x%qx))", size, ofs, ofs);
		return false;
	}
	uint64 v = createHostInt64(buf, size, e);
	if (sign) {
		switch (size) {
		case 1: v = sint64(sint8(v));
		case 2: v = sint64(sint16(v));
		case 4: v = sint64(sint32(v));
		}
	}
	scalar_create_int_q(result, v);	
	return true;
}

static bool reada(eval_scalar *result, ht_hex_viewer *h, int size, Endianess e, bool sign)
{
	FileOfs ofs;
	if (!h->get_current_offset(&ofs)) return false;
	return readao(result, h, size, e, sign, ofs);
}

bool ht_hex_viewer::symbol_handler(eval_scalar *result, char *name)
{
	if (strcmp(name, "$") == 0 || strcmp(name, "o") == 0) {
		FileOfs ofs;
		viewer_pos vp;
		vp.u = cursor;
		if (!pos_to_offset(vp, &ofs)) return false;
		scalar_create_int_q(result, ofs);
		return true;
	}
	if (strcmp(name, "first") == 0) {scalar_create_int_q(result, 0); return true;}
	if (strcmp(name, "last") == 0) {scalar_create_int_q(result, file->getSize() - 1); return true;}
	if (strcmp(name, "u8") == 0) return reada(result, this, 1, little_endian, false);
	if (strcmp(name, "u16") == 0) return reada(result, this, 2, little_endian, false);
	if (strcmp(name, "u32") == 0) return reada(result, this, 4, little_endian, false);
	if (strcmp(name, "u64") == 0) return reada(result, this, 8, little_endian, false);
	if (strcmp(name, "s8") == 0) return reada(result, this, 1, little_endian, true);
	if (strcmp(name, "s16") == 0) return reada(result, this, 2, little_endian, true);
	if (strcmp(name, "s32") == 0) return reada(result, this, 4, little_endian, true);
	if (strcmp(name, "u16be") == 0) return reada(result, this, 2, big_endian, false);
	if (strcmp(name, "u32be") == 0) return reada(result, this, 4, big_endian, false);
	if (strcmp(name, "u64be") == 0) return reada(result, this, 8, big_endian, false);
	if (strcmp(name, "s16be") == 0) return reada(result, this, 2, big_endian, true);
	if (strcmp(name, "s32be") == 0) return reada(result, this, 4, big_endian, true);
	return ht_uformat_viewer::symbol_handler(result, name);
}

static int func_readint(eval_scalar *result, eval_int *offset, int size, Endianess e)
{
	ht_hex_viewer *v = (ht_hex_viewer*)eval_get_context();
	return readao(result, v, size, e, false, offset->value);
}

static int func_readbyte(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 1, little_endian);
}

static int func_read16le(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 2, little_endian);
}

static int func_read32le(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 4, little_endian);
}

static int func_read64le(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 8, little_endian);
}

static int func_read16be(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 2, big_endian);
}

static int func_read32be(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 4, big_endian);
}

static int func_read64be(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 8, big_endian);
}


bool ht_hex_viewer::func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	eval_func myfuncs[] = {
		{"$", 0, {SCALAR_INT}, "current offset"},
		{"o", 0, {SCALAR_INT}, "current offset"},
		{"first", 0, {SCALAR_INT}, "first offset"},
		{"last", 0, {SCALAR_INT}, "last offset"},
		{"u8", 0, {SCALAR_INT}, "byte at cursor"},
		{"u16", 0, {SCALAR_INT}, "little endian 16 bit word at cursor"},
		{"u32", 0, {SCALAR_INT}, "little endian 32 bit word at cursor"},
		{"u64", 0, {SCALAR_INT}, "little endian 64 bit word at cursor"},
		{"s8", 0, {SCALAR_INT}, "signed byte at cursor"},
		{"s16", 0, {SCALAR_INT}, "signed little endian 16 bit word at cursor"},
		{"s32", 0, {SCALAR_INT}, "signed little endian 32 bit word at cursor"},
		{"u16be", 0, {SCALAR_INT}, "big endian 16 bit word at cursor"},
		{"u32be", 0, {SCALAR_INT}, "big endian 32 bit word at cursor"},
		{"u64be", 0, {SCALAR_INT}, "big endian 64 bit word at cursor"},
		{"s16be", 0, {SCALAR_INT}, "signed big endian 16 bit word at cursor"},
		{"s32be", 0, {SCALAR_INT}, "signed big endian 32 bit word at cursor"},
		{"readbyte", (void*)&func_readbyte, {SCALAR_INT}, "read byte from offset"},
		{"read16le", (void*)&func_read16le, {SCALAR_INT}, "read little endian 16 bit word from offset"},
		{"read32le", (void*)&func_read32le, {SCALAR_INT}, "read little endian 32 bit word from offset"},
		{"read64le", (void*)&func_read64le, {SCALAR_INT}, "read little endian 64 bit word from offset"},
		{"read16be", (void*)&func_read16be, {SCALAR_INT}, "read big endian 16 bit word from offset"},
		{"read32be", (void*)&func_read32be, {SCALAR_INT}, "read big endian 32 bit word from offset"},
		{"read64be", (void*)&func_read64be, {SCALAR_INT}, "read big endian 64 bit word from offset"},
//		{"readstring", (void*)&func_readstring, {SCALAR_INT, SCALAR_INT}, "read string (offset, length)"},
		{NULL},
	};
	return std_eval_func_handler(result, name, params, myfuncs);
}

/*
 *	CLASS ht_hex_file_sub
 */

void ht_hex_file_sub::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_filesize_changed) {
		fsize = file->getSize();
		return;
	}
	ht_hex_sub::handlemsg(msg);
}

