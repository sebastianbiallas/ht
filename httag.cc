/*
 *	HT Editor
 *	httag.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "global.h"
#include "htdebug.h"
#include "htstring.h"
#include "httag.h"
#include "tools.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/**/

TAGSTRING *tag_make_sel(TAGSTRING *buf, char *string)
{
	return tag_make_ref(buf, 0, 0, 0, 0, string);
}

TAGSTRING *tag_make_ref_len(TAGSTRING *buf, dword id128_1, dword id128_2, dword id128_3, dword id128_4, char *string, int strlen)
{
	ht_tag_sel *tag=(ht_tag_sel*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_SEL;
	UNALIGNED_MOVE(tag->id128_1, id128_1);
	UNALIGNED_MOVE(tag->id128_2, id128_2);
	UNALIGNED_MOVE(tag->id128_3, id128_3);
	UNALIGNED_MOVE(tag->id128_4, id128_4);
	UNALIGNED_MOVE(tag->strlen, strlen);
//	strcpy(buf+sizeof (ht_tag_sel), string);	/* FIXME: we assume that TAGSTRING is a char */
	memmove(buf+sizeof (ht_tag_sel), string, strlen);
//	buf[sizeof (ht_tag_sel)+strlen]=0;
	return buf+sizeof (ht_tag_sel)+strlen;
}

TAGSTRING *tag_make_ref(TAGSTRING *buf, dword id128_1, dword id128_2, dword id128_3, dword id128_4, char *string)
{
	return tag_make_ref_len(buf, id128_1, id128_2, id128_3, id128_4, string, strlen(string));
}

TAGSTRING *tag_make_flags(TAGSTRING *buf, dword ofs32, dword id)
{
	ht_tag_flags *tag=(ht_tag_flags*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_FLAGS;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id);
	return buf+sizeof (ht_tag_flags);
}

TAGSTRING *tag_make_group(TAGSTRING *buf)
{
	ht_tag_group *tag=(ht_tag_group*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_GROUP;
	return buf+sizeof (ht_tag_group);
}

TAGSTRING *tag_make_color(TAGSTRING *buf, dword color)
{
	ht_tag_color *tag=(ht_tag_color*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_COLOR;
	UNALIGNED_MOVE(tag->color, color);
	return buf+sizeof (ht_tag_color);
}

TAGSTRING *tag_make_default_color(TAGSTRING *buf)
{
	ht_tag_color *tag = (ht_tag_color*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_COLOR;
	UNALIGNED_MOVE_CONST(tag->color, 0xffffffff, dword);
	return buf+sizeof (ht_tag_color);
}

TAGSTRING *tag_make_edit_byte(TAGSTRING *buf, FILEOFS ofs32)
{
	ht_tag_edit_byte *tag=(ht_tag_edit_byte*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_EDIT_BYTE;
	UNALIGNED_MOVE(tag->offset, ofs32);
	return buf+sizeof (ht_tag_edit_byte);
}

TAGSTRING *tag_make_edit_word(TAGSTRING *buf, FILEOFS ofs32, tag_endian e)
{
	ht_tag_edit_word_generic *tag=(ht_tag_edit_word_generic*)buf;
	tag->escape='\e';
	byte m = 0xff;
	switch (e) {
		case tag_endian_big:
			m = HT_TAG_EDIT_WORD_BE;
			break;
		case tag_endian_little:
			m = HT_TAG_EDIT_WORD_LE;
			break;
		case tag_endian_var:
			m = HT_TAG_EDIT_WORD_VE;
			break;
	}
	tag->magic=m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	return buf+sizeof (ht_tag_edit_word_generic);
}

TAGSTRING *tag_make_edit_dword(TAGSTRING *buf, FILEOFS ofs32, tag_endian e)
{
	ht_tag_edit_dword_generic *tag=(ht_tag_edit_dword_generic*)buf;
	tag->escape='\e';
	byte m=0xff;
	switch (e) {
		case tag_endian_big:
			m = HT_TAG_EDIT_DWORD_BE;
			break;
		case tag_endian_little:
			m = HT_TAG_EDIT_DWORD_LE;
			break;
		case tag_endian_var:
			m = HT_TAG_EDIT_DWORD_VE;
			break;
	}
	tag->magic=m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	return buf+sizeof (ht_tag_edit_dword_generic);
}

TAGSTRING *tag_make_edit_qword(TAGSTRING *buf, FILEOFS ofs32, tag_endian e)
{
	ht_tag_edit_qword_generic *tag=(ht_tag_edit_qword_generic*)buf;
	tag->escape='\e';
	byte m=0xff;
	switch (e) {
		case tag_endian_big:
			m = HT_TAG_EDIT_QWORD_BE;
			break;
		case tag_endian_little:
			m = HT_TAG_EDIT_QWORD_LE;
			break;
		case tag_endian_var:
			m = HT_TAG_EDIT_QWORD_VE;
			break;
	}
	tag->magic=m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	return buf+sizeof (ht_tag_edit_qword_generic);
}

TAGSTRING *tag_make_edit_time(TAGSTRING *buf, FILEOFS ofs32)
{
	ht_tag_edit_time *tag=(ht_tag_edit_time*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_EDIT_TIME;
	UNALIGNED_MOVE(tag->offset, ofs32);
	return buf+sizeof (ht_tag_edit_time);
}

TAGSTRING *tag_make_edit_char(TAGSTRING *buf, FILEOFS ofs32)
{
	ht_tag_edit_char *tag=(ht_tag_edit_char*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_EDIT_CHAR;
	UNALIGNED_MOVE(tag->offset, ofs32);
	return buf+sizeof (ht_tag_edit_char);
}

TAGSTRING *tag_make_edit_bit(TAGSTRING *buf, FILEOFS ofs32, int bitidx)
{
	ht_tag_edit_bit *tag=(ht_tag_edit_bit*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_EDIT_BIT;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->bitidx, bitidx);
	return buf+sizeof (ht_tag_edit_bit);
}

TAGSTRING *tag_make_edit_selvis(TAGSTRING *buf, FILEOFS offset, char ch)
{
	ht_tag_edit_selvis *tag=(ht_tag_edit_selvis*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_EDIT_SELVIS;
	UNALIGNED_MOVE(tag->offset, offset);
	tag->ch=ch;
	return buf+sizeof (ht_tag_edit_selvis);
}

TAGSTRING *tag_make_desc_byte(TAGSTRING *buf, FILEOFS ofs32, dword id32)
{
	ht_tag_desc_byte *tag=(ht_tag_desc_byte*)buf;
	tag->escape='\e';
	tag->magic=HT_TAG_DESC_BYTE;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf+sizeof (ht_tag_desc_byte);
}

TAGSTRING *tag_make_desc_word(TAGSTRING *buf, FILEOFS ofs32, dword id32, tag_endian e)
{
	ht_tag_desc_word_generic *tag=(ht_tag_desc_word_generic*)buf;
	tag->escape='\e';
	byte m=0xff;
	switch (e) {
		case tag_endian_big:
			m = HT_TAG_DESC_WORD_BE;
			break;
		case tag_endian_little:
			m = HT_TAG_DESC_WORD_LE;
			break;
		case tag_endian_var:
			m = HT_TAG_DESC_WORD_VE;
			break;
	}
	tag->magic=m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf+sizeof (ht_tag_desc_word_generic);
}

TAGSTRING *tag_make_desc_dword(TAGSTRING *buf, FILEOFS ofs32, dword id32, tag_endian e)
{
	ht_tag_desc_dword_generic *tag=(ht_tag_desc_dword_generic*)buf;
	tag->escape='\e';
	byte m=0xff;
	switch (e) {
		case tag_endian_big:
			m = HT_TAG_DESC_DWORD_BE;
			break;
		case tag_endian_little:
			m = HT_TAG_DESC_DWORD_LE;
			break;
		case tag_endian_var:
			m = HT_TAG_DESC_DWORD_VE;
			break;
	}
	tag->magic=m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf+sizeof (ht_tag_desc_dword_generic);
}

TAGSTRING *tag_make_desc_qword(TAGSTRING *buf, FILEOFS ofs32, dword id32, tag_endian e)
{
	ht_tag_desc_qword_generic *tag=(ht_tag_desc_qword_generic*)buf;
	tag->escape='\e';
	byte m=0xff;
	switch (e) {
		case tag_endian_big:
			m = HT_TAG_DESC_QWORD_BE;
			break;
		case tag_endian_little:
			m = HT_TAG_DESC_QWORD_LE;
			break;
		case tag_endian_var:
			m = HT_TAG_DESC_QWORD_VE;
			break;
	}
	tag->magic=m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf+sizeof (ht_tag_desc_qword_generic);
}

/**/

void statictag_to_tag(char *statictag_str, TAGSTRING *tag_str, dword relocation, bool std_bigendian)
{
	FILEOFS ofs=0;
	ID id;
	while (*statictag_str) {
		if (*statictag_str=='\e') {
			switch ((byte)*(statictag_str+1)) {
				case HT_STATICTAG_EDIT_BYTE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_byte(tag_str, ofs+relocation);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_WORD_LE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_word(tag_str, ofs+relocation, tag_endian_little);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_DWORD_LE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_dword(tag_str, ofs+relocation, tag_endian_little);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_QWORD_LE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_qword(tag_str, ofs+relocation, tag_endian_little);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_WORD_BE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_word(tag_str, ofs+relocation, tag_endian_big);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_DWORD_BE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_dword(tag_str, ofs+relocation, tag_endian_big);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_QWORD_BE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_qword(tag_str, ofs+relocation, tag_endian_big);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_WORD_VE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_word(tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_DWORD_VE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_dword(tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_QWORD_VE:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_qword(tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_TIME:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_time(tag_str, ofs+relocation);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_CHAR:
					ofs=hexd(statictag_str+2);
					tag_str=tag_make_edit_char(tag_str, ofs+relocation);
					statictag_str+=2+8;
					break;
				case HT_STATICTAG_EDIT_BIT: {
					ofs=hexd(statictag_str+2);
					int bitidx=hexb(statictag_str+2+8);
					tag_str=tag_make_edit_bit(tag_str, ofs+relocation, bitidx);
					statictag_str+=2+8+2;
					break;
				}
				case HT_STATICTAG_EDIT_SELVIS: {
					ofs=hexd(statictag_str+2);
					char ch=hexb(statictag_str+2+8);
					tag_str=tag_make_edit_selvis(tag_str, ofs+relocation, ch);
					statictag_str+=2+8+2;
					break;
				}
				case HT_STATICTAG_DESC_BYTE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_byte(tag_str, ofs+relocation, id);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_WORD_LE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_word(tag_str, ofs+relocation, id, tag_endian_little);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_DWORD_LE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_dword(tag_str, ofs+relocation, id, tag_endian_little);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_QWORD_LE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_qword(tag_str, ofs+relocation, id, tag_endian_little);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_WORD_BE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_word(tag_str, ofs+relocation, id, tag_endian_big);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_DWORD_BE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_dword(tag_str, ofs+relocation, id, tag_endian_big);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_QWORD_BE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_qword(tag_str, ofs+relocation, id, tag_endian_big);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_WORD_VE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_word(tag_str, ofs+relocation, id, std_bigendian ? tag_endian_big : tag_endian_little);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_DWORD_VE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_dword(tag_str, ofs+relocation, id, std_bigendian ? tag_endian_big : tag_endian_little);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_DESC_QWORD_VE:
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_desc_qword(tag_str, ofs+relocation, id, std_bigendian ? tag_endian_big : tag_endian_little);
					statictag_str+=2+8+8;
					break;
				case HT_STATICTAG_SEL: {
					dword id_1=hexd(statictag_str+2);
					dword id_2=hexd(statictag_str+2+8);
					dword id_3=hexd(statictag_str+2+16);
					dword id_4=hexd(statictag_str+2+24);
					byte len=hexb(statictag_str+2+8+8+8+8);
					tag_str=tag_make_ref_len(tag_str, id_1, id_2, id_3, id_4, statictag_str+2+8+8+8+8+2, len);
					statictag_str+=2+8+8+8+8+2+len;
					break;
				}
				case HT_STATICTAG_FLAGS: {
					ofs=hexd(statictag_str+2);
					id=hexd(statictag_str+2+8);
					tag_str=tag_make_flags(tag_str, id, ofs+relocation);
					statictag_str+=2+8+8;
					break;
				}
				case HT_STATICTAG_GROUP:
					tag_str=tag_make_group(tag_str);
					statictag_str+=2;
					break;
				case HT_STATICTAG_COLOR: {
					byte color=hexb(statictag_str+2);
					tag_str=tag_make_color(tag_str, color);
					statictag_str+=2+2;
					break;
				}
				default:
					HT_ERROR("error in statictag string !");
			}
		} else {
			*(tag_str++)=*(statictag_str++);
		}
	}
	*tag_str=0;
}

TAGSTRING *tag_findnext(TAGSTRING *tagstring)
{
/* do not enable. works for x86 only ! */
#if 0
	int *s=(int*)tagstring;
	int e;
	while (*s) {
		e=*(s++);
		for (int i=0; i<4; i++) {
			if ((e&0xff)=='\e') return (TAGSTRING*)tagstring;
			if ((e&0xff)==0) return 0;
			e=e>>8;
			tagstring++;
		}
	}
	return 0;
#else
	return strchr(tagstring, '\e');
#endif
}

int tag_get_len(TAGSTRING *tagstring)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
			return HT_TAG_EDIT_BYTE_LEN;
		case HT_TAG_EDIT_WORD_LE:
			return HT_TAG_EDIT_WORD_LE_LEN;
		case HT_TAG_EDIT_DWORD_LE:
			return HT_TAG_EDIT_DWORD_LE_LEN;
		case HT_TAG_EDIT_QWORD_LE:
			return HT_TAG_EDIT_QWORD_LE_LEN;
		case HT_TAG_EDIT_WORD_BE:
			return HT_TAG_EDIT_WORD_BE_LEN;
		case HT_TAG_EDIT_DWORD_BE:
			return HT_TAG_EDIT_DWORD_BE_LEN;
		case HT_TAG_EDIT_QWORD_BE:
			return HT_TAG_EDIT_QWORD_BE_LEN;
		case HT_TAG_EDIT_WORD_VE:
			return HT_TAG_EDIT_WORD_VE_LEN;
		case HT_TAG_EDIT_DWORD_VE:
			return HT_TAG_EDIT_DWORD_VE_LEN;
		case HT_TAG_EDIT_QWORD_VE:
			return HT_TAG_EDIT_QWORD_VE_LEN;
		case HT_TAG_EDIT_TIME:
			return HT_TAG_EDIT_TIME_LEN;
		case HT_TAG_EDIT_CHAR:
			return HT_TAG_EDIT_CHAR_LEN;
		case HT_TAG_EDIT_BIT:
			return HT_TAG_EDIT_BIT_LEN;
		case HT_TAG_EDIT_SELVIS:
			return HT_TAG_EDIT_SELVIS_LEN;
		case HT_TAG_SEL:
			return HT_TAG_SEL_LEN(((ht_tag_sel*)tagstring)->strlen);
		case HT_TAG_FLAGS:
			return HT_TAG_FLAGS_LEN;
		case HT_TAG_COLOR:
			return HT_TAG_COLOR_LEN;
		case HT_TAG_GROUP:
			return HT_TAG_GROUP_LEN;
		case HT_TAG_DESC_BYTE:
			return HT_TAG_DESC_BYTE_LEN;
		case HT_TAG_DESC_WORD_LE:
			return HT_TAG_DESC_WORD_LE_LEN;
		case HT_TAG_DESC_DWORD_LE:
			return HT_TAG_DESC_DWORD_LE_LEN;
		case HT_TAG_DESC_QWORD_LE:
			return HT_TAG_DESC_QWORD_LE_LEN;
		case HT_TAG_DESC_WORD_BE:
			return HT_TAG_DESC_WORD_BE_LEN;
		case HT_TAG_DESC_DWORD_BE:
			return HT_TAG_DESC_DWORD_BE_LEN;
		case HT_TAG_DESC_QWORD_BE:
			return HT_TAG_DESC_QWORD_BE_LEN;
		case HT_TAG_DESC_WORD_VE:
			return HT_TAG_DESC_WORD_VE_LEN;
		case HT_TAG_DESC_DWORD_VE:
			return HT_TAG_DESC_DWORD_VE_LEN;
		case HT_TAG_DESC_QWORD_VE:
			return HT_TAG_DESC_QWORD_VE_LEN;
	}
	assert(0);
	return -1;
}

int tag_get_vlen(TAGSTRING *tagstring)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
			return HT_TAG_EDIT_BYTE_VLEN;
		case HT_TAG_EDIT_WORD_LE:
			return HT_TAG_EDIT_WORD_LE_VLEN;
		case HT_TAG_EDIT_DWORD_LE:
			return HT_TAG_EDIT_DWORD_LE_VLEN;
		case HT_TAG_EDIT_QWORD_LE:
			return HT_TAG_EDIT_QWORD_LE_VLEN;
		case HT_TAG_EDIT_WORD_BE:
			return HT_TAG_EDIT_WORD_BE_VLEN;
		case HT_TAG_EDIT_DWORD_BE:
			return HT_TAG_EDIT_DWORD_BE_VLEN;
		case HT_TAG_EDIT_QWORD_BE:
			return HT_TAG_EDIT_QWORD_BE_VLEN;
		case HT_TAG_EDIT_WORD_VE:
			return HT_TAG_EDIT_WORD_VE_VLEN;
		case HT_TAG_EDIT_DWORD_VE:
			return HT_TAG_EDIT_DWORD_VE_VLEN;
		case HT_TAG_EDIT_QWORD_VE:
			return HT_TAG_EDIT_QWORD_VE_VLEN;
		case HT_TAG_EDIT_TIME:
			return HT_TAG_EDIT_TIME_VLEN;
		case HT_TAG_EDIT_CHAR:
			return HT_TAG_EDIT_CHAR_VLEN;
		case HT_TAG_EDIT_BIT:
			return HT_TAG_EDIT_BIT_VLEN;
		case HT_TAG_EDIT_SELVIS:
			return HT_TAG_EDIT_SELVIS_VLEN;
		case HT_TAG_SEL: {
			return HT_TAG_SEL_VLEN(((ht_tag_sel*)tagstring)->strlen);
		}
		default:
			return 0;
	}
}

int time_mp[14]={0,1,3,4,6,7,9,10,12,13,15,16,17,18};

int tag_get_micropos(TAGSTRING *tagstring, int i)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
		case HT_TAG_EDIT_WORD_LE:
		case HT_TAG_EDIT_DWORD_LE:
		case HT_TAG_EDIT_QWORD_LE:
		case HT_TAG_EDIT_WORD_BE:
		case HT_TAG_EDIT_DWORD_BE:
		case HT_TAG_EDIT_QWORD_BE:
		case HT_TAG_EDIT_WORD_VE:
		case HT_TAG_EDIT_DWORD_VE:
		case HT_TAG_EDIT_QWORD_VE:
		case HT_TAG_EDIT_CHAR:
		case HT_TAG_EDIT_BIT:
			return i;
		case HT_TAG_EDIT_TIME:
			return time_mp[i];
	}
//	assert(0);
	return -1;
}

int tag_get_microsize(TAGSTRING *tagstring)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
			return HT_TAG_EDIT_BYTE_VLEN;
		case HT_TAG_EDIT_WORD_LE:
			return HT_TAG_EDIT_WORD_LE_VLEN;
		case HT_TAG_EDIT_DWORD_LE:
			return HT_TAG_EDIT_DWORD_LE_VLEN;
		case HT_TAG_EDIT_QWORD_LE:
			return HT_TAG_EDIT_QWORD_LE_VLEN;
		case HT_TAG_EDIT_WORD_BE:
			return HT_TAG_EDIT_WORD_BE_VLEN;
		case HT_TAG_EDIT_DWORD_BE:
			return HT_TAG_EDIT_DWORD_BE_VLEN;
		case HT_TAG_EDIT_QWORD_BE:
			return HT_TAG_EDIT_QWORD_BE_VLEN;
		case HT_TAG_EDIT_WORD_VE:
			return HT_TAG_EDIT_WORD_VE_VLEN;
		case HT_TAG_EDIT_DWORD_VE:
			return HT_TAG_EDIT_DWORD_VE_VLEN;
		case HT_TAG_EDIT_QWORD_VE:
			return HT_TAG_EDIT_QWORD_VE_VLEN;
		case HT_TAG_EDIT_TIME:
			return 14;
		case HT_TAG_EDIT_CHAR:
			return HT_TAG_EDIT_CHAR_VLEN;
		case HT_TAG_EDIT_BIT:
			return HT_TAG_EDIT_BIT_VLEN;
		default:
			return 1;
	}
}

int tag_get_size(TAGSTRING *tagstring)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
			return HT_TAG_EDIT_BYTE_SIZE;
		case HT_TAG_EDIT_WORD_LE:
			return HT_TAG_EDIT_WORD_LE_SIZE;
		case HT_TAG_EDIT_DWORD_LE:
			return HT_TAG_EDIT_DWORD_LE_SIZE;
		case HT_TAG_EDIT_QWORD_LE:
			return HT_TAG_EDIT_QWORD_LE_SIZE;
		case HT_TAG_EDIT_WORD_BE:
			return HT_TAG_EDIT_WORD_BE_SIZE;
		case HT_TAG_EDIT_DWORD_BE:
			return HT_TAG_EDIT_DWORD_BE_SIZE;
		case HT_TAG_EDIT_QWORD_BE:
			return HT_TAG_EDIT_QWORD_BE_SIZE;
		case HT_TAG_EDIT_WORD_VE:
			return HT_TAG_EDIT_WORD_VE_SIZE;
		case HT_TAG_EDIT_DWORD_VE:
			return HT_TAG_EDIT_DWORD_VE_SIZE;
		case HT_TAG_EDIT_QWORD_VE:
			return HT_TAG_EDIT_QWORD_VE_SIZE;
		case HT_TAG_EDIT_TIME:
			return HT_TAG_EDIT_TIME_SIZE;
		case HT_TAG_EDIT_CHAR:
			return HT_TAG_EDIT_CHAR_SIZE;
		case HT_TAG_EDIT_BIT:
			return HT_TAG_EDIT_BIT_SIZE;
		default:
			return 0;
	}
}

dword tag_get_offset(TAGSTRING *tagstring)
{
	FILEOFS f;
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_byte*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_WORD_LE:
		case HT_TAG_EDIT_WORD_BE:
		case HT_TAG_EDIT_WORD_VE: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_word_generic*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_DWORD_LE:
		case HT_TAG_EDIT_DWORD_BE:
		case HT_TAG_EDIT_DWORD_VE: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_dword_generic*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_QWORD_LE:
		case HT_TAG_EDIT_QWORD_BE:
		case HT_TAG_EDIT_QWORD_VE: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_qword_generic*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_TIME: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_time*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_CHAR: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_char*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_BIT: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_bit*)tagstring)->offset);
			return f;
		}
		case HT_TAG_EDIT_SELVIS: {
			UNALIGNED_MOVE(f, ((ht_tag_edit_selvis*)tagstring)->offset);
			return f;
		}
		case HT_TAG_DESC_BYTE: {
			UNALIGNED_MOVE(f, ((ht_tag_desc_byte*)tagstring)->offset);
			return f;
		}
		case HT_TAG_DESC_WORD_LE:
		case HT_TAG_DESC_WORD_BE:
		case HT_TAG_DESC_WORD_VE: {
			UNALIGNED_MOVE(f, ((ht_tag_desc_word_generic*)tagstring)->offset);
			return f;
		}
		case HT_TAG_DESC_DWORD_LE:
		case HT_TAG_DESC_DWORD_BE:
		case HT_TAG_DESC_DWORD_VE: {
			UNALIGNED_MOVE(f, ((ht_tag_desc_dword_generic*)tagstring)->offset);
			return f;
		}
		case HT_TAG_DESC_QWORD_LE:
		case HT_TAG_DESC_QWORD_BE:
		case HT_TAG_DESC_QWORD_VE: {
			UNALIGNED_MOVE(f, ((ht_tag_desc_qword_generic*)tagstring)->offset);
			return f;
		}
		case HT_TAG_FLAGS: {
			UNALIGNED_MOVE(f, ((ht_tag_flags*)tagstring)->offset);
			return f;
		}
	}
	assert(0);
	return 0;
}

