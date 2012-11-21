/*
 *	HT Editor
 *	textedit.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#include "evalx.h"
#include "cmds.h"
#include "htctrl.h"
#include "htdialog.h"
#include "htmenu.h"
#include "htobj.h"
#include "htpal.h"
#include "htclipboard.h"
#include "htiobox.h"
#include "textedit.h"
#include "tools.h"
#include "snprintf.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**/
#include "atom.h"
#include "hthist.h"
#include "htsearch.h"

static ht_search_request* create_request_hexascii(text_search_pos *start, text_search_pos *end, ht_view *f, uint search_class)
{
	ht_hexascii_search_form *form=(ht_hexascii_search_form*)f;
	ht_hexascii_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);
	
	ht_fxbin_search_request *request;
	
	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "hex/ascii");
	}
/*	if (test_str_to_ofs(&start->offset, d.start.text, d.start.textlen, format, "start-offset")
	&& test_str_to_ofs(&end->offset, d.end.text, d.end.textlen, format, "end-offset")) {*/
		request = new ht_fxbin_search_request(search_class,
			d.options.state & 1 ? SF_FXBIN_CASEINSENSITIVE: 0,
			d.str.textlen, d.str.text);
/*	} else {
		request = NULL;
	}*/
	return request;
}

typedef ht_search_request* (*create_request_func)(text_search_pos *ret_start, text_search_pos *ret_end, ht_view *form, uint search_class);

struct ht_text_search_method {
	const char *name;
	uint search_class;			// SC_*
	uint search_mode_mask;		// SEARCHMODE_*
	uint histid;
	create_form_func create_form;
	create_request_func create_request;
	create_desc_func create_desc;
};

static ht_text_search_method text_search_methods[] =
{
	{ "bin: hex/ascii", SC_PHYSICAL, SEARCHMODE_BIN, HISTATOM_SEARCH_BIN,
		create_form_hexascii, create_request_hexascii, create_desc_hexascii }
};

ht_search_request *text_search_dialog(ht_text_viewer *text_viewer, uint searchmodes, const text_viewer_pos *end_pos)
{
	ht_search_request *result = NULL;
	Bounds b;
	b.w = 50;
	b.h = 15;
	b.x = (screen->w - b.w)/2;
	b.y = (screen->h - b.h)/2;
	ht_search_dialog *dialog = new ht_search_dialog();
	dialog->init(&b, "search");

	Bounds k;
	dialog->search_mode_xgroup->getbounds(&k);

	k.x = 0;
	k.y = 0;

	int modes = 0;
	int i = 0;
	ht_text_search_method *q = text_search_methods;
	while (q->name) {
		if (q->search_mode_mask & searchmodes) {
			Bounds v = k;
			ht_view *form = q->create_form(&v, q->histid);
			dialog->insert_search_mode(i, q->name, form);
			modes++;
		}
		q++;
		i++;
	}
	
//	dialog->select_search_mode(lastsearchmodeid);
	
	if (dialog->run(false)) {
		int modeid = dialog->get_search_modeid();
//		lastsearchmodeid = modeid;

		ht_text_search_method *s = &text_search_methods[modeid];
		ht_view *form = dialog->get_search_modeform();

		text_search_pos start, end;

		try {
			/* create history entry */
			if (s->create_desc) {
				char hist_desc[1024];
				s->create_desc(hist_desc, sizeof hist_desc, form);
				insert_history_entry((List*)getAtomValue(s->histid), hist_desc, form);
			}
			/* search */
			switch (s->search_class) {
				case SC_PHYSICAL: {
					text_viewer_pos cursor;
					text_viewer->get_cursor_pos(&cursor);
					start.offset = 0;
					text_viewer->pos_to_offset(&cursor, &start.offset);
					end.offset = 0xffffffff;
					break;
				}
			}
			result = s->create_request(&start, &end, form, s->search_class);
		} catch (const Exception &e) {
			errorbox("error: %y", &e);
		}
	}
	dialog->done();
	delete dialog;
	return result;
}

/*
 *	CLASS ht_undo_data
 */
bool ht_undo_data::combine(ht_undo_data *ud)
{
	return false;
}

/*
 *	CLASS ht_undo_data_delete_string
 */

ht_undo_data_delete_string::ht_undo_data_delete_string(text_viewer_pos *APos, text_viewer_pos *BPos, void *String, uint Len)
{
	apos = *APos;
	bpos = *BPos;
	if (Len) {
		string = malloc(Len);
		memcpy(string, String, Len);
	} else {
		string = NULL;
	}
	len = Len;
}

ht_undo_data_delete_string::~ht_undo_data_delete_string()
{
	free(string);
}

bool ht_undo_data_delete_string::combine(ht_undo_data *ud)
{
	if (ud->getObjectID() == getObjectID()) {
		ht_undo_data_delete_string *ud2 = (ht_undo_data_delete_string *)ud;
		if (ud2->apos.line == apos.line) {
			if (ud2->bpos.pofs + ud2->len == bpos.pofs) {
				string = realloc(string, len+ud2->len);
				memmove((byte*)string+ud2->len, string, len);
				memcpy(string, ud2->string, ud2->len);
				len += ud2->len;
				bpos = ud2->bpos;
				return true;
			}
		}
	}
	return false;
}

uint ht_undo_data_delete_string::getsize()
{
	return len + sizeof *this;
}

void ht_undo_data_delete_string::gettext(char *text, uint maxlen)
{
	char *buf = ht_malloc(len+1);
	bin2str(buf, string, len);
	ht_snprintf(text, maxlen, "deletion of '%s' at %d:%d", buf, bpos.line+1, bpos.pofs+1);
	free(buf);
}

ObjectID ht_undo_data_delete_string::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_DELETE;
}

void ht_undo_data_delete_string::apply(ht_text_editor *te)
{
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
	te->delete_chars(bpos.line, bpos.pofs, len);
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
}

void ht_undo_data_delete_string::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(apos.line);
		te->cursor_pput(apos.pofs);
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;
	}
	*goto_only = false;
	if (string) {
		te->insert_chars(bpos.line, bpos.pofs, string, len);
	}
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
}

/*
 *	CLASS ht_undo_data_delete_string2
 */

ht_undo_data_delete_string2::ht_undo_data_delete_string2(text_viewer_pos *APos, text_viewer_pos *BPos, void *String, uint Len)
{
	apos = *APos;
	bpos = *BPos;
	if (Len) {
		string = malloc(Len);
		memcpy(string, String, Len);
	} else {
		string = NULL;
	}
	len = Len;
}

ht_undo_data_delete_string2::~ht_undo_data_delete_string2()
{
	free(string);
}

bool ht_undo_data_delete_string2::combine(ht_undo_data *ud)
{
	if (ud->getObjectID() == getObjectID()) {
		ht_undo_data_delete_string2 *ud2 = (ht_undo_data_delete_string2 *)ud;
		if (ud2->apos.line == apos.line) {
			if (ud2->apos.pofs == apos.pofs) {
				string = realloc(string, len+ud2->len);
				memcpy((byte*)string+len, ud2->string, ud2->len);
				len += ud2->len;
				return true;
			}
		}
	}
	return false;
}

uint ht_undo_data_delete_string2::getsize()
{
	return len + sizeof *this;
}

void ht_undo_data_delete_string2::gettext(char *text, uint maxlen)
{
	char *buf = ht_malloc(len+1);
	bin2str(buf, string, len);
	ht_snprintf(text, maxlen, "deletion of '%s' at %d:%d", buf, apos.line+1, apos.pofs+1);
	free(buf);
}

ObjectID ht_undo_data_delete_string2::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_DELETE2;
}

void ht_undo_data_delete_string2::apply(ht_text_editor *te)
{
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
	te->delete_chars(apos.line, apos.pofs, len);
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
}

void ht_undo_data_delete_string2::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (apos.line != te->top_line + te->cursory || te->physical_cursorx()!=apos.pofs)) {
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		te->goto_line(apos.line);
		te->cursor_pput(apos.pofs);
		return;
	}
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
	*goto_only = false;
	if (string) {
		te->insert_chars(apos.line, apos.pofs, string, len);
	}
}

/*
 *	CLASS ht_undo_data_insert_string
 */

ht_undo_data_insert_string::ht_undo_data_insert_string(text_viewer_pos *APos, text_viewer_pos *BPos, void *String, uint Len)
{
	apos = *APos;
	bpos = *BPos;
	if (Len) {
		string = malloc(Len);
		memcpy(string, String, Len);
	} else {
		string = NULL;
	}
	len = Len;
}

ht_undo_data_insert_string::~ht_undo_data_insert_string()
{
	free(string);
}

bool ht_undo_data_insert_string::combine(ht_undo_data *ud)
{
	if (ud->getObjectID() == getObjectID()) {
		ht_undo_data_insert_string *ud2 = (ht_undo_data_insert_string *)ud;
		if (ud2->cpos.line == cpos.line) {
			if (ud2->apos.pofs == apos.pofs + len) {
				string = realloc(string, len + ud2->len);
				memcpy((byte*)string+len, ud2->string, ud2->len);
				len += ud2->len;
				bpos = ud2->bpos;
				return true;
			}
		}
	}
	return false;
}

