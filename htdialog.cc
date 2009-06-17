/*
 *	HT Editor
 *	htdialog.cc
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

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "htclipboard.h"
#include "htctrl.h"
#include "htdialog.h"
#include "hthist.h"
#include "htidle.h"
#include "keyb.h"
#include "htpal.h"
#include "snprintf.h"
#include "strtools.h"
#include "tools.h"

ht_queued_msg::ht_queued_msg(ht_view *aTarget, htmsg &aMsg)
{
	target = aTarget;
	msg = aMsg;
}

void ht_dialog_widget::getminbounds(int *width, int *height)
{
	*width = 1;
	*height = 1;
}

/*
 *	CLASS ht_dialog
 */

void ht_dialog::init(Bounds *b, const char *desc, uint framestyle)
{
	ht_window::init(b, desc, framestyle);
	VIEW_DEBUG_NAME("ht_dialog");
	options &= ~VO_SELBOUND;
	msgqueue = new Queue(true);
}

void ht_dialog::done()
{
	delete msgqueue;
	ht_window::done();
}

int ht_dialog::aclone()
{
	return 1;
}

const char *ht_dialog::defaultpalette()
{
	return palkey_generic_dialog_default;
}

ht_queued_msg *ht_dialog::dequeuemsg()
{
	return (ht_queued_msg*)msgqueue->deQueue();
}

void ht_dialog::draw()
{
	clear(getcolor(palidx_generic_body));
	ht_group::draw();
}

int ht_dialog::getstate(int *aReturn_val)
{
	if (aReturn_val) *aReturn_val = return_val;
	return state;
}

void ht_dialog::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_button_pressed) {
		switch (msg->data1.integer) {
		case button_cancel:
			setstate(ds_term_cancel, msg->data1.integer);
			clearmsg(msg);
			return;
		default:
			setstate(ds_term_ok, msg->data1.integer);
			clearmsg(msg);
			return;
		}
	}
	ht_window::handlemsg(msg);
	if (msg->msg == msg_keypressed) {
		switch (msg->data1.integer) {
		case K_Escape:
			setstate(ds_term_cancel, msg->data1.integer);
			clearmsg(msg);
			return;
		case K_Return:
			sendmsg(msg_button_pressed, button_ok);
			clearmsg(msg);
			return;
		case K_Control_O: {
		}
		}
	}
}

void do_modal_resize();

int ht_dialog::run(bool modal)
{
	ht_view *orig_focused=app->getselected(), *orig_baseview=baseview;
	int oldx, oldy;
	ht_view *drawer=modal ? this : app;
	screen->getCursor(oldx, oldy);
	setstate(ds_normal, 0);
	((ht_group*)app)->insert(this);
	((ht_group*)app)->focus(this);
	baseview = this;
	drawer->sendmsg(msg_draw, 0);
	screen->show();
	while (getstate(0) == ds_normal) {
		if (keyb_keypressed()) {
			ht_key k = keyb_getkey();
			sendmsg(msg_keypressed, k);
			drawer->sendmsg(msg_draw, 0);
			screen->show();
		}
		if (sys_get_winch_flag()) {
			do_modal_resize();
		}
		ht_queued_msg *q;
		while ((q = dequeuemsg())) {
			htmsg m = q->msg;
			q->target->sendmsg(&m);
			sendmsg(msg_draw);
			delete q;
		}
		if (!modal) do_idle();
	}
	int return_val;
	int state = getstate(&return_val);
	screen->setCursor(oldx, oldy);
	((ht_group*)app)->remove(this);
	app->focus(orig_focused);
	baseview = orig_baseview;
	if (state != ds_term_cancel) {
		return return_val;
	} else {
		return 0;
	}
}

void ht_dialog::queuemsg(ht_view *target, htmsg &msg)
{
	msgqueue->enQueue(new ht_queued_msg(target, msg));
}

void ht_dialog::setstate(int st, int retval)
{
	state = st;
	return_val = retval;
}

/*
 *	CLASS ht_cluster
 */

void ht_cluster::init(Bounds *b, ht_string_list *_strings)
{
	ht_view::init(b, VO_SELECTABLE | VO_OWNBUFFER | VO_POSTPROCESS, 0);
	VIEW_DEBUG_NAME("ht_cluster");
	strings=_strings;
	scount=strings->count();
	if (scount>32) scount=32;			/* cant use more than 32... */
	sel=0;
	for (int i=0; i<scount; i++) {
		const char *s = strings->get_string(i);
		s = strchr(s, '~');
		if (s) {
			shortcuts[i] = keyb_metakey((ht_key)*(s+1));
		} else shortcuts[i] = K_INVALID;
	}
}

void ht_cluster::done()
{
	delete strings;
	ht_view::done();
}

const char *ht_cluster::defaultpalette()
{
	return palkey_generic_dialog_default;
}

/*
 *	CLASS ht_checkboxes
 */

void ht_checkboxes::init(Bounds *b, ht_string_list *strings)
{
	ht_cluster::init(b, strings);
	VIEW_DEBUG_NAME("ht_checkboxes");
	state=0;
}

void ht_checkboxes::done()
{
	ht_cluster::done();
}

int ht_checkboxes::datasize()
{
	return sizeof (ht_checkboxes_data);
}

void ht_checkboxes::draw()
{
	clear(getcolor(focused ? palidx_generic_cluster_focused
		: palidx_generic_cluster_unfocused));
	int i=0;
	int vx=0, vy=0;
	int maxcolstrlen=0;
	while (i<scount) {
		int c=getcolor(palidx_generic_cluster_unfocused);
		if ((focused) && (sel==i)) c=getcolor(palidx_generic_cluster_focused);
		const char *s = strings->get_string(i);
		int slen = strlen(s);
		if (slen > maxcolstrlen) maxcolstrlen = slen;
		if ((1 << i) & state) {
			buf->print(vx, vy, c, "[X]");
		} else {
			buf->print(vx, vy, c, "[ ]");
		}
		int k=0, oc=c;
		for (int q=0; q < size.w-4; q++) {
			if (!*(s+q)) break;
			if (*(s+q)=='~') {
				c = getcolor(palidx_generic_cluster_shortcut);
				continue;
			} else {
				buf->printChar(vx+k+4, vy, c, *(s+q));
				k++;
			}
			c=oc;
		}
		i++;
		vy++;
		if (vy>=size.h) {
			vx+=maxcolstrlen+5;
			vy=0;
			maxcolstrlen=0;
		}
	}
}

void ht_checkboxes::getdata(ObjectStream &s)
{
	PUT_INT32D(s, state);
}

void ht_checkboxes::handlemsg(htmsg *msg)
{
	if (msg->type==mt_postprocess) {
		if (msg->msg==msg_keypressed) {
			for (int i=0; i<scount; i++) {
				if (shortcuts[i] != -1 && msg->data1.integer == shortcuts[i]) {
					sel = i;
					state = state ^ (1<<sel);
					app->focus(this);
					dirtyview();
					clearmsg(msg);
					return;
				}
			}
		}
	} else {
		if (msg->msg==msg_keypressed) {
			switch (msg->data1.integer) {
			case K_Left:
				sel -= size.h;
				if (sel < 0) sel=0;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Right:
				sel += size.h;
				if (sel >= scount) sel = scount-1;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Up:
				sel--;
				if (sel < 0) sel = 0;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				sel++;
				if (sel >= scount) sel = scount-1;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Space:
				state = state ^ (1<<sel);
				dirtyview();
				clearmsg(msg);
				return;
			}
		}
	}
	ht_cluster::handlemsg(msg);
}

void ht_checkboxes::setdata(ObjectStream &s)
{
	GET_INT32D(s, state);
	dirtyview();
}

/*
 *	CLASS ht_radioboxes
 */

void ht_radioboxes::init(Bounds *b, ht_string_list *strings)
{
	ht_cluster::init(b, strings);
	VIEW_DEBUG_NAME("ht_radioboxes");
}

void ht_radioboxes::done()
{
	ht_cluster::done();
}

int ht_radioboxes::datasize()
{
	return sizeof (ht_radioboxes_data);
}

void ht_radioboxes::draw()
{
	clear(getcolor(focused ? palidx_generic_cluster_focused
		: palidx_generic_cluster_unfocused));
	int i=0;
	int vx=0, vy=0;
	int maxcolstrlen=0;
	while (i<scount) {
		int c=getcolor(palidx_generic_cluster_unfocused);
		if ((focused) && (sel==i)) c=getcolor(palidx_generic_cluster_focused);
		const char *s=strings->get_string(i);
		int slen=strlen(s);
		if (slen>maxcolstrlen) maxcolstrlen=slen;
		buf->print(vx, vy, c, "( )");
		if (i==sel) {
			buf->printChar(vx+1, vy, c, GC_FILLED_CIRCLE, CP_GRAPHICAL);
		}
		buf->print(vx+4, vy, c, s);
		i++;
		vy++;
		if (vy>=size.h) {
			vx+=maxcolstrlen+5;
			vy=0;
			maxcolstrlen=0;
		}
	}
}

