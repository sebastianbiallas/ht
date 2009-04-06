/*
 *	HT Editor
 *	httag.cc
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

#include "htdebug.h"
#include "strtools.h"
#include "httag.h"
#include "tools.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/**/

// these 3 functions are evil. but they are only used in statictag_to_tag().
// they should go sometime...
static uint32 hexb(const char *s)
{
	byte b=*(byte*)s;
	b -= '0';
	if (b > 9) b -= 'a'-'0'-10;
	byte c = *(byte*)(s+1);
	c -= '0';
	if (c > 9) c -= 'a'-'0'-10;
	return (b << 4) + c;
}

static uint32 hexw(const char *s)
{
	return (hexb(s)<<8) | hexb(s+2);
}

static uint32 hexd(const char *s)
{
	return (hexw(s)<<16) | hexw(s+4);
}
//

static TAGSTRING *tag_error(TAGSTRING *buf, int maxlen)
{
	while (maxlen-- > 0) {
		*buf++ = 0;
	}
	return buf;
}

TAGSTRING *tag_make_sel(TAGSTRING *buf, int maxlen, const char *string)
{
	return tag_make_ref(buf, maxlen, 0, 0, 0, 0, string);
}

TAGSTRING *tag_make_ref_len(TAGSTRING *buf, int maxlen, uint32 id128_1, uint32 id128_2, uint32 id128_3, uint32 id128_4, const char *string, int strlen)
{
	if (maxlen <= (signed)sizeof (ht_tag_sel)) return tag_error(buf, maxlen);
	if (maxlen <= (signed)sizeof (ht_tag_sel)+strlen) {
		strlen = maxlen - sizeof (ht_tag_sel) - 1;
	}
	ht_tag_sel *tag=(ht_tag_sel*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_SEL;
	UNALIGNED_MOVE(tag->id128_1, id128_1);
	UNALIGNED_MOVE(tag->id128_2, id128_2);
	UNALIGNED_MOVE(tag->id128_3, id128_3);
	UNALIGNED_MOVE(tag->id128_4, id128_4);
	UNALIGNED_MOVE(tag->strlen, strlen);
	memcpy(buf+sizeof (ht_tag_sel), string, strlen);
	return buf+sizeof (ht_tag_sel)+strlen;
}

TAGSTRING *tag_make_ref(TAGSTRING *buf, int maxlen, uint32 id128_1, uint32 id128_2, uint32 id128_3, uint32 id128_4, const char *string)
{
	return tag_make_ref_len(buf, maxlen, id128_1, id128_2, id128_3, id128_4, string, strlen(string));
}

TAGSTRING *tag_make_flags(TAGSTRING *buf, int maxlen, uint32 id, FileOfs ofs)
{
	if (maxlen <= (signed)sizeof (ht_tag_flags)) return tag_error(buf, maxlen);
	ht_tag_flags *tag = (ht_tag_flags*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_FLAGS;
	UNALIGNED_MOVE(tag->offset, ofs);
	UNALIGNED_MOVE(tag->id, id);
	return buf + sizeof (ht_tag_flags);
}

TAGSTRING *tag_make_group(TAGSTRING *buf, int maxlen)
{
	if (maxlen <= (signed)sizeof (ht_tag_group)) return tag_error(buf, maxlen);
	ht_tag_group *tag = (ht_tag_group*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_GROUP;
	return buf + sizeof (ht_tag_group);
}

TAGSTRING *tag_make_color(TAGSTRING *buf, int maxlen, uint32 color)
{
	if (maxlen <= (signed)sizeof (ht_tag_color)) return tag_error(buf, maxlen);
	ht_tag_color *tag = (ht_tag_color*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_COLOR;
	UNALIGNED_MOVE(tag->color, color);
	return buf + sizeof (ht_tag_color);
}

TAGSTRING *tag_make_default_color(TAGSTRING *buf, int maxlen)
{
	if (maxlen <= (signed)sizeof (ht_tag_color)) return tag_error(buf, maxlen);
	ht_tag_color *tag = (ht_tag_color*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_COLOR;
	UNALIGNED_MOVE_CONST(tag->color, 0xffffffff, uint32);
	return buf + sizeof (ht_tag_color);
}

TAGSTRING *tag_make_edit_byte(TAGSTRING *buf, int maxlen, FileOfs ofs)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_byte)) return tag_error(buf, maxlen);
	ht_tag_edit_byte *tag = (ht_tag_edit_byte*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_EDIT_BYTE;
	UNALIGNED_MOVE(tag->offset, ofs);
	return buf + sizeof (ht_tag_edit_byte);
}

TAGSTRING *tag_make_edit_word(TAGSTRING *buf, int maxlen, FileOfs ofs, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_word_generic)) return tag_error(buf, maxlen);
	ht_tag_edit_word_generic *tag = (ht_tag_edit_word_generic*)buf;
	tag->escape = '\e';
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
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs);
	return buf + sizeof (ht_tag_edit_word_generic);
}

