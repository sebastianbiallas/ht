/* 
 *	HT Editor
 *	htmenu.h
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

#ifndef __HTMENU_H__
#define __HTMENU_H__

#include "data.h"
#include "htdialog.h"
#include "htobj.h"

/*
 *	CLASS ht_context_menu_entry
 */

class ht_context_menu;

#define CME_ENTRY	0
#define CME_SEPARATOR	1
#define CME_SUBMENU	2

class ht_context_menu_entry: public Object {
public:
	int type;
	union {
		struct {
			char *name;
			char *shortcut;
			char *comment;
			int command;
			int key;
			bool active;
		} entry;
		ht_context_menu *submenu;
	};

	virtual ~ht_context_menu_entry();
};

/*
 *	CLASS ht_context_menu
 */

class ht_context_menu: public Object {
private:
	char *name;
	char *shortcut;
public:
	int xpos, width; /* used externally */

		void init(const char *name);
	virtual	void done();
/* new */
	virtual	int count();
	virtual ht_context_menu_entry *enum_entry_first();
	virtual ht_context_menu_entry *enum_entry_next();
	virtual	ht_context_menu_entry *get_entry(int n);
	virtual	const char *get_name();
	virtual	const char *get_shortcut();
};

/*
 *	CLASS ht_static_context_menu
 */

class ht_static_context_menu: public ht_context_menu {
protected:
	List *context_menu_entry;
	int enum_idx;

public:

		void init(const char *name);
	virtual	void done();
/* new */
		void insert_entry(const char *name, const char *comment, int command, int key, bool active);
		void insert_separator();
		void insert_submenu(ht_context_menu *submenu);
/* overwritten */
	virtual	int count();
	virtual   ht_context_menu_entry *enum_entry_first();
	virtual   ht_context_menu_entry *enum_entry_next();
};

/*
 *	CLASS ht_menu
 */

class ht_menu: public ht_view {
protected:
	int lastmenux;
	List *menu;
	int curmenu;
	int localmenu;
	bool context_menu_hack2;
	ht_context_menu *context_menu_hack;
	ht_context_menu *last_context_menu_hack;

		void execute_menu(int i);
		ht_context_menu *get_context_menu(int i);
		bool handle_key_context_menu(ht_context_menu *a, int k);
/* overwritten */
	virtual	const char *defaultpalette();
	virtual	const char *defaultpaletteclass();
public:
		void init(Bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual void getminbounds(int *width, int *height);
/* new */
		int count();
		void insert_menu(ht_context_menu *m);
		void insert_local_menu();
		bool set_local_menu(ht_context_menu *m);
		void delete_local_menu();
};

/*
 *	CLASS ht_context_menu_window_body
 */

class ht_context_menu_window_body: public ht_view {
protected:
	ht_context_menu *context_menu;
	int selected;
/* new */
	int next_selectable(int to);
	int prev_selectable(int to);

/* overwritten */
	virtual	const char *defaultpalette();
	virtual	const char *defaultpaletteclass();
public:
		void init(Bounds *b, ht_context_menu *menu);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	void getdata(ObjectStream &s);
	virtual	void setdata(ObjectStream &s);
};

/*
 *	CLASS ht_menu_window
 */

class ht_menu_window_body;

struct ht_menu_window_data {
	int selected;
};

class ht_menu_window: public ht_dialog {
protected:
	ht_menu_window_body *body;
	ht_context_menu *menu;
public:
		void init(Bounds *b, ht_context_menu *menu);
	virtual	void done();
/* overwritten */
	virtual	void getdata(ObjectStream &s);
	virtual	void handlemsg(htmsg *msg);
	virtual	void setdata(ObjectStream &s);
};

/*
 *	CLASS ht_menu_window_body
 */

class ht_menu_window_body: public ht_context_menu_window_body {
public:
		void init(Bounds *b, ht_context_menu *menu);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_menu_frame
 */

class ht_menu_frame: public ht_frame {
protected:
/* overwritten */
	virtual	int getcurcol_normal();
	virtual	int getcurcol_killer();
/* overwritten */
	virtual	const char *defaultpalette();
	virtual	const char *defaultpaletteclass();
public:
		void init(Bounds *b, const char *desc, uint style, uint number=0);
	virtual void done();
};

/*
 *	INIT
 */

bool init_menu();

/*
 *	DONE
 */

void done_menu();

#endif /* !__HTMENU_H__ */
