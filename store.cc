/*
 *      HT Editor
 *      store.cc
 *
 *      Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@web-productions.de)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2 as
 *      published by the Free Software Foundation.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "htendian.h"
#include "htatom.h"
#include "htdebug.h"
#include "htstring.h"
#include "language.h"
#include "store.h"
#include "tools.h"
#include <errno.h>
#include <string.h>

char hexchars2[]="0123456789abcdef";

/*
 *	CLASS ht_object_stream_inter
 */
 
object *ht_object_stream_inter::get_object(char *name)
{
	object *o;
	get_object(o, name);
	return o;
}

void ht_object_stream_inter::get_object(object *&o, char *name)
{
	OBJECT_ID id=get_int_hex(4, "id");
	object_builder b=(object_builder)find_atom(id);
	if (b) {
		o=b();
		if (o->load(this)!=0) {
//               o->done();
			delete o;
			o = NULL;
		}
	} else {
		o = NULL;
	}
}

void ht_object_stream_inter::put_object(object *obj, char *name)
{
	OBJECT_ID o;
	object_builder b=0;
	if (obj) {
		o=obj->object_id();
		b=(object_builder)find_atom(o);
	}
	if (b) {
		put_int_hex(o, 4, "id");
		obj->store(this);
	} else {
		put_int_hex(0, 4, "id");
	}
}

/*
 *	CLASS ht_object_stream_bin
 */
 
void	ht_object_stream_bin::init(ht_stream *s)
{
	ht_object_stream_inter::init(s);
}
 
