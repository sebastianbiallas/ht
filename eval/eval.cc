/* 
 *	HT Editor
 *	eval.cc
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

#include "evaltype.h"
#include "evalparse.h"
#include "eval.h"

#ifdef EVAL_DEBUG

int debug_dump_ident;

#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 *
 */

#define MAX_FUNCNAME_LEN	16
#define MAX_SYMBOLNAME_LEN	32
#define MAX_ERRSTR_LEN		64

static eval_func_handler_t eval_func_handler;
static eval_symbol_handler_t eval_symbol_handler;
static void *eval_context;

int f2i(double f)
{
	int r;
	if (f>0) r = (int)(f+.5); else r = (int)(f-.5);
	return r;
}

char *binstr2cstr(char *s, int len)
{
	char *x=(char*)malloc(len+1);
	memmove(x, s, len);
	x[len]=0;
	return x;
}
	
int bin2str(char *result, void *S, int len)
{
	unsigned char *s = (unsigned char*)S;
	while (len--) {
		if (*s==0) *result=' '; else *result=*s;
		result++;
		s++;
	}
	*result=0;
	return len;
}

/*
 *	ERROR HANDLING
 */

static int eval_error;
static int eval_error_pos;
static char eval_errstr[MAX_ERRSTR_LEN];

void clear_eval_error()
{
	eval_error=0;
}

int get_eval_error(char **str, int *pos)
{
	if (eval_error) {
		if (str) *str=eval_errstr;
		if (pos) *pos=eval_error_pos;
		return eval_error;
	}
	if (str) *str="?";
	if (pos) *pos=0;
	return 0;
}

void set_eval_error(char *format,...)
{
	va_list vargs;
	
	va_start(vargs, format);
	vsprintf(eval_errstr, format, vargs);
	va_end(vargs);
	eval_error_pos=lex_current_buffer_pos();
	eval_error=1;
}

void set_eval_error_ex(int pos, char *format, ...)
{
	va_list vargs;
	
	va_start(vargs, format);
	vsprintf(eval_errstr, format, vargs);
	va_end(vargs);
	eval_error_pos=pos;
	eval_error=1;
}

/*
 *
 */

#ifdef EVAL_DEBUG

void integer_dump(int_t *i)
{
	printf("%d", i->value);
}

void float_dump(float_t *f)
{
	printf("%f", f->value);
}

void string_dump(str_t *s)
{
	int i;
	for (i=0; i<s->len; i++) {
		if ((unsigned)s->value[i]<32) {
			printf("\\x%x", s->value[i]);
		} else {
			printf("%c", s->value[i]);
		}
	}
}

#endif

void string_destroy(str_t *s)
{
	if (s->value) free(s->value);
}

/*
 *	SCALARLIST
 */

void scalarlist_set(scalarlist_t *l, scalar_t *s)
{
	l->count=1;
	l->scalars=(scalar_t*)malloc(sizeof (scalar_t) * l->count);
	l->scalars[0]=*s;
}

void scalarlist_concat(scalarlist_t *l, scalarlist_t *a, scalarlist_t *b)
{
	l->count=a->count+b->count;
	l->scalars=(scalar_t*)malloc(sizeof (scalar_t) * l->count);
	memmove(l->scalars, a->scalars, sizeof (scalar_t) * a->count);
	memmove(l->scalars+a->count, b->scalars, sizeof (scalar_t) * b->count);
}

void scalarlist_destroy(scalarlist_t *l)
{
	int i;
	if (l && l->scalars) {
		for (i=0; i < l->count; i++) {
			scalar_destroy(&l->scalars[i]);
		}
		free(l->scalars);
	}		
}

void scalarlist_destroy_gentle(scalarlist_t *l)
{
	if (l && l->scalars) free(l->scalars);
}

#ifdef EVAL_DEBUG

void scalarlist_dump(scalarlist_t *l)
{
	int i;
	for (i=0; i<l->count; i++) {
		scalar_dump(&l->scalars[i]);
		if (i!=l->count-1) {
			printf(", ");
		}
	}
}

#endif

/*
 *	SCALAR
 */

void scalar_setint(scalar_t *s, int_t *i)
{
	s->type=SCALAR_INT;
	s->scalar.integer=*i;
}

void scalar_setstr(scalar_t *s, str_t *t)
{
	s->type=SCALAR_STR;
	s->scalar.str=*t;
}

#ifdef EVAL_DEBUG

