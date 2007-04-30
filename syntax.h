/*
 *	HT Editor
 *	syntax.h
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

#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include "data.h"
#include "htobj.h"

#define HT_HTML_SYNTAX_LEXER

struct text_pos {
	uint line;
	uint pofs;
};

typedef uint lexer_state;
typedef uint lexer_state_set;
typedef uint lexer_token;

enum lexer_rule_string_type {
	LRST_EMPTY,
	LRST_STRING,
	LRST_STRING_EXPECT,
	LRST_REGEX,
	LRST_CHARSET,
	LRST_WHITESPACE,
	LRST_QSTRING,			/* '[^']*' */
	LRST_DQSTRING,			/* "[^"]*" */
//     LRST_DQSTRING2,		/* "([^"]|\")*" */
	LRST_ANYCHAR,
	LRST_IDENTIFIER
};

struct syntax_lexer_rule {
	lexer_state_set needstate;
	bool need_line_start;
	lexer_rule_string_type string_type;
	const char *string;
	lexer_state state;
	lexer_token token;
};

/*
 *	CLASS ht_syntax_lexer
 */

class ht_syntax_lexer: public Object {
public:
/* new */
	virtual	void config_changed();
	virtual	vcp getcolor_syntax(uint pal_index)=0;
	virtual	lexer_state getinitstate()=0;
	virtual	lexer_token geterrortoken()=0;
	virtual	const char *getname()=0;
	virtual	lexer_token gettoken(void *buf, uint buflen, text_pos p, bool start_of_line, lexer_state *ret_state, uint *ret_len)=0;
	virtual	vcp gettoken_color(lexer_token t)=0;
};

/*
 *	CLASS ht_lang_syntax_lexer
 */

class ht_lang_syntax_lexer: public ht_syntax_lexer {
protected:
	syntax_lexer_rule *lexer_rules;
	void **lexer_rules_precompiled;
	int lexer_rules_count;

/* new */
			void free_lexer_rules();
			void set_lexer_rules(syntax_lexer_rule *lr);
public:
			void init(syntax_lexer_rule *lexer_rules);
	virtual	void done();
/* overwritten */
	virtual	lexer_token gettoken(void *buf, uint buflen, text_pos p, bool start_of_line, lexer_state *ret_state, uint *ret_len);
};

/*
 *	CLASS ht_c_syntax_lexer
 */

class ht_c_syntax_lexer: public ht_lang_syntax_lexer {
protected:
	const char **c_reserved_sorted;
	uint c_reserved_count;

	palette c_pal;
	
	virtual	void config_changed();
			void reloadpalette();
public:
			void init();
	virtual	void done();
/* overwritten */
	virtual	vcp getcolor_syntax(uint pal_index);
	virtual	lexer_state getinitstate();
	virtual	lexer_token geterrortoken();
	virtual	const char *getname();
	virtual	lexer_token gettoken(void *buf, uint buflen, text_pos p, bool start_of_line, lexer_state *ret_state, uint *ret_len);
	virtual	vcp gettoken_color(lexer_token t);
};

#ifdef HT_HTML_SYNTAX_LEXER
/*
 *	CLASS ht_html_syntax_lexer
 */

class ht_html_syntax_lexer: public ht_lang_syntax_lexer {
protected:
	char **html_reserved_sorted;
	uint html_reserved_count;

	palette html_pal;
	
	virtual	void config_changed();
			void reloadpalette();
public:
			void init();
	virtual	void done();
/* overwritten */
	virtual	vcp getcolor_syntax(uint pal_index);
	virtual	lexer_state getinitstate();
	virtual	lexer_token geterrortoken();
	virtual	const char *getname();
	virtual	lexer_token gettoken(void *buf, uint buflen, text_pos p, bool start_of_line, lexer_state *ret_state, uint *ret_len);
	virtual	vcp gettoken_color(lexer_token t);
};
#endif

const char **create_sorted_stringtable(const char **table);

/*
 *	syntax palette
 */

#define palkey_syntax_default						"c/default"

#define palidx_syntax_whitespace                       0
#define palidx_syntax_comment                          1
#define palidx_syntax_identifier                       2
#define palidx_syntax_reserved                         3
#define palidx_syntax_intnum                           4
#define palidx_syntax_floatnum                         5
#define palidx_syntax_string                           6
#define palidx_syntax_char                             7
#define palidx_syntax_symbol                           8
#define palidx_syntax_preprocess					9
#define palidx_syntax_meta                             10

#endif /* __SYNTAX_H__ */
