/* 
 *	HT Editor
 *	hthex.cc
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

#include <string.h>

#include "cmds.h"
#include "htctrl.h"
#include "htiobox.h"
#include "hthex.h"
#include "htmenu.h"
#include "htsearch.h"
#include "snprintf.h"
#include "stream.h"
#include "tools.h"

extern "C" {
#include "evalx.h"
#include "regex.h"
}

ht_view *hthex_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_hex_viewer *v=new ht_hex_viewer();
	v->init(b, DESC_HEX, VC_EDIT | VC_GOTO | VC_SEARCH | VC_REPLACE | VC_RESIZE, file, group);

	v->search_caps|=SEARCHMODE_BIN | SEARCHMODE_EVALSTR | SEARCHMODE_EXPR;

/*	ht_group_sub *g=new ht_group_sub();
	g->init(file);

	ht_hex_sub *h=new ht_hex_sub();
	h->init(file, 0, 2, 0);
	ht_collapsable_sub *ch=new ht_collapsable_sub();
	ch->init(file, h, true, "eins", true);

	ht_hex_sub *i=new ht_hex_sub();
	i->init(file, 0, 2, 1);
	ht_collapsable_sub *ci=new ht_collapsable_sub();
	ci->init(file, i, true, "zwei", true);
	
	g->insert(ch);
	g->insert(ci);

	ht_collapsable_sub *cs=new ht_collapsable_sub();
	cs->init(file, g, true, "jo", true);

	v->insertsub(cs);*/
	
	ht_hex_file_sub *h=new ht_hex_file_sub();
	h->init(file, 0x0, file->get_size(), 0);
	v->insertsub(h);
	return v;
}

format_viewer_if hthex_if = {
	hthex_init,
	0
};

/*
 *	CLASS ht_hex_viewer
 */

void ht_hex_viewer::get_pindicator_str(char *buf)
{
	FILEOFS o;
	FILEOFS sel_start, sel_end;
	pselect_get(&sel_start, &sel_end);
	if (get_current_offset(&o)) {
		char ttemp[1024];
		if (sel_end-sel_start > 0) {
			ht_snprintf(ttemp, sizeof ttemp, "selection %xh-%xh (%d byte%s)", sel_start, sel_end-1, sel_end-sel_start, sel_end-sel_start==1?"":"s");
		} else {
			ttemp[0]=0;
		}
		// FIXME: sizeof buf
		ht_snprintf(buf, 1024, " %s %xh/%u %s", edit() ? "edit" : "view", o, o, ttemp);
	} else {
		strcpy(buf, "?");
	}
}
	
bool ht_hex_viewer::get_vscrollbar_pos(int *pstart, int *psize)
{
	int s=file->get_size();
	if (s) {
		int z=MIN(size.h*16, s-(int)top.line_id.id1);
		return scrollbar_pos(top.line_id.id1, z, s, pstart, psize);
	}
	return false;
}

void ht_hex_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_filesize_changed:
			htmsg m;
			m.msg=msg_filesize_changed;
			m.type=mt_broadcast;
			sendsubmsg(&m);
			
			uf_initialized=false;
			complete_init();
			
			dirtyview();
			return;
		case msg_get_scrollinfo:
			switch (msg->data1.integer) {
				case gsi_pindicator: {
					get_pindicator_str((char*)msg->data2.ptr);
					break;
				}
				case gsi_hscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_hscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					break;
				}
				case gsi_vscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_vscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					break;
				}
			}
			clearmsg(msg);
			return;
		case cmd_hex_entropy: {
			FILEOFS ofs;
			if (get_current_offset(&ofs)) {
				byte buf[64];
				if (pread(ofs, buf, 64)==64) {
					int e = calc_entropy2(buf, 64);
					infobox("64-byte entropy at offset %08x: %d %%", ofs, e);
				}
			}
			clearmsg(msg);
			return;
		}
		case msg_contextmenuquery: {
			ht_static_context_menu *m=new ht_static_context_menu();
			m->init("~Local-Hex");
			m->insert_entry("~Block operations", "Ctrl+B", cmd_file_blockop, K_Control_B, 1);
			m->insert_entry("~Entropy", "Ctrl+T", cmd_hex_entropy, K_Control_T, 1);

			msg->msg = msg_retval;
			msg->data1.ptr = m;
			return;
		}
	}
	ht_uformat_viewer::handlemsg(msg);
}

bool ht_hex_viewer::pos_to_offset(viewer_pos p, FILEOFS *ofs)
{
	*ofs = p.u.line_id.id1 + p.u.tag_idx;
	return true;
}

bool ht_hex_viewer::offset_to_pos(FILEOFS ofs, viewer_pos *p)
{
	clear_viewer_pos(p);
	p->u.sub = first_sub;
	p->u.line_id.id1 = ofs  & (~0xf);
	p->u.line_id.id2 = 0;
	p->u.tag_idx = ofs  & 0xf;
	return true;
}

bool ht_hex_viewer::string_to_pos(char *string, viewer_pos *pos)
{
	eval_scalar r;
	if (eval(&r, string, NULL, NULL, NULL)) {
		eval_int i;
		scalar_context_int(&r, &i);
		scalar_destroy(&r);
		offset_to_pos(QWORD_GET_INT(i.value), pos);
		return true;
	}
	char *s;
	int p;
	get_eval_error(&s, &p);
	sprintf(globalerror, "%s at pos %d", s, p);
	return false;
}

/*
 *	CLASS ht_hex_file_sub
 */

void ht_hex_file_sub::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_filesize_changed) {
		UINT s = file->get_size();
		fsize = s;
		return;
	}
	ht_hex_sub::handlemsg(msg);
}

