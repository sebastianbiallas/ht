/*
 *	HT Editor
 *	htmenu.cc
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

#include <cstring>

#include "htapp.h"		// for queuemsg
#include "htctrl.h"
#include "keyb.h"
#include "htmenu.h"
#include "htpal.h"
#include "htreg.h"
#include "strtools.h"
#include "stream.h"
#include "tools.h"

#define button_left	100
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

bool execute_submenu(int x, int y, ht_context_menu *m)
{
	int curentry=0;
	bool term=false;
	Bounds scr;
	app->getbounds(&scr);
	do {
		Bounds b;
		b.x=x+m->xpos-1;
		b.y=y;
		b.w=m->width+4;
		b.h=m->count()+2;
		if (b.y+b.h > scr.h) {
			if (scr.h > b.h) {
				b.y = scr.h - b.h;
				if (b.y >= 2) b.y -= 2;
			} else {
				b.y = 0;
			}
		}
		ht_menu_window *d = new ht_menu_window();
		d->init(&b, m);
		ht_menu_window_data a;
		a.selected = curentry;
		d->databuf_set(&a, sizeof a);
		b.x = 0;
		b.y = 0;
		ht_frame *frame = new ht_menu_frame();
		frame->init(&b, 0, FS_MOVE/*just for fun*/);
		d->setframe(frame);
		d->setpalette(palkey_generic_menu_default);
		int r = d->run(false);
		d->done();
		delete d;
		switch (r) {
		case button_ok: {
			return true;
		}
		default:
			term = true;
		}
	} while (!term);
	return false;
}

/*
 *	CLASS ht_context_menu
 */

void ht_context_menu::init(const char *Name)
{
	name = ht_strdup(Name);
	shortcut = shortcut_str(name);
	width = 0;
}

void ht_context_menu::done()
{
	free(name);
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
	while (n-- && e) {
		e = enum_entry_next();
	}
	return e;
}

const char *ht_context_menu::get_name()
{
	return name;
}

const char *ht_context_menu::get_shortcut()
{
	return shortcut;
}

/*
 *	CLASS ht_static_context_menu
 */

void ht_static_context_menu::init(const char *name)
{
	ht_context_menu::init(name);
	context_menu_entry = new Array(true);
}

void ht_static_context_menu::done()
{
	delete context_menu_entry;
	ht_context_menu::done();
}

int ht_static_context_menu::count()
{
	return context_menu_entry->count();
}

ht_context_menu_entry *ht_static_context_menu::enum_entry_first()
{
	enum_idx = 0;
	ht_context_menu_entry *e = (ht_context_menu_entry*)(*context_menu_entry)[enum_idx];
	return e;
}

ht_context_menu_entry *ht_static_context_menu::enum_entry_next()
{
	enum_idx++;
	ht_context_menu_entry *e = (ht_context_menu_entry*)(*context_menu_entry)[enum_idx];
	return e;
}

void ht_static_context_menu::insert_entry(const char *Name, const char *Comment, int cmd, int k, bool a)
{
	char *name = ht_strdup(Name);
	char *comment = ht_strdup(Comment);
	char *shortcut = shortcut_str(name);
	int l = strlen(name);
	int cl = comment ? strlen(comment)+2 : 0;
	width = MAX(width, l+cl);
	ht_context_menu_entry *e = new ht_context_menu_entry();
	e->type = CME_ENTRY;
	e->entry.name = name;
	e->entry.shortcut = shortcut;
	e->entry.comment = comment;
	e->entry.command = cmd;
	e->entry.key = k;
	e->entry.active = a;
	context_menu_entry->insert(e);
}

void ht_static_context_menu::insert_separator()
{
	ht_context_menu_entry *entry = new ht_context_menu_entry();
	entry->type = CME_SEPARATOR;
	context_menu_entry->insert(entry);
}

void ht_static_context_menu::insert_submenu(ht_context_menu *submenu)
{
	ht_context_menu_entry *entry = new ht_context_menu_entry();
	entry->type = CME_SUBMENU;
	entry->submenu = submenu;
	submenu->xpos = 0;
	context_menu_entry->insert(entry);
}

/*
 *	CLASS ht_context_menu_entry
 */

ht_context_menu_entry::~ht_context_menu_entry()
{
	switch (type) {
	case CME_ENTRY:
		free(entry.name);
		free(entry.comment);
		break;
	case CME_SUBMENU:
		submenu->done();
		delete submenu;
		break;
	}
}

/*
 *	CLASS ht_menu
 */

