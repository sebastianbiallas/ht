/*
 *	HT Editor
 *	htobj.cc
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

#include "atom.h"
#include "cmds.h"
#include "htapp.h"
#include "htctrl.h"
#include "htdebug.h"
#include "keyb.h"
#include "htmenu.h"
#include "htobj.h"
#include "htpal.h"
#include "htreg.h"
#include "strtools.h"
#include "snprintf.h"
#include "store.h"
#include "tools.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define ATOM_HT_VIEW		MAGIC32("OBJ\0")
#define ATOM_HT_GROUP		MAGIC32("OBJ\1")
#define ATOM_HT_XGROUP		MAGIC32("OBJ\2")
#define ATOM_HT_WINDOW		MAGIC32("OBJ\3")
#define ATOM_HT_FRAME		MAGIC32("OBJ\4")
#define ATOM_HT_SCROLLBAR	MAGIC32("OBJ\5")

#define DEFAULT_VIEW_MIN_WIDTH 	25
#define DEFAULT_VIEW_MIN_HEIGHT	6

static void bounds_and(Bounds *a, Bounds *b)
{
	if (b->x > a->x) {
		a->w -= b->x-a->x;
		a->x = b->x;
	}
	if (b->y > a->y) {
		a->h -= b->y-a->y;
		a->y = b->y;
	}
	if (a->x + a->w > b->x+b->w) a->w -= a->x + a->w - b->x - b->w;
	if (a->y + a->h > b->y+b->h) a->h -= a->y + a->h - b->y - b->h;
	if (a->w < 0) a->w = 0;
	if (a->h < 0) a->h = 0;
}

void clearmsg(htmsg *msg)
{
	msg->msg = msg_empty;
	msg->type = mt_empty;
}

/*
 *	CLASS ht_text
 */

void ht_text::settext(const char *text)
{
}

/*
 *	CLASS ht_view
 */

void ht_view::init(Bounds *b, int o, const char *d)
{
	Object::init();
	VIEW_DEBUG_NAME("ht_view");
	desc = ht_strdup(d);
	group = NULL;
	focused = false;
	browse_idx = 0;
	view_is_dirty = true;
	size = *b;
	prev = NULL;
	next = NULL;
	setoptions(o);
	buf = NULL;
	enabled = true;

	growmode = MK_GM(GMH_LEFT, GMV_TOP);
	
//	Bounds rel(0, 0, b->w, b->h);
	if (options & VO_OWNBUFFER) {
		buf = new BufferedRDisplay(size);
	} else {
		buf = new SystemRDisplay(screen, size);
	}

	g_hdist = 0;
	g_vdist = 0;

	setbounds(b);

	pal.data = NULL;
	pal.size = 0;

	pal_class = defaultpaletteclass();
	pal_name = defaultpalette();

	reloadpalette();
}

void ht_view::done()
{
	free(desc);
	free(pal.data);
	delete buf;
	Object::done();
}

int ht_view::aclone()
{
	return (group && group->isaclone(this));
}

#if 0
int ht_view::buf_lprint(int aX, int aY, int c, int l, const char *text, Codepage cp)
{
	if (y+aY >= vsize.y && y+aY < vsize.y+vsize.h) {
		if (x+aX+l > vsize.x+vsize.w) l = vsize.x+vsize.w-size.x-aX;
		if (x+aX-vsize.x < 0) {
			int kqx = -x-aX+vsize.x;
			for (int i=0; i < kqx; i++) {
				if (!*text) return 0;
				text++;
				aX++;
				l--;
			}
		}
		return (l > 0) ? buf->b_lprint(x+aX, y+aY, c, l, text) : 0;
	}
	return 0;
}

int ht_view::buf_lprintw(int aX, int aY, int c, int l, const AbstractChar *text, Codepage cp)
{
	if (size.y+aY >= vsize.y && size.y+aY < vsize.y+vsize.h)) {
		if (x+aX+l > vsize.x+vsize.w) l=vsize.x+vsize.w-x-aX;
		if (x+aX-vsize.x < 0) {
			int kqx = -x-aX+vsize.x;
			for (int i=0; i < kqx; i++) {
				if (!*text) return 0;
				text++;
				x++;
				l--;
			}
		}
		return (l>0) ? buf->lprintw(size.x+x, size.y+y, c, l, text) : 0;
	}
	return 0;
}

int ht_view::buf_print(int x, int y, int c, const char *text, Codepage cp)
{
	if ((size.y+y>=vsize.y) && (size.y+y<vsize.y+vsize.h)) {
		int l=vsize.x+vsize.w-x-size.x;
		if (size.x+x-vsize.x<0) {
			int kqx=-size.x-x+vsize.x;
			for (int i=0; i<kqx; i++) {
				if (!*text) return 0;
				text++;
				x++;
				l--;
			}
		}
		return (l>0) ? buf->b_lprint(size.x+x, size.y+y, c, l, text) : 0;
	}
	return 0;
}

void ht_view::buf_printchar(int x, int y, int c, int ch, Codepage cp)
{
	if (pointvisible(size.x+x, size.y+y)) buf->b_printchar(size.x+x, size.y+y, c, ch, cp);
}

int ht_view::buf_printf(int x, int y, int c, CodePage cp, const char *format, ...)
{
	char buf[256];	/* secure */
	va_list arg;
	va_start(arg, format);
	ht_vsnprintf(buf, sizeof buf, format, arg);
	va_end(arg);
	return buf_print(x, y, c, buf, cp);
}

int ht_view::buf_printw(int x, int y, int c, const AbstractChar *text, Codepage cp)
{
	if ((size.y+y>=vsize.y) && (size.y+y<vsize.y+vsize.h)) {
		int l=vsize.x+vsize.w-x-size.x;
		if (size.x+x-vsize.x<0) {
			int kqx=-size.x-x+vsize.x;
			for (int i=0; i<kqx; i++) {
				if (!*text) return 0;
				text++;
				x++;
				l--;
			}
		}
		return (l>0) ? buf->b_lprintw(size.x+x, size.y+y, c, l, text) : 0;
	}
	return 0;
}
#endif