uint ht_undo_data_insert_string::getsize()
{
	return len + sizeof *this;
}

void ht_undo_data_insert_string::gettext(char *text, uint maxlen)
{
	char *buf = ht_malloc(len+1);
	bin2str(buf, string, len);
	ht_snprintf(text, maxlen, "insertion of '%s' at %d:%d", buf, apos.line+1, apos.pofs+1);
	free(buf);
}

ObjectID ht_undo_data_insert_string::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_INSERT;
}

void ht_undo_data_insert_string::apply(ht_text_editor *te)
{
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
	if (string) {
		uint l = te->get_line_length(apos.line);
		cpos.line = apos.line;
		if (apos.pofs > l) {
			uint k = apos.pofs - l;               
			te->indent(apos.line, l, k);
			cpos.pofs = l;
		} else {
			cpos.pofs = apos.pofs;
		}
		te->insert_chars(apos.line, apos.pofs, string, len);
	}
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
}

void ht_undo_data_insert_string::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(apos.line);
		te->cursor_pput(apos.pofs);
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;
	}
	*goto_only = false;
	te->unindent(cpos.line, cpos.pofs, bpos.pofs-cpos.pofs);
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
}

/*
 *	CLASS ht_undo_data_overwrite_string
 */
ht_undo_data_overwrite_string::ht_undo_data_overwrite_string(text_viewer_pos *APos, text_viewer_pos *BPos, void *String, uint Len, void *String2, uint Len2)
{
	apos = *APos;
	bpos = *BPos;
	if (Len) {
		string = malloc(Len);
		memcpy(string, String, Len);
	} else {
		string = NULL;
	}
	len = Len;
	if (Len2) {
		string2 = malloc(Len2);
		memcpy(string2, String2, Len2);
	} else {
		string2 = NULL;
	}
	len2 = Len2;
}

ht_undo_data_overwrite_string::~ht_undo_data_overwrite_string()
{
	free(string);
	free(string2);
}

bool ht_undo_data_overwrite_string::combine(ht_undo_data *ud)
{
	if (ud->getObjectID()==getObjectID()) {
		ht_undo_data_overwrite_string *ud2 = (ht_undo_data_overwrite_string *)ud;
		if (ud2->cpos.line == cpos.line) {
			if (ud2->apos.pofs == apos.pofs + len) {
				string = realloc(string, len + ud2->len);
				memcpy((byte*)string+len, ud2->string, ud2->len);
				len += ud2->len;
				bpos = ud2->bpos;
				if (ud2->len2) {
					if (!len2) {
						string2 = malloc(ud2->len2);
						memcpy(string2, ud2->string2, ud2->len2);
					} else {
						string2 = realloc(string2, len2 + ud2->len2);
						memcpy((byte*)string2+len2, ud2->string2, ud2->len2);
					}
					len2 += ud2->len;
				}
				return true;
			}
		}
	}
	return false;
}

uint ht_undo_data_overwrite_string::getsize()
{
	return len + len2 + sizeof(*this);
}

void ht_undo_data_overwrite_string::gettext(char *text, uint maxlen)
{
	char *buf = ht_malloc(len+1);
	bin2str(buf, string, len);
	ht_snprintf(text, maxlen, "insertion of '%s' at %d:%d", buf, apos.line+1, apos.pofs+1);
	free(buf);
}

ObjectID ht_undo_data_overwrite_string::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_OVERWRITE;
}

void ht_undo_data_overwrite_string::apply(ht_text_editor *te)
{
	if (len2) te->delete_chars(apos.line, apos.pofs, len2);
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
	if (string) {
		uint l = te->get_line_length(apos.line);
		cpos.line = apos.line;
		if (apos.pofs > l) {
			uint k = apos.pofs - l;               
			te->indent(apos.line, l, k);
			cpos.pofs = l;
		} else {
			cpos.pofs = apos.pofs;
		}
		te->insert_chars(apos.line, apos.pofs, string, len);
	}
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
}

void ht_undo_data_overwrite_string::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(apos.line);
		te->cursor_pput(apos.pofs);
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;
	}
	*goto_only = false;
	te->unindent(cpos.line, cpos.pofs, bpos.pofs-cpos.pofs);
	if (len2) te->insert_chars(apos.line, apos.pofs, string2, len2);
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
}

/*
 *	CLASS ht_undo_data_split_line
 */
ht_undo_data_split_line::ht_undo_data_split_line(text_viewer_pos *APos, text_viewer_pos *BPos, uint Indent)
{
	apos = *APos;
	bpos = *BPos;
	indent = Indent;
}

uint ht_undo_data_split_line::getsize()
{
	return sizeof *this;
}

void ht_undo_data_split_line::gettext(char *text, uint maxlen)
{
	ht_snprintf(text, maxlen, "split line at %d:%d", apos.line+1, apos.pofs+1);
}

ObjectID ht_undo_data_split_line::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_SPLIT_LINE;
}

void ht_undo_data_split_line::apply(ht_text_editor *te)
{
	te->split_line(apos.line, apos.pofs);
	if (indent) te->indent(bpos.line, 0, indent);
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
}

void ht_undo_data_split_line::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(apos.line);
		te->cursor_pput(apos.pofs);
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;
	}
	*goto_only = false;
	if (indent) te->unindent(bpos.line, 0, indent);
	te->concat_lines(apos.line);
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
}

/*
 *	CLASS ht_undo_data_join_line
 */
ht_undo_data_join_line::ht_undo_data_join_line(text_viewer_pos *APos, text_viewer_pos *BPos)
{
	apos = *APos;
	bpos = *BPos;
}

uint ht_undo_data_join_line::getsize()
{
	return sizeof *this;
}

void ht_undo_data_join_line::gettext(char *text, uint maxlen)
{
	ht_snprintf(text, maxlen, "join lines %d and %d", bpos.line+1, bpos.line+2);
}

ObjectID ht_undo_data_join_line::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_JOIN_LINE;
}

void ht_undo_data_join_line::apply(ht_text_editor *te)
{
	uint l = te->get_line_length(apos.line);
	cpos.line = apos.line;
	if (apos.pofs > l) {
		uint k = apos.pofs - l;
		te->indent(apos.line, l, k);
		cpos.pofs = l;
	} else {
		cpos.pofs = apos.pofs;
	}
	    
	te->concat_lines(bpos.line);
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
}

void ht_undo_data_join_line::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;     
	}
	*goto_only = false;
	te->split_line(bpos.line, bpos.pofs);
	if (cpos.line == bpos.line) te->unindent(cpos.line, cpos.pofs, bpos.pofs-cpos.pofs);
	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
}

/*
 *	INSERT/DELETE BLOCK
 */

text_viewer_pos insert_text_block(ht_text_editor *te, text_viewer_pos apos, text_viewer_pos bpos, void *block, uint size)
{
	text_viewer_pos cpos;

	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);

	ht_textfile *textfile = te->get_textfile();
	FileOfs o;
	
	textfile->convert_line2ofs(apos.line, apos.pofs, &o);

	uint l = te->get_line_length(apos.line);
	cpos.line = apos.line;
	if (apos.pofs > l) {
		uint k = apos.pofs - l;
		te->indent(apos.line, l, k);
		cpos.pofs = l;
		o += k;
	} else {
		cpos.pofs = apos.pofs;
	}

	textfile->seek(o);
	textfile->write(block, size);

	uint s = size;
	text_viewer_pos start, end;
	textfile->convert_ofs2line(o, &start.line, &start.pofs);
	textfile->convert_ofs2line(o+s, &end.line, &end.pofs);
	te->select_set(&start, &end);
	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);

	return cpos;
}

void delete_text_block(ht_text_editor *te, text_viewer_pos apos, text_viewer_pos bpos, text_viewer_pos cpos, text_viewer_pos sel_start, text_viewer_pos sel_end, bool copy, void **block, uint *size)
{
	ht_textfile *textfile = te->get_textfile();

	FileOfs s, e;
	if (textfile->convert_line2ofs(sel_start.line, sel_start.pofs, &s) &&
	textfile->convert_line2ofs(sel_end.line, sel_end.pofs, &e) && copy) {
		uint sz = e-s;
		void *bl = malloc(sz);
		textfile->seek(s);
		textfile->read(bl, sz);
		*block = bl;
		*size = sz;
	}

	int k=0;
	if (sel_start.line < sel_end.line) {
		k=textfile->getlinelength(sel_start.line)-sel_start.pofs;
	} else {
		k=sel_end.pofs-sel_start.pofs;
	}
	te->delete_chars(sel_start.line, sel_start.pofs, k);
	if (sel_start.line < sel_end.line) {
		te->delete_chars(sel_end.line, 0, sel_end.pofs);
		if (sel_start.line+1 < sel_end.line) {
			te->delete_lines(sel_start.line+1, sel_end.line-sel_start.line-1);
		}
		te->concat_lines(sel_start.line);
	}
	te->unindent(cpos.line, cpos.pofs, apos.pofs-cpos.pofs);

	te->goto_line(apos.line);
	te->cursor_pput(apos.pofs);
	te->select_clear();

	te->goto_line(bpos.line);
	te->cursor_pput(bpos.pofs);
}