void ht_radioboxes::getdata(ObjectStream &s)
{
	PUT_INT32D(s, sel);
}

void ht_radioboxes::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Left:
				sel-=size.h;
				if (sel<0) sel=0;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Right:
				sel+=size.h;
				if (sel>=scount) sel=scount-1;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Up:
				sel--;
				if (sel<0) sel=0;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				sel++;
				if (sel>=scount) sel=scount-1;
				dirtyview();
				clearmsg(msg);
				return;
/*			case K_Space:
				state=state^(1<<sel);
				dirtyview();
				clearmsg(msg);
				break;*/
		}
	}
	ht_cluster::handlemsg(msg);
}

void ht_radioboxes::setdata(ObjectStream &s)
{
	GET_INT32D(s, sel);
}


/*
 *	CLASS ht_history_listbox
 */
void ht_history_listbox::init(Bounds *b, List *hist)
{
	history = hist;
	ht_listbox::init(b);
}

int  ht_history_listbox::calcCount()
{
	return history->count();
}

void *ht_history_listbox::getFirst()
{
	if (history->count()) {
		return (void*)1;
	} else {
		return NULL;
	}
}

void *ht_history_listbox::getLast()
{
	if (history->count()) {
		return (void*)(history->count());
	} else {
		return NULL;
	}
}

void *ht_history_listbox::getNext(void *entry)
{
	unsigned long e=(unsigned long)entry;
	if (!e) return NULL;
	if (e < history->count()) {
		return (void*)(e+1);
	} else {
		return NULL;
	}
}

void *ht_history_listbox::getPrev(void *entry)
{
	unsigned long e=(unsigned long)entry;
	if (e > 1) {
		return (void*)(e-1);
	} else {
		return NULL;
	}
}

const char *ht_history_listbox::getStr(int col, void *entry)
{
	return ((ht_history_entry*)(*history)[(long)entry-1])->desc;
}

void ht_history_listbox::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_Delete: {
					int p = pos;
					cursorUp(1);
					history->del(history->findByIdx(p));
					update();
					if (p) cursorDown(1);
					dirtyview();
					clearmsg(msg);
					break;
				}
			}
		break;
	}
	ht_listbox::handlemsg(msg);
}

void *ht_history_listbox::quickfind(const char *s)
{
	void *item = getFirst();
	int slen = strlen(s);
	while (item && (ht_strncmp(getStr(0, item), s, slen)!=0)) {
		item = getNext(item);
	}
	return item;
}

char	*ht_history_listbox::quickfindCompletition(const char *s)
{
	void *item = getFirst();
	char *res = NULL;
	int slen = strlen(s);
	while (item) {
		if (ht_strncmp(getStr(0, item), s, slen)==0) {
			if (!res) {
				res = ht_strdup(getStr(0, item));
			} else {
				int a = ht_strccomm(res, getStr(0, item));
				res[a] = 0;
			}
		}
		item = getNext(item);
	}
	return res;
}


/*
 *	CLASS ht_history_popup_dialog
 */

void ht_history_popup_dialog::init(Bounds *b, List *hist)
{
	history = hist;
	ht_listpopup_dialog::init(b, "history");
}

void ht_history_popup_dialog::getdata(ObjectStream &s)
{
	// FIXME: public member needed:
	PUTX_INT32D(s, listbox->pos, NULL);
	if (history->count()) {
		PUTX_STRING(s, ((ht_history_entry*)(*history)[listbox->pos])->desc, NULL);
	} else {
		PUTX_STRING(s, NULL, NULL);
	}
}

void ht_history_popup_dialog::init_text_listbox(Bounds *b)
{
	listbox = new ht_history_listbox();
	((ht_history_listbox *)listbox)->init(b, history);
	insert(listbox);
}

void ht_history_popup_dialog::setdata(ObjectStream &s)
{
}

/*
 *	CLASS ht_inputfield
 */

void ht_inputfield::init(Bounds *b, int Maxtextlen, List *hist)
{
	ht_view::init(b, VO_SELECTABLE | VO_RESIZE, "some inputfield");
	VIEW_DEBUG_NAME("ht_inputfield");

	history = hist;
	maxtextlenv = Maxtextlen;
	growmode = MK_GM(GMH_FIT, GMV_TOP);
	
	textv = ht_malloc(maxtextlenv+1);
	curcharv = textv;
	textlenv = 0;
	selstartv = 0;
	selendv = 0;

	text = &textv;
	curchar = &curcharv;
	textlen = &textlenv;
	maxtextlen = &maxtextlenv;
	selstart = &selstartv;
	selend = &selendv;

	insert = 1;
	ofs = 0;
	attachedto = 0;
}

void ht_inputfield::done()
{
	freebuf();
	ht_view::done();
}

void ht_inputfield::attach(ht_inputfield *inputfield)
{
	freebuf();
	inputfield->query(&curchar, &text, &selstart, &selend, &textlen, &maxtextlen);
	attachedto=inputfield;
	ofs=0;
	insert=1;
}

int ht_inputfield::datasize()
{
	return sizeof (ht_inputfield_data);
}

const char *ht_inputfield::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_inputfield::freebuf()
{
	if (!attachedto && text) free(*text);
}

void ht_inputfield::getdata(ObjectStream &s)
{
	if (!attachedto) {
		PUTX_INT32D(s, *textlen, NULL);
		PUTX_BINARY(s, *text, *textlen, NULL);
	}
/*	FIXPORT uint h = s->recordStart(datasize());
	if (!attachedto) {
		s->putIntDec(*textlen, 4, NULL);
		s->putBinary(*text, *textlen, NULL);
	}
	s->recordEnd(h);*/
}

int ht_inputfield::insertbyte(byte *pos, byte b)
{
	if (*textlen<*maxtextlen) {
		if (*selstart) {
			if (pos<*selstart) {
				(*selstart)++;
				(*selend)++;
			} else if (pos<*selend) {
				(*selend)++;
			}
		}
		memmove(pos+1, pos, *textlen-(pos-*text));
		*pos=b;
		(*textlen)++;
		dirtyview();
		return 1;
	}
	return 0;
}

void ht_inputfield::isetcursor(uint pos)
{
	if (pos < (uint)*textlen) *curchar = *text + pos;
}

void ht_inputfield::query(byte ***c, byte ***t, byte ***ss, byte ***se, int **tl, int **mtl)
{
	*c=curchar;
	*t=text;
	*ss=selstart;
	*se=selend;
	*tl=textlen;
	*mtl=maxtextlen;
}

void ht_inputfield::select_add(byte *start, byte *end)
{
	if (end<start) {
		byte *temp=start;
		start=end;
		end=temp;
	}
	if (end==*selstart) {
		*selstart=start;
	} else if (start==*selend) {
		*selend=end;
	} else if (start==*selstart) {
		*selstart=end;
	} else if (end==*selend) {
		*selend=start;
	} else {
		*selstart=start;
		*selend=end;
	}
	if (*selstart>*selend) {
		byte *temp=*selstart;
		*selstart=*selend;
		*selend=temp;
	}
}

void ht_inputfield::setdata(ObjectStream &s)
{
//	FIXPORT uint h=s->recordStart(datasize());
	if (!attachedto) {
		textlen=&textlenv;
		GET_INT32D(s, *textlen);
		
		if (*textlen > *maxtextlen) *textlen = *maxtextlen;
		
		GET_BINARY(s, *text, *textlen);
		
		curchar = &curcharv;
		*curchar = *text + *textlen;
		
		if (*textlen) {
			*selstart = *text;
			*selend = *text+*textlen;
		} else {
			*selstart = 0;
			*selend = 0;
		}

		ofs = 0;
	}
	
//	s->recordEnd(h);
	dirtyview();
}

/*
 *	CLASS ht_strinputfield
 */

void ht_strinputfield::init(Bounds *b, int maxstrlen, List *history)
{
	ht_inputfield::init(b, maxstrlen, history);
	VIEW_DEBUG_NAME("ht_strinputfield");
	is_virgin = true;
	selectmode = false;
}

void ht_strinputfield::done()
{
	ht_inputfield::done();
}

void ht_strinputfield::correct_viewpoint()
{
	if (*curchar - *text < ofs) {
		ofs = *curchar-*text; 
	} else {
		if (*curchar - *text - (size.w-2)*size.h+1 > ofs) {
			ofs = *curchar-*text-(size.w-2)*size.h+1;
		}
	}
}

