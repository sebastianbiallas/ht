/* 
 *	HT Editor
 *	eval.h
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

typedef int (*eval_func_handler_t)(scalar_t *result, char *name, scalarlist_t *params);
typedef int (*eval_symbol_handler_t)(scalar_t *result, char *name);

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
#define MAX_ERRSTR_LEN		64

/*
int f2i(double f);
char *binstr2cstr(char *s, int len);
int bin2str(char *result, void *S, int len);
*/

/*
 *	ERROR HANDLING
 */

void clear_eval_error();
int get_eval_error(char **str, int *pos);
void set_eval_error(char *format,...);
void set_eval_error_ex(int pos, char *format, ...);

/*
 *
 */


#ifdef EVAL_DEBUG

void integer_dump(int_t *i);
void float_dump(float_t *f);
void string_dump(str_t *s);

#endif

void string_destroy(str_t *s);

/*
 *	SCALARLIST
 */

void scalarlist_set(scalarlist_t *l, scalar_t *s);
void scalarlist_concat(scalarlist_t *l, scalarlist_t *a, scalarlist_t *b);
void scalarlist_destroy(scalarlist_t *l);
void scalarlist_destroy_gentle(scalarlist_t *l);

#ifdef EVAL_DEBUG
void scalarlist_dump(scalarlist_t *l);
#endif

/*
 *	SCALAR
 */

void scalar_setint(scalar_t *s, int_t *i);
void scalar_setstr(scalar_t *s, str_t *t);

#ifdef EVAL_DEBUG
void scalar_dump(scalar_t *s);
#endif

void scalar_create_int(scalar_t *s, int_t *t);
void scalar_create_int_c(scalar_t *s, int i);
void scalar_create_int_q(scalar_t *s, qword q);
void scalar_create_str(scalar_t *s, str_t *t);
void scalar_create_str_c(scalar_t *s, char *cstr);
void scalar_create_float(scalar_t *s, float_t *t);
void scalar_create_float_c(scalar_t *s, double f);
void scalar_context_str(scalar_t *s, str_t *t);
void scalar_context_int(scalar_t *s, int_t *t);
void scalar_context_float(scalar_t *s, float_t *t);
void string_concat(str_t *s, str_t *a, str_t *b);
void scalar_concat(scalar_t *s, scalar_t *a, scalar_t *b);
void scalar_destroy(scalar_t *s);
int string_compare(str_t *a, str_t *b);
int scalar_strop(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op);
int scalar_float_op(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op);
int scalar_int_op(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op);
int scalar_op(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op);
void scalar_negset(scalar_t *xr, scalar_t *xa);
void scalar_miniif(scalar_t *xr, scalar_t *xa, scalar_t *xb, scalar_t *xc);
void sprintf_puts(char **b, char *blimit, char *buf);
int sprintf_percent(char **fmt, int *fmtl, char **b, char *blimit, scalar_t *s);
int func_sprintf(scalar_t *r, str_t *format, scalarlist_t *scalars);

/*
 *	FUNCTIONS
 */

int func_eval(scalar_t *r, str_t *p);
int func_error(scalar_t *r, str_t *s);
protomatch_t match_evalfunc_proto(char *name, scalarlist_t *params, evalfunc_t *proto);
int exec_evalfunc(scalar_t *r, scalarlist_t *params, evalfunc_t *proto);
int evalsymbol(scalar_t *r, char *sname);
int std_eval_func_handler(scalar_t *r, char *fname, scalarlist_t *params, evalfunc_t *protos);
int evalfunc(scalar_t *r, char *fname, scalarlist_t *params);
void *eval_get_context();
void eval_set_context(void *context);
void eval_set_func_handler(eval_func_handler_t func_handler);
void eval_set_symbol_handler(eval_symbol_handler_t symbol_handler);

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

