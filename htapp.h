/*
 *	HT Editor
 *	htapp.h
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

#ifndef __HTAPP_H__
#define __HTAPP_H__

#include "htctrl.h"
#include "htdialog.h"
#include "htformat.h"

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
	vcp color;
	char *msg;
	ht_log_msg(vcp Color, char *Msg);
	~ht_log_msg();
};

typedef unsigned int LogColor;

class ht_log: public ht_clist {
protected:
	uint maxlinecount;

	void deletefirstline();
	void	insertline(LogColor c, char *line);
public:
			void init(compare_keys_func_ptr compare_keys = 0);
/* new */
			void log(LogColor c, char *line);
};

class ht_logviewer: public ht_viewer {
private:
	ht_log *lines;
	bool own_lines;
	int ofs, xofs;
	ht_window *window;

/* new */
	int cursor_up(int n);
	int cursor_down(int n);
	bool get_vscrollbar_pos(int *pstart, int *psize);
	void update();
public:
			void init(bounds *b, ht_window *window, ht_log *log, bool own_log);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_vstate_history_entry
 */

class ht_vstate_history_entry: public ht_data {
public:
	Object *data;
	ht_view *view;

	ht_vstate_history_entry(Object *data, ht_view *view);
	~ht_vstate_history_entry();
};

/*
 *	CLASS ht_file_window
 */

class ht_file_window: public ht_window {
protected:
	ht_list *vstate_history;
	int vstate_history_pos;
	
			void add_vstate_history(ht_vstate_history_entry *e);
public:
	ht_streamfile	*file;

			void	init(bounds *b, char *desc, uint framestyle, uint number, ht_streamfile *file);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_project
 */

class ht_project: public ht_sorted_list {
protected:
	char *filename;
public:
		   void init(char *filename);
	virtual void done();
/* overwritten */
	virtual int	load(ObjectStream &s);
	virtual ObjectID getObjectID() const;
	virtual void	store(ObjectStream &s);
/* new */
		   char *get_filename();
};

/*
 *	CLASS ht_project_item
 */

class ht_project_item: public ht_data {
protected:
	char *filename;
	char *path;
public:
		   void init(char *filename, char *path);
	virtual void done();
/* overwritten */
	virtual int	load(ObjectStream &s);
	virtual ObjectID getObjectID() const;
	virtual void	store(ObjectStream &s);
/* new */
	const char *get_filename();
	const char *get_path();
};

/*
 *	CLASS ht_project_listbox
 */

class ht_project_listbox: public ht_listbox {
protected:
	ht_project *project;
	uint colwidths[4];
	
public:
			void		init(bounds *b, ht_project *project);
			
	virtual	int		calcCount();
	virtual	void		draw();
	virtual	void *	getFirst();
	virtual	void *	getLast();
	virtual	void *	getNext(void *entry);
	virtual	void *	getPrev(void *entry);
	virtual	char *	getStr(int col, void *entry);
	virtual	void		handlemsg(htmsg *msg);
	virtual	int		numColumns();
	virtual	void *	quickfind(char *s);
	virtual	char *	quickfindCompletition(char *s);
	virtual	bool		selectEntry(void *entry);
/* new */
			char *	func(UINT i, bool execute);
			void		set_project(ht_project *project);
};

/*
 *	CLASS ht_project_window
 */

class ht_project_window: public ht_window {
protected:
	ht_project **project;
	ht_project_listbox *plb;
	char wtitle[128];
public:

			void	init(bounds *b, char *desc, uint framestyle, uint number, ht_project **project);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_app_window_entry
 */

#define AWT_LOG		0
#define AWT_CLIPBOARD	1
#define AWT_HELP		2
#define AWT_FILE		3
#define AWT_OFM		4
#define AWT_PROJECT		5
#define AWT_TERM		6

class ht_app_window_entry: public ht_data {
public:
	uint type;
	ht_window *window;
	bool minimized;
	uint number;
	bool isfile;
	ht_layer_streamfile *layer;

	ht_app_window_entry(ht_window *window, uint number, uint type, bool minimized, bool isfile, ht_layer_streamfile *layer);
	~ht_app_window_entry();
};

/*
 *	CLASS ht_app
 */

class ht_app: public ht_dialog {
protected:
	ht_sorted_list *windows;

	ht_list *syntax_lexers;

	ht_keyline *keyline;
	ht_desktop *desktop;

	ht_group *battlefield;
	
	bool exit_program;

/* new */
			ht_window *create_window_file_bin(bounds *b, ht_layer_streamfile *file, char *title, bool isfile);
			ht_window *create_window_file_text(bounds *b, ht_layer_streamfile *file, char *title, bool isfile);
			
			bool accept_close_all_windows();
			uint find_free_window_number();
			
			uint get_window_number(ht_window *window);
			uint get_window_listindex(ht_window *window);

			void get_stdbounds_file(bounds *b);
			void get_stdbounds_tool(bounds *b);
			
			int popup_view_list_dump(ht_view *view, ht_text_listbox *listbox, ht_list *structure, int depth, int *currenti, ht_view *currentv);
/* overwritten */
	virtual	char *defaultpalette();
	virtual	char *defaultpaletteclass();
public:
	ht_view *menu;
		void insert_window(ht_window *window, uint type, bool minimized, bool isfile, ht_layer_streamfile *layer);

		void init(bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	int focus(ht_view *view);
	virtual	char *func(UINT i, bool execute);
	virtual	void handlemsg(htmsg *msg);
	virtual	int load(ObjectStream &f);
	virtual ObjectID getObjectID() const;
	virtual	int run(bool modal);
	virtual	void store(ObjectStream &f);
/* new */
			ht_window *create_window_clipboard();
			ht_window *create_window_file(char *filename, uint mode, bool allow_duplicates);
			ht_window *create_window_file_bin(char *filename, bool allow_duplicates);
			ht_window *create_window_file_text(char *filename, bool allow_duplicates);
			ht_window *create_window_help(char *file, char *node);
			ht_window *create_window_log();
			ht_window *create_window_ofm(char *url1, char *url2);
			ht_window *create_window_project();
			ht_window *create_window_term(const char *cmd);
			void delete_window(ht_window *window);
			ht_window *get_window_by_filename(char *filename);
			ht_window *get_window_by_number(UINT number);
			ht_window *get_window_by_type(UINT type);
			ht_view *popup_view_list(char *dialog_title);
			ht_window *popup_window_list(char *dialog_title);
			void project_opencreate(char *filename);
};

extern ht_log *loglines;

/*
 *	INIT
 */

bool init_app();

/*
 *	DONE
 */

void done_app();

#endif /* __HTAPP_H__ */
