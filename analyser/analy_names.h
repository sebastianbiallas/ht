/* 
 *	HT Editor
 *	analy_names.h
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

#ifndef ANALY_NAMES_H
#define ANALY_NAMES_H

char *import_func_name(const char *dllname, const char *funcname, int ordinal);
char *export_func_name(const char *funcname, int ordinal);
const char *label_type(int lt);
const char *label_type_short(int lt);
const char *xref_type(int xt);
char xref_type_short(int xt);

#define LPRFX_LOC "loc"
#define LPRFX_SUB "sub"
#define LPRFX_STUB "stub"
#define LPRFX_WRAP "wrapper"
#define LPRFX_OFS "offset"
#define LPRFX_DTA "data"
#define LPRFX_DTU "?data"
#define LPRFX_STR "str"

bool valid_name(const char *str);
void make_valid_name(char *result, const char *str);

/*
 *	generated names
 */
#define M_PREFIX_DUP ' '
#define M_PREFIX_DUPs " "
#define M_PREFIX_REF '*'
#define M_PREFIX_REFs "*"
#define M_PREFIX_LABEL 'l'
#define M_PREFIX_LABELs "l"
#define QUOTED_STRING(s) M_PREFIX_DUPs##s
#define REF_STRING(s) M_PREFIX_REFs##s

#endif
