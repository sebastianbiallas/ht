/* 
 *	HT Editor
 *	syntax.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "htexcept.h"
extern "C" {
#include "regex.h"
}
#include "syntax.h"
#include "htpal.h"
#include "htstring.h"

bool regmatch(char *str, regex_t *preg, int *len)
{
// FIXME: use REG_STARTEND ? non-POSIX but could be pretty useful.
	regmatch_t m;
	int r=regexec(preg, str, 1, &m, 0);
	if ((r==0) && (m.rm_so==0)) {
		if (len) *len=m.rm_eo-m.rm_so;
		return true;
	}
	return false;
}

bool match_sorted_stringtable(char *s, UINT slen, char **strings, UINT strings_count)
{
	int a=0, b=strings_count-1;
	int m;
	while (a<=b) {
		m=(a+b)>>1;
		UINT x=strlen(strings[m]);
		if (slen>x) x=slen;
		int d=strncmp(s, strings[m], x);
		if (d<0) b=m-1; else
			if (d>0) a=m+1; else return true;
	}
	return false;
}

/*
 *	CLASS ht_syntax_lexer
 */

void ht_syntax_lexer::config_changed()
{
}

/*
 *	CLASS ht_lang_syntax_lexer
 */

#define LSTSET(state) (1<<(state))

#define SL_RULE_ANYCHAR(needstates, token)\
	{ needstates,\
	  false, LRST_ANYCHAR, NULL, 0, token }

#define SL_RULE_LINEEND(needstates, state)\
	{ needstates,\
	  false, LRST_REGEX, "$", state, 0 }

void ht_lang_syntax_lexer::init(syntax_lexer_rule *lr)
{
	ht_syntax_lexer::init();
	set_lexer_rules(lr);
}

void ht_lang_syntax_lexer::done()
{
	free_lexer_rules();
	ht_syntax_lexer::done();
}

void ht_lang_syntax_lexer::free_lexer_rules()
{
	for (int i=0; i<lexer_rules_count; i++) {
		if (lexer_rules[i].string_type==LRST_REGEX) {
			regfree((regex_t*)lexer_rules_precompiled[i]);
			free(lexer_rules_precompiled[i]);
		} else {
		}
	}
	free(lexer_rules_precompiled);
}


lexer_token ht_lang_syntax_lexer::gettoken(void *b, UINT buflen, text_pos p, bool start_of_line, lexer_state *ret_state, UINT *ret_len)
{
	syntax_lexer_rule *lr=lexer_rules;
	char *buf = (char*)b;
	int i=0;
	while (lr->needstate) {
		if ((lr->needstate & LSTSET(*ret_state)) &&
		(!lr->need_line_start || start_of_line)) {
			int l=0;
			bool matched=false;
			bool preserve_state=false;
			switch (lr->string_type) {
				case LRST_ANYCHAR: {
					if (buflen>0) l=1;
					break;
				}
				case LRST_WHITESPACE: {
//					if ((buflen>0) && ((byte)*buf<=32)) l=1;
					if ((buflen>0) && ((*buf==32) || (*buf=='\t'))) l=1;
					break;
				}
				case LRST_STRING: {
					UINT sl = strlen(lr->string);
					if ((buflen >= sl) && (memcmp(buf, lr->string, sl)==0)) {
						l = sl;
					}
					break;
				}
				case LRST_ISTRING: {
					UINT sl = strlen(lr->string);
					if (buflen >= sl && ht_memicmp(buf, lr->string, sl)==0) {
						l = sl;
					}
					break;
				}
				case LRST_STRING_EXPECT: {
/*                    	char *q = strstr(buf, lr->string);
					if (q) {
						l=q-buf+strlen(lr->string);
					} else {
						l = strlen(buf);
						preserve_state=true;
					}*/
					break;
				}
				case LRST_REGEX: {
#if 0
					if /*(*/(strcmp(lr->string, "$")==0)/* && (buflen>0))*/ {
						matched=true;
					} else if (regmatch(buf, (regex_t*)lexer_rules_precompiled[i], &l)) {
					}
#else
					if (strcmp(lr->string, "$") == 0) {
						matched = (buflen == 0);
					} else if (regmatch(buf, (regex_t*)lexer_rules_precompiled[i], &l)) {
					}
#endif
					break;
				}
				case LRST_CHARSET: {
					if (buflen) {
						if (*buf && (strchr(lr->string, *buf))) l=1;
					}
					break;
				}
				case LRST_IDENTIFIER: {
					if (isalpha(*buf) || *buf == '_') {
						char *b = buf+1;
						while (isalnum(*b) || *b == '_') b++;
						l = b-buf;
					}
					break;
				}
				case LRST_DQSTRING: {
					if (*buf == '"') {
						char *b = buf+1;
						while (*b && (*b != '"')) b++;
						if (*b == '"') l = b+1-buf;
					}
					break;
				}
				case LRST_QSTRING: {
					if (*buf == '\'') {
						char *b = buf+1;
						while (*b && (*b != '\'')) b++;
						if (*b == '\'') l = b+1-buf;
					}
					break;
				}
				case LRST_EMPTY:
					break;
			}
			if (matched || l) {
				if (lr->state && !preserve_state) *ret_state=lr->state;
				*ret_len=l;
				return lr->token;
			}
		}
		lr++;
		i++;
	}
/* error, no rule matched... */
	if (buflen) {
		*ret_len=1;
		return geterrortoken();
	}
	*ret_len=0;
	return 0;
}

