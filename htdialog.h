/*
 *	HT Editor
 *	htdialog.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTDIALOG_H__
#define __HTDIALOG_H__

#include "htdebug.h"
#include "htobj.h"
#include "htstring.h"

/*
 *	CLASS ht_dialog
 */

#define ds_normal			0
#define ds_term_ok			1
#define ds_term_cancel		2

#define button_cancel		0
#define button_ok			1
#define button_yes            button_ok
#define button_no			2
#define button_skip			button_no
#define button_all			3
#define button_none			4

class ht_queued_msg: public ht_data {
public:
	ht_view *target;
	htmsg msg;
};

class ht_dialog: public ht_window {
protected:
	int state;
	int return_val;

	ht_queue *msgqueue;

/* overwritten */
	virtual	char *defaultpalette();
			ht_queued_msg *dequeuemsg();
public:
			void	init(bounds *b, char *desc, UINT framestyle);
	virtual	void	done();
/* overwritten */
	virtual	int alone();
	virtual 	void draw();
	virtual	void handlemsg(htmsg *msg);
			void queuemsg(ht_view *target, htmsg *msg);
/* new */
	virtual	int getstate(int *return_val);
	virtual	int run(bool modal);
	virtual	void setstate(int state, int return_val);
};

/*
 *	CLASS ht_cluster
 */

class ht_cluster: public ht_view {
protected:
			ht_string_list *strings;
			int sel;
			int scount;
			ht_key shortcuts[32];
/* overwritten */
	virtual	char *defaultpalette();
public:
			void	init(bounds *b, ht_string_list *strings);
	virtual	void	done();
};

/*
 *	CLASS ht_checkboxes
 */

struct ht_checkboxes_data {
	DDECL_UINT(state);
};

class ht_checkboxes: public ht_cluster {
protected:
			dword state;
public:
			void	init(bounds *b, ht_string_list *strings);
	virtual	void	done();
/* overwritten */
	virtual	int datasize();
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void getdata(ht_object_stream *s);
	virtual	void setdata(ht_object_stream *s);
};

/*
 *	CLASS ht_radioboxes
 */

struct ht_radioboxes_data {
	DDECL_UINT(sel);
};

class ht_radioboxes: public ht_cluster {
public:
			void	init(bounds *b, ht_string_list *strings);
	virtual	void	done();
/* overwritten */
	virtual	int datasize();
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void getdata(ht_object_stream *s);
	virtual	void setdata(ht_object_stream *s);
};

/*
 *	CLASS ht_inputfield
 */

class ht_inputfield;

struct ht_inputfield_data {
	DDECL_UINT(textlen);
	DDECL_PTR(byte, text);
};

class ht_inputfield: public ht_view {
protected:
			byte **text, *textv;
			byte **curchar, *curcharv;
			byte **selstart, *selstartv;
			byte **selend, *selendv;
			int *textlen, textlenv;
			int *maxtextlen, maxtextlenv;
			int insert;
			int ofs;
			ht_inputfield *attachedto;
			ht_list *history;

			void freebuf();
			int insertbyte(byte *pos, byte b);
			void select_add(byte *start, byte *end);
/* overwritten */
	virtual	char *defaultpalette();
public:
			void	init(bounds *b, int maxtextlen, ht_list *history=0);
	virtual	void	done();
/* overwritten */
	virtual	int datasize();
	virtual	void getdata(ht_object_stream *s);
	virtual	void setdata(ht_object_stream *s);
/* new */
			void attach(ht_inputfield *inputfield);
			void query(byte ***curchar, byte ***text, byte ***selstart, byte ***selend, int **textlen, int **maxtextlen);
			void isetcursor(UINT pos);
};

/*
 *	CLASS ht_strinputfield
 */

#define ht_strinputfield_data ht_inputfield_data

class ht_strinputfield: public ht_inputfield {
protected:
	bool is_virgin;		/* untouched except for cursor keys */
	bool selectmode;
	
/* new */
			void correct_viewpoint();
			void history_dialog();
			bool inputbyte(byte a);
			bool setbyte(byte a);
public:
			void	init(bounds *b, int maxtextlen, ht_list *history=0);
	virtual	void	done();
/* overwritten */
	virtual 	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void receivefocus();
};

/*
 *	CLASS ht_hexinputfield
 */

#define ht_hexinputfield_data ht_inputfield_data

class ht_hexinputfield: public ht_inputfield {
protected:
			int nib;

			void correct_viewpoint();
public:
			void	init(bounds *b, int maxtextlen);
	virtual	void	done();
/* overwritten */
	virtual 	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void receivefocus();
/* new */
			void setnibble(byte a);
};

/*
 *	CLASS ht_button
 */