void ht_strinputfield::draw()
{
	int c=focused ? getcolor(palidx_generic_input_focused) :
		getcolor(palidx_generic_input_unfocused);
	byte *t = *text + ofs;
	int l = *textlen - ofs;
	if (l > size.w) l = size.w;
	int y = 0;
	fill(0, 0, size.w, size.h, c, ' ');
	if (ofs) buf->printChar(0, y, getcolor(palidx_generic_input_clip), '<');
	for (int k=0; k < *textlen-ofs; k++) {
		if (1+k-y*(size.w-2) > size.w-2) {
			if (y+1 < size.h) y++; else break;
		}
		if (t < *selstart || t >= *selend) {
			buf->printChar(1+k-y*(size.w-2), y, c, *(t++));
		} else {
			buf->printChar(1+k-y*(size.w-2), y, getcolor(palidx_generic_input_selected), *(t++));
		}
	}
	if (*textlen-ofs > (size.w-2)*size.h) {
		buf->printChar(size.w-1, y, getcolor(palidx_generic_input_clip), '>');
	}
	if (history && history->count()) {
		buf->printChar(size.w-1, y+size.h-1, getcolor(palidx_generic_input_clip), GC_SMALL_ARROW_DOWN, CP_GRAPHICAL);
	}
	if (focused) {
		int cx, cy;
		if (*curchar-*text-ofs >= (size.w-2)*size.h) {
			cx = size.w-1;
			cy = size.h-1;
		} else {
			cx = (*curchar - *text - ofs) % (size.w-2)+1;
			cy = (*curchar - *text - ofs) / (size.w-2);
		}
		setcursor(cx, cy, insert ? CURSOR_NORMAL : CURSOR_BOLD);
	}
}

void ht_strinputfield::handlemsg(htmsg *msg)
{
	if (msg->type == mt_empty && msg->msg == msg_keypressed) {
		int k = msg->data1.integer;
		switch (k) {
			case K_Meta_S:
				selectmode = !selectmode;
				clearmsg(msg);
				break;
			case K_Up:
				is_virgin = false;
				if (*curchar - *text - ofs < size.w - 2) {
					ofs -= size.w-2;
					if (ofs < 0) ofs=0;
				}
				*curchar -= size.w-2;
				if (*curchar < *text) *curchar = *text;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				is_virgin = false;
				if (*curchar + size.w - 2 - *text > *textlen) {
					history_dialog();
					clearmsg(msg);
					return;
				} else {
					*curchar += size.w-2;
					if (*curchar - *text > *textlen) {
						*curchar = *text + *textlen;
					}
					correct_viewpoint();
					dirtyview();
				}
				clearmsg(msg);
				return;
			case K_Shift_Left:
			case K_Left:
				is_virgin = false;
				if (*curchar>*text) {
					(*curchar)--;
					if ((k==K_Shift_Left) != selectmode) {
						select_add(*curchar, *curchar+1);
					}
				}				    
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Shift_Right:				
			case K_Right:
				is_virgin = false;
				if (*curchar-*text<*textlen) {
					(*curchar)++;
					if ((k==K_Shift_Right) != selectmode) {
						select_add(*curchar-1, *curchar);
					}
				}					
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Backspace:
				if (is_virgin) {
					is_virgin = false;
					*selstart = 0;
					*selend = 0;
					*textlen = 0;
					*curchar = *text;
					dirtyview();
					clearmsg(msg);
				} else if (*curchar > *text) {
					*selstart = 0;
					*selend = 0;
					memmove(*curchar-1, *curchar, *textlen-(*curchar-*text));
					(*textlen)--;
					(*curchar)--;
					is_virgin=0;
					correct_viewpoint();
					dirtyview();
					clearmsg(msg);
					return;
				}
				break;
			case K_Delete:
				if (is_virgin) {
					is_virgin = false;
					*selstart = 0;
					*selend = 0;
					*textlen = 0;
					*curchar = *text;
					dirtyview();
					clearmsg(msg);
				} else if (*curchar-*text < *textlen && *textlen) {
					if (*selstart) {
						if (*curchar >= *selstart) {
							if (*curchar < *selend) (*selend)--;
							if (*selstart == *selend) {
								*selstart=0;
								*selend=0;
							}
						} else {
							(*selstart)--;
							(*selend)--;
						}
					}
					memmove(*curchar, *curchar+1, *textlen-(*curchar-*text)-1);
					(*textlen)--;
					dirtyview();
					clearmsg(msg);
					return;
				}
				break;
			case K_Shift_Home:
			case K_Home:
				is_virgin = false;
				if ((k == K_Shift_Home) != selectmode) {
					select_add(*curchar, *text);
				}					
				*curchar = *text;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Shift_End:
			case K_End:
				is_virgin = false;
				if ((k == K_Shift_End) != selectmode) {
					select_add(*curchar, *text+*textlen);
				}						
				*curchar = *text + *textlen;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Insert:
				insert=!insert;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Meta_X:
			case K_Shift_Delete:
				if (*selend > *selstart) clipboard_copy("inputfield", *selstart, *selend-*selstart);
			case K_Meta_D:
			case K_Control_Delete:
				if (*selend > *selstart) {
					memmove(*selstart, *selend, *textlen-(*selend-*text));
					*textlen -= *selend - *selstart;
					*curchar = *selstart;
					*selstart = 0;
					*selend = 0;
					is_virgin = false;
					correct_viewpoint();
				}
				dirtyview();
				clearmsg(msg);
				return;
			case K_Meta_C:
			case K_Control_Insert:
				if (*selend > *selstart) clipboard_copy("inputfield", *selstart, *selend-*selstart);
				is_virgin = false;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Meta_V:
			case K_Shift_Insert: {
				int maxsize = MIN(*maxtextlen-*textlen, (int)clipboard_getsize());
				byte *buf = ht_malloc(maxsize);
				int r = clipboard_paste(buf, maxsize);
				if (r) {
					for (int i=0; i < r; i++) {
						setbyte(buf[r-i-1]);
					}
					*selstart=*curchar;
					*selend=*curchar+r;
				}
				delete buf;
				is_virgin = false;
				dirtyview();
				clearmsg(msg);
				return;
			}
			default:
				if (msg->data1.integer >= ' ' && msg->data1.integer < 256) {
					if (is_virgin) {
						is_virgin = false;
						*selstart = 0;
						*selend = 0;
						*textlen = 0;
						*curchar = *text;
						ofs = 0;
					}
					inputbyte(msg->data1.integer);
					dirtyview();
					clearmsg(msg);
					return;
				}
		}
	}
	ht_inputfield::handlemsg(msg);
}

void ht_strinputfield::history_dialog()
{
	if (history && history->count()) {
		Bounds b;
		getbounds(&b);
		b.y--;
		b.h=8;
		ht_history_popup_dialog *l=new ht_history_popup_dialog();
		l->init(&b, history);
		if (l->run(false)) {
			ht_listpopup_dialog_data d;
			ViewDataBuf vdb(l, &d, sizeof d);

			if (d.cursor_string) {
				ht_history_entry *v=(ht_history_entry*)(*history)[d.cursor_pos];
				if (v->data && group) {
					v->datafile->seek(0);
					group->setdata(*v->data);
				} else {
					ht_inputfield_data e;
					e.textlen = strlen(d.cursor_string);
					e.text = (byte*)d.cursor_string;
					databuf_set(&e, sizeof e);
				}
			}
			is_virgin = false;
		}
		l->done();
		delete l;
	}
}

void ht_strinputfield::receivefocus()
{
	correct_viewpoint();
	ht_inputfield::receivefocus();
}

bool ht_strinputfield::inputbyte(byte a)
{
	if (setbyte(a)) {
		(*curchar)++;
		correct_viewpoint();
		is_virgin = false;
		return true;
	}
	return false;
}

bool ht_strinputfield::setbyte(byte a)
{
	if (insert || *curchar-*text >= *textlen) {
		if (insertbyte(*curchar, a) && *curchar-*text<*textlen) return true;
	} else {
		**curchar=a;
		return true;
	}
	return false;
}

/*
 *	CLASS ht_hexinputfield
 */

void ht_hexinputfield::init(Bounds *b, int maxstrlen)
{
	ht_inputfield::init(b, maxstrlen);
	VIEW_DEBUG_NAME("ht_strinputfield");
	nib=0;
	insert=1;
}

void ht_hexinputfield::done()
{
	ht_inputfield::done();
}

void ht_hexinputfield::correct_viewpoint()
{
	if (*curchar-*text<ofs) {
		ofs = *curchar-*text; 
	} else if ((*curchar-*text)*3-(size.w-2)*size.h+5>ofs*3) {
		ofs = ((*curchar-*text)*3-(size.w-2)*size.h+5) / 3;
	}
}

void ht_hexinputfield::draw()
{
	int c=focused ? getcolor(palidx_generic_input_focused) :
		getcolor(palidx_generic_input_unfocused);
	char hbuf[256], *h=hbuf;
	int y=0;
	fill(0, 0, size.w, size.h, c, ' ');
	if (ofs) buf->print(0, y, getcolor(palidx_generic_input_clip), "<");
	int vv=*textlen-ofs;
	if (vv<0) vv=0; else if (vv>(size.w-2)*size.h/3) vv=(size.w-2)*size.h/3+1;
	for (int k=0; k<vv; k++) {
		h += sprintf(h, "%02x ", *(*text+k+ofs));
	}
	if (vv) {
		h = hbuf;
		while (*h && y < size.h) {
			h += buf->nprint(1, y, c, h, size.w-2);
			y++;
		}
		y--;
	}
	if ((*textlen-ofs)*3 > (size.w-2)*size.h) {
		buf->print(size.w-1, y, getcolor(palidx_generic_input_clip), ">");
	}
	if (focused) {
		int cx, cy;
		if ((*curchar-*text-ofs)*3+nib+1 >= (size.w-2)*size.h) {
			cx = size.w-1;
			cy = size.h-1;
		} else {
			cx = ((*curchar-*text-ofs)*3+nib+1) % (size.w-2);
			cy = ((*curchar-*text-ofs)*3+nib+1) / (size.w-2);
		}
		setcursor(cx, cy, insert ? CURSOR_NORMAL : CURSOR_BOLD);
	}
}