void ht_lang_syntax_lexer::set_lexer_rules(syntax_lexer_rule *lr)
{
	lexer_rules=lr;
	
	lexer_rules_count=0;
	while (lexer_rules[lexer_rules_count].needstate) {
		lexer_rules_count++;
	}
	lexer_rules_precompiled=(void**)malloc(sizeof (void**) * lexer_rules_count);
	for (int i=0; i<lexer_rules_count; i++) {
		if (lexer_rules[i].string_type==LRST_REGEX) {
			regex_t *preg=(regex_t*)malloc(sizeof (regex_t));

			/* add an anchor in front of regex */
			int rl=strlen(lexer_rules[i].string)+1;
			char *regex=(char*)malloc(1+rl);
			*regex='^';
			memmove(regex+1, lexer_rules[i].string, rl);
			
			if (regcomp(preg, regex, REG_EXTENDED))
				throw ht_exception();
				
			free(regex);
			
			lexer_rules_precompiled[i]=preg;
		} else {
			lexer_rules_precompiled[i]=NULL;
		}
	}
}

/*
 *	CLASS ht_c_syntax_lexer
 */

/* C lexer states */
#define LEX_CST_NORMAL			1
#define LEX_CST_STRING			2
#define LEX_CST_PREPROCESS		3
#define LEX_CST_COMMENT			4
#define LEX_CST_COMMENT_EOL        5

/* C lexer tokens */
#define LEX_CTOK_ERROR			1
#define LEX_CTOK_WHITESPACE		2
#define LEX_CTOK_COMMENT			3
#define LEX_CTOK_PREPROCESS		4
#define LEX_CTOK_IDENTIFIER		5
#define LEX_CTOK_RIDENTIFIER		6
#define LEX_CTOK_NUMBER			7
#define LEX_CTOK_FNUMBER			8
#define LEX_CTOK_STRING			9
#define LEX_CTOK_CHAR			10
#define LEX_CTOK_SYMBOL			11

syntax_lexer_rule c_syntax_lexer_rules[]={
/* preprocessor directives */
	{ LSTSET(LEX_CST_NORMAL),
	  true, LRST_REGEX, " *#", LEX_CST_PREPROCESS, LEX_CTOK_PREPROCESS },
	SL_RULE_LINEEND(LSTSET(LEX_CST_PREPROCESS), LEX_CST_NORMAL),
	SL_RULE_ANYCHAR(LSTSET(LEX_CST_PREPROCESS), LEX_CTOK_PREPROCESS),
/* whitespaces */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_WHITESPACE, NULL, 0, LEX_CTOK_WHITESPACE },
/* '/ *' - '* /' multiline comments */
	{ LSTSET(LEX_CST_NORMAL) | LSTSET(LEX_CST_PREPROCESS),
	  false, LRST_STRING, "/*", LEX_CST_COMMENT, LEX_CTOK_COMMENT },
	{ LSTSET(LEX_CST_COMMENT),
	  false, LRST_STRING, "*/", LEX_CST_NORMAL, LEX_CTOK_COMMENT },
	SL_RULE_ANYCHAR(LSTSET(LEX_CST_COMMENT), LEX_CTOK_COMMENT),