TAGSTRING *tag_make_edit_dword(TAGSTRING *buf, int maxlen, FileOfs ofs, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_dword_generic)) return tag_error(buf, maxlen);
	ht_tag_edit_dword_generic *tag = (ht_tag_edit_dword_generic*)buf;
	tag->escape = '\e';
	byte m = 0xff;
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
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs);
	return buf + sizeof (ht_tag_edit_dword_generic);
}

TAGSTRING *tag_make_edit_qword(TAGSTRING *buf, int maxlen, FileOfs ofs, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_qword_generic)) return tag_error(buf, maxlen);
	ht_tag_edit_qword_generic *tag = (ht_tag_edit_qword_generic*)buf;
	tag->escape = '\e';
	byte m = 0xff;
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
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs);
	return buf + sizeof (ht_tag_edit_qword_generic);
}

TAGSTRING *tag_make_edit_time(TAGSTRING *buf, int maxlen, FileOfs ofs, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_time)) return tag_error(buf, maxlen);
	ht_tag_edit_time *tag = (ht_tag_edit_time*)buf;
	tag->escape = '\e';
	byte m = 0xff;	
	switch (e) {
	case tag_endian_big:
		m = HT_TAG_EDIT_TIME_BE;
		break;
	case tag_endian_little:
		m = HT_TAG_EDIT_TIME_LE;
		break;
	case tag_endian_var:
		m = HT_TAG_EDIT_TIME_VE;
		break;
	}
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs);
	return buf + sizeof (ht_tag_edit_time);
}

TAGSTRING *tag_make_edit_char(TAGSTRING *buf, int maxlen, FileOfs ofs)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_char)) return tag_error(buf, maxlen);
	ht_tag_edit_char *tag = (ht_tag_edit_char*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_EDIT_CHAR;
	UNALIGNED_MOVE(tag->offset, ofs);
	return buf + sizeof (ht_tag_edit_char);
}

TAGSTRING *tag_make_edit_bit(TAGSTRING *buf, int maxlen, FileOfs ofs, int bitidx)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_bit)) return tag_error(buf, maxlen);
	ht_tag_edit_bit *tag = (ht_tag_edit_bit*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_EDIT_BIT;
	UNALIGNED_MOVE(tag->offset, ofs);
	UNALIGNED_MOVE(tag->bitidx, bitidx);
	return buf+sizeof (ht_tag_edit_bit);
}

TAGSTRING *tag_make_edit_selvis(TAGSTRING *buf, int maxlen, FileOfs offset, char ch)
{
	if (maxlen <= (signed)sizeof (ht_tag_edit_selvis)) return tag_error(buf, maxlen);
	ht_tag_edit_selvis *tag=(ht_tag_edit_selvis*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_EDIT_SELVIS;
	UNALIGNED_MOVE(tag->offset, offset);
	tag->ch = ch;
	return buf + sizeof (ht_tag_edit_selvis);
}

TAGSTRING *tag_make_desc_byte(TAGSTRING *buf, int maxlen, FileOfs ofs32, uint32 id32)
{
	if (maxlen <= (signed)sizeof (ht_tag_desc_byte)) return tag_error(buf, maxlen);
	ht_tag_desc_byte *tag = (ht_tag_desc_byte*)buf;
	tag->escape = '\e';
	tag->magic = HT_TAG_DESC_BYTE;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf + sizeof (ht_tag_desc_byte);
}

TAGSTRING *tag_make_desc_word(TAGSTRING *buf, int maxlen, FileOfs ofs32, uint32 id32, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_desc_word_generic)) return tag_error(buf, maxlen);
	ht_tag_desc_word_generic *tag = (ht_tag_desc_word_generic*)buf;
	tag->escape = '\e';
	byte m = 0xff;
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
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf + sizeof (ht_tag_desc_word_generic);
}