void tag_get_id(TAGSTRING *tagstring, dword *id128_1, dword *id128_2, dword *id128_3, dword *id128_4)
{
	if (tagstring[1]==HT_TAG_SEL) {
		UNALIGNED_MOVE(*id128_1, ((ht_tag_sel*)tagstring)->id128_1);
		UNALIGNED_MOVE(*id128_2, ((ht_tag_sel*)tagstring)->id128_2);
		UNALIGNED_MOVE(*id128_3, ((ht_tag_sel*)tagstring)->id128_3);
		UNALIGNED_MOVE(*id128_4, ((ht_tag_sel*)tagstring)->id128_4);
	}
}

char* tag_get_seltext(TAGSTRING *tagstring)
{
	return tagstring+sizeof(ht_tag_sel);
}

int tag_get_seltextlen(TAGSTRING *tagstring)
{
	if (tagstring[1]==HT_TAG_SEL) {
		return ((ht_tag_sel*)tagstring)->strlen;
	}
	return -1;
}

vcp tag_get_color(TAGSTRING *tagstring)
{
	vcp c;
	UNALIGNED_MOVE(c, ((ht_tag_color*)tagstring)->color);
	return c;
}

bool tag_get_desc_id(TAGSTRING *tagstring, dword *id)
{
	switch (tagstring[1]) {
		case HT_TAG_DESC_BYTE:
			UNALIGNED_MOVE(*id, ((ht_tag_desc_byte*)tagstring)->id);
			return true;
		case HT_TAG_DESC_WORD_LE:
		case HT_TAG_DESC_WORD_BE:
		case HT_TAG_DESC_WORD_VE:
			UNALIGNED_MOVE(*id, ((ht_tag_desc_word_generic*)tagstring)->id);
			return true;
		case HT_TAG_DESC_DWORD_LE:
		case HT_TAG_DESC_DWORD_BE:
		case HT_TAG_DESC_DWORD_VE:
			UNALIGNED_MOVE(*id, ((ht_tag_desc_dword_generic*)tagstring)->id);
			return true;
		case HT_TAG_DESC_QWORD_LE:
		case HT_TAG_DESC_QWORD_BE:
		case HT_TAG_DESC_QWORD_VE:
			UNALIGNED_MOVE(*id, ((ht_tag_desc_qword_generic*)tagstring)->id);
			return true;
	}
	return false;
}

