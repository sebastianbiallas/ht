/*
 *	HT Editor
 *	htapp.cc
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

#include "analy.h"
#include "cmds.h"
#include "mfile.h"
#include "htapp.h"
#include "htatom.h"
#include "htcfg.h"
#include "htclipboard.h"
#include "htdialog.h"
#include "hteval.h"
#include "hthist.h"
#include "htidle.h"
#include "htinfo.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htmenu.h"
#include "htpal.h"
#include "htsearch.h"
#include "htstring.h"
#include "httree.h"
#include "htvfs.h"
#include "infoview.h"
#include "stream.h"
#include "textedit.h"
#include "textfile.h"
#include "tools.h"

#include "formats.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

extern "C" {
#include "regex.h"
}

#define ATOM_HT_APP		MAGICD("APP\0")

char _globalerror[1024];
char *globalerror=_globalerror;


/*
 *	CLASS ht_help_window
 */

class ht_help_window : public ht_window {
public:
/* overwritten */
	virtual void handlemsg(htmsg *msg);
};

void ht_help_window::handlemsg(htmsg *msg)
{
	ht_window::handlemsg(msg);
	if (msg->msg == msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Escape: {
				htmsg m;
				m.msg = cmd_window_close;
				((ht_app*)app)->queuemsg(app, &m);
				clearmsg(msg);
				return;
			}				
		}				
	}
}


// FIXME: move to tools / htsys
char *fn_suf_ptr(char *fn)
{
	char *p = fn ? strrchr(fn, '.') : NULL;
	return p ? p+1 : NULL;
}

bool file_new_dialog(UINT *mode)
{
	bounds b, c;
	
	app->getbounds(&b);

	b.x = (b.w - 40) / 2,
	b.y = (b.h - 8) / 2;
	b.w = 40;
	b.h = 8;
	
	ht_dialog *d=new ht_dialog();
	d->init(&b, "create new file", FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
	
	b.x=0;
	b.y=0;
		
/* mode (input) */
	c=b;
	c.x=0;
	c.y=1;
	c.w=b.w-2-c.x;
	c.h=b.h-2-c.y;

	ht_text_listbox *mode_input=new ht_text_listbox();
	mode_input->init(&c);
	
	mode_input->insert_str(FOM_TEXT, "text");
	mode_input->insert_str(FOM_BIN, "binary");
	mode_input->update();

	d->insert(mode_input);
	
/* mode (text) */
	c=b;
	c.x=0;
	c.y=0;
	c.w=12;
	c.h=1;

	ht_label *mode_text=new ht_label();
	mode_text->init(&c, "choose ~type", mode_input);

	d->insert(mode_text);

	bool retval = false;
	if (d->run(false)) {
		struct {
			ht_listbox_data type;
		} data;

		d->databuf_get(&data);

		*mode = data.type.cursor_id;
		
		retval = true;
	}

	d->done();
	delete d;
	return retval;
}

bool file_open_dialog(char **name, UINT *mode)
{
	bounds b, c;
	
	app->getbounds(&b);

	b.x = (b.w - 60) / 2,
	b.y = (b.h - 8) / 2;
	b.w = 60;
	b.h = 8;
	
	ht_dialog *d=new ht_dialog();
	d->init(&b, "open file", FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
	
	b.x=0;
	b.y=0;
		
/* name (input) */
	c=b;
	c.x=10;
	c.y=1;
	c.w-=4+c.x;
	c.h=1;

	ht_list *hist=(ht_list*)find_atom(HISTATOM_FILE);
	
	ht_strinputfield *name_input=new ht_strinputfield();
	name_input->init(&c, 128, hist);

	d->insert(name_input);
	
/* name (text) */
	c=b;
	c.x=1;
	c.y=1;
	c.w=9;
	c.h=1;

	ht_label *name_text=new ht_label();
	name_text->init(&c, "~filename", name_input);

	d->insert(name_text);

/* mode (input) */
	c=b;
	c.x=10;
	c.y=3;
	c.w=12;
	c.h=1;

	ht_listpopup *mode_input=new ht_listpopup();
	mode_input->init(&c);
	
	mode_input->insertstring("autodetect");
	mode_input->insertstring("binary");
	mode_input->insertstring("text");

	d->insert(mode_input);
	
/* mode (text) */
	c=b;
	c.x=1;
	c.y=3;
	c.w=9;
	c.h=1;

	ht_label *mode_text=new ht_label();
	mode_text->init(&c, "~mode", mode_input);

	d->insert(mode_text);
	
//
	if (d->run(false)) {
		struct {
			ht_strinputfield_data name;
			ht_listpopup_data mode;
		} data;

		d->databuf_get(&data);

		switch (data.mode.cursor_id) {
			case 0: *mode=FOM_AUTO; break;
			case 1: *mode=FOM_BIN; break;
			case 2: *mode=FOM_TEXT; break;
		}

		*name=(char*)malloc(data.name.textlen+1);
		bin2str(*name, data.name.text, data.name.textlen);
		
		if (hist) insert_history_entry(hist, *name, 0);
		
		d->done();
		delete d;
		return true;
	}
	
	d->done();
	delete d;
	return false;
}

UINT autodetect_file_open_mode(char *filename)
{
#define AUTODETECT_SIZE	128
	FILE *f=fopen(filename, "rb");
	UINT r=FOM_BIN;
	if (f) {
		byte buf[AUTODETECT_SIZE];
		int c=fread(buf, 1, AUTODETECT_SIZE, f);
		bool is_bin=false;
		UINT prob_bin_chars=0;
		for (int i=0; i<c; i++) {
			if (buf[i]==0) {
				is_bin=true;
				break;
			} else if (buf[i]<32) {
				prob_bin_chars++;
			} else if (buf[i]>0xa9) {
				prob_bin_chars++;
			}
		}
		if (c) {
			if (prob_bin_chars*100/c>=50) is_bin=true;
		} else is_bin=true;
		if (!is_bin) r=FOM_TEXT;
		fclose(f);
		return r;
	}
	return FOM_BIN;
}

int file_window_load_fcfg_func(ht_object_stream *f, void *context)
{
	ht_file_window *w=(ht_file_window*)context;

	pstat_t p;
	dword oldsize=f->get_int_dec(4, "filesize");
	dword oldtime=f->get_int_dec(4, "filetime");
	dword newsize=w->file->get_size();
	w->file->pstat(&p);
	dword newtime=(p.caps & pstat_mtime) ? p.mtime : 0;
	
	if (f->get_error()) return f->get_error();
	
	if ((newsize != oldsize) || (newtime != oldtime)) {
		char s_oldtime[64], s_newtime[64];
		struct tm *t;

		t=gmtime((time_t*)&newtime);
		strftime(s_newtime, sizeof s_newtime, "%X %d %h %Y", t);

		t=gmtime((time_t*)&oldtime);
		strftime(s_oldtime, sizeof s_oldtime, "%X %d %h %Y", t);

		if (confirmbox_c("\ecconfig file applies to different version of file '%s'.\n\n\elcurrent: %10d %s\n\elold:     %10d %s\n\n\ecload config file ?", w->file->get_desc(), newsize, s_newtime, oldsize, s_oldtime) != button_yes) {
			return f->get_error();
		}
	}

	analyser *a=(analyser*)f->get_object("analyser");

	if (f->get_error()) return f->get_error();

	htmsg m;
	m.msg=msg_set_analyser;
	m.type=mt_broadcast;
	m.data1.ptr=a;
	w->sendmsg(&m);

	return f->get_error();
}

void file_window_store_fcfg_func(ht_object_stream *f, void *context)
{
	ht_file_window *w=(ht_file_window*)context;
	htmsg m;
	m.msg=msg_get_analyser;
	m.type=mt_broadcast;
	m.data1.ptr=NULL;
	w->sendmsg(&m);
	if ((m.msg==msg_retval) && (m.data1.ptr)) {
		pstat_t s;
		w->file->pstat(&s);
		dword t = (s.caps & pstat_mtime) ? s.mtime : 0;;
		f->put_int_dec(w->file->get_size(), 4, "filesize");
		f->put_int_dec(t, 4, "filetime");
		
		analyser *a=(analyser*)m.data1.ptr;
		f->put_object(a, "analyser");
	}
}

/*
 *   app_stream_error_func()
 */

int app_stream_error_func(ht_stream *stream)
{
	int err=stream->get_error();
	char *name = stream->get_desc();
	if (err & STERR_SYSTEM) {
		err=err&0xffff;
		switch (err) {
			case 4: {	/* EACCES*/
#ifdef DJGPP
				struct stat sbuf;

				stat(name, &sbuf);

				if (!(sbuf.st_mode & S_IWUSR)) {
					if (msgbox(btmask_yes | btmask_no, "title", 1, align_center, "%s: stream error (Permission denied), seems to be a (DOS) read-only file. Change attribute ?", name)==button_yes) {
						if (chmod(name, S_IRUSR | S_IWUSR)) {
							errorbox_modal("%s: error (%04x) changing attribute", name, errno & 0xffff);
						} else {
							stat(name, &sbuf);

							if (!(sbuf.st_mode & S_IWUSR)) {
								errorbox_modal("%s: error (%04x) changing attribute", name, errno & 0xffff);
							} else {
								return SERR_RETRY;
							}
						}
					}
				}

				break;
#endif
			}
			default:
				errorbox_modal("%s: stream error %04x: %s", name, err, strerror(err));
		}
	} else {
		err=err&0xffff;
		errorbox_modal("%s: internal stream error %04x", name, err);
	}
	return SERR_FAIL;
}

/*
 *   app_out_of_memory_proc()
 */
void *app_memory_reserve=0;

int app_out_of_memory_proc(int size)
{
	if (app_memory_reserve) {
		free(app_memory_reserve);
		app_memory_reserve = 0;
		warnbox_modal("the memory is getting low...");
		return OUT_OF_MEMORY_RETRY;
	} else {
		return OUT_OF_MEMORY_FAIL;
	}
}

/*
 *	CLASS ht_status
 */
void ht_status::init(bounds *b)
{
	ht_view::init(b, VO_TRANSPARENT_CHARS, 0);
	VIEW_DEBUG_NAME("ht_status");
	idle_count = 0;
	analy_ani = 0;
	clear_len = 0;
	format = get_config_string("misc/statusline");
	
	if (!format) {
		format = strdup(STATUS_DEFAULT_FORMAT);
	}
	
	render();
	register_idle_object(this);
}

void ht_status::done()
{
	unregister_idle_object(this);
	if (format) free(format);
	ht_view::done();
}

char *ht_status::defaultpalette()
{
	return palkey_generic_menu_default;
}

void ht_status::draw()
{
	fill(size.w-clear_len, 0, clear_len, 1, getcolor(palidx_generic_text_focused), ' ');
	int len=strlen(workbuf);
	clear_len = len;
	buf_print(size.w-len, 0, getcolor(palidx_generic_text_focused), workbuf);
}

void ht_status::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_config_changed:
			if (format) free(format);
			format = get_config_string("misc/statusline");
			break;
	}
	ht_view::handlemsg(msg);
}