TAGSTRING *tag_make_desc_dword(TAGSTRING *buf, int maxlen, FileOfs ofs32, uint32 id32, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_desc_dword_generic)) return tag_error(buf, maxlen);
	ht_tag_desc_dword_generic *tag = (ht_tag_desc_dword_generic*)buf;
	tag->escape = '\e';
	byte m = 0xff;
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
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf + sizeof (ht_tag_desc_dword_generic);
}

TAGSTRING *tag_make_desc_qword(TAGSTRING *buf, int maxlen, FileOfs ofs32, uint32 id32, tag_endian e)
{
	if (maxlen <= (signed)sizeof (ht_tag_desc_qword_generic)) return tag_error(buf, maxlen);
	ht_tag_desc_qword_generic *tag = (ht_tag_desc_qword_generic*)buf;
	tag->escape = '\e';
	byte m = 0xff;
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
	tag->magic = m;
	UNALIGNED_MOVE(tag->offset, ofs32);
	UNALIGNED_MOVE(tag->id, id32);
	return buf + sizeof (ht_tag_desc_qword_generic);
}

/**/

void statictag_to_tag(const char *statictag_str, TAGSTRING *tag_str, int maxlen, uint64 relocation, bool std_bigendian)
{
	if (maxlen < 1) return;
	if (maxlen == 1) {
		*tag_str = 0;
		return;
	}
	FileOfs ofs = 0;
	ID id;
	TAGSTRING *tag_str_end = tag_str + maxlen - 1; 
	while (*statictag_str) {
		if (*statictag_str == '\e') {
			switch ((byte)*(statictag_str+1)) {
			case HT_STATICTAG_EDIT_BYTE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_byte(tag_str, tag_str_end-tag_str, ofs+relocation);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_WORD_LE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_word(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_DWORD_LE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_dword(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_QWORD_LE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_qword(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_WORD_BE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_word(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_big);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_DWORD_BE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_dword(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_big);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_QWORD_BE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_qword(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_big);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_WORD_VE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_word(tag_str, tag_str_end-tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_DWORD_VE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_dword(tag_str, tag_str_end-tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_QWORD_VE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_qword(tag_str, tag_str_end-tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_TIME_LE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_time(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_TIME_BE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_time(tag_str, tag_str_end-tag_str, ofs+relocation, tag_endian_big);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_TIME_VE:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_time(tag_str, tag_str_end-tag_str, ofs+relocation, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_CHAR:
				ofs = hexd(statictag_str+2);
				tag_str = tag_make_edit_char(tag_str, tag_str_end-tag_str, ofs+relocation);
				statictag_str += 2+8;
				break;
			case HT_STATICTAG_EDIT_BIT: {
				ofs = hexd(statictag_str+2);
				int bitidx = hexb(statictag_str+2+8);
				tag_str = tag_make_edit_bit(tag_str, tag_str_end-tag_str, ofs+relocation, bitidx);
				statictag_str += 2+8+2;
				break;
			}
			case HT_STATICTAG_EDIT_SELVIS: {
				ofs = hexd(statictag_str+2);
				char ch = hexb(statictag_str+2+8);
				tag_str = tag_make_edit_selvis(tag_str, tag_str_end-tag_str, ofs+relocation, ch);
				statictag_str += 2+8+2;
				break;
			}
			case HT_STATICTAG_DESC_BYTE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_byte(tag_str, tag_str_end-tag_str, ofs+relocation, id);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_WORD_LE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_word(tag_str, tag_str_end-tag_str, ofs+relocation, id, tag_endian_little);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_DWORD_LE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_dword(tag_str, tag_str_end-tag_str, ofs+relocation, id, tag_endian_little);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_QWORD_LE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_qword(tag_str, tag_str_end-tag_str, ofs+relocation, id, tag_endian_little);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_WORD_BE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_word(tag_str, tag_str_end-tag_str, ofs+relocation, id, tag_endian_big);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_DWORD_BE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_dword(tag_str, tag_str_end-tag_str, ofs+relocation, id, tag_endian_big);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_QWORD_BE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_qword(tag_str, tag_str_end-tag_str, ofs+relocation, id, tag_endian_big);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_WORD_VE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_word(tag_str, tag_str_end-tag_str, ofs+relocation, id, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_DWORD_VE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_dword(tag_str, tag_str_end-tag_str, ofs+relocation, id, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_DESC_QWORD_VE:
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_desc_qword(tag_str, tag_str_end-tag_str, ofs+relocation, id, std_bigendian ? tag_endian_big : tag_endian_little);
				statictag_str += 2+8+8;
				break;
			case HT_STATICTAG_SEL: {
				uint32 id_1 = hexd(statictag_str+2);
				uint32 id_2 = hexd(statictag_str+2+8);
				uint32 id_3 = hexd(statictag_str+2+16);
				uint32 id_4 = hexd(statictag_str+2+24);
				byte len = hexb(statictag_str+2+8+8+8+8);
				tag_str = tag_make_ref_len(tag_str, tag_str_end-tag_str, id_1, id_2, id_3, id_4, statictag_str+2+8+8+8+8+2, len);
				statictag_str += 2+8+8+8+8+2+len;
				break;
			}
			case HT_STATICTAG_FLAGS: {
				ofs = hexd(statictag_str+2);
				id = hexd(statictag_str+2+8);
				tag_str = tag_make_flags(tag_str, tag_str_end-tag_str, id, ofs+relocation);
				statictag_str += 2+8+8;
				break;
			}
			case HT_STATICTAG_GROUP:
				tag_str = tag_make_group(tag_str, tag_str_end-tag_str);
				statictag_str += 2;
				break;
			case HT_STATICTAG_COLOR: {
				byte color = hexb(statictag_str+2);
				tag_str = tag_make_color(tag_str, tag_str_end-tag_str, color);
				statictag_str += 2+2;
				break;
			}
			default:
				HT_ERROR("error in statictag string!");
			}
		} else {
			if (tag_str < tag_str_end) {
				*tag_str++ = *statictag_str;
			}
			statictag_str++;
		}
	}
	*tag_str = 0;
}

const TAGSTRING *tag_findnext(const TAGSTRING *tagstring)
{
	return strchr(tagstring, '\e');
}

int tag_get_len(const TAGSTRING *tagstring)
{
	assert(tagstring && tagstring[0] == '\e');
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
	case HT_TAG_EDIT_TIME_LE:
		return HT_TAG_EDIT_TIME_LE_LEN;
	case HT_TAG_EDIT_TIME_BE:
		return HT_TAG_EDIT_TIME_BE_LEN;
	case HT_TAG_EDIT_TIME_VE:
		return HT_TAG_EDIT_TIME_VE_LEN;
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

int tag_get_vlen(const TAGSTRING *tagstring)
{
	assert(tagstring && tagstring[0] == '\e');
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
	case HT_TAG_EDIT_TIME_LE:
		return HT_TAG_EDIT_TIME_LE_VLEN;
	case HT_TAG_EDIT_TIME_BE:
		return HT_TAG_EDIT_TIME_BE_VLEN;
	case HT_TAG_EDIT_TIME_VE:
		return HT_TAG_EDIT_TIME_VE_VLEN;
	case HT_TAG_EDIT_CHAR:
		return HT_TAG_EDIT_CHAR_VLEN;
	case HT_TAG_EDIT_BIT:
		return HT_TAG_EDIT_BIT_VLEN;
	case HT_TAG_EDIT_SELVIS:
		return HT_TAG_EDIT_SELVIS_VLEN;
	case HT_TAG_SEL:
		return HT_TAG_SEL_VLEN(((ht_tag_sel*)tagstring)->strlen);
	default:
		return 0;
	}
}

static int time_mp[14] = {0,1,3,4,6,7,9,10,12,13,15,16,17,18};

int tag_get_micropos(const TAGSTRING *tagstring, int i)
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
	case HT_TAG_EDIT_TIME_LE:
	case HT_TAG_EDIT_TIME_BE:
	case HT_TAG_EDIT_TIME_VE:
		return time_mp[i];
	}
//	assert(0);
	return -1;
}

int tag_get_microsize(const TAGSTRING *tagstring)
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
	case HT_TAG_EDIT_TIME_LE:
	case HT_TAG_EDIT_TIME_BE:
	case HT_TAG_EDIT_TIME_VE:
		return 14;
	case HT_TAG_EDIT_CHAR:
		return HT_TAG_EDIT_CHAR_VLEN;
	case HT_TAG_EDIT_BIT:
		return HT_TAG_EDIT_BIT_VLEN;
	default:
		return 1;
	}
}

int tag_get_size(const TAGSTRING *tagstring)
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
	case HT_TAG_EDIT_TIME_LE:
		return HT_TAG_EDIT_TIME_LE_SIZE;
	case HT_TAG_EDIT_TIME_BE:
		return HT_TAG_EDIT_TIME_BE_SIZE;
	case HT_TAG_EDIT_TIME_VE:
		return HT_TAG_EDIT_TIME_VE_SIZE;
	case HT_TAG_EDIT_CHAR:
		return HT_TAG_EDIT_CHAR_SIZE;
	case HT_TAG_EDIT_BIT:
		return HT_TAG_EDIT_BIT_SIZE;
	default:
		return 0;
	}
}

FileOfs tag_get_offset(const TAGSTRING *tagstring)
{
	FileOfs f;
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
	case HT_TAG_EDIT_TIME_LE:
	case HT_TAG_EDIT_TIME_BE:
	case HT_TAG_EDIT_TIME_VE: {
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
	case HT_TAG_FLAGS:
		UNALIGNED_MOVE(f, ((ht_tag_flags*)tagstring)->offset);
		return f;
	}
	assert(0);
	return 0;
}

void tag_get_id(const TAGSTRING *tagstring, uint32 *id128_1, uint32 *id128_2, uint32 *id128_3, uint32 *id128_4)
{
	if (tagstring[1] == HT_TAG_SEL) {
		UNALIGNED_MOVE(*id128_1, ((ht_tag_sel*)tagstring)->id128_1);
		UNALIGNED_MOVE(*id128_2, ((ht_tag_sel*)tagstring)->id128_2);
		UNALIGNED_MOVE(*id128_3, ((ht_tag_sel*)tagstring)->id128_3);
		UNALIGNED_MOVE(*id128_4, ((ht_tag_sel*)tagstring)->id128_4);
	}
}

TAGSTRING *tag_get_seltext(const TAGSTRING *tagstring)
{
	return (TAGSTRING *)(tagstring + sizeof(ht_tag_sel));
}

int tag_get_seltextlen(const TAGSTRING *tagstring)
{
	if (tagstring[1] == HT_TAG_SEL) {
		return ((ht_tag_sel*)tagstring)->strlen;
	}
	return -1;
}

vcp tag_get_color(const TAGSTRING *tagstring)
{
	vcp c;
	UNALIGNED_MOVE(c, ((ht_tag_color*)tagstring)->color);
	return c;
}

bool tag_get_desc_id(const TAGSTRING *tagstring, uint32 *id)
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

void tag_set_offset(const TAGSTRING *tagstring, FileOfs offset)
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
	case HT_TAG_EDIT_TIME_LE:
	case HT_TAG_EDIT_TIME_BE:
	case HT_TAG_EDIT_TIME_VE:
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


bool tag_is_editable(const TAGSTRING *tagstring)
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
	case HT_TAG_EDIT_TIME_LE:
	case HT_TAG_EDIT_TIME_BE:
	case HT_TAG_EDIT_TIME_VE:
	case HT_TAG_EDIT_CHAR:
	case HT_TAG_EDIT_BIT:
		return true;
	default:
		return false;
	}
	assert(0);
	return false;
}

void tag_strcat(TAGSTRING *dest, int maxlen, const TAGSTRING *src)
{
	int l = tag_strlen(dest);
	tag_strcpy(dest+l, maxlen-l, src);
}

void tag_strcpy(TAGSTRING *dest, int maxlen, const TAGSTRING *src)
{
	if (maxlen > 0) {
		maxlen--;
		while (*src && maxlen) {
			if (*src == '\e') {
				int l = tag_get_len(src);
				if (l > maxlen) {
					tag_error(dest, maxlen);
					return;
				} else {
					memcpy(dest, src, l);
					dest += l;
					src += l;
					maxlen -= l;
				}
			} else {
				*dest++ = *src++;
				maxlen--;
			}
		}
		*dest = 0;
	}
}

TAGSTRING *tag_strdup(const TAGSTRING *tagstring)
{
	if (!tagstring) return NULL;
	int l = tag_strlen(tagstring);
	TAGSTRING *s = ht_malloc(l + 1);
	memcpy(s, tagstring, l);
	s[l] = 0;
	return s;
}

int tag_strlen(const TAGSTRING *tagstring)
{
	int c = 0, r;
	while (*tagstring) {
		if (tagstring[0] == '\e') {
			r = tag_get_len(tagstring);
		} else {
			r = 1;
		}
		tagstring += r;
		c += r;
	}
	return c;
}

int tag_strvlen(const TAGSTRING *tagstring)
{
	int c=0, r, v;
	while (*tagstring) {
		if (tagstring[0] == '\e') {
			r = tag_get_len(tagstring);
			v = tag_get_vlen(tagstring);
		} else {
			r = 1;
			v = 1;
		}
		tagstring += r;
		c += v;
	}
	return c;
}

int tag_count_selectable_tags_in_group(const TAGSTRING *tagstring, int group)
{
	int c = 0;
	tagstring = tag_get_group(tagstring, group);
	while ((tagstring = tag_findnext(tagstring))) {
		switch (tagstring[1]) {
		case HT_TAG_EDIT_BYTE:
			c++;
			tagstring += HT_TAG_EDIT_BYTE_LEN;
			break;
		case HT_TAG_EDIT_WORD_LE:
			c++;
			tagstring += HT_TAG_EDIT_WORD_LE_LEN;
			break;
		case HT_TAG_EDIT_DWORD_LE:
			c++;
			tagstring += HT_TAG_EDIT_DWORD_LE_LEN;
			break;
		case HT_TAG_EDIT_QWORD_LE:
			c++;
			tagstring += HT_TAG_EDIT_QWORD_LE_LEN;
			break;
		case HT_TAG_EDIT_WORD_BE:
			c++;
			tagstring += HT_TAG_EDIT_WORD_BE_LEN;
			break;
		case HT_TAG_EDIT_DWORD_BE:
			c++;
			tagstring += HT_TAG_EDIT_DWORD_BE_LEN;
			break;
		case HT_TAG_EDIT_QWORD_BE:
			c++;
			tagstring += HT_TAG_EDIT_QWORD_BE_LEN;
			break;
		case HT_TAG_EDIT_WORD_VE:
			c++;
			tagstring += HT_TAG_EDIT_WORD_VE_LEN;
			break;
		case HT_TAG_EDIT_DWORD_VE:
			c++;
			tagstring += HT_TAG_EDIT_DWORD_VE_LEN;
			break;
		case HT_TAG_EDIT_QWORD_VE:
			c++;
			tagstring += HT_TAG_EDIT_QWORD_VE_LEN;
			break;
		case HT_TAG_EDIT_TIME_LE:
			c++;
			tagstring += HT_TAG_EDIT_TIME_LE_LEN;
			break;
		case HT_TAG_EDIT_TIME_BE:
			c++;
			tagstring += HT_TAG_EDIT_TIME_BE_LEN;
			break;
		case HT_TAG_EDIT_TIME_VE:
			c++;
			tagstring += HT_TAG_EDIT_TIME_VE_LEN;
			break;
		case HT_TAG_EDIT_CHAR:
			c++;
			tagstring += HT_TAG_EDIT_CHAR_LEN;
			break;
		case HT_TAG_EDIT_BIT:
			c++;
			tagstring += HT_TAG_EDIT_BIT_LEN;
			break;
		case HT_TAG_SEL:
			c++;
			tagstring += HT_TAG_SEL_LEN(((ht_tag_sel*)tagstring)->strlen);
			break;
		case HT_TAG_FLAGS:
			c++;
			tagstring += HT_TAG_FLAGS_LEN;
			break;
		case HT_TAG_GROUP:
			return c;
		case HT_TAG_DESC_BYTE:
			c++;
			tagstring += HT_TAG_DESC_BYTE_LEN;
			break;
		case HT_TAG_DESC_WORD_LE:
			c++;
			tagstring += HT_TAG_DESC_WORD_LE_LEN;
			break;
		case HT_TAG_DESC_DWORD_LE:
			c++;
			tagstring += HT_TAG_DESC_DWORD_LE_LEN;
			break;
		case HT_TAG_DESC_QWORD_LE:
			c++;
			tagstring += HT_TAG_DESC_QWORD_LE_LEN;
			break;
		case HT_TAG_DESC_WORD_BE:
			c++;
			tagstring += HT_TAG_DESC_WORD_BE_LEN;
			break;
		case HT_TAG_DESC_DWORD_BE:
			c++;
			tagstring += HT_TAG_DESC_DWORD_BE_LEN;
			break;
		case HT_TAG_DESC_QWORD_BE:
			c++;
			tagstring += HT_TAG_DESC_QWORD_BE_LEN;
			break;
		case HT_TAG_DESC_WORD_VE:
			c++;
			tagstring += HT_TAG_DESC_WORD_VE_LEN;
			break;
		case HT_TAG_DESC_DWORD_VE:
			c++;
			tagstring += HT_TAG_DESC_DWORD_VE_LEN;
			break;
		case HT_TAG_DESC_QWORD_VE:
			c++;
			tagstring += HT_TAG_DESC_QWORD_VE_LEN;
			break;
		default:
			tagstring += tag_get_len(tagstring);
			break;
		}
	}
	return c;
}

int tag_count_selectable_tags(const TAGSTRING *tagstring)
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
		case HT_TAG_EDIT_TIME_LE:
			c++;
			tagstring+=HT_TAG_EDIT_TIME_LE_LEN;
			break;
		case HT_TAG_EDIT_TIME_BE:
			c++;
			tagstring+=HT_TAG_EDIT_TIME_BE_LEN;
			break;
		case HT_TAG_EDIT_TIME_VE:
			c++;
			tagstring+=HT_TAG_EDIT_TIME_VE_LEN;
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

int tag_count_groups(const TAGSTRING *tagstring)
{
	int c = 1;
	while ((tagstring = tag_findnext(tagstring))) {
		if (tagstring[1] == HT_TAG_GROUP) {
			c++;
			tagstring += HT_TAG_GROUP_LEN;
		} else {
			tagstring += tag_get_len(tagstring);
		}
	}
	return c;
}

TAGSTRING *tag_get_selectable_tag(const TAGSTRING *tagstring, int n, int group)
{
	const TAGSTRING *r = NULL;
	if (group > 0) tagstring = tag_get_group(tagstring, group);
	n++;
	while (n && (tagstring = tag_findnext(tagstring))) {
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
			case HT_TAG_EDIT_TIME_LE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_TIME_LE_LEN;
				break;
			case HT_TAG_EDIT_TIME_BE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_TIME_BE_LEN;
				break;
			case HT_TAG_EDIT_TIME_VE:
				n--;
				r=tagstring;
				tagstring+=HT_TAG_EDIT_TIME_VE_LEN;
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

TAGSTRING *tag_get_group(const TAGSTRING *tagstring, int group)
{
	const TAGSTRING *r=tagstring;
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
	return (TAGSTRING *)r;
}

int tag_get_class(const TAGSTRING *tagstring)
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
	case HT_TAG_EDIT_TIME_LE:
	case HT_TAG_EDIT_TIME_BE:
	case HT_TAG_EDIT_TIME_VE:
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

char *tag_striptags(char *dest, const TAGSTRING *src)
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
				src += tag_get_len(src);
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