int ht_view::childcount() const
{
	return 1;
}

void ht_view::cleanview()
{
	view_is_dirty=0;
}

void ht_view::clear(int c)
{
	buf->fill(0, 0, size.w, size.h, c, ' ');
}

void ht_view::clipbounds(Bounds *b)
{
	Bounds c;
	getbounds(&c);
	bounds_and(b, &c);
	bounds_and(b, &vsize);
}

void ht_view::config_changed()
{
	reloadpalette();
	dirtyview();
}

int ht_view::countselectables()
{
	return (options & VO_SELECTABLE) ? 1 : 0;
}

int ht_view::datasize()
{
	return 0;
}

const char *ht_view::defaultpalette()
{
	return palkey_generic_window_default;
}

const char *ht_view::defaultpaletteclass()
{
	return palclasskey_generic;
}

void ht_view::dirtyview()
{
	view_is_dirty=1;
}

void ht_view::disable()
{
	enabled=0;
}

void ht_view::disable_buffering()
{
	if (options & VO_OWNBUFFER) {
		delete buf;
//		Bounds rel(0, 0, size.w, size.h);
		buf = new SystemRDisplay(screen, size);
		setoptions(options & ~VO_OWNBUFFER);
	}
}

void ht_view::draw()
{
}

void ht_view::enable()
{
	enabled=1;
}

void ht_view::enable_buffering()
{
	if (!(options & VO_OWNBUFFER)) {
		delete buf;
//		Bounds rel(0, 0, size.w, size.h);
		buf = new BufferedRDisplay(size);
		setoptions(options | VO_OWNBUFFER);
	}
}

static bool view_line_exposed(ht_view *v, int y, int x1, int x2)
{
	ht_group *g=v->group;
	while (g) {
		if (y >= g->size.y && y < g->size.y+g->size.h) {
			if (x1 < g->size.x) x1 = g->size.x;
			if (x2 > g->size.x + g->size.w) x2 = g->size.x+g->size.w;
			ht_view *n = g->first;
			while (n && n!=v) n=n->next;
			if (n) {
				n=n->next;
				if (n)
				while (n) {
					if (!(n->options & VO_TRANSPARENT_CHARS)) {
						if ((y>=n->size.y) && (y<n->size.y+n->size.h)) {
							if (n->size.x<=x1) {
								if (n->size.x+n->size.w>=x2) {
									return 0;
								} else if (n->size.x+n->size.w>x1) {
									x1=n->size.x+n->size.w;
								}
							} else if (n->size.x<=x2) {
								if (n->size.x+n->size.w<x2) {
									if (!view_line_exposed(n, y, x1, n->size.x)) return 0;
									x1=n->size.x+n->size.w;
								} else {
									x2=n->size.x;
								}
							}
						}
					}
					n=n->next;
				}
			}
		} else break;
		v=g;
		g=g->group;
	}
	return 1;
}

int ht_view::enum_start()
{
	return 0;
}

ht_view *ht_view::enum_next(int *handle)
{
	return 0;
}

bool ht_view::exposed()
{
#if 1
	for (int y=0; y < size.h; y++) {
		if (view_line_exposed(this, size.y+y, size.x, size.x+size.w)) return 1;
	}
	return 0;
#else
	return 1;
#endif
}

void ht_view::fill(int x, int y, int w, int h, int c, char chr, Codepage cp)
{
	Bounds b(x+size.x, y+size.y, w, h);
	bounds_and(&b, &vsize);
	buf->fill(b.x-size.x, b.y-size.y, b.w, b.h, c, chr, cp);
}

bool ht_view::focus(ht_view *view)
{
	if (view == this) {
		if (!focused) receivefocus();
		return true;
	}
	return false;
}

void ht_view::getbounds(Bounds *b)
{
	*b = size;
}

vcp ht_view::getcolor(uint index)
{
	return getcolorv(&pal, index);
}

void ht_view::getminbounds(int *width, int *height)
{
	*width = DEFAULT_VIEW_MIN_WIDTH;
	*height = DEFAULT_VIEW_MIN_HEIGHT;
}

struct databufdup_s {
	MemMapFile *f;
	ObjectStreamNative *s;
};

void ht_view::databuf_free(void *handle)
{
	databufdup_s *s = (databufdup_s*)handle;	
	delete s->s;
	delete s->f;
	delete s;
}

void *ht_view::databuf_get(void *buf, int bufsize)
{
	MemMapFile *f = new MemMapFile(buf, bufsize);	
	ObjectStreamNative *s = new ObjectStreamNative(f, false, true);
	
	getdata(*s);

	databufdup_s *q = new databufdup_s;
	q->f = f;
	q->s = s;
	return q;
}

void ht_view::databuf_set(void *buf, int bufsize)
{
	ConstMemMapFile f(buf, bufsize);
	ObjectStreamNative s(&f, false, true);
	setdata(s);	
}

void ht_view::getdata(ObjectStream &s)
{
}

ht_view *ht_view::getfirstchild()
{
	return 0;
}

uint ht_view::getnumber()
{
	return 0;
}

const char *ht_view::getpalette()
{
	return pal_name;
}

ht_view *ht_view::getselected()
{
	return this;
}

void ht_view::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_draw:
		redraw();
		return;
	case msg_dirtyview:
		dirtyview();
		if ((msg->type & mt_broadcast) == 0) clearmsg(msg);
		return;
	case msg_config_changed:
		config_changed();
//		clearmsg(msg);
		return;
	}
}

void ht_view::hidecursor()
{
	buf->setCursorMode(CURSOR_OFF);
	screen->setCursorMode(CURSOR_OFF);
}

int ht_view::isaclone(ht_view *view)
{
	return (view==this) && (countselectables()==1);
}

int ht_view::isviewdirty()
{
	return view_is_dirty;
}

void ht_view::load(ObjectStream &s)
{
/*     s->get_bool(enabled, NULL);
	s->get_bool(focused, NULL);
	s->get_int_dec(options, 4, NULL);
	s->get_int_dec(browse_idx, 4, NULL);
	s->get_string(desc, NULL);
	get_bounds(s, &size);
	get_bounds(s, &vsize);
	s->get_string(pal_class, NULL);
	s->get_string(pal_name, NULL);
	s->get_int_dec(growmode, 4, NULL);*/
}