void ht_hexinputfield::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Up:
				if (*curchar-*text-ofs<(size.w-2)/3) {
					ofs-=(size.w-2)/3;
					if (ofs<0) ofs=0;
				}
				*curchar-=(size.w-2)/3;
				if (*curchar<*text) *curchar=*text;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
/*				if (*curchar-*text-ofs>=(size.w-2)*(size.h-1)/3) {
					ofs+=(size.w-2)/3;
				}*/
				*curchar+=(size.w-2)/3;
				if (*curchar-*text>*textlen) *curchar=*text+*textlen;
				if (*curchar-*text>=*textlen) nib=0;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Left:
				if (nib) {
					nib=0;
				} else if (*curchar>*text) {
					(*curchar)--;
					nib=1;
				}
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Right:
				if (!nib) {
					if (*curchar-*text<*textlen) nib=1;
				} else if (*curchar-*text<*textlen) {
					(*curchar)++;
					nib=0;
				}
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Backspace:
				if (*textlen) {
					if (*curchar+nib>*text) {
/*						if (*curchar-*text<*textlen) {
							memmove(*curchar, *curchar+1, *textlen-(*curchar-*text)-1);
						}
					} else {*/
						memmove(*curchar-1+nib, *curchar+nib, *textlen-(*curchar-*text));
						(*textlen)--;
						if (*curchar-*text && !nib) (*curchar)--;
						nib=0;
						correct_viewpoint();
						dirtyview();
					}
					clearmsg(msg);
					return;
				}
				break;
			case K_Delete:
				if ((*curchar-*text<*textlen) && (*textlen)) {
					memmove(*curchar, *curchar+1, *textlen-(*curchar-*text)-1);
					(*textlen)--;
					nib=0;
					dirtyview();
					clearmsg(msg);
					return;
				}
				break;
			case K_Home:
				*curchar=*text;
				nib=0;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_End:
				*curchar=*text+*textlen;
				nib=0;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Insert:
				insert=!insert;
				dirtyview();
				clearmsg(msg);
				return;
			default:
				if ((msg->data1.integer>='0') && (msg->data1.integer<='9')) {
					setnibble(msg->data1.integer-'0');
					dirtyview();
					clearmsg(msg);
				} else if ((msg->data1.integer>='a') && (msg->data1.integer<='f')) {
					setnibble(msg->data1.integer-'a'+10);
					dirtyview();
					clearmsg(msg);
				}
				return;
		}
	}
	ht_inputfield::handlemsg(msg);
}

void ht_hexinputfield::receivefocus()
{
	correct_viewpoint();
	if (nib && *curchar-*text == *textlen) {
		nib=0;
	}
	ht_inputfield::receivefocus();
}

void ht_hexinputfield::setnibble(byte a)
{
	if ((insert || *curchar-*text >= *textlen) && nib == 0) {
		if ((insertbyte(*curchar, a<<4)) && (*curchar-*text<*textlen)) nib=1;
	} else {
		if (nib) {
			**curchar=(**curchar & 0xf0) | a;
			(*curchar)++;
			nib=0;
		} else {
			**curchar=(**curchar & 0xf) | (a<<4);
			nib=1;
		}
	}
	correct_viewpoint();
}

/*
 *	CLASS ht_button
 */

void ht_button::init(Bounds *b, const char *Text, int Value)
{
	ht_view::init(b, VO_SELECTABLE | VO_OWNBUFFER | VO_POSTPROCESS, "some button");
	VIEW_DEBUG_NAME("ht_button");
	value = Value;
	text = ht_strdup(Text);
	pressed = 0;
	magicchar = strchr(text, '~');
	if (magicchar) {
		int l = strlen(text);
		memmove(magicchar, magicchar+1, l-(magicchar-text));
		shortcut1 = keyb_metakey((ht_key)tolower(*magicchar));
		shortcut2 = (ht_key)tolower(*magicchar);
	} else {
		shortcut1 = K_INVALID;
		shortcut2 = K_INVALID;
	}
}

void ht_button::done()
{
	free(text);
	ht_view::done();
}

void ht_button::getminbounds(int *width, int *height)
{
	*width = 6;
	*height = 2;
}

const char *ht_button::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_button::draw()
{
	int c=focused ? getcolor(palidx_generic_button_focused) :
		getcolor(palidx_generic_button_unfocused);
	fill(0, 0, size.w-1, size.h-1, c, ' ');
	int xp = (size.w - strlen(text))/2, yp = (size.h-1)/2;
	buf->print(xp, yp, c, text);
	if (magicchar) buf->printChar(xp+(magicchar-text), yp, getcolor(palidx_generic_button_shortcut), *magicchar);
	/* shadow */
	buf->printChar(0, 1, getcolor(palidx_generic_button_shadow), ' ');
	for (int i=1; i<size.w-1; i++) {
		buf->printChar(i, 1, getcolor(palidx_generic_button_shadow), GC_FILLED_UPPER, CP_GRAPHICAL);
	}
	buf->printChar(size.w-1, 0, getcolor(palidx_generic_button_shadow), GC_FILLED_LOWER, CP_GRAPHICAL);
	buf->printChar(size.w-1, 1, getcolor(palidx_generic_button_shadow), GC_FILLED_UPPER, CP_GRAPHICAL);
}

void ht_button::handlemsg(htmsg *msg)
{
	if (msg->type == mt_postprocess) {
		if (msg->msg == msg_keypressed) {
			if ((shortcut1 != K_INVALID && msg->data1.integer==shortcut1) ||
			(shortcut2 != K_INVALID && msg->data1.integer==shortcut2)) {
				push();
				dirtyview();
				clearmsg(msg);
				return;
			}
		}
	} else if (msg->type == mt_empty) {
		if (msg->msg == msg_keypressed) {
			switch (msg->data1.integer) {
			case K_Return:
			case K_Space:
				push();
				dirtyview();
				clearmsg(msg);
				return;
			}
		}
	}
	ht_view::handlemsg(msg);
}

void ht_button::push()
{
	/* FIXME: wont work for encapsulated buttons... */
	/* FIXME: (thats why I hacked this now...) */
	app->sendmsg(msg_button_pressed, value);
//	baseview->sendmsg(msg_button_pressed, value);	// why not like this ?
	pressed=1;
}

/*
 *	CLASS ht_listbox_title
 */
void	ht_listbox_title::init(Bounds *b)
{
	ht_view::init(b, VO_RESIZE, "ht_listbox_title");
	growmode = MK_GM(GMH_FIT, GMV_TOP);
	texts = NULL;
	listbox = NULL;
	cols = 0;
}

void	ht_listbox_title::done()
{
	if (texts) {
		for (int i=0; i < cols; i++) {
			free(texts[i]);
		}
		free(texts);
	}
	ht_view::done();
}

const char *ht_listbox_title::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_listbox_title::draw()
{
	vcp color = getTextColor();
	clear(color);
	if (!texts || !listbox) return;
	int x = 0;
	for (int i=0; i < cols; i++) {     
		buf->nprint(x, 0, color, texts[i], size.w - x);
		x += listbox->widths[i];
		if (i+1 < cols) {
			if (x >= size.w) break;
			buf->printChar(x++, 0, color, ' ');
			if (x >= size.w) break;
			buf->printChar(x++, 0, color, GC_1VLINE, CP_GRAPHICAL);
			if (x >= size.w) break;
			buf->printChar(x++, 0, color, ' ');
		}
	}
}

vcp ht_listbox_title::getTextColor()
{
	return getcolor(palidx_generic_body);
}

void ht_listbox_title::setText(int cols, ...)
{
	va_list vargs;
	va_start(vargs, cols);
	setTextv(cols, vargs);
	va_end(vargs);
}

void ht_listbox_title::setTextv(int c, va_list vargs)
{
	if (texts) {
		for (int i=0; i<cols; i++) {
			free(texts[i]);
		}
		free(texts);
	}
	texts = NULL;
	cols = c;
	if (!c) return;
	texts = ht_malloc(c * sizeof(char*));
	for (int i=0; i<cols; i++) {
		texts[i] = ht_strdup(va_arg(vargs, char*));
	}
	update();
}

void ht_listbox_title::update()
{
	if (texts && listbox && listbox->widths) {
		for (int i=0; i<cols; i++) {
			int s = strlen(texts[i]);
			if (s > listbox->widths[i]) listbox->widths[i] = s;
		}
	}
}


