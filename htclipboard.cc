/*
 *	HT Editor
 *	htclipboard.cc
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
#include "htclipboard.h"
#include "htsearch.h"
#include "htstring.h"
#include "tools.h"

#include <stdlib.h>
#include <time.h>

ht_clipboard *clipboard;

class ht_clipboard_copy_history: public ht_data {
public:
	virtual ~ht_clipboard_copy_history() {
		if (source) free(source);
	}

	char *source;
	dword start;
	dword size;
	time_t time;
};

/*
 *	CLASS ht_clipboard
 */

void ht_clipboard::init()
{
	ht_mem_file::init(0, 16);
	copy_history=new ht_clist();
	((ht_clist*)copy_history)->init();
	select_start=0;
	select_len=0;
}

void ht_clipboard::done()
{
	copy_history->destroy();
	delete copy_history;
	ht_mem_file::done();
}

void ht_clipboard::clear()
{
	ht_mem_file::truncate(0);
	copy_history->destroy();
	delete copy_history;
	copy_history=new ht_clist();
	((ht_clist*)copy_history)->init();
	htmsg m;
	m.msg=msg_file_changed;
	m.data1.ptr=this;
	m.type=mt_broadcast;
	app->sendmsg(&m);
}

UINT	ht_clipboard::write(void *buf, UINT size)
{
	htmsg m;
	m.msg=msg_file_changed;
	m.data1.ptr=this;
	m.type=mt_broadcast;
	app->sendmsg(&m);
	return ht_mem_file::write(buf, size);
}

/*
 *	CLASS ht_clipboard_viewer
 */

void ht_clipboard_viewer::init(bounds *b, char *desc, int caps, ht_clipboard *clipb, ht_format_group *format_group)
{
	ht_uformat_viewer::init(b, desc, caps, clipboard, format_group);
	
	search_caps|=SEARCHMODE_BIN | SEARCHMODE_EVALSTR | SEARCHMODE_EXPR;

	lastentrycount=999999999;
	update_content();
}

void ht_clipboard_viewer::draw()
{
	update_content();
	ht_uformat_viewer::draw();
}

void ht_clipboard_viewer::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_file_changed) {
		if (msg->data1.ptr==clipboard) {
			dirtyview();
			clearmsg(msg);
			return;
		}
	}
	ht_uformat_viewer::handlemsg(msg);
}

void ht_clipboard_viewer::pselect_add(FILEOFS start, FILEOFS end)
{
	ht_uformat_viewer::pselect_add(start, end);
	selection_changed();
}

void ht_clipboard_viewer::pselect_set(FILEOFS start, FILEOFS end)
{
	ht_uformat_viewer::pselect_set(start, end);
	selection_changed();
}

void ht_clipboard_viewer::selection_changed()
{
	FILEOFS s, e;
	pselect_get(&s, &e);
	clipboard->select_start=s;
	clipboard->select_len=e-s;
}

void ht_clipboard_viewer::update_content()
{
	if (clipboard->copy_history->count()==lastentrycount) return;
	clear_subs();
	ht_clipboard *clipboard=(ht_clipboard*)file;
	int c=clipboard->copy_history->count();
	char title[512]; /* possible buffer overflow ! */

	for (int i=0; i<c; i++) {
		ht_clipboard_copy_history *j=(ht_clipboard_copy_history*)clipboard->copy_history->get(i);

		tm *t=localtime(&j->time);
		sprintf(title, "*** %02d:%02d:%02d, size %d(%xh), from %s", t->tm_hour, t->tm_min, t->tm_sec, j->size, j->size, j->source);

		ht_mask_sub *m=new ht_mask_sub();
		m->init(clipboard, i);
		m->add_mask(title);
		insertsub(m);

		ht_hex_sub *h=new ht_hex_sub();
		h->init(clipboard, j->start, j->size, 0);
		insertsub(h);
	}
	pselect_set(clipboard->select_start, clipboard->select_start+clipboard->select_len);
	lastentrycount=clipboard->copy_history->count();

	sendmsg(msg_complete_init, 0);
}

/* clipboard functions */

void clipboard_add_copy_history_entry(char *source, dword start, dword size, time_t time)
{
	ht_clipboard_copy_history *h=new ht_clipboard_copy_history();
	h->source=ht_strdup(source);
	h->start=start;
	h->size=size;
	h->time=time;
	clipboard->copy_history->insert(h);
}

#define CLIPBOARD_TRANSFER_BUF_SIZE	32*1024
//#define CLIPBOARD_TRANSFER_BUF_SIZE	2

int clipboard_copy(char *source_desc, void *buf, dword len)
{
	int r=0;
	if (len) {
		dword size=clipboard->get_size();
		clipboard->seek(size);
		r=clipboard->write(buf, len);
		clipboard->select_start=size;
		clipboard->select_len=r;
		clipboard_add_copy_history_entry(source_desc, size, r, time(0));
	}		
	return r;
}

int clipboard_copy(char *source_desc, ht_streamfile *file, dword offset, dword len)
{
	if (!len) return 0;

	dword size=clipboard->get_size();
	dword temp=file->tell();
	dword cpos=size, spos=offset;
	byte *buf=(byte*)malloc(CLIPBOARD_TRANSFER_BUF_SIZE);
	dword l=len, r=0;

	while ((len) && (l)) {
		l=len;
		if (l>CLIPBOARD_TRANSFER_BUF_SIZE) l=CLIPBOARD_TRANSFER_BUF_SIZE;
		file->seek(spos);
		l=file->read(buf, l);
		spos+=l;
		clipboard->seek(cpos);
		clipboard->write(buf, l);
		cpos+=l;
		len-=l;
		r+=l;
	}
	file->seek(temp);
	clipboard->select_start=size;
	clipboard->select_len=r;
	clipboard_add_copy_history_entry(source_desc, size, r, time(0));
	free(buf);
	
	return r;
}

int clipboard_paste(void *buf, dword maxlen)
{
	clipboard->seek(clipboard->select_start);
	return clipboard->read(buf, MIN(clipboard->select_len, maxlen));
}

int clipboard_paste(ht_streamfile *file, dword offset)
{
	dword len=clipboard->select_len;
	dword temp=file->tell();
	dword cpos=clipboard->select_start, spos=offset;
	byte *buf=(byte*)malloc(CLIPBOARD_TRANSFER_BUF_SIZE);
	dword l=len, r=0;
	while ((len) && (l)) {
		l=len;
		if (l>CLIPBOARD_TRANSFER_BUF_SIZE) l=CLIPBOARD_TRANSFER_BUF_SIZE;
		clipboard->seek(cpos);
		l=clipboard->read(buf, l);
		cpos+=l;
		file->seek(spos);
		file->write(buf, l);
		spos+=l;
		len-=l;
		r+=l;
	}
	file->seek(temp);
	free(buf);
	return r;
}

int clipboard_clear()
{
	clipboard->clear();
	return 1;
}

dword clipboard_getsize()
{
	return clipboard->select_len;
}

/*
 *	INIT
 */

bool init_clipboard()
{
	clipboard=new ht_clipboard();
	clipboard->init();
	return 1;
}

/*
 *	DONE
 */

void done_clipboard()
{
	clipboard->done();
	delete clipboard;
}

