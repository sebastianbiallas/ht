/*
 *	HT Editor
 *	htmenu.cc
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

#include "htctrl.h"
#include "htkeyb.h"
#include "htmenu.h"
#include "htpal.h"
#include "htreg.h"
#include "htstring.h"
#include "tools.h"

#include <string.h>


#define button_left		100
#define button_right	101

char *shortcut_str(char *str)
{
	while (*str) {
		if (*str=='~') {
			memmove(str, str+1, strlen(str));
			return str;
		}
		str++;
	}
	return NULL;
}

/*
 *	CLASS ht_context_menu
 */

void ht_context_menu::init(char *_name)
{
	ht_data::init();
	_name=ht_strdup(_name);
	shortcut=shortcut_str(_name);
	name=_name;
	width=0;
}

void ht_context_menu::done()
{
	delete name;
	ht_data::done();
}

int ht_context_menu::count()
{
	return 0;
}

ht_context_menu_entry *ht_context_menu::enum_entry_first()
{
	return NULL;
}

ht_context_menu_entry *ht_context_menu::enum_entry_next()
{
	return NULL;
}

ht_context_menu_entry *ht_context_menu::get_entry(int n)
{
	ht_context_menu_entry *e=enum_entry_first();
	while (n--) {
		if (!e) break;
		e=enum_entry_next();
	}
	return e;
}

/*
 *	CLASS ht_static_context_menu
 */

void ht_static_context_menu::init(char *name)
{
	ht_context_menu::init(name);
	context_menu_entry=new ht_clist();
	((ht_clist*)context_menu_entry)->init();
}

void ht_static_context_menu::done()
{
	context_menu_entry->destroy();
	delete context_menu_entry;
	ht_context_menu::done();
}

int ht_static_context_menu::count()
{
	return context_menu_entry->count();
}

ht_context_menu_entry *ht_static_context_menu::enum_entry_first()
{
	enum_idx=0;
	ht_context_menu_entry *e=(ht_context_menu_entry*)context_menu_entry->get(enum_idx);
	return e;
}

ht_context_menu_entry *ht_static_context_menu::enum_entry_next()
{
	enum_idx++;
	ht_context_menu_entry *e=(ht_context_menu_entry*)context_menu_entry->get(enum_idx);
	return e;
}

int ht_static_context_menu::insert_entry(char *_name, char *_comment, int _command, int _key, int _active)
{
	char *name=ht_strdup(_name), *comment, *shortcut=NULL;
	if (_comment) comment=ht_strdup(_comment); else comment=NULL;
	shortcut = shortcut_str(name);
	int l=strlen(name), cl=comment ? strlen(comment)+2 : 0;
	width=MAX(width, l+cl);
	ht_context_menu_entry *entry=new ht_context_menu_entry();
	entry->name=name;
	entry->shortcut=shortcut;
	entry->comment=comment;
	entry->command=_command;
	entry->key=_key;
	entry->active=_active;
	context_menu_entry->insert(entry);
	return 0;
}

int ht_static_context_menu::insert_separator()
{
	ht_context_menu_entry *entry=new ht_context_menu_entry();
	entry->name=NULL;
	entry->shortcut=NULL;
	entry->command=0;
	entry->comment=NULL;
	entry->active=0;
	context_menu_entry->insert(entry);
	return 0;
}

/*
 *	CLASS ht_context_menu_entry
 */

ht_context_menu_entry::~ht_context_menu_entry()
{
	if (name) free(name);
	if (comment) free(comment);
}

/*
 *	CLASS ht_menu
 */

void ht_menu::init(bounds *b)
{
	ht_view::init(b, VO_OWNBUFFER | VO_POSTPROCESS, "menu");
	VIEW_DEBUG_NAME("ht_menu");

	menu=new ht_clist();
	((ht_clist*)menu)->init();
	lastmenux = 1;
	curmenu = -1;
	localmenu = -1;
	context_menu_hack = NULL;
	last_context_menu_hack = NULL;
	context_menu_hack2 = false;
}

void ht_menu::done()
{
	menu->destroy();
	delete menu;
	ht_view::done();
}

char *ht_menu::defaultpalette()
{
	return palkey_generic_menu_default;
}

char *ht_menu::defaultpaletteclass()
{
	return palclasskey_generic;
}

ht_context_menu *ht_menu::get_context_menu(int i)
{
	ht_context_menu *q = (ht_context_menu*)menu->get(i);
	if (context_menu_hack && (i == localmenu) && !q) return context_menu_hack;
	return q;
}