/*
 *	CLASS ht_undo_data_insert_block
 */

ht_undo_data_insert_block::ht_undo_data_insert_block(text_viewer_pos *Apos, text_viewer_pos *Bpos, void *Block, uint Size)
{
	apos = *Apos;
	bpos = *Bpos;
	sel_start.line = sel_end.line = 0;
	sel_start.pofs = sel_end.pofs = 0;
	size = Size;
	block = malloc(size);
	memcpy(block, Block, size);
}

ht_undo_data_insert_block::~ht_undo_data_insert_block()
{
	free(block);
}

uint ht_undo_data_insert_block::getsize()
{
	return (sizeof *this)+size;
}

void ht_undo_data_insert_block::gettext(char *text, uint maxlen)
{
	ht_snprintf(text, maxlen, "insert block ...");
}

ObjectID ht_undo_data_insert_block::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_INSERT_BLOCK;
}

void ht_undo_data_insert_block::apply(ht_text_editor *te)
{
	cpos = insert_text_block(te, apos, bpos, block, size);
	if (text_viewer_pos_compare(&sel_start, &sel_end) == 0) {
		te->get_selection(&sel_start, &sel_end);
	}
}

void ht_undo_data_insert_block::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(apos.line);
		te->cursor_pput(apos.pofs);
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;
	}
	*goto_only = false;
	delete_text_block(te, bpos, apos, cpos, sel_start, sel_end, false, NULL, NULL);
}

/*
 *	CLASS ht_undo_data_delete_block
 */

ht_undo_data_delete_block::ht_undo_data_delete_block(text_viewer_pos *Apos, text_viewer_pos *Bpos, text_viewer_pos *Sel_start, text_viewer_pos *Sel_end)
{
	apos = *Apos;
	bpos = *Bpos;
	sel_start = *Sel_start;
	sel_end = *Sel_end;
	size = 0;
	block = NULL;
}

ht_undo_data_delete_block::~ht_undo_data_delete_block()
{
	free(block);
}

uint ht_undo_data_delete_block::getsize()
{
	return (sizeof *this)+size;
}

void ht_undo_data_delete_block::gettext(char *text, uint maxlen)
{
	// FIXME
	ht_snprintf(text, maxlen, "delete block ...");
}

ObjectID ht_undo_data_delete_block::getObjectID() const
{
	return ATOM_HT_UNDO_DATA_DELETE_BLOCK;
}

void ht_undo_data_delete_block::apply(ht_text_editor *te)
{
	free(block);
	delete_text_block(te, apos, bpos, apos, sel_start, sel_end, true, &block, &size);
}

void ht_undo_data_delete_block::unapply(ht_text_editor *te, bool *goto_only)
{
	if (*goto_only && (bpos.line != te->top_line + te->cursory || te->physical_cursorx()!=bpos.pofs)) {
		te->goto_line(sel_end.line);
		te->cursor_pput(sel_end.pofs);
		te->goto_line(bpos.line);
		te->cursor_pput(bpos.pofs);
		return;
	}
	*goto_only = false;
	insert_text_block(te, bpos, sel_end, block, size);
}

/*
 *	CLASS ht_text_editor_undo
 */
ht_text_editor_undo::ht_text_editor_undo(uint max_undo_size)
	: Array(true)
{
	size = 0;
	max_size = max_undo_size;
	clean_state = 0;
	goto_state = true;
	current_position = 0;
}

int ht_text_editor_undo::get_current_position()
{
	   return current_position;
}

void ht_text_editor_undo::insert_undo(ht_text_editor *tv, ht_undo_data *undo)
{
	if (undo) {
		if (current_position != (int)count()) {
			// remove all pending redo's
			uint test = count();
			for (uint i = current_position; i < test; i++) {
				ht_undo_data *u = (ht_undo_data*)get(findByIdx(current_position));
				size -= u->getsize();
				del(findByIdx(current_position));
			}
			assert(current_position == (int)count());
			// clean_state eventually becomes unreachable
			if (clean_state > current_position) {
				clean_state = -1;
			}
		}
		undo->apply(tv);
		uint gsize = undo->getsize();
		while (size + gsize > max_size) {
			if (clean_state > -1) clean_state--;
			if (isEmpty()) {
				size -= ((ht_undo_data*)get(findFirst()))->getsize();
				del(findFirst());
				if (current_position) current_position--;
			} else {
				delete undo;
				return;
			}
		}
		if (!isEmpty() && !is_clean()) {
			ht_undo_data *u = (ht_undo_data*)get(findLast());
			uint zsize = u->getsize();
			if (u->combine(undo)) {
				size -= zsize;
				size += u->getsize();
				delete undo;
				return;
			}
		}
		current_position++;
		insert(undo);
		size+=gsize;
	}
	goto_state = true;
}

void ht_text_editor_undo::mark_clean()
{
	clean_state = current_position;
	goto_state = true;
}

bool ht_text_editor_undo::is_clean()
{
	return clean_state == current_position;
}

bool ht_text_editor_undo::is_clean(int i)
{
	return clean_state == i;
}

void ht_text_editor_undo::redo(ht_text_editor *te)
{
	goto_state = true;
	if (current_position < (int)count()) {
		ht_undo_data *u = (ht_undo_data*)get(findByIdx(current_position));
		u->apply(te);
		current_position++;
	}
}

void ht_text_editor_undo::undo(ht_text_editor *te, bool place_cursor_first)
{
	if (current_position) {
		ht_undo_data *u = (ht_undo_data*)get(findByIdx(current_position-1));
		bool goto_state_test = place_cursor_first ? goto_state : false;
		u->unapply(te, &goto_state_test);
		if (!goto_state_test) {
			current_position--;
		}
		goto_state = !goto_state_test;
	}
}

/*
 *
 */
int text_viewer_pos_compare(text_viewer_pos *a, text_viewer_pos *b)
{
	if (a->line==b->line) {
		return a->pofs-b->pofs;
	}
	return a->line-b->line;
}

/*
 *	CLASS ht_text_viewer
 */

void ht_text_viewer::init(Bounds *b, bool own_t, ht_textfile *t, Container *l)
{
	ht_view::init(b, VO_OWNBUFFER | VO_SELECTABLE | VO_RESIZE, "text viewer");
	VIEW_DEBUG_NAME("ht_text_viewer");

	growmode = MK_GM(GMH_FIT, GMV_FIT);

	own_textfile = false;
	textfile = NULL;
	set_textfile(t, own_t);

	own_lexer = false;
	lexer = NULL;
//	set_lexer(l, own_l);
	lexers = l;

	top_line = 0;
	xofs = 0;
	cursorx = 0;
	cursory = 0;
	select_clear();

	selectcursor = false;
	
	EOL_string = NULL;
	EOF_string = NULL;

	show_EOL = false;
	show_EOF = false;
	highlight_wrap = true;

	last_search_request = NULL;

	config_changed();
}

void ht_text_viewer::done()
{
	delete last_search_request;

	if (own_textfile) delete textfile;

	if (own_lexer && lexer) {
		lexer->done();
		delete lexer;
	}

	free(EOL_string);
	free(EOF_string);

	ht_view::done();
}

uint ht_text_viewer::char_vsize(char c, uint x)
{
	if (c=='\t') return tab_size - x % tab_size;
	return 1;
}

void ht_text_viewer::clipboard_copy_cmd()
{
	if (text_viewer_pos_compare(&sel_start, &sel_end)<0) {
		FileOfs s, e;
		if (textfile->convert_line2ofs(sel_start.line, sel_start.pofs, &s)
		   && textfile->convert_line2ofs(sel_end.line, sel_end.pofs, &e)) {
			char dsc[1024];
			String fn;
			ht_snprintf(dsc, sizeof dsc, "%y::%s", &textfile->getDesc(fn), desc);
			clipboard_copy(dsc, textfile, s, e-s);
		}
	}
}

void ht_text_viewer::config_changed()
{
	free(EOL_string);
	free(EOF_string);
	EOL_string = get_config_string("editor/EOL");
	EOF_string = get_config_string("editor/EOF");
	tab_size = get_config_dword("editor/tab size");
	tab_size = MIN(MAX(tab_size, 1), 16);

	if (lexer) lexer->config_changed();
	ht_view::config_changed();
}

bool ht_text_viewer::continue_search()
{
	if (last_search_request) {
		ht_search_result *r = NULL;
		FileOfs o, no;
		text_viewer_pos cursor;
		get_cursor_pos(&cursor);
		o = 0;
		pos_to_offset(&cursor, &o);
		no = o+1;
		try {
			if (last_search_request->search_class == SC_PHYSICAL) {
				text_search_pos start, end;
				start.offset = no;
				end.offset = last_search_end_ofs;
				r = search(last_search_request, &start, &end);
			}
		} catch (const Exception &e) {
			errorbox("error: %y", &e);
		}
		
		if (r) return show_search_result(r);
	}
	return false;
}