void tag_set_offset(TAGSTRING *tagstring, dword offset)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
			UNALIGNED_MOVE(((ht_tag_edit_byte*)tagstring)->offset, offset);
			break;
		case HT_TAG_EDIT_WORD_LE:
		case HT_TAG_EDIT_WORD_BE:
		case HT_TAG_EDIT_WORD_VE:
			UNALIGNED_MOVE(((ht_tag_edit_word_generic*)tagstring)->offset, offset);
			break;
		case HT_TAG_EDIT_DWORD_LE:
		case HT_TAG_EDIT_DWORD_BE:
		case HT_TAG_EDIT_DWORD_VE:
			UNALIGNED_MOVE(((ht_tag_edit_dword_generic*)tagstring)->offset, offset);
			break;
		case HT_TAG_EDIT_QWORD_LE:
		case HT_TAG_EDIT_QWORD_BE:
		case HT_TAG_EDIT_QWORD_VE:
			UNALIGNED_MOVE(((ht_tag_edit_qword_generic*)tagstring)->offset, offset);
			break;
		case HT_TAG_EDIT_TIME:
			UNALIGNED_MOVE(((ht_tag_edit_time*)tagstring)->offset, offset);
			break;
		case HT_TAG_EDIT_CHAR:
			UNALIGNED_MOVE(((ht_tag_edit_char*)tagstring)->offset, offset);
			break;
		case HT_TAG_EDIT_BIT:
			UNALIGNED_MOVE(((ht_tag_edit_bit*)tagstring)->offset, offset);
			break;
	}
}

