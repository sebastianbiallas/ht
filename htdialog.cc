/*
 *	HT Editor
 *	htdialog.cc
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "htclipboard.h"
#include "htctrl.h"
#include "htdialog.h"
#include "hthist.h"
#include "htidle.h"
#include "htkeyb.h"
#include "htpal.h"
#include "htstring.h"
#include "tools.h"

/*
 *	CLASS ht_dialog
 */

void ht_dialog::init(bounds *b, char *desc, UINT framestyle)
{
	ht_window::init(b, desc, framestyle);
	VIEW_DEBUG_NAME("ht_dialog");
	options&=~VO_SELBOUND;
	msgqueue=new ht_queue();
	msgqueue->init();
}

void ht_dialog::done()
{
	msgqueue->done();
	delete msgqueue;
	ht_window::done();
}

int ht_dialog::alone()
{
	return 1;
}

char *ht_dialog::defaultpalette()
{
	return palkey_generic_dialog_default;
}

ht_queued_msg *ht_dialog::dequeuemsg()
{
	return (ht_queued_msg*)msgqueue->dequeue();
}

void ht_dialog::draw()
{
	clear(getcolor(palidx_generic_body));
	ht_group::draw();
}

int ht_dialog::getstate(int *_return_val)
{
	if (_return_val) *_return_val=return_val;
	return state;
}

