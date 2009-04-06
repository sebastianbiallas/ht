/* 

 *	HT Editor
 *	httext.cc
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

#include "htsearch.h"
#include "httext.h"
#include "stream.h"

#include <string.h>

ht_view *httext_init(Bounds *b, File *file, ht_format_group *group)
{
	/* no httext for file > 5 MiB */
	if (file->getSize() > 5*1024*1024) {
		return NULL;
	}

	ht_text_viewer2 *v=new ht_text_viewer2();
	v->init(b, TEXT_DESC, 0/*VC_EDIT | VC_GOTO | VC_SEARCH | VC_BLOCKOP | VC_TRUNCATE*/, file, group);

	v->search_caps|=SEARCHMODE_BIN | SEARCHMODE_EVALSTR;

	ht_text_sub *t=new ht_text_sub();
	t->init(file, 0x0, file->getSize());
	v->insertsub(t);
	return v;
}

format_viewer_if httext_if = {
	httext_init,
	0
};

/*
 *	CLASS ht_text_viewer2
 */

/*bool ht_text_viewer2::offset_to_pos(FileOfs ofs, viewer_pos *pos)
{
	pos->u.sub = first_sub;
	pos->u.line_id.id1 = ofs;
	pos->u.line_id.id2 = 0;
	pos->u.tag_idx = 0;
	pos->u.tag_group = 0;
	return true;
}

bool ht_text_viewer2::pos_to_offset(viewer_pos pos, FileOfs *ofs)
{
	*ofs = pos.u.line_id.id1;
	return true;
}*/

void ht_text_viewer2::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_keypressed:
		switch (msg->data1.integer) {
		case K_Left: {
			// FIXME: send cmd_bla when available
			htmsg m;
			m.msg = msg_keypressed;
			m.type = mt_empty;
			m.data1.integer = K_Control_Left;
			sendmsg(&m);
			clearmsg(msg);
			return;
		}
		case K_Right: {
			// FIXME: send cmd_bla when available
			htmsg m;
			m.msg = msg_keypressed;
			m.type = mt_empty;
			m.data1.integer = K_Control_Right;
			sendmsg(&m);
			clearmsg(msg);
			return;
		}
		}
		break;
	}
	return ht_uformat_viewer::handlemsg(msg);
}

/*
 *	CLASS ht_text_sub
 */

/* FIXME: put it somewhere else..., why ain't this a POSIX function ? */
const void *ht_memrchr(const void *string, int ch, size_t num)
{
	while (num--) {
		if (((const char*)string)[num]==ch) return ((const char*)string)+num;
	}
	return NULL;
}
 

#define TEXT_SUB_READSIZE		256
#define TEXT_SUB_MAX_LINELEN		512

#define TEXT_SUB_MAX_LINEENDLEN	2

#define TEXT_SUB_TABSIZE			5

byte ht_text_sub_line[TEXT_SUB_MAX_LINELEN];

void ht_text_sub::init(File *file, FileOfs offset, int size)
{
	ht_linear_sub::init(file, offset, size);
}

void ht_text_sub::done()
{
	ht_linear_sub::done();
}

bool ht_text_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = offset;
	prev_line_id(line_id, 1);
	return true;
}

bool ht_text_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset)
{
	return false;
}

uint ht_text_sub::find_linelen_backwd(byte *buf, uint maxbuflen, FileOfs ofs, int *le_len)
{
	uint readlen=(maxbuflen>TEXT_SUB_READSIZE) ? TEXT_SUB_READSIZE : maxbuflen;
	uint oreadlen=readlen;
	FileOfs oofs=ofs;
	byte *bufp;
	uint s;
	uint len=0;
	uint lineends=0;

	if (le_len) *le_len=0;
	do {
		if (ofs==fofs) break;
		if (readlen>ofs) readlen=ofs;
		if (ofs-readlen<fofs) readlen=ofs-fofs;
		ofs-=readlen;
		file->seek(ofs);
/* make sure current and next read overlap
   to guarantee proper lineend-matching */
		if (readlen==oreadlen) ofs+=TEXT_SUB_MAX_LINEENDLEN-1; else
			if (ofs+readlen+TEXT_SUB_MAX_LINEENDLEN-1<=oofs)
				readlen+=TEXT_SUB_MAX_LINEENDLEN-1;
		s=file->read(buf, readlen);
		int l;
		bufp=match_lineend_backwd(buf, s, &l);
		if (bufp) {
			lineends++;
			if (lineends==1) {
				bufp=match_lineend_backwd(buf, bufp-buf, &l);
				if (bufp) lineends++;
			}
			if (lineends==2) {
				len+=buf+s-bufp-1;
				if (len>TEXT_SUB_MAX_LINELEN) {
					len=TEXT_SUB_MAX_LINELEN;
					break;
				}
				if (le_len) *le_len=l;
				break;
			}
		}
		len+=s;
		if (len>TEXT_SUB_MAX_LINELEN) {
			len=TEXT_SUB_MAX_LINELEN;
			break;
		}
	} while (s);
	return len;
}