int tag_is_editable(TAGSTRING *tagstring)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
		case HT_TAG_EDIT_WORD_LE:
		case HT_TAG_EDIT_DWORD_LE:
		case HT_TAG_EDIT_QWORD_LE:
		case HT_TAG_EDIT_WORD_BE:
		case HT_TAG_EDIT_DWORD_BE:
		case HT_TAG_EDIT_QWORD_BE:
		case HT_TAG_EDIT_WORD_VE:
		case HT_TAG_EDIT_DWORD_VE:
		case HT_TAG_EDIT_QWORD_VE:
		case HT_TAG_EDIT_TIME:
		case HT_TAG_EDIT_CHAR:
		case HT_TAG_EDIT_BIT:
			return 1;
		default:
			return 0;
	}
	assert(0);
	return -1;
}

void tag_strcat(TAGSTRING *dest, TAGSTRING *src)
{
	int l=tag_strlen(dest);
	tag_strcpy(dest+l, src);
}

void tag_strcpy(TAGSTRING *dest, TAGSTRING *src)
{
	int l=tag_strlen(src);
	memmove(dest, src, l);
	dest[l]=0;
}

TAGSTRING *tag_strdup(TAGSTRING *tagstring)
{
	int l=tag_strlen(tagstring);
	TAGSTRING *s=(TAGSTRING*)malloc((sizeof(TAGSTRING)+1)*l);
	memmove(s, tagstring, sizeof(TAGSTRING)*l);
	s[l]=0;
	return s;
}

