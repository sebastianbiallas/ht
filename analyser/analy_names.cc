/*
 *	HT Editor
 *	analy_names.cc
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

#include "analy_names.h"
#include "htdebug.h"
#include "strtools.h"
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

const char *label_types[] = {"unknown", "function", "location ", "data"};
const char *label_types_short[] = {"?   ", "func", "loc ", "data"};

const char *label_type(int lt)
{
	assert(lt < 4);
	return label_types[lt];
}

const char *label_type_short(int lt)
{
	assert(lt < 4);
	return label_types_short[lt];
}

const char *xref_types[] = {"read", "write", "offset", "jump", "call", "ijump", "icall"};
const char *xref_types_short = "rwojcJC";

const char *xref_type(int xt)
{
	assert(xt < 7);
	return xref_types[xt];
}

char xref_type_short(int xt)
{
	assert(xt < 7);
	return xref_types_short[xt];
}

bool valid_name(const char *str)
{
	if (!str || !*str) return false;
	char mc = mapchar[(uint8)*str];
	if (mc == '_' || mc == '?' || mc == 'A' || mc == '@') {
		str++;
		while (*str) {
			mc = mapchar[(uint8)*str];
			if (mc == '_' || mc == '?' || mc == 'A' || mc == '0' || mc == ':' || mc == '.' || mc == '@') {
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
	char mc = mapchar[(uint8)*str];
	if (!(mc == '_' || mc == '?' || mc == 'A' || mc == '@')) {
		*result++ = '_';
		str++;
	}
	while (*str) {
		mc = mapchar[(uint8)*str];
		if (mc == '_' || mc == '?' || mc == 'A' || mc == '0' || mc == ':' || mc == '.' || mc == '@') {
			*result++ = *str;
		} else {
			*result++ = '_';
		}
		str++;
	}
	*result = 0;
}

