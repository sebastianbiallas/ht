/*
 *	HT Editor
 *	htdialog.h
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

#ifndef __HTDIALOG_H__
#define __HTDIALOG_H__

#include <stdarg.h>

#include "htdebug.h"
#include "htobj.h"
#include "strtools.h"
#include "io/keyb.h"

/*
 *	CLASS ht_dialog
 */

#define ds_normal			0
#define ds_term_ok			1
#define ds_term_cancel			2

#define button_cancel			0
#define button_ok			1
#define button_yes			button_ok
#define button_no			2
#define button_skip			button_no
#define button_all			3
#define button_none			4

class ht_queued_msg: public Object {
public:
	ht_view *target;
	htmsg msg;
		    ht_queued_msg(ht_view *target, htmsg &msg);
};

class ht_dialog: public ht_window {
protected:
	int state;
	int return_val;

	Queue *msgqueue;

	/* overwritten */
	virtual	const char *defaultpalette();
			ht_queued_msg *dequeuemsg();
public:
			ht_dialog() {};
			ht_dialog(BuildCtorArg&a): ht_window(a) {};
		void	init(Bounds *b, const char *desc, uint framestyle);
	virtual	void	done();
	/* overwritten */
	virtual	int aclone();
	virtual void draw();
	virtual	void handlemsg(htmsg *msg);
	/* new */
		void queuemsg(ht_view *target, htmsg &msg);
	virtual	int getstate(int *return_val);
	virtual	int run(bool modal);
	virtual	void setstate(int state, int return_val);
};


/*
 *	CLASS ht_cluster
 */

class ht_cluster: public ht_dialog_widget {
protected:
	ht_string_list *strings;
	int sel;
	int scount;
	ht_key shortcuts[32];
	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void	init(Bounds *b, ht_string_list *strings);
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
	uint32 state;
public:
		void	init(Bounds *b, ht_string_list *strings);
	virtual	void	done();
	/* overwritten */
	virtual	int datasize();
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void getdata(ObjectStream &s);
	virtual	void setdata(ObjectStream &s);
};

/*
 *	CLASS ht_radioboxes
 */

struct ht_radioboxes_data {
	DDECL_UINT(sel);
};

class ht_radioboxes: public ht_cluster {
public:
		void	init(Bounds *b, ht_string_list *strings);
	virtual	void	done();
	/* overwritten */
	virtual	int datasize();
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void getdata(ObjectStream &s);
	virtual	void setdata(ObjectStream &s);
};

/*
 *	CLASS ht_inputfield
 */

class ht_inputfield;

struct ht_inputfield_data {
	DDECL_UINT(textlen);
	DDECL_PTR(byte, text);
};

class ht_inputfield: public ht_dialog_widget {
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
	List *history;

		void freebuf();
		int insertbyte(byte *pos, byte b);
		void select_add(byte *start, byte *end);
	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void	init(Bounds *b, int maxtextlen, List *history = NULL);
	virtual	void	done();
	/* overwritten */
	virtual	int datasize();
	virtual	void getdata(ObjectStream &s);
	virtual	void setdata(ObjectStream &s);
	/* new */
		void attach(ht_inputfield *inputfield);
		void query(byte ***curchar, byte ***text, byte ***selstart, byte ***selend, int **textlen, int **maxtextlen);
		void isetcursor(uint pos);
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
		void	init(Bounds *b, int maxtextlen, List *history = NULL);
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
		void init(Bounds *b, int maxtextlen);
	virtual	void done();
	/* overwritten */
	virtual void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void receivefocus();
	/* new */
			void setnibble(byte a);
};

/*
 *	CLASS ht_button
 */

class ht_button: public ht_dialog_widget {
protected:
	int value;
	int pressed;
	char *text;
	char *magicchar;
	ht_key shortcut1;
	ht_key shortcut2;

	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void	init(Bounds *b, const char *text, int value);
	virtual	void	done();
	/* overwritten */
	virtual void 	draw();
	virtual	void 	handlemsg(htmsg *msg);
	virtual void	getminbounds(int *width, int *height);
	/* new */
	virtual	void 	push();
};


/*
 *	CLASS ht_listbox_title
 */

class ht_listbox;

class ht_listbox_title: public ht_dialog_widget {
public:
	ht_listbox *listbox;
protected:
	char **texts;
	int cols;

	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void init(Bounds *b);               
	virtual	void done();
	/* overwritten */
	virtual	void draw();
	/* new */
	virtual	vcp getTextColor();
		void setText(int cols, ...);
		void setTextv(int cols, va_list arguments);
		void update();
};

