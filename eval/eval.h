/* 
 *	HT Editor
 *	eval.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf
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

#ifndef __EVAL_H__
#define __EVAL_H__

/*#define EVAL_DEBUG*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "evaltype.h"

#ifndef __cplusplus
#define bool int
#endif

typedef bool (*eval_func_handler)(eval_scalar *result, char *name, eval_scalarlist *params);
typedef bool (*eval_symbol_handler)(eval_scalar *result, char *name);

#include "lex.h"
#include "evalx.h"

#ifdef EVAL_DEBUG

int debug_dump_ident;

#endif

/*
 *
 */

#define MAX_FUNCNAME_LEN		16
#define MAX_SYMBOLNAME_LEN	32
#define MAX_ERRSTR_LEN		128

/*
int f2i(double f);
char *binstr2cstr(char *s, int len);
int bin2str(char *result, void *S, int len);
*/

/*
 *	ERROR HANDLING
 */

void clear_eval_error();
int get_eval_error(const char **str, int *pos);
void set_eval_error(const char *format,...);
void set_eval_error_ex(int pos, const char *format, ...);

/*
 *
 */


#ifdef EVAL_DEBUG

void integer_dump(eval_int *i);
void float_dump(eval_float *f);
void string_dump(eval_str *s);

#endif

void string_destroy(eval_str *s);

/*
 *	SCALARLIST
 */

void scalarlist_set(eval_scalarlist *l, eval_scalar *s);
void scalarlist_concat(eval_scalarlist *l, eval_scalarlist *a, eval_scalarlist *b);
void scalarlist_destroy(eval_scalarlist *l);
void scalarlist_destroy_gentle(eval_scalarlist *l);

#ifdef EVAL_DEBUG
void scalarlist_dump(eval_scalarlist *l);
#endif

/*
 *	SCALAR
 */

void scalar_setint(eval_scalar *s, eval_int *i);
void scalar_setstr(eval_scalar *s, eval_str *t);

#ifdef EVAL_DEBUG
void scalar_dump(eval_scalar *s);
#endif

void scalar_create_int(eval_scalar *result, const eval_int *t);
void scalar_create_int_c(eval_scalar *result, const int i);
void scalar_create_int_q(eval_scalar *result, const uint64 q);
void scalar_create_str(eval_scalar *result, const eval_str *t);
void scalar_create_str_c(eval_scalar *result, const char *cstr);
void scalar_create_float(eval_scalar *result, const eval_float *t);
void scalar_create_float_c(eval_scalar *result, const double f);

void scalar_clone(eval_scalar *result, const eval_scalar *s);

void scalar_context_str(const eval_scalar *s, eval_str *t);
void scalar_context_int(const eval_scalar *s, eval_int *t);
void scalar_context_float(const eval_scalar *s, eval_float *t);
void string_concat(eval_str *s, eval_str *a, eval_str *b);
void scalar_concat(eval_scalar *s, const eval_scalar *a, const eval_scalar *b);
void scalar_destroy(eval_scalar *s);
int string_compare(const eval_str *a, const eval_str *b);
int scalar_strop(eval_scalar *xr, const eval_scalar *xa, const eval_scalar *xb, int op);
int scalar_float_op(eval_scalar *xr, const eval_scalar *xa, const eval_scalar *xb, int op);
int scalar_int_op(eval_scalar *xr, const eval_scalar *xa, const eval_scalar *xb, int op);
int scalar_op(eval_scalar *xr, eval_scalar *xa, eval_scalar *xb, int op);
void scalar_negset(eval_scalar *xr, eval_scalar *xa);
void scalar_notset(eval_scalar *xr, eval_scalar *xa);
void scalar_lnotset(eval_scalar *xr, eval_scalar *xa);
void scalar_miniif(eval_scalar *xr, eval_scalar *xa, eval_scalar *xb, eval_scalar *xc);
void sprintf_puts(char **b, char *blimit, const char *buf);
int sprintf_percent(char **fmt, int *fmtl, char **b, char *blimit, eval_scalar *s);
int func_sprintf(eval_scalar *r, const eval_str *format, const eval_scalarlist *scalars);

/*
 *	FUNCTIONS
 */

int func_eval(eval_scalar *r, eval_str *p);
int func_error(eval_scalar *r, eval_str *s);
eval_protomatch match_evalfunc_proto(char *name, eval_scalarlist *params, eval_func *proto);
int exec_evalfunc(eval_scalar *r, eval_scalarlist *params, eval_func *proto);
int evalsymbol(eval_scalar *r, char *sname);
int std_eval_func_handler(eval_scalar *r, char *fname, eval_scalarlist *params, eval_func *protos);
int evalfunc(eval_scalar *r, char *fname, eval_scalarlist *params);
void *eval_get_context();
void eval_set_context(void *context);
void eval_set_func_handler(eval_func_handler func_handler);
void eval_set_symbol_handler(eval_symbol_handler symbol_handler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*
 *	Debugging
 */

#ifdef EVAL_DEBUG

extern int debug_dump_ident;

#define DEBUG_DUMP_INDENT {\
	int i;\
	for (i=0; i<debug_dump_ident; i++) printf("\t");\
}

#define DEBUG_DUMP(text...) {\
	DEBUG_DUMP_INDENT;\
	printf(text);\
	printf("\n");\
}
	
#define DEBUG_DUMP_SCALAR(scalarptr, text...) {\
	DEBUG_DUMP_INDENT;\
	printf(text);\
	scalar_dump(scalarptr);\
	printf("\n");\
}

#define DEBUG_DUMP_SCALARLIST(scalarlistptr, text...) {\
	DEBUG_DUMP_INDENT;\
	printf(text);\
	scalarlist_dump(scalarlistptr);\
	printf("\n");\
}

#define DEBUG_DUMP_INDENT_IN debug_dump_ident++
#define DEBUG_DUMP_INDENT_OUT debug_dump_ident--

#else

#define DEBUG_DUMP(text...)
#define DEBUG_DUMP_SCALAR(scalarptr, text...)
#define DEBUG_DUMP_SCALARLIST(scalarlistptr, text...)
#define DEBUG_DUMP_INDENT_IN
#define DEBUG_DUMP_INDENT_OUT

#endif

#endif /* __EVAL_H__ */

