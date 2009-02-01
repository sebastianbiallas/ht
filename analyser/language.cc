/*
 *	HT Editor
 *	language.cc
 *
 * 	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License version 2 as
 * 	published by the Free Software Foundation.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program; if not, write to the Free Software
 * 	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <string.h>
#include "htdebug.h"
#include "strtools.h"
#include "language.h"
#include "tools.h"

/*
 *	' ' = separator -> skip
 *	INV = invalid char -> error
 */
byte mapchar[] = {
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ', 10,' ',' ',' ',' ',' ', // 0-15
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ', // 16-31
	' ','!','"','#','$','%','&', 39,'(',')','*','+',',','-','.','/', // 32-47
	'0','0','0','0','0','0','0','0','0','0',':',';','<','=','>','?', // 48-63
	'@','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A', // 64-79
	'A','A','A','A','A','A','A','A','A','A','A','[', 92,']','^','_', // 80-95
	INV,'A','A','A','A','A','A','A','A','A','A','A','A','A','A','A', // 96-111
	'A','A','A','A','A','A','A','A','A','A','A','{','|','}','~',INV, // 112-127
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,
	INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV,INV
};

int analy_string__raw_test(const byte *s, int len)
{
	// could be a "" string
	if (!len) return 1;

	int all_word_len=0;
	int bad_chars=0;
	int words=0;
	bool word_start=false;
	for (int i=0; i<len; i++) {
		byte mc = mapchar[s[i]];
		if (s[i] == '\n' || s[i] == '\t') {
			if (word_start) {
				words++;
				word_start = false;
			}
		} else if (s[i] < 32 || mc == INV) {
			if (word_start) {
				words++;
				word_start = false;
			}
			bad_chars++;
			if (s[i] == 0) {
				bad_chars += 50;
			}
		} else if (s[i] == ' ') {
			if (word_start) {
				words++;
				word_start = false;
			}
		} else if (mc == 'A' || mc == '0') {
			word_start = true;
			all_word_len++;
		} else {
			// symbols (+-/* ...)
			// currently the same as ' '
			if (word_start) {
				words++;
				word_start = false;
			}
		}
	}
	if (word_start) {
		words++;
		word_start = false;
	}
	// more than 10% badchars --> no string
	if (bad_chars*10>len) return -1;
	// no words found
	if (!words) return len/2-bad_chars*5+1;
	int average_word_len = all_word_len / words;
	int av_res[10] = {1, 2, 4, 8, 16, 16, 10, 8, 6, 3};
	int av_plus = 0;
	if (average_word_len > 1 && average_word_len < 12) av_plus = av_res[average_word_len - 2];
	return words*2 + av_plus - bad_chars*5 + len/5 + average_word_len;
}

/*
 *	CLASS analy_string
 */
void analy_string::init(const byte *s, int Len)
{
	string = ht_malloc(Len);
	memcpy(string, s, Len);
	len = Len;
}

void analy_string::done()
{
	free(string);
}

int  analy_string::length()
{
	return len;
}

void analy_string::set_len(int Len)
{
	len = Len;
}

/*
 *	CLASS analy_raw_string
 */
void analy_raw_string::render_string(char *result, int maxlen)
{
}

int analy_raw_string::string_test(const byte *s, int testlen, int &foundlen)
{
	foundlen = 0;
	return 0;
}

const char *analy_raw_string::name()
{
	return "raw";
}

/*
 *	CLASS analy_c_string
 */
void analy_c_string::render_string(char *result, int maxlen)
{
	assert(maxlen);
	maxlen--;
	int Len = MIN(len, maxlen);
	if (Len) Len--;
	memcpy(result, string, Len);
	result[Len]=0;          
}

int analy_c_string::string_test(const byte *s, int testlen, int &foundlen)
{
	// search for \0
	byte *np = (byte *)memchr(s, 0, testlen);
	if (!np) return -1;
	int len = np-s+1;
	foundlen = len;
	return analy_string__raw_test(s, len-1);
}

const char *analy_c_string::name()
{
	return "strz";
}

/*
 *	CLASS analy_unicode_string
 */
void analy_unicode_string::render_string(char *result, int maxlen)
{
	wide_char_to_multi_byte(result, string, maxlen);
}

int analy_unicode_string::string_test(const byte *s, int testlen, int &foundlen)
{
	// this is not good
	byte *a = ht_malloc(testlen/2+1);
	wide_char_to_multi_byte((char*)a, s, testlen/2);
	// search for \0
	byte *np = (byte *)memchr(a, 0, testlen/2);
	if (!np) {
		free(a);
		return -1;
	}
	int len = np-a;
	foundlen = len*2+2;
	int res = analy_string__raw_test(a, len);
	free(a);
	return res;
}

const char *analy_unicode_string::name()
{
	return "strw";
}

/*
 *	CLASS analy_pascal_string
 */
void analy_pascal_string::render_string(char *result, int maxlen)
{
	assert(maxlen);
	maxlen--;
	int Len = MIN(*string, maxlen);
	if (Len) memcpy(result, string+1, Len);
	result[Len]=0;
}

int analy_pascal_string::string_test(const byte *s, int testlen, int &foundlen)
{
	int len = *s;
	if (len>testlen) return -1;
	foundlen = len+1;
	return analy_string__raw_test(s+1, len);
}

const char *analy_pascal_string::name()
{
	return "strp";
}

#define STRING_TESTS 2
analy_string *string_test(const byte *s, int testlen)
{
	if (!testlen) return NULL;     
	int p[STRING_TESTS+1];
	int len[STRING_TESTS];
	p[0] = analy_c_string::string_test(s, testlen, len[0]);
	p[1] = analy_unicode_string::string_test(s, testlen, len[1]);
//	p[2] = analy_pascal_string::string_test(s, testlen, len[2]);
	p[STRING_TESTS] = 5;
	int j = STRING_TESTS;
	for (int i=0; i < STRING_TESTS; i++) {
		if (p[i] > p[j]) j = i;
	}
	analy_string *as = NULL;;
	switch (j) {
		case 0:
			as = new analy_c_string();
			break;
		case 1:
			as = new analy_unicode_string();
			break;
/*		case 2:
			as = new analy_pascal_string();
			break;*/
		default:
			break;
	}
	if (as) {
		as->init(s, testlen);
		as->set_len(len[j]);
	}
	return as;
}

