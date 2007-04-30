/* 
 *	HT Editor
 *	textedit.h
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

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "htobj.h"
#include "textfile.h"

#include "htformat.h"

#define TEXTEDITOPT_NULL			0
#define TEXTEDITOPT_INPUTTABS		1
#define TEXTEDITOPT_UNDO			2

struct text_viewer_pos {
	uint line;
	uint pofs;
};

union text_search_pos {
	FileOfs offset;
};

class ht_text_editor;

#define ATOM_HT_UNDO_DATA_INSERT	MAGIC32("UND\x01")
#define ATOM_HT_UNDO_DATA_OVERWRITE	MAGIC32("UND\x02")
#define ATOM_HT_UNDO_DATA_DELETE	MAGIC32("UND\x03")
#define ATOM_HT_UNDO_DATA_DELETE2	MAGIC32("UND\x04")
#define ATOM_HT_UNDO_DATA_SPLIT_LINE	MAGIC32("UND\x05")
#define ATOM_HT_UNDO_DATA_JOIN_LINE	MAGIC32("UND\x06")
#define ATOM_HT_UNDO_DATA_INSERT_BLOCK	MAGIC32("UND\x07")
#define ATOM_HT_UNDO_DATA_DELETE_BLOCK	MAGIC32("UND\x08")

/*
 *	CLASS ht_undo_data
 */
class ht_undo_data: public Object {
public:
	virtual bool combine(ht_undo_data *ud);
	virtual uint getsize() = 0;
	virtual void gettext(char *text, uint maxlen) = 0;
	virtual void apply(ht_text_editor *te) = 0;
	virtual void unapply(ht_text_editor *te, bool *goto_only) = 0;
};

/*
 *	CLASS ht_undo_data_delete_string
 */
