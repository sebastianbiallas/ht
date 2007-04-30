/*
 *	HT Editor
 *	htstring.cc
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

#include "atom.h"
#include "htdebug.h"
#include "strtools.h"
#include "snprintf.h"
#include "stream.h"
#include "tools.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define ATOM_HT_DATA_STRING			MAGIC32("STR\x00")
#define ATOM_COMPARE_KEYS_STRING		MAGIC32("STR\x10")
#define ATOM_ICOMPARE_KEYS_STRING		MAGIC32("STR\x11")

char hexchars[17]="0123456789abcdef";

char *ht_strdup(const char *str)
{
	if (str) {
		int len = strlen(str)+1;
		char *s = ht_malloc(len);
		memmove(s, str, len);
		return s;
	} else {
		return NULL;
	}
}

/**
 *	Like ht_strdup but dups a maximum of |maxlen| characters of |str|.
 *	@returns new string
 */
char *ht_strndup(const char *str, int maxlen)
{
	maxlen ++;
	if (str) {
		int len = strlen(str)+1;
		len = MIN(len, maxlen);
		char *s = ht_malloc(len);
		memmove(s, str, len);
		return s;
	} else {
		return NULL;
	}
}

/**
 *	Like strcpy but copies a maximum of |maxlen| characters
 *	(including trainling zero).
 *	The operation is performed in a way that the trailing zero
 *	is always written if maxlen is > 0.
 *	@returns number of characters copied (without trailing zero)
 */
int ht_strncpy(char *s1, const char *s2, int maxlen)
{
	if (maxlen <= 0) return 0;
	char *os1 = s1;
	while (maxlen && *s2) {
		*s1 = *s2;
		maxlen--;
		s1++;
	}
	s1[-1] = 0;
	return s1-os1-1;
}


int ht_strncmp(const char *s1, const char *s2, size_t max)
{
	if (!s1) return s2 ? -1 : 0;
	if (!s2) return s1 ? 1 : 0;
	while (max--) {
		if (!*s1) return *s2 ? -1 : 0;
		if (!*s2) return *s1 ? 1 : 0;
		if (*s1>*s2) {
			return 1;
		} else if (*s1<*s2) {
			return -1;
		}
		s1++;s2++;
	}
	return 0;
}

int ht_strnicmp(const char *s1, const char *s2, size_t max)
{
	if (!s1) return s2 ? -1 : 0;
	if (!s2) return s1 ? 1 : 0;
	while (max--) {
		if (!*s1) return *s2 ? -1 : 0;
		if (!*s2) return *s1 ? 1 : 0;
		char c1=tolower(*s1), c2=tolower(*s2);
		if (c1>c2) {
			return 1;
		} else if (c1<c2) {
			return -1;
		}
		s1++;s2++;
	}
	return 0;
}

int ht_stricmp(const char *s1, const char *s2)
{
	if (!s1) return s2 ? -1 : 0;
	if (!s2) return s1 ? 1 : 0;
	while (1) {
		if (!*s1) return *s2 ? -1 : 0;
		if (!*s2) return *s1 ? 1 : 0;
		char c1=tolower(*s1), c2=tolower(*s2);
		if (c1>c2) {
			return 1;
		} else if (c1<c2) {
			return -1;
		}
		s1++;s2++;
	}
	return 0;
}

int strccomm(const char *s1, const char *s2)
{
	if (!s1 || !s2) return 0;
	int r=0;
	while (*s1 && *s2 && (*s1==*s2)) { s1++; s2++; r++; }
	return r;
}

int strcicomm(const char *s1, const char *s2)
{
	if (!s1 || !s2) return 0;
	int r=0;
	while (*s1 && *s2 && (tolower(*s1)==tolower(*s2))) { s1++; s2++; r++; }
	return r;
}

int escape_special_str(char *result, int resultmaxlen, const char *s, const char *specialchars, bool bit7)
{
	return escape_special(result, resultmaxlen, s, strlen(s), specialchars, bit7);
}

