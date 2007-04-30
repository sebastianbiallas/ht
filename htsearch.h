/* 
 *	HT Editor
 *	htsearch.h
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

#ifndef __HTSEARCH_H__
#define __HTSEARCH_H__

#include "atom.h"
#include "htdialog.h"
#include "except.h"
#include "htformat.h"
#include "htobj.h"
#include "strtools.h"
#include "stream.h"

#include <sys/types.h>
extern "C" {
#include "regex.h"
}

/*
 *	searching
 */

// search types + flags
#define ST_FXBIN		0		// search using fixed binary pattern
	#define SF_FXBIN_CASEINSENSITIVE		1
	#define SF_FXBIN_IDLE				2
	
#define ST_REGEX		1         // search using regular expression
	#define SF_REGEX_CASEINSENSITIVE		1
	#define SF_REGEX_IDLE				2

#define ST_EXPR		3         // search stops when expression evals to non-zero

typedef ht_view* (*create_form_func)(Bounds *b, uint histid);
typedef void (*create_desc_func)(char *buf, int buflen, ht_view *form);

/*
 *	CLASS ht_fxbin_search_request
 */
class ht_fxbin_search_request: public ht_search_request {
public:
	uint data_size;
	byte *data;
	
			ht_fxbin_search_request(uint search_class, uint flags, uint data_size, byte *data);
	virtual		~ht_fxbin_search_request();
	/* overwritten */
	virtual	ht_fxbin_search_request *clone() const;
};

/*
 *	CLASS ht_regex_search_request
 */
class ht_regex_search_exception: public Exception {
protected:
	char rxerr[128];
public:	
	ht_regex_search_exception(int e, regex_t *r);
	virtual String &reason(String &) const;
};

class ht_regex_search_request: public ht_search_request {
public:
	char *rx_str;
	regex_t rx;

			ht_regex_search_request(uint search_class, uint flags, char *regex);
	virtual	~ht_regex_search_request();
	/* overwritten */
	virtual	 ht_regex_search_request* clone() const;
};

/*
 *	CLASS ht_expr_search_request
 */
class ht_expr_search_request: public ht_search_request {
public:
	char *expr;

		ht_expr_search_request(uint search_class, uint flags, char *Expr);
	virtual	~ht_expr_search_request();
	/* overwritten */
	virtual	ht_expr_search_request *clone() const;
};

/* binary search function */

/*
 *	CLASS ht_hexascii_search_form
 */
struct ht_hexascii_search_form_data {
	ht_strinputfield_data str;
//	ht_hexinputfield_data hex; // is attached to |str|
	ht_strinputfield_data start;
	ht_strinputfield_data end;
	ht_checkboxes_data options;
};

class ht_hexascii_search_form: public ht_group {
protected:
	ht_strinputfield *str;
	ht_strinputfield *range_start;
	ht_strinputfield *range_end;
	ht_checkboxes *option_boxes;
public:
			void init(Bounds *b, int options, List *history=0);
};

/*
 *	CLASS ht_evalstr_search_form
 */
struct ht_evalstr_search_form_data {
	ht_strinputfield_data str;
	ht_strinputfield_data start;
	ht_strinputfield_data end;
	ht_checkboxes_data options;
};

class ht_evalstr_search_form: public ht_group {
protected:
	ht_strinputfield *str;
	ht_strinputfield *range_start;
	ht_strinputfield *range_end;
	ht_checkboxes *option_boxes;
public:
			void	init(Bounds *b, int options, List *history=0);
};

/*
 *	CLASS ht_vregex_search_form
 */
struct ht_vregex_search_form_data {
	ht_strinputfield_data str;
	ht_strinputfield_data start;
	ht_strinputfield_data end;
	ht_checkboxes_data options;
};

class ht_vregex_search_form: public ht_group {
protected:
	ht_strinputfield *str;
	ht_strinputfield *range_start;
	ht_strinputfield *range_end;
	ht_checkboxes *option_boxes;
public:
			void	init(Bounds *b, int options, List *history=0);
};

/*
 *	CLASS ht_expr_search_form
 */
struct ht_expr_search_form_data {
	ht_strinputfield_data str;
	ht_strinputfield_data start;
	ht_strinputfield_data end;
	ht_checkboxes_data options;
};