bool ht_status::idle()
{
	if (idle_count % 100 == 0) {
		char *oldstatus=ht_strdup(workbuf);
		render();
		if (strcmp(oldstatus, workbuf)) {
			dirtyview();
			redraw();
			screen->show();
		}
		free(oldstatus);
		
		analy_ani++;
		analy_ani &= 7;
	}
	idle_count++;
	return false;
}

void ht_status::render()
{
	char *f = format;
	char *buf = workbuf;
	if (f)
	while (*f) {
		if (*f==STATUS_ESCAPE) {
			switch (*(++f)) {
				case STATUS_ESCAPE:
					*(buf++) = STATUS_ESCAPE;
					break;
				case STATUS_ANALY_ACTIVE:
					if (some_analyser_active) {
						char *analysers[] = {"Analy", "aNaly", "anAly", "anaLy", "analY", "anaLy", "anAly", "aNaly"};
						strcpy(buf, analysers[analy_ani]);
						buf += 5;
					}
					break;
				case STATUS_ANALY_LINES:
					if (some_analyser_active) {
						buf += sprintf(buf, "(%d)", num_ops_parsed);
					}
					break;
				case STATUS_TIME: {
					time_t Time;
					time(&Time);
					tm *t=localtime(&Time);
					buf += sprintf(buf, "%02d:%02d", t->tm_hour, t->tm_min);
					break;
				}
				case STATUS_DATE: {
					time_t Time;
					time(&Time);
					tm *t=localtime(&Time);
					buf += sprintf(buf, "%02d.%02d.%04d", t->tm_mday, t->tm_mon+1, t->tm_year+1900);
					break;
				}
			}
		} else {
			*(buf++) = *f;
		}
		f++;
	}
	*buf = 0;
}


/*
 *	CLASS ht_keyline
 */

void ht_keyline::init(bounds *b)
{
	ht_view::init(b, 0, 0);
	VIEW_DEBUG_NAME("ht_keyline");
}

void ht_keyline::done()
{
	ht_view::done();
}

char *ht_keyline::defaultpalette()
{
	return palkey_generic_keys_default;
}

void ht_keyline::draw()
{
	clear(getcolor(palidx_generic_text_disabled));
	int x=0;
	for (int i=1; i<=10; i++) {
		htmsg msg;
		msg.type=mt_empty;
		msg.msg=msg_funcquery;
		msg.data1.integer=i;
		baseview->sendmsg(&msg);
		buf_printchar(x, 0, getcolor(palidx_generic_text_shortcut), '0'+i%10);
		if (msg.msg==msg_retval) {
			char *s=msg.data1.str;
			if (s) {
				if (s[0]=='~') {
					buf_printf(x+1, 0, getcolor(palidx_generic_text_disabled), s+1);
				} else {
					for (int j=0; j<size.w/10-1; j++) {
						buf_printf(x+j+1, 0, getcolor(palidx_generic_text_focused), " ");
					}
					buf_printf(x+1, 0, getcolor(palidx_generic_text_focused), s);
				}
			}
		}
		x+=size.w/10;
	}
}

void ht_keyline::handlemsg(htmsg *msg)
{
	ht_view::handlemsg(msg);
}

/*
 *	CLASS ht_desktop
 */

void ht_desktop::init(bounds *b)
{
	ht_view::init(b, VO_OWNBUFFER, 0);
	VIEW_DEBUG_NAME("ht_desktop");
}

void ht_desktop::done()
{
	ht_view::done();
}

char *ht_desktop::defaultpalette()
{
	return palkey_generic_desktop_default;
}

void ht_desktop::draw()
{
	fill(0, 0, size.w, size.h, getcolor(palidx_generic_body), CHAR_FILLED_M);
}

/*
 *	CLASS ht_log_msg
 */
 
ht_log_msg::ht_log_msg(char *Msg, int Color)
{
	msg = ht_strdup(Msg);
	color = Color;
}

ht_log_msg::~ht_log_msg()
{
	free(msg);
}

/*
 *	CLASS ht_logviewer
 */

void ht_logviewer::init(bounds *b, ht_clist *l, ht_window *w)
{
	ht_viewer::init(b, "log", 0);
	VIEW_DEBUG_NAME("ht_logviewer");
	log=l;
	ofs=0;
	xofs=0;
	window=w;
	update();
}

void ht_logviewer::done()
{
	ht_viewer::done();
}

int ht_logviewer::cursor_up(int n)
{
	ofs-=n;
	if (ofs<0) ofs=0;
	return n;
}

int ht_logviewer::cursor_down(int n)
{
	int c=log->count();
	if (c>=size.h) {
		ofs+=n;
		if (ofs>c-size.h) ofs=c-size.h;
	}
	return n;
}