class ht_undo_data_delete_string: public ht_undo_data {
	text_viewer_pos apos;           /* cursor before */
	text_viewer_pos bpos;           /* cursor after */
	void *string;
	uint len;
public:
	ht_undo_data_delete_string(text_viewer_pos *apos, text_viewer_pos *bpos, void *string, uint len);
	~ht_undo_data_delete_string();
	virtual bool combine(ht_undo_data *ud);
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_undo_data_delete_string2
 */
class ht_undo_data_delete_string2: public ht_undo_data {
	text_viewer_pos apos;           /* cursor before */
	text_viewer_pos bpos;           /* cursor after */
	void *string;
	uint len;
public:
	ht_undo_data_delete_string2(text_viewer_pos *apos, text_viewer_pos *bpos, void *string, uint len);
	~ht_undo_data_delete_string2();
	virtual bool combine(ht_undo_data *ud);
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_undo_data_insert_string
 */
class ht_undo_data_insert_string: public ht_undo_data {
	text_viewer_pos apos;
	text_viewer_pos bpos;
	text_viewer_pos cpos;
	void *string;
	uint len;
public:
	ht_undo_data_insert_string(text_viewer_pos *apos, text_viewer_pos *bpos, void *string, uint len);
	~ht_undo_data_insert_string();
	virtual bool combine(ht_undo_data *ud);
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_undo_data_overwrite_string
 */
class ht_undo_data_overwrite_string: public ht_undo_data {
	text_viewer_pos apos;
	text_viewer_pos bpos;
	text_viewer_pos cpos;
	void *string;
	uint len;
	void *string2;
	uint len2;
public:
	ht_undo_data_overwrite_string(text_viewer_pos *apos, text_viewer_pos *bpos, void *string, uint len, void *string2, uint len2);
	~ht_undo_data_overwrite_string();
	virtual bool combine(ht_undo_data *ud);
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};


/*
 *	CLASS ht_undo_data_split_line
 */
class ht_undo_data_split_line: public ht_undo_data {
	text_viewer_pos apos;
	text_viewer_pos bpos;
	uint indent;
public:
	ht_undo_data_split_line(text_viewer_pos *apos, text_viewer_pos *bpos, uint Indent);
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_undo_data_join_line
 */
class ht_undo_data_join_line: public ht_undo_data {
	text_viewer_pos apos;
	text_viewer_pos bpos;     
	text_viewer_pos cpos;
public:
	ht_undo_data_join_line(text_viewer_pos *apos, text_viewer_pos *bpos);
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_undo_data_insert_block
 */

class ht_undo_data_insert_block: public ht_undo_data {
	text_viewer_pos apos;
	text_viewer_pos bpos;     
	text_viewer_pos cpos;
	text_viewer_pos sel_start;
	text_viewer_pos sel_end;
	void *block;
	uint size;
public:
	ht_undo_data_insert_block(text_viewer_pos *apos, text_viewer_pos *bpos, void *block, uint size);
	~ht_undo_data_insert_block();
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_undo_data_delete_block
 */

class ht_undo_data_delete_block: public ht_undo_data {
	text_viewer_pos apos;
	text_viewer_pos bpos;     
	text_viewer_pos cpos;
	text_viewer_pos sel_start;
	text_viewer_pos sel_end;
	void *block;
	uint size;
public:
	ht_undo_data_delete_block(text_viewer_pos *apos, text_viewer_pos *bpos, text_viewer_pos *Sel_start, text_viewer_pos *Sel_end);
	~ht_undo_data_delete_block();
	virtual uint getsize();
	virtual void gettext(char *text, uint maxlen);
	virtual ObjectID getObjectID() const;
	virtual void apply(ht_text_editor *te);
	virtual void unapply(ht_text_editor *te, bool *goto_only);
};

/*
 *	CLASS ht_text_editor_undo
 */
 
class ht_text_editor_undo: public Array {
public:
	uint size, max_size;
	int clean_state;
	int current_position;
	bool goto_state;
public:
	ht_text_editor_undo(uint max_undo_size);
	void insert_undo(ht_text_editor *te, ht_undo_data *undo);
	bool is_clean();
	bool is_clean(int i);
	int get_current_position();
	void mark_clean();
	void undo(ht_text_editor *te, bool place_cursor_first);
	void redo(ht_text_editor *te);
};

/*
 *	CLASS ht_text_viewer
 */

#define cmd_text_viewer_goto				HT_COMMAND(601)
#define cmd_text_viewer_change_highlight     HT_COMMAND(602)

#define cmd_text_editor_undo			HT_COMMAND(620)
#define cmd_text_editor_redo			HT_COMMAND(621)
#define cmd_text_editor_protocol		HT_COMMAND(622)
#define cmd_text_editor_delete_line		HT_COMMAND(623)

class ht_text_viewer: public ht_view {
friend class ht_undo_data;
friend class ht_undo_data_delete_string;
friend class ht_undo_data_delete_string2;
friend class ht_undo_data_insert_string;
friend class ht_undo_data_overwrite_string;
friend class ht_undo_data_split_line;
friend class ht_undo_data_join_line;
friend class ht_undo_data_insert_block;
friend class ht_undo_data_delete_block;
protected:
	ht_textfile *textfile;
	bool own_textfile;

	Container *lexers;
	
	ht_syntax_lexer *lexer;
	bool own_lexer;

	uint cursorx, cursory;
	text_viewer_pos sel_start, sel_end;

	uint top_line;
	uint xofs;

	bool selectcursor;
	bool selectmode;
	text_viewer_pos selectstart;

	char *EOL_string;
	char *EOF_string;
	bool show_EOL;
	bool show_EOF;
	bool highlight_wrap;
	uint	tab_size;

