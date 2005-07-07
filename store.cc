/*
 *	HT Editor
 *	store.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#include <errno.h>
#include <string.h>

#include "htendian.h"
#include "htatom.h"
#include "htdebug.h"
#include "htstring.h"
#include "language.h"
#include "store.h"
#include "snprintf.h"
#include "tools.h"

static char hexchars2[]="0123456789abcdef";

/*
 *	CLASS ht_object_stream_inter
 */
 
Object *ht_object_stream_inter::getObject(char *name)
{
	Object *o;
	getObject(o, name);
	return o;
}

void ht_object_stream_inter::getObject(Object *&o, char *name)
{
	ObjectID id=getIntHex(4, "id");
	if (id) {
		object_builder b=(object_builder)find_atom(id);
		if (b) {
			o=b();
			if (o->load(this)!=0) {
//	               o->done();
				delete o;
				o = NULL;
			}
		} else {
			/* object not registered! */
//			assert(0);
			o = NULL;
		}
	} else {
		o = NULL;
	}
}

void ht_object_stream_inter::putObject(Object *obj, char *name)
{
	if (obj) {
		ObjectID o=obj->getObjectID();
		object_builder b=(object_builder)find_atom(o);;
		if (b) {
			putIntHex(o, 4, "id");
			obj->store(this);
		} else {
			/* object not registered! */
			assert(0);
			putIntHex(0, 4, "id");
		}
	} else {
		putIntHex(0, 4, "id");
	}
}

/*
 *	CLASS ht_object_stream_bin
 */
 
void	ht_object_stream_bin::init(ht_stream *s)
{
	ht_object_stream_inter::init(s);
}
 