/*
 *	CLASS ht_listbox
 */

class ht_listbox_vstate: public Object {
public:
	void *e_top;
	void *e_cursor;

	ht_listbox_vstate(void *top, void *cursor)
	{
		e_top = top;
		e_cursor = cursor;
	}
};

void ht_listbox::init(Bounds *b, uint Listboxcaps)
{
	ht_view::init(b, VO_SELECTABLE | VO_OWNBUFFER | VO_RESIZE, 0);
	cached_count = 0;

	growmode = MK_GM(GMH_FIT, GMV_FIT);

	Bounds c = *b;
	c.x = c.w-1;
	c.y = 0;
	c.w = 1;
	scrollbar = new ht_scrollbar();
	scrollbar->init(&c, &pal, true);

	pos = 0;
	cursor = 0;
	e_top = getFirst();
	e_cursor = e_top;
	title = NULL;
	visible_height = 0;
	x = 0;
	widths = NULL;
	clearQuickfind();
	update();
	listboxcaps = Listboxcaps;
	cols = 0;
}

void	ht_listbox::done()
{
	scrollbar->done();
	delete scrollbar;
	free(widths);
	ht_view::done();
}

void ht_listbox::adjustPosHack()
{
	if (e_cursor != e_top) return;
	int i=0;
	void *tmp = e_cursor;
	if (!tmp) return;
	while (tmp && i <= visible_height) {
		tmp = getNext(tmp);
		i++;
	}
	if (i < visible_height) {
		cursorDown(cursorUp(visible_height - pos - i));
	}
}

void ht_listbox::adjustScrollbar()
{
	int pstart, psize;
	if (scrollbar_pos(pos-cursor, size.h, cached_count, &pstart, &psize)) {
		mScrollbarEnabled = true;
		scrollbar->enable();
		Bounds c = size;
		c.x = c.w-1;
		c.y = 0;
		c.w = 1;
		scrollbar->setbounds(&c);
		scrollbar->setpos(pstart, psize);
	} else {
		mScrollbarEnabled = false;
		scrollbar->disable();
	}
}

void ht_listbox::attachTitle(ht_listbox_title *aTitle)
{
	if (numColumns() > cols) rearrangeColumns();
	title = aTitle;
	title->listbox = this;
	title->update();
	title->dirtyview();
}

void ht_listbox::clearQuickfind()
{
	quickfinder[0] = 0;
	qpos = quickfinder;
	updateCursor();
}

int  ht_listbox::cursorAdjust()
{
	return 0;
}

int  ht_listbox::cursorUp(int n)
{
	void *tmp;
	int  i = 0;

	while (n--) {
		tmp = getPrev(e_cursor);
		if (!tmp) break;
		if (e_cursor == e_top) {
			e_top = tmp;
		} else {
			cursor--;
		}
		e_cursor = tmp;
		pos--;
		if (pos < 0) pos = 0; // if cursor was out of sync
		i++;
	}
	return i;
}

int  ht_listbox::cursorDown(int n)
{
	void *tmp;
	int  i = 0;

	while (n--) {
		tmp = getNext(e_cursor);
		if (!tmp) break;
		if (cursor+1 >= visible_height) {
			e_top = getNext(e_top);
		} else {
			cursor++;
		}
		pos++;
		e_cursor = tmp;
		i++;
	}
	return i;
}

int  ht_listbox::datasize()
{
	return sizeof (ht_listbox_data);
}

const char *ht_listbox::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_listbox::draw()
{
	int fc = focused ? getcolor(palidx_generic_list_focused_unselected) :
		getcolor(palidx_generic_list_unfocused_unselected);

	int Cols = numColumns();
	if (Cols > cols) rearrangeColumns();

	bool title_redraw = false;
	bool resizing_cols;

	do {
		resizing_cols = false;
		clear(fc);
		void *entry = e_top;
		int i = 0;
		while (entry && i < visible_height) {
			int c = (i==cursor) ? (focused ? getcolor(palidx_generic_list_focused_selected) :
				getcolor(palidx_generic_list_unfocused_selected)) : fc;
			if (i == cursor) {
				fill(0, i, size.w, 1, c, ' ');
			}
			int X = -x;
			for (int j=0; j < cols; j++) {
				const char *s = getStr(j, entry);
				int slen = strlen(s);
				if (slen > widths[j]) {
					widths[j] = slen;
					/*
					 *	a column has been resized,
					 *	therefore we have to redraw a second time.
					 */
					resizing_cols = true;
					title_redraw = true;
				}
				if (s) {
					if (X >= 0) {
						buf->nprint(X, i, c, s, size.w);
					} else {
						if (slen > -X) buf->nprint(0, i, c, &s[-X], size.w);
					}
				}
				if (j==cols-1) {
					X += slen;
				} else {
					X += widths[j];
				}
				if (j+1 < cols) {
					buf->printChar(X++, i, c, ' ');
					buf->printChar(X++, i, c, GC_1VLINE, CP_GRAPHICAL);
					buf->printChar(X++, i, c, ' ');
				}
			}
			if (x > 0) {
				// more text on the left
				buf->printChar(0, i, c, '<');
			}
			// position of '>' char is scrollbar dependent
			int a = mScrollbarEnabled ? 0 : 1;
			if (X >= size.w+a) {
				// more text right
				buf->printChar(size.w-2+a, i, c, '>');
			}
			entry = getNext(entry);
			i++;
		}
	} while (resizing_cols);
	updateCursor();
	if (title_redraw && title) {
		title->update();
		title->dirtyview();
	}
/*     char dbg[100];
	sprintf(dbg, "cursor=%d pos=%d vh=%d qc:%s", cursor, pos, visible_height, quickfinder);
	lprint(0, 0, 1, size.w, dbg);
	sprintf(dbg, "e_top=%s", getstr(0, e_top));
	lprint(0, 1, 1, size.w, dbg);
	sprintf(dbg, "e_cursor=%s", getstr(0, e_cursor));
	lprint(0, 2, 1, size.w, dbg);*/
}

int  ht_listbox::estimateEntryPos(void *entry)
{
	// this is slow!
	void *tmp = getFirst();
	int res = 0;
	while (tmp) {
		if (tmp == entry) break;
		tmp = getNext(tmp);
		res++;
	}
	return (tmp==entry) ? res : -1;
}

void ht_listbox::getdata(ObjectStream &s)
{
	ht_listbox_data_internal d;
	d.top_ptr = e_top;
	d.cursor_ptr = e_cursor;
	PUTX_BINARY(s, &d, sizeof d, NULL);
}

void ht_listbox::gotoItemByEntry(void *entry, bool clear_quickfind)
{
	if (clear_quickfind) clearQuickfind();
	if (!entry) return;
	void *tmp = e_top;
	int i=0;
	bool ok=false;
	pos -= cursor;
	if (pos<0) pos = 0; // if cursor was out of sync
	cursor = 0;

	while (tmp && i < visible_height) {
		if (tmp == entry) {
			ok = true;
			break;
		}
		pos++;
		cursor++;
		i++;
		tmp = getNext(tmp);
	}
	e_cursor = entry;
	if (!ok) {
		e_top = entry;
		cursor = 0;
		pos = estimateEntryPos(entry);
		assert(pos != -1);
	}
	adjustPosHack();
	stateChanged();
}

void ht_listbox::gotoItemByPosition(uint pos)
{
	void *entry = getFirst();
	while (pos--) entry = getNext(entry);
	gotoItemByEntry(entry, true);
}

void ht_listbox::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_vstate_restore: {
			ht_listbox_vstate *vs = (ht_listbox_vstate*)msg->data1.ptr;
			e_top = vs->e_top;
			e_cursor = vs->e_cursor;
			update();
			// FIXME: what about deleting entries?
			clearmsg(msg);
			return;
		}
		case msg_keypressed: switch (msg->data1.integer) {
			case K_Control_PageUp:
			case K_Home:
				clearQuickfind();
				pos = cursor = 0;
				e_top = e_cursor = getFirst();
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			case K_Control_PageDown:
			case K_End: {
				clearQuickfind();
				cursor = 0;
				pos = cached_count ? cached_count-1 : 0;
				e_cursor = e_top = getLast();
				cursorUp(visible_height-1);
				cursorDown(visible_height-1);
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			}
			case K_PageUp:
				clearQuickfind();
				cursorUp(visible_height-1);
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			case K_PageDown: {
				clearQuickfind();
				cursorDown(visible_height-1);
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			}
			case K_Up:
				clearQuickfind();
				cursorUp(1);
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			case K_Down:
				clearQuickfind();
				cursorDown(1);
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			case K_Left:
			case K_Control_Left:
				if (x > 0) x--;
				updateCursor();
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			case K_Right:
			case K_Control_Right:
				x++;
				updateCursor();
				dirtyview();
				clearmsg(msg);
				stateChanged();
				return;
			case K_Tab:
				if (listboxcaps & LISTBOX_QUICKFIND) {
					if (*quickfinder) {
						char *qc = quickfindCompletition(quickfinder);
						if (qc) {
							strcpy(quickfinder, qc);
							qpos = ht_strend(quickfinder);
							free(qc);
							goto qf;
						}
					}
				}
				break;
			case K_Backspace: {
				if (listboxcaps & LISTBOX_QUICKFIND) {
					if (qpos > quickfinder) {
						*(--qpos) = 0;
						qf:
						void *a = quickfind(quickfinder);
						if (a) {
							gotoItemByEntry(a, false);
							updateCursor();
							dirtyview();
						}
						clearmsg(msg);
						stateChanged();
					}
				}
				return;
			}
			default: {
				if ((listboxcaps & LISTBOX_QUICKFIND) && msg->data1.integer > 31 && msg->data1.integer < 0xff) {
					*qpos++ = msg->data1.integer;
					*qpos = 0;
					void *a = quickfind(quickfinder);
					if (a) {
						gotoItemByEntry(a, false);
						updateCursor();
						dirtyview();
						clearmsg(msg);
						stateChanged();
					} else {
						*(--qpos) = 0;
					}
				}
			}
		}
		break;
	}
	ht_view::handlemsg(msg);
}

