/*
 *	HT Editor
 *	htclipboard.cc
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

#include "htclipboard.h"
#include "htctrl.h"
#include "htsearch.h"
#include "strtools.h"
#include "snprintf.h"
#include "tools.h"
#include "io/file.h"

#include <stdlib.h>
#include <time.h>

ht_clipboard *clipboard;

class ht_clipboard_copy_history: public Object {
public:
	char *source;
	FileOfs start;
	FileOfs size;
	time_t time;

	ht_clipboard_copy_history(const char *aSource, FileOfs aStart, FileOfs aSize, time_t aTime)
	{
		source = ht_strdup(aSource);
		start = aStart;
		size = aSize;
		time = aTime;
	}
	
	virtual ~ht_clipboard_copy_history() {
		free(source);
	}
};

/*
 *	CLASS ht_clipboard
 */

ht_clipboard::ht_clipboard()
	: MemoryFile(0, 0, IOAM_READ | IOAM_WRITE)
{
	copy_history = new Array(true);
	select_start = 0;
	select_len = 0;
}

ht_clipboard::~ht_clipboard()
{
	delete copy_history;
}

void ht_clipboard::clear()
{
	truncate(0);
	copy_history->delAll();
	htmsg m;
	m.msg = msg_file_changed;
	m.data1.ptr = this;
	m.type = mt_broadcast;
	app->sendmsg(&m);
}

uint	ht_clipboard::write(const void *buf, uint size)
{
	htmsg m;
	m.msg = msg_file_changed;
	m.data1.ptr = this;
	m.type = mt_broadcast;
	app->sendmsg(&m);
	return MemoryFile::write(buf, size);
}

/*
 *	CLASS ht_clipboard_viewer
 */

void ht_clipboard_viewer::init(Bounds *b, const char *desc, int caps, ht_clipboard *clipb, ht_format_group *format_group)
{
	ht_uformat_viewer::init(b, desc, caps, clipboard, format_group);
	
	search_caps |= SEARCHMODE_BIN | SEARCHMODE_EVALSTR | SEARCHMODE_EXPR;

	lastentrycount = 999999999;
	update_content();
}

void ht_clipboard_viewer::draw()
{
	update_content();
	ht_uformat_viewer::draw();
}

void ht_clipboard_viewer::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_file_changed) {
		if (msg->data1.ptr == clipboard) {
			dirtyview();
			clearmsg(msg);
			return;
		}
	}
	ht_uformat_viewer::handlemsg(msg);
}

void ht_clipboard_viewer::pselect_add(FileOfs start, FileOfs end)
{
	ht_uformat_viewer::pselect_add(start, end);
	selection_changed();
}

void ht_clipboard_viewer::pselect_set(FileOfs start, FileOfs end)
{
	ht_uformat_viewer::pselect_set(start, end);
	selection_changed();
}

void ht_clipboard_viewer::selection_changed()
{
	FileOfs s, e;
	pselect_get(&s, &e);
	clipboard->select_start=s;
	clipboard->select_len=e-s;
}

void ht_clipboard_viewer::update_content()
{
	if (clipboard->copy_history->count() == lastentrycount) return;
	clear_subs();
	ht_clipboard *clipboard = (ht_clipboard*)file;
	int c = clipboard->copy_history->count();
	char title[512];	/* secure */
	
	uint uid = 0;
	for (int i = 0; i < c; i++) {
		ht_clipboard_copy_history *j = (ht_clipboard_copy_history*)(*clipboard->copy_history)[i];

		tm *t = localtime(&j->time);
		ht_snprintf(title, sizeof title, "*** %02d:%02d:%02d, size %qd(%qxh), from %s", t->tm_hour, t->tm_min, t->tm_sec, j->size, j->size, j->source);

		ht_mask_sub *m = new ht_mask_sub();
		m->init(clipboard, uid++);
		m->add_mask(title);
		insertsub(m);

		ht_hex_sub *h = new ht_hex_sub();
		h->init(clipboard, j->start, j->size, 16, uid++, -1);
		insertsub(h);
	}
	pselect_set(clipboard->select_start, clipboard->select_start+clipboard->select_len);
	lastentrycount = clipboard->copy_history->count();

	sendmsg(msg_complete_init, 0);
}