/*
 *	CLASS ht_listbox
 */

struct ht_listbox_data_internal {
	void *top_ptr;
	void *cursor_ptr;
};

struct ht_listbox_data {
	DDECL_PTR(ht_listbox_data_internal, data);
};

#define LISTBOX_NORMAL 0
#define LISTBOX_QUICKFIND 1

class ht_listbox: public ht_dialog_widget {
protected:
public:
	int	cursor, pos, cached_count;
	int	visible_height;
	void	*e_top, *e_cursor;
	int	x;
	char	quickfinder[100];
	char	*qpos;
	uint	listboxcaps;

	int	cols;
	int	*widths;
	
	ht_scrollbar *scrollbar;
	bool	mScrollbarEnabled;
	ht_listbox_title *title;

public:
		void		init(Bounds *b, uint Listboxcaps=LISTBOX_QUICKFIND);
	virtual	void		done();
		void		attachTitle(ht_listbox_title *title);
		void		adjustPosHack();
		void		adjustScrollbar();
	virtual	int		calcCount() = 0;
		void		clearQuickfind();
	virtual	int		cursorAdjust();
		int		cursorUp(int n);
		int		cursorDown(int n);
	virtual	int		datasize();
	virtual	const char *	defaultpalette();
	virtual	void		draw();
	virtual	int		estimateEntryPos(void *entry);
	virtual	void		getdata(ObjectStream &s);
	virtual	void *		getFirst() = 0;
	virtual	void *		getLast() = 0;
	virtual	void *		getNext(void *entry) = 0;
	virtual	void *		getPrev(void *entry) = 0;
	virtual	const char *	getStr(int col, void *entry) = 0;
		void		gotoItemByEntry(void *entry, bool clear_quickfind = true);
	virtual	void		gotoItemByPosition(uint pos);
	virtual	void		handlemsg(htmsg *msg);
	virtual	int		numColumns();
	virtual	void *		quickfind(const char *s) = 0;
	virtual	char *		quickfindCompletition(const char *s);
	virtual	void		redraw();
	virtual	void		resize(int rw, int rh);
	virtual	bool		selectEntry(void *entry);
	virtual	void		setdata(ObjectStream &s);
	virtual	void		stateChanged();
	virtual	void		update();
		void		updateCursor();
	virtual	Object *	vstate_create();
		void		vstate_save();
protected:
		void		rearrangeColumns();
};

/*
 *	CLASS ht_text_listbox
 */

#define ht_text_listbox_data ht_listbox_data

struct ht_text_listbox_item {
	ht_text_listbox_item	*next, *prev;
	int id;
	void			*extra_data;
	char			*data[0];
};

struct ht_text_listbox_sort_order {
	int		col;
	int		(*compare_func)(const char *key_a, const char *key_b);
};

class ht_text_listbox: public ht_listbox {
protected:
	int			cols, keycol, count;
	ht_text_listbox_item	*first, *last;
	int			Cursor_adjust;

public:
		void	init(Bounds *b, int Cols=1, int Keycol=0, uint Listboxcaps=LISTBOX_QUICKFIND);
	virtual	void	done();
	virtual	int	calcCount();
	virtual	int	compare_strn(const char *s1, const char *s2, int l);
	virtual	int	compare_ccomm(const char *s1, const char *s2);
	virtual	int	cursorAdjust();
		void *	getEntryByID(uint id);
	virtual	void *	getFirst();
	virtual	void *	getLast();
		uint	getID(void *entry);
		void *	getExtra(void *entry);
	virtual	void *	getNext(void *entry);
	virtual	void *	getPrev(void *entry);
	virtual	const char *getStr(int col, void *entry);
		void	insert_str(int id, const char *str, ...);
		void	insert_str(int id, const char **strs);
		void	insert_str_extra(int id, void *extra_data, const char *str, ...);
		void	insert_str_extra(int id, void *extra_data, const char **strs);
	virtual	int	numColumns();
	virtual	void *	quickfind(const char *s);
	virtual	char *	quickfindCompletition(const char *s);
		void	sort(int count, ht_text_listbox_sort_order *so);
	virtual	void	update();
protected:
	virtual	void	clearAll();
	virtual	void	freeExtraData(void *extra_data);
};

#define ht_itext_listbox_data ht_text_listbox_data