void ht_logviewer::draw()
{
	clear(getcolor(palidx_generic_body));
	int c=log->count();
	for (int i=0; i<size.h; i++) {
		if (i+ofs>=c) break;
		ht_log_msg *msg=(ht_log_msg*)log->get(i+ofs);
		int l=strlen(msg->msg);
		if (xofs<l) buf_print(0, i, /*getcolor(palidx_generic_body)*/msg->color, msg->msg+xofs);
	}
}

bool ht_logviewer::get_hscrollbar_pos(int *pstart, int *psize)
{
	return scrollbar_pos(ofs, size.h, log->count(), pstart, psize);
}


void ht_logviewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_get_scrollinfo:
			switch (msg->data1.integer) {
/*				case gsi_pindicator: {
					get_pindicator_str((char*)msg->data2.ptr);
					break;
				}*/
				case gsi_hscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_hscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					break;
				}
/*				case gsi_vscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					get_vscrollbar_pos(&p->pstart, &p->psize);
					break;
				}*/
			}
			clearmsg(msg);
			return;
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_Up:
					cursor_up(1);
					update();
					clearmsg(msg);
					return;
				case K_Down:
					cursor_down(1);
					update();
					clearmsg(msg);
					return;
				case K_PageUp:
					cursor_up(size.h);
					update();
					clearmsg(msg);
					return;
				case K_PageDown:
					cursor_down(size.h);
					update();
					clearmsg(msg);
					return;
				case K_Right: case K_Control_Right:
					xofs+=2;
					update();
					clearmsg(msg);
					return;
				case K_Left: case K_Control_Left:
					if (xofs-2>=0) xofs-=2;
					update();
					clearmsg(msg);
					return;
				case K_Control_PageUp:
					ofs=0;
					update();
					clearmsg(msg);
					return;
				case K_Control_PageDown:
					ofs=log->count()-size.h;
					if (ofs<0) ofs=0;
					update();
					clearmsg(msg);
					return;
			}
			break;
	}
	ht_viewer::handlemsg(msg);
}

void ht_logviewer::update()
{
	dirtyview();
}

/*
 *	CLASS ht_app_window_entry
 */

#define AWT_LOG		0
#define AWT_CLIPBOARD	1
#define AWT_HELP		2
#define AWT_FILE		3
#define AWT_OFM		4

ht_app_window_entry::ht_app_window_entry(ht_window *w, UINT n, UINT t, bool m, bool isf, ht_layer_streamfile *l)
{
	window=w;
	number=n;
	type=t;
	minimized=m;
	isfile=isf;
	layer=l;
}

ht_app_window_entry::~ht_app_window_entry()
{
}

int compare_keys_app_window_entry(ht_data *key_a, ht_data *key_b)
{
	UINT a=((ht_app_window_entry*)key_a)->number;
	UINT b=((ht_app_window_entry*)key_b)->number;
	if (a>b) return 1; else if (a<b) return -1;
	return 0;
}

/*
 *	CLASS ht_app
 */

/*debug*/
//#define DRAW_TIMINGS
//#define NO_AVG
#define AVG_TIMINGS 10
int timings[AVG_TIMINGS];
int cur_timing=0, max_timing=0;
int h0;
/**/

void ht_app::init(bounds *pq)
{
	ht_dialog::init(pq, 0, 0);
	menu = NULL;
	setframe(NULL);
	VIEW_DEBUG_NAME("ht_app");
	exit_program=0;
	focused=1;
	bounds b;

	windows=new ht_sorted_list();
	windows->init(compare_keys_app_window_entry);
	
	log=new ht_clist();
	log->init();

	syntax_lexers = new ht_clist();
	((ht_clist*)syntax_lexers)->init();

	ht_c_syntax_lexer *c_lexer=new ht_c_syntax_lexer();
	c_lexer->init();

	syntax_lexers->insert(c_lexer);

	ht_html_syntax_lexer *html_lexer=new ht_html_syntax_lexer();
	html_lexer->init();

	syntax_lexers->insert(html_lexer);
	
/* init timer */
	h0=new_timer();
/* create menu */
	getbounds(&b);
	b.x=0;
	b.y=0;
	b.h=1;
	ht_menu *m=new ht_menu();
	m->init(&b);

	ht_static_context_menu *file=new ht_static_context_menu();
	file->init("~File");
	file->insert_entry("~New...", 0, cmd_file_new, 0, 1);
	file->insert_entry("~Open...", 0, cmd_file_open, 0, 1);
	file->insert_entry("~Save", 0, cmd_file_save, 0, 1);
	file->insert_entry("Save ~As...", 0, cmd_file_saveas, 0, 1);
	file->insert_separator();
	file->insert_entry("~Quit", "F10", cmd_quit, 0, 1);
	m->insert_menu(file);

	ht_static_context_menu *edit=new ht_static_context_menu();
	edit->init("~Edit");
	edit->insert_entry("Cu~t", "Shift+Del", cmd_edit_cut, 0, 1);
	edit->insert_entry("~Delete", "Ctrl+Del", cmd_edit_delete, 0, 1);
	edit->insert_entry("~Copy", "Ctrl+Ins", cmd_edit_copy, 0, 1);
	edit->insert_entry("~Paste", "Shift+Ins", cmd_edit_paste, 0, 1);
	edit->insert_entry("~Show clipboard", 0, cmd_edit_show_clipboard, 0, 1);
	edit->insert_entry("C~lear clipboard", 0, cmd_edit_clear_clipboard, 0, 1);
	edit->insert_separator();
	edit->insert_entry("Copy ~from file...", 0, cmd_edit_copy_from_file, 0, 1);
	edit->insert_entry("Paste ~into file...", 0, cmd_edit_paste_into_file, 0, 1);
	edit->insert_separator();
	edit->insert_entry("~Evaluate...", 0, cmd_popup_dialog_eval, 0, 1);
	m->insert_menu(edit);

/*	ht_static_context_menu *options=new ht_static_context_menu();
	options->init("~Options");
	options->insert_entry("~Palette", NULL, cmd_options_palette, 0, 1);
	m->insert_menu(options);*/

	ht_static_context_menu *windows=new ht_static_context_menu();
	windows->init("~Windows");
	windows->insert_entry("~Size/Move", "Alt+F5", cmd_window_resizemove, K_Alt_F5, 1);
	windows->insert_entry("~Close", "Alt+F3", cmd_window_close, K_Alt_F3, 1);
	windows->insert_entry("~List", "Alt+0", cmd_popup_dialog_wlist, K_Alt_0, 1);
	windows->insert_separator();
	windows->insert_entry("Lo~g window", NULL, cmd_popup_window_log, 0, 1);
	windows->insert_entry("~Options", NULL, cmd_popup_window_options, 0, 1);
	m->insert_menu(windows);

	ht_static_context_menu *help=new ht_static_context_menu();
	help->init("~Help");
	help->insert_entry("~About "ht_name, "", cmd_about, 0, 1);
	help->insert_separator();
	help->insert_entry("~Help contents", "F1", cmd_popup_window_help, 0, 1);
	m->insert_menu(help);

	m->insert_local_menu();

	menu=m;
	insert(menu);
	
/* create status */
	/* the status should have the same bounds as the menu */
	ht_status *status = new ht_status();
	status->init(&b);
	status->setpalette(menu->getpalette());
	insert(status);
	
/* create desktop */
	getbounds(&b);
	b.x=0;
	b.y=1;
	b.h-=2;
	desktop=new ht_desktop();
	desktop->init(&b);
	insert(desktop);
/* create keyline */
	getbounds(&b);
	b.x=0;
	b.y=b.h-1;
	b.h=1;
	keyline=new ht_keyline();
	keyline->init(&b);
	insert(keyline);
/* create battlefield */
	getbounds(&b);
	b.x=0;
	b.y=1;
	b.h-=2;

	battlefield=new ht_group();
	battlefield->init(&b, VO_TRANSPARENT_CHARS, "battlefield");
	insert(battlefield);

	create_window_log();
}