int escape_special(char *result, int resultmaxlen, const void *S, int len, const char *specialchars, bool bit7)
{
	byte *s = (byte*)S;
	if (!s) return 0;
	char *old = result;
	if (!specialchars) specialchars="";
	while (len--) {
		if (*s && strchr(specialchars, *s)) {
			*result++='\\';
			if (--resultmaxlen<2) break;
			*result++=*s;
			if (--resultmaxlen<2) break;
		} else if (*s<32 || (bit7 && (*s>0x7f)) || *s=='\\') {
			*result++='\\';
			if (--resultmaxlen<2) break;
			switch (*s) {
				case '\0':
					*result++='0';
					break;
				case '\a':
					*result++='a';
					break;
				case '\b':
					*result++='b';
					break;
				case '\e':
					*result++='e';
					break;
				case '\f':
					*result++='f';
					break;
				case '\n':
					*result++='n';
					break;
				case '\r':
					*result++='r';
					break;
				case '\t':
					*result++='t';
					break;
				case '\v':
					*result++='v';
					break;
				case '\\':
					*result++='\\';
					break;
				case '\"':
					*result++='"';
					break;
				default:
					*result++='x';
					if (--resultmaxlen<2) break;
					*result++=hexchars[((*s & 0xf0) >> 4)];
					if (--resultmaxlen<2) break;
					*result++=hexchars[(*s & 0x0f)];
			}
			if (--resultmaxlen<2) break;
		} else {
			*result++ = *s;
			if (--resultmaxlen<2) break;
		}
		s++;
	}
	*result = 0;
	return result-old;
}

int unescape_special_str(char *result, int resultmaxlen, const char *s)
{
	int l=unescape_special(result, resultmaxlen-1, s);
	result[l]=0;
	return l;
}

int unescape_special(void *Result, int resultmaxlen, const char *s)
{
	char *result = (char*)Result;
	char *old = result;
	while (s && *s) {
		if (*s == '\\') {
			s++;
			switch (*s) {
				case '0':
					*result++='\0';
					break;
				case 'a':
					*result++='\a';
					break;
				case 'b':
					*result++='\b';
					break;
				case 'e':
					*result++='\e';
					break;
				case 'f':
					*result++='\f';
					break;
				case 'n':
					*result++='\n';
					break;
				case 'r':
					*result++='\r';
					break;
				case 't':
					*result++='\t';
					break;
				case 'v':
					*result++='\v';
					break;
				case '\\':
					*result++='\\';
					break;
				case '\"':
					*result++='"';
					break;
				case 'x':
					s++;
					byte v=hexdigit(*s)*16;
					s++;
					v+=hexdigit(*s);
					*result++=(char)v;
			}
			if (!--resultmaxlen) break;
		} else {
			*result++ = *s;
			if (!--resultmaxlen) break;
		}
		s++;
	}
	return result-old;
}

int bin2str(char *result, const void *S, int len)
{
	byte *s = (byte*)S;
	while (len--) {
//		if (*s<32) *result=' '; else *result=*s;
		if (*s==0) *result=' '; else *result=*s;
		result++;
		s++;
	}
	*result=0;
	return len;
}

void wide_char_to_multi_byte(char *result, const byte *Unicode, int maxlen)
{
	if (!maxlen) return;
	struct doof {
		char c1;
		char c2;
	};
	doof *unicode = (doof*)Unicode;
	for (int i=0; i<maxlen-1; i++) {
		if (unicode->c2) {
			*result++ = '”';
		} else {
			if (!unicode->c1) break;
			*result++ = unicode->c1;
		}
		unicode++;
	}
	*result=0;
}

/* common string parsing functions */

void whitespaces(char **str)
{
	while ((unsigned char)**str<=32) {
		if (!**str) return;
		(*str)++;
	}
}

void non_whitespaces(char **str)
{
	while ((unsigned char)**str>32) {
		(*str)++;
	}
}

bool waitforchar(char **str, char b)
{
	while (**str!=b) {
		if (!**str) return false;
		(*str)++;
	}
	return true;
}