void ht_view::move(int rx, int ry)
{
	size.x += rx;
	size.y += ry;
	buf->move(rx, ry);
	vsize = size;
	if (group) group->clipbounds(&vsize);
	app->clipbounds(&vsize);
}

ObjectID ht_view::getObjectID() const
{
	return ATOM_HT_VIEW;
}

bool ht_view::pointvisible(int x, int y)
{
	x += size.x;
	y += size.y;
	return (x >= vsize.x && y >= vsize.y && x < vsize.x+vsize.w && y < vsize.y+vsize.h);
}

void ht_view::receivefocus()
{
	dirtyview();
	focused = true;
}

void ht_view::redraw()
{
	if (exposed()) {
		if (options & VO_OWNBUFFER) {
			if (isviewdirty()) {
				draw();
				cleanview();
			}
			screen->copyFromDisplay(*buf, size.x, size.y, vsize);
		} else {
			draw();
			cleanview();
		}
	}
}

void ht_view::resize(int sx, int sy)
{
	if (options & VO_RESIZE) {
		int min_width, min_height;
		getminbounds(&min_width, &min_height);
		if (size.w+sx <= min_width) sx = min_width - size.w;
		if (size.h+sy <= min_height) sy = min_height - size.h;
		size.w += sx;
		size.h += sy;
		buf->resize(sx, sy);
	}
	vsize = size;
	if (group) group->clipbounds(&vsize);
	app->clipbounds(&vsize);
}

void ht_view::releasefocus()
{
	dirtyview();
	hidecursor();
	focused=0;
}

void ht_view::reloadpalette()
{
	if (pal.data) {
		free(pal.data);
		pal.data=0;
	}	    
	load_pal(pal_class, pal_name, &pal);
}

void ht_view::relocate_to(ht_view *view)
{
	Bounds b;
	view->getbounds(&b);
	move(b.x, b.y);
}

int ht_view::select(ht_view *view)
{
	return (view==this);
}

void ht_view::selectfirst()
{
}

void ht_view::selectlast()
{
}

void ht_view::sendmsg(htmsg *msg)
{
	if (enabled) handlemsg(msg);
}

void ht_view::sendmsg(int msg, void *data1, void *data2)
{
	htmsg m;
	m.msg=msg;
	m.type=mt_empty;
	m.data1.ptr=data1;
	m.data2.ptr=data2;
	sendmsg(&m);
}

void ht_view::sendmsg(int msg, int data1, int data2)
{
	htmsg m;
	switch (msg) {
	case msg_empty:
		return;
	case msg_draw:
	case msg_dirtyview:
		m.msg = msg;
		m.type = mt_broadcast;
		m.data1.integer = data1;
		m.data2.integer = data2;
		break;
	default:
		m.msg = msg;
		m.type = mt_empty;
		m.data1.integer = data1;
		m.data2.integer = data2;
		break;
	}
	sendmsg(&m);
}

void ht_view::setbounds(Bounds *b)
{
	size = *b;
	setvisualbounds(b);
}

void ht_view::setvisualbounds(Bounds *b)
{
	vsize = *b;
//	Bounds rel(0, 0, b->w, b->h);
	buf->setBounds(*b);
}

void ht_view::setcursor(int x, int y, CursorMode c)
{
	if (pointvisible(x, y)) {
		buf->setCursor(x, y, c);
		screen->setCursor(size.x+x, size.y+y, c);
	} else {
		buf->setCursorMode(CURSOR_OFF);
		screen->setCursorMode(CURSOR_OFF);
	}
}

void ht_view::setdata(ObjectStream &s)
{
}

void ht_view::setgroup(ht_group *_group)
{
	group=_group;
}

void ht_view::setnumber(uint number)
{
}

void ht_view::setoptions(int Options)
{
	options = Options;
}

void ht_view::setpalette(const char *Pal_name)
{
	pal_name = Pal_name;
	reloadpalette();
}

void ht_view::setpalettefull(const char *_pal_name, const char *_pal_class)
{
	pal_class=_pal_class;
	setpalette(pal_name);
}

void	ht_view::store(ObjectStream &s) const
{
/*	s->putBool(enabled, NULL);
	s->putBool(focused, NULL);
	s->putIntDec(options, 4, NULL);
	s->putIntDec(browse_idx, 4, NULL);
	s->putString(desc, NULL);
	put_bounds(s, &size);
	put_bounds(s, &vsize);
	s->putString(pal_class, NULL);
	s->putString(pal_name, NULL);
	s->putIntDec(growmode, 4, NULL);*/
}

void ht_view::unrelocate_to(ht_view *view)
{
	Bounds b;
	view->getbounds(&b);
	b.x=-b.x;
	b.y=-b.y;
	move(b.x, b.y);
}

/*
 *	CLASS ht_group
 */

void ht_group::init(Bounds *b, int options, const char *desc)
{
	first=0;
	current=0;
	last=0;
	ht_view::init(b, options, desc);
	VIEW_DEBUG_NAME("ht_group");
	view_count=0;
	shared_data=0;

	growmode = MK_GM(GMH_FIT, GMV_FIT);
}

void ht_group::done()
{
	ht_view *a, *b;
	a=first;
	while (a) {
		b=a->next;
		a->done();
		delete a;
		a=b;
	}
	ht_view::done();
}

int ht_group::childcount() const
{
	return view_count;
}

int ht_group::countselectables()
{
	int c=0;
	ht_view *v=first;
	while (v) {
		c+=v->countselectables();
		v=v->next;
	}
	return c;
}

int ht_group::datasize()
{
	uint size=0;
	ht_view *v=first;
	while (v) {
		size+=v->datasize();
		v=v->next;
	}
	return size;
}

int ht_group::enum_start()
{
	return -1;
}