uint ht_text_viewer::cursor_up(uint n)
{
	if (cursory>n) cursory-=n; else {
		n=scroll_up(n-cursory);
		cursory=0;
	}
	return n;
}

uint ht_text_viewer::cursor_down(uint n)
{
	uint lmh=textfile->linecount()-top_line;
	if (lmh>(uint)size.h) lmh=size.h;
	if (cursory+n>lmh-1) {
		uint k = scroll_down(cursory+n-(lmh-1));
		lmh = textfile->linecount()-top_line;
		if (lmh>(uint)size.h) lmh = size.h;
		n = k+(lmh-1)-cursory;
		cursory = lmh-1;
	} else cursory+=n;
	return n;
}

uint ht_text_viewer::cursor_left(uint n)
{
	uint p;
	if (cursorx+xofs>n) p=cursorx+xofs-n; else p=0;
	cursor_vput(p);
	return 1;
}

uint ht_text_viewer::cursor_right(uint n)
{
	cursor_vput(cursorx+xofs+n);
	return 1;
}

void ht_text_viewer::cursor_home()
{
	cursor_vput(0);
}

void ht_text_viewer::cursor_end()
{
	cursor_vput(get_line_vlength(top_line+cursory));
}

void ht_text_viewer::cursor_pput(uint dx)
{
	uint vx = 0, px = 0;
	char line[1024];
	char *linep=line;

	uint linelen;
	if (!textfile->getline(top_line+cursory, 0, line, sizeof line, &linelen, NULL)) return;

	while (linelen--) {
		if (px==dx) break;
		int k = char_vsize(*(linep++), vx);
		vx += k;
		px++;
	}
	vx += dx-px;
	if (xofs > vx) {
		cursorx = 0;
		xofs = vx;
	} else {
		cursorx = vx-xofs;
		if (cursorx > (uint)size.w-1) {
			xofs += cursorx-(size.w-1);
			cursorx = size.w-1;
		}
	}
}

void ht_text_viewer::cursor_set(text_viewer_pos *pos)
{
	goto_line(pos->line);
	cursor_pput(pos->pofs);
}

void ht_text_viewer::cursor_vput(uint vx)
{
	if ((vx>=xofs) && (vx<xofs+size.w)) {
		cursorx=vx-xofs;
	} else if (vx<xofs) {
		xofs=vx;
		cursorx=0;
	} else if (vx>(uint)size.w-1) {
		xofs=vx-(size.w-1);
		cursorx=size.w-1;
	} else {
		xofs=0;
		cursorx=vx;
	}
}

void ht_text_viewer::draw()
{
//#define TIME_DRAW
#ifdef TIME_DRAW
	timer_handle h=new_timer();
	start_timer(h);
#endif /* TIME_DRAW */
	if (!textfile) return;

	vcp bgcolor = get_bgcolor();
	vcp metacolor = lexer ? lexer->getcolor_syntax(palidx_syntax_meta) :
			VCP(VCP_BACKGROUND(bgcolor), VCP_BACKGROUND(bgcolor));
	bool drawmeta = (lexer != NULL);
	clear(bgcolor);
	char line[1024];
	text_viewer_pos pos;
	pos.line = top_line;
	int y;
	for (y=0; y < size.h; y++) {
		lexer_state state;

		pos.pofs = 0;

//FIXME:debug:		if (!textfile->getline((top_line+y)|0x80000000, line, sizeof line, &state)) break;
		uint linelen;
		if (!textfile->getline(top_line+y, 0, line, sizeof line, &linelen, &state)) break;
		line[linelen] = 0;

		uint x=0;
		if (lexer) {
			char *linep=(char*)line;
			uint toklen;
			lexer_token tok;
			bool start_of_line=true;
			text_pos p;
			p.line=pos.line;
			p.pofs=pos.pofs;
			int prev_linelen = -1;
			while ((tok = lexer->gettoken(linep, linelen, p, start_of_line, &state, &toklen)) || (!*linep && (linelen>0))) {
				uint k, i;
				uint vtoklen=toklen;
				bool print=true;
				bool is_tab=((toklen==1) && (*linep=='\t'));
				vcp color=lexer->gettoken_color(tok);

				if (is_tab) vtoklen=tab_size-x%tab_size;

				if (x >= xofs) {
					k = x-xofs;
					i = 0;
					if (k > (uint)size.w-1) {
						break;
					}
				} else if (x+vtoklen >= xofs) {
					k=0;
					i=xofs-x;
				} else {
					print=false;
				}
				if (print) {
					if (is_tab) {
						char tab[17];
						uint z;
						for (z=0; z<vtoklen; z++) tab[z]=' ';
						tab[z]=0;
						render_str(k, y, color, &pos, vtoklen-i, tab+i, false);
					} else {
						render_str(k, y, color, &pos, vtoklen-i, linep+i, true);
					}
				}
				x += vtoklen;
				linep += toklen;
				linelen -= toklen;
				pos.pofs += toklen;
				start_of_line = false;
				p.line = pos.line;
				p.pofs = pos.pofs;
				if (!linelen && !prev_linelen) break;
				prev_linelen = linelen;
			}
			if (drawmeta) render_meta(x-xofs, y, &pos, metacolor);
		} else {
			char *linep=line;
			while (linelen--) {
				uint vtoklen=char_vsize(*linep, x);
				if (x>=xofs) {
					if (x-xofs>(uint)size.w-1) break;
					if (*linep=='\t') {
						vcp c=bgcolor;
						char tab[17];
						uint z;
						for (z=0; z<vtoklen; z++) tab[z]=' ';
						tab[z]=0;
						render_str_color(&c, &pos);
						buf->nprint(x-xofs, y, c, tab, z);
					} else {
						vcp c=bgcolor;
						render_str_color(&c, &pos);
						buf->printChar(x-xofs, y, c, *linep);
					}
				}
				x+=vtoklen;
				linep++;
				pos.pofs++;
			}
			if (drawmeta) render_meta(x-xofs, y, &pos, metacolor);
		}
		if (highlight_wrap && pos.line < sel_end.line
		 && pos.line >= sel_start.line) {
			int q = (drawmeta && show_EOL) ? strlen(EOL_string) : 0;
			int p = (x+q>xofs) ? x+q-xofs : 0;
			fill(p, y, size.w-p, 1, getcolor(palidx_generic_input_selected), ' ');
		}
		pos.line++;
	}
	if (focused) setcursor(cursorx, cursory, get_cursor_mode());

#ifdef TIME_DRAW
	stop_timer(h);
	int tix=get_timer_tick(h);
	delete_timer(h);
	buf_printf(40, 0, bgcolor, "%dtix", tix);
#endif /* TIME_DRAW */
}

const char *ht_text_viewer::func(uint i, bool execute)
{
	switch (i) {
		case 5: {
			if (execute) {
				sendmsg(cmd_text_viewer_goto);
			}
			return "goto";
		}
		case 7: {
			if (execute) {
				text_viewer_pos end_pos;
				uint search_caps = SEARCHMODE_BIN;
				ht_search_request *request = text_search_dialog(this, search_caps, &end_pos);
				ht_search_result *result = NULL;
				if (request) {
					text_search_pos start, end;
					text_viewer_pos cursor;
					get_cursor_pos(&cursor);
					pos_to_offset(&cursor, &start.offset);
					end.offset = 0xffffffff;
					result = search(request, &start, &end);
					if (result) {
						// FIXME: !!!!!!!!?
						if (!show_search_result(result)) infobox("couldn't display result (internal error)");
						delete result;
					} else infobox("not found");
				}
			}
			return "search";
		}
		default:
			return NULL;
	}
	return NULL;
}

vcp ht_text_viewer::get_bgcolor()
{
	return getcolor(palidx_generic_body);
}

void ht_text_viewer::get_cursor_pos(text_viewer_pos *cursor)
{
	cursor->pofs = physical_cursorx();
	cursor->line = top_line+cursory;
}

CursorMode ht_text_viewer::get_cursor_mode()
{
	return CURSOR_NORMAL;
}

ht_syntax_lexer *ht_text_viewer::get_lexer()
{
	return lexer;
}

/*
 * 0xffffffff --> ignore this line
 */
uint ht_text_viewer::get_line_indent(uint line)
{
	char s[1024];
	uint i, r, j;
	textfile->getline(line, 0, s, 1024, &i, NULL);     
	if (i==0) return 0xffffffff;
	j = r = 0;
	while (i && (s[j]==' ' || s[j]=='\t')) {
		r += char_vsize(s[j], r);
		j++;
		i--;
	}

	if (i==0) return 0xffffffff;
	
	return r;
}

uint ht_text_viewer::get_line_length(uint line)
{
	return textfile->getlinelength(line);
}