class ht_itext_listbox: public ht_text_listbox {
public:
		void	init(Bounds *b, int Cols=1, int Keycol=0);
	virtual	int	compare_strn(const char *s1, const char *s2, int l);
	virtual	int	compare_ccomm(const char *s1, const char *s2);
};

/*
 *	CLASS ht_statictext
 */

enum statictext_align {
	align_left,
	align_center,
	align_right,
	align_custom
};

#define ALIGN_CHAR_ESCAPE	'\e'
#define ALIGN_CHAR_LEFT		'l'
#define ALIGN_CHAR_CENTER	'c'
#define ALIGN_CHAR_RIGHT	'r'
 
struct ht_statictext_linedesc {
	int ofs;
	int len;
	char *text;
};

class ht_statictext: public ht_text {
protected:
	char *text;
	statictext_align align;
	bool breaklines;
	bool transparent;

	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void	init(Bounds *b, const char *text, statictext_align align, bool breaklines=true, bool transparent=false);
	virtual	void	done();
	/* overwritten */
	virtual	void draw();
	virtual	void settext(const char *text);
	/* new */
	virtual	int gettext(char *text, int maxlen);
	virtual	vcp gettextcolor();
};

/*
 *	CLASS ht_listpopup_dialog
 */

struct ht_listpopup_dialog_data {
	DDECL_UINT(cursor_pos);
	DDECL_PTR(char, cursor_string);
};
 
class ht_listpopup_dialog: public ht_dialog {
protected:
	ht_listbox *listbox;
	virtual	void init_text_listbox(Bounds *b);
public:
		void init(Bounds *b, const char *desc);
	/* overwritten */
	virtual	const char *defaultpalette();
	virtual	int  datasize();
	virtual	void getdata(ObjectStream &s);
		void insertstring(const char *string);
	virtual	void setdata(ObjectStream &s);
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
		void	init(Bounds *b);
	virtual	void	done();
	/* overwritten */
	virtual	int  datasize();
	virtual void draw();
	virtual	vcp  gettextcolor();
	virtual	void getdata(ObjectStream &s);
	virtual	int gettext(char *text, int maxlen);
	virtual	void handlemsg(htmsg *msg);
	virtual	void setdata(ObjectStream &s);
	/* new */
		void	insertstring(const char *string);
};

/*
 *	CLASS ht_history_listbox
 */

class ht_history_listbox: public ht_listbox {
	List	*history;
public:
		void init(Bounds *b, List *hist);
	virtual int  calcCount();
	virtual void *getFirst();
	virtual void *getLast();
	virtual void *getNext(void *entry);
	virtual void *getPrev(void *entry);
	virtual const char *getStr(int col, void *entry);
	virtual	void handlemsg(htmsg *msg);
	virtual	void *quickfind(const char *s);
	virtual	char *quickfindCompletition(const char *s);
};

/*
 *	CLASS ht_history_popup_dialog
 */

class ht_history_popup_dialog: public ht_listpopup_dialog {
protected:
	List	*history;
	virtual	void init_text_listbox(Bounds *b);
public:
		void init(Bounds *b, List *hist);
	virtual	void getdata(ObjectStream &s);
	virtual	void setdata(ObjectStream &s);
};

/*
 *	CLASS ht_label
 */

class ht_label: public ht_dialog_widget {
protected:
	ht_view *connected;
	char *text;
	char *magicchar;
	ht_key shortcut;

	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void	init(Bounds *b, const char *text, ht_view *connected);
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
	virtual	const char *defaultpalette();
public:
	ht_statictext *text;
	
		void init(Bounds *b, const char *hint);
	/* new */
		void settext(const char *text);
};

/*
 *	CLASS ht_color_block
 */

struct ht_color_block_data {
	DDECL_UINT(color);
};

#define cf_light	1
#define cf_transparent	2

class ht_color_block: public ht_dialog_widget {
protected:
	int color;
	int colors;
	int flags;

	/* overwritten */
	virtual	const char *defaultpalette();
public:
		void	init(Bounds *b, int selected, int flags);
	/* overwritten */
	virtual	int datasize();
	virtual	void draw();
	virtual	void getdata(ObjectStream &s);
	virtual	void setdata(ObjectStream &s);
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_listbox_ptr
 */

class ht_listbox_ptr: public Object {
public:
	ht_listbox *listbox;

	ht_listbox_ptr(ht_listbox *aListbox)
		: listbox(aListbox)
	{
	}
};

void center_bounds(Bounds *b);

#endif /* !__HTDIALOG_H__ */