ht_view *ht_group::enum_next(int *handle)
{
	int lowest = 0x7fffffff;
	ht_view *view = 0;

	ht_view *v = first;
	while (v) {
		if (v->browse_idx > *handle && v->browse_idx < lowest) {
			lowest = v->browse_idx;
			view = v;
		}
		v = v->next;
	}
	*handle = lowest;
	return view;
}

bool ht_group::focus(ht_view *view)
{
	ht_view *v = first;
	while (v) {
		if (v->focus(view)) {
			releasefocus();
			current = v;
			putontop(v);
			receivefocus();
			return true;
		}
		v = v->next;
	}
	return ht_view::focus(view);
}

bool ht_group::focusnext()
{
	if (!current) return false;
	int i = current->browse_idx;
	bool r = (options & VO_SELBOUND);
	ht_view *x = NULL;
	while (true) {
		i++;
		if (i > view_count-1) i=0;
		if (i == current->browse_idx) break;
		ht_view *y = get_by_browse_idx(i);
		if (y && (y->options & VO_SELECTABLE)) {
			x = y;
			break;
		}
	}
	if (i < current->browse_idx && !aclone() && !r) {
		return false;
	}
	if (x) {
		x->selectfirst();
		focus(x);
		return true;
	}
	return r;
}

bool ht_group::focusprev()
{
	if (!current) return false;
	int i = current->browse_idx;
	bool r = (options & VO_SELBOUND);
	if (!i && !aclone() && !r) {
		return false;
	}
	while (true) {
		i--;
		if (i<0) i=view_count-1;
		if (i==current->browse_idx) break;
		ht_view *v=get_by_browse_idx(i);
		if (v && (v->options & VO_SELECTABLE)) {
			v->selectlast();
			focus(v);
			return true;
		}
	}
	return r;
}

ht_view *ht_group::get_by_browse_idx(int i)
{
	ht_view *v=first;
	while (v) {
		if (v->browse_idx==i) return v;
		v=v->next;
	}
	return 0;
}

void ht_group::getdata(ObjectStream &s)
{
	ht_view *v;
	int h = enum_start();
	while ((v = enum_next(&h))) {
		v->getdata(s);
	}
}

ht_view *ht_group::getselected()
{
	if (current) return current->getselected(); else return NULL;
}

ht_view *ht_group::getfirstchild()
{
	return first;
}

void ht_group::getminbounds(int *width, int *height)
{
	ht_view::getminbounds(width, height);
	ht_view *v = first;
	while (v) {
		if (v->options & VO_RESIZE) {
			int w, h;
			v->getminbounds(&w, &h);
			w += v->size.x + size.w - (v->size.x + v->size.w);
			h += v->size.y + size.h - (v->size.y + v->size.h);
			uint gmh = GET_GM_H(v->growmode);
			uint gmv = GET_GM_V(v->growmode);
			if (gmh == GMH_FIT && w > *width) *width = w;
			if (gmv == GMV_FIT && h > *height) *height = h;
		}
		v = v->next;
	}
}

void ht_group::handlemsg(htmsg *msg)
{
	if (!enabled) return;
	if (msg->type == mt_broadcast) {
		ht_view::handlemsg(msg);
		ht_view *v=first;
		while (v) {
			v->handlemsg(msg);
			v = v->next;
		}
	} else if (msg->type == mt_empty) {
		int msgtype = msg->type;
		ht_view *v;

		msg->type=mt_preprocess;
		v=first;
		while (v) {
			if (v->options & VO_PREPROCESS) {
				v->handlemsg(msg);
			}
			v=v->next;
		}

		msg->type=mt_empty;
		if (current) current->handlemsg(msg);

		msg->type=mt_postprocess;
		v=first;
		while (v) {
			if (v->options & VO_POSTPROCESS) {
				v->handlemsg(msg);
			}
			v=v->next;
		}

		msg->type=msgtype;
		ht_view::handlemsg(msg);
	} else if (msg->type == mt_preprocess) {
		ht_view *v;

		v=first;
		while (v) {
			if (v->options & VO_PREPROCESS) {
				v->handlemsg(msg);
			}
			v=v->next;
		}
	} else if (msg->type == mt_postprocess) {
		ht_view *v;

		v=first;
		while (v) {
			if (v->options & VO_POSTPROCESS) {
				v->handlemsg(msg);
			}
			v=v->next;
		}
	}

	if ((msg->type==mt_empty || msg->type==mt_broadcast) && (msg->msg == msg_keypressed)) {
		switch (msg->data1.integer) {
		case K_Left:
		case K_Shift_Tab: {
			if (focusprev()) {
				clearmsg(msg);
				dirtyview();
				return;
			}
			break;
		}
		case K_Right:
		case K_Tab: {
			if (focusnext()) {
				clearmsg(msg);
				dirtyview();
				return;
			}
			break;
		}
		}
	}
}

void ht_group::insert(ht_view *view)
{
	if (current) current->releasefocus();
	if (view->options & VO_PREPROCESS) setoptions(options | VO_PREPROCESS);
	if (view->options & VO_POSTPROCESS) setoptions(options | VO_POSTPROCESS);
	if (view->pal_class && pal_class && strcmp(view->pal_class, pal_class)==0) view->setpalette(pal_name);

	view->g_hdist=size.w - (view->size.x+view->size.w);
	view->g_vdist=size.h - (view->size.y+view->size.h);

	Bounds c;
	getbounds(&c);
	view->move(c.x, c.y);

	if (last) last->next=view;
	view->prev = last;
	view->next = NULL;
	last = view;
	if (!first) first = view;
	view->setgroup(this);
	view->browse_idx = view_count++;
	if (!current || (current && (!(current->options & VO_SELECTABLE)) && (view->options & VO_SELECTABLE))) {
		current = view;
	}
	if (current && (current->options & VO_SELECTABLE)) {
		if (focused) {
			focus(current);
		} else {
			select(current);
		}
	}
}

int ht_group::isaclone(ht_view *view)
{
	ht_view *v = first;
	while (v) {
		if (v != view && v->countselectables()) return 0;
		v = v->next;
	}
	return 1;
}