uint ht_text_viewer::get_line_vlength(uint line)
{
	char l[1024];
	char *linep=l;
	uint vl=0;

	uint linelen;
	if (!textfile->getline(line, 0, l, sizeof l, &linelen, NULL)) return 0;
	
	while (linelen--) vl+=char_vsize(*(linep++), vl);
	
	return vl;
}

int ht_text_viewer::get_pindicator_str(char *buf, int max_len)
{
	ht_syntax_lexer *l = get_lexer();
	const char *ln = l ? l->getname() : NULL;
	if (ln) {
		return ht_snprintf(buf, max_len, " %d:%d (%s) ", top_line+cursory+1, xofs+cursorx+1, ln);
	} else {
		return ht_snprintf(buf, max_len, " %d:%d ", top_line+cursory+1, xofs+cursorx+1);
	}
}

bool ht_text_viewer::get_vscrollbar_pos(int *pstart, int *psize)
{
	return (scrollbar_pos(top_line, size.h, textfile->linecount(), pstart, psize));
}

void ht_text_viewer::get_selection(text_viewer_pos *start, text_viewer_pos *end)
{
	*start = sel_start;
	*end = sel_end;
}

ht_textfile *ht_text_viewer::get_textfile()
{
	return textfile;
}

bool ht_text_viewer::get_hscrollbar_pos(int *pstart, int *psize)
{
	return false;
}

bool ht_text_viewer::goto_line(uint line)
{
	if (line >= textfile->linecount()) {
		return false;
	}
	if ((line >= top_line) && (line - top_line < (uint)size.h)) {
		cursory = line - top_line;
	} else {
		cursory = 0;
		if (line > top_line) {
			top_line = line;
			cursor_down(cursor_up(size.h));
		} else {
			top_line = line;
			cursor_up(cursor_down(size.h));
		}
	}
	return true;
}