void ht_app::done()
{
	delete_timer(h0);

	syntax_lexers->destroy();
	delete syntax_lexers;

	log->destroy();
	delete log;
	
	if (windows) {
		windows->destroy();
		delete windows;
	}

	ht_dialog::done();
}

bool ht_app::accept_close_all_windows()
{
	UINT wc=windows->count();
	ht_app_window_entry *e;
	for (UINT i=0; i<wc; i++) {
		e=(ht_app_window_entry*)windows->get(i);
		htmsg m;
		m.msg=msg_accept_close;
		m.type=mt_empty;
		e->window->sendmsg(&m);
		if (m.msg!=msg_accept_close) return false;
	}
	return true;
}

bool ht_app::create_window_log()
{
	ht_window *w=get_window_by_type(AWT_LOG);
	if (w) {
		focus(w);
	} else {
		bounds b;
		battlefield->getbounds(&b);
		b.x=0;
		b.y=0;
		ht_window *logwindow=new ht_window();
		logwindow->init(&b, "log window", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);
		
		bounds k=b;
		k.x=b.w-2;
		k.y=0;
		k.w=1;
		k.h-=2;
		ht_scrollbar *hs=new ht_scrollbar();
		hs->init(&k, &logwindow->pal, false);

		logwindow->sethscrollbar(hs);

		b.x=0;
		b.y=0;
		b.w-=2;
		b.h-=2;
		ht_logviewer *logviewer=new ht_logviewer();
		logviewer->init(&b, log, logwindow);
		logwindow->insert(logviewer);
		
		insert_window(logwindow, AWT_LOG, 0, false, NULL);
	}
	return true;
}

bool ht_app::create_window_clipboard()
{
	ht_window *w=get_window_by_type(AWT_CLIPBOARD);
	if (w) {
		focus(w);
		return true;
	} else {
		bounds b;
		battlefield->getbounds(&b);
		b.x=0;
		b.y=0;
/*		ht_file_window *window=new ht_file_window();
		window->init(&b, "clipboard", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, clipboard);*/
		ht_window *window=new ht_window();
		window->init(&b, "clipboard", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);
		
/*	     bounds k=b;
		k.x=b.w-2;
		k.y=0;
		k.w=1;
		k.h-=2;
		ht_scrollbar *hs=new ht_scrollbar();
		hs->init(&k, &window->pal, false);

		window->sethscrollbar(hs);*/

		b.x=0;
		b.y=0;
		b.w-=2;
		b.h-=2;
		ht_clipboard_viewer *v=new ht_clipboard_viewer();
		v->init(&b, "clipboard", VC_EDIT | VC_GOTO | VC_SEARCH, clipboard, 0);

		window->insert(v);
// FIXME: needs wrapper (layer)
//		insert_window(window, AWT_CLIPBOARD, 0, false, clipboard);
		insert_window(window, AWT_CLIPBOARD, 0, false, NULL);
	}
	return false;
}
			
bool ht_app::create_window_file(char *filename, UINT mode)
{
	if (mode==FOM_AUTO) mode=autodetect_file_open_mode(filename);
	switch (mode) {
		case FOM_BIN: return create_window_file_bin(filename);
		case FOM_TEXT: return create_window_file_text(filename);
	}
	return false;
}

bool ht_app::create_window_file_bin(char *filename)
{
	bounds b;
	battlefield->getbounds(&b);
	b.x=0;
	b.y=0;
	int e;
	char fullfilename[FILENAME_MAX];
	if ((e=sys_canonicalize(filename, fullfilename))) {
		LOG_EX(LOG_ERROR, "error loading file %s: %s", fullfilename, strerror(e & ~STERR_SYSTEM));
		return false;
	}
	ht_file *emfile=new ht_file();
	emfile->init(fullfilename, FAM_READ);
	
	ht_streamfile_modifier *mfile=new ht_streamfile_modifier();
	mfile->init(emfile, true, 8*1024);

	ht_layer_streamfile *file=new ht_layer_streamfile();
	file->init(mfile, true);
			 
	if ((e=file->get_error())) {
		LOG_EX(LOG_ERROR, "error loading file %s: %s", fullfilename, strerror(e & ~STERR_SYSTEM));
		return false;
	}
	LOG("loading binary file %s...", fullfilename);
	file->set_error_func(app_stream_error_func);

	return create_window_file_bin(&b, file, fullfilename, true);
}

bool ht_app::create_window_file_bin(bounds *b, ht_layer_streamfile *file, char *title, bool isfile)
{
	ht_file_window *window=new ht_file_window();
	window->init(b, title, FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, file);

	bounds k=*b;
	k.x=b->w-2;
	k.y=0;
	k.w=1;
	k.h-=2;
	ht_scrollbar *hs=new ht_scrollbar();
	hs->init(&k, &window->pal, false);

	window->sethscrollbar(hs);

	k=*b;
	k.x=3;
	k.y=k.h-2;
	k.w-=7;
	k.h=1;
	ht_statictext *ind=new ht_statictext();
	ind->init(&k, NULL, align_left, false, true);
	ind->disable_buffering();
	ind->growmode=GM_BOTTOM | GM_HDEFORM;

	window->setpindicator(ind);

	k=*b;
	k.x=0;
	k.y=0;
	k.w-=2;
	k.h-=2;
	ht_format_group *format_group=new ht_format_group();
	format_group->init(&k, VO_SELECTABLE | VO_RESIZE, VIEWERGROUP_NAME, file, true, true, format_viewer_ifs, NULL);

	window->insert(format_group);
/**/
	if (isfile) {
		char cfgfilename[FILENAME_MAX];
		strcpy(cfgfilename, title);
		strcat(cfgfilename, ".htcfg");

		int einfo;
//		LOG("%s: loading config file...", cfgfilename);
		loadstore_result lsr=load_fileconfig(cfgfilename, file_window_load_fcfg_func, window, &einfo);
		if (lsr==LS_ERROR_CORRUPTED) {
			LOG_EX(LOG_ERROR, "%s: error in line %d", cfgfilename, einfo);
			errorbox("%s: error in line %d",cfgfilename,  einfo);
		} else if (lsr==LS_ERROR_NOT_FOUND) {
		} else if (lsr!=LS_OK) {
			LOG_EX(LOG_ERROR, "%s: some error", cfgfilename);
			errorbox("%s: some error", cfgfilename);
		} else {
			LOG("%s: ok", cfgfilename);
		}
	}
/**/
	if (isfile) LOG("%s: done.", title);

	htmsg m;
	m.msg = msg_postinit;
	m.type = mt_broadcast;
	window->sendmsg(&m);

	insert_window(window, AWT_FILE, 0, isfile, file);
	return true;
}

bool ht_app::create_window_file_text(char *filename)
{
	bounds b, c;
	battlefield->getbounds(&c);
	c.x=0;
	c.y=0;
	b=c;
	int e;
	char fullfilename[FILENAME_MAX];
	if ((e=sys_canonicalize(filename, fullfilename))) {
		LOG_EX(LOG_ERROR, "error loading file %s: %s", fullfilename, strerror(e & ~STERR_SYSTEM));
		return false;
	}

	ht_file *emfile=new ht_file();
	emfile->init(fullfilename, FAM_READ);
	
	ht_ltextfile *tfile=new ht_ltextfile();
	tfile->init(emfile, true, NULL);

	ht_layer_textfile *file=new ht_layer_textfile();
	file->init(tfile, true);

	if ((e=file->get_error())) {
		LOG_EX(LOG_ERROR, "error loading file %s: %s", fullfilename, strerror(e & ~STERR_SYSTEM));
		return false;
	}

	LOG("loading text file %s...", fullfilename);
	file->set_error_func(app_stream_error_func);

	return create_window_file_text(&b, file, fullfilename, true);
}