int ht_group::isviewdirty()
{
	ht_view *v = first;
	while (v) {
		if (v->isviewdirty()) return 1;
		v = v->next;
	}
	return 0;
}

void ht_group::load(ObjectStream &f)
{
}

void ht_group::move(int rx, int ry)
{
	ht_view::move(rx, ry);
	ht_view *v = first;
	while (v) {
		v->move(rx, ry);
		v = v->next;
	}
}

ObjectID ht_group::getObjectID() const
{
	return ATOM_HT_GROUP;
}

void ht_group::putontop(ht_view *view)
{
	if (view->next) {
		if (view->prev) view->prev->next=view->next; else first=view->next;
		view->next->prev=view->prev;
		view->prev=last;
		view->next=0;
		last->next=view;
		last=view;
	}
}

void ht_group::receivefocus()
{
	ht_view::receivefocus();
	if (current) current->receivefocus();
}

void ht_group::releasefocus()
{
	ht_view::releasefocus();
	if (current)
		current->releasefocus();
}

void ht_group::remove(ht_view *view)
{
	ht_view *n = view->next ? view->next : view->prev;
	if (n) {
		focus(n);
	} else {
		releasefocus();
		current = NULL;
	}
	
	Bounds c;
	getbounds(&c);
	view->move(-c.x, -c.y);
	
	if (view->prev) view->prev->next = view->next;
	if (view->next) view->next->prev = view->prev;
	if (first == view) first = first->next;
	if (last == view) last = last->prev;
}

void ht_group::reorder_view(ht_view *v, int rx, int ry)
{
	int px = 0, py = 0;
	int sx = 0, sy = 0;

	int gmv = GET_GM_V(v->growmode);
	int gmh = GET_GM_H(v->growmode);
	switch (gmh) {
	case GMH_LEFT:
		/* do nothing */
		break;
	case GMH_RIGHT:
		px = rx;
		break;
	case GMH_FIT:
		sx = rx;
		break;
	}
	
	switch (gmv) {
	case GMV_TOP:
		/* do nothing */
		break;
	case GMV_BOTTOM:
		py = ry;
		break;
	case GMV_FIT:
		sy = ry;
		break;
	}

	v->move(px, py);
	v->resize(sx, sy);
}

void ht_group::resize(int sx, int sy)
{
	int min_width, min_height;
	getminbounds(&min_width, &min_height);
	if (size.w+sx <= min_width) sx = min_width - size.w;
	if (size.h+sy <= min_height) sy = min_height - size.h;

	ht_view::resize(sx, sy);
	
	ht_view *v = first;
	while (v) {
		reorder_view(v, sx, sy);
		v = v->next;
	}
}

int ht_group::select(ht_view *view)
{
	ht_view *v = first;
	while (v) {
		if (v->select(view)) {
			current = v;
			putontop(v);
			return 1;
		}
		v=v->next;
	}
	return ht_view::select(view);
}

void ht_group::selectfirst()
{
	for (int i=0; i<view_count; i++) {
		ht_view *v=first;
		while (v) {
			if ((v->browse_idx==i) && (v->options & VO_SELECTABLE)) {
				select(v);
				return;
			}
			v=v->next;
		}
	}
}

void ht_group::selectlast()
{
	for (int i=view_count-1; i>=0; i--) {
		ht_view *v=first;
		while (v) {
			if ((v->browse_idx==i) && (v->options & VO_SELECTABLE)) {
				select(v);
				return;
			}
			v=v->next;
		}
	}
}

void ht_group::setdata(ObjectStream &s)
{
	ht_view *v;
	int h=enum_start();
	while ((v=enum_next(&h))) {
		v->setdata(s);
	}
}

void ht_group::setpalette(const char *pal_name)
{
	ht_view *v=first;
	while (v) {
		if (strcmp(pal_class, v->pal_class)==0) v->setpalette(pal_name);
		v=v->next;
	}
	ht_view::setpalette(pal_name);
}

void ht_group::store(ObjectStream &s) const
{
	ht_view::store(s);
	PUTX_INT32D(s, childcount(), "childcount");
	ht_view *v=first;
	while (v) {
		PUTX_OBJECT(s, v, "obj");
		v=v->next;
	}
}

/*
 *	CLASS ht_xgroup
 */

void ht_xgroup::init(Bounds *b, int options, const char *desc)
{
	ht_group::init(b, options, desc);
	VIEW_DEBUG_NAME("ht_xgroup");
	first=0;
	current=0;
	last=0;
}

void ht_xgroup::done()
{
	ht_group::done();
}

int ht_xgroup::countselectables()
{
	return current->countselectables();
}

void ht_xgroup::handlemsg(htmsg *msg)
{
	if ((msg->msg!=msg_draw) && (msg->type==mt_broadcast)) {
		ht_group::handlemsg(msg);
	} else {
		if (msg->msg==msg_complete_init) return;
		if (current) current->handlemsg(msg);
		ht_view::handlemsg(msg);
	}
}

int ht_xgroup::isaclone(ht_view *view)
{
	if (group) return group->isaclone(this);
	return 0;
}

void ht_xgroup::load(ObjectStream &s)
{
	ht_group::load(s);
}

ObjectID ht_xgroup::getObjectID() const
{
	return ATOM_HT_XGROUP;
}

void ht_xgroup::redraw()
{
	ht_view::redraw();
/* no broadcasts. */
	if (current) current->redraw();
}

void ht_xgroup::selectfirst()
{
	current->selectfirst();
}

void ht_xgroup::selectlast()
{
	current->selectlast();
}

void	ht_xgroup::store(ObjectStream &s) const
{
	ht_group::store(s);
}

/*
 *	CLASS ht_scrollbar
 */

bool scrollbar_pos(sint64 start, sint64 size, sint64 all, int *pstart, int *psize)
{
	if (!all) return false;
	if (start+size >= all) {
		if (size >= all) return false;
		*psize = (int)(((double)size)*100/all);
		*pstart = 100 - *psize;
	} else {
		*psize = (int)(((double)size)*100/all);
		*pstart = (int)(((double)start)*100/all);
	}
	return true;
}

