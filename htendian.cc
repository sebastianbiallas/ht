/* 
 *	HT Editor
 *	htendian.cc
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

#include "htdebug.h"
#include "htendian.h"
#include "global.h"
#include <string.h>

void create_foreign_int(void *buf, int i, int size, endianess to_endianess)
{
	byte *b=(byte*)buf;
	switch (size) {
		case 1:
			b[0]=i;
			break;
		case 2:
			switch (to_endianess) {
				case big_endian:
					b[0]=i>>8;
					b[1]=i;
					break;
				case little_endian:
					b[0]=i;
					b[1]=i>>8;
					break;
			}
			break;
		case 4:
			switch (to_endianess) {
				case big_endian:
					b[0]=i>>24;
					b[1]=i>>16;
					b[2]=i>>8;
					b[3]=i;
					break;
				case little_endian:
					b[0]=i;
					b[1]=i>>8;
					b[2]=i>>16;
					b[3]=i>>24;
					break;
			}
			break;
	}
}

int create_host_int(const void *buf, int size, endianess from_endianess)
{
	byte *b=(byte*)buf;
	switch (size) {
		case 1:
			return b[0];
		case 2:
			switch (from_endianess) {
				case big_endian:
					return (b[0]<<8) | b[1];
				case little_endian:
					return (b[1]<<8) | b[0];
			}
			break;
		case 4:
			switch (from_endianess) {
				case big_endian:
					return (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3];
				case little_endian:
					return (b[3]<<24) | (b[2]<<16) | (b[1]<<8) | b[0];
			}
			break;
	}
	assert(0);
	return 0;
}

void create_foreign_int64(void *buf, const qword i, int size, endianess to_endianess)
{
	byte *b = (byte*)buf;
	dword hi = QWORD_GET_HI(i);
	dword lo = QWORD_GET_LO(i);
	switch (to_endianess) {
		case big_endian:
			b[0]=hi>>24;
			b[1]=hi>>16;
			b[2]=hi>>8;
			b[3]=hi;
			b[4]=lo>>24;
			b[5]=lo>>16;
			b[6]=lo>>8;
			b[7]=lo;
			break;
		case little_endian:
			b[0]=lo;
			b[1]=lo>>8;
			b[2]=lo>>16;
			b[3]=lo>>24;
			b[4]=hi;
			b[5]=hi>>8;
			b[6]=hi>>16;
			b[7]=hi>>24;
			break;
	}
}

qword create_host_int64(const void *buf, endianess from_endianess)
{
	byte *b = (byte*)buf;
	qword q;
	switch (from_endianess) {
		case big_endian:
			QWORD_SET_HI(q, (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3]);
			QWORD_SET_LO(q, (b[4]<<24) | (b[5]<<16) | (b[6]<<8) | b[7]);
			break;
		case little_endian:
			QWORD_SET_HI(q, (b[7]<<24) | (b[6]<<16) | (b[5]<<8) | b[4]);
			QWORD_SET_LO(q, (b[3]<<24) | (b[2]<<16) | (b[1]<<8) | b[0]);
			break;
	}
	return q;
}

void create_host_struct(void *buf, const byte *table, endianess from)
{
	byte *buf2 = (byte *)buf;
	while (*table) {
		int table2 = *table & ~STRUCT_ENDIAN_HOST;
		if (*table & STRUCT_ENDIAN_HOST) {
			switch (table2) {
				case STRUCT_ENDIAN_BYTE: {
					byte a = create_host_int(buf2, STRUCT_ENDIAN_BYTE, from);
					memcpy(buf2, &a, STRUCT_ENDIAN_BYTE);
					break;
				}
				case STRUCT_ENDIAN_WORD: {
					word a = create_host_int(buf2, STRUCT_ENDIAN_WORD, from);
					memcpy(buf2, &a, STRUCT_ENDIAN_WORD);
					break;
				}
				case STRUCT_ENDIAN_DWORD: {
					dword a = create_host_int(buf2, STRUCT_ENDIAN_DWORD, from);
					memcpy(buf2, &a, STRUCT_ENDIAN_DWORD);
					break;
				}
				case STRUCT_ENDIAN_QWORD: {
					qword q = create_host_int64(buf2, from);
					memcpy(buf2, &q, STRUCT_ENDIAN_QWORD);
					break;
				}
				default: {
					assert(0);
				}
			}
		}
		buf2+=table2;
		table++;
	}
}