class ht_expr_search_form: public ht_group {
protected:
	ht_strinputfield *str;
	ht_strinputfield *range_start;
	ht_strinputfield *range_end;
	ht_checkboxes *option_boxes;
public:
			void	init(Bounds *b, int options, List *history=0);
};

/*
 *	CLASS ht_replace_hexascii_search_form
 */
struct ht_replace_hexascii_search_form_data {
	ht_strinputfield_data str;
	ht_hexinputfield_data hex;
};

class ht_replace_hexascii_search_form: public ht_group {
protected:
	ht_strinputfield *str;
public:
			void init(Bounds *b, int options, List *history=0);
};

/*
 *	CLASS ht_search_dialog
 */
struct ht_search_dialog_mode {
	int id;
	ht_view *view;
};

#define MAX_SEARCH_DIALOG_MODES 16

class ht_search_dialog: public ht_dialog {
protected:
	ht_listpopup *search_mode_popup;

	int smodeidx;
	int smodecount;
	ht_search_dialog_mode smodes[MAX_SEARCH_DIALOG_MODES];

	int find_search_mode(int id);
	void select_search_mode_bymodeidx();
public:
	ht_xgroup *search_mode_xgroup;
	
			void	init(Bounds *b, const char *title);
	virtual	void	done();
	/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	/* new */
			void insert_search_mode(int id, const char *desc, ht_view *v);
			void select_search_mode(int id);
			ht_view *get_search_modeform();
			int get_search_modeid();
};

/*
 *	CLASS ht_replace_dialog
 */
#define MAX_REPLACE_DIALOG_MODES 16

class ht_replace_dialog: public ht_search_dialog {
protected:
	ht_listpopup *replace_mode_popup;

	int rmodeidx;
	int rmodecount;
	ht_search_dialog_mode rmodes[MAX_REPLACE_DIALOG_MODES];

	int find_replace_mode(int id);
	void select_replace_mode_bymodeidx();
public:
	ht_xgroup *replace_mode_xgroup;
	
			void	init(Bounds *b);
	virtual	void	done();
	/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	/* new */
			void insert_replace_mode(int id, const char *desc, ht_view *v);
			void select_replace_mode(int id);
			ht_view *get_replace_modeform();
			int get_replace_modeid();
};


/*
 *
 */
#define SEARCHMODE_BIN		1
#define SEARCHMODE_EVALSTR	2
#define SEARCHMODE_VREGEX	4
#define SEARCHMODE_EXPR		8

ht_search_request *search_dialog(ht_format_viewer *format, uint searchmodes, viewer_pos *start, viewer_pos *end);
uint replace_dialog(ht_format_viewer *format, uint searchmodes, bool *cancelled);

class ht_search_bin_context: public Object {
public:
	File *file;
	bool file_end;
	uint flags;
	
	FileOfs ofs;
	FileOfs len;

	byte *pat;
	uint patlen;

	FileOfs o;
	uint c;
	
	byte *buf;
	byte *bufptr;

	bool *return_success;
	FileOfs *return_ofs;
	
	~ht_search_bin_context();
};

// flags
#define SFBIN_CASEINSENSITIVE	1

Object* create_search_bin_context(File *file, FileOfs ofs, FileOfs len, byte *pat, uint patlen, uint flags, FileOfs *return_ofs, bool *return_success);
bool search_bin_process(Object *context, ht_text *progress_indicator);

ht_view* create_form_hexascii(Bounds *b, uint histid);
void create_desc_hexascii(char *buf, int buflen, ht_view *f);

/*
 *
 */
class ht_replace_bin_context: public Object {
public:
	File *file;

	FileOfs ofs;
	FileOfs len;
	
	FileOfs repllen;
	byte *repl;

	FileOfs o;
	FileOfs z;
	byte *buf;
	
	FileOfs *return_repllen;

	~ht_replace_bin_context();
};
 
Object* create_replace_bin_context(File *file, FileOfs ofs, FileOfs len, byte *repl, FileOfs repllen, FileOfs *return_repllen);
bool replace_bin_process(Object *context, ht_text *progress_indicator);

#endif /* __HTSEARCH_H__ */