void ht_scrollbar::init(Bounds *b, palette *p, bool isv)
{
	ht_view::init(b, VO_RESIZE, 0);
	VIEW_DEBUG_NAME("ht_scrollbar");
	
	pstart = 0;
	psize = 0;

	gpal = p;

	isvertical = isv;

	if (isvertical) {
		growmode = MK_GM(GMH_RIGHT, GMV_FIT);
	} else {
		growmode = MK_GM(GMH_FIT, GMV_BOTTOM);
	}

	enable();	// enabled by default
}

void ht_scrollbar::done()
{
	ht_view::done();
}

void ht_scrollbar::enable()
{
	enable_buffering();
	ht_view::enable();
	dirtyview();
}

void ht_scrollbar::disable()
{
	disable_buffering();
	ht_view::disable();
	dirtyview();
}

void ht_scrollbar::draw()
{
	if (enabled) {
		vcp color = getcolorv(gpal, palidx_generic_scrollbar);
//          vcp color2 = VCP(VCP_BACKGROUND(color), VCP_FOREGROUND(color));
		if (isvertical) {
			fill(0, 1, size.w, size.h-2, color, ' ');
			int e, s;
			e=((size.h-2)*psize)/100;
			if (pstart+psize>=100) {
				s=size.h-2-e;
			} else {
				s=((size.h-2)*pstart)/100;
			}
			if (!e) {
				if (s==size.h-2) s--;
				e=1;
			}
			fill(0, s+1, 1, e, color, GC_MEDIUM, CP_GRAPHICAL);
			buf->printChar(0, 0, color, GC_ARROW_UP, CP_GRAPHICAL);
			buf->printChar(0, size.h-1, color, GC_ARROW_DOWN, CP_GRAPHICAL);
		} else {
		}
	}
}

void ht_scrollbar::getminbounds(int *width, int *height)
{
	*width = 1;
	*height = 1;
}

ObjectID ht_scrollbar::getObjectID() const
{
	return ATOM_HT_SCROLLBAR;
}

void ht_scrollbar::setpos(int ps, int pz)
{
	pstart = ps;
	psize = pz;
	dirtyview();
}

/*
 *	CLASS ht_frame
 */

void ht_frame::init(Bounds *b, const char *desc, uint s, uint n)
{
	ht_view::init(b, VO_RESIZE, desc);
	VIEW_DEBUG_NAME("ht_frame");

	number = n;
	style = s;
	framestate = FST_UNFOCUSED;
	
	growmode = MK_GM(GMH_FIT, GMV_FIT);
}

void ht_frame::done()
{
	ht_view::done();
}

void ht_frame::draw()
{
	int cornerul, cornerur, cornerll, cornerlr;
	int lineh, linev;
	ht_window *w = (ht_window*)group;
	if (framestate != FST_MOVE && framestate != FST_RESIZE) {
		setframestate(w->focused ? FST_FOCUSED : FST_UNFOCUSED); 
	}
	if (style & FS_THICK) {
		cornerul = GC_2CORNER3;
		cornerur = GC_2CORNER0;
		cornerll = GC_2CORNER2;
		cornerlr = GC_2CORNER1;
		lineh = GC_2HLINE;
		linev = GC_2VLINE;
	} else {
		cornerul = GC_1CORNER3;
		cornerur = GC_1CORNER0;
		cornerll = GC_1CORNER2;
		cornerlr = GC_1CORNER1;
		lineh = GC_1HLINE;
		linev = GC_1VLINE;
	}

	vcp c = getcurcol_normal();

	/* "/...\" */
	buf->printChar(0, 0, c, cornerul, CP_GRAPHICAL);
	for (int i=1; i < size.w-1; i++) buf->printChar(i, 0, c, lineh, CP_GRAPHICAL);
	buf->printChar(0+size.w-1, 0, c, cornerur, CP_GRAPHICAL);

	/* "\.../" */
	buf->printChar(0, size.h-1, c, cornerll, CP_GRAPHICAL);
	for (int i=1; i < size.w-1; i++) buf->printChar(i, size.h-1, c, lineh, CP_GRAPHICAL);
/*	if (style & FS_RESIZE) {
		buf->printChar(size.w-1, size.h-1, getcurcol_killer(), GC_1CORNER1, CP_GRAPHICAL);
	} else {*/
		buf->printChar(size.w-1, size.h-1, c, cornerlr, CP_GRAPHICAL);
//     }

	/* "|", "|" */
	for (int i=1; i < size.h-1; i++) {
		buf->printChar(0, i, c, linev, CP_GRAPHICAL);
		buf->printChar(size.w-1, i, c, linev, CP_GRAPHICAL);
	}

	/* "[x]" */
	if (style & FS_KILLER) {
		buf->print(2, 0, c, "[ ]");
		buf->printChar(3, 0, getcurcol_killer(), GC_FILLED_QUAD, CP_GRAPHICAL);
	}

	/* e.g. "1" */
	int ns=0;
	if (style & FS_NUMBER) {
		int l = number;
		do {
			l = l/10;
			ns++;
		} while (l);
		buf->printf(size.w-4-ns, 0, c, CP_DEVICE, "%d", number);
		ns += 4;
	}

	/* <title> */
	const char *d;
	switch (framestate) {
	case FST_MOVE:
		d = (style & FS_RESIZE) ? "(moving) - hit space to resize" : "(moving)";
		break;
	case FST_RESIZE:
		d = (style & FS_MOVE) ? "(resizing) - hit space to move" : "(resizing)";
		break;
	default:
		d = desc;
	}
	int ks = (style & FS_KILLER) ? 4 : 0;
	ns++;
	if (d && (style & FS_TITLE)) {
		int l = strlen(d), k = 0;
		if (l > size.w-(5+ks+ns)) {
			k = l-(size.w-(6+ks+ns+2));
			if (size.w > 6+ks+ns+2) {
				d += k;
			} else {
				d = "";
			}
			buf->printf(2+ks, 0, c, CP_DEVICE, " ...%s ", d);
		} else {
			buf->printf((size.w-l-2)/2, 0, c, CP_DEVICE, " %s ", d);
		}
	}
}