bool ht_app::create_window_file_text(bounds *c, ht_layer_streamfile *f, char *title, bool isfile)
{
	bounds b=*c;

	ht_layer_textfile *file = (ht_layer_textfile *)f;
	
	ht_file_window *window=new ht_file_window();
	window->init(&b, title, FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, file);

	b.x=0;
	b.y=0;
	b.w-=2;
	b.h-=2;
	ht_text_editor *text_editor=new ht_text_editor();
	text_editor->init(&b, true, file, syntax_lexers, TEXTEDITOPT_INPUTTABS|TEXTEDITOPT_UNDO);

	char *fn_suf = fn_suf_ptr(file->get_filename());

	if (fn_suf) {
		if ((ht_stricmp(fn_suf, "c") == 0) || (ht_stricmp(fn_suf, "cc") == 0)
		|| (ht_stricmp(fn_suf, "cpp") == 0)
		|| (ht_stricmp(fn_suf, "h") == 0) || (ht_stricmp(fn_suf, "hpp") == 0)) {
			text_editor->set_lexer((ht_syntax_lexer*)syntax_lexers->get(0), false);
		}
#ifdef HT_HTML_SYNTAX_LEXER
		if (ht_stricmp(fn_suf, "htm") == 0 || ht_stricmp(fn_suf, "html") == 0) {
			text_editor->set_lexer((ht_syntax_lexer*)syntax_lexers->get(1), false);
		}
#endif
	}

	bounds k=*c;
	k.x=k.w-2;
	k.y=0;
	k.w=1;
	k.h-=2;
	ht_scrollbar *hs=new ht_scrollbar();
	hs->init(&k, &window->pal, false);

	window->sethscrollbar(hs);

	k=*c;
	k.x=3;
	k.y=k.h-2;
	k.w-=7;
	k.h=1;
	
	ht_statictext *ind=new ht_statictext();
	ind->init(&k, NULL, align_left, false, true);
	ind->disable_buffering();
	ind->growmode=GM_BOTTOM | GM_HDEFORM;

	window->setpindicator(ind);

	window->insert(text_editor);
	if (isfile) LOG("%s: done.", title);

	insert_window(window, AWT_FILE, 0, isfile, file);

	return true;
}

bool ht_app::create_window_help(char *file, char *node)
{
	ht_window *w=get_window_by_type(AWT_HELP);
	if (w) {
		focus(w);
		return true;
	} else {
		bounds b, c;
		battlefield->getbounds(&c);
		b.w=c.w*7/8;
		b.h=c.h*7/8;
		b.x=(c.w-b.w)/2;
		b.y=(c.h-b.h)/2;
		bounds k = b;

		ht_help_window *window=new ht_help_window();
		window->init(&b, "help", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);

		b.x=0;
		b.y=0;
		b.w-=2;
		b.h-=2;
		c=b;

		b=c;
		b.x+=b.w;
		b.w=1;
		ht_scrollbar *scrollbar=new ht_scrollbar();
		scrollbar->init(&b, &window->pal, false);
		scrollbar->enable();
		window->sethscrollbar(scrollbar);

		k.x=3;
		k.y=k.h-2;
		k.w-=7;
		k.h=1;
		ht_statictext *ind=new ht_statictext();
		ind->init(&k, NULL, align_left, false, true);
		ind->disable_buffering();
		ind->growmode=GM_BOTTOM | GM_HDEFORM;

		window->setpindicator(ind);

		b=c;
		ht_info_viewer *infoviewer=new ht_info_viewer();
		infoviewer->init(&b);
		window->insert(infoviewer);

		if (infoviewer->gotonode(file, node)) {
			insert_window(window, AWT_HELP, 0, false, NULL);
			
			window->setpalette(palkey_generic_cyan);

			return true;
		}
		errorbox("help topic '(%s)%s' not found", file, node);
		window->done();
		delete window;
	}
	return false;
}

ht_list *build_vfs_list()
{
/* build vfs list */
	ht_clist *vfslist=new ht_clist();
	vfslist->init();

/* file_vfs */
	ht_file_vfs *file_vfs=new ht_file_vfs();
	file_vfs->init();

/* reg_vfs */
	ht_reg_vfs *reg_vfs=new ht_reg_vfs();
	reg_vfs->init(registry);

	vfslist->insert(file_vfs);
	vfslist->insert(reg_vfs);
/**/
	return vfslist;
}

ht_view *create_ofm_single(bounds *c, char *url, ht_vfs_viewer **x)
{
	bounds b=*c;
	b.h-=2;
	ht_group *g=new ht_group();
	g->init(&b, VO_SELECTABLE, 0);

	bounds d=b;
	
	b.x=0;
	b.y=0;
	b.h=1;
	ht_vfs_viewer_status *vst=new ht_vfs_viewer_status();
	vst->init(&b);
	   
	d.x=0;
	d.y=1;
	d.h--;
	ht_vfs_viewer *v=new ht_vfs_viewer();
	v->init(&d, "vfs viewer", 0, 0, 0, vst);
	   
	g->insert(v);

	g->insert(vst);

	ht_list *vfslist=build_vfs_list();

	ht_vfs_sub *vs=new ht_vfs_sub();
	vs->init(vfslist, url);

	v->insertsub(vs);

	v->sendmsg(msg_complete_init, 0);

	*x=v;
	return g;
}

bool ht_app::create_window_ofm(char *url1, char *url2)
{
	bounds b;
	battlefield->getbounds(&b);
	b.x=0;
	b.y=0;
	ht_window *window=new ht_window();
	window->init(&b, "file manager", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);

	bounds b1, b2, b3;

	ht_vfs_viewer *v1, *v2=NULL;

	b.w-=2;

	b1=b;
	b1.w/=2;
	b1.w--;
	window->insert(create_ofm_single(&b1, url1, &v1));

	if (url2) {
		b2=b;
		b2.w/=2;
		b2.x=b2.w-1;
		b2.w=1;
		b2.h-=2;
		ht_vbar *x=new ht_vbar();
		x->init(&b2, 0, 0);
		window->insert(x);

		b3=b;
		b3.w/=2;
		b3.x=b3.w;
		window->insert(create_ofm_single(&b3, url2, &v2));
	}

	if (v2) {
		v1->set_assoc_vfs_viewer(v2);
		v2->set_assoc_vfs_viewer(v1);
	}

	insert_window(window, AWT_OFM, 0, false, NULL);
	return true;
}

char *ht_app::defaultpalette()
{
	return NULL;
}

char *ht_app::defaultpaletteclass()
{
	return NULL;
}

int analy_id = 0;
void ht_app::draw()
{
/* show draw timings */
#ifdef DRAW_TIMING
	int xyz=get_timer_1024tick(h0);
	buf->printf(17, 1, 7, "cur: %d (%d msec)", xyz*1024, get_timer_msec(h0));
#ifndef NO_AVG
	if (cur_timing>=AVG_TIMINGS-1) cur_timing=0;
	timings[cur_timing++]=xyz;
	if (cur_timing>max_timing) max_timing=cur_timing;
	int avg=0;
	for (int i=0; i<max_timing; i++) avg+=timings[i];
	avg=avg/max_timing;
	buf->printf(57, 1, 7, "avg%d: %d", max_timing+1, avg*1024);
#endif
#endif
/* display flag if analyser(s) active */
/*	char *analysers[] = {"Analy", "aNaly", "anAly", "anaLy", "analY", "anaLy", "anAly", "aNaly"};
	if (some_analyser_active) buf->printf(60, 0, VCP(VC_TRANSPARENT, VC_TRANSPARENT), "%s (%d)", analysers[analy_id++], num_ops_parsed);
	if (analy_id > 7) analy_id = 0;*/
}

void ht_app::delete_window(ht_window *window)
{
	UINT i=get_window_listindex(window);
	if (i!=LIST_UNDEFINED) {
		battlefield->remove(window);

		windows->del(i);

		window->done();
		delete window;
	}
}

UINT ht_app::find_free_window_number()
{
	UINT c=windows->count();
	UINT k=0;
repeat:
	k++;
	for (UINT i=0; i<c; i++) {
		ht_app_window_entry *e=(ht_app_window_entry*)windows->get(i);
		if (e->number==k) goto repeat;
	}
	return k;
}