uint ht_text_sub::find_linelen_forwd(byte *buf, uint maxbuflen, FileOfs ofs, int *le_len)
{
	uint readlen=(maxbuflen>TEXT_SUB_READSIZE) ? TEXT_SUB_READSIZE : maxbuflen;
	byte *bufp;
	uint s;
	uint len = 0;

	if (le_len) *le_len = 0;
	do {
		file->seek(ofs);
		s = file->read(buf, readlen);
		int l;
		bufp = match_lineend_forwd(buf, s, &l);
		if (bufp) {
			len += bufp-buf+l;
			if (le_len) *le_len = l;
			break;
		}
		if (s != readlen) {
			len += s;
			break;
		}
		/* make sure current and next read overlap
		   to guarantee proper lineend-matching */
		if (s > (TEXT_SUB_MAX_LINEENDLEN-1)) {
			len += s-(TEXT_SUB_MAX_LINEENDLEN-1);
		}
		ofs += s-(TEXT_SUB_MAX_LINEENDLEN-1);
	} while (s == readlen);
	if (len > TEXT_SUB_MAX_LINELEN) {
		len = TEXT_SUB_MAX_LINELEN;
		if (le_len) *le_len = 0;
	}
	return len;
}

void ht_text_sub::first_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = 0;
}

bool ht_text_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	byte *bufp = (byte*)line;
	FileOfs ofs = line_id.id1;
	int ll;
	uint l = find_linelen_forwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, &ll);
	if (l) {
		l -= ll;
		if (l > 255) l = 255;
		file->seek(ofs);
		l = file->read(line, l);
		while (l--) {
			if (*bufp=='\e' || *bufp==0) *bufp = '.';
			bufp++;
		}
		*bufp = 0;
		return true;
	}
	return false;
}

void ht_text_sub::last_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	FileOfs ofs = fofs+fsize;
	uint l = find_linelen_backwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, NULL);
	line_id->id1 = ofs-l;
}

byte *ht_text_sub::match_lineend_forwd(byte *buf, uint buflen, int *le_len)
{
	byte *result=NULL;
	
	byte *n=(byte*)memchr(buf, '\n', buflen);
	if (n) {
		if ((n>buf) && (n[-1] == '\r')) {
			*le_len=2;
			result=n-1;
		} else {
			*le_len=1;
			result=n;
		}
	}
	return result;
}

byte *ht_text_sub::match_lineend_backwd(byte *buf, uint buflen, int *le_len)
{
	byte *result=NULL;
	
	byte *n=(byte*)ht_memrchr(buf, '\n', buflen);
	if (n) {
		if ((n>buf) && (n[-1] == '\r')) {
			*le_len=2;
			result=n-1;
		} else {
			*le_len=1;
			result=n;
		}
	}
	return result;
}

int ht_text_sub::next_line_id(LINE_ID *line_id, int n)
{
	FileOfs ofs = line_id->id1;
	int r=0;
	while (n--) {
		uint l=find_linelen_forwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, NULL);
		ofs+=l;
		if (!l) break;
		r++;
	}
	line_id->id1 = ofs;
	return r;
}

int ht_text_sub::prev_line_id(LINE_ID *line_id, int n)
{
	FileOfs ofs = line_id->id1;
	int r=0;
	while (n--) {
		uint l=find_linelen_backwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, NULL);
		ofs-=l;
		if (!l) break;
		r++;
	}
	line_id->id1 = ofs;
	return r;
}