void ht_menu::draw()
{
	clear(getcolor(palidx_generic_body));
	int c = count();
	for (int i=0; i<c; i++) {
		ht_context_menu *c = get_context_menu(i);
		if (i==curmenu) {
			buf_printchar(c->xpos-1, 0, getcolor(palidx_generic_text_selected), ' ');
			buf_print(c->xpos, 0, getcolor(palidx_generic_text_selected), c->name);
			buf_printchar(c->xpos+strlen(c->name), 0, getcolor(palidx_generic_text_selected), ' ');
			if (c->shortcut) buf_printchar(c->xpos+(c->shortcut-c->name), 0, getcolor(palidx_generic_text_shortcut_selected), *c->shortcut);
		} else {
			buf_print(c->xpos, 0, getcolor(palidx_generic_text_focused), c->name);
			if (c->shortcut) buf_printchar(c->xpos+(c->shortcut-c->name), 0, getcolor(palidx_generic_text_shortcut), *c->shortcut);
		}
	}
}

void ht_menu::execute_menu(int i)
{
	curmenu=i;
	int cur_entry=0;
	int term=0;
	dirtyview();
	do {
		ht_context_menu *m = get_context_menu(curmenu);
		if (!m) return;		/* FIXME: should never happen */
		bounds b;
		b.x=m->xpos-1;
		b.y=1;
		b.w=m->width+4;
		b.h=m->count()+2;
		ht_menu_window *d=new ht_menu_window();
		d->init(&b, m);
		ht_context_menu_window_data a;
		a.selected=cur_entry;
		d->databuf_set(&a);
		b.x=0;
		b.y=0;
		ht_frame *frame=new ht_menu_frame();
		frame->init(&b, 0, FS_MOVE);	// just for fun
		d->setframe(frame);
		d->setpalette(palkey_generic_menu_default);
		int r=d->run(0);
		m = get_context_menu(curmenu);
		switch (r) {
			case button_left:
				curmenu--;
				if (curmenu<0) curmenu = count()-1;
				cur_entry=0;
				dirtyview();
				break;
			case button_right:
				curmenu++;
				if (curmenu>(int)count()-1) curmenu=0;
				cur_entry=0;
				dirtyview();
				break;
			case button_ok: {
				ht_context_menu_window_data a;
				d->databuf_get(&a);
				cur_entry = a.selected;
				ht_context_menu_entry *e = m->get_entry(a.selected);
				if (e->active) {
					term=1;
					curmenu=-1;
					dirtyview();
					app->sendmsg(e->command);
				}
				break;
			}
			default:
				term=1;
		}
		d->done();
		delete d;
	} while (!term);
	curmenu=-1;
	dirtyview();
}

void ht_menu::handlemsg(htmsg *msg)
{
	if (msg->type==mt_postprocess) {
		if (msg->msg==msg_keypressed) {
/* shortcuts */
			ht_key k=ht_unmetakey((ht_key)msg->data1.integer);
			if (k!=K_INVALID) {
				int c=count();
				for (int i=0; i<c; i++) {
					ht_context_menu *m = get_context_menu(i);
					int s=*m->shortcut;
					if ((s>='A') && (s<='Z')) s+='a'-'A';
					if (s==k) {
						if (last_context_menu_hack) {
							last_context_menu_hack->done();
							delete last_context_menu_hack;
							last_context_menu_hack = NULL;
						}
						if (localmenu != -1) {
							context_menu_hack = (ht_context_menu*)menu->get(localmenu);
							last_context_menu_hack = NULL;
							if (context_menu_hack) {
								menu->remove(localmenu);
							}
						}
						context_menu_hack2 = true;
						execute_menu(i);
						context_menu_hack2 = false;
						if (context_menu_hack) {
							context_menu_hack->done();
							delete context_menu_hack;
							context_menu_hack = NULL;
						}
						if (last_context_menu_hack) {
							set_local_menu(last_context_menu_hack);
							last_context_menu_hack = NULL;
						} else {
							delete_local_menu();
						}
						clearmsg(msg);
						return;
					}
				}
			}
/* keys associated with menu entries */
			int c = count();
			for (int i=0; i<c; i++) {
				ht_context_menu *a = get_context_menu(i);
				int t=a->count();
				ht_context_menu_entry *e=a->enum_entry_first();
				for (int j=0; j<t; j++) {
					if ((e->name) && (e->key) && (msg->data1.integer==e->key)) {
						app->sendmsg(e->command);
						clearmsg(msg);
						return;
					}
					e=a->enum_entry_next();
				}
			}
		}
	}
	ht_view::handlemsg(msg);
}

int ht_menu::count()
{
	ht_context_menu *q = (ht_context_menu*)menu->get(localmenu);
	if (context_menu_hack) return menu->count() + (((localmenu != -1) && !q) 
		? 1 : 0);
	return menu->count();
}