void ht_menu::init(Bounds *b)
{
	ht_view::init(b, VO_OWNBUFFER | VO_POSTPROCESS | VO_RESIZE, "menu");
	VIEW_DEBUG_NAME("ht_menu");

	growmode = MK_GM(GMH_FIT, GMV_TOP);

	menu = new Array(true);
	lastmenux = 1;
	curmenu = -1;
	localmenu = -1;
	context_menu_hack = NULL;
	last_context_menu_hack = NULL;
	context_menu_hack2 = false;
}

void ht_menu::done()
{
	delete menu;
	ht_view::done();
}

const char *ht_menu::defaultpalette()
{
	return palkey_generic_menu_default;
}

const char *ht_menu::defaultpaletteclass()
{
	return palclasskey_generic;
}

ht_context_menu *ht_menu::get_context_menu(int i)
{
	ht_context_menu *q = (ht_context_menu*)(*menu)[i];
	if (context_menu_hack && (i == localmenu) && !q) return context_menu_hack;
	return q;
}

void ht_menu::getminbounds(int *width, int *height)
{
	*width = 1;
	*height = 1;
}

void ht_menu::draw()
{
	clear(getcolor(palidx_generic_body));
	int c = count();
	for (int i=0; i < c; i++) {
		ht_context_menu *c = get_context_menu(i);
		const char *n = c->get_name();
		const char *s = c->get_shortcut();
		if (i == curmenu) {
			buf->printChar(c->xpos-1, 0, getcolor(palidx_generic_text_selected), ' ');
			buf->print(c->xpos, 0, getcolor(palidx_generic_text_selected), n);
			buf->printChar(c->xpos+strlen(n), 0, getcolor(palidx_generic_text_selected), ' ');
			if (s) buf->printChar(c->xpos+(s - n), 0, getcolor(palidx_generic_text_shortcut_selected), *s);
		} else {
			buf->print(c->xpos, 0, getcolor(palidx_generic_text_focused), n);
			if (s) buf->printChar(c->xpos+(s - n), 0, getcolor(palidx_generic_text_shortcut), *s);
		}
	}
}

void ht_menu::execute_menu(int i)
{
	curmenu = i;
	int curentry = 0;
	bool term = false;
	dirtyview();
	do {
		ht_context_menu *m = get_context_menu(curmenu);
		Bounds b;
		b.x = m->xpos-1;
		b.y = 1;
		b.w = m->width + 4;
		b.h = m->count() + 2;
		ht_menu_window *d = new ht_menu_window();
		d->init(&b, m);
		ht_menu_window_data a;
		a.selected = curentry;
		d->databuf_set(&a, sizeof a);
		b.x = 0;
		b.y = 0;
		ht_frame *frame = new ht_menu_frame();
		frame->init(&b, 0, FS_MOVE);	// just for fun
		d->setframe(frame);
		d->setpalette(palkey_generic_menu_default);
		int r = d->run(false);
		switch (r) {
		case button_left:
			curmenu--;
			if (curmenu < 0) curmenu = count() - 1;
			curentry=0;
			dirtyview();
			break;
		case button_right:
			curmenu++;
			if (curmenu > (int)count()-1) curmenu = 0;
			curentry = 0;
			dirtyview();
			break;
		case button_ok: {
//			return true;
		}
		default:
			term = true;
		}
		d->done();
		delete d;
	} while (!term);
	curmenu = -1;
	dirtyview();
}