int tag_strlen(TAGSTRING *tagstring)
{
	int c=0, r;
	while (*tagstring) {
		if (tagstring[0]=='\e') {
			r=tag_get_len(tagstring);
		} else {
			r=1;
		}
		tagstring+=r;
		c+=r;
	}
	return c;
}

int tag_strvlen(TAGSTRING *tagstring)
{
	int c=0, r, v;
	while (*tagstring) {
		if (tagstring[0]=='\e') {
			r=tag_get_len(tagstring);
			v=tag_get_vlen(tagstring);
		} else {
			r=1;
			v=1;
		}
		tagstring+=r;
		c+=v;
	}
	return c;
}

int tag_count_selectable_tags_in_group(TAGSTRING *tagstring, int group)
{
	int c=0;
	tagstring=tag_get_group(tagstring, group);
	while ((tagstring=tag_findnext(tagstring))) {
		switch (tagstring[1]) {
			case HT_TAG_EDIT_BYTE:
				c++;
				tagstring+=HT_TAG_EDIT_BYTE_LEN;
				break;
			case HT_TAG_EDIT_WORD_LE:
				c++;
				tagstring+=HT_TAG_EDIT_WORD_LE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_LE:
				c++;
				tagstring+=HT_TAG_EDIT_DWORD_LE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_LE:
				c++;
				tagstring+=HT_TAG_EDIT_QWORD_LE_LEN;
				break;
			case HT_TAG_EDIT_WORD_BE:
				c++;
				tagstring+=HT_TAG_EDIT_WORD_BE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_BE:
				c++;
				tagstring+=HT_TAG_EDIT_DWORD_BE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_BE:
				c++;
				tagstring+=HT_TAG_EDIT_QWORD_BE_LEN;
				break;
			case HT_TAG_EDIT_WORD_VE:
				c++;
				tagstring+=HT_TAG_EDIT_WORD_VE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_VE:
				c++;
				tagstring+=HT_TAG_EDIT_DWORD_VE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_VE:
				c++;
				tagstring+=HT_TAG_EDIT_QWORD_VE_LEN;
				break;
			case HT_TAG_EDIT_TIME:
				c++;
				tagstring+=HT_TAG_EDIT_TIME_LEN;
				break;
			case HT_TAG_EDIT_CHAR:
				c++;
				tagstring+=HT_TAG_EDIT_CHAR_LEN;
				break;
			case HT_TAG_EDIT_BIT:
				c++;
				tagstring+=HT_TAG_EDIT_BIT_LEN;
				break;
			case HT_TAG_SEL:
				c++;
				tagstring+=HT_TAG_SEL_LEN(((ht_tag_sel*)tagstring)->strlen);
				break;
			case HT_TAG_FLAGS:
				c++;
				tagstring+=HT_TAG_FLAGS_LEN;
				break;
			case HT_TAG_GROUP:
				return c;
			case HT_TAG_DESC_BYTE:
				c++;
				tagstring+=HT_TAG_DESC_BYTE_LEN;
				break;
			case HT_TAG_DESC_WORD_LE:
				c++;
				tagstring+=HT_TAG_DESC_WORD_LE_LEN;
				break;
			case HT_TAG_DESC_DWORD_LE:
				c++;
				tagstring+=HT_TAG_DESC_DWORD_LE_LEN;
				break;
			case HT_TAG_DESC_QWORD_LE:
				c++;
				tagstring+=HT_TAG_DESC_QWORD_LE_LEN;
				break;
			case HT_TAG_DESC_WORD_BE:
				c++;
				tagstring+=HT_TAG_DESC_WORD_BE_LEN;
				break;
			case HT_TAG_DESC_DWORD_BE:
				c++;
				tagstring+=HT_TAG_DESC_DWORD_BE_LEN;
				break;
			case HT_TAG_DESC_QWORD_BE:
				c++;
				tagstring+=HT_TAG_DESC_QWORD_BE_LEN;
				break;
			case HT_TAG_DESC_WORD_VE:
				c++;
				tagstring+=HT_TAG_DESC_WORD_VE_LEN;
				break;
			case HT_TAG_DESC_DWORD_VE:
				c++;
				tagstring+=HT_TAG_DESC_DWORD_VE_LEN;
				break;
			case HT_TAG_DESC_QWORD_VE:
				c++;
				tagstring+=HT_TAG_DESC_QWORD_VE_LEN;
				break;
			default:
				tagstring+=tag_get_len(tagstring);
				break;
		}
	}
	return c;
}