int	ht_listbox::numColumns()
{
	return 1;
}

char	*ht_listbox::quickfindCompletition(const char *s)
{
	return ht_strdup(s);
}

void ht_listbox::rearrangeColumns()
{
	free(widths);
	cols = numColumns();
	widths = (int*)calloc(cols*sizeof(int), 1);
}

void ht_listbox::redraw()
{
	ht_view::redraw();
	scrollbar->relocate_to(this);
//	fprintf(stderr, "scrollbar: x=%d, y=%d, w=%d, h=%d\n", scrollbar->vsize.x, scrollbar->vsize.y, scrollbar->vsize.w, scrollbar->vsize.h);
	scrollbar->redraw();
	if (title) {
		title->redraw();
	}
	scrollbar->unrelocate_to(this);
}

void ht_listbox::resize(int rw, int rh)
{
	ht_view::resize(rw, rh);
	update();
}

void ht_listbox::stateChanged()
{
	adjustScrollbar();
}

bool ht_listbox::selectEntry(void *entry)
{
	return true;
}

void ht_listbox::setdata(ObjectStream &s)
{
	ht_listbox_data_internal d;
	GET_BINARY(s, &d, sizeof d);
	e_top = d.top_ptr;
	e_cursor = d.cursor_ptr;
	update();
}

Object *ht_listbox::vstate_create()
{
	return new ht_listbox_vstate(e_top, e_cursor);
}

void ht_listbox::vstate_save()
{
	Object *vs = vstate_create();
	if (vs) {
		htmsg m;
		m.msg = msg_vstate_save;
		m.type = mt_empty;
		m.data1.ptr = vs;
		m.data2.ptr = this;
		app->sendmsg(&m);
	}
}

/*
 *	must be called if data has changed
 */
void ht_listbox::update()
{
	void *entry = getFirst();
	cached_count = calcCount();
	visible_height = MIN(size.h, cached_count);
	if (cached_count <= size.h) {
		if (!e_cursor) e_cursor = getFirst();
		cursor = 0;
		while (entry && cursor < visible_height) {
			if (entry == e_cursor) {
				e_top = getFirst();
				goto ok;
			}
			entry = getNext(entry);
			cursor++;
		}
	}
	if (!e_top) {
		e_top = getFirst();
	}     
	if (!e_cursor) e_cursor = e_top;
	entry = e_top;
	cursor = 0;
	while (entry && cursor < visible_height) {
		if (entry == e_cursor) goto ok;
		entry = getNext(entry);
		cursor++;
	}
	cursor = 0;
	e_top = e_cursor;
ok:
	adjustPosHack();
	stateChanged();
	if (title) {
		title->update();
		title->dirtyview();
	}
	dirtyview();
}

void ht_listbox::updateCursor()
{
	if (focused) {
		if (*quickfinder) {
			setcursor((qpos-quickfinder)+cursorAdjust()-x, cursor);
		} else {
			hidecursor();
		}
	}		
}

/*
 *	ht_text_listbox
 */

void	ht_text_listbox::init(Bounds *b, int aCols, int aKeycol, uint aListboxcaps)
{
	first = last = NULL;
	count = 0;
	ht_listbox::init(b, aListboxcaps);
	cols = aCols;
	keycol = aKeycol;
	Cursor_adjust = 0;
	rearrangeColumns();
}

void ht_text_listbox::done()
{
	clearAll();
	ht_listbox::done();
}

void ht_text_listbox::clearAll()
{
	ht_text_listbox_item *temp = first;
	while (temp) {
		ht_text_listbox_item *temp2 = temp->next;
		freeExtraData(temp->extra_data);
		for (int i=0; i < cols; i++) {
			free(temp->data[i]);
		}
		free(temp);
		temp = temp2;
	}
	first = last = NULL;
	
	pos = 0;
	cursor = 0;
	e_top = getFirst();
	e_cursor = e_top;
	x = 0;
	clearQuickfind();

	count = 0;
	Cursor_adjust = 0;
}

int	ht_text_listbox::calcCount()
{
	return count;
}

int	ht_text_listbox::compare_strn(const char *s1, const char *s2, int l)
{
	return ht_strncmp(s1, s2, l);
}

int	ht_text_listbox::compare_ccomm(const char *s1, const char *s2)
{
	return ht_strccomm(s1, s2);
}

int  ht_text_listbox::cursorAdjust()
{
	return Cursor_adjust;
}

void ht_text_listbox::freeExtraData(void *extra_data)
{
}

void *ht_text_listbox::getFirst()
{
	return first;
}

uint ht_text_listbox::getID(void *entry)
{
	if (entry) {
		return ((ht_text_listbox_item *)entry)->id;
	} else {
		return 0;
	}	    
}

void *ht_text_listbox::getExtra(void *entry)
{
	if (entry) {
		return ((ht_text_listbox_item *)entry)->extra_data;
	} else {
		return NULL;
	}	    
}

void *ht_text_listbox::getLast()
{
	return last;
}

void *ht_text_listbox::getNext(void *entry)
{
	if (!entry) return NULL;
	return ((ht_text_listbox_item *)entry)->next;
}

void *ht_text_listbox::getPrev(void *entry)
{
	if (!entry) return NULL;
	return ((ht_text_listbox_item *)entry)->prev;
}

const char *ht_text_listbox::getStr(int col, void *entry)
{
	if (entry && col < cols) {
		return ((ht_text_listbox_item *)entry)->data[col];
	} else {
		return "";
	}
}

void ht_text_listbox::insert_str_extra(int id, void *extra_data, const char **strs)
{
	// FIXME: code duplication...
	ht_text_listbox_item *item = ht_malloc(sizeof(ht_text_listbox_item)+sizeof(char *)*cols);
	item->next = NULL;
	item->prev = last;
	item->id = id;
	item->extra_data = extra_data;
	for (int i=0; i<cols; i++) {
		int slen = strlen(strs[i]);
		if (slen > widths[i]) {
			widths[i] = slen;
		}

		item->data[i] = ht_strdup(strs[i]);
	}
	if (first) {
		last->next = item;
	} else {
		first = item;
	}
	last = item;
	count++;
}

void	ht_text_listbox::insert_str_extra(int id, void *extra_data, const char *str, ...)
{
	ht_text_listbox_item *item = ht_malloc(sizeof(ht_text_listbox_item)+sizeof(char *)*cols);
	item->next = NULL;
	item->prev = last;
	item->id = id;
	item->extra_data = extra_data;
	va_list str2;
	va_start(str2, str);
	const char *str3 = str;
	for (int i=0; i<cols; i++) {
		int slen = strlen(str3);
		if (slen > widths[i]) {
			widths[i] = slen;
		}

		item->data[i] = ht_strdup(str3);
		str3 = va_arg(str2, char *);
	}
	va_end(str2);
	if (first) {
		last->next = item;
	} else {
		first = item;
	}
	last = item;
	count++;
}

void	ht_text_listbox::insert_str(int id, const char **strs)
{
	insert_str_extra(id, NULL, strs);
}

void ht_text_listbox::insert_str(int id, const char *str, ...)
{
	// FIXME: same as insert_str(id, NULL, str, ...)
	ht_text_listbox_item *item = ht_malloc(sizeof(ht_text_listbox_item)+sizeof(char *)*cols);
	item->next = NULL;
	item->prev = last;
	item->id = id;
	item->extra_data = NULL;
	va_list str2;
	va_start(str2, str);
	const char *str3 = str;
	for (int i=0; i<cols; i++) {
		int slen = strlen(str3);
		if (slen > widths[i]) {
			widths[i] = slen;
		}

		item->data[i] = ht_strdup(str3);
		str3 = va_arg(str2, char *);
	}
	va_end(str2);
	if (first) {
		last->next = item;
	} else {
		first = item;
	}
	last = item;
	count++;
}

int ht_text_listbox::numColumns()
{
	return cols;
}