/* "..." (multiline) strings */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_STRING, "\"", LEX_CST_STRING, LEX_CTOK_STRING },
	{ LSTSET(LEX_CST_STRING),
	  false, LRST_STRING, "\\\"", LEX_CST_STRING, LEX_CTOK_STRING },
	{ LSTSET(LEX_CST_STRING),
	  false, LRST_STRING, "\"", LEX_CST_NORMAL, LEX_CTOK_STRING },
	SL_RULE_ANYCHAR(LSTSET(LEX_CST_STRING), LEX_CTOK_STRING),
/* '//' one line comments */
	{ LSTSET(LEX_CST_NORMAL) | LSTSET(LEX_CST_PREPROCESS),
	  false, LRST_STRING, "//", LEX_CST_COMMENT_EOL, LEX_CTOK_COMMENT },
	SL_RULE_LINEEND(LSTSET(LEX_CST_COMMENT_EOL), LEX_CST_NORMAL),
	SL_RULE_ANYCHAR(LSTSET(LEX_CST_COMMENT_EOL), LEX_CTOK_COMMENT),
/* symbols */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_CHARSET, "(){};,.[]!~%+-/*=<>|&^?:", 0, LEX_CTOK_SYMBOL },
/* identifiers */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_IDENTIFIER, NULL, 0, LEX_CTOK_IDENTIFIER },
/* floats */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "[0-9]+\\.[0-9]+(e[+-]?[0-9]+)?", 0, LEX_CTOK_FNUMBER },
/* numbers */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "0[xX][0-9a-fA-F]+", 0, LEX_CTOK_NUMBER },
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "[0-9]+", 0, LEX_CTOK_NUMBER },
/* chars */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "'[^'\\]'", 0, LEX_CTOK_CHAR },
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "'\\\\.{1,3}'", 0, LEX_CTOK_CHAR },
/**/
	{ 0, 0, LRST_EMPTY, false, false, 0 }
};

static char *c_reserved[]=
{
/* types */
	"bool", "char", "void", "int", "short", "long",
	"unsigned", "signed", "float", "double",
/* consts */
	"true", "false",
/* statements */
	"return", "if", "else", "while", "do", "goto", "asm",
	"switch", "case", "default", "break", "continue", "for",
/* object */
	"new", "delete", "this",
/* declarations */
	"struct", "union", "enum", "class", "template", "operator",
	"typedef",
/* modifiers */
	"public", "protected", "private", "friend", "const",
	"extern", "inline", "register", "static", "volatile", "virtual",
/* exceptions */
	"try", "catch", "throw",
/* misc */
	"sizeof",
/**/
	NULL
};

#define palkey_syntax_c_default		"c/default"

void ht_c_syntax_lexer::init()
{
	ht_lang_syntax_lexer::init(c_syntax_lexer_rules);
	c_reserved_sorted=create_sorted_stringtable(c_reserved);

	char **table=c_reserved;
	
	char **x=table;
	while (*x) x++;
	c_reserved_count=x-table;

	c_pal.data = NULL;
	c_pal.size = 0;

	config_changed();
}

void ht_c_syntax_lexer::done()
{
	free(c_pal.data);
	free(c_reserved_sorted);
	ht_lang_syntax_lexer::done();
}

void ht_c_syntax_lexer::config_changed()
{
	reloadpalette();
}

vcp ht_c_syntax_lexer::getcolor_syntax(UINT pal_index)
{
	return getcolorv(&c_pal, pal_index);
}

lexer_token ht_c_syntax_lexer::geterrortoken()
{
	return LEX_CTOK_ERROR;
}

lexer_state ht_c_syntax_lexer::getinitstate()
{
	return LEX_CST_NORMAL;
}

char *ht_c_syntax_lexer::getname()
{
	return "C/C++";
}