void ht_text_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed: {
			int k=msg->data1.integer;
			bool sel;
			switch (k) {
				case K_Meta_S:
					selectcursor=!selectcursor;
					clearmsg(msg);
					return;
				case K_Control_Shift_Right:
				case K_Control_Right: {
					sel=(k==K_Control_Shift_Right) != selectcursor;
					char line[1024];
					uint linelen;
					uint i=0;
					uint px=physical_cursorx();
					bool phase = true;
					while (1) {
						if (!textfile->getline(top_line+cursory+i, 0, line, sizeof line, &linelen, NULL)) return;
						if (!linelen) {
							phase = false;
						} else while (px < linelen) {
							if (phase ^ ((line[px]>='0' && line[px]<='9') || (line[px]>='A' && line[px]<='Z') || (line[px]>='a' && line[px]<='z') || line[px]=='_')) {
								phase = !phase;
								if (phase) {
									if (sel) select_start();
									goto_line(top_line+cursory+i);
									cursor_pput(px);
									if (sel) select_end();
									dirtyview();
									clearmsg(msg);
									return;
								}
							}
							px++;
						}
						phase = false;
						px = 0;
						i++;
					}
				}
				case K_Control_Shift_Left:
				case K_Control_Left: {
					sel=(k==K_Control_Shift_Left) != selectcursor;
					char line[1024];
					uint linelen;
					int i=top_line+cursory;
					uint px=physical_cursorx();
					bool phase = true;
					while (i >= 0) {
						if (!textfile->getline(i, 0, line, sizeof line, &linelen, NULL)) return;
						if (px > linelen) px = linelen;
						while (px > 0) {
							if (phase ^ !((line[px-1]>='0' && line[px-1]<='9') || (line[px-1]>='A' && line[px-1]<='Z') || (line[px-1]>='a' && line[px-1]<='z') || line[px-1]=='_')) {
								phase = !phase;
								if (phase) goto bloed3;
							}
							px--;
						}
						if (!px && !phase) goto bloed3;
						px = (uint)-1;
						i--;
					}
					return;
					bloed3:
					if (sel) select_start();
					goto_line(i);
					cursor_pput(px);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				}
				case K_Control_J:
					sendmsg(cmd_text_viewer_goto);
					clearmsg(msg);
					return;
				case K_Up:
				case K_Shift_Up:
					sel=(k==K_Shift_Up) != selectcursor;
					if (sel) select_start();
					cursor_up(1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Down:
				case K_Shift_Down:
					sel=(k==K_Shift_Down) != selectcursor;
					if (sel) select_start();
					cursor_down(1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Left:
				case K_Shift_Left:
					sel=(k==K_Shift_Left) != selectcursor;
					if (sel) select_start();
					cursor_left(1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Right:
				case K_Shift_Right:
					sel=(k==K_Shift_Right) != selectcursor;
					if (sel) select_start();
					cursor_right(1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_PageUp:
				case K_Shift_PageUp:
					sel=(k==K_Shift_PageUp) != selectcursor;
					if (sel) select_start();
					scroll_up(size.h-1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_PageDown:
				case K_Shift_PageDown:
					sel=(k==K_Shift_PageDown) != selectcursor;
					if (sel) select_start();
					scroll_down(size.h-1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Home:
				case K_Shift_Home:
					sel=(k==K_Shift_Home) != selectcursor;
					if (sel) select_start();
					cursor_home();
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_End:
				case K_Shift_End:
					sel=(k==K_Shift_End) != selectcursor;
					if (sel) select_start();
					cursor_end();
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Control_PageUp:
					sel=selectcursor;
					if (sel) select_start();
					top_line=0;
					cursorx=0;
					cursory=0;
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Control_PageDown:
					sel=selectcursor;
					if (sel) select_start();
					top_line=textfile->linecount()-1;
					cursory=0;
					cursor_pput(0);
					scroll_down(size.h-1);
					scroll_up(size.h-1);
					cursor_down(size.h-1);
					if (sel) select_end();
					dirtyview();
					clearmsg(msg);
					return;
				case K_Meta_C:
				case K_Control_Insert:
					sendmsg(cmd_edit_copy);
					clearmsg(msg);
					return;
				case K_Control_L:
				case K_Shift_F7:
					if (!continue_search()) infobox("no further matches");
					dirtyview();
					clearmsg(msg);
					return;
			}
			break;
		}
		case cmd_edit_copy: {
			clipboard_copy_cmd();
			clearmsg(msg);
			return;
		}
		case cmd_text_viewer_goto: {          
			char line[1024];
			line[0]=0;
			if (inputbox("goto", "line", line, 1024)) {
				eval_scalar r;
				if (eval(&r, line, NULL, NULL, NULL)) {
					eval_int i;
					scalar_context_int(&r, &i);
					if (!i.value || !goto_line(i.value - 1)) {
						errorbox("no such line: %qd!", i.value);
					}
				}
			}
			return;
		}
		case cmd_text_viewer_change_highlight: {
			popup_change_highlight();
			dirtyview();
			clearmsg(msg);
			return;
		}
		case msg_get_pindicator: {
			msg->data1.integer = get_pindicator_str((char*)msg->data2.ptr, msg->data1.integer);
			clearmsg(msg);
			return;
		}
		case msg_get_scrollinfo: {
			switch (msg->data1.integer) {
				case gsi_hscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_hscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					clearmsg(msg);
					return;
				}
				case gsi_vscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_vscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					clearmsg(msg);
					return;
				}
			}
			break;
		}
		case msg_funcexec:
			if (func(msg->data1.integer, 1)) {
				clearmsg(msg);
				return;
			}
			break;
		case msg_funcquery: {
			const char *s=func(msg->data1.integer, 0);
			if (s) {
				msg->msg=msg_retval;
				msg->data1.cstr=s;
			}
			break;
		}
	}
	ht_view::handlemsg(msg);
}

void ht_text_viewer::make_pos_physical(text_viewer_pos *p)
{
	uint l=textfile->getlinelength(p->line);
	if (p->pofs > l) p->pofs=l;
}

void ht_text_viewer::normalize_selection()
{
	if (text_viewer_pos_compare(&sel_end, &sel_start) <= 0) {
		sel_start.line = 0;
		sel_start.pofs = 0;
		sel_end.line = 0;
		sel_end.pofs = 0;
	}
}

uint ht_text_viewer::physical_cursorx()
{
	int vx=0, px=0, v=cursorx+xofs;
	char line[1024];
	char *linep=line;

	uint linelen;
	if (!textfile->getline(top_line+cursory, 0, line, sizeof line, &linelen, NULL)) return 0;
	
	while (linelen--) {
		int k=char_vsize(*(linep++), vx);
		vx+=k;
		v-=k;
		if (v<0) break;
		px++;
	}
	if (v>0) px+=v;
	
	return px;
}

void ht_text_viewer::popup_change_highlight()
{
	Bounds b, c;
	
	app->getbounds(&b);

	b.x = (b.w - 40) / 2,
	b.y = (b.h - 8) / 2;
	b.w = 40;
	b.h = 8;
	
	ht_dialog *d = new ht_dialog();
	d->init(&b, "change highlighting mode", FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
	
	b.x = 0;
	b.y = 0;
		
	/* mode (input) */
	c = b;
	c.x = 0;
	c.y = 1;
	c.w = b.w-2-c.x;
	c.h = b.h-2-c.y;

	ht_itext_listbox *mode_input = new ht_itext_listbox();
	mode_input->init(&c);
	
	mode_input->insert_str(-1, "no highlighting");
	uint lc = lexers->count();
	int selected = -1;
	for (uint i=0; i < lc; i++) {
		ht_syntax_lexer *l = (ht_syntax_lexer*)(*lexers)[i];
		mode_input->insert_str(i, l->getname());
		if (lexer && (strcmp(lexer->getname(), l->getname()) == 0)) {
			selected = i+1;
		}
	}
	mode_input->update();
	if (selected >= 0) mode_input->gotoItemByPosition(selected);
	d->insert(mode_input);
	
	/* mode (text) */
	c = b;
	c.x = 0;
	c.y = 0;
	c.w = 30;
	c.h = 1;

	ht_label *mode_text = new ht_label();
	mode_text->init(&c, "choose ~highlighting mode", mode_input);

	d->insert(mode_text);
	
	if (d->run(false)) {
		ht_listbox_data type;

		ViewDataBuf vdb(mode_input, &type, sizeof type);

		ht_syntax_lexer *l = (ht_syntax_lexer*)(*lexers)[
			mode_input->getID(type.data->cursor_ptr)];
		set_lexer(l, false);
	}
	
	d->done();
	delete d;
}

bool ht_text_viewer::pos_to_offset(text_viewer_pos *pos, FileOfs *ofs)
{
	return textfile->convert_line2ofs(pos->line, pos->pofs, ofs);
}

int ht_text_viewer::ppos_str(char *buf, uint bufsize, text_viewer_pos *ppos)
{
	return ht_snprintf(buf, bufsize, "some pos");
}

void ht_text_viewer::render_meta(uint x, uint y, text_viewer_pos *pos, vcp color)
{
	text_viewer_pos p = *pos;
	render_str_color(&color, &p);
	if (pos->line == textfile->linecount()-1) {
		if (show_EOF && EOF_string) buf->print(x, y, color, EOF_string);
	} else {
		if (show_EOL && EOL_string) buf->print(x, y, color, EOL_string);
	}
}

void ht_text_viewer::render_str(int x, int y, vcp color, text_viewer_pos *pos, uint len, char *str, bool multi)
{
	if ((pos->line == sel_start.line || pos->line == sel_end.line)
	 && text_viewer_pos_compare(&sel_start, &sel_end) != 0) {
		text_viewer_pos p=*pos;
		vcp c;
		while (len--) {
			c = color;
			render_str_color(&c, &p);
			buf_lprint0(x++, y, c, 1, str);
			str++;
			if (multi) p.pofs++;
		}
	} else {
		render_str_color(&color, pos);
		buf_lprint0(x, y, color, len, str);
	}
}

int ht_text_viewer::buf_lprint0(int x, int y, int c, int l, char *text)
{
	while (l--) {
		char s = *text;
		if (!s) s = ' ';
		buf->printChar(x++, y, c, s);
		text++;
	}
	return l;
}

void ht_text_viewer::render_str_color(vcp *color, text_viewer_pos *pos)
{
	if (text_viewer_pos_compare(pos, &sel_start) >= 0
	 && text_viewer_pos_compare(pos, &sel_end) < 0) {
		vcp selcolor = getcolor(palidx_generic_input_selected);
		*color = VCP(*color, selcolor);
	}
}

void ht_text_viewer::resize(int rw, int rh)
{
	ht_view::resize(rw, rh);
	if ((int)cursorx > size.w-1) {
		xofs += (int)cursorx-(size.w-1);
		cursorx = size.w-1;
	}
	if ((int)cursory > size.h-1) {
		top_line += (int)cursory-(size.h-1);
		cursory = size.h-1;
	}
}

uint ht_text_viewer::scroll_up(uint n)
{
	if (top_line > n) top_line -= n; else {
		int q = top_line;
		top_line = 0;
		cursory = 0;
		return q;
	}
	return n;
}

uint ht_text_viewer::scroll_down(uint n)
{
	uint lc=textfile->linecount();
	if (top_line+n+size.h <= lc) {
		top_line += n;
	} else {
		if (lc-top_line >= (uint)size.h) {
			int q = top_line;
			top_line = lc-size.h;
			cursory = size.h-1;
			return top_line-q;
		}
		cursory = lc-top_line-1;
		return 0;
	}
	return n;
}

uint ht_text_viewer::scroll_left(uint n)
{
	if (xofs > n) xofs -= n; else xofs = 0;
	return n;
}

uint ht_text_viewer::scroll_right(uint n)
{
	xofs += n;
	return n;
}

ht_search_result *ht_text_viewer::search(ht_search_request *request, text_search_pos *s, text_search_pos *e)
{
	if (request != last_search_request) {
		delete last_search_request;
		last_search_request = request->clone();
	}
	last_search_end_ofs = e->offset;

	switch (request->search_class) {
		case SC_PHYSICAL: {
			FileOfs start = s->offset, end = e->offset;
			return linear_bin_search(request, start, end, textfile, 0, 0xffffffff);
		}
	}
	return NULL;
}

void ht_text_viewer::select_add(text_viewer_pos *s, text_viewer_pos *e)
{
	text_viewer_pos start=*s, end=*e;
	make_pos_physical(&start);
	make_pos_physical(&end);
	bool downward = true;
	if (text_viewer_pos_compare(&start, &end)>0) {
		text_viewer_pos temp=start;
		downward = false;
		start=end;
		end=temp;
	}

	if ((text_viewer_pos_compare(&end, &sel_end)==0) && !downward) {
		sel_end=start;
	} else if ((text_viewer_pos_compare(&start, &sel_end)==0) && downward) {
		sel_end=end;
	} else if ((text_viewer_pos_compare(&end, &sel_start)==0) && !downward) {
		sel_start=start;
	} else if ((text_viewer_pos_compare(&start, &sel_start)==0) && downward){
		sel_start=end;
	} else {
		sel_start=start;
		sel_end=end;
	}
	if (text_viewer_pos_compare(&sel_start, &sel_end)>0) {
		text_viewer_pos temp=sel_start;
		sel_start=sel_end;
		sel_end=temp;
	}
	normalize_selection();
}

void ht_text_viewer::select_clear()
{
	sel_start.line = 0;
	sel_start.pofs = 0;
	sel_end.line = 0;
	sel_end.pofs = 0;
}

void ht_text_viewer::select_set(text_viewer_pos *s, text_viewer_pos *e)
{
	sel_start = *s;
	sel_end = *e;
	normalize_selection();
}

void ht_text_viewer::select_start()
{
	selectmode = true;
	selectstart.line = top_line+cursory;
	selectstart.pofs = physical_cursorx();
}

void ht_text_viewer::select_end()
{
	text_viewer_pos p;
	selectmode = false;
	p.line = top_line+cursory;
	p.pofs = physical_cursorx();
	select_add(&selectstart, &p);
}

void ht_text_viewer::set_lexer(ht_syntax_lexer *l, bool own_l)
{
	if (own_lexer) {
		lexer->done();
		delete lexer;
	}
	// FIXME: is this "the right thing"?
	if (l) l->config_changed();
	lexer = l;
	own_lexer = own_l;
	textfile->set_lexer(l);
}

void ht_text_viewer::set_textfile(ht_textfile *t, bool own_t)
{
	if (own_textfile) delete textfile;
	textfile = t;
	own_textfile = own_t;
}

bool ht_text_viewer::show_search_result(ht_search_result *result)
{
	switch (result->search_class) {
	case SC_PHYSICAL: {
		ht_physical_search_result *r = (ht_physical_search_result*)result;
		text_viewer_pos start, end;
		textfile->convert_ofs2line(r->offset, &start.line, &start.pofs);
		textfile->convert_ofs2line(r->offset+r->size, &end.line, &end.pofs);
		select_set(&start, &end);
		cursor_set(&start);
	}
	}
	return true;
}

/*
 *	CLASS ht_text_editor
 */

void ht_text_editor::init(Bounds *b, bool own_t, ht_textfile *t, Container *l, uint e)
{
	ht_text_viewer::init(b, own_t, t, l);
	edit_options = e;
	if (edit_options & TEXTEDITOPT_UNDO) {
		undo_list = new ht_text_editor_undo(1024*1024);
	} else {
		undo_list = NULL;
	}
	overwrite_mode = false;

	show_EOL = true;
	show_EOF = true;
}

void ht_text_editor::done()
{
	delete undo_list;
	ht_text_viewer::done();
}

void ht_text_editor::clipboard_cut_cmd()
{
	clipboard_copy_cmd();
	clipboard_delete_cmd();
}

void ht_text_editor::clipboard_delete_cmd()
{
	if (sel_start.line || sel_start.pofs || sel_end.line || sel_end.pofs) {
		text_viewer_pos apos, bpos;
		uint px = physical_cursorx();
		apos.line = top_line+cursory;
		apos.pofs = px;
		bpos = sel_start;
		textoperation_apply(new ht_undo_data_delete_block(&apos, &bpos, &sel_start, &sel_end));
	}
}

void ht_text_editor::clipboard_paste_cmd()
{
	uint bsize = clipboard_getsize();
	void *block = malloc(bsize);
	clipboard_paste(block, bsize);

	text_viewer_pos apos, bpos;
	uint px = physical_cursorx();
	apos.line = bpos.line = top_line+cursory;
	apos.pofs = bpos.pofs = px;
	textoperation_apply(new ht_undo_data_insert_block(&apos, &bpos, block, bsize));
	free(block);
}

bool ht_text_editor::concat_lines(uint a)
{
	uint b = a+1;
	if (textfile->has_line(a) && textfile->has_line(b)) {
		uint alen = textfile->getlinelength(a);
		char *aline = ht_malloc(alen+1);
		uint alinelen;
		textfile->getline(a, 0, aline, alen+1, &alinelen, NULL);
	
		text_viewer_pos ss, se;

		ss=sel_start;
		se=sel_end;

		insert_chars(b, 0, aline, alinelen);

		free(aline);
	
		delete_lines(a, 1);

		if (b > ss.line) {
			if (b == se.line) {
				se.pofs += alen;
				se.line--;
			} else if (b < se.line) {
				se.line--;
			}
		} else {
			if (b == ss.line) {
				ss.pofs += alen;
			}
			ss.line--;
			se.line--;
		}

		sel_start = ss;
		sel_end = se;
		normalize_selection();
		return true;
	}
	return false;
}

void ht_text_editor::config_changed()
{
	auto_indent = get_config_dword("editor/auto indent");
	ht_text_viewer::config_changed();
}

void ht_text_editor::delete_chars(uint line, uint ofs, uint count)
{
	text_viewer_pos pos;
	pos.line=line;
	pos.pofs=ofs;
	if ((sel_end.line==pos.line) &&
	(text_viewer_pos_compare(&pos, &sel_start)>=0) &&
	(text_viewer_pos_compare(&pos, &sel_end)<0)) {
		sel_end.pofs-=count;
	} else if ((sel_start.line==pos.line) &&
	(text_viewer_pos_compare(&pos, &sel_start)<0)) {
		sel_start.pofs-=count;
		if (sel_end.line==pos.line) sel_end.pofs-=count;
	}
	textfile->delete_chars(line, ofs, count);
}

void ht_text_editor::delete_lines(uint line, uint count)
{
	if (sel_start.line+1>line) sel_start.line--;
	if (sel_end.line>line) sel_end.line--;
	normalize_selection();
	textfile->delete_lines(line, count);
}

const char *ht_text_editor::func(uint i, bool execute)
{
	switch (i) {
	case 2:
		if (execute) {
			String fn;
			if (!textfile->getFilename(fn).isEmpty()) {
				sendmsg(cmd_file_save);
			} else {
				app->sendmsg(cmd_file_saveas);
				bool dirty = true;
				FileOfs start = 0;
				FileOfs end = 0x7fffffff;
				textfile->cntl(FCNTL_MODS_IS_DIRTY, start, end, &dirty);
				if (undo_list && !dirty) {
					undo_list->mark_clean();
				}
			}
		}
		return "save";

	}
	return ht_text_viewer::func(i, execute);
}

vcp ht_text_editor::get_bgcolor()
{
	return getcolor(focused ? palidx_generic_input_focused :
			palidx_generic_input_unfocused);
}

CursorMode ht_text_editor::get_cursor_mode()
{
	return overwrite_mode ? CURSOR_BOLD : CURSOR_NORMAL;
}

int ht_text_editor::get_pindicator_str(char *buf, int max_len)
{
	ht_syntax_lexer *l = get_lexer();
	const char *ln = l ? l->getname() : NULL;
	bool dirty = true;
	textfile->cntl(FCNTL_MODS_IS_DIRTY, 0ULL, 0x7fffffffULL, &dirty);
	if (ln) {
		return ht_snprintf(buf, max_len, " %c%d:%d (%s) ", dirty ? '*' : ' ', top_line+cursory+1, xofs+cursorx+1, ln);
	} else {
		return ht_snprintf(buf, max_len, " %c%d:%d ", dirty ? '*' : ' ', top_line+cursory+1, xofs+cursorx+1);
	}
}

void ht_text_editor::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed: {
			int k=msg->data1.integer;
			switch (k) {
				case K_Insert: {
					overwrite_mode = !overwrite_mode;
					dirtyview();
					clearmsg(msg);
					return;
				}
				case K_Return: {
					uint px=physical_cursorx();
					text_viewer_pos apos, bpos;
					apos.line = top_line+cursory;
					bpos.line = apos.line+1;
					apos.pofs = px;
					bpos.pofs = 0;
					if (auto_indent) {
						int i=0;
						bpos.pofs = 0xffffffff;
						while (bpos.pofs==0xffffffff && (apos.line-i) && (i<32)) {
							bpos.pofs = get_line_indent(apos.line-i);
							i++;
						}
						if (bpos.pofs == 0xffffffff) bpos.pofs = 0;
					}
					uint indent = bpos.pofs;
					if (px >= get_line_length(top_line+cursory)) indent = 0;
					textoperation_apply(new ht_undo_data_split_line(&apos, &bpos, indent));
					
					dirtyview();
					clearmsg(msg);
					return;
				}
				case K_Delete: {
					uint cx=physical_cursorx();
					if (cx < get_line_length(top_line+cursory)) {
						uint px=physical_cursorx();
						text_viewer_pos apos, bpos;
						char s[1024];
						apos.line = bpos.line = top_line+cursory;
						apos.pofs = px;
						bpos.pofs = px;
						uint i;
						textfile->getline(apos.line, 0, s, 1024, &i, NULL);
						textoperation_apply(new ht_undo_data_delete_string2(&apos, &bpos, &s[px], 1));
					} else if (textfile->has_line(top_line+cursory+1)) {
						uint px=physical_cursorx();
						text_viewer_pos apos, bpos;
						apos.line = top_line+cursory;
						apos.pofs = px;
						bpos.line = top_line+cursory;
						bpos.pofs = px;
						textoperation_apply(new ht_undo_data_join_line(&apos, &bpos));
					}
					dirtyview();
					clearmsg(msg);
					return;
				}
				case K_Backspace: {
					uint cx=physical_cursorx();
					if (cx) {
						if (cx <= textfile->getlinelength(top_line+cursory)) {
							uint px=physical_cursorx()-1;
							text_viewer_pos apos, bpos;
							char s[1024];
							apos.line = bpos.line = top_line+cursory;
							apos.pofs = cx;
							bpos.pofs = px;
							uint i;
							textfile->getline(apos.line, 0, s, 1024, &i, NULL);
							textoperation_apply(new ht_undo_data_delete_string(&apos, &bpos, &s[px], 1));
						} else {
							// place cursor only
							cursor_pput(cx-1);
						}               
					} else {
						if (top_line+cursory) {
							uint px=physical_cursorx();
							text_viewer_pos apos, bpos;
							apos.line = top_line+cursory;
							apos.pofs = px;
							bpos.line = apos.line-1;
							bpos.pofs = get_line_length(bpos.line);
							textoperation_apply(new ht_undo_data_join_line(&apos, &bpos));
						}
					}
					dirtyview();
					clearmsg(msg);
					return;
				}
				case K_Meta_U:
				case K_Meta_Backspace: {
					sendmsg(cmd_text_editor_undo);
					clearmsg(msg);
					return;
				}
				case K_Meta_R: {
					sendmsg(cmd_text_editor_redo);
					clearmsg(msg);
					return;
				}
				case K_Meta_V:
				case K_Shift_Insert:
					sendmsg(cmd_edit_paste);
					clearmsg(msg);
					return;
				case K_Meta_X:
				case K_Shift_Delete:
					sendmsg(cmd_edit_cut);
					clearmsg(msg);
					return;
				case K_Meta_D:
				case K_Control_Delete:
					sendmsg(cmd_edit_delete);
					clearmsg(msg);
					return;
				case K_Control_Y:
					sendmsg(cmd_text_editor_delete_line);
					clearmsg(msg);
					return;
				default: {
					int k=msg->data1.integer;
					if (((k>=' ') && (k<=255)) ||
					((k=='\t') && (edit_options & TEXTEDITOPT_INPUTTABS))) {
						char s=k;
						text_viewer_pos apos, bpos;
						uint px=physical_cursorx();
						apos.line = bpos.line = top_line+cursory;
						apos.pofs = px;
						bpos.pofs = px+1;
						if (overwrite_mode) {
							char old[1024];
							uint i, j=0;
							textfile->getline(apos.line, 0, old, 1024, &i, NULL);
							if (i>px) j = 1;
							textoperation_apply(new ht_undo_data_overwrite_string(&apos, &bpos, &s, 1, &old[px], j));
						} else {
							textoperation_apply(new ht_undo_data_insert_string(&apos, &bpos, &s, 1));
						}
						dirtyview();
						clearmsg(msg);
						return;
					}							
				}
			}
			break;
		}
		case msg_contextmenuquery: {
			ht_static_context_menu *m=new ht_static_context_menu();
			m->init("~Texteditor");
			if (undo_list) {
				char buf[30], buf2[20];
				// FIXME: implementme correctly/better
/*				if (undo_list->current_position) {
					((ht_undo_data*)undo_list->get(undo_list->current_position-1))->gettext(buf2, sizeof buf2);
				} else {
				}*/
				buf2[0] = 0;
				ht_snprintf(buf, sizeof buf, "~Undo %s", buf2);
			
				m->insert_entry(buf, "Alt+U", cmd_text_editor_undo, 0, 1);
				m->insert_entry("~Redo", "Alt+R", cmd_text_editor_redo, 0, 1);
				m->insert_entry("~Protocol", "Alt+P", cmd_text_editor_protocol, K_Meta_P, 1);
				m->insert_separator();
			}
			m->insert_entry("Change ~highlight", "", cmd_text_viewer_change_highlight, 0, 1);
			m->insert_separator();
			// FIXME: somewhat hacked
			m->insert_entry("~Delete line", "Control+Y", cmd_text_editor_delete_line, 0, 1);
			m->insert_separator();
			m->insert_entry("~Go to line", "", cmd_text_viewer_goto, 0, 1);
			msg->msg = msg_retval;
			msg->data1.ptr = m;
			return;
		}
		case cmd_edit_paste: {
			clipboard_paste_cmd();
			dirtyview();
			clearmsg(msg);
			return;
		}
		case cmd_edit_cut: {
			clipboard_cut_cmd();
			dirtyview();
			clearmsg(msg);
			return;
		}
		case cmd_edit_delete: {
			clipboard_delete_cmd();
			dirtyview();
			clearmsg(msg);
			return;
		}
		case cmd_file_save: {
			if (save()) clearmsg(msg);
			return;
		}
		case cmd_text_editor_undo: {
			undo(true);
			dirtyview();
			clearmsg(msg);
			return;
		}          
		case cmd_text_editor_redo: {
			redo();
			dirtyview();
			clearmsg(msg);
			return;
		}          
		case cmd_text_editor_protocol: {
			show_protocol();
			dirtyview();
			clearmsg(msg);
			return;
		}
		case cmd_text_editor_delete_line: {
			if (top_line+cursory <= textfile->linecount()-1) {
				text_viewer_pos apos, bpos, a, b;
				apos.line = top_line+cursory;
				apos.pofs = physical_cursorx();
				bpos.line = top_line+cursory;
				bpos.pofs = 0;
				a.line = top_line+cursory;
				a.pofs = 0;
				b = a;
				// if last line
				if (b.line+1 > textfile->linecount()-1) {
					b.pofs = textfile->getlinelength(b.line);
				} else {
					b.line++;
				}
				if ((a.line != b.line) || (a.pofs != b.pofs))
					textoperation_apply(new ht_undo_data_delete_block(&apos, &bpos, &a, &b));
			}
			dirtyview();
			clearmsg(msg);
			return;
		}
	}
	ht_text_viewer::handlemsg(msg);
}

void ht_text_editor::indent(uint line, uint start, uint size)
{
	char *w = ht_malloc(size);
	memset(w, ' ', size);
	textfile->insert_chars(line, start, w, size);
	free(w);
}

void ht_text_editor::unindent(uint line, uint start, uint size)
{
	textfile->delete_chars(line, start, size);
}

void ht_text_editor::insert_chars(uint line, uint ofs, void *chars, uint len)
{
	text_viewer_pos pos;
	pos.line=line;
	pos.pofs=ofs;
	if ((sel_end.line==pos.line) &&
	(text_viewer_pos_compare(&pos, &sel_start)>=0) &&
	(text_viewer_pos_compare(&pos, &sel_end)<0)) {
		sel_end.pofs+=len;
	} else if ((sel_start.line==pos.line) &&
	(text_viewer_pos_compare(&pos, &sel_start)<0)) {
		sel_start.pofs+=len;
		if (sel_end.line==pos.line) sel_end.pofs+=len;
	}
	textfile->insert_chars(line, ofs, chars, len);
}

void ht_text_editor::insert_lines(uint line, uint count)
{
	if (sel_start.line+1>line) sel_start.line++;
	if (sel_end.line>line) sel_end.line++;
	normalize_selection();
	textfile->insert_lines(line, count);
}

bool ht_text_editor::save()
{
//	asm(".byte 0xcc");
	String oldname;
	if (textfile->getFilename(oldname).isEmpty()) return false;
	dirtyview();

	TempFile temp(IOAM_READ | IOAM_WRITE);

	ht_ltextfile *old = dynamic_cast<ht_ltextfile *>(textfile->getLayered());
	String blub;
	old->getDesc(blub);

	old->seek(0);
	old->copyAllTo(&temp);

	pstat_t st1, st2;
	old->pstat(st1);
	temp.pstat(st2);
	if (st1.size != st2.size) {
		errorbox("couldn't write backup file - file not saved. (o=%qd, t=%qd)", st1.size, st2.size);
		return false;
	}


	old->setAccessMode(IOAM_WRITE);
	
	File *f = old->getLayered();
	f->truncate(0);
	temp.seek(0);
	temp.copyAllTo(f);

	old->setAccessMode(IOAM_READ);
	old->reread();

//	textfile->set_layered_assume(old, true, true);

	if (undo_list) {
		undo_list->mark_clean();
	}
	return true;
}

void ht_text_editor::show_protocol()
{
	Bounds c, b;
	app->getbounds(&c);
	b.w=c.w*5/6;
	uint bh=b.h=c.h*5/6;
	b.x=(c.w-b.w)/2;
	b.y=(c.h-b.h)/2;
	ht_dialog *dialog;
	NEW_OBJECT(dialog, ht_dialog, &b, "protocol", FS_KILLER | FS_TITLE | FS_MOVE);
	b.assign(1, 0, b.w-4, 1);
	ht_statictext *text;
	NEW_OBJECT(text, ht_statictext, &b, " ", align_left);
	dialog->insert(text);
	b.y = 1;
	b.h = bh-5;
	ht_text_listbox *list;
	NEW_OBJECT(list, ht_text_listbox, &b, 2, 1);
	uint cp = undo_list->get_current_position();
	const char *od;
	if (undo_list->is_clean(0)) {
		od = "disk";
	} else {
		od = "    ";
	}
	list->insert_str(0, od, "--- initial state ---");
	for (uint i=0; i<undo_list->count(); i++) {
		char buf[1024];
		((ht_undo_data*)(*undo_list)[i])->gettext(buf, 1024);
		if (undo_list->is_clean(i+1)) {
			od = "disk";
		} else {
			od = "    ";
		}
		list->insert_str(i+1, od, buf);
	}
	list->update();
	list->gotoItemByPosition(cp);
	dialog->insert(list);
	if (dialog->run(false) == button_ok) {
		ht_listbox_data d;
		ViewDataBuf vdb(list, &d, sizeof d);
		int a = list->getID(d.data->cursor_ptr);
		int b = cp;

		if (a-b < 0) {
			for (int i=0; i < b-a; i++) {
				undo(false);
			}
		} else if (a-b > 0) {
			for (int i=0; i < a-b; i++) {
				redo();
			}
		}
	}
	dialog->done();
	delete dialog;
}

void ht_text_editor::split_line(uint a, uint pos)
{
	uint l=textfile->getlinelength(a);
	if (pos>l) pos=l;
	char *aline = ht_malloc(pos+1);
	uint alinelen;
	textfile->getline(a, 0, aline, pos+1, &alinelen, NULL);

	text_viewer_pos p, ss, se;
	p.line=a;
	p.pofs=pos;

	ss=sel_start;
	se=sel_end;

	insert_lines(a, 1);
	insert_chars(a, 0, aline, alinelen);

	delete_chars(a+1, 0, pos);
	free(aline);
	
	if (text_viewer_pos_compare(&p, &ss)>0) {
		if (text_viewer_pos_compare(&p, &se)<0) {
			if (se.line==p.line) {
				se.pofs-=pos;
			}
			se.line++;
		}
	} else {
		if (ss.line==p.line) {
			ss.pofs-=pos;
		}
		ss.line++;
		se.line++;
	}
	sel_start=ss;
	sel_end=se;
	normalize_selection();
}

void ht_text_editor::textoperation_apply(ht_undo_data *ud)
{
	if (undo_list) {
		undo_list->insert_undo(this, ud);
	} else {
		ud->apply(this);
		delete ud;
	}
}

void ht_text_editor::redo()
{
	if (undo_list) {
		undo_list->redo(this);
		if (undo_list->is_clean()) textfile->cntl(FCNTL_MODS_CLEAR_DIRTY_RANGE, 0ULL, 0ULL);
	}
}

void ht_text_editor::undo(bool place_cursor_first)
{
	if (undo_list) {
		undo_list->undo(this, place_cursor_first);
		if (undo_list->is_clean()) textfile->cntl(FCNTL_MODS_CLEAR_DIRTY_RANGE, 0ULL, 0ULL);
	}
}

