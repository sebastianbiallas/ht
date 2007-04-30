%{

#define USE_PURE_PARSER

#ifdef USE_PURE_PARSER
#define YY_DECL int yylex (YYSTYPE *lvalp)
#define YY_LVALP lvalp
#else
#define YY_LVALP (&yylval)
#endif

/* get current pos (offset from start of line) */
#define YY_USER_ACTION lex_pos=yy_bp - YY_CURRENT_BUFFER->yy_ch_buf;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "eval.h"
#include "evaltype.h"
#include "evalparse.h"

static int lex_pos;

static int char2int(char a, int base)
{
	int i;
	if ((a>='A') && (a<='Z')) {
		i=a-'A'+10;
	} else if ((a>='a') && (a<='z')) {
		i=a-'a'+10;
	} else if ((a>='0') && (a<='9')) {
		i=a-'0';
	} else return -1;
	if (i>=base) return -1;
	return i;
}

static int parse_float(eval_scalar *f, char *fpn)
{
	char *end;
	double d;
	d=strtod(fpn, &end);
	if (*end == 0) {
        	f->type=SCALAR_FLOAT;
        	f->scalar.floatnum.value=d;
		return 1;
	}
	return 0;
}

static int parse_integer(eval_scalar *i, char *num, int base, int lenmod)
{
	uint64 k = 0;
	int l = strlen(num) + lenmod;
	while (l--) {
		int c=char2int(*num, base);
		if (c==-1) return 0;
		k *= base;
		k += c;
		num++;
	}
	i->type=SCALAR_INT;
	i->scalar.integer.value=k;
	i->scalar.integer.type=TYPE_UNKNOWN;
	return 1;
}

static int parse_cstring(eval_scalar *r, char *s, int len)
{
	char *result;

	int alloclen = len;
	if (alloclen < 1) alloclen = 1;

	r->type = SCALAR_STR;
	r->scalar.str.value = (char*)malloc(alloclen);
	if (!r->scalar.str.value) return 0;

	result = r->scalar.str.value;

	// may not end with '\\'
	if (len && s[len-1] == '\\') return 0;

	while (s && *s && len) {
		if (*s == '\\') {
			s++;len--;if (!len) break;
			switch (*s) {
				case '0':
					*result++='\0';
					break;
				case 'a':
					*result++='\a';
					break;
				case 'b':
					*result++='\b';
					break;
				case 'e':
					*result++='\e';
					break;
				case 'f':
					*result++='\f';
					break;
				case 'n':
					*result++='\n';
					break;
				case 'r':
					*result++='\r';
					break;
				case 't':
					*result++='\t';
					break;
				case 'v':
					*result++='\v';
					break;
				case '\"':
					*result++='"';
					break;
				case '\\':
					*result++='\\';
					break;
				case 'x': {
					int p, q;
					s++;len--;if (!len) break;
					p=char2int(*s, 16);
					if (p==-1)  return 0;
					s++;len--;if (!len) break;
					q=char2int(*s, 16);
					if (q==-1) return 0;
					*result++=(char)p*16+q;
					break;
				}
				default:
					*result++='\\';
					if (len) *result++=*s;
					break;
			}
		} else {
			*result++ = *s;
		}
		s++;len--;
	}
	
	r->scalar.str.len=result-r->scalar.str.value;
	
	return 1;
}

static int parse_pstring(eval_scalar *s, char *cstr, int len)
{
	int alloclen=len;
	if (!len) alloclen=1;
	
	s->type=SCALAR_STR;
	s->scalar.str.value=(char*)malloc(alloclen);
	memcpy(s->scalar.str.value, cstr, len);
	s->scalar.str.len=len;
	return 1;
}

void *lex_current_buffer()
{
	return (void*)YY_CURRENT_BUFFER;
}

int lex_current_buffer_pos()
{
	return lex_pos;
}

void lex_switch_buffer(void *buffer)
{
	yy_switch_to_buffer(buffer);	
}

void lex_delete_buffer(void *buffer)
{
	yy_delete_buffer(buffer);	
}

void *lex_scan_string_buffer(const char *str)
{
	return yy_scan_string(str);
}

/*
*/
%}

%option noyywrap

%%

[ \t]+				/* nop */
\"(\\\"|[^"])*\"			if (parse_cstring(&YY_LVALP->scalar, yytext+1, strlen(yytext+1)-1)) return EVAL_STR;
'[^']*'				if (parse_pstring(&YY_LVALP->scalar, yytext+1, strlen(yytext+1)-1)) return EVAL_STR;
lt				return EVAL_STR_LT;
le				return EVAL_STR_LE;
gt				return EVAL_STR_GT;
ge				return EVAL_STR_GE;
eq				return EVAL_STR_EQ;
ne				return EVAL_STR_NE;
\*\*				return EVAL_POW;
\<\<				return EVAL_SHL;
\>\>				return EVAL_SHR;
\<				return EVAL_LT;
\<\=				return EVAL_LE;
\>				return EVAL_GT;
\>\=				return EVAL_GE;
\=\=				return EVAL_EQ;
\!\=				return EVAL_NE;
\&\&				return EVAL_LAND;
\|\|				return EVAL_LOR;
\^\^				return EVAL_LXOR;
[$@a-zA-Z_][a-zA-Z0-9_]*		YY_LVALP->ident=strdup(yytext); return EVAL_IDENT;
[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?	if (parse_float(&YY_LVALP->scalar, yytext)) return EVAL_FLOAT;
[0-9]+				if (parse_integer(&YY_LVALP->scalar, yytext, 10, 0)) return EVAL_INT;
0x[0-9a-fA-F]+			if (parse_integer(&YY_LVALP->scalar, yytext+2, 16, 0)) return EVAL_INT;
[0-9][0-9a-fA-F]*h		if (parse_integer(&YY_LVALP->scalar, yytext, 16, -1)) return EVAL_INT;
[0-9]+d				if (parse_integer(&YY_LVALP->scalar, yytext, 10, -1)) return EVAL_INT;
[0-7]+o				if (parse_integer(&YY_LVALP->scalar, yytext, 8, -1)) return EVAL_INT;
[0-1]+b				if (parse_integer(&YY_LVALP->scalar, yytext, 2, -1)) return EVAL_INT;
.				return *yytext;
\n				return '\n';

%%
