/*
 *	HT Editor
 *	htstructure.cc
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

#include "htapp.h"
#include "htkeyb.h"
#include "htstruct.h"

#include <string.h>

#if 0

ht_view *htstructure_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	bounds c;
	ht_group *g=new ht_group();
	g->init(b, 0, "structure-g");
/* status */
	c=*b;
	c.h=2;
	ht_structure_status *s=new ht_structure_status();
	s->init(&c, 0, 0);
	g->insert(s);
/* viewer */
	c=*b;
	c.y+=2;
	c.h-=2;
	ht_structure_viewer *v=new ht_structure_viewer();
	v->init(&c, DESC_STRUCTURE, VC_GOTO, file, s);
	g->insert(v);
	
	return g;
}

format_viewer_if htstructure_if = {
	htstructure_init,
	0
};

int compare_keys_loc(ht_data *key_a, ht_data *key_b)
{
	UINT a=((ht_format_loc*)key_a)->start;
	UINT b=((ht_format_loc*)key_b)->start;
	if (a>b) return 1; else if (a<b) return -1;
	return 0;
}
 
/*
 *	CLASS ht_format_loc_loc
 */
 
ht_format_loc_loc::ht_format_loc_loc()
{
	sublocs=new ht_sorted_list();
	sublocs->init(compare_keys_loc);
}

ht_format_loc_loc::~ht_format_loc_loc()
{
	sublocs->destroy();
	delete sublocs;
}

/*
 *	CLASS ht_structure_status
 */

void	ht_structure_status::init(bounds *b, char *text, int align, bool breaklines=1)
{
	ht_statictext::init(b, text, align, breaklines);
	name=0;
	cursor_ofs=0;
}

char *ht_structure_status::gettext()
{
	sprintf(status, "offset %08x belongs to %s", cursor_ofs, name);
	return status;
}

void ht_structure_status::setdata(void *data)
{
	ht_structure_status_data *d=(ht_structure_status_data*)data;
	name=d->name;
	cursor_ofs=d->cursor_ofs;
	dirtyview();
}

/*
 *	CLASS ht_structure_viewer
 */

#define OV_MAX_SIMULTANEOUS_FORMATS	4
#define OV_WIDTH					64

void ht_structure_viewer::init(bounds *b, char *desc, UINT caps, ht_streamfile *f, ht_structure_status *st)
{
	ht_viewer::init(b, desc, caps);

	file=f;
	status=st;
	loclocs=new ht_format_loc_loc();

	init_completed=0;
	granularity=64;
	cursor_ofs=0;
	scroll=0;
	cursor_loc=0;
	cursor_loc_unassigned=1;

	path=new ht_clist();
	path->init();
	
	update_cursor_loc();
	update_status();
}

void ht_structure_viewer::done()
{
	delete loclocs;

	path->done();
	delete path;
	
	ht_viewer::done();
}

void ht_structure_viewer::build_loc_string(char *buf, ht_list *clocs)
{
	int c=clocs->count();
	   if (c) {
		buf[0]=0;
		for (int i=0; i<c; i++) {
			ht_format_loc_loc *l=(ht_format_loc_loc*)clocs->get(i);
			strcat(buf, l->name);
			if (i<c-1) strcat(buf, ", ");
		}
	} else {
		strcpy(buf, "?");
	   }
}

UINT ht_structure_viewer::calc_y_extent()
{
	return (loclocs->length+granularity-1)/granularity;
}

UINT ht_structure_viewer::collect_locs(ht_format_viewer *v, ht_format_loc_loc *f, ht_format_loc *loc)
{
	if (v->loc_enum_init(loc)) {
		while (1) {
			ht_format_loc_loc *l=new ht_format_loc_loc();
			if (v->loc_enum_next(l)) {
				f->sublocs->insert(l);
			} else {
				delete l;
				break;
			}
		}
	}
	UINT c=f->sublocs->count();
	for (UINT i=0; i<c; i++) {
		ht_format_loc_loc *l=(ht_format_loc_loc*)f->sublocs->get(i);
		collect_locs(v, l, l);
	}		
	return 0;
}