/*
static bool bnstr2bin(char *str, char *p, int base, uint32 *v)
{
	*v=0;
	do {
		int c=hexdigit(*str);
		if ((c==-1) || (c>=base)) return false;
		(*v)*=base;
		*v+=c;
		str++;
	} while (str<p);
	return true;
}
*/

static bool bnstr2bin(char *str, char *p, int base, uint64 *q)
{
	*q = to_qword(0);
	uint64 qbase = to_qword(base);
	do {
		int c = hexdigit(*str);
		if ((c == -1) || (c >= base)) return false;
		(*q) *= qbase;
		*q += to_qword(c);
		str++;
	} while (str < p);
	return true;
}

bool bnstr(char **str, uint64 *q, int defaultbase)
{
	int base=defaultbase;
	int t=0;
	char *p=*str;
	while (!strchr("+-*/%()[] \t#.,:;", *p) && (*p)) p++;
	if (p==*str) return false; /* zero length */
	if (strncmp("0x", *str, 2)==0) {
		(*str)+=2;
		base=16;
	} else {
		switch (*(p-1)) {
			case 'b':
				if (base <= 'b'-'a'+10) {
					base = 2;
					p--;
					t++;
				}
				break;
			case 'o':
				if (base <= 'o'-'a'+10) {
					base = 8;
					p--;
					t++;
				}
				break;
			case 'd':
				if (base <= 'd'-'a'+10) {
					base = 10;
					p--;
					t++;
				}
				break;
			case 'h':
				if (base <= 'h'-'a'+10) {
					base = 16;
					p--;
					t++;
				}
				break;
			default:
				if (**str=='0') base=8;
		}
	}
	if (bnstr2bin(*str, p, base, q)) {
		*str=p+t;
		return true;
	}
	return false;
}

bool bnstr(char **str, uint32 *v, int defaultbase)
{
	uint64 q;
	bool res = bnstr(str, &q, defaultbase);
	*v = QWORD_GET_LO(q);
	return res;
}

/* hex/string functions */

int hexdigit(char a)
{
	if ((a>='0') && (a<='9')) {
		return a-'0';
	} else if ((a>='a') && (a<='f')) {
		return a-'a'+10;
	} else if ((a>='A') && (a<='F')) {
		return a-'A'+10;
	}
	return -1;
}

bool hexb_ex(uint8 &result, const char *s)
{
	int v, w;
	v = hexdigit(s[0]);
	w = hexdigit(s[1]);
	if ((v < 0) || (w < 0)) return false;
	result = (v<<4) | w;
	return true;
}

bool hexw_ex(uint16 &result, const char *s)
{
	uint8 v, w;
	if (!hexb_ex(v, s) || !hexb_ex(w, s+2)) return false;
	result = (v<<8) | w;
	return true;
}

bool hexd_ex(uint32 &result, const char *s)
{
	uint16 v, w;
	if (!hexw_ex(v, s) || !hexw_ex(w, s+4)) return false;
	result = (v<<16) | w;
	return true;
}

char *mkhexb(char *buf, byte d)
{
	*buf++=hexchars[(d>>4)&0xf];
	*buf++=hexchars[d&0xf];
	return buf;
}

char *mkhexw(char *buf, uint16 d)
{
	*buf++=hexchars[(d>>12)&0xf];
	*buf++=hexchars[(d>>8)&0xf];
	*buf++=hexchars[(d>>4)&0xf];
	*buf++=hexchars[d&0xf];
	return buf;
}

char *mkhexd(char *buf, uint32 d)
{
	*buf++=hexchars[(d>>28)&0xf];
	*buf++=hexchars[(d>>24)&0xf];
	*buf++=hexchars[(d>>20)&0xf];
	*buf++=hexchars[(d>>16)&0xf];
	*buf++=hexchars[(d>>12)&0xf];
	*buf++=hexchars[(d>>8)&0xf];
	*buf++=hexchars[(d>>4)&0xf];
	*buf++=hexchars[d&0xf];
	return buf;
}

