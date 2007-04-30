/*
 *	HT Editor
 *	language.h
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

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "io/types.h"
#include "data.h"

#define NL 10
#define WHITESPACE ' '
#define COMMENT '#'
#define INV 255

extern byte mapchar[];

class analy_string: public Object {
protected:
	byte *string;
	int len;
public:
		void init(const byte *s, int Len);
	virtual void done();
	virtual void render_string(char *result, int maxlen) = 0;
		void set_len(int Len);
		int  length();
	virtual const char *name() = 0;
};

class analy_raw_string: public analy_string {
public:
	virtual	void	render_string(char *result, int maxlen);
	static	int	string_test(const byte *s, int testlen, int &foundlen);
	virtual	const char *name();
};

class analy_c_string: public analy_string {
public:
	virtual void render_string(char *result, int maxlen);
	static	int string_test(const byte *s, int testlen, int &foundlen);
	virtual const char *name();
};

class analy_unicode_string: public analy_string {
public:
	virtual void render_string(char *result, int maxlen);
	static	int string_test(const byte *s, int testlen, int &foundlen);
	virtual const char *name();
};

class analy_pascal_string: public analy_string {
public:
	virtual void render_string(char *result, int maxlen);
	static	int string_test(const byte *s, int testlen, int &foundlen);
	virtual const char *name();
};


analy_string *string_test(const byte *s, int testlen);

#endif