lexer_token ht_c_syntax_lexer::gettoken(void *buf, UINT buflen, text_pos p, bool start_of_line, lexer_state *ret_state, UINT *ret_len)
{
	lexer_token t=ht_lang_syntax_lexer::gettoken(buf, buflen, p, start_of_line, ret_state, ret_len);
	if (t==LEX_CTOK_IDENTIFIER) {
		if (match_sorted_stringtable((char*)buf, *ret_len, c_reserved_sorted, c_reserved_count)) {
			t=LEX_CTOK_RIDENTIFIER;
		}
	}
	return t;
}

vcp ht_c_syntax_lexer::gettoken_color(lexer_token t)
{
	switch (t) {
		case LEX_CTOK_WHITESPACE: return getcolor_syntax(palidx_syntax_whitespace);
		case LEX_CTOK_COMMENT: return getcolor_syntax(palidx_syntax_comment);
		case LEX_CTOK_PREPROCESS: return getcolor_syntax(palidx_syntax_preprocess);
		case LEX_CTOK_IDENTIFIER: return getcolor_syntax(palidx_syntax_identifier);
		case LEX_CTOK_RIDENTIFIER: return getcolor_syntax(palidx_syntax_reserved);
		case LEX_CTOK_NUMBER: return getcolor_syntax(palidx_syntax_intnum);
		case LEX_CTOK_FNUMBER: return getcolor_syntax(palidx_syntax_floatnum);
		case LEX_CTOK_STRING: return getcolor_syntax(palidx_syntax_string);
		case LEX_CTOK_CHAR: return getcolor_syntax(palidx_syntax_char);
		case LEX_CTOK_SYMBOL: return getcolor_syntax(palidx_syntax_symbol);
	}
	return VCP(VC_BLACK, VC_RED);
}

void ht_c_syntax_lexer::reloadpalette()
{
	free(c_pal.data);
	c_pal.data = NULL;
	load_pal(palclasskey_syntax, palkey_syntax_c_default, &c_pal);
}

/*
 *	CLASS ht_pascal_syntax_lexer
 */

/* Pascal lexer states */
#define LEX_PASCALST_NORMAL			1
#define LEX_PASCALST_COMMENT			3
#define LEX_PASCALST_COMMENT_EOL        	4
#define LEX_PASCALST_ASM	        	5

/* Pascal lexer tokens */
#define LEX_PASCALTOK_ERROR			1
#define LEX_PASCALTOK_WHITESPACE		2
#define LEX_PASCALTOK_COMMENT			3
#define LEX_PASCALTOK_IDENTIFIER		5
#define LEX_PASCALTOK_RIDENTIFIER		6
#define LEX_PASCALTOK_NUMBER			7
#define LEX_PASCALTOK_FNUMBER			8
#define LEX_PASCALTOK_STRING			9
#define LEX_PASCALTOK_CHAR			10
#define LEX_PASCALTOK_SYMBOL			11