int tag_count_selectable_tags(TAGSTRING *tagstring)
{
	int c=0;
	while ((tagstring=tag_findnext(tagstring))) {
		switch (tagstring[1]) {
			case HT_TAG_EDIT_BYTE:
				c++;
				tagstring+=HT_TAG_EDIT_BYTE_LEN;
				break;
			case HT_TAG_EDIT_WORD_LE:
				c++;
				tagstring+=HT_TAG_EDIT_WORD_LE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_LE:
				c++;
				tagstring+=HT_TAG_EDIT_DWORD_LE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_LE:
				c++;
				tagstring+=HT_TAG_EDIT_QWORD_LE_LEN;
				break;
			case HT_TAG_EDIT_WORD_BE:
				c++;
				tagstring+=HT_TAG_EDIT_WORD_BE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_BE:
				c++;
				tagstring+=HT_TAG_EDIT_DWORD_BE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_BE:
				c++;
				tagstring+=HT_TAG_EDIT_QWORD_BE_LEN;
				break;
			case HT_TAG_EDIT_WORD_VE:
				c++;
				tagstring+=HT_TAG_EDIT_WORD_VE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_VE:
				c++;
				tagstring+=HT_TAG_EDIT_DWORD_VE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_VE:
				c++;
				tagstring+=HT_TAG_EDIT_QWORD_VE_LEN;
				break;
			case HT_TAG_EDIT_TIME:
				c++;
				tagstring+=HT_TAG_EDIT_TIME_LEN;
				break;
			case HT_TAG_EDIT_CHAR:
				c++;
				tagstring+=HT_TAG_EDIT_CHAR_LEN;
				break;
			case HT_TAG_EDIT_BIT:
				c++;
				tagstring+=HT_TAG_EDIT_BIT_LEN;
				break;
			case HT_TAG_SEL:
				c++;
				tagstring+=HT_TAG_SEL_LEN(((ht_tag_sel*)tagstring)->strlen);
				break;
			case HT_TAG_FLAGS:
				c++;
				tagstring+=HT_TAG_FLAGS_LEN;
				break;
			case HT_TAG_DESC_BYTE:
				c++;
				tagstring+=HT_TAG_DESC_BYTE_LEN;
				break;
			case HT_TAG_DESC_WORD_LE:
				c++;
				tagstring+=HT_TAG_DESC_WORD_LE_LEN;
				break;
			case HT_TAG_DESC_DWORD_LE:
				c++;
				tagstring+=HT_TAG_DESC_DWORD_LE_LEN;
				break;
			case HT_TAG_DESC_QWORD_LE:
				c++;
				tagstring+=HT_TAG_DESC_QWORD_LE_LEN;
				break;
			case HT_TAG_DESC_WORD_BE:
				c++;
				tagstring+=HT_TAG_DESC_WORD_BE_LEN;
				break;
			case HT_TAG_DESC_DWORD_BE:
				c++;
				tagstring+=HT_TAG_DESC_DWORD_BE_LEN;
				break;
			case HT_TAG_DESC_QWORD_BE:
				c++;
				tagstring+=HT_TAG_DESC_QWORD_BE_LEN;
				break;
			case HT_TAG_DESC_WORD_VE:
				c++;
				tagstring+=HT_TAG_DESC_WORD_VE_LEN;
				break;
			case HT_TAG_DESC_DWORD_VE:
				c++;
				tagstring+=HT_TAG_DESC_DWORD_VE_LEN;
				break;
			case HT_TAG_DESC_QWORD_VE:
				c++;
				tagstring+=HT_TAG_DESC_QWORD_VE_LEN;
				break;
			default:
				tagstring+=tag_get_len(tagstring);
				break;
		}
	}
	return c;
}