int ht_clipboard_viewer::get_pindicator_str(char *buf, int max_len)
{
	FileOfs o;
	if (get_current_offset(&o)) {
		FileOfs sel_start, sel_end;
		pselect_get(&sel_start, &sel_end);
		char ttemp[1024];
		if (sel_end-sel_start > 0) {
			ht_snprintf(ttemp, sizeof ttemp, "selection %qxh-%qxh (%qd byte%s) ", sel_start, sel_end-1, sel_end-sel_start, sel_end-sel_start==1?"":"s");
		} else {
			ttemp[0] = 0;
		}
		return ht_snprintf(buf, max_len, " %s %qxh/%qu %s", edit() ? "edit" : "view", o, o, ttemp);
	} else {
		return ht_snprintf(buf, max_len, "?");
	}
}

/* clipboard functions */

void clipboard_add_copy_history_entry(const char *source, FileOfs start, FileOfs size, time_t time)
{
	clipboard->copy_history->insert(new ht_clipboard_copy_history(source, start, size, time));
}

#define CLIPBOARD_TRANSFER_BUF_SIZE	32*1024
//#define CLIPBOARD_TRANSFER_BUF_SIZE	2

FileOfs clipboard_copy(const char *source_desc, void *buf, uint len)
{
	uint r = 0;
	if (len) {
		FileOfs size = clipboard->getSize();
		clipboard->seek(size);
		r = clipboard->write(buf, len);
		clipboard->select_start = size;
		clipboard->select_len = r;
		clipboard_add_copy_history_entry(source_desc, size, r, time(0));
	}		
	return r;
}

FileOfs clipboard_copy(const char *source_desc, File *file, FileOfs offset, FileOfs len)
{
	if (!len) return 0;

	FileOfs size = clipboard->getSize();
	FileOfs oldpos = file->tell();
	FileOfs cpos = size;
	FileOfs spos = offset;
	byte *buf = ht_malloc(CLIPBOARD_TRANSFER_BUF_SIZE);
	FileOfs l = len, r = 0;

	while (len && l) {
		l = len;
		if (l > CLIPBOARD_TRANSFER_BUF_SIZE) l = CLIPBOARD_TRANSFER_BUF_SIZE;
		file->seek(spos);
		l = file->read(buf, l);
		spos += l;
		clipboard->seek(cpos);
		clipboard->write(buf, l);
		cpos += l;
		len -= l;
		r += l;
	}
	file->seek(oldpos);
	clipboard->select_start = size;
	clipboard->select_len = r;
	clipboard_add_copy_history_entry(source_desc, size, r, time(0));
	free(buf);
	return r;
}

FileOfs clipboard_paste(void *buf, FileOfs maxlen)
{
	clipboard->seek(clipboard->select_start);
	return clipboard->read(buf, MIN(clipboard->select_len, maxlen));
}

FileOfs clipboard_paste(File *file, FileOfs offset)
{
	FileOfs len = clipboard->select_len;
	FileOfs oldpos = file->tell();
	FileOfs cpos = clipboard->select_start, spos=offset;
	byte *buf = ht_malloc(CLIPBOARD_TRANSFER_BUF_SIZE);
	FileOfs l = len, r = 0;
	while (len && l) {
		l = len;
		if (l > CLIPBOARD_TRANSFER_BUF_SIZE) l=CLIPBOARD_TRANSFER_BUF_SIZE;
		clipboard->seek(cpos);
		l = clipboard->read(buf, l);
		cpos += l;
		file->seek(spos);
		file->write(buf, l);
		spos += l;
		len -= l;
		r += l;
	}
	file->seek(oldpos);
	free(buf);
	return r;
}

bool clipboard_clear()
{
	clipboard->clear();
	return true;
}

FileOfs clipboard_getsize()
{
	return clipboard->select_len;
}

/*
 *	INIT
 */

bool init_clipboard()
{
	clipboard = new ht_clipboard();
	return true;
}

/*
 *	DONE
 */

void done_clipboard()
{
	delete clipboard;
}