void ht_menu::delete_local_menu()
{
	if (localmenu != -1) {
/*		if (last_context_menu_hack) {
			last_context_menu_hack = false;
			if (last_context_menu) {
				last_context_menu->done();
				delete last_context_menu;
				last_context_menu = NULL;
			}
			return;*/
/*			ht_context_menu *q = (ht_context_menu*)menu->get(localmenu);
			if (q && (q == last_context_menu)) menu->remove(localmenu);
			return;*/
//		}
		if (context_menu_hack2 && last_context_menu_hack) {
			last_context_menu_hack->done();
			delete last_context_menu_hack;
			last_context_menu_hack = NULL;
		}
		menu->del(localmenu);
	}		
}

void ht_menu::insert_local_menu()
{
	localmenu = menu->count();
}

void ht_menu::insert_menu(ht_context_menu *m)
{
	int namelen = strlen(m->name);

	m->xpos = lastmenux;
	menu->insert(m);

	lastmenux += namelen+1;
}

bool ht_menu::set_local_menu(ht_context_menu *m)
{
	if (localmenu == -1) return false;
	if (context_menu_hack2) {
		if (last_context_menu_hack) {
			last_context_menu_hack->done();
			delete last_context_menu_hack;
			last_context_menu_hack = NULL;
		}
		last_context_menu_hack = m;
	} else {
		int namelen = strlen(m->name);

		ht_context_menu *p = (ht_context_menu*)menu->get(localmenu-1);
		if (p) lastmenux = p->xpos+strlen(p->name)+1; else lastmenux = 1;

		m->xpos = lastmenux;

		menu->set(localmenu, m);

		lastmenux += namelen+1;
	}
	return true;
}

/*
 *	CLASS ht_menu_frame
 */

void ht_menu_frame::init(bounds *b, char *desc, UINT style, UINT number)
{
	ht_frame::init(b, desc, style, number);
	VIEW_DEBUG_NAME("ht_menu_frame");
}

void ht_menu_frame::done()
{
	ht_frame::done();
}

char *ht_menu_frame::defaultpalette()
{
	return palkey_generic_menu_default;
}

char *ht_menu_frame::defaultpaletteclass()
{
	return palclasskey_generic;
}

int ht_menu_frame::getcurcol_normal()
{
	return ht_frame::getcurcol_normal();
}

int ht_menu_frame::getcurcol_killer()
{
	return 0;
}

/*
 *	CLASS ht_context_menu_window
 */

void ht_context_menu_window::init(bounds *b, ht_context_menu *menu)
{
	ht_dialog::init(b, NULL, 0);
	VIEW_DEBUG_NAME("ht_context_menu_window");
	bounds c=*b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	body=new ht_context_menu_window_body();
	body->init(&c, menu);
	insert(body);
}

void ht_context_menu_window::done()
{
	ht_dialog::done();
}

void ht_context_menu_window::getdata(ht_object_stream *s)
{
	body->getdata(s);
}

void ht_context_menu_window::setdata(ht_object_stream *s)
{
	body->setdata(s);
}

/*
 *	CLASS ht_context_menu_window_body
 */

void ht_context_menu_window_body::init(bounds *b, ht_context_menu *_menu)
{
	ht_view::init(b, VO_OWNBUFFER | VO_SELECTABLE, 0);
	VIEW_DEBUG_NAME("ht_context_menu_window_body");
	context_menu=_menu;
	selected=next_selectable(-1);
}

void ht_context_menu_window_body::done()
{
	ht_view::done();
}

char *ht_context_menu_window_body::defaultpalette()
{
	return palkey_generic_menu_default;
}

char *ht_context_menu_window_body::defaultpaletteclass()
{
	return palclasskey_generic;
}

