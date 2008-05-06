/*
 *	HT Editor
 *	eval.cc
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

#include "evaltype.h"
#include "evalparse.h"
#include "eval.h"
#include "snprintf.h"
#include "strtools.h"

#ifdef EVAL_DEBUG

int debug_dump_ident;

#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 *
 */

static eval_func_handler g_eval_func_handler;
static eval_symbol_handler g_eval_symbol_handler;
static void *eval_context;
static int helpmode = 0;
static eval_scalar helpstring;
static char helpname[MAX_FUNCNAME_LEN+1];

uint64 f2i(double f)
{
	uint64 r;
	if (f>0) r = (sint64)(f+.5); else r = (sint64)(f-.5);
	return r;
}

void set_helpmode(int flag, char *name)
{
	helpmode = flag;
	int l = name ? strlen(name) : (MAX_FUNCNAME_LEN+1);
	if (l>MAX_FUNCNAME_LEN) {
		*helpname = 0;
		return;
	}
	strcpy(helpname, name);
}

static uint64 ipow(uint64 a, uint64 b)
{
	uint64 r = 1ULL;
	uint64 m = 1ULL << 63;
	while (m != 0) {
		r *= r;
		if ((b & m) != 0) {
			r *= a;
		}
		m = m >> 1;
	}
	return r;
}

/*
static int sprint_basen(char *buffer, int base, uint64 q)
{
	static char *chars="0123456789abcdef";
	if ((base<2) || (base>16)) return 0;
	int n = 0;
	char *b = buffer;
	while (q != to_qword(0)) {
		int c = QWORD_GET_INT(q % to_qword(base));
		*buffer++ = chars[c];
		n++;
		q /= to_qword(base);
	}
	for (int i=0; i < n/2; i++) {
		char t = b[i];
		b[i] = b[n-i-1];
		b[n-i-1] = t;
	}
	b[n] = 0;
	return n;
}
*/

static void str2int(char *str, uint64 *q, int base)
{
	*q = 0;
	uint64 qbase = base;
	while (*str) {
		int c = hexdigit(*str);
		if ((c == -1) || (c >= base)) break;
		*q *= qbase;
		*q += c;
		str++;
	}
}