void *ht_text_listbox::quickfind(const char *s)
{
	ht_text_listbox_item *item = first;
	int slen = strlen(s);
	while (item && (compare_strn(item->data[keycol], s, slen)!=0)) {
		item = item->next;
	}
	return item;
}

char *ht_text_listbox::quickfindCompletition(const char *s)
{
	ht_text_listbox_item *item = first;
	char *res = NULL;
	int slen = strlen(s);
	while (item) {
		if (compare_strn(item->data[keycol], s, slen)==0) {
			if (!res) {
				res = ht_strdup(item->data[keycol]);
			} else {
				int a = compare_ccomm(item->data[keycol], res);
				res[a] = 0;
			}
		}
		item = item->next;
	}
	return res;
}

static int ht_text_listboxcomparatio(ht_text_listbox_item *a, ht_text_listbox_item *b, int count, ht_text_listbox_sort_order *so)
{
	for (int i=0;i<count;i++) {
		int r = so[i].compare_func(a->data[so[i].col], b->data[so[i].col]);
		if (r!=0) return r;
	}
	return 0;
}

static void ht_text_listboxqsort(int l, int r, int count, ht_text_listbox_sort_order *so, ht_text_listbox_item **list)
{
	int m = (l+r)/2;
	int L = l;
	int R = r;
	ht_text_listbox_item *c = list[m];
	do {
		while (l <= r && ht_text_listboxcomparatio(list[l], c, count, so) < 0) l++;
		while (l <= r && ht_text_listboxcomparatio(list[r], c, count, so) > 0) r--;
		if (l<=r) {
			ht_text_listbox_item *t = list[l];
			list[l] = list[r];
			list[r] = t;
			l++;
			r--;
		}
	} while(l < r);
	if (L < r) ht_text_listboxqsort(L, r, count, so, list);
	if (l < R) ht_text_listboxqsort(l, R, count, so, list);
}

void ht_text_listbox::sort(int count, ht_text_listbox_sort_order *so)
{
	ht_text_listbox_item **list;
	ht_text_listbox_item *tmp;
	int i=0;
	int cnt = calcCount();

	if (cnt < 2) return;
	
	list = ht_malloc(cnt*sizeof(void *));
	tmp = first;
	while (tmp) {
		list[i++] = tmp;
		tmp = (ht_text_listbox_item *)getNext(tmp);
	}

	int c_id = ((ht_text_listbox_item*)e_cursor)->id;

	ht_text_listboxqsort(0, cnt-1, count, so, list);

	for (i=0; i<cnt; i++) {
		if (list[i]->id == c_id) {
			pos = i;
			e_cursor = list[i];
		}
	}

	first = list[0];
	last = list[cnt-1];
	first->prev = NULL;
	last->next = NULL;
	last->prev = list[cnt-2];
	tmp = first->next = list[1];
	for (i=1; i<cnt-1; i++) {
		tmp->prev = list[i-1];
		tmp->next = list[i+1];
		tmp = list[i+1];
	}
	free(list);

	update();
	stateChanged();
}

void ht_text_listbox::update()
{
	ht_listbox::update();
	Cursor_adjust = 0;
	if (widths) {
		for (int i=0; i<keycol; i++) {
			Cursor_adjust+=widths[i]+3;
		}
	}
}

/*
 *	CLASS ht_itext_listbox
 */

void	ht_itext_listbox::init(Bounds *b, int Cols, int Keycol)
{
	ht_text_listbox::init(b, Cols, Keycol);
}

int	ht_itext_listbox::compare_strn(const char *s1, const char *s2, int l)
{
	return ht_strnicmp(s1, s2, l);
}

int	ht_itext_listbox::compare_ccomm(const char *s1, const char *s2)
{
	return ht_strcicomm(s1, s2);
}

/*
 *	CLASS ht_statictext
 */

#define STATICTEXT_MIN_LINE_FILL 70	/* percent */

void ht_statictext_align(ht_statictext_linedesc *d, statictext_align align, int w)
{
	switch (align) {
	case align_center:
		d->ofs = (w-d->len)/2;
		break;
	case align_right:
		d->ofs = w-d->len;
		break;
	default:
		d->ofs = 0;
		break;
	}
}

void ht_statictext::init(Bounds *b, const char *t, statictext_align al, bool breakl, bool trans)
{
	ht_view::init(b, VO_OWNBUFFER | VO_RESIZE, "some statictext");
	VIEW_DEBUG_NAME("ht_statictext");

	align = al;
	breaklines = breakl;
	transparent = trans;
	text = ht_strdup(t);
}

void ht_statictext::done()
{
	free(text);
	ht_view::done();
}

const char *ht_statictext::defaultpalette()
{
	return palkey_generic_dialog_default;
}

#define ssst_word		0
#define ssst_separator	1
#define ssst_whitespace	2

static int get_ssst(char s)
{
	if (strchr(".,:;+-*/=()[]", s)) {
		return ssst_separator;
	} else if (s==' ') {
		return ssst_whitespace;
	}
	return ssst_word;
}

void ht_statictext::draw()
{
	if (!transparent) clear(gettextcolor());
	char text[size.w*size.h];
	if (gettext(text, size.w*size.h) <= 0) return;
	char *t = text;
	if (breaklines) {
		/* format string... */	
		ht_statictext_linedesc *orig_d = ht_malloc(sizeof (ht_statictext_linedesc)*size.h);
		ht_statictext_linedesc *d = orig_d;
		statictext_align lalign = align;
		int c=0;
		while (*t && c < size.h) {
			/* custom alignment */
			if (*t == ALIGN_CHAR_ESCAPE && align == align_custom) {
				switch (t[1]) {
				case ALIGN_CHAR_LEFT:
					lalign=align_left;
					break;
				case ALIGN_CHAR_CENTER:
					lalign=align_center;
					break;
				case ALIGN_CHAR_RIGHT:
					lalign=align_right;
					break;
				}
				t+=2;
			}
			/* determine line length */
			int i=0, len=1;
			char *bp = t+1;
			char *n = t+1;
			int ssst = get_ssst(t[i]);
			while (t[i]) {
				if (i+1 > size.w || t[i]=='\n' || !t[i+1]) {
					bool kill_ws = (t[i]!='\n');
					if (i+1 <= size.w) {
						/* line shorter than size.w */
						bp=t+i+1;
					} else if (t[i]=='\n') {
						/* line end */
						bp=t+i+1;
					} else if (size.w && ((bp-t)*100/size.w < STATICTEXT_MIN_LINE_FILL)) {
						/* force break to make line long enough */
						bp=t+i;
					}
					len=bp-t;
					if (t[len-1]=='\n') len--;
					n=bp;
					if (kill_ws) {
						while (*n==' ') n++;
						while (t[len-1]==' ') len--;
					}
					break;
				}
				int s=get_ssst(t[i+1]);
				if ((ssst!=s) || (ssst==ssst_separator)) {
					bp=t+i+1;
				}
				ssst=s;
				i++;
			}
			d->text=t;
			d->len=len;
			ht_statictext_align(d, lalign, size.w);
			d++;
			c++;
			t=n;
		}

/**/
		d = orig_d;
		for (int i=0; i<c; i++) {
			buf->nprint(d->ofs, i, gettextcolor(), d->text, d->len);
			d++;
		}
		free(orig_d);
	} else {
		int o=0;
		buf->print(o, 0, gettextcolor(), t);
	}
}

vcp ht_statictext::gettextcolor()
{
	return getcolor(palidx_generic_body);
//	return VCP(VC_RED, VC_BLACK);
}

int ht_statictext::gettext(char *aText, int maxlen)
{
	if (text) {
		return ht_strlcpy(aText, text, maxlen);
	} else {
		if (maxlen > 0) *aText = 0;
		return 0;
	}
}

void ht_statictext::settext(const char *aText)
{
	free(text);
	text = ht_strdup(aText);
	dirtyview();
}

/*
 *	CLASS ht_listpopup_dialog
 */

void ht_listpopup_dialog::init(Bounds *b, const char *desc)
{
	ht_dialog::init(b, desc, FS_TITLE | FS_MOVE);
	VIEW_DEBUG_NAME("ht_listpopup_dialog");

	Bounds c;
	getclientarea(&c);
	c.x=0;
	c.y=0;
	init_text_listbox(&c);
}

int ht_listpopup_dialog::datasize()
{
	return sizeof (ht_listpopup_dialog_data);
}

const char *ht_listpopup_dialog::defaultpalette()
{
	return palkey_generic_blue;
}

void ht_listpopup_dialog::getdata(ObjectStream &s)
{
	ht_listbox_data d;
	ViewDataBuf vdb(listbox, &d, sizeof d);

	PUTX_INT32D(s, ((ht_text_listbox*)listbox)->getID(d.data->cursor_ptr), NULL);

	ht_text_listbox_item *cursor = (ht_text_listbox_item*)d.data->cursor_ptr;
	if (cursor) {
		PUTX_STRING(s, cursor->data[0], NULL);
	} else {
		PUTX_STRING(s, NULL, NULL);
	}
}