void ht_dialog::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_button_pressed) {
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
	if (msg->msg==msg_keypressed) {
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

int ht_dialog::run(bool modal)
{
	ht_view *orig_focused=app->getselected(), *orig_baseview=baseview;
	int oldx, oldy;
	ht_view *drawer=modal ? this : app;
	screen->getcursor(&oldx, &oldy);
	setstate(ds_normal, 0);
	((ht_group*)app)->insert(this);
	((ht_group*)app)->focus(this);
	baseview=this;
	drawer->sendmsg(msg_draw, 0);
	screen->show();
	while (getstate(0)==ds_normal) {
		if (ht_keypressed()) {
			ht_key k=ht_getkey();
			sendmsg(msg_keypressed, k);
			drawer->sendmsg(msg_draw, 0);
			screen->show();
		}
		ht_queued_msg *q;
		while ((q=dequeuemsg())) {
			htmsg m=q->msg;
			q->target->sendmsg(&m);
			sendmsg(msg_draw);
			delete q;
		}
		if (!modal) do_idle();
	}
	int return_val;
	int state=getstate(&return_val);
	screen->setcursor(oldx, oldy);
	((ht_group*)app)->remove(this);
	app->focus(orig_focused);
	baseview=orig_baseview;
	if (state!=ds_term_cancel) {
		return return_val;
	} else {
		return 0;
	}
}

/*int ht_dialog::run(bool modal)
{
	ht_view *orig_focused=app->getselected(), *orig_baseview=baseview;
	int oldx, oldy;
	ht_view *drawer=modal ? this : app;
	screen->getcursor(&oldx, &oldy);
	setstate(ds_normal, 0);

	bounds b, c;
	getbounds(&b);
	((ht_group*)baseview)->insert(this);
	getbounds(&c);
	b.x -= c.x;
	b.y -= c.y;
	move(b.x, b.y);

	baseview->focus(this);
	baseview=this;
	drawer->sendmsg(msg_draw, 0);
	screen->show();
	while (getstate(0)==ds_normal) {
		if (ht_keypressed()) {
			ht_key k=ht_getkey();
			sendmsg(msg_keypressed, k);
			drawer->sendmsg(msg_draw, 0);
			screen->show();
		}
		ht_queued_msg *q;
		while ((q=dequeuemsg())) {
			htmsg m=q->msg;
			q->target->sendmsg(&m);
			sendmsg(msg_draw);
			delete q;
		}
		if (!modal) do_idle();
	}
	int return_val;
	int state=getstate(&return_val);
	screen->setcursor(oldx, oldy);

	getbounds(&b);
	((ht_group*)orig_baseview)->remove(this);
	getbounds(&c);
	b.x -= c.x;
	b.y -= c.y;
	move(b.x, b.y);

	baseview=orig_baseview;
	baseview->focus(orig_focused);
	if (state!=ds_term_cancel) {
		return return_val;
	} else {
		return 0;
	}
}*/

void ht_dialog::queuemsg(ht_view *target, htmsg *msg)
{
	ht_queued_msg *q=new ht_queued_msg();
	q->target=target;
	q->msg=*msg;
	msgqueue->enqueue(q);
}

void ht_dialog::setstate(int st, int retval)
{
	state = st;
	return_val = retval;
}

/*
 *	CLASS ht_cluster
 */

void ht_cluster::init(bounds *b, ht_string_list *_strings)
{
	ht_view::init(b, VO_SELECTABLE | VO_OWNBUFFER | VO_POSTPROCESS, 0);
	VIEW_DEBUG_NAME("ht_cluster");
	strings=_strings;
	scount=strings->count();
	if (scount>32) scount=32;			/* cant use more than 32... */
	sel=0;
	for (int i=0; i<scount; i++) {
		char *s=strings->get_string(i);
		s=strchr(s, '~');
		if (s) {
			shortcuts[i]=ht_metakey((ht_key)*(s+1));
		} else shortcuts[i]=K_INVALID;
	}
}

void ht_cluster::done()
{
	ht_view::done();
}

char *ht_cluster::defaultpalette()
{
	return palkey_generic_dialog_default;
}

/*
 *	CLASS ht_checkboxes
 */

void ht_checkboxes::init(bounds *b, ht_string_list *strings)
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
		char *s=strings->get_string(i);
		int slen=strlen(s);
		if (slen>maxcolstrlen) maxcolstrlen=slen;
		if ((1<<i) & state) {
			buf_print(vx, vy, c, "[X]");
		} else {
			buf_print(vx, vy, c, "[ ]");
		}
		int k=0, oc=c;
		for (int q=0; q<size.w-4; q++) {
			if (!*(s+q)) break;
			if (*(s+q)=='~') {
				c=getcolor(palidx_generic_cluster_shortcut);
				continue;
			} else {
				buf_printchar(vx+k+4, vy, c, *(s+q));
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

void ht_checkboxes::getdata(ht_object_stream *s)
{
	s->putIntDec(state, 4, NULL);
}

void ht_checkboxes::handlemsg(htmsg *msg)
{
	if (msg->type==mt_postprocess) {
		if (msg->msg==msg_keypressed) {
			for (int i=0; i<scount; i++) {
				if ((shortcuts[i]!=-1) && (msg->data1.integer==shortcuts[i])) {
					sel=i;
					state=state^(1<<sel);
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
				case K_Space:
					state=state^(1<<sel);
					dirtyview();
					clearmsg(msg);
					return;
			}
		}
	}
	ht_cluster::handlemsg(msg);
}

void ht_checkboxes::setdata(ht_object_stream *s)
{
	state=s->getIntDec(4, NULL);
	dirtyview();
}

/*
 *	CLASS ht_radioboxes
 */

void ht_radioboxes::init(bounds *b, ht_string_list *strings)
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
		char *s=strings->get_string(i);
		int slen=strlen(s);
		if (slen>maxcolstrlen) maxcolstrlen=slen;
		buf_print(vx, vy, c, "( )");
		if (i==sel) {
			buf_printchar(vx+1, vy, c, CHAR_RADIO);
		}
		buf_print(vx+4, vy, c, s);
		i++;
		vy++;
		if (vy>=size.h) {
			vx+=maxcolstrlen+5;
			vy=0;
			maxcolstrlen=0;
		}
	}
}

void ht_radioboxes::getdata(ht_object_stream *s)
{
	s->putIntDec(sel, 4, NULL);
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

void ht_radioboxes::setdata(ht_object_stream *s)
{
	sel=s->getIntDec(4, NULL);
}


/*
 *	CLASS ht_history_listbox
 */
void ht_history_listbox::init(bounds *b, ht_list *hist)
{
	history = hist;
	ht_listbox::init(b);
}

int  ht_history_listbox::calc_count()
{
	return history->count();
}

void *ht_history_listbox::getfirst()
{
	if (history->count()) {
		return (void*)1;
	} else {
		return NULL;
	}
}

void *ht_history_listbox::getlast()
{
	if (history->count()) {
		return (void*)(history->count());
	} else {
		return NULL;
	}
}

void *ht_history_listbox::getnext(void *entry)
{
	UINT e=(UINT)entry;
	if (!e) return NULL;
	if (e < history->count()) {
		return (void*)(e+1);
	} else {
		return NULL;
	}
}

void *ht_history_listbox::getnth(int n)
{
	return history->get(n-1);
}

void *ht_history_listbox::getprev(void *entry)
{
	UINT e=(UINT)entry;
	if (e > 1) {
		return (void*)(e-1);
	} else {
		return NULL;
	}
}

char *ht_history_listbox::getstr(int col, void *entry)
{
	return ((ht_history_entry*)history->get((int)entry-1))->desc;
}

void ht_history_listbox::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_Delete: {
					int p = pos;
					cursor_up(1);
					history->del(p);
					update();
					if (p) cursor_down(1);
					dirtyview();
					clearmsg(msg);
					break;
				}
			}
		break;
	}
	ht_listbox::handlemsg(msg);
}

void *ht_history_listbox::quickfind(char *s)
{
	void *item = getfirst();
	int slen = strlen(s);
	while (item && (ht_strncmp(getstr(0, item), s, slen)!=0)) {
		item = getnext(item);
	}
	return item;
}

char	*ht_history_listbox::quickfind_completition(char *s)
{
	void *item = getfirst();
	char *res = NULL;
	int slen = strlen(s);
	while (item) {
		if (ht_strncmp(getstr(0, item), s, slen)==0) {
			if (!res) {
				res = ht_strdup(getstr(0, item));
			} else {
				int a = strccomm(res, getstr(0, item));
				res[a] = 0;
			}
		}
		item = getnext(item);
	}
	return res;
}


/*
 *	CLASS ht_history_popup_dialog
 */

void ht_history_popup_dialog::init(bounds *b, ht_list *hist)
{
	history = hist;
	ht_listpopup_dialog::init(b, "history");
}

void ht_history_popup_dialog::getdata(ht_object_stream *s)
{
	s->putIntDec(listbox->pos, 4, NULL);
	if (history->count()) {
		s->putString(((ht_history_entry*)history->get(listbox->pos))->desc, NULL);
	} else {
		s->putString(NULL, NULL);
	}
}

void ht_history_popup_dialog::init_text_listbox(bounds *b)
{
	listbox=new ht_history_listbox();
	((ht_history_listbox *)listbox)->init(b, history);
	insert(listbox);
}

void ht_history_popup_dialog::setdata(ht_object_stream *s)
{
}

/*
 *	CLASS ht_inputfield
 */

void ht_inputfield::init(bounds *b, int Maxtextlen, ht_list *hist)
{
	ht_view::init(b, VO_SELECTABLE, "some inputfield");
	VIEW_DEBUG_NAME("ht_inputfield");
	
	history=hist;
	maxtextlenv=Maxtextlen;
	
	textv=(byte*)malloc(maxtextlenv+1);
	curcharv=textv;
	textlenv=0;
	selstartv=0;
	selendv=0;

	text=&textv;
	curchar=&curcharv;
	textlen=&textlenv;
	maxtextlen=&maxtextlenv;
	selstart=&selstartv;
	selend=&selendv;

	insert=1;
	ofs=0;
	attachedto=0;
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

char *ht_inputfield::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_inputfield::freebuf()
{
	if (!attachedto && (text) && (*text)) free(*text);
}

void ht_inputfield::getdata(ht_object_stream *s)
{
	UINT h=s->recordStart(datasize());
	if (!attachedto) {
		s->putIntDec(*textlen, 4, NULL);
		s->putBinary(*text, *textlen, NULL);
	}
	s->recordEnd(h);
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

void ht_inputfield::isetcursor(UINT pos)
{
	if (pos<(UINT)*textlen) *curchar=*text+pos;
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

void ht_inputfield::setdata(ht_object_stream *s)
{
	UINT h=s->recordStart(datasize());
	if (!attachedto) {
		textlen=&textlenv;
		*textlen=s->getIntDec(4, NULL);
		
		if (*textlen>*maxtextlen) *textlen=*maxtextlen;
		
		s->getBinary(*text, *textlen, NULL);
		
		curchar=&curcharv;
		*curchar=*text+*textlen;
		
		if (*textlen) {
			*selstart=*text;
			*selend=*text+*textlen;
		} else {
			*selstart=0;
			*selend=0;
		}

		ofs=0;
	}
	
	s->recordEnd(h);
	dirtyview();
}

/*
 *	CLASS ht_strinputfield
 */

void ht_strinputfield::init(bounds *b, int maxstrlen, ht_list *history)
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
	if (*curchar-*text<ofs) ofs=*curchar-*text; else
	if (*curchar-*text-(size.w-2)*size.h+1>ofs) ofs=*curchar-*text-(size.w-2)*size.h+1;
}

void ht_strinputfield::draw()
{
	int c=focused ? getcolor(palidx_generic_input_focused) :
		getcolor(palidx_generic_input_unfocused);
	byte *t=*text+ofs;
	int l=*textlen-ofs;
	if (l>size.w) l=size.w;
	int y=0;
	fill(0, 0, size.w, size.h, c, ' ');
	if (ofs) buf_printchar(0, y, getcolor(palidx_generic_input_clip), '<');
	for (int k=0; k<*textlen-ofs; k++) {
		if (1+k-y*(size.w-2)>size.w-2) {
			if (y+1<size.h) y++; else break;
		}
		if ((t<*selstart) || (t>=*selend)) {
			buf_printchar(1+k-y*(size.w-2), y, c, *(t++));
		} else {
			buf_printchar(1+k-y*(size.w-2), y, getcolor(palidx_generic_input_selected), *(t++));
		}
	}
	if (*textlen-ofs > (size.w-2)*size.h) {
		buf_printchar(size.w-1, y, getcolor(palidx_generic_input_clip), '>');
	}
	if (history && history->count()) {
		buf_printchar(size.w-1, y+size.h-1, getcolor(palidx_generic_input_clip), CHAR_ARROW_DOWN);
	}
	if (focused) {
		if (*curchar-*text-ofs>=(size.w-2)*size.h) setcursor(size.w-1, size.h-1); else setcursor((*curchar-*text-ofs)%(size.w-2)+1, (*curchar-*text-ofs)/(size.w-2));
	}
}

void ht_strinputfield::handlemsg(htmsg *msg)
{
	if ((msg->type==mt_empty) && (msg->msg==msg_keypressed)) {
		int k = msg->data1.integer;
		switch (k) {
			case K_Alt_S:
				selectmode=!selectmode;
				clearmsg(msg);
				break;
			case K_Up:
				is_virgin=0;
				if (*curchar-*text-ofs<size.w-2) {
					ofs-=size.w-2;
					if (ofs<0) ofs=0;
				}
				*curchar-=size.w-2;
				if (*curchar<*text) *curchar=*text;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Down:
				is_virgin=0;
				if (*curchar+size.w-2-*text>*textlen) {
					history_dialog();
					clearmsg(msg);
					return;
				} else {
					*curchar+=size.w-2;
					if (*curchar-*text>*textlen) *curchar=*text+*textlen;
					correct_viewpoint();
					dirtyview();
				}
				clearmsg(msg);
				return;
			case K_Shift_Left:
			case K_Left:
				is_virgin=0;
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
				is_virgin=0;
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
			case K_BackSpace:
				if (is_virgin) {
					is_virgin=0;
					*selstart=0;
					*selend=0;
					*textlen=0;
					*curchar=*text;
					dirtyview();
					clearmsg(msg);
				} else if (*curchar>*text) {
					*selstart=0;
					*selend=0;
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
					is_virgin=0;
					*selstart=0;
					*selend=0;
					*textlen=0;
					*curchar=*text;
					dirtyview();
					clearmsg(msg);
				} else if ((*curchar-*text<*textlen) && (*textlen)) {
					if (*selstart) {
						if (*curchar>=*selstart) {
							if (*curchar<*selend) (*selend)--;
							if (*selstart==*selend) {
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
				is_virgin=0;
				if ((k==K_Shift_Home) != selectmode) {
					select_add(*curchar, *text);
				}					
				*curchar=*text;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Shift_End:
			case K_End:
				is_virgin=0;
				if ((k==K_Shift_End) != selectmode) {
					select_add(*curchar, *text+*textlen);
				}						
				*curchar=*text+*textlen;
				correct_viewpoint();
				dirtyview();
				clearmsg(msg);
				return;
			case K_Insert:
				insert=!insert;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Alt_X:
			case K_Shift_Delete:
				if (*selend>*selstart) clipboard_copy("inputfield", *selstart, *selend-*selstart);
			case K_Alt_D:
			case K_Control_Delete:
				if (*selend>*selstart) {
					memmove(*selstart, *selend, *textlen-(*selend-*text));
					*textlen-=*selend-*selstart;
					*curchar=*selstart;
					*selstart=0;
					*selend=0;
					is_virgin=0;
					correct_viewpoint();
				}
				dirtyview();
				clearmsg(msg);
				return;
			case K_Alt_C:
			case K_Control_Insert:
				if (*selend>*selstart) clipboard_copy("inputfield", *selstart, *selend-*selstart);
				is_virgin=0;
				dirtyview();
				clearmsg(msg);
				return;
			case K_Alt_V:
			case K_Shift_Insert: {
				int maxsize=MIN(*maxtextlen-*textlen, (int)clipboard_getsize());
				byte *buf=(byte*)malloc(maxsize);
				int r=clipboard_paste(buf, maxsize);
				if (r) {
					for (int i=0; i<r; i++) {
						setbyte(buf[r-i-1]);
					}
					*selstart=*curchar;
					*selend=*curchar+r;
				}
				delete buf;
				is_virgin=0;
				dirtyview();
				clearmsg(msg);
				return;
			}
			default:
				if ((msg->data1.integer>=' ') && (msg->data1.integer<256)) {
					if (is_virgin) {
						is_virgin=0;
						*selstart=0;
						*selend=0;
						*textlen=0;
						*curchar=*text;
						ofs=0;
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
		bounds b;
		getbounds(&b);
		b.y--;
		b.h=8;
		ht_history_popup_dialog *l=new ht_history_popup_dialog();
		l->init(&b, history);
		if (l->run(false)) {
			ht_listpopup_dialog_data d;
			l->databuf_get(&d);

			if (d.cursor_string) {
				ht_history_entry *v=(ht_history_entry*)history->get(d.cursor_id);
				if ((v->data) && (group)) {
					v->datafile->seek(0);
					group->setdata(v->data);
				} else {
					ht_inputfield_data e;
					e.textlen=strlen(d.cursor_string);
					e.text=(byte*)d.cursor_string;
					databuf_set(&e);
				}
			}
			is_virgin=0;
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
		is_virgin=0;
		return true;
	}
	return false;
}

bool ht_strinputfield::setbyte(byte a)
{
	if ((insert) || (*curchar-*text>=*textlen)) {
		if ((insertbyte(*curchar, a)) && (*curchar-*text<*textlen)) return true;
	} else {
		**curchar=a;
		return true;
	}
	return false;
}

/*
 *	CLASS ht_hexinputfield
 */

void ht_hexinputfield::init(bounds *b, int maxstrlen)
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
	if (*curchar-*text<ofs) ofs=*curchar-*text; else
	if ((*curchar-*text)*3-(size.w-2)*size.h+5>ofs*3) ofs=((*curchar-*text)*3-(size.w-2)*size.h+5)/3;
}

void ht_hexinputfield::draw()
{
	int c=focused ? getcolor(palidx_generic_input_focused) :
		getcolor(palidx_generic_input_unfocused);
	char hbuf[256], *h=hbuf;
	int y=0;
	fill(0, 0, size.w, size.h, c, ' ');
	if (ofs) buf_print(0, y, getcolor(palidx_generic_input_clip), "<");
	int vv=*textlen-ofs;
	if (vv<0) vv=0; else if (vv>(size.w-2)*size.h/3) vv=(size.w-2)*size.h/3+1;
	for (int k=0; k<vv; k++) {
		h+=sprintf(h, "%02x ", *(*text+k+ofs));
	}
	if (vv) {
		h=hbuf;
		while ((*h) && (y<size.h)) {
			h+=buf_lprint(1, y, c, size.w-2, h);
			y++;
		}
		y--;
	}
	if ((*textlen-ofs)*3 > (size.w-2)*size.h) {
		buf_print(size.w-1, y, getcolor(palidx_generic_input_clip), ">");
	}
	if (focused) {
		if ((*curchar-*text-ofs)*3+nib+1>=(size.w-2)*size.h) setcursor(size.w-1, size.h-1); else
		setcursor(((*curchar-*text-ofs)*3+nib+1)%(size.w-2), ((*curchar-*text-ofs)*3+nib+1)/(size.w-2));
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
			case K_BackSpace:
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
	if ((nib) && (*curchar-*text==*textlen)) {
		nib=0;
	}
	ht_inputfield::receivefocus();
}

void ht_hexinputfield::setnibble(byte a)
{
	if (((insert) || (*curchar-*text>=*textlen)) && (nib==0)) {
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

void ht_button::init(bounds *b, char *Text, int Value)
{
	ht_view::init(b, VO_SELECTABLE | VO_OWNBUFFER | VO_POSTPROCESS, "some button");
	VIEW_DEBUG_NAME("ht_button");
	value=Value;
	text=ht_strdup(Text);
	pressed=0;
	magicchar=strchr(text, '~');
	if (magicchar) {
		int l=strlen(text);
		memmove(magicchar, magicchar+1, l-(magicchar-text));
		shortcut1=ht_metakey((ht_key)tolower(*magicchar));
		shortcut2=(ht_key)tolower(*magicchar);
	} else {
		shortcut1=K_INVALID;
		shortcut2=K_INVALID;
	}
}

void ht_button::done()
{
	if (text) delete text;
	ht_view::done();
}

char *ht_button::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_button::draw()
{
	int c=focused ? getcolor(palidx_generic_button_focused) :
		getcolor(palidx_generic_button_unfocused);
	fill(0, 0, size.w-1, size.h-1, c, ' ');
	int xp=(size.w-strlen(text))/2, yp=(size.h-1)/2;
	buf_print(xp, yp, c, text);
	if (magicchar) buf_printchar(xp+(magicchar-text), yp, getcolor(palidx_generic_button_shortcut), *magicchar);
/* shadow */
	buf_printchar(0, 1, getcolor(palidx_generic_button_shadow), ' ');
	for (int i=1; i<size.w-1; i++) {
		buf_printchar(i, 1, getcolor(palidx_generic_button_shadow), CHAR_FILLED_HU);
	}
	buf_printchar(size.w-1, 0, getcolor(palidx_generic_button_shadow), CHAR_FILLED_HL);
	buf_printchar(size.w-1, 1, getcolor(palidx_generic_button_shadow), CHAR_FILLED_HU);
}

void ht_button::handlemsg(htmsg *msg)
{
	if (msg->type==mt_postprocess) {
		if (msg->msg==msg_keypressed) {
			if (((shortcut1!=K_INVALID) && (msg->data1.integer==shortcut1)) ||
			((shortcut2!=K_INVALID) && (msg->data1.integer==shortcut2))) {
				push();
				dirtyview();
				clearmsg(msg);
				return;
			}
		}
	} else if (msg->type==mt_empty) {
		if (msg->msg==msg_keypressed) {
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
 *	ht_listbox
 */

class ht_listbox_vstate: public ht_data {
public:
	void *e_top;
	void *e_cursor;

	ht_listbox_vstate(void *top, void *cursor)
	{
		e_top = top;
		e_cursor = cursor;
	}
};

void ht_listbox::init(bounds *b, UINT Listboxcaps)
{
	ht_view::init(b, VO_SELECTABLE | VO_OWNBUFFER | VO_RESIZE, 0);

	growmode = GM_HDEFORM | GM_VDEFORM;

	bounds c=*b;
	c.x=c.w-1;
	c.y=0;
	c.w=1;
	scrollbar=new ht_scrollbar();
	scrollbar->init(&c, &pal, true);

	pos = 0;
	cursor = 0;
	e_top = getfirst();
	e_cursor = e_top;
	visible_height = 0;
	x = 0;
	clear_quickfind();
	update();
	listboxcaps = Listboxcaps;
}

int 	ht_listbox::load(ht_object_stream *f)
{
	return 0;
}

void	ht_listbox::done()
{
	scrollbar->done();
	delete scrollbar;

	ht_view::done();
}

void ht_listbox::adjust_pos_hack()
{
	if (e_cursor!=e_top) return;
	int i=0;
	void *tmp = e_cursor;
	if (!tmp) return;
	while ((tmp) && (i<=visible_height)) {
		tmp = getnext(tmp);
		i++;
	}
	if (i<visible_height) {
		cursor_down(cursor_up(visible_height-pos-i));
	}
}

void ht_listbox::adjust_scrollbar()
{
	int pstart, psize;
	if (scrollbar_pos(pos-cursor, size.h, count, &pstart, &psize)) {
		scrollbar->enable();
		scrollbar->setpos(pstart, psize);
	} else {
		scrollbar->disable();
	}
}

void ht_listbox::clear_quickfind()
{
	quickfinder[0] = 0;
	qpos = quickfinder;
	update_cursor();
}

int  ht_listbox::cursor_adjust()
{
	return 0;
}

int  ht_listbox::cursor_up(int n)
{
	void *tmp;
	int  i = 0;

	while (n--) {
		tmp = getprev(e_cursor);
		if (!tmp) break;
		if (e_cursor==e_top) {
			e_top = tmp;
		} else {
			cursor--;
		}
		e_cursor = tmp;
		pos--;
		if (pos<0) pos = 0; // if cursor was out of sync
		i++;
	}
	return i;
}

int  ht_listbox::cursor_down(int n)
{
	void *tmp;
	int  i = 0;

	while (n--) {
		tmp = getnext(e_cursor);
		if (!tmp) break;
		if (cursor+1>=visible_height) {
			e_top = getnext(e_top);
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
	return sizeof(ht_listbox_data);
}

char *ht_listbox::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_listbox::draw()
{
	int fc = focused ? getcolor(palidx_generic_list_focused_unselected) :
		getcolor(palidx_generic_list_unfocused_unselected);

	clear(fc);

	void *entry = e_top;

	int i=0;
	int cols = num_cols();
	while ((entry) && (i < visible_height)) {
		int c=(i==cursor) ? (focused ? getcolor(palidx_generic_list_focused_selected) :
			getcolor(palidx_generic_list_unfocused_selected)) : fc;
		if (i==cursor) {
			fill(0, i, size.w, 1, c, ' ');
		}
		int X = -x;
		for (int j=0; j<cols; j++) {
			char *s = getstr(j, entry);
			int slen = strlen(s);
			if (s) {
				if (X > 0) {
					buf_lprint(X, i, c, size.w, s);
				} else {
					if (slen > -X) buf_lprint(0, i, c, size.w, &s[-X]);
				}
			}
			X += slen;
			if (j+1<cols) {
				buf_printchar(X++, i, c, ' ');
				buf_printchar(X++, i, c, CHAR_LINEV);
				buf_printchar(X++, i, c, ' ');
			}
		}
		entry = getnext(entry);
		i++;
	}
	update_cursor();
/*     char dbg[100];
	sprintf(dbg, "cursor=%d pos=%d vh=%d qc:%s", cursor, pos, visible_height, quickfinder);
	lprint(0, 0, 1, size.w, dbg);
	sprintf(dbg, "e_top=%s", getstr(0, e_top));
	lprint(0, 1, 1, size.w, dbg);
	sprintf(dbg, "e_cursor=%s", getstr(0, e_cursor));
	lprint(0, 2, 1, size.w, dbg);*/
}

int  ht_listbox::estimate_entry_pos(void *entry)
{
	// this is slow!
	void *tmp = getfirst();
	int res = 0;
	while (tmp) {
		if (tmp==entry) break;
		tmp = getnext(tmp);
		res++;
	}
	return (tmp==entry) ? res : -1;
}

void *ht_listbox::getbyid(UINT id)
{
	void *p = getfirst();
	while (p) {
		if (id == getid(p)) return p;
		p = getnext(p);
	}
	return NULL;
}

void ht_listbox::getdata(ht_object_stream *s)
{
/* FIXME: fixme */
	s->putIntDec(getid(e_top), 4, NULL);
	s->putIntDec(getid(e_cursor), 4, NULL);
}

UINT ht_listbox::getid(void *entry)
{
	return (UINT)entry;
}

void *ht_listbox::getnth(int n)
{
	void *e=getfirst();
	while (n--) e=getnext(e);
	return e;
}

void ht_listbox::goto_item(void *entry)
{
	if (!entry) return;
	void *tmp = e_top;
	int i=0;
	bool ok=false;
	pos -= cursor;
	if (pos<0) pos = 0; // if cursor was out of sync
	cursor = 0;

	while ((tmp) && (i < visible_height)) {
		if (tmp == entry) {
			ok = true;
			break;
		}
		pos++;
		cursor++;
		i++;
		tmp = getnext(tmp);
	}
	e_cursor = entry;
	if (!ok) {
		e_top = entry;
		cursor = 0;
		pos = estimate_entry_pos(entry);
		assert(pos != -1);
	}
	adjust_pos_hack();
	adjust_scrollbar();
}

void ht_listbox::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_vstate_restore: {
			ht_listbox_vstate *vs = (ht_listbox_vstate*)msg->data1.ptr;
			e_top = vs->e_top;
			e_cursor = vs->e_cursor;
			update();
// FIXME: what about deleting entries !!!
			clearmsg(msg);
			return;
		}
		case msg_keypressed: switch (msg->data1.integer) {
			case K_Control_PageUp:
			case K_Home:
				clear_quickfind();
				pos = cursor = 0;
				e_top = e_cursor = getfirst();
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			case K_Control_PageDown:
			case K_End: {
				clear_quickfind();
				cursor = 0;
				pos = count ? count-1 : 0;
				e_cursor = e_top = getlast();
				cursor_up(visible_height-1);
				cursor_down(visible_height-1);
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			}
			case K_PageUp:
				clear_quickfind();
				cursor_up(visible_height-1);
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			case K_PageDown: {
				clear_quickfind();
				cursor_down(visible_height-1);
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			}
			case K_Up:
				clear_quickfind();
				cursor_up(1);
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			case K_Down:
				clear_quickfind();
				cursor_down(1);
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			case K_Left:
			case K_Control_Left:
				if (x > 0) x--;
				update_cursor();
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			case K_Right:
			case K_Control_Right:
				x++;
				update_cursor();
				dirtyview();
				clearmsg(msg);
				adjust_scrollbar();
				return;
			case K_Tab:
				if (listboxcaps & LISTBOX_QUICKFIND) {
					if (*quickfinder) {
						char *qc = quickfind_completition(quickfinder);
						if (qc) {
							strcpy(quickfinder, qc);
							qpos = strend(quickfinder);
							free(qc);
							goto qf;
						}
					}
				}
				break;
			case K_BackSpace: {
				if (listboxcaps & LISTBOX_QUICKFIND) {
					if (qpos > quickfinder) {
						*(--qpos) = 0;
						qf:
						void *a = quickfind(quickfinder);
						if (a) {
							goto_item(a);
							update_cursor();
							dirtyview();
						}
						clearmsg(msg);
						adjust_scrollbar();
					}
				}
				return;
			}
			default: {
				if ((listboxcaps & LISTBOX_QUICKFIND) && (msg->data1.integer > 31) && (msg->data1.integer < 0xff)) {
					*(qpos++) = msg->data1.integer;
					*qpos = 0;
					void *a = quickfind(quickfinder);
					if (a) {
						goto_item(a);
						update_cursor();
						dirtyview();
						clearmsg(msg);
						adjust_scrollbar();
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

int	ht_listbox::num_cols()
{
	return 1;
}

char	*ht_listbox::quickfind_completition(char *s)
{
	return ht_strdup(s);
}

void ht_listbox::redraw()
{
	ht_view::redraw();
	scrollbar->relocate_to(this);
//	fprintf(stderr, "scrollbar: x=%d, y=%d, w=%d, h=%d\n", scrollbar->vsize.x, scrollbar->vsize.y, scrollbar->vsize.w, scrollbar->vsize.h);
	scrollbar->redraw();
	scrollbar->unrelocate_to(this);
}

bool ht_listbox::seek(int index)
{
	clear_quickfind();
	goto_item(getnth(index));
	adjust_scrollbar();
	return true;
//	if (index < count) {
//		cursor = index;
/*		pos = index;
		if (cursor > visible_height-1) cursor = visible_height-1;
		void *e = getfirst();
		for (int i=0; i<cursor; i++) {
			e = getnext(e);
		}
		e_cursor = e;
		adjust_scrollbar();
		return true;
//	}
	return false;*/
}

void ht_listbox::select_entry(void *entry)
{
}

void ht_listbox::setdata(ht_object_stream *s)
{
	e_top = getbyid(s->getIntDec(4, NULL));
	e_cursor = getbyid(s->getIntDec(4, NULL));
	update();
}

void	ht_listbox::store(ht_object_stream *f)
{
}

ht_data *ht_listbox::vstate_create()
{
	return new ht_listbox_vstate(e_top, e_cursor);
}

void ht_listbox::vstate_save()
{
	ht_data *vs = vstate_create();
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
	void *entry = getfirst();
	count = calc_count();
	visible_height = MIN(size.h, count);
	if (count <= size.h) {
		if (!e_cursor) e_cursor = getfirst();
		cursor = 0;
		while (entry && (cursor < visible_height)) {
			if (entry == e_cursor) {
				e_top = getfirst();
				goto ok;
			}
			entry = getnext(entry);
			cursor++;
		}
	}
	if (!e_top) {
		e_top = getfirst();
	}     
	if (!e_cursor) e_cursor = e_top;
	entry = e_top;
	cursor = 0;
	while (entry && (cursor < visible_height)) {
		if (entry == e_cursor) goto ok;
		entry = getnext(entry);
		cursor++;
	}
	cursor = 0;
	e_top = e_cursor;
ok:
	adjust_pos_hack();
	adjust_scrollbar();
	dirtyview();
}

void ht_listbox::update_cursor()
{
	if (focused) {
		if (*quickfinder) {
			setcursor((qpos-quickfinder)+cursor_adjust()-x, cursor);
		} else {
			hidecursor();
		}
	}		
}

/*
 *	ht_text_listbox
 */

void	ht_text_listbox::init(bounds *b, int Cols, int Keycol, UINT Listboxcaps)
{
	cols = Cols;
	keycol = Keycol;
	widths = (int *) calloc(sizeof(int)*cols, 1);
	first = last = NULL;
	count = 0;
	return_str = (char *) malloc(1024);
	Cursor_adjust = 0;
	ht_listbox::init(b, Listboxcaps);
}

void ht_text_listbox::done()
{
	ht_text_listbox_item *temp = first;
	while (temp) {
		ht_text_listbox_item *temp2 = temp->next;
		for (int i=0; i<cols; i++) {
			free(temp->data[i]);
		}
		free(temp);
		temp = temp2;
	}
	free(widths);
	free(return_str);
	ht_listbox::done();
}

int 	ht_text_listbox::load(ht_object_stream *f)
{
	return 0;
}

int	ht_text_listbox::calc_count()
{
	return count;
}

int	ht_text_listbox::compare_strn(char *s1, char *s2, int l)
{
	return ht_strncmp(s1, s2, l);
}

int	ht_text_listbox::compare_ccomm(char *s1, char *s2)
{
	return strccomm(s1, s2);
}

int  ht_text_listbox::cursor_adjust()
{
	return Cursor_adjust;
}

void *ht_text_listbox::getfirst()
{
	return first;
}

UINT ht_text_listbox::getid(void *entry)
{
	if (entry) {
		return ((ht_text_listbox_item *)entry)->id;
	} else {
		return 0;
	}	    
}

void *ht_text_listbox::getlast()
{
	return last;
}

void *ht_text_listbox::getnext(void *entry)
{
	if (!entry) return NULL;
	return ((ht_text_listbox_item *)entry)->next;
}

void *ht_text_listbox::getprev(void *entry)
{
	if (!entry) return NULL;
	return ((ht_text_listbox_item *)entry)->prev;
}

char *ht_text_listbox::getstr(int col, void *entry)
{
	memset(return_str, ' ', widths[col]);
	return_str[widths[col]] = 0;
	if (entry && (col < cols)) {
		char *str = ((ht_text_listbox_item *)entry)->data[col];
		int sl = strlen(str);
		memmove(return_str, str, MIN(1024, sl));
	}
	return return_str;
}

void ht_text_listbox::goto_item_by_id(UINT id)
{
	void *e = getfirst();
	while (e && getid(e) != id) {
		e = getnext(e);
	}
	if (e) {
		goto_item(e);
	}
}

void ht_text_listbox::goto_item_by_position(UINT pos)
{
	goto_item(getnth(pos));
}

void ht_text_listbox::insert_str(int id, char *str, ...)
{
	ht_text_listbox_item *item = (ht_text_listbox_item *)malloc(sizeof(ht_text_listbox_item)+sizeof(char *)*cols);
	item->next = NULL;
	item->prev = last;
	item->id = id;
	va_list str2;
	va_start(str2, str);
	char *str3 = str;
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

int	ht_text_listbox::num_cols()
{
	return cols;
}

void *ht_text_listbox::quickfind(char *s)
{
	ht_text_listbox_item *item = first;
	int slen = strlen(s);
	while (item && (compare_strn(item->data[keycol], s, slen)!=0)) {
		item = item->next;
	}
	return item;
}

char	*ht_text_listbox::quickfind_completition(char *s)
{
	ht_text_listbox_item *item = first;
	char *res = NULL;
	int slen = strlen(s);
	while (item) {
		if (compare_strn(item->data[keycol], s, slen)==0) {
			if (!res) {
				res = ht_strdup(item->data[keycol]);
			} else {
				int a = compare_ccomm(res, item->data[keycol]);
				res[a] = 0;
			}
		}
		item = item->next;
	}
	return res;
}

int ht_text_listboxcomparatio(ht_text_listbox_item *a, ht_text_listbox_item *b, int count, ht_text_listbox_sort_order *so)
{
	for (int i=0;i<count;i++) {
		int r = so[i].compare_func(a->data[so[i].col], b->data[so[i].col]);
		if (r!=0) return r;
	}
	return 0;
}

static void ht_text_listboxqsort(int l, int r, int count, ht_text_listbox_sort_order *so, ht_text_listbox_item **list)
{
	int m=(l+r)/2;
	int L=l;
	int R=r;
	ht_text_listbox_item *c=list[m];
	do {
		while (ht_text_listboxcomparatio(list[l], c, count, so)<0) l++;
		while (ht_text_listboxcomparatio(list[r], c, count, so)>0) r--;
		if (l<=r) {
			ht_text_listbox_item *t=list[l];
			list[l]=list[r];
			list[r]=t;
			l++;
			r--;
		}
	} while(l<r);
	if (L<r) ht_text_listboxqsort(L, r, count, so, list);
	if (l<R) ht_text_listboxqsort(l, R, count, so, list);
}

void ht_text_listbox::sort(int count, ht_text_listbox_sort_order *so)
{
	ht_text_listbox_item **list;
	ht_text_listbox_item *tmp;
	int i=0;
	int cnt = calc_count();

	if (cnt<2) return;
	
	list = (ht_text_listbox_item **)malloc(cnt*4);
	tmp = first;
	while (tmp) {
		list[i++] = tmp;
		tmp = (ht_text_listbox_item *)getnext(tmp);
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

	update();
	adjust_scrollbar();
}

void	ht_text_listbox::store(ht_object_stream *f)
{
}

void ht_text_listbox::update()
{
	ht_listbox::update();
	Cursor_adjust = 0;
	for (int i=0; i<keycol; i++) {
		Cursor_adjust+=widths[i]+3;
	}
}

/*
 *	CLASS ht_itext_listbox
 */

void	ht_itext_listbox::init(bounds *b, int Cols, int Keycol)
{
	ht_text_listbox::init(b, Cols, Keycol);
}

void	ht_itext_listbox::done()
{
	ht_text_listbox::done();
}

int	ht_itext_listbox::compare_strn(char *s1, char *s2, int l)
{
	return ht_strnicmp(s1, s2, l);
}

int	ht_itext_listbox::compare_ccomm(char *s1, char *s2)
{
	return strcicomm(s1, s2);
}

/*
 *	CLASS ht_statictext
 */

#define STATICTEXT_MIN_LINE_FILL 70	/* percent */

void ht_statictext_align(ht_statictext_linedesc *d, int align, int w)
{
	switch (align) {
		case align_center:
			d->ofs=(w-d->len)/2;
			break;
		case align_right:
			d->ofs=w-d->len;
			break;
		default:
			d->ofs=0;
			break;
	}
}

void ht_statictext::init(bounds *b, char *t, int al, bool breakl, bool trans)
{
	ht_view::init(b, VO_OWNBUFFER | VO_RESIZE, "some statictext");
	VIEW_DEBUG_NAME("ht_statictext");

	align=al;
	breaklines=breakl;
	transparent=trans;
	text=ht_strdup(t);
}

void ht_statictext::done()
{
	if (text) delete text;
	ht_view::done();
}

char *ht_statictext::defaultpalette()
{
	return palkey_generic_dialog_default;
}

#define ssst_word		0
#define ssst_separator	1
#define ssst_whitespace	2

int get_ssst(char s)
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
	char *t=gettext();
	if (!t) return;
	if (breaklines) {
/* format string... */	
		ht_statictext_linedesc *orig_d=(ht_statictext_linedesc *)malloc(sizeof (ht_statictext_linedesc)*size.h);
		ht_statictext_linedesc *d=orig_d;
		int lalign=align;
		int c=0;
		while ((*t) && (c<size.h)) {
/* custom alignment */
			if ((*t==ALIGN_CHAR_ESCAPE) && (align==align_custom)) {
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
			char *bp=t+1, *n=t+1;
			int ssst=get_ssst(t[i]);
			while (t[i]) {
				if ((i+1>size.w) || (t[i]=='\n') || !(t[i+1])) {
					bool kill_ws=(t[i]!='\n');
					if (i+1<=size.w) {
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
		d=orig_d;
		for (int i=0; i<c; i++) {
			buf_lprint(d->ofs, i, gettextcolor(), d->len, d->text);
			d++;
		}
		free(orig_d);
	} else {
		int o=0;
		buf_print(o, 0, gettextcolor(), t);
	}
}

vcp ht_statictext::gettextcolor()
{
	return getcolor(palidx_generic_body);
//	return VCP(VC_RED, VC_BLACK);
}

char *ht_statictext::gettext()
{
	return text;
}

void ht_statictext::settext(char *_text)
{
	if (text) delete text;
	text=ht_strdup(_text);
	dirtyview();
}

/*
 *	CLASS ht_listpopup_dialog
 */

void ht_listpopup_dialog::init(bounds *b, char *desc)
{
	ht_dialog::init(b, desc, FS_TITLE | FS_MOVE);
	VIEW_DEBUG_NAME("ht_listpopup_dialog");

	bounds c;
	getclientarea(&c);
	c.x=0;
	c.y=0;
	init_text_listbox(&c);
}

void ht_listpopup_dialog::done()
{
	ht_dialog::done();
}

int	ht_listpopup_dialog::datasize()
{
	return sizeof (ht_listpopup_dialog_data);
}

char *ht_listpopup_dialog::defaultpalette()
{
	return palkey_generic_blue;
}

void ht_listpopup_dialog::getdata(ht_object_stream *s)
{
	ht_listbox_data d;
	listbox->databuf_get(&d);

	s->putIntDec(d.cursor_id, 4, NULL);

	ht_text_listbox_item *cursor = (ht_text_listbox_item*) listbox->getbyid(d.cursor_id);
	if (cursor) {
		s->putString(cursor->data[0], NULL);
	} else {
		s->putString(NULL, NULL);
	}
}

void ht_listpopup_dialog::init_text_listbox(bounds *b)
{
	listbox=new ht_text_listbox();
	((ht_text_listbox *)listbox)->init(b);
	insert(listbox);
}

void ht_listpopup_dialog::insertstring(char *string)
{
	((ht_text_listbox *)listbox)->insert_str(listbox->calc_count(), string);
	listbox->update();
}

void ht_listpopup_dialog::select_next()
{
	listbox->cursor_down(1);
}

void ht_listpopup_dialog::select_prev()
{
	listbox->cursor_up(1);
}

void ht_listpopup_dialog::setdata(ht_object_stream *s)
{
	int cursor_id=s->getIntDec(4, NULL);
	s->getString(NULL);	/* ignored */
	
	listbox->seek(cursor_id);
}

/*
 *	CLASS ht_listpopup
 */

void	ht_listpopup::init(bounds *b)
{
	ht_statictext::init(b, 0, align_left, 0);
	setoptions(options|VO_SELECTABLE);
	VIEW_DEBUG_NAME("ht_listpopup");

	bounds c=*b;
	c.x=0;
	c.y=0;
	c.h=8;
	
	listpopup=new ht_listpopup_dialog();
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
	buf_printchar(size.w-1, 0, gettextcolor(), CHAR_ARROW_DOWN);
}

vcp ht_listpopup::gettextcolor()
{
	return focused ? getcolor(palidx_generic_input_selected) :
		getcolor(palidx_generic_input_focused);
}

void ht_listpopup::getdata(ht_object_stream *s)
{
	listpopup->getdata(s);
}

char *ht_listpopup::gettext()
{
	ht_listpopup_dialog_data d;
	listpopup->databuf_get(&d);
	return d.cursor_string;
}

void ht_listpopup::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Up: {
				int r;
				ht_listpopup_dialog_data d;
				listpopup->databuf_get(&d);
				listpopup->select_prev();
				r=run_listpopup();
				clearmsg(msg);
				if (!r) listpopup->databuf_set(&d);
				return;
			}
			case K_Down: {
				int r;
				ht_listpopup_dialog_data d;
				listpopup->databuf_get(&d);
				listpopup->select_next();
				r=run_listpopup();
				clearmsg(msg);
				if (!r) listpopup->databuf_set(&d);
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
	r=listpopup->run(false);
	listpopup->unrelocate_to(this);
	return r;
}

void ht_listpopup::insertstring(char *string)
{
	listpopup->insertstring(string);
}

void ht_listpopup::setdata(ht_object_stream *s)
{
	listpopup->setdata(s);
}

/*
 *	CLASS ht_listbox_ptr
 */

ht_listbox_ptr::ht_listbox_ptr(ht_listbox *_listbox)
{
	listbox=_listbox;
}

ht_listbox_ptr::~ht_listbox_ptr()
{
}

/*
 *	CLASS ht_label
 */

void ht_label::init(bounds *b, char *_text, ht_view *_connected)
{
	ht_view::init(b, VO_POSTPROCESS, 0);
	text = ht_strdup(_text);
	magicchar = strchr(text, '~');
	if (magicchar) {
		int l = strlen(text);
		memmove(magicchar, magicchar+1, l-(magicchar-text));
	}
	connected=_connected;
	if (magicchar) {
		shortcut = ht_metakey((ht_key)*magicchar);
	} else shortcut = K_INVALID;
}

void ht_label::done()
{
	if (text) free(text);
	ht_view::done();
}

char *ht_label::defaultpalette()
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
	buf_lprint(0, 0, c, size.w, text);
	if (magicchar) buf_printchar(magicchar-text, 0, sc, *magicchar);
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

void	ht_progress_indicator::init(bounds *b, char *hint)
{
	ht_window::init(b, NULL, 0);

	bounds c=*b;

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

char *ht_progress_indicator::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_progress_indicator::settext(char *t)
{
	text->settext(t);
}

/*
 *	CLASS ht_color_block
 */

int vcs[16]={VC_BLACK, VC_BLUE, VC_GREEN, VC_CYAN, VC_RED, VC_MAGENTA, VC_YELLOW, VC_WHITE,   VC_LIGHT(VC_BLACK), VC_LIGHT(VC_BLUE), VC_LIGHT(VC_GREEN), VC_LIGHT(VC_CYAN), VC_LIGHT(VC_RED), VC_LIGHT(VC_MAGENTA), VC_LIGHT(VC_YELLOW), VC_LIGHT(VC_WHITE)};

void ht_color_block::init(bounds *b, int selected, int Flags)
{
	ht_view::init(b, VO_OWNBUFFER | VO_SELECTABLE, 0);
	VIEW_DEBUG_NAME("ht_color_block");
	flags=Flags;
	
	ht_color_block_data d;
	d.color=selected;
	databuf_set(&d);
	if (flags & cf_light) colors=16; else colors=8;
}

void ht_color_block::done()
{
	ht_view::done();
}

int ht_color_block::datasize()
{
	return sizeof (ht_color_block_data);
}

char *ht_color_block::defaultpalette()
{
	return palkey_generic_dialog_default;
}

void ht_color_block::draw()
{
	clear(getcolor(palidx_generic_body));
	dword cursor=VCP(focused ? VC_LIGHT(VC_WHITE) : VC_BLACK, VC_TRANSPARENT);
	for (int i=0; i<colors; i++) {
		buf_printchar((i%4)*3+1, i/4, VCP(vcs[i], VC_TRANSPARENT), CHAR_FILLED_F);
		buf_printchar((i%4)*3+2, i/4, VCP(vcs[i], VC_BLACK), CHAR_FILLED_M);
		if (i==color) {
			buf_printchar((i%4)*3, i/4, cursor, '>');
			buf_printchar((i%4)*3+3, i/4, cursor, '<');
		}
	}
	if (flags & cf_transparent) {
		buf_print(1, (colors==8) ? 2 : 4, VCP(VC_BLACK, VC_TRANSPARENT), "transparent");
		if (color==-1) {
			buf_printchar(0, (colors==8) ? 2 : 4, cursor, '>');
			buf_printchar(12, (colors==8) ? 2 : 4, cursor, '<');
		}
	}
}

void ht_color_block::getdata(ht_object_stream *s)
{
	s->putIntDec((color==-1) ? VC_TRANSPARENT : vcs[color], 4, NULL);
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
				if (color!=-1)
					if (color+4<colors) color+=4; else color=-1;
				dirtyview();
				clearmsg(msg);
				return;
		}
	}
	ht_view::handlemsg(msg);
}

void ht_color_block::setdata(ht_object_stream *s)
{
	int c=s->getIntDec(4, NULL);
	if (c==VC_TRANSPARENT) color=-1; else {
		for (int i=0; i<16; i++) if (vcs[i]==c) {
			color=i;
			break;
		}
	}
	dirtyview();
}

void center_bounds(bounds *b)
{
	bounds c;
	app->getbounds(&c);
	b->x=(c.w-b->w)/2;
	b->y=(c.h-b->h)/2;     
}