char *binstr2cstr(char *s, int len)
{
	char *x = ht_malloc(len+1);
	memcpy(x, s, len);
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

int get_eval_error(const char **str, int *pos)
{
	if (eval_error) {
		if (str) *str = eval_errstr;
		if (pos) *pos = eval_error_pos;
		return eval_error;
	}
	if (str) *str = "?";
	if (pos) *pos = 0;
	return 0;
}

void set_eval_error(const char *format,...)
{
	va_list vargs;
	
	va_start(vargs, format);
	ht_vsnprintf(eval_errstr, sizeof eval_errstr, format, vargs);
	va_end(vargs);
	eval_error_pos = lex_current_buffer_pos();
	eval_error = 1;
}

void set_eval_error_ex(int pos, const char *format, ...)
{
	va_list vargs;
	
	va_start(vargs, format);
	ht_vsnprintf(eval_errstr, sizeof eval_errstr, format, vargs);
	va_end(vargs);
	eval_error_pos = pos;
	eval_error = 1;
}

/*
 *
 */

#ifdef EVAL_DEBUG

void integer_dump(eval_int *i)
{
	printf("%d", i->value);
}

void float_dump(eval_float *f)
{
	printf("%f", f->value);
}

void string_dump(eval_str *s)
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

void string_destroy(eval_str *s)
{
	if (s->value) free(s->value);
}

/*
 *	SCALARLIST
 */

void scalarlist_set(eval_scalarlist *l, eval_scalar *s)
{
	l->count = 1;
	l->scalars = ht_malloc(sizeof (eval_scalar) * l->count);
	l->scalars[0] = *s;
}

void scalarlist_concat(eval_scalarlist *l, eval_scalarlist *a, eval_scalarlist *b)
{
	l->count = a->count+b->count;
	l->scalars = ht_malloc(sizeof (eval_scalar) * l->count);
	memcpy(l->scalars, a->scalars, sizeof (eval_scalar) * a->count);
	memcpy(l->scalars + a->count, b->scalars, sizeof (eval_scalar) * b->count);
}

void scalarlist_destroy(eval_scalarlist *l)
{
	int i;
	if (l && l->scalars) {
		for (i=0; i < l->count; i++) {
			scalar_destroy(&l->scalars[i]);
		}
		free(l->scalars);
	}		
}

void scalarlist_destroy_gentle(eval_scalarlist *l)
{
	if (l && l->scalars) free(l->scalars);
}

#ifdef EVAL_DEBUG

void scalarlist_dump(eval_scalarlist *l)
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

void scalar_setint(eval_scalar *s, eval_int *i)
{
	s->type=SCALAR_INT;
	s->scalar.integer=*i;
}

void scalar_setstr(eval_scalar *s, eval_str *t)
{
	s->type=SCALAR_STR;
	s->scalar.str=*t;
}

#ifdef EVAL_DEBUG

void scalar_dump(eval_scalar *s)
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

void scalar_create_int(eval_scalar *s, const eval_int *t)
{
	s->type = SCALAR_INT;
	s->scalar.integer=*t;
}

void scalar_create_int_c(eval_scalar *s, const int i)
{
	s->type = SCALAR_INT;
	s->scalar.integer.value = (sint64)i;
	s->scalar.integer.type = TYPE_UNKNOWN;
}

void scalar_create_int_q(eval_scalar *s, const uint64 q)
{
	s->type = SCALAR_INT;
	s->scalar.integer.value = q;
	s->scalar.integer.type = TYPE_UNKNOWN;
}

void scalar_create_str(eval_scalar *s, const eval_str *t)
{
	s->type = SCALAR_STR;
	s->scalar.str.value = ht_malloc(t->len ? t->len : 1);
	memcpy(s->scalar.str.value, t->value, t->len);
	s->scalar.str.len = t->len;
}

void scalar_create_str_c(eval_scalar *s, const char *cstr)
{
	eval_str t;
	t.value = (char*)cstr;
	t.len = strlen(cstr);
	scalar_create_str(s, &t);
}

void scalar_create_float(eval_scalar *s, const eval_float *t)
{
	s->type=SCALAR_FLOAT;
	s->scalar.floatnum=*t;
}

void scalar_create_float_c(eval_scalar *s, const double f)
{
	s->type=SCALAR_FLOAT;
	s->scalar.floatnum.value=f;
}

void scalar_context_str(const eval_scalar *s, eval_str *t)
{
	switch (s->type) {
		case SCALAR_INT: {
			char buf[64];
			ht_snprintf(buf, sizeof buf, "%qd",
				/* FIXME: by reference*/ &s->scalar.integer.value);
			t->value = (char*)strdup(buf);
			t->len = strlen(buf);
			break;
		}
		case SCALAR_STR: {
			t->value = ht_malloc(s->scalar.str.len ? s->scalar.str.len : 1);
			t->len = s->scalar.str.len;
			memcpy(t->value, s->scalar.str.value, t->len);
			break;
		}			
		case SCALAR_FLOAT: {
			char buf[64];
			ht_snprintf(buf, sizeof buf, "%f", s->scalar.floatnum.value);
			t->value = (char*)strdup(buf);
			t->len = strlen(buf);
			break;
		}
		default:
			break;
	}					
}

void scalar_context_int(const eval_scalar *s, eval_int *t)
{
	switch (s->type) {
		case SCALAR_INT: {
			*t=s->scalar.integer;
			break;
		}
		case SCALAR_STR: {
			char *x = binstr2cstr(s->scalar.str.value, s->scalar.str.len);
			str2int(x, &t->value, 10);
			t->type = TYPE_UNKNOWN;
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

void scalar_context_float(const eval_scalar *s, eval_float *t)
{
	switch (s->type) {
	case SCALAR_INT:
		t->value = s->scalar.integer.value;
		break;
	case SCALAR_STR:  {
		char *x = binstr2cstr(s->scalar.str.value, s->scalar.str.len);
		t->value = strtod(x, (char**)NULL);
		free(x);
		break;
	}			
	case SCALAR_FLOAT:
		*t = s->scalar.floatnum;
		break;
	default: 
		break;
	}					
}

void string_concat(eval_str *s, eval_str *a, eval_str *b)
{
	s->value = ht_malloc(a->len+b->len ? a->len+b->len : 1);
	memcpy(s->value, a->value, a->len);
	memcpy(s->value + a->len, b->value, b->len);
	s->len = a->len+b->len;
	
	free(a->value);
	a->len = 0;
	free(b->value);
	b->len = 0;
}

void scalar_clone(eval_scalar *result, const eval_scalar *s)
{
	switch (s->type) {
	case SCALAR_INT: {
		*result = *s;
		break;
	}
	case SCALAR_STR:  {
		*result = *s;
		result->scalar.str.value = ht_malloc(result->scalar.str.len);
		memcpy(result->scalar.str.value, s->scalar.str.value, result->scalar.str.len);
		break;
	}			
	case SCALAR_FLOAT: {
		*result = *s;
		break;
	}
	default:
		break;
	}
}

void scalar_concat(eval_scalar *s, const eval_scalar *a, const eval_scalar *b)
{
	eval_str as, bs, rs;
	scalar_context_str(a, &as);
	scalar_context_str(b, &bs);
	string_concat(&rs, &as, &bs);
	s->type=SCALAR_STR;
	s->scalar.str=rs;
}

void scalar_destroy(eval_scalar *s)
{
	switch (s->type) {
		case SCALAR_STR:
			string_destroy(&s->scalar.str);
			break;
		default:
			break;
	}
}

int string_compare(const eval_str *a, const eval_str *b)
{
	if (a->len==b->len) {
		return memcmp(a->value, b->value, a->len);
	}
	return a->len-b->len;
}

int scalar_strop(eval_scalar *xr, const eval_scalar *xa, const eval_scalar *xb, int op)
{
	eval_str as, bs;
	int r;
	int c;
	scalar_context_str(xa, &as);
	scalar_context_str(xb, &bs);
	
	c = string_compare(&as, &bs);

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
	xr->type = SCALAR_INT;
	xr->scalar.integer.value = r;
	xr->scalar.integer.type = TYPE_UNKNOWN;
	return 1;
}

int scalar_float_op(eval_scalar *xr, const eval_scalar *xa, const eval_scalar *xb, int op)
{
	eval_float ai, bi;
	float a, b, r;
	scalar_context_float(xa, &ai);
	scalar_context_float(xb, &bi);

	a = ai.value;
	b = bi.value;
	switch (op) {
		case '*': r = a*b; break;
		case '/': {
		    if (!b) {
			    set_eval_error("division by zero");
			    return 0;
		    }
		    r = a/b;
		    break;
		}			    
		case '+': r = a+b; break;
		case '-': r = a-b; break;
		case EVAL_POW: r = pow(a,b); break;
		default: {
			uint64 ri;
			switch (op) {
				case EVAL_EQ: ri = a==b; break;
				case EVAL_NE: ri = a!=b; break;
				case EVAL_GT: ri = a>b; break;
				case EVAL_GE: ri = a>=b; break;
				case EVAL_LT: ri = a<b; break;
				case EVAL_LE: ri = a<=b; break;
				case EVAL_LAND: ri = a && b; break;
				case EVAL_LXOR: ri = !a != !b; break;
				case EVAL_LOR: ri = a || b ; break;
				default:
					set_eval_error("invalid operator");
					return 0;
			}
			xr->type = SCALAR_INT;
			xr->scalar.integer.value = ri;
			xr->scalar.integer.type = TYPE_UNKNOWN;
			return 1;
		}
	}
	xr->type = SCALAR_FLOAT;
	xr->scalar.floatnum.value = r;
	return 1;
}

int scalar_int_op(eval_scalar *xr, const eval_scalar *xa, const eval_scalar *xb, int op)
{
	eval_int ai, bi;
	uint64 a, b, r;
	scalar_context_int(xa, &ai);
	scalar_context_int(xb, &bi);

	a = ai.value;
	b = bi.value;
	switch (op) {
		case '*': r = a*b; break;
		case '/': {
		    if (!b) {
			    set_eval_error("division by zero");
			    return 0;
		    }
		    r = a/b;
		    break;
		}			    
		case '%': {
		    if (!b) {
			    set_eval_error("division by zero");
			    return 0;
		    }
		    r = a%b;
		    break;
		}			    
		case '+': r=a+b; break;
		case '-': r=a-b; break;
		case '&': r=a&b; break;
		case '|': r=a|b; break;
		case '^': r=a^b; break;
		// FIXME
		case EVAL_POW: r = ipow(a, b); break;
//		case EVAL_POW: r = to_qword((int)pow(QWORD_GET_INT(a),QWORD_GET_INT(b))); break;
		case EVAL_SHL: r = a << b; break;
		case EVAL_SHR: r = a >> b; break;
		case EVAL_EQ: r = a==b; break;
		case EVAL_NE: r = a!=b; break;
		case EVAL_GT: r = a>b; break;
		case EVAL_GE: r = a>=b; break;
		case EVAL_LT: r = a<b; break;
		case EVAL_LE: r = a<=b; break;
		case EVAL_LAND: r = a && b; break;
		case EVAL_LXOR: r = !a != !b; break;
		case EVAL_LOR: r = a || b; break;
		default: 
			set_eval_error("invalid operator");
			return 0;
	}
	xr->type = SCALAR_INT;
	xr->scalar.integer.value = r;
	xr->scalar.integer.type = TYPE_UNKNOWN;
	return 1;
}

int scalar_op(eval_scalar *xr, eval_scalar *xa, eval_scalar *xb, int op)
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
	
void scalar_negset(eval_scalar *xr, eval_scalar *xa)
{
	if (xa->type==SCALAR_FLOAT) {
		eval_float a;
		a=xa->scalar.floatnum;
	
		xr->type=SCALAR_FLOAT;
		xr->scalar.floatnum.value=-a.value;
	} else {
		eval_int a;
		scalar_context_int(xa, &a);
	
		xr->type=SCALAR_INT;
		xr->scalar.integer.value=-a.value;
		xr->scalar.integer.type=TYPE_UNKNOWN;
	}
	scalar_destroy(xa);
}

void scalar_notset(eval_scalar *xr, eval_scalar *xa)
{
	eval_int a;
	scalar_context_int(xa, &a);

	xr->type=SCALAR_INT;
	xr->scalar.integer.value=~a.value;
	xr->scalar.integer.type=TYPE_UNKNOWN;

	scalar_destroy(xa);
}

void scalar_lnotset(eval_scalar *xr, eval_scalar *xa)
{
	eval_int a;
	scalar_context_int(xa, &a);

	xr->type = SCALAR_INT;
	xr->scalar.integer.value = !a.value;
	xr->scalar.integer.type = TYPE_UNKNOWN;

	scalar_destroy(xa);
}

void scalar_miniif(eval_scalar *xr, eval_scalar *xa, eval_scalar *xb, eval_scalar *xc)
{
	eval_int a;
	scalar_context_int(xa, &a);
	if (a.value != 0) {
		*xr = *xb;
	} else {
		*xr = *xc;
	}
	scalar_destroy(xa);
}

/*
 *	BUILTIN FUNCTIONS
 */

int func_typeof(eval_scalar *r, eval_scalar *s)
{
	switch (s->type) {
		case SCALAR_INT:
			scalar_create_str_c(r, "int");
			break;
		case SCALAR_STR:
			scalar_create_str_c(r, "string");
			break;
		case SCALAR_FLOAT:
			scalar_create_str_c(r, "float");
			break;
		default:
// FIXME: should never happen
			scalar_create_str_c(r, "unknown");
			break;
	}
	return 1;
}

int func_is_int(eval_scalar *r, eval_scalar *s)
{
	if (s->type == SCALAR_INT) {
		scalar_create_int_c(r, 1);
	} else {
		scalar_create_int_c(r, 0);
	}
	return 1;
}

int func_is_string(eval_scalar *r, eval_scalar *s)
{
	if (s->type == SCALAR_STR) {
		scalar_create_int_c(r, 1);
	} else {
		scalar_create_int_c(r, 0);
	}
	return 1;
}

int func_is_float(eval_scalar *r, eval_scalar *s)
{
	if (s->type == SCALAR_FLOAT) {
		scalar_create_int_c(r, 1);
	} else {
		scalar_create_int_c(r, 0);
	}
	return 1;
}

int func_char(eval_scalar *r, eval_int *i)
{
	eval_str s;
	char c = i->value;
	s.value = &c;
	s.len = 1;
	scalar_create_str(r, &s);
	return 1;
}

int func_byte(eval_scalar *r, eval_int *i)
{
	uint c = i->value;
	scalar_create_int_c(r, c & 0xff);
	return 1;
}

int func_word(eval_scalar *r, eval_int *i)
{
	uint c = i->value;
	scalar_create_int_c(r, c & 0xffff);
	return 1;
}

int func_dword(eval_scalar *r, eval_int *i)
{
	uint c = i->value;
	scalar_create_int_q(r, c & 0xffffffff);
	return 1;
}

int func_sbyte(eval_scalar *r, eval_int *i)
{
	uint c = i->value;
	scalar_create_int_c(r, (signed char)c);
	return 1;
}

int func_short(eval_scalar *r, eval_int *i)
{
	uint c = i->value;
	scalar_create_int_c(r, (short)c);
	return 1;
}

int func_long(eval_scalar *r, eval_int *i)
{
	int c = i->value;
	scalar_create_int_c(r, c);
	return 1;
}

int func_float(eval_scalar *r, eval_float *p)
{
	scalar_create_float(r, p);
	return 1;
}

int func_fmax(eval_scalar *r, eval_float *p1, eval_float *p2)
{
	r->type=SCALAR_FLOAT;
	r->scalar.floatnum.value=(p1->value>p2->value) ? p1->value : p2->value;
	return 1;
}

int func_fmin(eval_scalar *r, eval_float *p1, eval_float *p2)
{
	r->type=SCALAR_FLOAT;
	r->scalar.floatnum.value=(p1->value<p2->value) ? p1->value : p2->value;
	return 1;
}

int func_int(eval_scalar *r, eval_int *p)
{
	scalar_create_int(r, p);
	return 1;
}

int func_ord(eval_scalar *r, eval_str *s)
{
	if (s->len>=1) {
		scalar_create_int_c(r, s->value[0]);
		return 1;
	}
	set_eval_error("string must at least contain one character");
	return 0;		
}

int func_max(eval_scalar *r, eval_int *p1, eval_int *p2)
{
	scalar_create_int(r, (p1->value>p2->value) ? p1 : p2);
	return 1;
}

int func_min(eval_scalar *r, eval_int *p1, eval_int *p2)
{
	scalar_create_int(r, (p1->value<p2->value) ? p1 : p2);
	return 1;
}

int func_random(eval_scalar *r, eval_int *p1)
{
	uint64 d = rand();
	scalar_create_int_q(r, (p1->value != 0) ? (d % p1->value):0);
	return 1;
}

int func_rnd(eval_scalar *r)
{
	scalar_create_int_c(r, rand() % 2);
	return 1;
}

int func_round(eval_scalar *r, eval_float *p)
{
	r->type=SCALAR_INT;
	r->scalar.integer.value=f2i(p->value+0.5);
	r->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

int func_strchr(eval_scalar *r, eval_str *p1, eval_str *p2)
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

int func_strcmp(eval_scalar *r, eval_str *p1, eval_str *p2)
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

int func_string(eval_scalar *r, eval_str *p)
{
	scalar_create_str(r, p);
	return 1;
}

int func_strlen(eval_scalar *r, eval_str *p1)
{
	scalar_create_int_c(r, p1->len);
	return 1;
}

int func_strncmp(eval_scalar *r, eval_str *p1, eval_str *p2, eval_int *p3)
{
	return 1;
}

int func_strrchr(eval_scalar *r, eval_str *p1, eval_str *p2)
{
	return 1;
}

int func_strstr(eval_scalar *r, eval_str *p1, eval_str *p2)
{

	return 1;
}

int func_substr(eval_scalar *r, eval_str *p1, eval_int *p2, eval_int *p3)
{
	if (p2->value >= 0 && p3->value > 0) {
		if (p2->value < p1->len) {
			eval_str s;
			s.len = MIN(p3->value, p1->len - p2->value);
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

int func_trunc(eval_scalar *r, eval_float *p)
{
	r->type=SCALAR_INT;
	r->scalar.integer.value=f2i(p->value);
	r->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

#define EVALFUNC_FMATH1(name) int func_##name(eval_scalar *r, eval_float *p)\
{\
	r->type=SCALAR_FLOAT;\
	r->scalar.floatnum.value=name(p->value);\
	return 1;\
}

#define EVALFUNC_FMATH1i(name) int func_##name(eval_scalar *r, eval_float *p)\
{\
	r->type=SCALAR_INT;\
	r->scalar.integer.value=f2i(name(p->value));\
	r->scalar.integer.type=TYPE_UNKNOWN;\
	return 1;\
}

#define EVALFUNC_FMATH2(name) int func_##name(eval_scalar *r, eval_float *p1, eval_float *p2)\
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

void sprintf_puts(char **b, char *blimit, const char *buf)
{
	while (*b < blimit && *buf) {
		**b = *(buf++);
		(*b)++;
	}
}

int sprintf_percent(char **fmt, int *fmtl, char **b, char *blimit, eval_scalar *s)
{
	char cfmt[32];
	char buf[512];
	int ci=1;
	cfmt[0]='%';
	while (*fmtl && ci < 32-1) {
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
				eval_int i;
				scalar_context_int(s, &i);
				
				ht_snprintf(buf, sizeof buf, cfmt, i.value);
				sprintf_puts(b, blimit, buf);
				
				return 1;
			}
			case 's': {
				char *q=cfmt+1;
				eval_str t;
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
				
				ht_snprintf(buf, sizeof buf, cfmt, t.value);
				
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
				eval_float f;
				scalar_context_float(s, &f);
				
				ht_snprintf(buf, sizeof buf, cfmt, f.value);
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

int func_sprintf(eval_scalar *r, const eval_str *format, const eval_scalarlist *scalars)
{
	char buf[512];		/* FIXME: possible buffer overflow */
	char *b=buf;
	char *fmt;
	int fmtl;
	eval_scalar *s=scalars->scalars;
	
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
	r->scalar.str.value=ht_strdup(buf);
	r->scalar.str.len=strlen(r->scalar.str.value);
	return 1;
}

/*
 *	FUNCTIONS
 */

int func_eval(eval_scalar *r, eval_str *p)
{
	char *q = ht_malloc(p->len+1);
	int x;
	memcpy(q, p->value, p->len);
	q[p->len] = 0;
	x = eval(r, q, g_eval_func_handler, g_eval_symbol_handler, eval_context);
	free(q);
/*	if (get_eval_error(NULL, NULL)) {
		eval_error_pos+=lex_current_buffer_pos();
	}*/
	return x;
}

int func_error(eval_scalar *r, eval_str *s)
{
	char c[1024];
	bin2str(c, s->value, MIN((unsigned int)s->len, sizeof c));
	set_eval_error(c);
	return 0;
}

int func_help_helper(eval_scalar *r, eval_str *s)
{
	if (s->len > MAX_FUNCNAME_LEN) return 0;
	char b[MAX_FUNCNAME_LEN+1];
	bin2str(b, s->value, s->len);
	set_helpmode(1, b);
	eval_scalar str;
	scalar_create_str_c(&str, "NaF()");
	scalar_create_str_c(&helpstring, "");
	func_eval(r, &str.scalar.str);
	*r = helpstring;
	scalar_destroy(&str);
	set_helpmode(0, NULL);
	return 1;
}

int func_whatis(eval_scalar *r, eval_str *s)
{
	int t = 0;
	if (s->len != 0) t = func_help_helper(r, s);
	if ((s->len == 0) || (r->scalar.str.len == 0)) {
		char n[MAX_FUNCNAME_LEN+1];
		bin2str(n, s->value, s->len);
		set_eval_error("no such function \"%s\"", n);
		return 0;
	}
	return t;
}

int func_help(eval_scalar *r)
{
	eval_scalar str;
	scalar_create_str_c(&str, "");
	int t = func_help_helper(r, &str.scalar.str);
	scalar_destroy(&str);
	return t;
}

eval_func builtin_evalfuncs[]=	{
/* eval */
	{ "eval", (void*)&func_eval, {SCALAR_STR}, "evaluate string" },
/* help */
	{ "help", (void*)&func_help, {}, "get help on all functions"},
	{ "whatis", (void*)&func_whatis, {SCALAR_STR}, "get help on specific function"},
/* type juggling */
	{ "int", (void*)&func_int, {SCALAR_INT}, "converts to integer" },
	{ "string", (void*)&func_string, {SCALAR_STR}, "converts to string" },
	{ "float", (void*)&func_float, {SCALAR_FLOAT}, "converts to float" },
	{ "byte", (void*)&func_byte, {SCALAR_INT}, "converts to byte (1 bytes)" },
	{ "word", (void*)&func_word, {SCALAR_INT}, "converts to uint16 (2 bytes unsigned)" },
	{ "dword", (void*)&func_dword, {SCALAR_INT}, "converts to uint32 (4 bytes unsigned)" },
	{ "sbyte", (void*)&func_sbyte, {SCALAR_INT}, "converts to signed byte (1 bytes signed)" },
	{ "short", (void*)&func_short, {SCALAR_INT}, "converts to short (2 bytes signed)" },
	{ "long", (void*)&func_long, {SCALAR_INT}, "converts to long (4 bytes signed)" },
	{ "typeof", (void*)&func_typeof, {SCALAR_ANY}, "returns \"int\", \"string\" or \"float\"" },
	{ "is_int", (void*)&func_is_int, {SCALAR_ANY}, "returns non-zero if param is an integer" },
	{ "is_string", (void*)&func_is_string, {SCALAR_ANY}, "returns non-zero if param is a string" },
	{ "is_float", (void*)&func_is_float, {SCALAR_ANY}, "returns non-zero if param is a float" },

/* general */
	{ "error", (void*)&func_error, {SCALAR_STR}, "abort with error" },
/* string functions */
	{ "char", (void*)&func_char, {SCALAR_INT}, "return the ascii character (1-char string) specified by p1" },
	{ "ord", (void*)&func_ord, {SCALAR_STR}, "return the ordinal value of p1" },
	{ "sprintf", (void*)&func_sprintf, {SCALAR_STR, SCALAR_VARARGS}, "returns formatted string" },
	{ "strchr", (void*)&func_strchr, {SCALAR_STR, SCALAR_STR}, "returns position of first occurrence of character param2 in param1" },
	{ "strcmp", (void*)&func_strcmp, {SCALAR_STR, SCALAR_STR}, "returns zero for equality, positive number for str1 > str2 and negative number for str1 < str2" },
	{ "strlen", (void*)&func_strlen, {SCALAR_STR}, "returns length of string" },
	{ "strncmp", (void*)&func_strncmp, {SCALAR_STR, SCALAR_STR, SCALAR_INT}, "like strcmp, but considers a maximum of param3 characters" },
	{ "strstr", (void*)&func_strchr, {SCALAR_STR, SCALAR_STR}, "returns position of first occurrence of string param2 in param1" },
	{ "substr", (void*)&func_substr, {SCALAR_STR, SCALAR_INT, SCALAR_INT}, "returns substring from param1, start param2, length param3" },
/*	{ "stricmp", (void*)&func_stricmp, {SCALAR_STR, SCALAR_STR}, "like strcmp but case-insensitive" },
	{ "strnicmp", (void*)&func_strnicmp, {SCALAR_STR, SCALAR_STR}, "" }, */
/* math */	
	{ "pow", (void*)&func_pow, {SCALAR_FLOAT, SCALAR_FLOAT}, 0 },
	{ "sqrt", (void*)&func_sqrt, {SCALAR_FLOAT}, 0 },
	
	{ "fmin", (void*)&func_fmin, {SCALAR_FLOAT, SCALAR_FLOAT}, 0 },
	{ "fmax", (void*)&func_fmax, {SCALAR_FLOAT, SCALAR_FLOAT}, 0 },
	{ "min", (void*)&func_min, {SCALAR_INT, SCALAR_INT}, 0 },
	{ "max", (void*)&func_max, {SCALAR_INT, SCALAR_INT}, 0 },
	
	{ "random", (void*)&func_random, {SCALAR_INT}, "returns a random integer between 0 and param1-1" },
	{ "rnd", (void*)&func_rnd, {}, "returns a random bit as integer (0 or 1)" },

	{ "exp", (void*)&func_exp, {SCALAR_FLOAT}, 0 },
	{ "log", (void*)&func_log, {SCALAR_FLOAT}, 0 },
	
	{ "ceil", (void*)&func_ceil, {SCALAR_FLOAT}, 0 },
	{ "floor", (void*)&func_floor, {SCALAR_FLOAT}, 0 },
	{ "round", (void*)&func_round, {SCALAR_FLOAT}, 0 },
	{ "trunc", (void*)&func_trunc, {SCALAR_FLOAT}, 0 },
	
	{ "sin", (void*)&func_sin, {SCALAR_FLOAT}, 0 },
	{ "cos", (void*)&func_cos, {SCALAR_FLOAT}, 0 },
	{ "tan", (void*)&func_tan, {SCALAR_FLOAT}, 0 },
	
	{ "asin", (void*)&func_asin, {SCALAR_FLOAT}, 0 },
	{ "acos", (void*)&func_acos, {SCALAR_FLOAT}, 0 },
	{ "atan", (void*)&func_atan, {SCALAR_FLOAT}, 0 },
	
	{ "sinh", (void*)&func_sinh, {SCALAR_FLOAT}, 0 },
	{ "cosh", (void*)&func_cosh, {SCALAR_FLOAT}, 0 },
	{ "tanh", (void*)&func_tanh, {SCALAR_FLOAT}, 0 },
	
#ifdef HAVE_ASINH
	{ "asinh", (void*)&func_asinh, {SCALAR_FLOAT}, 0 },
#endif

#ifdef HAVE_ACOSH
	{ "acosh", (void*)&func_acosh, {SCALAR_FLOAT}, 0 },
#endif

#ifdef HAVE_ATANH
	{ "atanh", (void*)&func_atanh, {SCALAR_FLOAT}, 0 },
#endif
	
	{ NULL, NULL }
};

eval_protomatch match_evalfunc_proto(char *name, eval_scalarlist *params, eval_func *proto)
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

int exec_evalfunc(eval_scalar *r, eval_scalarlist *params, eval_func *proto)
{
	int j;
	int retv;
	eval_scalar sc[MAX_EVALFUNC_PARAMS];
	void *pptrs[MAX_EVALFUNC_PARAMS];
	int protoparams=0;
	eval_scalarlist *sclist=0;
	const char *errmsg;
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
			case SCALAR_ANY: {
				protoparams++;
				if (params->count<protoparams) return 0;
				scalar_clone(&sc[j], &params->scalars[j]);
				pptrs[j] = &sc[j];

				DEBUG_DUMP_SCALAR(&sc[j], "param %d: vararg=", j);
				break;
			}
			case SCALAR_VARARGS: {
				sclist = ht_malloc(sizeof (eval_scalarlist));
				sclist->count=params->count-j;
				if (sclist->count) {
					sclist->scalars=(eval_scalar*)malloc(sizeof (eval_scalar) * sclist->count);
					memcpy(sclist->scalars, &params->scalars[j], sizeof (eval_scalar) * sclist->count);
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
		retv=((int(*)(eval_scalar*,void*,void*,void*,void*,void*,void*,void*,void*))proto->func)(r, pptrs[0], pptrs[1], pptrs[2], pptrs[3], pptrs[4], pptrs[5], pptrs[6], pptrs[7]);
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
		ht_strlcpy(ee, proto->name, sizeof ee);
		ht_strlcat(ee, "(): ", sizeof ee);
		ht_strlcat(ee, errmsg, sizeof ee);
		set_eval_error_ex(errpos, "%s", ee);
	}
	return retv;
}

int evalsymbol(eval_scalar *r, char *sname)
{
	int s=0;
	if (g_eval_symbol_handler) s = g_eval_symbol_handler(r, sname);
	if (!get_eval_error(NULL, NULL) && !s) {
		char sname_short[MAX_SYMBOLNAME_LEN+1];
		ht_strlcpy(sname_short, sname, sizeof sname_short);
		set_eval_error("unknown symbol: %s", sname_short);
	}
	return s;
}

static const char *type2str(eval_scalartype t)
{
	switch (t) {
	case SCALAR_INT:
		return "INT";
		break;
	case SCALAR_STR:
		return "STRING";
		break;
	case SCALAR_FLOAT:
		return "FLOAT";
		break;
	case SCALAR_ANY:
		return "ANY";
		break;
	case SCALAR_VARARGS:
		return "...";
		break;
	default:
		return 0;
		break;
	}
}

static void proto_dump(char *buf, int bufsize, eval_func *proto, int full, const char *separator)
{
// FIXME: buffer safety/possible buffer overflow
	strcpy(buf, proto->name);
	strcat(buf, "(");
	for (int j=0; j<MAX_EVALFUNC_PARAMS; j++) {
		if (proto->ptype[j]==SCALAR_NULL) break;
		if (j) strcat(buf, ", ");
		strcat(buf, type2str(proto->ptype[j]));
		if (proto->ptype[j] == SCALAR_VARARGS) break;
	}
	strcat(buf, ")");
	if (full && proto->desc) {
		strcat(buf, separator);
		strcat(buf, proto->desc);
	}
}

int std_eval_func_handler(eval_scalar *result, char *fname, eval_scalarlist *params, eval_func *protos)
{
	if (strlen(fname) > MAX_FUNCNAME_LEN) {
		set_eval_error("invalid function, function name too long");
		return 0;
	}
	
	if (helpmode) {
		while (protos->name) {
			char buf[256];
			if ((!*helpname) || (*helpname && (strcmp(helpname, protos->name)==0))) {
				if (protos->func) {
					proto_dump(buf, sizeof buf, protos, 1, "\n");
				} else {
					strcpy(buf, protos->name);
					strcat(buf, " : ");
					strcat(buf, type2str(protos->ptype[0]));
					strcat(buf, "\n");
					strcat(buf, protos->desc);
				}
				strcat(buf, "\n\n");
				eval_scalar s;
				scalar_create_str_c(&s, buf);
				eval_scalar r;
				scalar_concat(&r, &helpstring, &s);
				scalar_destroy(&helpstring);
				scalar_destroy(&s);
				helpstring = r;
			}

			protos++;
		}
	} else {
		while (protos->name) {
			if (protos->func)
			switch (match_evalfunc_proto(fname, params, protos)) {
				case PROTOMATCH_OK:
					return exec_evalfunc(result, params, protos);
				case PROTOMATCH_PARAM_FAIL: {
					char b[256];
					proto_dump(b, sizeof b, protos, 0, "");
					set_eval_error("invalid params to function %s, declaration is: %s", protos->name, b);
					return 0;
				}
				default: {}
			}
			protos++;
		}
	}
	return 0;
}

int evalfunc(eval_scalar *r, char *fname, eval_scalarlist *params)
{
	if (strlen(fname) > MAX_FUNCNAME_LEN) {
		set_eval_error("invalid function, function name too long");
		return 0;
	}
	
	int s;
	if (g_eval_func_handler) {
		s = g_eval_func_handler(r, fname, params);
		if (get_eval_error(NULL, NULL)) return 0;
		if (s) return s;
	}
	
	s = std_eval_func_handler(r, fname, params, builtin_evalfuncs);
	if (get_eval_error(NULL, NULL)) return 0;
	if (s) return s;
	
	if (!helpmode) {
		set_eval_error("unknown function %s", fname);
		return 0;
	} else return 1;
}

void *eval_get_context()
{
	return eval_context;
}

void eval_set_context(void *context)
{
	eval_context = context;
}

void eval_set_func_handler(eval_func_handler func_handler)
{
	g_eval_func_handler = func_handler;
}

void eval_set_symbol_handler(eval_symbol_handler symbol_handler)
{
	g_eval_symbol_handler = symbol_handler;
}