void ht_structure_viewer::collect_views(ht_format_viewer *v)
{
	ht_format_viewer *w=(ht_format_viewer*)v->getfirstchild();
	while (w) {
		if ((w->options & VO_FORMAT_VIEW) && (strcmp(w->desc, DESC_STRUCTURE))) {
			collect_locs(w, loclocs, 0);
			collect_views(w);
		}
		w=(ht_format_viewer*)w->next;
	}
}

void ht_structure_viewer::complete_init()
{
	loclocs->name="all";
	loclocs->start=0;
	loclocs->length=file->get_size();

	collect_views((ht_format_viewer*)group->group);
	update_cursor_loc();
	update_status();
	app->sendmsg(msg_draw);
}

UINT ht_structure_viewer::count_starts(ht_list *s, FILEOFS offset)
{
	UINT i=0;
	UINT c=s->count();
	for (UINT k=0; k<c; k++) {
		ht_format_loc_loc *l=(ht_format_loc_loc*)s->get(k);
		if ((l) && ((l->start+granularity-1) / granularity==offset / granularity)) i++;
	}
	return i;
}

UINT ht_structure_viewer::count_ends(ht_list *s, FILEOFS offset)
{
	UINT i=0;
	UINT c=s->count();
	for (UINT k=0; k<c; k++) {
		ht_format_loc_loc *l=(ht_format_loc_loc*)s->get(k);
		if ((l) && ((l->start+l->length+granularity-1) / granularity==offset / granularity)) i++;
	}
	return i;
}

void ht_structure_viewer::cursor_up(UINT n)
{
	if (n*granularity*OV_WIDTH>cursor_ofs) cursor_ofs=0;
		else cursor_ofs-=n*granularity*OV_WIDTH;
	limit_cursor();
}

void ht_structure_viewer::cursor_down(UINT n)
{
	cursor_ofs+=n*granularity*OV_WIDTH;
	limit_cursor();
}

void ht_structure_viewer::cursor_left(UINT n)
{
	if (n*granularity>cursor_ofs) cursor_ofs=0;
		else cursor_ofs-=n*granularity;
	limit_cursor();
}

void ht_structure_viewer::cursor_right(UINT n)
{
	cursor_ofs+=n*granularity;
	limit_cursor();
}

void ht_structure_viewer::draw()
{
	if (!init_completed) {
		init_completed=1;
		complete_init();
	}
	clear(getcolor(palidx_windialog_body));

	ht_format_loc_loc *frame_loc=loclocs;
	
	ht_list *s=frame_loc->sublocs;

	UINT y=0;
	FILEOFS offset=frame_loc->start+scroll*granularity;
	FILEOFS cursor_start=0xffffffff, cursor_end=0;

	if (cursor_loc) {
		cursor_start=cursor_loc->start;
		cursor_end=cursor_loc->start+cursor_loc->length;
	}
	
	bool end=0;

	UINT temp_depth=0;
	
#define OV_MAX_COLORS 4
	vc cols[OV_MAX_COLORS];
	cols[0]=VC_GREEN;
	cols[1]=VC_CYAN;
	cols[2]=VC_BLUE;
	cols[3]=VC_WHITE;

	UINT colidx=0;
	   int nc=-1;
	   
	   ht_list *clocs=find_locs(loclocs->sublocs, offset);
	   char name[128];		/* FIXME: possible buffer overflow ! */
	build_loc_string(name, clocs);

	while (y<(UINT)size.h) {
		buf_printf(0, y, VCP(VC_WHITE, VC_BLACK), "%08x ", offset);
		for (UINT x=0; x<OV_WIDTH; x++) {
			UINT n_start=count_starts(s, offset);
			UINT n_end=count_ends(s, offset);
			if (offset > frame_loc->start+frame_loc->length) {
				end=1;
				break;
			}

			vcp c;
				    int ch=CHAR_FILLED_M;
				    
				    temp_depth+=n_start-n_end;
				    
			if (n_start+n_end) {
				   if (clocs) {
					clocs->done();
						 delete clocs;
				   }
						  
					colidx++;
				colidx%=OV_MAX_COLORS;
						  
				clocs=find_locs(loclocs->sublocs, offset);
						  build_loc_string(name, clocs);
						  nc=0;
			}
				    c=VCP(VC_BLACK, cols[colidx]);
				    
				    if (nc!=-1) {
					if (!name[nc]) nc=-1; else
							ch=name[nc++];
			}
			
			if (cursor_ofs==offset) setcursor(x+9, y);
				    
			buf_printchar(x+9, y, c, ch);
			offset+=granularity;
		}
		if (end) break;
		y++;
	}
	   if (clocs) {
		clocs->done();
			 delete clocs;
	   }
}