void ht_menu::handlemsg(htmsg *msg)
{
	if (msg->type == mt_postprocess) {
		if (msg->msg == msg_keypressed) {
			/* shortcuts */
			ht_key k = keyb_unmetakey((ht_key)msg->data1.integer);
			if (k != K_INVALID) {
				int c=count();
				for (int i=0; i<c; i++) {
					ht_context_menu *m = get_context_menu(i);
					int s = *m->get_shortcut();
					if (s >= 'A' && s <= 'Z') s += 'a'-'A';
					if (s == k) {
						if (last_context_menu_hack) {
							last_context_menu_hack->done();
							delete last_context_menu_hack;
							last_context_menu_hack = NULL;
						}
						if (localmenu != -1) {
							context_menu_hack = (ht_context_menu*)(*menu)[localmenu];
							last_context_menu_hack = NULL;
							if (context_menu_hack) {
								menu->remove(menu->findByIdx(localmenu));
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
				if (handle_key_context_menu(a, msg->data1.integer)) {
					clearmsg(msg);
					return;
				}
			}
		}
	}
	ht_view::handlemsg(msg);
}

bool ht_menu::handle_key_context_menu(ht_context_menu *a, int k)
{
	ht_context_menu_entry *e = a->enum_entry_first();
	while (e) {
		if (e->type == CME_ENTRY && e->entry.name && e->entry.key && k == e->entry.key) {
			htmsg m;
			m.msg = e->entry.command;
			m.type = mt_empty;
			((ht_app*)app)->queuemsg(app, m);
			return true;
		} else if (e->type == CME_SUBMENU) {
			if (handle_key_context_menu(e->submenu, k)) return true;
		}
		e = a->enum_entry_next();
	}
	return false;
}

int ht_menu::count()
{
	ht_context_menu *q = (ht_context_menu*)(*menu)[localmenu];
	if (context_menu_hack) return menu->count() + !!(localmenu != -1 && !q);
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
		menu->del(menu->findByIdx(localmenu));
	}		
}

void ht_menu::insert_local_menu()
{
	localmenu = menu->count();
}

void ht_menu::insert_menu(ht_context_menu *m)
{
	int namelen = strlen(m->get_name());

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
		ht_context_menu *p = (ht_context_menu*)(*menu)[localmenu-1];
		if (p) {
			lastmenux = p->xpos + strlen(p->get_name()) + 1; 
		} else {
			lastmenux = 1;
		}

		m->xpos = lastmenux;

		menu->forceSetByIdx(localmenu, m);

		lastmenux += strlen(m->get_name()) + 1;
	}
	return true;
}

/*
 *	CLASS ht_menu_frame
 */

void ht_menu_frame::init(Bounds *b, const char *desc, uint style, uint number)
{
	ht_frame::init(b, desc, style, number);
	VIEW_DEBUG_NAME("ht_menu_frame");
}

void ht_menu_frame::done()
{
	ht_frame::done();
}

const char *ht_menu_frame::defaultpalette()
{
	return palkey_generic_menu_default;
}

const char *ht_menu_frame::defaultpaletteclass()
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
 *	CLASS ht_context_menu_window_body
 */

void ht_context_menu_window_body::init(Bounds *b, ht_context_menu *Menu)
{
	ht_view::init(b, VO_OWNBUFFER | VO_SELECTABLE, 0);
	VIEW_DEBUG_NAME("ht_context_menu_window_body");
	context_menu = Menu;
	selected = next_selectable(-1);
}

void ht_context_menu_window_body::done()
{
	ht_view::done();
}

const char *ht_context_menu_window_body::defaultpalette()
{
	return palkey_generic_menu_default;
}

const char *ht_context_menu_window_body::defaultpaletteclass()
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
		switch (e->type) {
			case CME_ENTRY: if (i==selected) {
				if (e->entry.active) {
					vcp c = getcolor(palidx_generic_text_selected);
					fill(0, i, size.w, 1, c, ' ');
					buf->nprint(1, i, c, e->entry.name, size.w);
					if (e->entry.comment) buf->nprint(size.w-1-strlen(e->entry.comment), i, c, e->entry.comment, size.w);
					if (e->entry.shortcut) buf->printChar(e->entry.shortcut-e->entry.name+1, i, getcolor(palidx_generic_text_shortcut_selected), *e->entry.shortcut);
				} else {
					vcp c = getcolor(palidx_generic_text_disabled);
					fill(0, i, size.w, 1, c, ' ');
					buf->nprint(1, i, c, e->entry.name, size.w);
					if (e->entry.comment) buf->nprint(size.w-1-strlen(e->entry.comment), i, c, e->entry.comment, size.w);
				}
			} else {
				if (e->entry.active) {
					vcp c = getcolor(palidx_generic_text_focused);
					buf->nprint(1, i, c, e->entry.name, size.w);
					if (e->entry.comment) buf->nprint(size.w-1-strlen(e->entry.comment), i, c, e->entry.comment, size.w);
					if (e->entry.shortcut) buf->printChar(e->entry.shortcut-e->entry.name+1, i, getcolor(palidx_generic_text_shortcut), *e->entry.shortcut);
				} else {
					vcp c = getcolor(palidx_generic_text_disabled);
					buf->nprint(1, i, c, e->entry.name, size.w);
					if (e->entry.comment) buf->nprint(size.w-1-strlen(e->entry.comment), i, c, e->entry.comment, size.w);
				}
			}
			break;
			case CME_SEPARATOR:
				fill(0, i, size.w, 1, getcolor(palidx_generic_text_focused), GC_1HLINE, CP_GRAPHICAL);
				break;
			case CME_SUBMENU: {
				const char *n = e->submenu->get_name();
				const char *s = e->submenu->get_shortcut();
				vcp c;
				if (i == selected) {
					c = getcolor(palidx_generic_text_selected);
				} else {
					c = getcolor(palidx_generic_text_focused);
				}
				fill(0, i, size.w, 1, c, ' ');
				buf->nprint(1, i, c, n, size.w);
				if (s) buf->printChar(n-s+1, i, getcolor(palidx_generic_text_shortcut), *s);
				buf->printChar(size.w-2, i, c, GC_ARROW_RIGHT, CP_GRAPHICAL);
				break;
			}
		}
		e = context_menu->enum_entry_next();
	}
}