int ht_app::focus(ht_view *view)
{
	return ht_dialog::focus(view);
}

char *ht_app::func(UINT i, bool execute)
{
	switch (i) {
		case 1:
			if (execute) sendmsg(cmd_popup_window_help);
			return "help";
		case 6:
			if (execute) popup_view_list();
			return "mode";
		case 10:
			if (execute) sendmsg(cmd_quit);
			return "quit";
	}
	return 0;
}

ht_window *ht_app::get_window_by_number(UINT number)
{
	UINT c=windows->count();
	for (UINT i=0; i<c; i++) {
		ht_app_window_entry *e=(ht_app_window_entry*)windows->get(i);
		if (e->number==number) return e->window;
	}
	return 0;
}

ht_window *ht_app::get_window_by_type(UINT type)
{
	UINT c=windows->count();
	for (UINT i=0; i<c; i++) {
		ht_app_window_entry *e=(ht_app_window_entry*)windows->get(i);
		if (e->type==type) return e->window;
	}
	return 0;
}

UINT ht_app::get_window_number(ht_window *window)
{
	ht_app_window_entry *e=(ht_app_window_entry*)windows->get(get_window_listindex(window));
	if (e) return e->number; else return 0;
}

UINT ht_app::get_window_listindex(ht_window *window)
{
	UINT c=windows->count();
	for (UINT i=0; i<c; i++) {
		ht_app_window_entry *e=(ht_app_window_entry*)windows->get(i);
		if (e->window==window) return i;
	}
	return LIST_UNDEFINED;
}

void ht_app::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case cmd_file_save: {
			UINT i = get_window_listindex((ht_window*)battlefield->current);
			ht_app_window_entry *e = (ht_app_window_entry*)windows->get(i);
			if ((e) && (e->layer) && (e->isfile)) break;
		}
		case cmd_file_saveas: {
			UINT i=get_window_listindex((ht_window*)battlefield->current);
			ht_app_window_entry *e=(ht_app_window_entry*)windows->get(i);
			if ((e) && (e->layer)) {
				char fn[FILENAME_MAX];
				fn[0]=0;
				if (inputbox("Save as", "~Filename", fn, sizeof fn, HISTATOM_FILE) == button_ok) {
					int err;
					bool work = true;

					if (access(fn, F_OK) == 0) {
						work = (confirmbox("File '%s' already exists. Overwrite ?", fn) == button_yes);
					}

					if (work) {
						ht_file *f = new ht_file();
						f->init(fn, FAM_CREATE | FAM_WRITE);

						if ((err = f->get_error())) {
							f->done();
							delete f;
							LOG_EX(LOG_ERROR, "error saving file %s: %s", fn, strerror(err & ~STERR_SYSTEM));
							errorbox("error saving file %s: %s", fn, strerror(err & ~STERR_SYSTEM));
							return;
						}

						e->layer->seek(0);
						e->layer->copy_to(f);

						ht_streamfile *old = e->layer->get_layered();

						f->set_access_mode(old->get_access_mode());

						e->layer->set_layered(f);
						e->isfile = true;

						old->done();
						delete old;

						char fullfn[FILENAME_MAX], *ff=fullfn;
						if (sys_canonicalize(fn, fullfn)!=0) ff=fn;
						e->window->settitle(ff);
						clearmsg(msg);
					}
				}
			}
			return;
		}
		case msg_kill: {
			htmsg m;
			ht_window *w=(ht_window*)msg->data1.ptr;
			m.msg=msg_accept_close;
			m.type=mt_broadcast;
			m.data1.ptr=NULL;
			m.data2.ptr=NULL;
			w->sendmsg(&m);
			if (m.msg==msg_accept_close) delete_window(w);
			clearmsg(msg);
			return;
		}
	}
	if (msg->msg==msg_draw) {
		start_timer(h0);
		if (msg->type==mt_broadcast) {
			ht_view *v=first;
			while (v) {
				v->handlemsg(msg);
				v=v->next;
			}
		} else {
			current->handlemsg(msg);
		}
		stop_timer(h0);
		draw();
		screen->show();
	} else {
		ht_group::handlemsg(msg);
	}
	switch (msg->msg) {
		case msg_keypressed: {
			int i=0;
			switch (msg->data1.integer) {
				case K_Alt_9: i++;
				case K_Alt_8: i++;
				case K_Alt_7: i++;
				case K_Alt_6: i++;
				case K_Alt_5: i++;
				case K_Alt_4: i++;
				case K_Alt_3: i++;
				case K_Alt_2: i++;
				case K_Alt_1: i++;
					focus(get_window_by_number(i));
					clearmsg(msg);
					return;
				case K_F12: i++;
				case K_F11: i++;
				case K_F10: i++;
				case K_F9: i++;
				case K_F8: i++;
				case K_F7: i++;
				case K_F6: i++;
				case K_F5: i++;
				case K_F4: i++;
				case K_F3: i++;
				case K_F2: i++;
				case K_F1:
					i++;
					htmsg m;
					m.msg=msg_funcquery;
					m.type=mt_empty;
					m.data1.integer=i;
					sendmsg(&m);
					if (m.msg==msg_retval) {
						sendmsg(msg_funcexec, i);
						clearmsg(msg);
						return;
					}
					break;
/* FIXME: experimental */				
				case K_Alt_R: {
					char *n = "./ht.reg";
					ht_file *f = new ht_file();
					f->init(n, FAM_CREATE | FAM_WRITE);

					ht_object_stream_bin *b = new ht_object_stream_bin();
					b->init(f);

					b->put_object(registry, NULL);

					b->done();
					delete b;
					
					f->done();
					delete f;

					infobox("registry dumped to '%s'", n);
					
					clearmsg(msg);
					return;
				}
/* FIXME: experimental */				
/*				case K_Alt_T:
					create_window_ofm("reg://", "file://");
					clearmsg(msg);
					return;*/
/* FIXME: experimental */				
/*				case K_Control_W: {
					char file[256];
					strcpy(file, "/HT/res/test.info");
					if (inputbox("open info file", "filename", file, sizeof file, HISTATOM_FILE) == button_ok) {
						char node[256];
						strcpy(node, "Top");
						if (inputbox("open info file", "nodename", node, sizeof node, HISTATOM_GOTO) == button_ok) {
							create_window_help(file, node);
							dirtyview();
						}
					}
					clearmsg(msg);
					return;
				}*/
/* FIXME: experimental */				
/*				case K_Control_A:
					create_window_help("/HT/res/info/intidx.info", "Top");
//					create_window_help("c:/djgpp/projects/enew/res/info/ibnidx.info", "Interrupts By Number");
					dirtyview();
					clearmsg(msg);
					return;*/
				case K_Space:
					popup_view_list();
					clearmsg(msg);
					return;
/*				case K_Space: {
				msgbox(0, "title", 0, align_custom, "test, baby ! aaaaaaaaaaaarghssssssssssssssssssssssssssssssssssssssssssssssss,bf\n\n"
"Was willst DU Ich komm   nicht dahinter Ich zaehm dir das  Feuer      Du schenkst mir "
"nur Winter Wenn du nicht bald anf„ngst mir Antwort zu geben faengt es ohne "
"dich an das richtige Leben Ich hab lange gewartet bin nicht durchgeknallt "
"aber wenn du jetzt nicht \n\n\ecaufspringst wirst du ohne mich alt Erklaer mir ich "
"warte Wohin geht die Reise \n\n\erich kenn doch die Karte Nimm mich nicht als "
"Vorwand nimm mich nicht auf die Schippe sonst kuess ich dich luftleer und "
"zerbeiss dir die Lippe ich kann ohne dich leben auch wenn du nicht glaubst     \n"
"da ist mehr an mir dran als was du mir raubst ");
					clearmsg(msg);
					break;
				}*/
			}
			break;
		}
		case cmd_about:
			msgbox(btmask_ok, "About "ht_name, 0, align_custom, "\n\ec"ht_name" "ht_version" ("HT_SYS_NAME")\n\n\el"ht_copyright1"\n"ht_copyright2"\n\nThis program is GPL'd. See Help for more information.");
			break;
		case msg_funcexec:
			if (func(msg->data1.integer, 1)) {
				clearmsg(msg);
				return;
			}
			break;
		case msg_funcquery: {
			char *s=func(msg->data1.integer, 0);
			if (s) {
				msg->msg=msg_retval;
				msg->data1.str=s;
			} else clearmsg(msg);
			return;
		}
		case cmd_file_extend: {
			ht_streamfile *f=(ht_streamfile *)msg->data1.ptr;
			UINT s=(UINT)msg->data2.integer;
			if (confirmbox("really extend %s to offset %08x ?", f->get_filename(), s)==button_ok) {
				f->extend(s);
			}
			clearmsg(msg);
			return;
		}
		case cmd_file_truncate: {
			ht_streamfile *f=(ht_streamfile *)msg->data1.ptr;
			UINT s=(UINT)msg->data2.integer;
/*               ht_app_window_entry *e;
			if ((e=windows->enum_first())) {
				do {
					if ((e->type==AWT_FILE) && ((ht_streamfile*)e->data==f)) {
						check_collide();
					}
				} while ((e=windows->enum_first()));
			}*/
			if (confirmbox("really truncate %s at offset %08x ?", f->get_filename(), s)==button_ok) {
				f->truncate(s);
			}
			clearmsg(msg);
			return;
		}
		case cmd_quit:
			if (accept_close_all_windows()) {
				LOG("terminating...");
				exit_program=1;
				clearmsg(msg);
			}
			return;
		case cmd_file_open: {
			char *name;
			UINT mode;
			if (file_open_dialog(&name, &mode)) {
				if (name[0]) create_window_file(name, mode);
				free(name);
			}
			clearmsg(msg);
			return;
		}
		case cmd_file_new: {
			bounds b;
			battlefield->getbounds(&b);
			b.x=0;
			b.y=0;
			
			UINT mode;
			
			if (file_new_dialog(&mode)) {
				ht_mem_file *mfile=new ht_mem_file();
				mfile->init();
				switch (mode) {
					case FOM_TEXT: {
						ht_syntax_lexer *lexer = NULL;
			
						ht_ltextfile *tfile = new ht_ltextfile();
						tfile->init(mfile, true, lexer);
			
						ht_layer_textfile *file = new ht_layer_textfile();
						file->init(tfile, true);
				
						create_window_file_text(&b, file, "Untitled", false);
						break;
					}
					case FOM_BIN: {
						ht_streamfile_modifier *modfile=new ht_streamfile_modifier();
						modfile->init(mfile, true, 8*1024);

						ht_layer_streamfile *file = new ht_layer_streamfile();
						file->init(modfile, true);

						create_window_file_bin(&b, file, "Untitled", false);
					}
				}
			}
			clearmsg(msg);
			return;
		}
		case cmd_edit_show_clipboard: {
			create_window_clipboard();
			clearmsg(msg);
			return;
		}
		case cmd_edit_clear_clipboard: {
			if (confirmbox("Do you really want to delete the clipboard ?")==button_ok) {
				clipboard_clear();
			}
			clearmsg(msg);
			return;
		}
		case cmd_edit_paste_into_file: {
			char filename[129];
			filename[0]=0;
			if (inputbox("clipboard - paste into file", "filename", filename, 128, HISTATOM_FILE)==button_ok) {
				ht_file *f=new ht_file();
				f->init(filename, FAM_WRITE | FAM_CREATE);
				if (!f->get_error()) {
					clipboard_paste(f, 0);
				} else errorbox("can't open file '%s'", filename);
				f->done();
				delete f;
			}
			clearmsg(msg);
			return;
		}
		case cmd_edit_copy_from_file: {
			char filename[129];
			filename[0]=0;
			if (inputbox("clipboard - copy from file", "filename", filename, 128, HISTATOM_FILE)==button_ok) {
				char desc[1024]; /* FIXME: possible buffer overflow */
				ht_file *f=new ht_file();
				f->init(filename, FAM_READ);
				if (!f->get_error()) {
					sprintf(desc, "file %s", f->get_filename());
					clipboard_copy(desc, f, 0, f->get_size());
				} else errorbox("can't open file '%s'", filename);
				f->done();
				delete f;
			}
			clearmsg(msg);
			return;
		}
		case cmd_popup_dialog_eval: {
			eval_dialog();
			clearmsg(msg);
			return;
		}
/*		case cmd_options_palette:
			create_window_ofm("reg:///palette", NULL);
			clearmsg(msg);
			return;*/
		case cmd_window_resizemove:
			sendmsg(msg_resize, 0);
			clearmsg(msg);
			return;
		case cmd_window_close:
			if (battlefield->current) sendmsg(msg_kill, battlefield->current);
			clearmsg(msg);
			return;
		case cmd_popup_dialog_wlist:
			popup_window_list();
			clearmsg(msg);
			return;
		case cmd_popup_window_log:
			create_window_log();
			clearmsg(msg);
			return;
		case cmd_popup_window_options:
			create_window_ofm("reg://", NULL);
			clearmsg(msg);
			return;
		case cmd_popup_window_help:
			create_window_help("hthelp.info", "Top");
			clearmsg(msg);
			return;
	}
}

