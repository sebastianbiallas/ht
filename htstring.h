/* 
 *	HT Editor
 *	htstring.h
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

#ifndef __HTSTRING_H__
#define __HTSTRING_H__

#include "global.h"
#include "config.h"
#include "htdata.h"

char *ht_strdup(const char *str);
int ht_strncmp(const char *s1, const char *s2, size_t max);
int ht_strnicmp(const char *s1, const char *s2, size_t max);
int ht_stricmp(const char *s1, const char *s2);

int strcicomm(const char *s1, const char *s2);
int strccomm(const char *s1, const char *s2);
#define strend(s) ((s)+strlen(s))
int escape_special_str(char *result, int resultmaxlen, const char *s, const char *specialchars=0, bool bit7=true);
int escape_special(char *result, int resultmaxlen, const void *s, int len, const char *specialchars=0, bool bit7=true);
int unescape_special_str(char *result, int resultmaxlen, const char *s);
int unescape_special(void *result, int resultmaxlen, const char *s);
int bin2str(char *result, const void *s, int len);
void wide_char_to_multi_byte(char *result, const byte *unicode, int maxlen);

/* common string parsing functions */

void whitespaces(char **str);
bool waitforchar(char **str, char b);

/* string evaluation functions */

bool bnstr(char **str, dword *v, int defaultbase);
bool bnstr(char **str, qword *q, int defaultbase);

/* hex/string functions */

int hexdigit(char a);

dword hexb(char *s);
dword hexw(char *s);
dword hexd(char *s);

char *mkhexb(char *buf, byte d);
char *mkhexw(char *buf, word d);
char *mkhexd(char *buf, dword d);
char *mkhexq(char *buf, qword q);

/*
 *	CLASS ht_data_string
 */

class ht_data_string: public ht_data {
public:
	char *value;

			ht_data_string(const char *s = 0);
	virtual 	~ht_data_string();
/* overwritten */
	virtual	int  load(ht_object_stream *f);
	virtual	void store(ht_object_stream *f);
	virtual	int	toString(char *s, int maxlen);
	virtual	OBJECT_ID object_id();
};

/*
 *	CLASS ht_string_list
 */

class ht_string_list: public ht_clist {
public:
			void init();
/* new */
			char *get_string(UINT i);
			void insert_string(char *s);
};

/*
 *	CLASS ht_sorted_string_list
 */

class ht_sorted_string_list: public ht_sorted_list {
public:
			void init(int (*compare_keys_proc)(ht_data *key_a, ht_data *key_b));
/* new */
			char *get_string(char *s);
			void insert_string(char *s);
};

int compare_keys_string(ht_data *key_a, ht_data *key_b);
int icompare_keys_string(ht_data *key_a, ht_data *key_b);

/*
 *	INIT
 */

bool init_string();

/*
 *	DONE
 */

void done_string();

#endif /* !__HTSTRING_H__ */