char *mkhexq(char *buf, uint64 q)
{
	*buf++=hexchars[(q.hi>>28)&0xf];
	*buf++=hexchars[(q.hi>>24)&0xf];
	*buf++=hexchars[(q.hi>>20)&0xf];
	*buf++=hexchars[(q.hi>>16)&0xf];
	*buf++=hexchars[(q.hi>>12)&0xf];
	*buf++=hexchars[(q.hi>>8)&0xf];
	*buf++=hexchars[(q.hi>>4)&0xf];
	*buf++=hexchars[q.hi&0xf];
	*buf++=hexchars[(q.lo>>28)&0xf];
	*buf++=hexchars[(q.lo>>24)&0xf];
	*buf++=hexchars[(q.lo>>20)&0xf];
	*buf++=hexchars[(q.lo>>16)&0xf];
	*buf++=hexchars[(q.lo>>12)&0xf];
	*buf++=hexchars[(q.lo>>8)&0xf];
	*buf++=hexchars[(q.lo>>4)&0xf];
	*buf++=hexchars[q.lo&0xf];
	return buf;
}

/*
 *	ht_data_string
 */
ht_data_string::ht_data_string(const char *s)
{
	value = ht_strdup(s);
}

ht_data_string::~ht_data_string()
{
	if (value) free(value);
}

int ht_data_string::load(ObjectStream &f)
{
	value = f->getString(NULL);
	return f->get_error();
}

void ht_data_string::store(ObjectStream &f)
{
	f->putString(value, NULL);
}

int ht_data_string::toString(char *s, int maxlen)
{
	return ht_snprintf(s, maxlen, "%s", value);
}

ObjectID ht_data_string::getObjectID() const
{
	return ATOM_HT_DATA_STRING;
}

/*
 *	ht_string_list
 */
void ht_string_list::init()
{
	ht_clist::init(compare_keys_string);
}

char *ht_string_list::get_string(uint i)
{
	ht_data_string *s=(ht_data_string*)get(i);
	if (s) return s->value;
	return 0;
}

void ht_string_list::insert_string(char *s)
{
	insert(new ht_data_string(s));
}

/*
 *	ht_sorted_string_list
 */
void ht_sorted_string_list::init(int (*compare_keys_proc)(ht_data *key_a, Object *key_b))
{
	ht_sorted_list::init(compare_keys_proc);
}

char *ht_sorted_string_list::get_string(char *s)
{
	Object *d = new ht_data_string(s);
	uint i=find(d);
	char *ret=NULL;
	if (i != LIST_UNDEFINED) {
		Object *r=get(i);
		if (r) ret=((ht_data_string *)r)->value;
	}
	delete d;
	return ret;
}

void ht_sorted_string_list::insert_string(char *s)
{
	insert(new ht_data_string(s));
}

/*
 *	compare_keys_string
 */

int compare_keys_string(ht_data *key_a, Object *key_b)
{
	// FIXME:
	if (((ht_data_string*)key_a)->value && ((ht_data_string*)key_b)->value) {
	return strcmp(((ht_data_string*)key_a)->value, ((ht_data_string*)key_b)->value);
	} else return 0;
}

/*
 *	icompare_keys_string
 */

int icompare_keys_string(ht_data *key_a, Object *key_b)
{
	return ht_stricmp(((ht_data_string*)key_a)->value, ((ht_data_string*)key_b)->value);
}

BUILDER(ATOM_HT_DATA_STRING, ht_data_string);

/*
 *	INIT
 */

bool init_string()
{
	REGISTER(ATOM_HT_DATA_STRING, ht_data_string);
	register_atom(ATOM_COMPARE_KEYS_STRING, (void*)compare_keys_string);
	register_atom(ATOM_ICOMPARE_KEYS_STRING, (void*)icompare_keys_string);
	return true;
}

/*
 *	DONE
 */

void done_string()
{
	UNREGISTER(ATOM_HT_DATA_STRING, ht_data_string);
	unregister_atom(ATOM_COMPARE_KEYS_STRING);
	unregister_atom(ATOM_ICOMPARE_KEYS_STRING);
}