void *ht_object_stream_bin::get_binary(int size, char *desc)
{
	void *p = smalloc(size);
	if (stream->read(p, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
	return p;
}

void	ht_object_stream_bin::get_binary(void *p, int size, char *desc)
{
	if (stream->read(p, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

bool ht_object_stream_bin::get_bool(char *desc)
{
	bool b;
	if (stream->read(&b, 1) != 1) set_error(EIO | STERR_SYSTEM);
	return b;
}

int  ht_object_stream_bin::get_int_dec(int size, char *desc)
{
	return get_int_hex(size, desc);
}

int  ht_object_stream_bin::get_int_hex(int size, char *desc)
{
	if (size>8) assert(0);
	byte neta[8];
	int a;
	if (stream->read(&neta, size)!=(UINT)size) set_error(EIO | STERR_SYSTEM);
	a = create_host_int(neta, size, big_endian);
	return a;
}

void ht_object_stream_bin::get_separator()
{
	// empty
}

char *ht_object_stream_bin::get_string(char *desc)
{
	return getstrz(stream);
}

void ht_object_stream_bin::put_binary(void *mem, int size, char *desc)
{
	if (stream->write(mem, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::put_bool(bool b, char *desc)
{
	b = (b)?1:0;
	if (stream->write(&b, 1) != 1) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::put_info(char *info)
{
	// empty
}

void ht_object_stream_bin::put_int_dec(int a, int size, char *desc)
{
	put_int_hex(a, size, desc);
}

void ht_object_stream_bin::put_int_hex(int a, int size, char *desc)
{
	if (size>8) assert(0);
	byte neta[8];
	create_foreign_int(neta, a, size, big_endian);
	if (stream->write(&neta, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::put_separator()
{
	// empty
}

void ht_object_stream_bin::put_string(char *string, char *desc)
{
	UINT len = strlen(string)+1;
	if (stream->write(string, len) != len) set_error(EIO | STERR_SYSTEM);
}

/*
 *	CLASS ht_object_stream_txt
 */
 
void ht_object_stream_txt::init(ht_stream *s)
{
	ht_object_stream::init(s);
	indent = 0;
	cur = ' '; // important to initialize it to a whitespace char
	line = 1;
	errorline = 0;
}

void *ht_object_stream_txt::get_binary(int size, char *desc)
{
	void *p=smalloc(size);
	get_binary(p, size, desc);
	return p;
}

void	ht_object_stream_txt::get_binary(void *p, int size, char *desc)
{
	read_desc(desc);
	expect('=');
	expect('[');
	byte *pp=(byte *)p;
	for (int i=0; i<size; i++) {
		skip_white();

		int bb;
		if ((bb = hexdigit(cur))==-1) set_syntax_error();
		int b = bb*16;

		read_char();
		if ((bb = hexdigit(cur))==-1) set_syntax_error();
		b += bb;

		*pp++=b;

		read_char();
	}
	expect(']');
}

bool ht_object_stream_txt::get_bool(char *desc)
{
	read_desc(desc);
	expect('=');
	skip_white();     
	if (cur=='f') {
		read_desc("false");
		return false;
	} else {
		read_desc("true");
		return true;
	}
}

int  ht_object_stream_txt::get_int_dec(int size, char *desc)
{
	return get_int_hex(size, desc);
}

int  ht_object_stream_txt::get_int_hex(int size, char *desc)
{
	read_desc(desc);
	expect('=');
	skip_white();
	if (mapchar[cur]!='0') set_syntax_error();
	char str[12];
	char *s=str;
	do {
		*s++ = cur;
		read_char();
		if (get_error()) return 0;
	} while (mapchar[cur]=='0' || mapchar[cur]=='A');
	*s=0; s=str;
	dword a;
	if (!bnstr(&s, &a, 10)) set_syntax_error();
	return a;
}

void	ht_object_stream_txt::get_object(object *&o, char *name)
{
	read_desc(name);
	expect('=');
	expect('{');
	if (!get_error()) {
		ht_object_stream_inter::get_object(o, name);
	} else {
		o = NULL;
	}
	expect('}');
}

void ht_object_stream_txt::get_separator()
{
	// do nothing
}

char *ht_object_stream_txt::get_string(char *desc)
{
	read_desc(desc);
	expect('=');
	skip_white();
	if (cur=='"') {
		char str[1024]; // FIXME: the good old buffer overflow
		char *s=str;
		do {
			read_char();
			*s++=cur;
			if (cur=='\\') {
				read_char();
				*s++=cur;
				cur = 0; // hackish
			}
			if (get_error()) return NULL;
		} while (cur!='"');
		s--;*s=0;
		read_char();
		int str2l = strlen(str)+1;
		char *str2 = (char *)smalloc(str2l);
		unescape_special_str(str2, str2l, str);
		return str2;
	} else {
		read_desc("NULL");
		return NULL;
	}
}

void ht_object_stream_txt::put_binary(void *mem, int size, char *desc)
{
	put_desc(desc);
	put_char('[');
	for(int i=0; i<size; i++) {
		byte a = *((byte *)mem+i);
		put_char(hexchars2[(a & 0xf0) >> 4]);
		put_char(hexchars2[(a & 0x0f)]);
		if (i+1<size) put_char(' ');
	}
	put_s("]\n");
}

void ht_object_stream_txt::put_bool(bool b, char *desc)
{
	put_desc(desc);
	if (b) put_s("true"); else put_s("false");
	put_char('\n');
}

void ht_object_stream_txt::put_info(char *info)
{
	put_indent();
	put_s("# ");
	put_s(info);
	put_char('\n');
}

void ht_object_stream_txt::put_int_dec(int a, int size, char *desc)
{
	put_desc(desc);
	int b;
	switch (size) {
		case 1:
			b = (char) a;
		case 2:
			b = (short) a;
		case 4:
			b = (int) a;
		default:
			b = a;
	}
	char number[12];
	sprintf(number, "%d\n", a);
	put_s(number);
}

void ht_object_stream_txt::put_int_hex(int a, int size, char *desc)
{
	put_desc(desc);
	int b;
	switch (size) {
		case 1:
			b = (char) a;
		case 2:
			b = (short) a;
		case 4:
			b = (int) a;
		default:
			b = a;
	}
	char number2[12];
	sprintf(number2, "0x%x\n", b);
	put_s(number2);
}

void	ht_object_stream_txt::put_object(object *obj, char *name)
{
	put_desc(name);
	put_s("{\n");
	indent++;
	ht_object_stream_inter::put_object(obj, name);
	indent--;
	put_indent();
	put_s("}\n");
}

void ht_object_stream_txt::put_separator()
{
	put_indent();
	put_s("# ------------------------ \n");
}

void ht_object_stream_txt::put_string(char *string, char *desc)
{
	put_desc(desc);
	if (string) {
		int strl=strlen(string)*4+1;
		char *str = (char*)smalloc(strl);
		put_char('"');
		escape_special_str(str, strl, string, "\"");
		put_s(str);
		put_char('"');
		free(str);
	} else {
		put_s("NULL");
	}
	put_char('\n');
}

void	ht_object_stream_txt::set_syntax_error()
{
	if (!errorline) {
		set_error(EIO | STERR_SYSTEM);
		errorline = line;
	}
}

int	ht_object_stream_txt::get_error_line()
{
	return errorline;
}

void	ht_object_stream_txt::expect(char c)
{
	skip_white();
	if (cur!=c) set_syntax_error();
	read_char();
}

void	ht_object_stream_txt::skip_white()
{
	while (1) {
		switch (mapchar[cur]) {
			case '\n':
				line++;  // fallthrough
			case ' ':
				read_char();
				if (get_error()) return;
				break;
			case '#':
				do {
					read_char();
					if (get_error()) return;
				} while (cur!='\n');
				break;
			default: return;
		}
	}
}

char	ht_object_stream_txt::read_char()
{
	if (stream->read(&cur, 1) != 1) set_syntax_error();
	return cur;
}

void	ht_object_stream_txt::read_desc(char *desc)
{
	skip_white();
	if (!desc) desc="data";
	while (*desc) {
		if (*desc!=cur) set_syntax_error();
		read_char();
		desc++;
	}
}

void ht_object_stream_txt::put_desc(char *desc)
{
	put_indent();
	if (desc) put_s(desc); else put_s("data");
	put_char('=');
}

void ht_object_stream_txt::put_indent()
{
	for(int i=0; i<indent; i++) put_char(' ');
}

void ht_object_stream_txt::put_char(char c)
{
	if (stream->write(&c, 1) != 1) set_syntax_error();
}

void ht_object_stream_txt::put_s(char *s)
{
	UINT len=strlen(s);
	if (stream->write(s, len) != len) set_syntax_error();
}

/*
 *	CLASS ht_object_stream_memmap
 *   ht_object_stream_memmap dups strings + mem for set/getdata (pointers)
 *	and uses host endianess (integers)
 */
 
void ht_object_stream_memmap::init(ht_stream *s, bool d)
{
	ht_object_stream_bin::init(s);
	duplicate=d;
	allocd = new ht_clist();
	allocd->init();
}

void	ht_object_stream_memmap::done()
{
	ht_object_stream_bin::done();
	allocd->destroy();
	delete allocd;
}

void	*ht_object_stream_memmap::duppa(void *p, int size)
{
	if (duplicate) {
		ht_data_mem *pp = new ht_data_mem(p, size);
		allocd->insert(pp);
		return pp->value;
	} else {
		return p;
	}
}

void *ht_object_stream_memmap::get_binary(int size, char *desc)
{
	void *pp;
	stream->read(&pp, sizeof pp);
	return pp;
}

void	ht_object_stream_memmap::get_binary(void *p, int size, char *desc)
{
	void *pp;
	stream->read(&pp, sizeof pp);
	memmove(p, pp, size);
}

int  ht_object_stream_memmap::get_int_dec(int size, char *desc)
{
	return get_int_hex(size, desc);
}

int  ht_object_stream_memmap::get_int_hex(int size, char *desc)
{
	if (size>8) assert(0);
	byte neta[8];
	int a;
	if (stream->read(&neta, size)!=(UINT)size) set_error(EIO | STERR_SYSTEM);
/*	if (netendian) {
		if (!hostint(&a, neta, size)) assert(0);
	} else {*/
		memmove(&a, neta, size);
/*	}*/
	return a;
}

char	*ht_object_stream_memmap::get_string(char *desc)
{
	char *pp;
	stream->read(&pp, sizeof pp);
	return pp;
}

UINT	ht_object_stream_memmap::record_start(UINT size)
{
	return ((ht_streamfile*)stream)->tell()+size;
}

void	ht_object_stream_memmap::record_end(UINT a)
{
	FILEOFS o =((ht_streamfile*)stream)->tell();
	if (o>a) HT_ERROR("kput");
	((ht_streamfile*)stream)->seek(a);
}

void	ht_object_stream_memmap::put_binary(void *mem, int size, char *desc)
{
	void *pp = mem ? duppa(mem, size) : NULL;
	stream->write(&pp, sizeof pp);
}

void	ht_object_stream_memmap::put_int_dec(int a, int size, char *desc)
{
	put_int_hex(a, size, desc);
}

void	ht_object_stream_memmap::put_int_hex(int a, int size, char *desc)
{
	if (size>8) assert(0);
	byte neta[8];
/*	if (netendian) {
		if (!netint(neta, a, size)) assert(0);
	} else {*/
		memmove(neta, &a, size);
/*	}*/
	if (stream->write(&neta, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void	ht_object_stream_memmap::put_string(char *string, char *desc)
{
	char *pp = string ? (char*)duppa(string, strlen(string)+1) : NULL;
	stream->write(&pp, sizeof pp);
}