void *ht_object_stream_bin::getBinary(int size, char *desc)
{
	void *p = smalloc(size);
	if (stream->read(p, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
	return p;
}

void	ht_object_stream_bin::getBinary(void *p, int size, char *desc)
{
	if (stream->read(p, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

bool ht_object_stream_bin::getBool(char *desc)
{
	bool b;
	if (stream->read(&b, 1) != 1) set_error(EIO | STERR_SYSTEM);
	return b;
}

int  ht_object_stream_bin::getIntDec(int size, char *desc)
{
	return getIntHex(size, desc);
}

int  ht_object_stream_bin::getIntHex(int size, char *desc)
{
	assert(size <= 8);
	byte neta[8];
	if (stream->read(&neta, size)!=(UINT)size) set_error(EIO | STERR_SYSTEM);
	return create_host_int(neta, size, big_endian);
}

uint64  ht_object_stream_bin::getQWordDec(int size, char *desc)
{
	return getQWordHex(size, desc);
}

uint64  ht_object_stream_bin::getQWordHex(int size, char *desc)
{
	assert(size == 8);
	byte neta[8];
	if (stream->read(&neta, size)!=(UINT)size) set_error(EIO | STERR_SYSTEM);
	return create_host_int64(neta, big_endian);
}

void ht_object_stream_bin::getSeparator()
{
	// empty
}

char *ht_object_stream_bin::getString(char *desc)
{
	return getstrz(stream);
}

void ht_object_stream_bin::putBinary(void *mem, int size, char *desc)
{
	if (stream->write(mem, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::putBool(bool b, char *desc)
{
	b = (b)?1:0;
	if (stream->write(&b, 1) != 1) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::putInfo(char *info)
{
	// empty
}

void ht_object_stream_bin::putIntDec(int a, int size, char *desc)
{
	putIntHex(a, size, desc);
}

void ht_object_stream_bin::putIntHex(int a, int size, char *desc)
{
	assert(size <= 8);
	byte neta[8];
	create_foreign_int(neta, a, size, big_endian);
	if (stream->write(&neta, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::putQWordDec(uint64 a, int size, char *desc)
{
	putQWordHex(a, size, desc);
}

void ht_object_stream_bin::putQWordHex(uint64 a, int size, char *desc)
{
	assert(size == 8);
	byte neta[8];
	create_foreign_int64(neta, a, size, big_endian);
	if (stream->write(&neta, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void ht_object_stream_bin::putSeparator()
{
	// empty
}

void ht_object_stream_bin::putString(char *string, char *desc)
{
	uint len = strlen(string)+1;
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

void *ht_object_stream_txt::getBinary(int size, char *desc)
{
	void *p=smalloc(size);
	getBinary(p, size, desc);
	return p;
}

void	ht_object_stream_txt::getBinary(void *p, int size, char *desc)
{
	readDesc(desc);
	expect('=');
	expect('[');
	byte *pp=(byte *)p;
	for (int i=0; i<size; i++) {
		skipWhite();

		int bb;
		if ((bb = hexdigit(cur))==-1) setSyntaxError();
		int b = bb*16;

		readChar();
		if ((bb = hexdigit(cur))==-1) setSyntaxError();
		b += bb;

		*pp++=b;

		readChar();
	}
	expect(']');
}

bool ht_object_stream_txt::getBool(char *desc)
{
	readDesc(desc);
	expect('=');
	skipWhite();
	if (cur=='f') {
		readDesc("false");
		return false;
	} else {
		readDesc("true");
		return true;
	}
}

int  ht_object_stream_txt::getIntDec(int size, char *desc)
{
	return getIntHex(size, desc);
}

int  ht_object_stream_txt::getIntHex(int size, char *desc)
{
	return QWORD_GET_LO(getQWordHex(size, desc));
}

uint64 ht_object_stream_txt::getQWordDec(int size, char *desc)
{
	return getQWordHex(size, desc);
}

uint64 ht_object_stream_txt::getQWordHex(int size, char *desc)
{
	readDesc(desc);
	expect('=');
	skipWhite();
	if (mapchar[cur]!='0') setSyntaxError();
	char str[40];
	char *s=str;
	do {
		*s++ = cur;
		if (s-str >= 39) setSyntaxError();
		readChar();
		if (get_error()) return to_qword(0);
	} while (mapchar[cur]=='0' || mapchar[cur]=='A');
	*s=0; s=str;
	uint64 a;
	if (!bnstr(&s, &a, 10)) setSyntaxError();
	return a;
}

void	ht_object_stream_txt::getObject(Object *&o, char *name)
{
	readDesc(name);
	expect('=');
	expect('{');
	if (!get_error()) {
		ht_object_stream_inter::getObject(o, name);
	} else {
		o = NULL;
	}
	expect('}');
}

void ht_object_stream_txt::getSeparator()
{
	// do nothing
}

char *ht_object_stream_txt::getString(char *desc)
{
	readDesc(desc);
	expect('=');
	skipWhite();
	if (cur=='"') {
		char str[1024]; // FIXME: the good old buffer overflow
		char *s=str;
		do {
			readChar();
			*s++=cur;
			if (cur=='\\') {
				readChar();
				*s++=cur;
				cur = 0; // hackish
			}
			if (get_error()) return NULL;
		} while (cur!='"');
		s--;*s=0;
		readChar();
		int str2l = strlen(str)+1;
		char *str2 = (char *)smalloc(str2l);
		unescape_special_str(str2, str2l, str);
		return str2;
	} else {
		readDesc("NULL");
		return NULL;
	}
}

void ht_object_stream_txt::putBinary(void *mem, int size, char *desc)
{
	putDesc(desc);
	putChar('[');
	for(int i=0; i<size; i++) {
		byte a = *((byte *)mem+i);
		putChar(hexchars2[(a & 0xf0) >> 4]);
		putChar(hexchars2[(a & 0x0f)]);
		if (i+1<size) putChar(' ');
	}
	putS("]\n");
}

void ht_object_stream_txt::putBool(bool b, char *desc)
{
	putDesc(desc);
	if (b) putS("true"); else putS("false");
	putChar('\n');
}

void ht_object_stream_txt::putInfo(char *info)
{
	putIndent();
	putS("# ");
	putS(info);
	putChar('\n');
}

void ht_object_stream_txt::putIntDec(int a, int size, char *desc)
{
	putDesc(desc);
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
	putS(number);
}

void ht_object_stream_txt::putIntHex(int a, int size, char *desc)
{
	putDesc(desc);
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
	putS(number2);
}

void ht_object_stream_txt::putQWordDec(uint64 a, int size, char *desc)
{
	putDesc(desc);
	char number[40];
	ht_snprintf(number, sizeof number, "%qd\n", &a);
	putS(number);
}

void ht_object_stream_txt::putQWordHex(uint64 a, int size, char *desc)
{
	putDesc(desc);
	char number2[40];
	ht_snprintf(number2, sizeof number2, "0x%qx\n", a);
	putS(number2);
}

void	ht_object_stream_txt::putObject(Object *obj, char *name)
{
	putDesc(name);
	putS("{\n");
	indent++;
	ht_object_stream_inter::putObject(obj, name);
	indent--;
	putIndent();
	putS("}\n");
}

void ht_object_stream_txt::putSeparator()
{
	putIndent();
	putS("# ------------------------ \n");
}

void ht_object_stream_txt::putString(char *string, char *desc)
{
	putDesc(desc);
	if (string) {
		int strl=strlen(string)*4+1;
		char *str = (char*)smalloc(strl);
		putChar('"');
		escape_special_str(str, strl, string, "\"");
		putS(str);
		putChar('"');
		free(str);
	} else {
		putS("NULL");
	}
	putChar('\n');
}

void	ht_object_stream_txt::setSyntaxError()
{
	if (!errorline) {
		set_error(EIO | STERR_SYSTEM);
		errorline = line;
	}
}

int	ht_object_stream_txt::getErrorLine()
{
	return errorline;
}

void	ht_object_stream_txt::expect(char c)
{
	skipWhite();
	if (cur!=c) setSyntaxError();
	readChar();
}

void	ht_object_stream_txt::skipWhite()
{
	while (1) {
		switch (mapchar[cur]) {
			case '\n':
				line++;  // fallthrough
			case ' ':
				readChar();
				if (get_error()) return;
				break;
			case '#':
				do {
					readChar();
					if (get_error()) return;
				} while (cur!='\n');
				break;
			default: return;
		}
	}
}

char	ht_object_stream_txt::readChar()
{
	if (stream->read(&cur, 1) != 1) setSyntaxError();
	return cur;
}

void	ht_object_stream_txt::readDesc(char *desc)
{
	skipWhite();
	if (!desc) desc="data";
	while (*desc) {
		if (*desc!=cur) setSyntaxError();
		readChar();
		desc++;
	}
}

void ht_object_stream_txt::putDesc(char *desc)
{
	putIndent();
	if (desc) putS(desc); else putS("data");
	putChar('=');
}

void ht_object_stream_txt::putIndent()
{
	for(int i=0; i<indent; i++) putChar(' ');
}

void ht_object_stream_txt::putChar(char c)
{
	if (stream->write(&c, 1) != 1) setSyntaxError();
}

void ht_object_stream_txt::putS(char *s)
{
	uint len=strlen(s);
	if (stream->write(s, len) != len) setSyntaxError();
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

void *ht_object_stream_memmap::getBinary(int size, char *desc)
{
	void *pp;
	stream->read(&pp, sizeof pp);
	return pp;
}

void	ht_object_stream_memmap::getBinary(void *p, int size, char *desc)
{
	void *pp;
	stream->read(&pp, sizeof pp);
	memmove(p, pp, size);
}

int  ht_object_stream_memmap::getIntDec(int size, char *desc)
{
	return getIntHex(size, desc);
}

int  ht_object_stream_memmap::getIntHex(int size, char *desc)
{
	assert(size <= 8);
	int a;
	if (stream->read(&a, size)!=(UINT)size) set_error(EIO | STERR_SYSTEM);
	return a;
}

uint64 ht_object_stream_memmap::getQWordDec(int size, char *desc)
{
	return getQWordHex(size, desc);
}

uint64 ht_object_stream_memmap::getQWordHex(int size, char *desc)
{
	assert(size==8);
	uint64 a;
	if (stream->read(&a, size)!=(UINT)size) set_error(EIO | STERR_SYSTEM);
	return a;
}

char	*ht_object_stream_memmap::getString(char *desc)
{
	char *pp;
	stream->read(&pp, sizeof pp);
	return pp;
}

uint	ht_object_stream_memmap::recordStart(UINT size)
{
	return ((ht_streamfile*)stream)->tell()+size;
}

void	ht_object_stream_memmap::recordEnd(UINT a)
{
	FILEOFS o =((ht_streamfile*)stream)->tell();
	if (o>a) HT_ERROR("kput");
	((ht_streamfile*)stream)->seek(a);
}

void	ht_object_stream_memmap::putBinary(void *mem, int size, char *desc)
{
	void *pp = mem ? duppa(mem, size) : NULL;
	stream->write(&pp, sizeof pp);
}

void	ht_object_stream_memmap::putIntDec(int a, int size, char *desc)
{
	putIntHex(a, size, desc);
}

void	ht_object_stream_memmap::putIntHex(int a, int size, char *desc)
{
	assert(size <= 8);
	if (stream->write(&a, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void	ht_object_stream_memmap::putQWordDec(uint64 a, int size, char *desc)
{
	putQWordHex(a, size, desc);
}

void	ht_object_stream_memmap::putQWordHex(uint64 a, int size, char *desc)
{
	assert(size <= 8);
	if (stream->write(&a, size) != (UINT)size) set_error(EIO | STERR_SYSTEM);
}

void	ht_object_stream_memmap::putString(char *string, char *desc)
{
	char *pp = string ? (char*)duppa(string, strlen(string)+1) : NULL;
	stream->write(&pp, sizeof pp);
}
