/*
 *	HT Editor
 *	htclipboard.h
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

#ifndef __HTCLIPBOARD_H__
#define __HTCLIPBOARD_H__

#include "io/types.h"
#include "stream.h"
#include "htformat.h"

/*
 *	CLASS ht_clipboard
 */

class ht_clipboard: public ht_mem_file {
public:
	ht_list *copy_history;
	uint32 select_start, select_len;

			void init();
	virtual	void done();
/* overwritten */
	virtual	uint	write(const void *buf, uint size);
/* new */
			void clear();
};

/*
 *	CLASS ht_clipboard_viewer
 */

class ht_clipboard_viewer: public ht_uformat_viewer {
protected:
	uint lastwritecount;
	uint lastentrycount;

			void get_pindicator_str(char *buf);
			void selection_changed();
public:
			void init(bounds *b, char *desc, int caps, ht_clipboard *clipboard, ht_format_group *format_group);
/* overwritten */
	virtual	void draw();
	virtual 	void handlemsg(htmsg *msg);
	virtual	void pselect_add(FileOfs start, FileOfs end);
	virtual	void pselect_set(FileOfs start, FileOfs end);
/* new */
			void update_content();
};

/* clipboard functions */

void clipboard_add_copy_history_entry(char *source, uint32 start, uint32 size, time_t time);
int clipboard_copy(char *source_desc, void *buf, uint32 len);
int clipboard_copy(char *source_desc, File *streamfile, uint32 offset, uint32 len);
int clipboard_paste(void *buf, uint32 maxlen);
int clipboard_paste(File *streamfile, uint32 offset);
int clipboard_clear();
uint32 clipboard_getsize();

/*
 *	INIT
 */

bool init_clipboard();

/*
 *	DONE
 */

void done_clipboard();

extern ht_clipboard *clipboard;

#endif /* __HTCLIPBOARD_H__ */