syntax_lexer_rule pascal_syntax_lexer_rules[]={
	/* asm */
	{ LSTSET(LEX_PASCALST_NORMAL),
	  false, LRST_STRING, "asm", 0, LEX_CTOK_WHITESPACE },
	/* whitespaces */
	{ LSTSET(LEX_PASCALST_NORMAL) | LSTSET(LEX_PASCALST_ASM),
	  false, LRST_WHITESPACE, NULL, 0, LEX_CTOK_WHITESPACE },
	/* {} multiline comments */
	{ LSTSET(LEX_PASCALST_NORMAL) | LSTSET(LEX_PASCALST_ASM),
	  false, LRST_STRING, "{", LEX_PASCALST_COMMENT, LEX_CTOK_COMMENT },
	{ LSTSET(LEX_PASCALST_COMMENT),
	  false, LRST_STRING, "}", LEX_PASCALST_NORMAL, LEX_CTOK_COMMENT },
	SL_RULE_ANYCHAR(LSTSET(LEX_PASCALST_COMMENT), LEX_CTOK_COMMENT),
	/* "" strings */
	{ LSTSET(LEX_PASCALST_NORMAL),
	  false, LRST_REGEX, "'.*'[^']", 0, LEX_CTOK_STRING },
//	{ LSTSET(LEX_PASCALST_NORMAL),
	/* '//' one line comments */
	{ LSTSET(LEX_PASCALST_NORMAL) | LSTSET(LEX_PASCALST_ASM),
	  false, LRST_STRING, "//", LEX_PASCALST_COMMENT_EOL, LEX_CTOK_COMMENT },
	SL_RULE_LINEEND(LSTSET(LEX_PASCALST_COMMENT_EOL), LEX_PASCALST_NORMAL),
	SL_RULE_ANYCHAR(LSTSET(LEX_PASCALST_COMMENT_EOL), LEX_CTOK_COMMENT),
	/* symbols */
	{ LSTSET(LEX_PASCALST_NORMAL),
	  false, LRST_CHARSET, "(){};,.[]!~%+-/*=<>|&^?:", 0, LEX_CTOK_SYMBOL },
/* identifiers */
	{ LSTSET(LEX_PASCALST_NORMAL),
	  false, LRST_IDENTIFIER, NULL, 0, LEX_CTOK_IDENTIFIER },
/* floats */
	{ LSTSET(LEX_PASCALST_NORMAL),
	  false, LRST_REGEX, "[0-9]+\\.[0-9]+(e[+-]?[0-9]+)?", 0, LEX_CTOK_FNUMBER },
/* numbers */
	{ LSTSET(LEX_PASCALST_NORMAL),
	  false, LRST_REGEX, "0[xX][0-9a-fA-F]+", 0, LEX_CTOK_NUMBER },
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "[0-9]+", 0, LEX_CTOK_NUMBER },
/* chars */
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "'[^'\\]'", 0, LEX_CTOK_CHAR },
	{ LSTSET(LEX_CST_NORMAL),
	  false, LRST_REGEX, "'\\\\.{1,3}'", 0, LEX_CTOK_CHAR },
/**/
	{ 0, 0, LRST_EMPTY, false, false, 0 }
};

static char *pascal_reserved[]=
{
/* Directives */
	"Abstract",
"Default",
"Dynamic",
"Export",
"Index",
"Out",
"Overload",
"Override",
"Private",
"Protected",
"Public",
"Published",
"Virtual",

/* Constants: */
"Infinity"
"MaxInt"
"MaxLongInt"
"MinsPerDay"
"MonthDays"
"NaN"
"Nil"
"SecsPerDay"
"VarTypeMask"

/* Keywords: */

"Absolute",
"And",
"Array",
"As",
"Asm",
"Assembler",
"Begin",
"Case",
"Class",
"Const",
"Constructor",
"Destructor",
"Div",
"Do",
"DownTo",
"Else",
"End",
"Except",
"Exports",
"External",
"Far",
"File",
"Finally"
"For",
"Forward",
"Function",
"Goto",
"If",
"Implementation",
"In",
"Inherited",
"Inline",
"Interface",
"Interrupt",
"Is",
"Label",
"Library",
"Mod",
"Name",
"Near",
"Not",
"Object",
"Of",
"On",
"Or",
"Packed",
"Procedure",
"Program",
"Property",
"Raise",
"Record",
"Repeat",
"Resident",
"Set",
"Shl",
"Shr",
"String",
"Then",
"ThreadVar",
"To",
"Try",
"Type",
"Unit",
"Until",
"Uses",
"Var",
"Virtual",
"While",
"With",
"Xor",
	NULL
};

#define palkey_syntax_c_default		"c/default"

void ht_pascal_syntax_lexer::init()
{
	ht_lang_syntax_lexer::init(c_syntax_lexer_rules);
	pascal_reserved_sorted=create_sorted_stringtable(pascal_reserved);

	char **table=c_reserved;
	
	char **x=table;
	while (*x) x++;
	pascal_reserved_count=x-table;

	c_pal.data = NULL;
	c_pal.size = 0;

	config_changed();
}

void ht_pascal_syntax_lexer::done()
{
	free(c_pal.data);
	free(pascal_reserved_sorted);
	ht_lang_syntax_lexer::done();
}

void ht_pascal_syntax_lexer::config_changed()
{
	reloadpalette();
}

vcp ht_pascal_syntax_lexer::getcolor_syntax(UINT pal_index)
{
	return getcolorv(&c_pal, pal_index);
}