void ht_context_menu_window_body::draw()
{
	clear(getcolor(palidx_generic_body));
	int c=context_menu->count();
	if (c>size.h) c=size.h;
	ht_context_menu_entry *e=context_menu->enum_entry_first();
	for (int i=0; i<c; i++) {
		if (e->name) {
			if (i==selected) {
				if (e->active) {
					fill(0, i, size.w, 1, getcolor(palidx_generic_text_selected), ' ');
					buf_lprint(1, i, getcolor(palidx_generic_text_selected), size.w, e->name);
					if (e->comment) buf_lprint(size.w-1-strlen(e->comment), i, getcolor(palidx_generic_text_selected), size.w, e->comment);
					if (e->shortcut) buf_printchar(e->shortcut-e->name+1, i, getcolor(palidx_generic_text_shortcut_selected), *e->shortcut);
				} else {
					fill(0, i, size.w, 1, getcolor(palidx_generic_text_disabled), ' ');
					buf_lprint(1, i, getcolor(palidx_generic_text_disabled), size.w, e->name);
					if (e->comment) buf_lprint(size.w-1-strlen(e->comment), i, getcolor(palidx_generic_text_disabled), size.w, e->comment);
				}
			} else {
				if (e->active) {
					buf_lprint(1, i, getcolor(palidx_generic_text_focused), size.w, e->name);
					if (e->comment) buf_lprint(size.w-1-strlen(e->comment), i, getcolor(palidx_generic_text_focused), size.w, e->comment);
					if (e->shortcut) buf_printchar(e->shortcut-e->name+1, i, getcolor(palidx_generic_text_shortcut), *e->shortcut);
				} else {
					buf_lprint(1, i, getcolor(palidx_generic_text_disabled), size.w, e->name);
					if (e->comment) buf_lprint(size.w-1-strlen(e->comment), i, getcolor(palidx_generic_text_disabled), size.w, e->comment);
				}
			}
		} else {
			fill(0, i, size.w, 1, getcolor(palidx_generic_text_focused), CHAR_LINEH);
		}
		e=context_menu->enum_entry_next();
	}
}

void ht_context_menu_window_body::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Up:
				selected=prev_selectable(selected);
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				selected=next_selectable(selected);
				dirtyview();
				clearmsg(msg);
				return;
			case K_Home:
				selected=next_selectable(-1);
				dirtyview();
				clearmsg(msg);
				return;
			case K_End:
				selected=prev_selectable(0);
				dirtyview();
				clearmsg(msg);
				return;
		}
/* shortcuts */
		int k=msg->data1.integer;
//		int k=htalt2key(msg->data_int);
//		if (k!=-1) {
			int c=context_menu->count();
			ht_context_menu_entry *e=context_menu->enum_entry_first();
			for (int i=0; i<c; i++) {
				if (e->shortcut) {
					int s=*e->shortcut;
					if ((s>='A') && (s<='Z')) s+='a'-'A';
					if (s==k) {
						selected=i;
						((ht_dialog*)group->group)->sendmsg(msg_button_pressed, button_ok);
						dirtyview();
						clearmsg(msg);
						return;
					}
				}					
				e=context_menu->enum_entry_next();
			}
//		}
	}
	ht_view::handlemsg(msg);
}

int ht_context_menu_window_body::next_selectable(int to)
{
	int s=to+1;
	int c=context_menu->count();
	if (s>c-1) s=0;
	while (s!=to) {
		ht_context_menu_entry *e=context_menu->get_entry(s);
		if (e->name) return s;
		s++;
		if (s>c-1) s=0;
	}
	return to;
}

int ht_context_menu_window_body::prev_selectable(int to)
{
	int s=to-1;
	int c=context_menu->count();
	if (s<0) s=c-1;
	while (s!=to) {
		ht_context_menu_entry *e=context_menu->get_entry(s);
		if (e->name) return s;
		s--;
		if (s<0) s=c-1;
	}
	return to;
}

void ht_context_menu_window_body::getdata(ht_object_stream *s)
{
	s->put_int_dec(selected, 4, NULL);
}

void ht_context_menu_window_body::setdata(ht_object_stream *s)
{
	selected=s->get_int_dec(4, NULL);
}

/*
 *	CLASS ht_menu_window
 */

void ht_menu_window::init(bounds *b, ht_context_menu *menu)
{
	ht_dialog::init(b, 0, 0);
	VIEW_DEBUG_NAME("ht_menu_window");

	bounds c=*b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	body=new ht_menu_window_body();
	body->init(&c, menu);
	insert(body);
}

void ht_menu_window::done()
{
	ht_dialog::done();
}

void ht_menu_window::getdata(ht_object_stream *s)
{
	body->getdata(s);
}

void ht_menu_window::setdata(ht_object_stream *s)
{
	body->setdata(s);
}

/*
 *	CLASS ht_menu_window_body
 */

void ht_menu_window_body::init(bounds *b, ht_context_menu *menu)
{
	ht_context_menu_window_body::init(b, menu);
	VIEW_DEBUG_NAME("ht_menu_window_body");
}

void ht_menu_window_body::done()
{
	ht_context_menu_window_body::done();
}

void ht_menu_window_body::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Left:
				((ht_dialog*)baseview)->setstate(ds_term_ok, button_left);
				dirtyview();
				clearmsg(msg);
				return;
			case K_Right:
				((ht_dialog*)baseview)->setstate(ds_term_ok, button_right);
				dirtyview();
				clearmsg(msg);
				return;
		}
	}
	ht_context_menu_window_body::handlemsg(msg);
}

/*
 *	INIT
 */

bool init_menu()
{
	return true;
}

/*
 *	DONE
 */

void done_menu()
{
}