void scalar_dump(scalar_t *s)
{
	switch (s->type) {
		case SCALAR_STR: {
			string_dump(&s->scalar.str);
			break;
		}
		case SCALAR_INT: {
			integer_dump(&s->scalar.integer);
			break;
		}
		case SCALAR_FLOAT: {
			float_dump(&s->scalar.floatnum);
			break;
		}
		default:
			break;
	}
}

#endif

void scalar_create_int(scalar_t *s, int_t *t)
{
	s->type=SCALAR_INT;
	s->scalar.integer=*t;
}

void scalar_create_int_c(scalar_t *s, int i)
{
	s->type=SCALAR_INT;
	s->scalar.integer.value=i;
	s->scalar.integer.type=TYPE_UNKNOWN;
}

void scalar_create_str(scalar_t *s, str_t *t)
{
	s->type=SCALAR_STR;
	s->scalar.str.value=(char*)malloc(t->len ? t->len : 1);
	memmove(s->scalar.str.value, t->value, t->len);
	s->scalar.str.len=t->len;
}

void scalar_create_str_c(scalar_t *s, char *cstr)
{
	str_t t;
	t.value=cstr;
	t.len=strlen(cstr);
	scalar_create_str(s, &t);
}

void scalar_create_float(scalar_t *s, float_t *t)
{
	s->type=SCALAR_FLOAT;
	s->scalar.floatnum=*t;
}

void scalar_create_float_c(scalar_t *s, double f)
{
	s->type=SCALAR_FLOAT;
	s->scalar.floatnum.value=f;
}

void scalar_context_str(scalar_t *s, str_t *t)
{
	switch (s->type) {
		case SCALAR_INT: {
			char buf[16];
			sprintf(buf, "%d", s->scalar.integer.value);
			t->value=(char*)strdup(buf);
			t->len=strlen(buf);
			break;
		}
		case SCALAR_STR: {
			t->value=(char*)malloc(s->scalar.str.len ? s->scalar.str.len : 1);
			t->len=s->scalar.str.len;
			memmove(t->value, s->scalar.str.value, t->len);
			break;
		}			
		case SCALAR_FLOAT: {
			char buf[32];
			sprintf(buf, "%f", s->scalar.floatnum.value);
			t->value=(char*)strdup(buf);
			t->len=strlen(buf);
			break;
		}
		default:
			break;
	}					
}

void scalar_context_int(scalar_t *s, int_t *t)
{
	switch (s->type) {
		case SCALAR_INT: {
			*t=s->scalar.integer;
			break;
		}
		case SCALAR_STR: {
			char *x=binstr2cstr(s->scalar.str.value, s->scalar.str.len);
			t->value=strtol(x, (char**)NULL, 10);
			t->type=TYPE_UNKNOWN;
			free(x);
			break;
		}			
		case SCALAR_FLOAT: {
			t->value=f2i(s->scalar.floatnum.value);
			t->type=TYPE_UNKNOWN;
			break;
		}
		default:
			break;
	}					
}

void scalar_context_float(scalar_t *s, float_t *t)
{
	switch (s->type) {
		case SCALAR_INT: {
			t->value=s->scalar.integer.value;
			break;
		}
		case SCALAR_STR:  {
			char *x=binstr2cstr(s->scalar.str.value, s->scalar.str.len);
			t->value=strtod(x, (char**)NULL);
			free(x);
			break;
		}			
		case SCALAR_FLOAT: {
			*t=s->scalar.floatnum;
			break;
		}
		default:
			break;
	}					
}

void string_concat(str_t *s, str_t *a, str_t *b)
{
	s->value=(char*)malloc(a->len+b->len ? a->len+b->len : 1);
	memmove(s->value, a->value, a->len);
	memmove(s->value+a->len, b->value, b->len);
	s->len=a->len+b->len;
	
	free(a->value);
	a->len=0;
	free(b->value);
	b->len=0;
}

void scalar_concat(scalar_t *s, scalar_t *a, scalar_t *b)
{
	str_t as, bs, rs;
	scalar_context_str(a, &as);
	scalar_context_str(b, &bs);
	string_concat(&rs, &as, &bs);
	s->type=SCALAR_STR;
	s->scalar.str=rs;
}

void scalar_destroy(scalar_t *s)
{
	switch (s->type) {
		case SCALAR_STR:
			string_destroy(&s->scalar.str);
			break;
		default:
			break;
	}
}

int string_compare(str_t *a, str_t *b)
{
	if (a->len==b->len) {
		return memcmp(a->value, b->value, a->len);
	}
	return a->len-b->len;
}