lexer_token ht_pascal_syntax_lexer::geterrortoken()
{
	return LEX_PASCALTOK_ERROR;
}

lexer_state ht_pascal_syntax_lexer::getinitstate()
{
	return LEX_PASCALST_NORMAL;
}

char *ht_pascal_syntax_lexer::getname()
{
	return "Pascal/Delphi";
}

lexer_token ht_pascal_syntax_lexer::gettoken(void *buf, UINT buflen, text_pos p, bool start_of_line, lexer_state *ret_state, UINT *ret_len)
{
	lexer_token t=ht_lang_syntax_lexer::gettoken(buf, buflen, p, start_of_line, ret_state, ret_len);
	if (t==LEX_PASCALTOK_IDENTIFIER) {
		if (match_sorted_stringtable((char*)buf, *ret_len, pascal_reserved_sorted, pascal_reserved_count)) {
			t=LEX_PASCALTOK_RIDENTIFIER;
		}
	}
	return t;
}

vcp ht_pascal_syntax_lexer::gettoken_color(lexer_token t)
{
	switch (t) {
		case LEX_PASCALTOK_WHITESPACE: return getcolor_syntax(palidx_syntax_whitespace);
		case LEX_PASCALTOK_COMMENT: return getcolor_syntax(palidx_syntax_comment);
//		case LEX_PASCALTOK_PREPROCESS: return getcolor_syntax(palidx_syntax_preprocess);
		case LEX_PASCALTOK_IDENTIFIER: return getcolor_syntax(palidx_syntax_identifier);
		case LEX_PASCALTOK_RIDENTIFIER: return getcolor_syntax(palidx_syntax_reserved);
		case LEX_PASCALTOK_NUMBER: return getcolor_syntax(palidx_syntax_intnum);
		case LEX_PASCALTOK_FNUMBER: return getcolor_syntax(palidx_syntax_floatnum);
		case LEX_PASCALTOK_STRING: return getcolor_syntax(palidx_syntax_string);
		case LEX_PASCALTOK_CHAR: return getcolor_syntax(palidx_syntax_char);
		case LEX_PASCALTOK_SYMBOL: return getcolor_syntax(palidx_syntax_symbol);
	}
	return VCP(VC_BLACK, VC_RED);
}

void ht_pascal_syntax_lexer::reloadpalette()
{
	free(c_pal.data);
	c_pal.data = NULL;
	load_pal(palclasskey_syntax, palkey_syntax_c_default, &c_pal);
}

#ifdef HT_HTML_SYNTAX_LEXER
/*
 *	CLASS ht_html_syntax_lexer
 */

#if 0
/* HTML lexer states */
#define LEX_HTMLST_NORMAL			1
#define LEX_HTMLST_TAG				2
#define LEX_HTMLST_COMMENT			3
#define LEX_HTMLST_CSS				4
#define LEX_HTMLST_SCRIPT			5

/* HTML lexer tokens */
#define LEX_HTMLTOK_ERROR			1
#define LEX_HTMLTOK_WHITESPACE		2
#define LEX_HTMLTOK_NORMAL			3
#define LEX_HTMLTOK_COMMENT			4
#define LEX_HTMLTOK_TAG				5
#define LEX_HTMLTOK_ATTRIBUTE			6
#define LEX_HTMLTOK_SYMBOL			7
#define LEX_HTMLTOK_CDATA			8
#define LEX_HTMLTOK_ENTITY			9

syntax_lexer_rule html_syntax_lexer_rules[] = {
/* whitespaces */
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_WHITESPACE, NULL, 0, LEX_CTOK_WHITESPACE },
/* '<!--' - '-->' multiline comments */
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_STRING, "<!--", LEX_HTMLST_COMMENT, LEX_HTMLTOK_COMMENT },
	{ LSTSET(LEX_HTMLST_COMMENT),
	  false, LRST_STRING, "-->", LEX_HTMLST_NORMAL, LEX_HTMLTOK_COMMENT },
	SL_RULE_ANYCHAR(LSTSET(LEX_HTMLST_COMMENT), LEX_HTMLTOK_COMMENT),
