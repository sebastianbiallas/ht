/* 
 *	HT Editor
 *	httext.h
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

#ifndef __HTTEXT_H__
#define __HTTEXT_H__

#include "htformat.h"
#include "formats.h"

#define TEXT_DESC "text"

extern format_viewer_if httext_if;

/*
 *	CLASS ht_text_viewer
 */

class ht_text_viewer2: public ht_uformat_viewer {
public:
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_text_sub
 */
 
class ht_text_sub: public ht_linear_sub {
protected:
/* new */
			uint find_linelen_backwd(byte *buf, uint maxbuflen, FileOfs ofs, int *le_len);
			uint find_linelen_forwd(byte *buf, uint maxbuflen, FileOfs ofs, int *le_len);
	virtual	byte *match_lineend_forwd(byte *buf, uint buflen, int *le_len);
	virtual	byte *match_lineend_backwd(byte *buf, uint buflen, int *le_len);
public:
			void init(File *file, FileOfs offset, int size);
	virtual	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
};

#endif /* !__HTTEXT_H__ */