void	ht_app::insert_window(ht_window *window, UINT type, bool minimized, bool isfile, ht_layer_streamfile *layer)
{
	UINT n=find_free_window_number();
	ht_app_window_entry *e=new ht_app_window_entry(window, n, type, minimized, isfile, layer);
	windows->insert(e);
	window->setnumber(n);
	battlefield->insert(window);
	focus(window);
}

int ht_app::load(ht_object_stream *f)
{
	ht_registry *temp;
	
	if (!(temp=(ht_registry*)f->get_object(NULL))) return 1;

	if (registry) {
		registry->done();
		delete registry;
	}
	registry=temp;
	
	load_history(f);
	
	htmsg m;
	m.msg=msg_config_changed;
	m.type=mt_broadcast;
	app->sendmsg(&m);
	return f->get_error();
}

#define MAXLOGLINECOUNT	128
void ht_app::log_deletefirstline()
{
	log->del(0);
}

void	ht_app::log_insertline(int c, char *line)
{
	if (log->count()>=MAXLOGLINECOUNT) log_deletefirstline();
	int c2;
	switch (c) {
		case LOG_NORMAL: c2 = VCP(VC_WHITE, VC_TRANSPARENT); break;
		case LOG_WARN: c2 = VCP(VC_LIGHT(VC_YELLOW), VC_TRANSPARENT); break;
		case LOG_ERROR: c2 = VCP(VC_LIGHT(VC_RED), VC_TRANSPARENT); break;
		default: c2 = VCP(VC_WHITE, VC_TRANSPARENT); break;
	}
	log->insert(new ht_log_msg(line, c2));
}

void ht_app::logl(int c, char *line)
{
	log_insertline(c, line);
	sendmsg(msg_draw);
	
	ht_window *w=get_window_by_type(AWT_LOG);
	if (w) w->sendmsg(msg_dirtyview);
}

void ht_app::logf(int c, char *lineformat, ...)
{
	char buf[1024];	/* FIXME: possible buffer overflow ! */
	va_list arg;
	va_start(arg, lineformat);
	vsprintf(buf, lineformat, arg);
	va_end(arg);
	logl(c, buf);
}

OBJECT_ID ht_app::object_id()
{
	return ATOM_HT_APP;
}

int my_compare_func(char *a, char *b)
{
	return strcmp(a, b);
}