int scalar_strop(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op)
{
	str_t as, bs;
	int r;
	int c;
	scalar_context_str(xa, &as);
	scalar_context_str(xb, &bs);
	
	c=string_compare(&as, &bs);
	switch (op) {
		case EVAL_STR_EQ: r=(c==0); break;
		case EVAL_STR_NE: r=(c!=0); break;
		case EVAL_STR_GT: r=(c>0); break;
		case EVAL_STR_GE: r=(c>=0); break;
		case EVAL_STR_LT: r=(c<0); break;
		case EVAL_STR_LE: r=(c>=0); break;
		default: 
			return 0;
	}
	xr->type=SCALAR_INT;
	xr->scalar.integer.value=r;
	xr->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

int ipow(int a, int b)
{
	b %= sizeof (int) * 8;
	int i, r = 1;
	for (i=0; i<b; i++) {
		r *= a;
	}
	return r;
}

int scalar_float_op(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op)
{
	float_t ai, bi;
	float a, b, r;
	scalar_context_float(xa, &ai);
	scalar_context_float(xb, &bi);
	
	a=ai.value;
	b=bi.value;
	switch (op) {
		case '*': r=a*b; break;
		case '/': {
		    if (!b) {
			    set_eval_error("division by zero");
			    return 0;
		    }
		    r=a/b;
		    break;
		}			    
		case '+': r=a+b; break;
		case '-': r=a-b; break;
		case EVAL_POW: r=pow(a,b); break;
		case EVAL_EQ: r=(a==b); break;
		case EVAL_NE: r=(a!=b); break;
		case EVAL_GT: r=(a>b); break;
		case EVAL_GE: r=(a>=b); break;
		case EVAL_LT: r=(a<b); break;
		case EVAL_LE: r=(a<=b); break;
		case EVAL_LAND: r=(a) && (b); break;
		case EVAL_LXOR: r=(a && !b) || (!a && b); break;
		case EVAL_LOR: r=(a||b); break;
		default: 
			set_eval_error("invalid operator");
			return 0;
	}
	xr->type=SCALAR_FLOAT;
	xr->scalar.floatnum.value=r;
	return 1;
}

int scalar_int_op(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op)
{
	int_t ai, bi;
	int a, b, r;
	scalar_context_int(xa, &ai);
	scalar_context_int(xb, &bi);
	
	a=ai.value;
	b=bi.value;
	switch (op) {
		case '*': r=a*b; break;
		case '/': {
		    if (!b) {
			    set_eval_error("division by zero");
			    return 0;
		    }
		    r=a/b;
		    break;
		}			    
		case '%': {
		    if (!b) {
			    set_eval_error("division by zero");
			    return 0;
		    }
		    r=a%b;
		    break;
		}			    
		case '+': r=a+b; break;
		case '-': r=a-b; break;
		case '&': r=a&b; break;
		case '|': r=a|b; break;
		case '^': r=a^b; break;
		case EVAL_POW: r=ipow(a,b); break;
		case EVAL_SHL: r=a<<b; break;
		case EVAL_SHR: r=a>>b; break;
		case EVAL_EQ: r=(a==b); break;
		case EVAL_NE: r=(a!=b); break;
		case EVAL_GT: r=(a>b); break;
		case EVAL_GE: r=(a>=b); break;
		case EVAL_LT: r=(a<b); break;
		case EVAL_LE: r=(a<=b); break;
		case EVAL_LAND: r=(a) && (b); break;
		case EVAL_LXOR: r=(a && !b) || (!a && b); break;
		case EVAL_LOR: r=(a||b); break;
		default: 
			set_eval_error("invalid operator");
			return 0;
	}
	xr->type=SCALAR_INT;
	xr->scalar.integer.value=r;
	xr->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

int scalar_op(scalar_t *xr, scalar_t *xa, scalar_t *xb, int op)
{
	int r;
	if ((xa->type==SCALAR_FLOAT) || (xb->type==SCALAR_FLOAT)) {
		r=scalar_float_op(xr, xa, xb, op);
	} else {
		r=scalar_int_op(xr, xa, xb, op);
	}
	scalar_destroy(xa);
	scalar_destroy(xb);
	return r;
}
	
void scalar_negset(scalar_t *xr, scalar_t *xa)
{
	if (xa->type==SCALAR_FLOAT) {
		float_t a;
		a=xa->scalar.floatnum;
	
		xr->type=SCALAR_FLOAT;
		xr->scalar.floatnum.value=-a.value;
	} else {
		int_t a;
		scalar_context_int(xa, &a);
	
		xr->type=SCALAR_INT;
		xr->scalar.integer.value=-a.value;
		xr->scalar.integer.type=TYPE_UNKNOWN;
	}
	scalar_destroy(xa);
}

void scalar_miniif(scalar_t *xr, scalar_t *xa, scalar_t *xb, scalar_t *xc)
{
	int_t a;
	scalar_context_int(xa, &a);
	if (a.value) {
		*xr=*xb;
	} else {
		*xr=*xc;
	}
	scalar_destroy(xa);
}

/*
 *	BUILTIN FUNCTIONS
 */

int func_char(scalar_t *r, int_t *i)
{
	str_t s;
	char c=i->value;
	s.value=&c;
	s.len=1;
	scalar_create_str(r, &s);
	return 1;
}

int func_float(scalar_t *r, float_t *p)
{
	scalar_create_float(r, p);
	return 1;
}

int func_fmax(scalar_t *r, float_t *p1, float_t *p2)
{
	r->type=SCALAR_FLOAT;
	r->scalar.floatnum.value=(p1->value>p2->value) ? p1->value : p2->value;
	return 1;
}

int func_fmin(scalar_t *r, float_t *p1, float_t *p2)
{
	r->type=SCALAR_FLOAT;
	r->scalar.floatnum.value=(p1->value<p2->value) ? p1->value : p2->value;
	return 1;
}

int func_int(scalar_t *r, int_t *p)
{
	scalar_create_int(r, p);
	return 1;
}

int func_ord(scalar_t *r, str_t *s)
{
	if (s->len>=1) {
		scalar_create_int_c(r, s->value[0]);
		return 1;
	}
	set_eval_error("string must at least contain one character");
	return 0;		
}

int func_max(scalar_t *r, int_t *p1, int_t *p2)
{
	scalar_create_int(r, (p1->value>p2->value) ? p1 : p2);
	return 1;
}

int func_min(scalar_t *r, int_t *p1, int_t *p2)
{
	scalar_create_int(r, (p1->value<p2->value) ? p1 : p2);
	return 1;
}

int func_random(scalar_t *r, int_t *p1)
{
	scalar_create_int_c(r, (p1->value) ? (rand() % p1->value):0);
	return 1;
}

int func_rnd(scalar_t *r)
{
	scalar_create_int_c(r, rand() % 10);
	return 1;
}

int func_round(scalar_t *r, float_t *p)
{
	r->type=SCALAR_INT;
	r->scalar.integer.value=f2i(p->value+0.5);
	r->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

int func_strchr(scalar_t *r, str_t *p1, str_t *p2)
{
	if (p2->len) {
		if (p1->len) {
			char *pos = (char *)memchr(p1->value, *p2->value, p1->len);
			if (pos) {
				scalar_create_int_c(r, pos-p1->value);
			} else {
				scalar_create_int_c(r, -1);
			}
		} else {
			scalar_create_int_c(r, -1);
		}
		return 1;
	} else {
		return 0;
	}
}

int func_strcmp(scalar_t *r, str_t *p1, str_t *p2)
{
	int r2=memcmp(p1->value, p2->value, MIN(p1->len, p2->len));
	if (r2) {
		scalar_create_int_c(r, r2);
	} else {
		if (p1->len > p2->len) {
			scalar_create_int_c(r, 1);
		} else if (p1->len < p2->len) {
			scalar_create_int_c(r, -1);
		} else {
			scalar_create_int_c(r, 0);
		}
	}
	return 1;     
}

int func_string(scalar_t *r, str_t *p)
{
	scalar_create_str(r, p);
	return 1;
}

int func_strlen(scalar_t *r, str_t *p1)
{
	scalar_create_int_c(r, p1->len);
	return 1;
}

int func_strncmp(scalar_t *r, str_t *p1, str_t *p2, int_t *p3)
{
	return 1;
}

int func_strrchr(scalar_t *r, str_t *p1, str_t *p2)
{
	return 1;
}

int func_strstr(scalar_t *r, str_t *p1, str_t *p2)
{

	return 1;
}

int func_substr(scalar_t *r, str_t *p1, int_t *p2, int_t *p3)
{
	if (p2->value >= 0 && p3->value > 0) {
		if (p2->value < p1->len) {
			str_t s;
			s.len = MIN(p3->value, p1->len-p2->value);
			s.value = &p1->value[p2->value];
			scalar_create_str(r, &s);
		} else {
			scalar_create_str_c(r, "");
		}
	} else {
		scalar_create_str_c(r, "");
	}
	return 1;
}

int func_trunc(scalar_t *r, float_t *p)
{
	r->type=SCALAR_INT;
	r->scalar.integer.value=f2i(p->value);
	r->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

#define EVALFUNC_FMATH1(name) int func_##name(scalar_t *r, float_t *p)\
{\
	r->type=SCALAR_FLOAT;\
	r->scalar.floatnum.value=name(p->value);\
	return 1;\
}

#define EVALFUNC_FMATH1i(name) int func_##name(scalar_t *r, float_t *p)\
{\
	r->type=SCALAR_INT;\
	r->scalar.integer.value=f2i(name(p->value));\
	r->scalar.integer.type=TYPE_UNKNOWN;\
	return 1;\
}

#define EVALFUNC_FMATH2(name) int func_##name(scalar_t *r, float_t *p1, float_t *p2)\
{\
	r->type=SCALAR_FLOAT;\
	r->scalar.floatnum.value=name(p1->value, p2->value);\
	return 1;\
}

EVALFUNC_FMATH2(pow)

EVALFUNC_FMATH1(sqrt)

EVALFUNC_FMATH1(exp)
EVALFUNC_FMATH1(log)

EVALFUNC_FMATH1i(ceil)
EVALFUNC_FMATH1i(floor)

EVALFUNC_FMATH1(sin)
EVALFUNC_FMATH1(cos)
EVALFUNC_FMATH1(tan)

EVALFUNC_FMATH1(asin)
EVALFUNC_FMATH1(acos)
EVALFUNC_FMATH1(atan)

EVALFUNC_FMATH1(sinh)
EVALFUNC_FMATH1(cosh)
EVALFUNC_FMATH1(tanh)

#ifdef HAVE_ASINH
EVALFUNC_FMATH1(asinh)
#endif

#ifdef HAVE_ACOSH
EVALFUNC_FMATH1(acosh)
#endif

#ifdef HAVE_ATANH
EVALFUNC_FMATH1(atanh)
#endif

void sprintf_puts(char **b, char *blimit, char *buf)
{
	while ((*b<blimit) && (*buf)) {
		**b=*(buf++);
		(*b)++;
	}
}

int sprintf_percent(char **fmt, int *fmtl, char **b, char *blimit, scalar_t *s)
{
	char cfmt[32];
	char buf[512];
	int ci=1;
	cfmt[0]='%';
	while ((*fmtl) && (ci<32-1)) {
		cfmt[ci]=(*fmt)[0];
		cfmt[ci+1]=0;
		switch ((*fmt)[0]) {
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			case 'c': {
				int_t i;
				scalar_context_int(s, &i);
				
				sprintf(buf, cfmt, i.value);
				sprintf_puts(b, blimit, buf);
				
				return 1;
			}
			case 's': {
				char *q=cfmt+1;
				str_t t;
/*				int l;*/
				scalar_context_str(s, &t);
				
				while (*q!='s') {
					if ((*q>='0') && (*q<='9')) {
						unsigned int sl=strtol(q, NULL, 10);
						if (sl>sizeof buf-1) sl=sizeof buf-1;
						sprintf(q, "%ds", sl);
						break;
					} else {
						switch (*q) {
							case '+':
							case '-':
							case '#':
							case ' ':
								break;
							default:
							/* FIXME: invalid format */
								break;
						}
					}
					q++;
				}
				
				if ((unsigned int)t.len>sizeof buf-1) t.len=sizeof buf-1;
				t.value[t.len]=0;
				
				sprintf(buf, cfmt, t.value);
				
/*				l=t.len;
				if (l > (sizeof buf)-1) l=(sizeof buf)-1;

				memmove(buf, t.value, l);
				buf[l]=0;*/
				sprintf_puts(b, blimit, buf);
				
				string_destroy(&t);
				return 1;
			}
			case 'e':
			case 'E':
			case 'f':
			case 'F':
			case 'g':
			case 'G': {
				float_t f;
				scalar_context_float(s, &f);
				
				sprintf(buf, cfmt, f.value);
				sprintf_puts(b, blimit, buf);
				
				return 1;
			}
			case '%':
				sprintf_puts(b, blimit, "%");
				return 1;
		}
		(*fmt)++;
		(*fmtl)--;
		ci++;
	}
	return 0;
}

int func_sprintf(scalar_t *r, str_t *format, scalarlist_t *scalars)
{
	char buf[512];		/* FIXME: possible buffer overflow */
	char *b=buf;
	char *fmt;
	int fmtl;
	scalar_t *s=scalars->scalars;
	
	fmt=format->value;
	fmtl=format->len;
	
	while (fmtl) {
		if (fmt[0]=='%') {
			fmt++;
			fmtl--;
			if (!fmtl) break;
			if (fmt[0]!='%') {
				if (s-scalars->scalars >= scalars->count) {
					DEBUG_DUMP("too few parameters");
					return 0;
				}					
				if (!sprintf_percent(&fmt, &fmtl, &b, buf+sizeof buf, s)) return 0;
				s++;
			} else {
				*b++=fmt[0];
				if (b-buf>=512) break;
			}
		} else {
			*b++=fmt[0];
			if (b-buf>=512) break;
		}
		fmt++;
		fmtl--;
	}
	*b=0;
	r->type=SCALAR_STR;
	r->scalar.str.value=(char*)strdup(buf);
	r->scalar.str.len=strlen(r->scalar.str.value);
	return 1;
}

/*
 *	FUNCTIONS
 */

int func_eval(scalar_t *r, str_t *p)
{
	char *q=(char*)malloc(p->len+1);
	int x;
	memmove(q, p->value, p->len);
	q[p->len]=0;
	x=eval(r, q, eval_func_handler, eval_symbol_handler, eval_context);
	free(q);
/*     if (get_eval_error(NULL, NULL)) {
		eval_error_pos+=lex_current_buffer_pos();
	}*/
	return x;
}

int func_error(scalar_t *r, str_t *s)
{
	char c[1024];
	bin2str(c, s->value, MIN((unsigned int)s->len, sizeof c));
	set_eval_error(c);
	return 0;
}

evalfunc_t builtin_evalfuncs[]=	{
/* eval */
	{ "eval", &func_eval, {SCALAR_STR}, "evaluate string" },
/* type juggling */
	{ "int", &func_int, {SCALAR_INT}, "converts to integer" },
	{ "string", &func_string, {SCALAR_STR}, "converts to string" },
	{ "float", &func_float, {SCALAR_FLOAT}, "converts to float" },
/*
	{ "is_int", &func_is_int, {SCALAR_INT}, "returns non-zero if param is an integer" },
	{ "is_string", &func_is_string, {SCALAR_STR}, "returns non-zero if param is a string" },
	{ "is_float", &func_is_float, {SCALAR_FLOAT}, "returns non-zero if param is a float" },
*/
/* general */
	{ "error", &func_error, {SCALAR_STR}, "abort with error" },
/* string functions */
	{ "char", &func_char, {SCALAR_INT}, "return the ascii character (1-char string) specified by p1" },
	{ "ord", &func_ord, {SCALAR_STR}, "return the ordinal value of p1" },
	{ "sprintf", &func_sprintf, {SCALAR_STR, SCALAR_VARARGS}, "returns formatted string" },
	{ "strchr", &func_strchr, {SCALAR_STR, SCALAR_STR}, "returns position of first occurrence of character param2 in param1" },
	{ "strcmp", &func_strcmp, {SCALAR_STR, SCALAR_STR}, "returns zero for equality, positive number for str1 > str2 and negative number for str1 < str2" },
	{ "strlen", &func_strlen, {SCALAR_STR}, "returns length of string" },
	{ "strncmp", &func_strncmp, {SCALAR_STR, SCALAR_STR, SCALAR_INT}, "like strcmp, but considers a maximum of param3 characters" },
	{ "strstr", &func_strchr, {SCALAR_STR, SCALAR_STR}, "returns position of first occurrence of string param2 in param1" },
	{ "substr", &func_substr, {SCALAR_STR, SCALAR_INT, SCALAR_INT}, "returns substring from param1, start param2, length param3" },
/*	{ "stricmp", &func_stricmp, {SCALAR_STR, SCALAR_STR}, "like strcmp but case-insensitive" },
	{ "strnicmp", &func_strnicmp, {SCALAR_STR, SCALAR_STR}, "" }, */
/* math */	
	{ "pow", &func_pow, {SCALAR_FLOAT, SCALAR_FLOAT}, 0 },
	{ "sqrt", &func_sqrt, {SCALAR_FLOAT}, 0 },
	
	{ "fmin", &func_fmin, {SCALAR_FLOAT, SCALAR_FLOAT}, 0 },
	{ "fmax", &func_fmax, {SCALAR_FLOAT, SCALAR_FLOAT}, 0 },
	{ "min", &func_min, {SCALAR_INT, SCALAR_INT}, 0 },
	{ "max", &func_max, {SCALAR_INT, SCALAR_INT}, 0 },
	
	{ "random", &func_random, {SCALAR_INT}, "returns a random integer between 0 and param1-1" },
	{ "rnd", &func_rnd, {}, "returns a random number between 0 and 1" },

	{ "exp", &func_exp, {SCALAR_FLOAT}, 0 },
	{ "log", &func_log, {SCALAR_FLOAT}, 0 },
	
	{ "ceil", &func_ceil, {SCALAR_FLOAT}, 0 },
	{ "floor", &func_floor, {SCALAR_FLOAT}, 0 },
	{ "round", &func_round, {SCALAR_FLOAT}, 0 },
	{ "trunc", &func_trunc, {SCALAR_FLOAT}, 0 },
	
	{ "sin", &func_sin, {SCALAR_FLOAT}, 0 },
	{ "cos", &func_cos, {SCALAR_FLOAT}, 0 },
	{ "tan", &func_tan, {SCALAR_FLOAT}, 0 },
	
	{ "asin", &func_asin, {SCALAR_FLOAT}, 0 },
	{ "acos", &func_acos, {SCALAR_FLOAT}, 0 },
	{ "atan", &func_atan, {SCALAR_FLOAT}, 0 },
	
	{ "sinh", &func_sinh, {SCALAR_FLOAT}, 0 },
	{ "cosh", &func_cosh, {SCALAR_FLOAT}, 0 },
	{ "tanh", &func_tanh, {SCALAR_FLOAT}, 0 },
	
#ifdef HAVE_ASINH
	{ "asinh", &func_asinh, {SCALAR_FLOAT}, 0 },
#endif

#ifdef HAVE_ACOSH
	{ "acosh", &func_acosh, {SCALAR_FLOAT}, 0 },
#endif

#ifdef HAVE_ATANH
	{ "atanh", &func_atanh, {SCALAR_FLOAT}, 0 },
#endif
	
	{ NULL, NULL }
};

protomatch_t match_evalfunc_proto(char *name, scalarlist_t *params, evalfunc_t *proto)
{
	int j;
	int protoparams=0;
	
	if (strcmp(name, proto->name)!=0) return PROTOMATCH_NAME_FAIL;
	
	for (j=0; j<MAX_EVALFUNC_PARAMS; j++) {
		if (proto->ptype[j]==SCALAR_NULL) break;
		if (proto->ptype[j]==SCALAR_VARARGS) {
			if (params->count > protoparams) protoparams=params->count;
			break;
		}
		protoparams++;
	}
	return (protoparams==params->count) ? PROTOMATCH_OK : PROTOMATCH_PARAM_FAIL;
}

int exec_evalfunc(scalar_t *r, scalarlist_t *params, evalfunc_t *proto)
{
	int j;
	int retv;
	scalar_t sc[MAX_EVALFUNC_PARAMS];
	void *pptrs[MAX_EVALFUNC_PARAMS];
	int protoparams=0;
	scalarlist_t *sclist=0;
	char *errmsg;
	int errpos;

	for (j=0; j<MAX_EVALFUNC_PARAMS; j++) {
		sc[j].type=SCALAR_NULL;
		pptrs[j]=NULL;
	}					
	
	DEBUG_DUMP("%s:", proto->name);
	
	for (j=0; j<MAX_EVALFUNC_PARAMS; j++) {
		int term=0;
		if (proto->ptype[j]==SCALAR_NULL) break;
		switch (proto->ptype[j]) {
			case SCALAR_INT:
				protoparams++;
				if (params->count<protoparams) return 0;
				sc[j].type=SCALAR_INT;
				scalar_context_int(&params->scalars[j], &sc[j].scalar.integer);
				pptrs[j]=&sc[j].scalar.integer;

				DEBUG_DUMP_SCALAR(&sc[j], "param %d: int=", j);
				break;
			case SCALAR_STR:
				protoparams++;
				if (params->count<protoparams) return 0;
				sc[j].type=SCALAR_STR;
				scalar_context_str(&params->scalars[j], &sc[j].scalar.str);
				pptrs[j]=&sc[j].scalar.str;
				
				DEBUG_DUMP_SCALAR(&sc[j], "param %d: str=", j);
				break;
			case SCALAR_FLOAT:
				protoparams++;
				if (params->count<protoparams) return 0;
				sc[j].type=SCALAR_FLOAT;
				scalar_context_float(&params->scalars[j], &sc[j].scalar.floatnum);
				pptrs[j]=&sc[j].scalar.floatnum;

				DEBUG_DUMP_SCALAR(&sc[j], "param %d: float=", j);
				break;
			case SCALAR_VARARGS: {
				sclist=(scalarlist_t*)malloc(sizeof (scalarlist_t));
				sclist->count=params->count-j;
				if (sclist->count) {
					sclist->scalars=(scalar_t*)malloc(sizeof (scalar_t) * sclist->count);
					memmove(sclist->scalars, &params->scalars[j], sizeof (scalar_t) * sclist->count);
				} else {
					sclist->scalars=NULL;
				}					
				pptrs[j]=sclist;
				protoparams = params->count;
				term=1;
				
				DEBUG_DUMP_SCALARLIST(params, "param %d: varargs=", j);
				break;
			}								
			default:
				set_eval_error("internal error (%s:%d)", __FILE__, __LINE__);
				return 0;
		}
		if (term) break;
	}
	if (params->count == protoparams) {
		DEBUG_DUMP_INDENT_IN;
		retv=((int(*)(scalar_t*,void*,void*,void*,void*,void*,void*,void*,void*))proto->func)(r, pptrs[0], pptrs[1], pptrs[2], pptrs[3], pptrs[4], pptrs[5], pptrs[6], pptrs[7]);
		DEBUG_DUMP_INDENT_OUT;
	} else {
		retv=0;
	}		
	if (retv) {
		DEBUG_DUMP_SCALAR(r, "returns ");
	} else {
		DEBUG_DUMP("fails...");
	}

	if (sclist) {	
		scalarlist_destroy_gentle(sclist);
		free(sclist);
	}		
	
	for (j=0; j<MAX_EVALFUNC_PARAMS; j++) {
		if (sc[j].type!=SCALAR_NULL) {
			scalar_destroy(&sc[j]);
		}
	}
	
	if (!get_eval_error(NULL, NULL) && !retv) {
		set_eval_error("?");
	}
	
	if (get_eval_error(&errmsg, &errpos)) {
		char ee[MAX_ERRSTR_LEN+1];
		ee[MAX_ERRSTR_LEN]=0;
		strncpy(ee, proto->name, sizeof ee);
		strncat(ee, "(): ", sizeof ee);
		strncat(ee, errmsg, sizeof ee);
		set_eval_error_ex(errpos, "%s", ee);
	}
	return retv;
}

int evalsymbol(scalar_t *r, char *sname)
{
	int s=0;
	if (eval_symbol_handler) s=eval_symbol_handler(r, sname);
	if (!get_eval_error(NULL, NULL) && !s) {
		char sname_short[MAX_SYMBOLNAME_LEN+1];
		sname_short[MAX_SYMBOLNAME_LEN]=0;
		strncpy(sname_short, sname, MAX_SYMBOLNAME_LEN);
		set_eval_error("unknown symbol: %s", sname_short);
	}
	return s;
}

int std_eval_func_handler(scalar_t *r, char *fname, scalarlist_t *params, evalfunc_t *protos)
{
	char fname_short[MAX_FUNCNAME_LEN+1];
	
	fname_short[MAX_FUNCNAME_LEN]=0;
	strncpy(fname_short, fname, MAX_FUNCNAME_LEN);
	
	while (protos->name) {
		switch (match_evalfunc_proto(fname_short, params, protos)) {
			case PROTOMATCH_OK:
				return exec_evalfunc(r, params, protos);
			case PROTOMATCH_PARAM_FAIL:
				set_eval_error("invalid params to function %s", fname_short);
				return 0;
			default: {}
		}
		protos++;
	}
	return 0;
}

int evalfunc(scalar_t *r, char *fname, scalarlist_t *params)
{
	char fname_short[MAX_FUNCNAME_LEN+1];
	
	int s;
	if (eval_func_handler) {
		s=eval_func_handler(r, fname, params);
		if (get_eval_error(NULL, NULL)) return 0;
		if (s) return s;
	}
	
	s=std_eval_func_handler(r, fname, params, builtin_evalfuncs);
	if (get_eval_error(NULL, NULL)) return 0;
	if (s) return s;
	
	fname_short[MAX_FUNCNAME_LEN]=0;
	strncpy(fname_short, fname, MAX_FUNCNAME_LEN);
	
	set_eval_error("unknown function %s", fname_short);
	return 0;
}

void *eval_get_context()
{
	return eval_context;
}

void eval_set_context(void *context)
{
	eval_context=context;
}

void eval_set_func_handler(eval_func_handler_t func_handler)
{
	eval_func_handler=func_handler;
}

void eval_set_symbol_handler(eval_symbol_handler_t symbol_handler)
{
	eval_symbol_handler=symbol_handler;
}