vcp ht_frame::getcurcol_normal()
{
	switch (framestate) {
	case FST_FOCUSED:
		return getcolor(palidx_generic_frame_focused);
	case FST_UNFOCUSED:
		return getcolor(palidx_generic_frame_unfocused);
	case FST_MOVE:
	case FST_RESIZE:
		return getcolor(palidx_generic_frame_move_resize);
	}
	return 0;
}

vcp ht_frame::getcurcol_killer()
{
	return getcolor(palidx_generic_frame_killer);
}

uint ht_frame::getnumber()
{
	return number;
}

uint ht_frame::getstyle()
{
	return style;
}

ObjectID ht_frame::getObjectID() const
{
	return ATOM_HT_FRAME;
}

void ht_frame::setframestate(uint _framestate)
{
	framestate=_framestate;
	dirtyview();
}

void ht_frame::setnumber(uint _number)
{
	number=_number;
	dirtyview();
}

void ht_frame::setstyle(uint s)
{
	style=s;
}

void ht_frame::settext(const char *text)
{
	free(desc);
	desc = ht_strdup(text);
	dirtyview();
}

/*
 *	CLASS ht_window
 */

void	ht_window::init(Bounds *b, const char *desc, uint framestyle, uint num)
{
	ht_group::init(b, VO_SELECTABLE | VO_SELBOUND | VO_BROWSABLE, desc);
	VIEW_DEBUG_NAME("ht_window");
	number=num;
	hscrollbar=NULL;
	vscrollbar=NULL;
	pindicator=NULL;
	Bounds c=*b;
	c.x=0;
	c.y=0;
	frame=0;
	action_state=WAC_NORMAL;
	ht_frame *f=new ht_frame();
	f->init(&c, desc, framestyle, number);
	setframe(f);
}

void ht_window::done()
{
	pindicator=NULL;
	hscrollbar=NULL;
	vscrollbar=NULL;
	ht_group::done();
}

void ht_window::draw()
{
	vcp c=getcolor(palidx_generic_body);
	clear(c);
}

void ht_window::getclientarea(Bounds *b)
{
	getbounds(b);
	if (frame) {
		b->x++;
		b->y++;
		b->w-=2;
		b->h-=2;
	}
}

uint ht_window::getnumber()
{
	return number;
}

ht_frame *ht_window::getframe()
{
	return frame;
}

void ht_window::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed:
			if (action_state==WAC_MOVE) {
				if (options & VO_MOVE) {
					switch (msg->data1.integer) {
						case K_Up:
							if (size.y>group->size.y) move(0, -1);
							break;
						case K_Down:
							if (size.y<group->size.y+group->size.h-1) move(0, 1);
							break;
						case K_Left:
							if (size.x+size.w>group->size.x+1) move(-1, 0);
							break;
						case K_Right:
							if (size.x<group->size.x+group->size.w-1) move(1, 0);
							break;
						case K_Control_Up:
							if (size.y>group->size.y+5-1) move(0, -5); else
								move(0, group->size.y-size.y);
							break;
						case K_Control_Down:
							if (size.y<group->size.y+group->size.h-5) move(0, 5); else
								move(0, group->size.y+group->size.h-size.y-1);
							break;
						case K_Control_Left:
							if (size.x+size.w>group->size.x+5) move(-5, 0); else
								move(-(size.x+size.w)+group->size.x+1, 0);
							break;
						case K_Control_Right:
							if (size.x<group->size.x+group->size.w-5) move(5, 0); else
								move(group->size.x+group->size.w-size.x-1, 0);
							break;
					}
				}
			} else if (action_state == WAC_RESIZE) {
				if (options & VO_RESIZE) {
					int min_width, min_height;
					getminbounds(&min_width, &min_height);
					switch (msg->data1.integer) {
					case K_Up:
						if (size.h > min_height) resize(0, -1);
						break;
					case K_Down:
						if (size.h < group->size.h) resize(0, 1);
						break;
					case K_Left:
						if (size.x+size.w > 1 && size.w > min_width) resize(-1, 0);
						break;
					case K_Right:
						if (size.w < group->size.w) resize(1, 0);
						break;
					}
				}
			} else {
				if ((msg->data1.integer == K_Control_F5) ||
				 (msg->data1.integer == K_Control_R)) {
					sendmsg(cmd_window_resizemove);
				}
				break;
			}
			switch (msg->data1.integer) {
				case K_Escape:
				case K_Return:
					sendmsg(cmd_window_resizemove);
					break;
				case K_Space:
				case K_Control_F5:
					sendmsg(cmd_window_switch_resizemove);
					break;
			}
			app->sendmsg(msg_dirtyview, 0);
			clearmsg(msg);
			return;
		case cmd_window_resizemove: {
			bool b = (action_state == WAC_NORMAL);
			do {
				if (!next_action_state()) break;
			} while (b == (action_state == WAC_NORMAL));
			dirtyview();
			clearmsg(msg);
			return;
		}
		case cmd_window_switch_resizemove:
			do {
				if (!next_action_state()) break;
			} while (action_state == WAC_NORMAL);
			dirtyview();
			clearmsg(msg);
			return;
	}
	ht_group::handlemsg(msg);
}

void ht_window::insert(ht_view *view)
{
	if (frame) view->move(1, 1);
	ht_group::insert(view);
}

void ht_window::load(ObjectStream &s)
{
	ht_group::load(s);
}

bool ht_window::next_action_state()
{
#define wstate_count 3
	int ass[wstate_count] = { WAC_NORMAL, WAC_MOVE, WAC_RESIZE };
	int fss[wstate_count] = { FST_FOCUSED, FST_MOVE, FST_RESIZE };
	for (int i=0; i < wstate_count; i++) {
		if (action_state == ass[i]) {
			int p = i;
			while (++p != i) {
				if (p > wstate_count-1) p = 0;
				bool allowed = true;
				switch (ass[p]) {
					case WAC_MOVE: allowed = ((options & VO_MOVE) != 0); break;
					case WAC_RESIZE: allowed = ((options & VO_RESIZE) != 0); break;
				}
				if (allowed) {
					action_state = ass[p];
					if (frame) frame->setframestate(fss[p]);
					return (p != i);
				}
			}
			return false;
		}
	}
	return false;
}