/* '<' - '>' tags */
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_STRING, "<!", LEX_HTMLST_TAG, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_REGEX, "</[-_A-Za-z0-9]+", LEX_HTMLST_TAG, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_REGEX, "<[-_A-Za-z0-9]+", LEX_HTMLST_TAG, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_STRING, ">", LEX_HTMLST_NORMAL, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_STRING, "/>", LEX_HTMLST_NORMAL, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_WHITESPACE, NULL, 0, LEX_HTMLTOK_TAG },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_REGEX, "[-_A-Za-z0-9]+", 0, LEX_HTMLTOK_ATTRIBUTE },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_CHARSET, "=", 0, LEX_HTMLTOK_SYMBOL },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_DQSTRING, NULL, 0, LEX_HTMLTOK_CDATA },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_QSTRING, NULL, 0, LEX_HTMLTOK_CDATA },
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_REGEX, "&[#A-Za-z0-9]+?;", 0, LEX_HTMLTOK_ENTITY },
	SL_RULE_ANYCHAR(LSTSET(LEX_HTMLST_NORMAL), LEX_HTMLTOK_NORMAL),
/**/
	{ 0, 0, LRST_EMPTY, false, false, 0 }
};
#else

/* HTML lexer states */
#define LEX_HTMLST_NORMAL			1
#define LEX_HTMLST_TAG_OPEN			2
#define LEX_HTMLST_TAG				3
#define LEX_HTMLST_COMMENT			4
#define LEX_HTMLST_CSS				5
#define LEX_HTMLST_SCRIPT			6

/* HTML lexer tokens */
#define LEX_HTMLTOK_ERROR			1
#define LEX_HTMLTOK_WHITESPACE		2
#define LEX_HTMLTOK_NORMAL			3
#define LEX_HTMLTOK_COMMENT			4
#define LEX_HTMLTOK_TAG				5
#define LEX_HTMLTOK_ATTRIBUTE			6
#define LEX_HTMLTOK_SYMBOL			7
#define LEX_HTMLTOK_CDATA			8
#define LEX_HTMLTOK_ENTITY			9

syntax_lexer_rule html_syntax_lexer_rules[] = {
/* whitespaces */
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_WHITESPACE, NULL, 0, LEX_CTOK_WHITESPACE },
/* '<!--' - '-->' multiline comments */
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_STRING, "<!--", LEX_HTMLST_COMMENT, LEX_HTMLTOK_COMMENT },
	{ LSTSET(LEX_HTMLST_COMMENT),
	  false, LRST_STRING, "-->", LEX_HTMLST_NORMAL, LEX_HTMLTOK_COMMENT },
	SL_RULE_ANYCHAR(LSTSET(LEX_HTMLST_COMMENT), LEX_HTMLTOK_COMMENT),
/* '<' - '>' tags */
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_STRING, "<", LEX_HTMLST_TAG_OPEN, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG_OPEN),
	  false, LRST_STRING, "/", LEX_HTMLST_TAG_OPEN, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG_OPEN),
	  false, LRST_STRING, "!", LEX_HTMLST_TAG, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG_OPEN),
	  false, LRST_REGEX, "[-_A-Za-z0-9]+", LEX_HTMLST_TAG, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_STRING, ">", LEX_HTMLST_NORMAL, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_STRING, "/>", LEX_HTMLST_NORMAL, LEX_HTMLTOK_TAG},
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_WHITESPACE, NULL, 0, LEX_HTMLTOK_TAG },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_REGEX, "[-_A-Za-z0-9]+", 0, LEX_HTMLTOK_ATTRIBUTE },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_CHARSET, "=", 0, LEX_HTMLTOK_SYMBOL },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_DQSTRING, NULL, 0, LEX_HTMLTOK_CDATA },
	{ LSTSET(LEX_HTMLST_TAG),
	  false, LRST_QSTRING, NULL, 0, LEX_HTMLTOK_CDATA },
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_REGEX, "[^&<]", 0, LEX_HTMLTOK_NORMAL },
	{ LSTSET(LEX_HTMLST_NORMAL),
	  false, LRST_REGEX, "&[#A-Za-z0-9]+?;", 0, LEX_HTMLTOK_ENTITY },