void ht_listpopup_dialog::init_text_listbox(Bounds *b)
{
	listbox = new ht_text_listbox();
	((ht_text_listbox *)listbox)->init(b);
	insert(listbox);
}

void ht_listpopup_dialog::insertstring(const char *string)
{
	((ht_text_listbox *)listbox)->insert_str(listbox->calcCount(), string);
	listbox->update();
}

void ht_listpopup_dialog::select_next()
{
	listbox->cursorDown(1);
}

void ht_listpopup_dialog::select_prev()
{
	listbox->cursorUp(1);
}

void ht_listpopup_dialog::setdata(ObjectStream &s)
{
	int cursor_id = GETX_INT32D(s, NULL);
//	free(GETX_STRING(s, NULL));	/* ignored */

	listbox->gotoItemByPosition(cursor_id);
}

/*
 *	CLASS ht_listpopup
 */

void	ht_listpopup::init(Bounds *b)
{
	ht_statictext::init(b, 0, align_left, 0);
	setoptions(options | VO_SELECTABLE);
	VIEW_DEBUG_NAME("ht_listpopup");

	Bounds c=*b;
	c.x=0;
	c.y=0;
	c.h=5;
	
	listpopup = new ht_listpopup_dialog();
	listpopup->init(&c, 0);
}

void	ht_listpopup::done()
{
	listpopup->done();
	delete listpopup;
	
	ht_view::done();
}

int ht_listpopup::datasize()
{
	return listpopup->datasize();
}

void ht_listpopup::draw()
{
	ht_statictext::draw();
	buf->printChar(size.w-1, 0, gettextcolor(), GC_SMALL_ARROW_DOWN, CP_GRAPHICAL);
}

vcp ht_listpopup::gettextcolor()
{
	return focused ? getcolor(palidx_generic_input_selected) :
		getcolor(palidx_generic_input_focused);
}

void ht_listpopup::getdata(ObjectStream &s)
{
	listpopup->getdata(s);
}

int ht_listpopup::gettext(char *text, int maxlen)
{
	ht_listpopup_dialog_data d;
	ViewDataBuf vdb(listpopup, &d, sizeof d);
	return ht_strlcpy(text, d.cursor_string, maxlen);
}

void ht_listpopup::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_keypressed) {
		switch (msg->data1.integer) {
		case K_Up: {
			int r;
			ht_listpopup_dialog_data d;
			ViewDataBuf vdb(listpopup, &d, sizeof d);
			listpopup->select_prev();
			r = run_listpopup();
			clearmsg(msg);
			if (!r) listpopup->databuf_set(&d, sizeof d);
			return;
		}
		case K_Down: {
			int r;
			ht_listpopup_dialog_data d;
			ViewDataBuf vdb(listpopup, &d, sizeof d);
			listpopup->select_next();
			r = run_listpopup();
			clearmsg(msg);
			if (!r) listpopup->databuf_set(&d, sizeof d);
			return;
		}				
		}
	}
	ht_statictext::handlemsg(msg);
}

int ht_listpopup::run_listpopup()
{
	int r;
	listpopup->relocate_to(this);
	r = listpopup->run(false);
	listpopup->unrelocate_to(this);
	return r;
}

void ht_listpopup::insertstring(const char *string)
{
	listpopup->insertstring(string);
}

void ht_listpopup::setdata(ObjectStream &s)
{
	listpopup->setdata(s);
}

/*
 *	CLASS ht_label
 */

void ht_label::init(Bounds *b, const char *_text, ht_view *_connected)
{
	ht_view::init(b, VO_POSTPROCESS, 0);
	text = ht_strdup(_text);
	magicchar = strchr(text, '~');
	if (magicchar) {
		int l = strlen(text);
		memmove(magicchar, magicchar+1, l-(magicchar-text));
	}
	connected = _connected;
	if (magicchar) {
		shortcut = keyb_metakey((ht_key)*magicchar);
	} else {
		shortcut = K_INVALID;
	}
}

void ht_label::done()
{
	free(text);
	ht_view::done();
}

const char *ht_label::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_label::draw()
{
	vcp c;
	vcp sc = getcolor(palidx_generic_text_shortcut);
	if (connected->focused) {
		c = getcolor(palidx_generic_text_focused);
	} else {
		c = getcolor(palidx_generic_text_unfocused);
	}
	buf->nprint(0, 0, c, text, size.w);
	if (magicchar) buf->printChar(magicchar-text, 0, sc, *magicchar);
}

void ht_label::handlemsg(htmsg *msg)
{
	if (msg->type==mt_postprocess) {
		if (msg->msg==msg_keypressed) {
			if ((shortcut!=-1) && (msg->data1.integer==shortcut)) {
				app->focus(connected);
				dirtyview();
				clearmsg(msg);
				return;
			}
		}
	} else ht_view::handlemsg(msg);
}

/*
 *	CLASS ht_progress_indicator
 */

void	ht_progress_indicator::init(Bounds *b, const char *hint)
{
	ht_window::init(b, NULL, 0);

	Bounds c=*b;

	c.x=1;
	c.y=1;
	c.w-=c.x+2;
	c.h-=c.y+3;
	text=new ht_statictext();
	text->init(&c, NULL, align_center, true);
	insert(text);

	c.y+=2;
	c.h=1;
	ht_statictext *t=new ht_statictext();
	t->init(&c, hint, align_center);
	insert(t);
}

const char *ht_progress_indicator::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_progress_indicator::settext(const char *t)
{
	text->settext(t);
}

/*
 *	CLASS ht_color_block
 */

int vcs[16] = {
	VC_BLACK, VC_BLUE, VC_GREEN, VC_CYAN, VC_RED, VC_MAGENTA, VC_YELLOW, VC_WHITE,
	VC_LIGHT(VC_BLACK), VC_LIGHT(VC_BLUE), VC_LIGHT(VC_GREEN), VC_LIGHT(VC_CYAN), VC_LIGHT(VC_RED), VC_LIGHT(VC_MAGENTA), VC_LIGHT(VC_YELLOW), VC_LIGHT(VC_WHITE)
};

void ht_color_block::init(Bounds *b, int selected, int Flags)
{
	ht_view::init(b, VO_OWNBUFFER | VO_SELECTABLE, 0);
	VIEW_DEBUG_NAME("ht_color_block");
	flags = Flags;
	
	ht_color_block_data d;
	d.color = selected;
	databuf_set(&d, sizeof d);
	if (flags & cf_light) colors = 16; else colors = 8;
}

int ht_color_block::datasize()
{
	return sizeof (ht_color_block_data);
}

const char *ht_color_block::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_color_block::draw()
{
	clear(getcolor(palidx_generic_body));
	uint32 cursor = VCP(focused ? VC_LIGHT(VC_WHITE) : VC_BLACK, VC_TRANSPARENT);
	for (int i=0; i < colors; i++) {
		buf->printChar((i%4)*3+1, i/4, VCP(vcs[i], VC_TRANSPARENT), GC_FULL, CP_GRAPHICAL);
		buf->printChar((i%4)*3+2, i/4, VCP(vcs[i], VC_BLACK), GC_MEDIUM, CP_GRAPHICAL);
		if (i == color) {
			buf->printChar((i%4)*3, i/4, cursor, '>');
			buf->printChar((i%4)*3+3, i/4, cursor, '<');
		}
	}
	if (flags & cf_transparent) {
		buf->print(1, (colors==8) ? 2 : 4, VCP(VC_BLACK, VC_TRANSPARENT), "transparent");
		if (color == -1) {
			buf->printChar(0, (colors==8) ? 2 : 4, cursor, '>');
			buf->printChar(12, (colors==8) ? 2 : 4, cursor, '<');
		}
	}
}

void ht_color_block::getdata(ObjectStream &s)
{
	PUTX_INT32D(s, (color==-1) ? VC_TRANSPARENT : vcs[color], NULL);
}

void ht_color_block::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Left:
				if (color==-1) color=(flags & cf_light) ? 15 : 7; else
					if (color%4-1>=0) color--;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Right:
				if (color==-1) color=(flags & cf_light) ? 15 : 7; else
					if (color%4+1<=3) color++;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Up:
				if (color==-1) color=(flags & cf_light) ? 15 : 7; else
					if (color-4>=0) color-=4;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				if (color != -1) {
					if (color+4 < colors) {
						color += 4;
					} else {
						color = -1;
					}
				}
				dirtyview();
				clearmsg(msg);
				return;
		}
	}
	ht_view::handlemsg(msg);
}

void ht_color_block::setdata(ObjectStream &s)
{
	int c = GETX_INT32D(s, NULL);
	if (c == VC_TRANSPARENT) {
		color = -1; 
	} else {
		for (int i=0; i<16; i++) if (vcs[i]==c) {
			color=i;
			break;
		}
	}
	dirtyview();
}

void center_bounds(Bounds *b)
{
	Bounds c;
	app->getbounds(&c);
	b->x = (c.w - b->w) / 2;
	b->y = (c.h - b->h) / 2;     
}