ht_list *ht_structure_viewer::find_locs(ht_list *s, FILEOFS offset)
{
	ht_clist *locs=new ht_clist();
	locs->init();
	UINT c=s->count();
	for (UINT i=0; i<c; i++) {
		ht_format_loc_loc *l=(ht_format_loc_loc*)s->get(i);
		if ((offset>=l->start) && (offset<l->start+l->length)) {
			locs->insert(l);
		}
	}
	return locs;
}

bool ht_structure_viewer::goto_offset(FILEOFS ofs, ht_view *source_object=0)
{
	if ((ofs>=loclocs->start) && (ofs<loclocs->start+loclocs->length)) {
		if ((ofs<scroll*granularity) || (ofs>=(scroll+size.h)*granularity)) {
			scroll=ofs/granularity;
		}
		cursor_ofs=ofs;
		if (source_object) push_vs_history(source_object);
		return 1;
	}
	return 0;
}

void ht_structure_viewer::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		switch (msg->data1.integer) {
			case K_Up:
				cursor_up(1);
				dirtyview();
				update_cursor_loc();
				update_status();
				clearmsg(msg);
				return;
			case K_Down:
				cursor_down(1);
				dirtyview();
				update_cursor_loc();
				update_status();
				clearmsg(msg);
				return;
			case K_Left:
				cursor_left(1);
				dirtyview();
				update_cursor_loc();
				update_status();
				clearmsg(msg);
				return;
			case K_Right:
				cursor_right(1);
				dirtyview();
				update_cursor_loc();
				update_status();
				clearmsg(msg);
				return;
			case K_Space:
				ref();
				dirtyview();
				clearmsg(msg);
				return;
		}
	}
	ht_viewer::handlemsg(msg);
}

void ht_structure_viewer::limit_cursor()
{
	if (cursor_ofs<loclocs->start) cursor_ofs=loclocs->start;
		else if (cursor_ofs>=loclocs->start+loclocs->length) cursor_ofs=loclocs->start+loclocs->length-1;
	cursor_ofs-=cursor_ofs%granularity;
}

void ht_structure_viewer::ref()
{
	if (cursor_loc) {
		loclocs=cursor_loc;
		scroll=0;
		if (!goto_offset(cursor_ofs)) goto_offset(loclocs->start);
	}
}

void ht_structure_viewer::scroll_up(UINT n)
{
	if (scroll) scroll--;
}

void ht_structure_viewer::scroll_down(UINT n)
{
	if (scroll+calc_y_extent()<(UINT)size.h) scroll++;
}

void ht_structure_viewer::set_granularity(UINT _granularity)
{
	granularity=_granularity;
}

char status_loc_ownership[128];         /* FIXME: possible buffer overflow ! */

void ht_structure_viewer::update_cursor_loc()
{
	ht_list *clocs=find_locs(loclocs->sublocs, cursor_ofs);
	cursor_loc=0;
	cursor_loc_unassigned=1;
	if (clocs) {
		UINT c=clocs->count();
		if (c==1) {
			cursor_loc=(ht_format_loc_loc*)clocs->get(0);
		} else if (c>1) {
			cursor_loc_unassigned=0;
			build_loc_string(status_loc_ownership, clocs);
		}
	}
	clocs->done();
	delete clocs;
}
	
void ht_structure_viewer::update_status()
{
	if (status) {
		ht_structure_status_data d;
		if (cursor_loc) d.name=cursor_loc->name; else
		if (cursor_loc_unassigned) d.name="?"; else
			d.name=status_loc_ownership;
		d.cursor_ofs=cursor_ofs;
		status->setdata(&d);
	}
}
#endif /* 0 */