void ht_app::popup_view_list()
{
	if (!battlefield->current) return;
	bounds b, c;
	getbounds(&b);
	b.x=b.w/4;
	b.y=b.h/4;
	b.w/=2;
	b.h/=2;
	ht_dialog *dialog=new ht_dialog();
	dialog->init(&b, "Select mode", FS_KILLER | FS_TITLE | FS_MOVE);

/* create listbox */
	c=b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	ht_text_listbox *listbox=new ht_text_listbox();
	listbox->init(&c, 1, 0, LISTBOX_NORMAL);
/* insert all browsable views */
	ht_clist *structure=new ht_clist();
	((ht_clist*)structure)->init();
	int index=0;
/*	int count=*/popup_view_list_dump(battlefield->current, listbox, structure, 0, &index, battlefield->getselected());
/* and seek to the currently selected one */
	listbox->update();
	listbox->seek(index);

	ht_text_listbox_sort_order so[1];
	so[0].col = 0;
	so[0].compare_func = my_compare_func;
/*     so[1].col = 1;
	so[1].compare_func = my_compare_func;*/
	
//     listbox->sort(1, so);

	dialog->insert(listbox);
	dialog->setpalette(palkey_generic_special);

	if (dialog->run(0)) {
		ht_listbox_data data;
		listbox->databuf_get(&data);
		ht_data_ptr *p=(ht_data_ptr*)structure->get(data.cursor_id);
		if (p) focus((ht_view*)p->value);
	}

	structure->destroy();
	delete structure;

	dialog->done();
	delete dialog;
}

int ht_app::popup_view_list_dump(ht_view *view, ht_text_listbox *listbox, ht_list *structure, int depth, int *currenti, ht_view *currentv)
{
	if (!view) return 0;
	char str[256];
	char *s=str;
	if (!(view->options & VO_BROWSABLE)) {
		depth--;
	}

	for (int i=0; i<depth; i++) { *(s++)=' ';*(s++)=' '; }
	*s=0;

	int c=view->childcount();
	int count=0;
	for (int i=0; i<c; i++) {
		ht_view *v=view->getfirstchild();
		while (v) {
			if (v->browse_idx==i) break;
			v=v->next;
		}
		if (!v) return count;
		
// FIXME: "viewergroup": dirty hack !!!
		if ((v->desc) && (strcmp(v->desc, "viewergroup")) && (v->options & VO_BROWSABLE)) {
			sprintf(s, "- %s", v->desc);
			structure->insert(new ht_data_ptr(v));
			if (v==currentv)
				*currenti=structure->count()-1;
			listbox->insert_str(structure->count()-1, str);
			count++;
		}
		count+=popup_view_list_dump(v, listbox, structure, depth+1, currenti, currentv);
	}
	return count;
}

void ht_app::popup_window_list()
{
	bounds b, c;
	getbounds(&b);
	c=b;
	b.w=b.w*2/3;
	b.h=b.h*2/3;
	b.x=(c.w-b.w)/2;
	b.y=(c.h-b.h)/2;
	ht_dialog *dialog=new ht_dialog();
	dialog->init(&b, "windows", FS_KILLER | FS_TITLE | FS_MOVE);

/* create listbox */
	c=b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	ht_text_listbox *listbox=new ht_itext_listbox();
	listbox->init(&c, 2, 1);

	UINT vc=windows->count();
	for (UINT i=0; i<vc; i++) {
		ht_app_window_entry *e=(ht_app_window_entry*)windows->get(i);
		char l[16];
		sprintf(l, " %2d", e->number);
		listbox->insert_str(e->number, l, e->window->desc);
	}
	listbox->update();
	if (battlefield->current) listbox->seek(battlefield->current->getnumber()-1);

	dialog->insert(listbox);
	dialog->setpalette(palkey_generic_special);

	if (dialog->run(0)) {
		ht_listbox_data data;
		listbox->databuf_get(&data);
		ht_window *w=get_window_by_number(data.cursor_id);
		if (w) focus(w);
	}
	dialog->done();
	delete dialog;
}

int ht_app::run(bool modal)
{
	sendmsg(msg_draw, 0);
	while (!exit_program) {
		if (ht_keypressed()) {
			int k=ht_getkey();
			sendmsg(msg_keypressed, k);
			sendmsg(msg_draw);
		}
		ht_queued_msg *q;
		while ((q=dequeuemsg())) {
			htmsg m=q->msg;
			q->target->sendmsg(&m);
			sendmsg(msg_draw);
			delete q;
		}
		do_idle();
	}
	return 0;
}

void ht_app::store(ht_object_stream *f)
{
	f->put_object(registry, NULL);

	store_history(f);

#if 0
	f->put_int_dec(windows->count(), 4, "windows");
	ht_app_window_entry *e;
	if ((e=(ht_app_window_entry*)windows->enum_first())) {
		char prjcfg_path[1024];	/* FIXME: possible buffer overflow */
		sys_dirname(f->get_name(), prjcfg_path);
		do {
			f->put_int_dec(e->type, 4, "type");
			switch (e->type) {
				case AWT_LOG:
					break;
				case AWT_HELP:
					break;
				case AWT_FILE: {
					char relfilename[1024];	/* FIXME: possible buffer overflow */
					sys_relname(e->window->desc, prjcfg_path, relfilename);
					strcat(relfilename, ".htcfg");
					f->put_string(relfilename, "file");
					save_fileconfig(relfilename, e->window);
					break;
				}
				case AWT_OFM:
					break;
			}
		} while ((e=(ht_app_window_entry*)windows->enum_next()));
	}
#endif	
}

/*
 *	CLASS ht_file_window
 */

void	ht_file_window::init(bounds *b, char *desc, UINT framestyle, UINT number, ht_streamfile *f)
{
	ht_window::init(b, desc, framestyle, number);
	file=f;
}

void ht_file_window::done()
{
	ht_window::done();
}

void ht_file_window::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_accept_close: if (file) {
			bool modified=false;
			if (file->get_filename() != NULL) {
				file->cntl(FCNTL_MODS_IS_DIRTY, 0, file->get_size(), &modified);
			} else {
				modified = true;
			}
			if (modified) {
				switch (msgbox(btmask_yes+btmask_no+btmask_cancel, "confirmation", 0, align_center, "file %s has been modified, save ?", file->get_filename())) {
					case button_yes: {
						htmsg msg;
						msg.msg = cmd_file_save;
						msg.type = mt_empty;
						app->sendmsg(&msg);
						if ((UINT)msg.msg != cmd_file_save) break;
					}
					case button_cancel:
						clearmsg(msg);
						return;
				}
			}

			file->cntl(FCNTL_FLUSH_STAT);
			
			analyser *a;
			htmsg m;
			m.msg=msg_get_analyser;
			m.type=mt_broadcast;
			m.data1.ptr=NULL;
			sendmsg(&m);
			a = (analyser*)m.data1.ptr;
			if ((m.msg==msg_retval) && (a) && (a->is_dirty())) {
				sendmsg(cmd_analyser_save);
			}
		}
		break;
		case cmd_analyser_save: {
			char filename[1024];	/* FIXME: possible buffer overflow */
			strcpy(filename, file->get_filename());
			strcat(filename, ".htcfg");
			LOG("%s: saving config", filename);
			save_fileconfig(filename, file_window_store_fcfg_func, this);
			LOG("%s: done", filename);
			clearmsg(msg);
			break;
		}
	}
	ht_window::handlemsg(msg);
}

BUILDER(ATOM_HT_APP, ht_app);

/*
 *	INIT
 */

bool init_app()
{
	bounds b;
	screen=new screendrawbuf(ht_name" "ht_version);

	b.x=0;
	b.y=0;
	b.w=screen->size.w;
	b.h=screen->size.h;
	app=new ht_app();
	((ht_app*)app)->init(&b);
	baseview=app;

	app_memory_reserve = malloc(16384); // malloc, not smalloc
	out_of_memory_func = &app_out_of_memory_proc;

	REGISTER(ATOM_HT_APP, ht_app);

	return true;
}

/*
 *	DONE
 */

void done_app()
{
	UNREGISTER(ATOM_HT_APP, ht_app);
	
	out_of_memory_func = &out_of_memory;
	if (app_memory_reserve) free(app_memory_reserve);

	if (app) {
		app->done();
		delete app;
	}
	
	if (screen) delete screen;
}