class ht_button: public ht_view {
protected:
	int value;
	int pressed;
	char *text;
	char *magicchar;
	ht_key shortcut1;
	ht_key shortcut2;

/* overwritten */
	virtual	char *defaultpalette();
public:
			void	init(bounds *b, char *text, int value);
	virtual	void	done();
/* overwritten */
	virtual 	void draw();
	virtual	void handlemsg(htmsg *msg);
/* new */
	virtual	void push();
};

/*
 *	CLASS ht_listbox
 */

struct ht_listbox_data {
	DDECL_UINT(top_id);
	DDECL_UINT(cursor_id);
};

#define LISTBOX_NORMAL 0
#define LISTBOX_QUICKFIND 1

class ht_listbox: public ht_view {
public:
	int cursor, pos, count;
	int visible_height;
	void *e_top, *e_cursor;
	ht_scrollbar *scrollbar;
	int x;
	char		quickfinder[100];
	char		*qpos;
	UINT		listboxcaps;

			void	init(bounds *b, UINT Listboxcaps=LISTBOX_QUICKFIND);
	virtual	void	done();
	virtual	int 	load(ht_object_stream *f);
			void adjust_pos_hack();
			void adjust_scrollbar();
	virtual   int  calc_count() = 0;
			void clear_quickfind();
	virtual   int  cursor_adjust();						// stub
			int  cursor_up(int n);
			int  cursor_down(int n);
	virtual	int	datasize();
	virtual	char *defaultpalette();
	virtual	void draw();
	virtual	int	estimate_entry_pos(void *entry);
	virtual	void *getbyid(UINT id);
	virtual	void getdata(ht_object_stream *s);
	virtual   void *getfirst() = 0;
	virtual   void *getlast() = 0;
	virtual	UINT getid(void *entry);                          // stub
	virtual   void *getnext(void *entry) = 0;
	virtual   void *getnth(int n);
	virtual   void *getprev(void *entry) = 0;
	virtual   char *getstr(int col, void *entry) = 0;
			void goto_item(void *entry);
	virtual	void handlemsg(htmsg *msg);
	virtual   int	num_cols();							// stub
	virtual	void	*quickfind(char *s) = 0;
	virtual	char	*quickfind_completition(char *s);            // stub
	virtual	void redraw();
	virtual	bool	seek(int index);
	virtual   void select_entry(void *entry);
	virtual	void setdata(ht_object_stream *s);
	virtual	void	store(ht_object_stream *f);
	virtual	void update();
			void update_cursor();
	virtual	ht_data *vstate_create();
			void vstate_save();
/* overwritten */
	virtual	void resize(int rw, int rh);
};

/*
 *	CLASS ht_text_listbox
 */

struct ht_text_listbox_item {
	ht_text_listbox_item	*next, *prev;
	int					id;
	char					*data[0];
};

struct ht_text_listbox_sort_order {
	int		col;
	int		(*compare_func)(const char *key_a, const char *key_b);
};

class ht_text_listbox: public ht_listbox {
public:
	int					cols, keycol, count;
	ht_text_listbox_item	*first, *last;
	int					*widths;
	char					*return_str;
	int					Cursor_adjust;
			void	init(bounds *b, int Cols=1, int Keycol=0, UINT Listboxcaps=LISTBOX_QUICKFIND);
	virtual	void	done();
	virtual	int 	load(ht_object_stream *f);
	virtual   int  calc_count();
	virtual	int	compare_strn(char *s1, char *s2, int l);
	virtual	int	compare_ccomm(char *s1, char *s2);
	virtual   int	cursor_adjust();
	virtual   void	*getfirst();
	virtual	UINT getid(void *entry);                          // stub
	virtual   void	*getlast();
	virtual   void	*getnext(void *entry);
	virtual   void	*getprev(void *entry);
	virtual   char	*getstr(int col, void *entry);
			void goto_item_by_id(UINT id);
			void goto_item_by_position(UINT pos);
			void	insert_str(int id, char *str, ...);
	virtual   int	num_cols();
	virtual	void	*quickfind(char *s);
	virtual	char	*quickfind_completition(char *s);
	virtual	void	store(ht_object_stream *f);
			void sort(int count, ht_text_listbox_sort_order *so);
	virtual   void update();
};

class ht_itext_listbox: public ht_text_listbox {
public:
			void	init(bounds *b, int Cols=1, int Keycol=0);
	virtual	void	done();
	virtual	int	compare_strn(char *s1, char *s2, int l);
	virtual	int	compare_ccomm(char *s1, char *s2);
};

/*
 *	CLASS ht_statictext
 */

#define align_left		0
#define align_center	1
#define align_right		2
#define align_custom	3