//	SL_RULE_ANYCHAR(LSTSET(LEX_HTMLST_NORMAL), LEX_HTMLTOK_NORMAL),
/**/
	{ 0, 0, LRST_EMPTY, false, false, 0 }
};
#endif

void ht_html_syntax_lexer::init()
{
	ht_lang_syntax_lexer::init(html_syntax_lexer_rules);
/*	html_reserved_sorted=create_sorted_stringtable(html_reserved);
	char **table=c_reserved;
	
	char **x=table;
	while (*x) x++;
	c_reserved_count=x-table;

	c_pal.data = NULL;
	c_pal.size = 0;*/

	config_changed();
}

void ht_html_syntax_lexer::done()
{
//	free(c_pal.data);
//	free(c_reserved_sorted);
	ht_lang_syntax_lexer::done();
}

void ht_html_syntax_lexer::config_changed()
{
	reloadpalette();
}

vcp ht_html_syntax_lexer::getcolor_syntax(UINT pal_index)
{
//	return getcolorv(&c_pal, pal_index);
	return VCP(VC_LIGHT(VC_BLUE), VC_TRANSPARENT);
}

lexer_token ht_html_syntax_lexer::geterrortoken()
{
	return LEX_HTMLTOK_ERROR;
}

lexer_state ht_html_syntax_lexer::getinitstate()
{
	return LEX_HTMLST_NORMAL;
}

char *ht_html_syntax_lexer::getname()
{
	return "HTML";
}

lexer_token ht_html_syntax_lexer::gettoken(void *buf, UINT buflen, text_pos p, bool start_of_line, lexer_state *ret_state, UINT *ret_len)
{
	lexer_token t=ht_lang_syntax_lexer::gettoken(buf, buflen, p, start_of_line, ret_state, ret_len);
/*	if (t==LEX_CTOK_IDENTIFIER) {
		if (match_sorted_stringtable((char*)buf, *ret_len, c_reserved_sorted, c_reserved_count)) {
			t=LEX_CTOK_RIDENTIFIER;
		}
	}*/
	return t;
}

vcp ht_html_syntax_lexer::gettoken_color(lexer_token t)
{
	switch (t) {
		case LEX_HTMLTOK_ERROR: return VCP(VC_BLACK, VC_RED);
		case LEX_HTMLTOK_WHITESPACE: return VCP(VC_LIGHT(VC_YELLOW), VC_TRANSPARENT);
		case LEX_HTMLTOK_NORMAL: return VCP(VC_LIGHT(VC_YELLOW), VC_TRANSPARENT);
		case LEX_HTMLTOK_COMMENT: return VCP(VC_WHITE, VC_TRANSPARENT);
		case LEX_HTMLTOK_TAG: return VCP(VC_LIGHT(VC_GREEN), VC_TRANSPARENT);
		case LEX_HTMLTOK_ATTRIBUTE: return VCP(VC_GREEN, VC_TRANSPARENT);
		case LEX_HTMLTOK_SYMBOL: return VCP(VC_LIGHT(VC_WHITE), VC_TRANSPARENT);
		case LEX_HTMLTOK_CDATA: return VCP(VC_LIGHT(VC_MAGENTA), VC_TRANSPARENT);
		case LEX_HTMLTOK_ENTITY: return VCP(VC_LIGHT(VC_WHITE), VC_TRANSPARENT);
	}
	return VCP(VC_BLACK, VC_RED);
}

void ht_html_syntax_lexer::reloadpalette()
{
/*	if (c_pal.data) {
	    free(c_pal.data);
	    c_pal.data = NULL;
	}	    
	load_pal(palclasskey_syntax, palkey_syntax_c_default, &c_pal);*/
}
#endif

/*
 *	sorted stringtable
 */

int qsort_stringlist(const void *e1, const void *e2)
{
	return strcmp(*(char **)e1, *(char **)e2);
}
	
char **create_sorted_stringtable(char **table)
{
	char **x=table;
	while (*x) x++;
	char **stab=(char **)malloc(sizeof (char*) * (x-table+1));
	memmove(stab, table, sizeof (char*) * (x-table+1));
	
	qsort(stab, x-table, sizeof(char*), qsort_stringlist);
	return stab;
}

