/*
 *	HT Editor
 *	htapp.h
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

#ifndef __HTAPP_H__
#define __HTAPP_H__

#include "htctrl.h"
#include "htdialog.h"
#include "htformat.h"

#define msg_filesize_changed			HT_MESSAGE(100)

// file open modes

#define FOM_AUTO					0
#define FOM_BIN					1
#define FOM_TEXT					2

//
#define VIEWERGROUP_NAME				"viewergroup"

/*
 *	CLASS ht_status
 */
#define STATUS_DEFAULT_FORMAT "%a %L %t %d"
#define STATUS_ESCAPE '%'
#define STATUS_ANALY_ACTIVE 'a'
#define STATUS_ANALY_LINES 'L'
#define STATUS_TIME 't'
#define STATUS_DATE 'd'
#define STATUS_WORKBUFLEN 80

/*
 *	CLASS ht_status
 */

class ht_status: public ht_view {
protected:
	int		idle_count;
	char		*format;
	char		workbuf[STATUS_WORKBUFLEN];
	int		clear_len;
	int		analy_ani;
public:
			void init(bounds *b);
	virtual	void done();
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	bool idle();
private:
			void render();
	virtual	char *defaultpalette();
};

/*
 *	CLASS ht_keyline
 */

class ht_keyline: public ht_view {
public:
			void init(bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	char *defaultpalette();
};

/*
 *	CLASS ht_desktop
 */

class ht_desktop: public ht_view {
public:
			void init(bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	char *defaultpalette();
};

/*
 *	CLASS ht_logviewer
 */

class ht_log_msg: public ht_data {
public:
	int color;
	char *msg;
	ht_log_msg(char *Msg, int Color);
	~ht_log_msg();
};

class ht_logviewer: public ht_viewer {
private:
	ht_clist *log;
	int ofs, xofs;
	ht_window *window;

/* new */
	int cursor_up(int n);
	int cursor_down(int n);
	bool get_hscrollbar_pos(int *pstart, int *psize);
	void update();
public:
			void init(bounds *b, ht_clist *log, ht_window *window);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_file_window
 */

class ht_file_window: public ht_window {
public:
	ht_streamfile	*file;
			void	init(bounds *b, char *desc, UINT framestyle, UINT number, ht_streamfile *file);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_app_window_entry
 */

class ht_app_window_entry: public ht_data {
public:
	UINT type;
	ht_window *window;
	bool minimized;
	UINT number;
	bool isfile;
	ht_layer_streamfile *layer;

	ht_app_window_entry(ht_window *window, UINT number, UINT type, bool minimized, bool isfile, ht_layer_streamfile *layer);
	~ht_app_window_entry();
};

/*
 *	CLASS ht_app
 */

class ht_app: public ht_dialog {
protected:
	ht_sorted_list *windows;

	ht_clist *log;
	
	ht_list *syntax_lexers;

	ht_keyline *keyline;
	ht_desktop *desktop;

	ht_group *battlefield;
	
	int exit_program;

/* new */
			void log_deletefirstline();
			void	log_insertline(int c, char *line);
	
			bool create_window_file_bin(bounds *b, ht_layer_streamfile *file, char *title, bool isfile);
			bool create_window_file_text(bounds *b, ht_layer_streamfile *file, char *title, bool isfile);
			
			bool accept_close_all_windows();
			UINT find_free_window_number();
			void	insert_window(ht_window *window, UINT type, bool minimized, bool isfile, ht_layer_streamfile *layer);
			
			ht_window *get_window_by_number(UINT number);
			ht_window *get_window_by_type(UINT type);
			UINT get_window_number(ht_window *window);
			UINT get_window_listindex(ht_window *window);
			
			int	popup_view_list_dump(ht_view *view, ht_text_listbox *listbox, ht_list *structure, int depth, int *currenti, ht_view *currentv);
/* overwritten */
	virtual	char *defaultpalette();
	virtual	char *defaultpaletteclass();
public:
	ht_view *menu;

			void init(bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	int focus(ht_view *view);
	virtual	char *func(UINT i, bool execute);
	virtual	void handlemsg(htmsg *msg);
	virtual	int load(ht_object_stream *f);
	virtual   OBJECT_ID object_id();
	virtual	int run(bool modal);
	virtual	void store(ht_object_stream *f);
/* new */
			bool create_window_clipboard();
			bool create_window_file(char *filename, UINT mode);
			bool create_window_file_bin(char *filename);
			bool create_window_file_text(char *filename);
			bool create_window_help(char *file, char *node);
			bool create_window_log();
			bool create_window_ofm(char *url1, char *url2);
			void	delete_window(ht_window *window);
			void logl(int c, char *line);
			void logf(int c, char *lineformat, ...);
			void	popup_view_list();
			void	popup_window_list();
};

/*
 *	exports
 */

extern char *globalerror;

#define LOG_NORMAL 0
#define LOG_WARN 1
#define LOG_ERROR 2

#define LOG(a...) ((ht_app*)app)->logf(LOG_NORMAL, a);
#define LOG_EX(c, a...) ((ht_app*)app)->logf(c, a);

/*
 *	INIT
 */

bool init_app();

/*
 *	DONE
 */

void done_app();

#endif /* __HTAPP_H__ */
