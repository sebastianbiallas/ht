/*
 *	HT Editor
 *	analy_names.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include "analy_names.h"
#include "htdebug.h"
#include "htstring.h"
#include "language.h"
#include "snprintf.h"
#include "stdio.h"
#include "string.h"
#include "tools.h"

char *import_func_name(const char *dllname, const char *funcname, int ordinal)
{
	char res[1024];
	if (funcname) {
		ht_snprintf(res, sizeof res, "%s:%s", dllname, funcname);

	} else {
		ht_snprintf(res, sizeof res, "%s:%04x", dllname, ordinal);
	}
	return ht_strdup(res);
}

char *export_func_name(const char *funcname, int ordinal)
{
	char res[1024];
	if (funcname) {
		ht_snprintf(res, sizeof res, "exp_%s_%x", funcname, ordinal);
	} else {
		ht_snprintf(res, sizeof res, "exp_%x", ordinal);
	}
	return ht_strdup(res);
}

char *label_types[] = {"unknown", "function", "location ", "data"};
char *label_types_short[] = {"?   ", "func", "loc ", "data"};

char *label_type(int lt)
{
	assert(lt < 4);
	return label_types[lt];
}

char *label_type_short(int lt)
{
	assert(lt < 4);
	return label_types_short[lt];
}

char	*xref_types[] = {"read", "write", "offset", "jump", "call", "ijump", "icall"};
char	xref_types_short[] = "rwojcJC";

char *xref_type(int xt)
{
	assert(xt < 7);
	return xref_types[xt];
}

char xref_type_short(int xt)
{
	assert(xt < 7);
	return xref_types_short[xt];
}

char *label_prefixes[] = {
	"unknown",
	"loc",
	"sub",
	"stub",
	"wrapper",
	"offset",
	"data",
	"?data",
	"str"
};

char *label_prefix(const char *p)
{
	if (p <= LPRFX_MAX) {
		return label_prefixes[(int)p];
	} else {
		return (char*)p;
	}
}

bool valid_name(const char *str)
{
	if ((!str) || (!*str)) return false;
	char mc = mapchar[*str];
	if ((mc == '_') || (mc == '?') || (mc == 'A') || (mc == '@')) {
		str++;
		while (*str) {
			mc = mapchar[*str];
			if ((mc == '_') || (mc == '?') || (mc == 'A') || (mc == '0') || (mc == ':') || (mc == '.') || (mc == '@')) {
				str++;
			} else return false;
		}
		return true;
	} else {
		return false;
	}
}

void make_valid_name(char *result, const char *str)
{
	if (!str || !*str) {
		*result++ = '_';
		*result = '\0';
		return;
	}
	char mc = mapchar[*str];
	if (!((mc == '_') || (mc == '?') || (mc == 'A') || (mc == '@'))) {
		*result++ = '_';
		str++;
	}
	while (*str) {
		mc = mapchar[*str];
		if ((mc == '_') || (mc == '?') || (mc == 'A') || (mc == '0') || (mc == ':') || (mc == '.') || (mc == '@')) {
			*result++ = *str;
		} else {
			*result++ = '_';
		}
		str++;
	}
	*result = 0;
}

/*
 *
 */
char the_label_prefix_string[2]="l";

char *addr_label()
{
	return the_label_prefix_string;
}

char *real_name(char *s)
{
	if (!s) return NULL;
	switch (s[0]) {
		case M_PREFIX_LABEL: {
			return "label";
			break;
		}
		case M_PREFIX_DUP:
		case M_PREFIX_REF: {
			return &s[1];
			break;
		}
		default: {}
	}
	return s;
}

char *quote_string(char *s)
{
	char *s2 = (char *) smalloc(strlen(s)+2);
	s2[0] = M_PREFIX_DUP;
	strcpy(&s2[1], s);
	return s2;
}

ht_sorted_string_list *reference_strings;

char *reference_string(char *s)
{
	char *r = reference_strings->get_string(s);
	if (!r) {
		reference_strings->insert_string(s);
		r = reference_strings->get_string(s);
	}
	return r;
}

char *comment_lookup(int special)
{
	return "testa";
}

void init_analy_names()
{
	reference_strings = new ht_sorted_string_list();
	reference_strings->init(compare_keys_string);
}

void done_analy_names()
{
	reference_strings->done();
	delete reference_strings;
}