	ht_search_request *last_search_request;
	FileOfs last_search_end_ofs;

/* new */
			int buf_lprint0(int x, int y, int c, int l, char *text);
			uint char_vsize(char c, uint x);
			void clipboard_copy_cmd();
			bool continue_search();
	virtual	vcp  get_bgcolor();
			void make_pos_physical(text_viewer_pos *p);
			void normalize_selection();
			uint physical_cursorx();
			void popup_change_highlight();
			void render_meta(uint x, uint y, text_viewer_pos *pos, vcp color);
			void render_str(int x, int y, vcp color, text_viewer_pos *pos, uint len, char *str, bool multi);
			void render_str_color(vcp *color, text_viewer_pos *pos);
			ht_search_result *search(ht_search_request *request, text_search_pos *start, text_search_pos *end);
			bool show_search_result(ht_search_result *result);
public:
			void init(Bounds *b, bool own_textfile, ht_textfile *textfile, Container *lexers);
	virtual	void done();
/* overwritten */
	virtual	void config_changed();
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void resize(int rw, int rh);
/* new */
	virtual	const char *func(uint i, bool execute);
			uint get_line_length(uint line);
			uint get_line_vlength(uint line);
			uint get_line_indent(uint line);
			void get_selection(text_viewer_pos *start, text_viewer_pos *end);
			bool goto_line(uint line);
	/* position indicator string */
	virtual	int get_pindicator_str(char *buf, int max_len);
	/* scrollbar pos */
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	/* cursor */
	virtual	CursorMode get_cursor_mode();
	virtual	void get_cursor_pos(text_viewer_pos *cursor);
	/* conversions */
	virtual	bool pos_to_offset(text_viewer_pos *pos, FileOfs *ofs);
	/**/
			ht_syntax_lexer *get_lexer();
			int ppos_str(char *buf, uint bufsize, text_viewer_pos *ppos);
			void set_lexer(ht_syntax_lexer *l, bool own_l);
			void set_textfile(ht_textfile *t, bool own_t);
			ht_textfile *get_textfile();
/**/
			uint cursor_up(uint n);
			uint cursor_down(uint n);
			uint cursor_left(uint n);
			uint cursor_right(uint n);
			void cursor_home();
			void cursor_end();
			void cursor_vput(uint vx);
			void cursor_pput(uint px);
			void cursor_set(text_viewer_pos *pos);
			uint scroll_up(uint n);
			uint scroll_down(uint n);
			uint scroll_left(uint n);
			uint scroll_right(uint n);
			void select_add(text_viewer_pos *from, text_viewer_pos *to);
			void select_clear();
			void select_end();
			void select_set(text_viewer_pos *from, text_viewer_pos *to);
			void select_start();
};

/*
 *	CLASS ht_text_editor
 */

class ht_text_editor: public ht_text_viewer {
protected:
	uint edit_options;
	ht_text_editor_undo *undo_list;
	bool	auto_indent;
	bool overwrite_mode;
	
/* new */
			void clipboard_cut_cmd();
			void clipboard_delete_cmd();
			void clipboard_paste_cmd();
	virtual	vcp  get_bgcolor();
			bool save();
public:
			void init(Bounds *b, bool own_textfile, ht_textfile *textfile, Container *lexers, uint edit_options);
	virtual	void done();
/* overwritten */
	virtual	void config_changed();
	virtual	const char *func(uint i, bool execute);
	virtual	void handlemsg(htmsg *msg);
	/* position indicator string */
	virtual	int get_pindicator_str(char *buf, int max_len);
	/* cursor mode */
	virtual	CursorMode get_cursor_mode();
/* new */
			bool concat_lines(uint a);
			void delete_chars(uint line, uint ofs, uint count);
			void delete_lines(uint line, uint count);
			void indent(uint line, uint start, uint size);
			void insert_chars(uint line, uint ofs, void *chars, uint len);
			void insert_lines(uint line, uint count);
			void split_line(uint a, uint pos);
			void unindent(uint line, uint start, uint size);
	/* undo/redo */
			void textoperation_apply(ht_undo_data *ud);
			void redo();
			void show_protocol();
			void undo(bool place_cursor_first);
};

extern int text_viewer_pos_compare(text_viewer_pos *a, text_viewer_pos *b);

#endif /* __TEXTEDIT_H__ */