ObjectID ht_window::getObjectID() const
{
	return ATOM_HT_WINDOW;
}

void ht_window::receivefocus()
{
	htmsg m;
	m.msg = msg_contextmenuquery;
	m.type = mt_empty;
	sendmsg(&m);
	if (m.msg == msg_retval) {
		ht_menu *q = (ht_menu*)((ht_app*)app)->menu;
		ht_context_menu *n = (ht_context_menu*)m.data1.ptr;
		if (q) {
			if (!q->set_local_menu(n)) {
				n->done();
				delete n;
			}
			q->sendmsg(msg_dirtyview);
		} else {
			n->done();
			delete n;
		}
	}
	ht_group::receivefocus();
	if (frame) frame->setstyle(frame->getstyle() | FS_THICK);
}

void ht_window::redraw()
{
	htmsg m;
	
	if (pindicator) {
		char buf[256];
		buf[0] = 0;
	
		m.msg = msg_get_pindicator;
		m.type = mt_empty;
		m.data1.integer = sizeof buf;
		m.data2.ptr = buf;
		sendmsg(&m);

		pindicator->settext(buf);
	}

	gsi_scrollbar_t p;

	if (hscrollbar) {
		p.pstart = 0;
		p.psize = 200;
		m.msg = msg_get_scrollinfo;
		m.type = mt_empty;
		m.data1.integer = gsi_hscrollbar;
		m.data2.ptr = &p;
		sendmsg(&m);

		if (p.psize >= 100) {
			hscrollbar->disable();
		} else {
			hscrollbar->enable();
			hscrollbar->setpos(p.pstart, p.psize);
		}
	}

	if (vscrollbar) {
		p.pstart = 0;
		p.psize = 200;
		m.msg = msg_get_scrollinfo;
		m.type = mt_empty;
		m.data1.integer = gsi_vscrollbar;
		m.data2.ptr = &p;
		sendmsg(&m);

		if (p.psize >= 100) {
			vscrollbar->disable();
		} else {
			vscrollbar->enable();
			vscrollbar->setpos(p.pstart, p.psize);
		}
	}

	ht_group::redraw();
}

void ht_window::releasefocus()
{
	ht_menu *q = (ht_menu*)((ht_app*)app)->menu;
	if (q) {
		q->delete_local_menu();
		q->sendmsg(msg_dirtyview);
	}

	if (frame) frame->setstyle(frame->getstyle() & (~FS_THICK));
	ht_group::releasefocus();
}

void ht_window::setframe(ht_frame *newframe)
{
	if (frame) {
		ht_group::remove(frame);
		frame->done();
		delete frame;
		frame = NULL;
	}
	if (newframe) {
		uint style=newframe->getstyle();
		if (style & FS_MOVE) options |= VO_MOVE; else options &= ~VO_MOVE;
		if (style & FS_RESIZE) options |= VO_RESIZE; else options &= ~VO_RESIZE;
		insert(newframe);
	} else {
		options &= ~VO_MOVE;
		options &= ~VO_RESIZE;
	}
	frame = newframe;
}

void ht_window::setnumber(uint aNumber)
{
	if (frame) frame->setnumber(aNumber);
	number = aNumber;
	dirtyview();
}

void ht_window::sethscrollbar(ht_scrollbar *s)
{
	if (hscrollbar) remove(hscrollbar);
	hscrollbar = s;
	insert(hscrollbar);
	putontop(hscrollbar);
}

void ht_window::setpindicator(ht_text *p)
{
	if (pindicator) remove(pindicator);
	pindicator = p;
	insert(pindicator);
	putontop(pindicator);
}

void ht_window::settitle(char *title)
{
	free(desc);
	desc = ht_strdup(title);
	if (frame) frame->settext(title);
}

void ht_window::setvscrollbar(ht_scrollbar *s)
{
	if (vscrollbar) remove(vscrollbar);
	vscrollbar = s;
	insert(vscrollbar);
	putontop(vscrollbar);
}

void	ht_window::store(ObjectStream &s) const
{
	ht_group::store(s);
}

/*
 *	CLASS ht_vbar
 */

void ht_vbar::draw()
{
	fill(0, 0, 1, size.h, getcolor(palidx_generic_body), GC_1VLINE, CP_GRAPHICAL);
}

/*
 *	CLASS ht_hbar
 */

void ht_hbar::draw()
{
	fill(0, 0, size.w, 1, getcolor(palidx_generic_body), GC_1HLINE, CP_GRAPHICAL);
}

/***/
BUILDER(ATOM_HT_VIEW, ht_view, Object);
BUILDER(ATOM_HT_GROUP, ht_group, ht_view);
BUILDER(ATOM_HT_XGROUP, ht_xgroup, ht_group);
BUILDER(ATOM_HT_WINDOW, ht_window, ht_group);
BUILDER(ATOM_HT_SCROLLBAR, ht_scrollbar, ht_view);

/*
 *	INIT
 */

bool init_obj()
{
	REGISTER(ATOM_HT_VIEW, ht_view);
	REGISTER(ATOM_HT_GROUP, ht_group);
	REGISTER(ATOM_HT_XGROUP, ht_xgroup);
	REGISTER(ATOM_HT_WINDOW, ht_window);
	REGISTER(ATOM_HT_SCROLLBAR, ht_scrollbar);
	return true;
}

/*
 *	DONE
 */

void done_obj()
{
	UNREGISTER(ATOM_HT_VIEW, ht_view);
	UNREGISTER(ATOM_HT_GROUP, ht_group);
	UNREGISTER(ATOM_HT_XGROUP, ht_xgroup);
	UNREGISTER(ATOM_HT_WINDOW, ht_window);
	UNREGISTER(ATOM_HT_FRAME, ht_frame);
	UNREGISTER(ATOM_HT_SCROLLBAR, ht_scrollbar);
}