int tag_count_groups(TAGSTRING *tagstring)
{
	int c=1;
	while ((tagstring=tag_findnext(tagstring))) {
		if (tagstring[1]==HT_TAG_GROUP) {
			c++;
			tagstring+=HT_TAG_GROUP_LEN;
		} else {
			tagstring+=tag_get_len(tagstring);
		}
	}
	return c;
}

TAGSTRING *tag_get_selectable_tag(TAGSTRING *tagstring, int n, int group)
{
	TAGSTRING *r=0;
	if (group>0) tagstring=tag_get_group(tagstring, group);
	n++;
	while ((n) && (tagstring=tag_findnext(tagstring))) {
		switch (tagstring[1]) {
			case HT_TAG_EDIT_BYTE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_BYTE_LEN;
				break;
			case HT_TAG_EDIT_WORD_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_WORD_LE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_DWORD_LE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_QWORD_LE_LEN;
				break;
			case HT_TAG_EDIT_WORD_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_WORD_BE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_DWORD_BE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_QWORD_BE_LEN;
				break;
			case HT_TAG_EDIT_WORD_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_WORD_VE_LEN;
				break;
			case HT_TAG_EDIT_DWORD_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_DWORD_VE_LEN;
				break;
			case HT_TAG_EDIT_QWORD_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_QWORD_VE_LEN;
				break;
			case HT_TAG_EDIT_TIME:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_TIME_LEN;
				break;
			case HT_TAG_EDIT_CHAR:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_CHAR_LEN;
				break;
			case HT_TAG_EDIT_BIT:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_BIT_LEN;
				break;
			case HT_TAG_SEL:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_SEL_LEN(((ht_tag_sel*)tagstring)->strlen);
				break;
			case HT_TAG_FLAGS:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_FLAGS_LEN;
				break;
			case HT_TAG_GROUP:
				if (group!=-1) return (char*)r;
				tagstring+=HT_TAG_GROUP_LEN;
				break;
			case HT_TAG_DESC_BYTE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_BYTE_LEN;
				break;
			case HT_TAG_DESC_WORD_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_WORD_LE_LEN;
				break;
			case HT_TAG_DESC_DWORD_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_DWORD_LE_LEN;
				break;
			case HT_TAG_DESC_QWORD_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_QWORD_LE_LEN;
				break;
			case HT_TAG_DESC_WORD_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_WORD_BE_LEN;
				break;
			case HT_TAG_DESC_DWORD_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_DWORD_BE_LEN;
				break;
			case HT_TAG_DESC_QWORD_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_QWORD_BE_LEN;
				break;
			case HT_TAG_DESC_WORD_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_WORD_VE_LEN;
				break;
			case HT_TAG_DESC_DWORD_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_DWORD_VE_LEN;
				break;
			case HT_TAG_DESC_QWORD_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_DESC_QWORD_VE_LEN;
				break;
			default:
				tagstring+=tag_get_len(tagstring);
				break;
		}
	}
	return (n == 0) ? (char*)r : NULL;
}

