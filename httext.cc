/* 
 *	HT Editor
 *	httext.cc
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

#include "htsearch.h"
#include "httext.h"
#include "stream.h"

#include <string.h>

ht_view *httext_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_text_viewer2 *v=new ht_text_viewer2();
	v->init(b, TEXT_DESC, 0/*VC_EDIT | VC_GOTO | VC_SEARCH | VC_BLOCKOP | VC_TRUNCATE*/, file, group);

	v->search_caps|=SEARCHMODE_BIN | SEARCHMODE_EVALSTR;

	ht_text_sub *t=new ht_text_sub();
	t->init(file, 0x0, file->get_size());
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

bool ht_text_viewer2::address_to_offset(fmt_vaddress addr, FILEOFS *ofs)
{
	*ofs=addr;
	return 1;
}

bool ht_text_viewer2::offset_to_address(FILEOFS ofs, fmt_vaddress *addr)
{
	*addr=ofs;
	return 1;
}

/*
 *	CLASS ht_text_sub
 */

/* FIXME: put it somewhere else..., why ain't this a POSIX function ? */
void *memrchr(const void *string, int ch, size_t num)
{
	while (num--) {
		if (((char*)string)[num]==ch) return ((char*)string)+num;
	}
	return NULL;
}
 

#define TEXT_SUB_READSIZE		256
#define TEXT_SUB_MAX_LINELEN		512

#define TEXT_SUB_MAX_LINEENDLEN	2

#define TEXT_SUB_TABSIZE			5

byte ht_text_sub_line[TEXT_SUB_MAX_LINELEN];

void ht_text_sub::init(ht_streamfile *file, FILEOFS offset, int size)
{
	ht_linear_sub::init(file, offset, size);
}

void ht_text_sub::done()
{
	ht_linear_sub::done();
}

UINT ht_text_sub::find_linelen_backwd(byte *buf, UINT maxbuflen, FILEOFS ofs, int *le_len)
{
	UINT readlen=(maxbuflen>TEXT_SUB_READSIZE) ? TEXT_SUB_READSIZE : maxbuflen;
	UINT oreadlen=readlen;
	FILEOFS oofs=ofs;
	byte *bufp;
	UINT s;
	UINT len=0;
	UINT lineends=0;

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

UINT ht_text_sub::find_linelen_forwd(byte *buf, UINT maxbuflen, FILEOFS ofs, int *le_len)
{
	UINT readlen=(maxbuflen>TEXT_SUB_READSIZE) ? TEXT_SUB_READSIZE : maxbuflen;
	byte *bufp;
	UINT s;
	UINT len=0;

	if (le_len) *le_len=0;
	do {
		file->seek(ofs);
		s=file->read(buf, readlen);
		int l;
		bufp=match_lineend_forwd(buf, s, &l);
		if (bufp) {
			len+=bufp-buf+l;
			if (len>TEXT_SUB_MAX_LINELEN) {
				len=TEXT_SUB_MAX_LINELEN;
				break;
			}
			if (le_len) *le_len=l;
			break;
		} else len+=s;
		if (len>TEXT_SUB_MAX_LINELEN) {
			len=TEXT_SUB_MAX_LINELEN;
			break;
		}
/* make sure current and next read overlap
   to guarantee proper lineend-matching */
		ofs+=s-(TEXT_SUB_MAX_LINEENDLEN-1);
	} while (s>TEXT_SUB_MAX_LINEENDLEN);
	return len;
}

void ht_text_sub::first_line_id(ID *id1, ID *id2)
{
	*id1=0;
	*id2=0;
}

bool ht_text_sub::getline(char *line, ID id1, ID id2)
{
	byte *bufp=(byte*)line;
	FILEOFS ofs=id1;
	int ll;
	UINT l=find_linelen_forwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, &ll);
	if (l) {
		l-=ll;
		if (l>255) l=255;
		file->seek(ofs);
		l=file->read(line, l);
		while (l--) {
			if ((*bufp=='\e') || (*bufp==0)) *bufp='.';
			bufp++;
		}
		*bufp=0;
		return true;
	}
	return false;
}

void ht_text_sub::last_line_id(ID *id1, ID *id2)
{
	FILEOFS ofs=fofs+fsize;
	UINT l=find_linelen_backwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, NULL);
	*id1=ofs-l;
	*id2=0;
}

byte *ht_text_sub::match_lineend_forwd(byte *buf, UINT buflen, int *le_len)
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

byte *ht_text_sub::match_lineend_backwd(byte *buf, UINT buflen, int *le_len)
{
	byte *result=NULL;
	
	byte *n=(byte*)memrchr(buf, '\n', buflen);
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

int ht_text_sub::next_line_id(ID *id1, ID *id2, int n)
{
	FILEOFS ofs=*id1;
	int r=0;
	while (n--) {
		UINT l=find_linelen_forwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, NULL);
		ofs+=l;
		if (!l) break;
		r++;
	}
	*id1=ofs;
	return r;
}

int ht_text_sub::prev_line_id(ID *id1, ID *id2, int n)
{
	FILEOFS ofs=*id1;
	int r=0;
	while (n--) {
		UINT l=find_linelen_backwd(ht_text_sub_line, sizeof ht_text_sub_line, ofs, NULL);
		ofs-=l;
		if (!l) break;
		r++;
	}
	*id1=ofs;
	return r;
}