void ht_context_menu_window_body::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Up:
				selected = prev_selectable(selected);
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				selected = next_selectable(selected);
				dirtyview();
				clearmsg(msg);
				return;
			case K_Control_PageUp:
			case K_PageUp:
			case K_Home:
				selected = next_selectable(-1);
				dirtyview();
				clearmsg(msg);
				return;
			case K_Control_PageDown:
			case K_PageDown:
			case K_End:
				selected = prev_selectable(0);
				dirtyview();
				clearmsg(msg);
				return;
		}
		/* shortcuts */
		int k = msg->data1.integer;
//		int k = htalt2key(msg->data_int);
//		if (k != -1) {
			int c=context_menu->count();
			ht_context_menu_entry *e = context_menu->enum_entry_first();
			for (int i=0; i < c; i++) {
				const char *shortcut = NULL;
				if (e->type == CME_ENTRY && e->entry.shortcut) {
					shortcut = e->entry.shortcut;
				} else if (e->type == CME_SUBMENU && e->submenu->get_shortcut()) {
					shortcut = e->submenu->get_shortcut();
				}
				if (shortcut) {
					int s = *shortcut;
					if (s >= 'A' && s <= 'Z') s += 'a'-'A';
					if (s==k) {
						selected=i;
						((ht_dialog*)group->group)->sendmsg(msg_button_pressed, button_ok);
						dirtyview();
						clearmsg(msg);
						return;
					}
				}					
				e = context_menu->enum_entry_next();
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
		if (e->type == CME_ENTRY || e->type == CME_SUBMENU) return s;
		s++;
		if (s > c-1) s=0;
	}
	return to;
}

int ht_context_menu_window_body::prev_selectable(int to)
{
	int s = to-1;
	int c = context_menu->count();
	if (s < 0) s = c-1;
	while (s != to) {
		ht_context_menu_entry *e=context_menu->get_entry(s);
		if (e->type == CME_ENTRY || e->type == CME_SUBMENU) return s;
		s--;
		if (s < 0) s = c-1;
	}
	return to;
}

void ht_context_menu_window_body::getdata(ObjectStream &s)
{
	PUT_INT32D(s, selected);
}

void ht_context_menu_window_body::setdata(ObjectStream &s)
{
	GET_INT32D(s, selected);
}

/*
 *	CLASS ht_menu_window
 */

void ht_menu_window::init(Bounds *b, ht_context_menu *m)
{
	ht_dialog::init(b, 0, 0);
	VIEW_DEBUG_NAME("ht_menu_window");

	Bounds c=*b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	menu = m;
	body=new ht_menu_window_body();
	body->init(&c, menu);
	insert(body);
}

void ht_menu_window::done()
{
	ht_dialog::done();
}

void ht_menu_window::getdata(ObjectStream &s)
{
	body->getdata(s);
}

void ht_menu_window::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_button_pressed) {
		switch (msg->data1.integer) {
			case button_ok: {
				ht_menu_window_data a;
				ViewDataBuf vdb(this, &a, sizeof a);
				int curentry = a.selected;
				ht_context_menu_entry *e = menu->get_entry(a.selected);
				if (e->type == CME_ENTRY && e->entry.active) {
					dirtyview();
					htmsg m;
					m.msg = e->entry.command;
					m.type = mt_empty;
					((ht_app*)app)->queuemsg(app, m);
//					app->sendmsg(e->entry.command);
				} else if (e->type == CME_SUBMENU) {
					if (execute_submenu(menu->xpos+2, 3+curentry, e->submenu)) {
						sendmsg(msg_button_pressed, button_cancel);
					}
					clearmsg(msg);
				}
				break;
			}
		}
	}
	ht_dialog::handlemsg(msg);
}

void ht_menu_window::setdata(ObjectStream &s)
{
	body->setdata(s);
}

/*
 *	CLASS ht_menu_window_body
 */

void ht_menu_window_body::init(Bounds *b, ht_context_menu *menu)
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
	if (msg->msg == msg_keypressed) {
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