TAGSTRING *tag_get_group(TAGSTRING *tagstring, int group)
{
	TAGSTRING *r=tagstring;
	while ((group) && (tagstring=tag_findnext(tagstring))) {
		switch (tagstring[1]) {
			case HT_TAG_GROUP:
				group--;
				tagstring+=HT_TAG_GROUP_LEN;
				r=tagstring;
				break;
			default:
				tagstring+=tag_get_len(tagstring);
				break;
		}
	}
	return (char*)r;
}

int tag_get_class(TAGSTRING *tagstring)
{
	switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
		case HT_TAG_EDIT_WORD_LE:
		case HT_TAG_EDIT_DWORD_LE:
		case HT_TAG_EDIT_QWORD_LE:
		case HT_TAG_EDIT_WORD_BE:
		case HT_TAG_EDIT_DWORD_BE:
		case HT_TAG_EDIT_QWORD_BE:
		case HT_TAG_EDIT_WORD_VE:
		case HT_TAG_EDIT_DWORD_VE:
		case HT_TAG_EDIT_QWORD_VE:
		case HT_TAG_EDIT_TIME:
		case HT_TAG_EDIT_CHAR:
		case HT_TAG_EDIT_BIT:
			return tag_class_edit;
		case HT_TAG_SEL:
		case HT_TAG_FLAGS:
		case HT_TAG_DESC_BYTE:
		case HT_TAG_DESC_WORD_LE:
		case HT_TAG_DESC_DWORD_LE:
		case HT_TAG_DESC_QWORD_LE:
		case HT_TAG_DESC_WORD_BE:
		case HT_TAG_DESC_DWORD_BE:
		case HT_TAG_DESC_QWORD_BE:
		case HT_TAG_DESC_WORD_VE:
		case HT_TAG_DESC_DWORD_VE:
		case HT_TAG_DESC_QWORD_VE:
			return tag_class_sel;
		default:
			return tag_class_no;
	}
}

char *tag_striptags(char *dest, TAGSTRING *src)
{
	if (!dest) return NULL;
	if (!src) {
		*dest = 0;
		return dest;
	}
	char *d = dest;
	while (*src) {
		if (src[0]=='\e') {
			switch (src[1]) {
				case HT_TAG_SEL: {
					src += sizeof (ht_tag_sel);
					break;
				}
				default:
					src+=tag_get_len(src);
			}
		} else {
			*dest = *src;
			dest++;
			src++;
		}
	}
	*dest = 0;
	return d;
}