#define ALIGN_CHAR_ESCAPE	'\e'
#define ALIGN_CHAR_LEFT		'l'
#define ALIGN_CHAR_CENTER	'c'
#define ALIGN_CHAR_RIGHT		'r'
 
struct ht_statictext_linedesc {
	int ofs;
	int len;
	char *text;
};

class ht_statictext: public ht_text {
protected:
	char *text;
	int align;
	bool breaklines;
	bool transparent;

/* overwritten */
	virtual	char *defaultpalette();
public:
			void	init(bounds *b, char *text, int align, bool breaklines=true, bool transparent=false);
	virtual	void	done();
/* overwritten */
	virtual 	void draw();
	virtual	char *gettext();
/* new */
	virtual	vcp gettextcolor();
	virtual	void settext(char *text);
};

/*
 *	CLASS ht_listpopup_dialog
 */

struct ht_listpopup_dialog_data {
	DDECL_UINT(cursor_id);
	DDECL_PTR(char, cursor_string);
};
 
class ht_listpopup_dialog: public ht_dialog {
protected:
	ht_listbox *listbox;
	virtual	void init_text_listbox(bounds *b);
public:
			void init(bounds *b, char *desc);
	virtual	void done();
/* overwritten */
	virtual	char *defaultpalette();
	virtual	int	datasize();
	virtual	void getdata(ht_object_stream *s);
			void	insertstring(char *string);
	virtual	void setdata(ht_object_stream *s);
/* new */
			void select_next();
			void select_prev();
};

/*
 *	CLASS ht_listpopup
 */
 
#define ht_listpopup_data ht_listpopup_dialog_data

class ht_listpopup: public ht_statictext {
protected:
	ht_listpopup_dialog *listpopup;
/* new */	
			int	run_listpopup();
public:
			void	init(bounds *b);
	virtual	void	done();
/* overwritten */
	virtual	int  datasize();
	virtual 	void draw();
	virtual	vcp  gettextcolor();
	virtual	void getdata(ht_object_stream *s);
	virtual	char *gettext();
	virtual	void handlemsg(htmsg *msg);
	virtual	void setdata(ht_object_stream *s);
/* new */
			void	insertstring(char *string);
};

/*
 *	CLASS ht_history_listbox
 */

class ht_history_listbox: public ht_listbox {
	ht_list	*history;
public:
			void init(bounds *b, ht_list *hist);
	virtual   int  calc_count();
	virtual   void *getfirst();
	virtual   void *getlast();
	virtual   void *getnext(void *entry);
	virtual   void *getnth(int n);
	virtual   void *getprev(void *entry);
	virtual   char *getstr(int col, void *entry);
	virtual	void handlemsg(htmsg *msg);
	virtual	void *quickfind(char *s);
	virtual	char	*quickfind_completition(char *s);
};

/*
 *	CLASS ht_history_popup_dialog
 */

class ht_history_popup_dialog: public ht_listpopup_dialog {
protected:
	ht_list	*history;
	virtual	void init_text_listbox(bounds *b);
public:
			void init(bounds *b, ht_list *hist);
	virtual	void getdata(ht_object_stream *s);
	virtual	void setdata(ht_object_stream *s);
};

/*
 *	CLASS ht_label
 */

class ht_label: public ht_view {
protected:
	ht_view *connected;
	char *text;
	char *magicchar;
	ht_key shortcut;

/* overwritten */
	virtual	char *defaultpalette();
public:
			void	init(bounds *b, char *text, ht_view *connected);
	virtual	void	done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_progress_indicator
 */

class ht_progress_indicator: public ht_window {
protected:
/* overwritten */
	virtual	char *defaultpalette();
public:
	ht_statictext *text;
	
			void	init(bounds *b, char *hint);
/* new */
			void settext(char *text);
};

/*
 *	CLASS ht_color_block
 */

struct ht_color_block_data {
	DDECL_UINT(color);
};

#define cf_light		1
#define cf_transparent   2

class ht_color_block: public ht_view {
protected:
	int color;
	int colors;
	int flags;

/* overwritten */
	virtual	char *defaultpalette();
public:
			void	init(bounds *b, int selected, int flags);
	virtual	void	done();
/* overwritten */
	virtual	int datasize();
	virtual	void draw();
	virtual	void getdata(ht_object_stream *s);
	virtual	void setdata(ht_object_stream *s);
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_listbox_ptr
 */

class ht_listbox_ptr: public ht_data {
public:
	ht_listbox *listbox;

	ht_listbox_ptr(ht_listbox *listbox);
	~ht_listbox_ptr();
};

void center_bounds(bounds *b);

#endif /* !__HTDIALOG_H__ */

