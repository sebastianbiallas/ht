/*
 *	HT Editor
 *	htsearch.cc
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

#include "atom.h"
#include "cmds.h"
#include "htctrl.h"
#include "hthist.h"
#include "htiobox.h"
#include "keyb.h"
#include "htsearch.h"
#include "strtools.h"
#include "htprocess.h"
#include "snprintf.h"

extern "C" {
#include "evalx.h"
}

union search_pos {
	FileOfs offset;
	viewer_pos pos;
};

/* FIXME: get rid of global vars */
uint lastsearchmodeid = 0;
uint lastreplacemodeid = 0;

typedef ht_search_request* (*create_request_func)(search_pos *ret_start, search_pos *ret_end, ht_view *form, ht_format_viewer *format, uint search_class);

typedef Object* (*create_replace_context_func)(File *file, FileOfs ofs, FileOfs len, ht_view *form, FileOfs *return_repllen);

struct ht_search_method {
	const char *name;
	uint search_class;			// SC_*
	uint search_mode_mask;		// SEARCHMODE_*
	uint histid;
	create_form_func create_form;
	create_request_func create_request;
	create_desc_func create_desc;
};

struct ht_replace_method {
	const char *name;
	uint histid;
	create_form_func create_form;
	create_replace_context_func create_replace_context;
	process_func replace_process;
};

#include <stdlib.h>
#include <string.h>

static bool test_str_to_ofs(FileOfs *ofs, byte *str, uint strlen, ht_format_viewer *format, const char *desc)
{
#define TEST_STR_TO_OFS_MAXSTRLEN       128
	if (strlen > TEST_STR_TO_OFS_MAXSTRLEN) {
		throw MsgfException("%s: expression too long (len %d, max %d)", desc, strlen, TEST_STR_TO_OFS_MAXSTRLEN);
		return false;
	}
	if (strlen > 0) {
		char s[TEST_STR_TO_OFS_MAXSTRLEN+1];
		bin2str(s, str, strlen);
		if (!format->string_to_offset(s, ofs)) {
			throw MsgfException("%s: invalid expression: '%s'", desc, s);
			return false;
		}
	}
	return true;
}

static bool test_str_to_pos(viewer_pos *pos, byte *str, uint strlen, ht_format_viewer *format, const char *desc)
{
#define TEST_STR_TO_POS_MAXSTRLEN      128
	if (strlen > TEST_STR_TO_POS_MAXSTRLEN) {
		throw MsgfException("%s: expression too long (len %d, max %d)", desc, strlen, TEST_STR_TO_POS_MAXSTRLEN);
	}
	if (strlen > 0) {
		char s[TEST_STR_TO_POS_MAXSTRLEN+1];
		bin2str(s, str, strlen);
		if (!format->string_to_pos(s, pos)) {
			throw MsgfException("%s: invalid expression: '%s'", desc, s);
		}
	}
	return true;
}

/*
 *	CLASS ht_fxbin_search_request
 */

ht_view* create_form_hexascii(Bounds *b, uint histid)
{
	ht_hexascii_search_form *form = new ht_hexascii_search_form();
	form->init(b, 0, (List*)getAtomValue(histid));
	return form;
}

ht_search_request* create_request_hexascii(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, uint search_class)
{
	ht_hexascii_search_form *form = (ht_hexascii_search_form*)f;
	ht_hexascii_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);
	
	ht_fxbin_search_request *request;
	
	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "hex/ascii");
	}
	if (test_str_to_ofs(&start->offset, d.start.text, d.start.textlen, format, "start-offset")
	&& test_str_to_ofs(&end->offset, d.end.text, d.end.textlen, format, "end-offset")) {
		request = new ht_fxbin_search_request(search_class,
			d.options.state & 1 ? SF_FXBIN_CASEINSENSITIVE: 0,
			d.str.textlen, d.str.text);
	} else {
		request = NULL;
	}
	return request;
}

void create_desc_hexascii(char *buf, int buflen, ht_view *f)
{
	ht_hexascii_search_form *form=(ht_hexascii_search_form*)f;
	ht_hexascii_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);

	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "hex/ascii");
	}

	char *b = buf;
	b += escape_special(b, buflen, d.str.text, d.str.textlen, NULL, false);
	strncat(b, " (", buflen-(b-buf));
	if (d.options.state & 1) {
		strncat(b, "case-insensitive", buflen-(b-buf));
	} else {
		strncat(b, "case-sensitive", buflen-(b-buf));
	}
	if (d.start.textlen) {
		strncat(b, ", start=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b += escape_special(b, buflen-(b-buf), d.start.text, d.start.textlen);
	}
	if (d.end.textlen) {
		strncat(b, ", end=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b += escape_special(b, buflen-(b-buf), d.end.text, d.end.textlen);
	}
	strncat(b, ")", buflen-(b-buf));
}

/***/

ht_view* create_form_evalstr(Bounds *b, uint histid)
{
	ht_evalstr_search_form *form=new ht_evalstr_search_form();
	form->init(b, 0, (List*)getAtomValue(histid));
	return form;
}

ht_search_request* create_request_evalstr(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, uint search_class)
{
#define EVALSTR_MAXSTRLEN		256
	ht_evalstr_search_form *form=(ht_evalstr_search_form*)f;
	ht_evalstr_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);
	
	ht_fxbin_search_request *request = NULL;
		
	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "eval str");
	} else if (d.str.textlen<=EVALSTR_MAXSTRLEN) {
		char strbuf[EVALSTR_MAXSTRLEN+1];
		bin2str(strbuf, d.str.text, d.str.textlen);

		eval_scalar r;
		eval_str s;
		if (eval(&r, strbuf, NULL, NULL, NULL)) {
			scalar_context_str(&r, &s);
			scalar_destroy(&r);
			if (test_str_to_ofs(&start->offset, d.start.text, d.start.textlen, format, "start-offset")
			&& test_str_to_ofs(&end->offset, d.end.text, d.end.textlen, format, "end-offset")) {
				request = new ht_fxbin_search_request(search_class,
					d.options.state & 1 ? SF_FXBIN_CASEINSENSITIVE: 0,
					s.len, (byte*)s.value);
			}
		} else {
			const char *str;
			int pos;
			get_eval_error(&str, &pos);
			throw MsgfException("eval error at pos %d: %s", pos, str);
		}
	} else {
		throw MsgfException("%s: expression too long (len %d, max %d)", "eval str", d.str.textlen, EVALSTR_MAXSTRLEN);
	}
	return request;
}

void create_desc_evalstr(char *buf, int buflen, ht_view *f)
{
	ht_evalstr_search_form *form=(ht_evalstr_search_form*)f;
	ht_evalstr_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);

	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "eval str");
	}

	char *b = buf;
	b += escape_special(b, buflen, d.str.text, d.str.textlen, NULL, false);
	strncat(b, " (", buflen-(b-buf));
	if (d.options.state & 1) {
		strncat(b, "case-insensitive", buflen-(b-buf));
	} else {
		strncat(b, "case-sensitive", buflen-(b-buf));
	}
	if (d.start.textlen) {
		strncat(b, ", start=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b+=escape_special(b, buflen-(b-buf), d.start.text, d.start.textlen);
	}
	if (d.end.textlen) {
		strncat(b, ", end=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b+=escape_special(b, buflen-(b-buf), d.end.text, d.end.textlen);
	}
	strncat(b, ")", buflen-(b-buf));
}

/***/

ht_fxbin_search_request::ht_fxbin_search_request(uint search_class, uint flags, uint ds, byte *d) :
	ht_search_request(search_class, ST_FXBIN, flags)
{
	data_size = ds;
	data = ht_malloc(data_size);
	memcpy(data, d, data_size);
}

ht_fxbin_search_request::~ht_fxbin_search_request()
{
	free(data);
}

ht_fxbin_search_request *ht_fxbin_search_request::clone() const
{
	return new ht_fxbin_search_request(search_class, flags, data_size, data);
}

/*
 *	CLASS ht_regex_search_request
 */
 
ht_view* create_form_vregex(Bounds *b, uint histid)
{
	ht_vregex_search_form *form=new ht_vregex_search_form();
	form->init(b, 0, (List*)getAtomValue(histid));
	return form;
}

ht_search_request* create_request_vregex(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, uint search_class)
{
#define VREGEX_MAXSTRLEN		256
	ht_vregex_search_form *form=(ht_vregex_search_form*)f;
	ht_vregex_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);

	ht_regex_search_request *request=NULL;

	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "regex");
	} else if (d.str.textlen <= VREGEX_MAXSTRLEN) {
		char strbuf[VREGEX_MAXSTRLEN+1];
		bin2str(strbuf, d.str.text, d.str.textlen);

		if (test_str_to_pos(&start->pos, d.start.text, d.start.textlen, format, "start-address")
		&& test_str_to_pos(&end->pos, d.end.text, d.end.textlen, format, "end-address")) {
			request = new ht_regex_search_request(search_class, d.options.state & 1 ? SF_REGEX_CASEINSENSITIVE : 0, strbuf);
		}
	} else {
		throw MsgfException("%s: expression too long (size %d, max %d)", "regex", strlen, VREGEX_MAXSTRLEN);
	}
	return request;
}

void create_desc_vregex(char *buf, int buflen, ht_view *f)
{
	ht_vregex_search_form *form=(ht_vregex_search_form*)f;
	ht_vregex_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);

	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "regex");
	}

	char *b = buf;
	b += escape_special(b, buflen, d.str.text, d.str.textlen, NULL, false);
	strncat(b, " (", buflen-(b-buf));
	if (d.options.state & 1) {
		strncat(b, "case-insensitive", buflen-(b-buf));
	} else {
		strncat(b, "case-sensitive", buflen-(b-buf));
	}
	if (d.start.textlen) {
		strncat(b, ", start=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b+=escape_special(b, buflen-(b-buf), d.start.text, d.start.textlen);
	}
	if (d.end.textlen) {
		strncat(b, ", end=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b+=escape_special(b, buflen-(b-buf), d.end.text, d.end.textlen);
	}
	strncat(b, ")", buflen-(b-buf));
}

/***/

ht_regex_search_request::ht_regex_search_request(uint search_class, uint flags, char *regex) :
	ht_search_request(search_class, ST_REGEX, flags)
{
	rx_str = ht_strdup(regex);
	int r = regcomp(&rx, rx_str, REG_EXTENDED | ((flags & SF_REGEX_CASEINSENSITIVE) ? REG_ICASE : 0));
	if (r) throw ht_regex_search_exception(r, &rx);
}

ht_regex_search_request::~ht_regex_search_request()
{
	regfree(&rx);
	free(rx_str);
}

ht_regex_search_request *ht_regex_search_request::clone() const
{
	return new ht_regex_search_request(search_class, flags, rx_str);
}

/*
 *	CLASS ht_regex_search_request
 */
 
ht_regex_search_exception::ht_regex_search_exception(int e, regex_t *r)
{
	char s[128];
	regerror(e, r, s, sizeof s);
	ht_snprintf(rxerr, sizeof rxerr, "error compiling regex: %s", s);
}
	
String &ht_regex_search_exception::reason(String &s) const
{
	return s = rxerr;
}

/*
 *	CLASS ht_expr_search_request
 */

ht_view* create_form_expr(Bounds *b, uint histid)
{
	ht_expr_search_form *form = new ht_expr_search_form();
	form->init(b, 0, (List*)getAtomValue(histid));
	return form;
}

ht_search_request* create_request_expr(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, uint search_class)
{
#define EXPR_MAXSTRLEN		256
	ht_expr_search_form *form=(ht_expr_search_form*)f;
	ht_expr_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);
	
	ht_expr_search_request *request = NULL;

	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "expr");
	} else if (d.str.textlen <= EXPR_MAXSTRLEN) {
		char strbuf[EXPR_MAXSTRLEN+1];
		bin2str(strbuf, d.str.text, d.str.textlen);
		
		if (test_str_to_ofs(&start->offset, d.start.text, d.start.textlen, format, "start-offset")
		&& test_str_to_ofs(&end->offset, d.end.text, d.end.textlen, format, "end-offset")) {
			request = new ht_expr_search_request(search_class, 0, strbuf);
		}
	} else {
		throw MsgfException("%s: expression too long (size %d, max %d)", "expr", strlen, EXPR_MAXSTRLEN);
	}
	return request;
}

void create_desc_expr(char *buf, int buflen, ht_view *f)
{
	ht_vregex_search_form *form=(ht_vregex_search_form*)f;
	ht_vregex_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);

	if (!d.str.textlen) {
		throw MsgfException("%s: string is empty", "expr");
	}

	char *b = buf;
	b += escape_special(b, buflen, d.str.text, d.str.textlen, NULL, false);
	strncat(b, " (", buflen-(b-buf));
	if (d.options.state & 1) {
		strncat(b, "case-insensitive", buflen-(b-buf));
	} else {
		strncat(b, "case-sensitive", buflen-(b-buf));
	}
	if (d.start.textlen) {
		strncat(b, ", start=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b+=escape_special(b, buflen-(b-buf), d.start.text, d.start.textlen);
	}
	if (d.end.textlen) {
		strncat(b, ", end=", buflen-(b-buf));
		if (b-buf<buflen) b+=strlen(b);
		b+=escape_special(b, buflen-(b-buf), d.end.text, d.end.textlen);
	}
	strncat(b, ")", buflen-(b-buf));
}

/***/

ht_expr_search_request::ht_expr_search_request(uint search_class, uint flags, char *Expr) :
	ht_search_request(search_class, ST_EXPR, flags)
{
	expr = ht_strdup(Expr);
}

ht_expr_search_request::~ht_expr_search_request()
{
	free(expr);
}

ht_expr_search_request *ht_expr_search_request::clone() const
{
	return new ht_expr_search_request(search_class, flags, expr);
}

/*
 *	binary search
 */

/* FIXME: put somewhere else */
void bufdowncase(byte *buf, uint32 len)
{
	for (uint32 i=0; i<len; i++) {
		if (buf[i] >= 'A' && buf[i] <= 'Z') buf[i] += 32;
	}
}

#define SEARCH_BUF_SIZE 256*1024

ht_search_bin_context::~ht_search_bin_context()
{
	free(buf);
	free(pat);
}

Object* create_search_bin_context(File *file, FileOfs ofs, FileOfs len, byte *pat, uint patlen, uint flags, FileOfs *return_ofs, bool *return_success)
{
	if (patlen > SEARCH_BUF_SIZE) return NULL;
	
	ht_search_bin_context *ctx = new ht_search_bin_context();
	ctx->file = file;
	ctx->file_end = false;
	ctx->ofs = ofs;
	ctx->flags = flags;
	ctx->len = len;
	ctx->pat = ht_malloc(patlen);
	memcpy(ctx->pat, pat, patlen);
	ctx->patlen = patlen;

	ctx->o = ofs;

	if (ctx->flags & SFBIN_CASEINSENSITIVE) bufdowncase(ctx->pat, ctx->patlen);

	ctx->buf = ht_malloc(SEARCH_BUF_SIZE);

	ctx->return_ofs = return_ofs;
	ctx->return_success = return_success;

	file->seek(ctx->o);
	ctx->c = file->read(ctx->buf, SEARCH_BUF_SIZE);
	ctx->bufptr = ctx->buf;
	ctx->file_end = (ctx->c != SEARCH_BUF_SIZE);
	if (ctx->flags & SFBIN_CASEINSENSITIVE) bufdowncase(ctx->buf, ctx->c);

	return ctx;
}

bool search_bin_process(Object *context, ht_text *progress_indicator)
{
	ht_search_bin_context *ctx = (ht_search_bin_context*)context;

	if (ctx->bufptr - ctx->buf + ctx->patlen > ctx->c) {
		if (ctx->file_end) {
			*ctx->return_success = false;
			return false;
		}
		ctx->file->seek(ctx->o);
		ctx->c = ctx->file->read(ctx->buf, SEARCH_BUF_SIZE);
		ctx->bufptr = ctx->buf;
		ctx->file_end = (ctx->c != SEARCH_BUF_SIZE);
		if (ctx->flags & SFBIN_CASEINSENSITIVE) bufdowncase(ctx->buf, ctx->c);
	}

	while (ctx->bufptr + ctx->patlen <= ctx->buf + ctx->c) {
		if (memcmp(ctx->pat, ctx->bufptr, ctx->patlen) == 0) {
			*ctx->return_success = true;
			*ctx->return_ofs = ctx->o;
			return false;
		}
		ctx->bufptr++;
		ctx->o++;
	}
	ctx->o -= ctx->patlen-1;

	int p = (((double)(ctx->o - ctx->ofs))*100/ctx->len);

	char status[64];
	ht_snprintf(status, sizeof status, "%d %% (%d MiB)", p, int ((unsigned long long)ctx->o >> 20));
	progress_indicator->settext(status);

	return true;
}

/*
 *	CLASS ht_hexascii_search_form
 */
void ht_hexascii_search_form::init(Bounds *b, int options, List *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_hexascii_search_form");

	Bounds c;
	/* ascii string */
	c.x=6;
	c.y=0;
	c.w=b->w-6;
	c.h=1;
	str=new ht_strinputfield();
	str->init(&c, 64, history);
	insert(str);
	/* ascii string label */
	c.x=0;
	c.w=5;
	c.h=1;
	ht_label *strlabel=new ht_label();
	strlabel->init(&c, "~ascii", str);
	insert(strlabel);

	/* hex string */
	c.x=6;
	c.y=2;
	c.w=b->w-6;
	c.h=1;
	ht_hexinputfield *hex=new ht_hexinputfield();
	hex->init(&c, 64);
	hex->attach(str);
	insert(hex);
	/* hex string label */
	c.x=0;
	c.w=5;
	c.h=1;
	ht_label *hexlabel=new ht_label();
	hexlabel->init(&c, "~hex", hex);
	insert(hexlabel);

	/* range start */
	c.x=10;
	c.y=4;
	c.w=10;
	c.h=1;
	range_start=new ht_strinputfield();
	range_start->init(&c, 10);
	insert(range_start);
	/* range start label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *rslabel=new ht_label();
	rslabel->init(&c, "~from ofs", range_start);
	insert(rslabel);

	/* range end */
	c.x=10;
	c.y=6;
	c.w=10;
	c.h=1;
	range_end=new ht_strinputfield();
	range_end->init(&c, 10);
	insert(range_end);
	/* range end label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *relabel=new ht_label();
	relabel->init(&c, "~to ofs", range_end);
	insert(relabel);
	/* options */
	c.w=35;
	c.y=8;
	c.h=2;
	ht_string_list *opts = new ht_string_list();
	opts->init();
	opts->insert_string("case ~insensitive");
	option_boxes = new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state = options;
	option_boxes->databuf_set(&d, sizeof d);
	insert(option_boxes);
}

/*
 *	CLASS ht_evalstr_search_form
 */
void ht_evalstr_search_form::init(Bounds *b, int options, List *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_evalstr_search_form");

	Bounds c;
	/* string */
	c.x=0;
	c.y=1;
	c.w=b->w;
	c.h=1;
	str=new ht_strinputfield();
	str->init(&c, 64, history);
	insert(str);
	/* string label */
	c.x=0;
	c.y=0;
	c.w=23;
	c.h=1;
	ht_label *strlabel=new ht_label();
	strlabel->init(&c, "s~earch evaluated string", str);
	insert(strlabel);
	/* hint */
/*	c.x=0;
	c.y=2;
	c.w=b->w-2;
	c.h=1;
	ht_statictext *hint=new ht_statictext();
	hint->init(&c, "(example: \"hello\\n\\0\\077\\xd\" 'ho',011b,66o)", 0);
	insert(hint);*/

	/* range start */
	c.x=10;
	c.y=4;
	c.w=10;
	c.h=1;
	range_start=new ht_strinputfield();
	range_start->init(&c, 10);
	insert(range_start);
	/* range start label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *rslabel=new ht_label();
	rslabel->init(&c, "~from ofs", range_start);
	insert(rslabel);

	/* range end */
	c.x=10;
	c.y=6;
	c.w=10;
	c.h=1;
	range_end=new ht_strinputfield();
	range_end->init(&c, 10);
	insert(range_end);
	/* range end label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *relabel=new ht_label();
	relabel->init(&c, "~to ofs", range_end);
	insert(relabel);
	/* options */
	c.w = 35;
	c.y = 8;
	c.h = 2;
	ht_string_list *opts = new ht_string_list();
	opts->init();
	opts->insert_string("case ~insensitive");
	option_boxes = new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state = options;
	option_boxes->databuf_set(&d, sizeof d);
	insert(option_boxes);
}

/*
 *	CLASS ht_vregex_search_form
 */
void ht_vregex_search_form::init(Bounds *b, int options, List *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_text_search_form");

	Bounds c;
	/* string */
	c.x=0;
	c.y=1;
	c.w=b->w;
	c.h=1;
	str=new ht_strinputfield();
	str->init(&c, 64, history);
	insert(str);
	/* string label */
	c.x=0;
	c.y=0;
	c.w=12;
	c.h=1;
	ht_label *strlabel=new ht_label();
	strlabel->init(&c, "s~earch regex", str);
	insert(strlabel);

	/* range start */
	c.x=10;
	c.y=4;
	c.w=10;
	c.h=1;
	range_start=new ht_strinputfield();
	range_start->init(&c, 10);
	insert(range_start);
	/* range start label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *rslabel=new ht_label();
	rslabel->init(&c, "~from addr", range_start);
	insert(rslabel);

	/* range end */
	c.x=10;
	c.y=6;
	c.w=10;
	c.h=1;
	range_end=new ht_strinputfield();
	range_end->init(&c, 10);
	insert(range_end);
	/* range end label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *relabel=new ht_label();
	relabel->init(&c, "~to addr", range_end);
	insert(relabel);
	/* options */
	c.w = 35;
	c.y = 8;
	c.h = 2;
	ht_string_list *opts = new ht_string_list();
	opts->init();
	opts->insert_string("case ~insensitive");
	option_boxes = new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state = options;
	option_boxes->databuf_set(&d, sizeof d);
	insert(option_boxes);
}

/*
 *	CLASS ht_expr_search_form
 */
void	ht_expr_search_form::init(Bounds *b, int options, List *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_expr_search_form");

	Bounds c;
	/* string */
	c.x=0;
	c.y=1;
	c.w=b->w;
	c.h=1;
	str=new ht_strinputfield();
	str->init(&c, 256, history);
	insert(str);
	/* string label */
	c.x=0;
	c.y=0;
	c.w=17;
	c.h=1;
	ht_label *strlabel=new ht_label();
	strlabel->init(&c, "s~earch expression", str);
	insert(strlabel);
	/* hint */
	c.x=0;
	c.y=2;
	c.w=b->w-2;
	c.h=1;
	ht_statictext *hint=new ht_statictext();
	hint->init(&c, "stops if expression evaluates to non-zero", align_left);
	insert(hint);

	/* range start */
	c.x=10;
	c.y=4;
	c.w=10;
	c.h=1;
	range_start=new ht_strinputfield();
	range_start->init(&c, 10);
	insert(range_start);
	/* range start label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *rslabel=new ht_label();
	rslabel->init(&c, "~from ofs", range_start);
	insert(rslabel);

	/* range end */
	c.x=10;
	c.y=6;
	c.w=10;
	c.h=1;
	range_end=new ht_strinputfield();
	range_end->init(&c, 10);
	insert(range_end);
	/* range end label */
	c.x=0;
	c.w=9;
	c.h=1;
	ht_label *relabel=new ht_label();
	relabel->init(&c, "~to ofs", range_end);
	insert(relabel);
	/* options */
/*	c.w=35;
	c.y=8;
	c.h=2;
	ht_string_list *opts=new ht_string_list();
	opts->init();
	option_boxes=new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state=options;
	option_boxes->databuf_set(&d);
	insert(option_boxes);*/
}

/*
 *	CLASS ht_replace_hexascii_search_form
 */
ht_replace_bin_context::~ht_replace_bin_context()
{
	free(repl);
}

ht_view* create_form_replace_hexascii(Bounds *b, uint histid)
{
	ht_replace_hexascii_search_form *form=new ht_replace_hexascii_search_form();
	form->init(b, 0, (List*)getAtomValue(histid));
	return form;
}

Object* create_replace_hexascii_context(File *file, FileOfs ofs, FileOfs len, ht_view *form, FileOfs *return_repllen)
{
	ht_replace_hexascii_search_form_data d;
	ViewDataBuf vdb(form, &d, sizeof d);
	
	ht_replace_bin_context *ctx = (ht_replace_bin_context*)
	create_replace_bin_context(file, ofs, len, d.str.text, d.str.textlen, return_repllen);

	return ctx;
}

void ht_replace_hexascii_search_form::init(Bounds *b, int options, List *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_replace_hexascii_search_form");

	Bounds c;
	/* ascii string */
	c.x=6;
	c.y=0;
	c.w=40;
	c.h=1;
	str=new ht_strinputfield();
	str->init(&c, 64, history);
	insert(str);
	/* ascii string label */
	c.x=0;
	c.w=5;
	c.h=1;
	ht_label *strlabel=new ht_label();
	strlabel->init(&c, "~ascii", str);
	insert(strlabel);

	/* hex string */
	c.x=6;
	c.y=2;
	c.w=40;
	c.h=1;
	ht_hexinputfield *hex=new ht_hexinputfield();
	hex->init(&c, 64);
	hex->attach(str);
	insert(hex);
	/* hex string label */
	c.x=0;
	c.w=5;
	c.h=1;
	ht_label *hexlabel=new ht_label();
	hexlabel->init(&c, "~hex", hex);
	insert(hexlabel);
}

/*
 *
 */

static ht_search_method search_methods[] =
{
	{ "bin: hex/ascii", SC_PHYSICAL, SEARCHMODE_BIN, HISTATOM_SEARCH_BIN,
		create_form_hexascii, create_request_hexascii, create_desc_hexascii },
	{ "bin: eval str", SC_PHYSICAL, SEARCHMODE_EVALSTR, HISTATOM_SEARCH_EVALSTR,
		create_form_evalstr, create_request_evalstr, create_desc_evalstr },
	{ "display: regex", SC_VISUAL, SEARCHMODE_VREGEX, HISTATOM_SEARCH_VREGEX,
		create_form_vregex, create_request_vregex, create_desc_vregex },
	{ "expr non-zero", SC_PHYSICAL, SEARCHMODE_EXPR, HISTATOM_SEARCH_EXPR,
		create_form_expr, create_request_expr, create_desc_expr },
	{ NULL }
};

ht_search_request *search_dialog(ht_format_viewer *format, uint searchmodes, viewer_pos *start, viewer_pos *end)
{
	ht_search_request *result = NULL;
	Bounds b;
	b.w = 50;
	b.h = 15;
	b.x = (screen->w - b.w)/2;
	b.y = (screen->h - b.h)/2;
	ht_search_dialog *dialog = new ht_search_dialog();
	dialog->init(&b, "search");

	Bounds k;
	dialog->search_mode_xgroup->getbounds(&k);

	k.x = 0;
	k.y = 0;

	int modes = 0;
	int i = 0;
	ht_search_method *q = search_methods;
	while (q->name) {
		if (q->search_mode_mask & searchmodes) {
			Bounds v = k;
			ht_view *form = q->create_form(&v, q->histid);
			dialog->insert_search_mode(i, q->name, form);
			modes++;
		}
		q++;
		i++;
	}
	
	dialog->select_search_mode(lastsearchmodeid);
	
	if (dialog->run(false)) {
		int modeid = dialog->get_search_modeid();
		lastsearchmodeid = modeid;

		ht_search_method *s = &search_methods[modeid];
		ht_view *form = dialog->get_search_modeform();

		search_pos sstart, send;

		try {
			/* create history entry */
			if (s->create_desc) {
				char hist_desc[1024];
				s->create_desc(hist_desc, sizeof hist_desc, form);
				insert_history_entry((List*)getAtomValue(s->histid), hist_desc, form);
			}
			/* create request */
			switch (s->search_class) {
			case SC_PHYSICAL:
				if (!format->pos_to_offset(*start, &sstart.offset)) {
					throw MsgfException("Internal error: can't convert viewer_pos to offset");
				}
				if (!format->pos_to_offset(*end, &send.offset)) {
					send.offset = -1;
				}
				break;
			case SC_VISUAL:
				sstart.pos = *start;
				send.pos = *end;
				break;
			}
			result = s->create_request(&sstart, &send, form, format, s->search_class);
			switch (s->search_class) {
			case SC_PHYSICAL:
				if (!format->offset_to_pos(sstart.offset, start)) {
					throw MsgfException("Internal error: can't convert offset to viewer_pos");
				}
				if (!format->pos_to_offset(*end, &send.offset)) {
					send.offset = -1;
				}
				break;
			case SC_VISUAL:
				*start = sstart.pos;
				*end = send.pos;
				break;
			}
		} catch (const Exception &e) {
			errorbox("error: %y", &e);
		}
	}
	dialog->done();
	delete dialog;
	return result;
}

static ht_replace_method replace_methods[] =
{
	{ "bin: hex/ascii", 0, create_form_replace_hexascii,
		create_replace_hexascii_context, replace_bin_process },
	{ NULL }
};

uint replace_dialog(ht_format_viewer *format, uint searchmodes, bool *cancelled)
{
	*cancelled = false;
	Bounds b;
	b.w = 50;
	b.h = 22;
	b.x = (screen->w - b.w)/2;
	b.y = (screen->h - b.h)/2;
	ht_replace_dialog *dialog = new ht_replace_dialog();
	dialog->init(&b);

	Bounds k;
	dialog->search_mode_xgroup->getbounds(&k);

	k.x = 0;
	k.y = 0;

	int i;

	i = 0;
	ht_search_method *q = search_methods;
	while (q->name) {
		if ((q->search_mode_mask & searchmodes) &&
		(q->search_class == SC_PHYSICAL)) {
			Bounds v = k;
			ht_view *form = q->create_form(&v, q->histid);
			dialog->insert_search_mode(i, q->name, form);
		}
		q++;
		i++;
	}

	dialog->replace_mode_xgroup->getbounds(&k);
	k.x = 0;
	k.y = 0;

	i = 0;
	ht_replace_method *w = replace_methods;
	while (w->name) {
		Bounds v = k;
		ht_view *form = w->create_form(&v, w->histid);
		dialog->insert_replace_mode(i, w->name, form);
		w++;
		i++;
	}

	dialog->select_search_mode(lastsearchmodeid);
	dialog->select_replace_mode(lastreplacemodeid);

	uint repl_count = 0;
	if (dialog->run(false)) {
		int smodeid = dialog->get_search_modeid();
		int rmodeid = dialog->get_replace_modeid();
		lastsearchmodeid = smodeid;
		lastreplacemodeid = rmodeid;

		ht_search_method *s = &search_methods[smodeid];
		ht_replace_method *r = &replace_methods[rmodeid];
		ht_view *sform = dialog->get_search_modeform();
		ht_view *rform = dialog->get_replace_modeform();

		search_pos start, end;

		ht_search_request *request = NULL;
		
		try {
			/* create history entry */
			if (s->create_desc) {
				char hist_desc[1024];
				s->create_desc(hist_desc, sizeof hist_desc, sform);
				insert_history_entry((List*)getAtomValue(s->histid), hist_desc, sform);
			}
			/* search */
			start.offset=0;
			end.offset=0xffffffff;
			format->get_current_offset(&start.offset);

			request = s->create_request(&start, &end, sform, format, s->search_class);
		} catch (const Exception &e) {
			errorbox("error: %y", &e);
		}
		
		if (request) {
			FileOfs so = start.offset, eo = end.offset;
			ht_physical_search_result *result;

			format->vstate_save();
				
			try {
				bool replace_all = false;
				while ((result = (ht_physical_search_result*)format->psearch(request, so, eo))) {
					FileOfs irepllen = 0;
					bool do_replace = false;

					if (!replace_all) {
						bool showed = format->show_search_result(result);
						int r = msgbox(btmask_yes+btmask_no+btmask_all+btmask_cancel, "confirmation", 0, align_center, "replace?");
						if (showed) app->sendmsg(cmd_vstate_restore);
						switch (r) {
							case button_yes:
								do_replace = true;
								break;
							case button_no:
								break;
							case button_all:
								replace_all = true;
								break;
						}
						if (r == button_cancel) {
							*cancelled = true;
							delete result;
							break;
						}
					}

					if (replace_all || do_replace) {
						Object *rctx = r->create_replace_context(format->get_file(), result->offset, result->size, rform, &irepllen);
						bool p = execute_process(r->replace_process, rctx);
						delete rctx;
						if (!p) {
							delete result;
							break;
						}

						if (irepllen != result->size) {
							format->sendmsg(msg_filesize_changed);
						}
						so = result->offset + irepllen;
						repl_count++;
					} else {
						so = result->offset + result->size;
					}
					delete result;
				}
			} catch (const Exception &e) {
				errorbox("error: %y", &e);
			}

			app->sendmsg(cmd_vstate_restore);
		}
				
		delete request;
	}
	dialog->done();
	delete dialog;
	return repl_count;
}

#define REPLACE_COPY_BUF_SIZE	64*1024
Object* create_replace_bin_context(File *file, FileOfs ofs, FileOfs len, byte *repl, FileOfs repllen, FileOfs *return_repllen)
{
	ht_replace_bin_context *ctx = new ht_replace_bin_context();
	ctx->file = file;
	ctx->ofs = ofs;
	ctx->len = len;
	ctx->repl = ht_malloc(repllen);
	memcpy(ctx->repl, repl, repllen);
	ctx->repllen = repllen;
	if (repllen > len) {
		ctx->o = file->getSize();
	} else if (len > repllen) {
		ctx->o = ofs + len;
	}
	ctx->z = REPLACE_COPY_BUF_SIZE;
	if (len != repllen)
		ctx->buf = ht_malloc(REPLACE_COPY_BUF_SIZE);
	ctx->return_repllen = return_repllen;
	return ctx;
}

bool replace_bin_process(Object *context, ht_text *progress_indicator)
{
	progress_indicator->settext("replacing...\n");
	
	ht_replace_bin_context *c = (ht_replace_bin_context*)context;
	if (c->repllen > c->len) {
		/* grow */
		uint size = c->file->getSize();
		c->file->extend(size + c->repllen - c->len);
		
		if (c->o > c->z) {
			c->o -= c->z;
		} else {
			c->z = c->o;
			c->o = 0;
		}
		if (c->o < c->ofs+c->len) {
			c->z -= c->ofs + c->len - c->o;
			c->o = c->ofs + c->len;
		}
		c->file->seek(c->o);
		c->file->readx(c->buf, c->z);
		c->file->seek(c->o + c->repllen - c->len);
		c->file->writex(c->buf, c->z);
			
		if (c->o > c->ofs + c->len) return true;
		
		c->file->seek(c->ofs);
		c->file->writex(c->repl, c->repllen);

		free(c->buf);
	} else if (c->repllen < c->len) {
		/* shrink */
		uint size = c->file->getSize();
		if (c->o == c->ofs + c->len) {
			c->file->seek(c->ofs);
			c->file->writex(c->repl, c->repllen);
		}
		
		if (c->z > size - c->o) {
			c->z = size - c->o;
		}
		c->file->seek(c->o);
		c->file->readx(c->buf, c->z);
		c->file->seek(c->o - (c->len - c->repllen));
		c->file->writex(c->buf, c->z);
		c->o += REPLACE_COPY_BUF_SIZE;
		
		if (c->z == REPLACE_COPY_BUF_SIZE) return true;
		
		c->file->truncate(size - (c->len - c->repllen));
		free(c->buf);
	} else {
		c->file->seek(c->ofs);
		c->file->writex(c->repl, c->repllen);
	}
	if (c->return_repllen) *c->return_repllen = c->repllen;
	return false;
}

/*
 *	CLASS ht_search_dialog
 */
void ht_search_dialog::init(Bounds *b, const char *title)
{
	ht_dialog::init(b, title, FS_KILLER | FS_TITLE | FS_MOVE);
	VIEW_DEBUG_NAME("ht_search_dialog");

	smodecount = 0;
	smodeidx = -1;

	Bounds c;
	c.x=1;
	c.y=1;
	c.w=20;
	c.h=1;

	search_mode_popup = new ht_listpopup();
	search_mode_popup->init(&c);
	insert(search_mode_popup);

	c.x=1;
	c.y=0;
	c.w=4;
	c.h=1;
	ht_label *mlabel=new ht_label();
	mlabel->init(&c, "~mode", search_mode_popup);
	insert(mlabel);

	c.x = 1;
	c.y = 3;
	c.w = size.w-4;
	c.h = MIN(10, size.h-4);
	search_mode_xgroup = new ht_xgroup();
	search_mode_xgroup->init(&c, VO_SELECTABLE, "modes");
	insert(search_mode_xgroup);
}

void ht_search_dialog::done()
{
	ht_dialog::done();
}

int ht_search_dialog::find_search_mode(int id)
{
	for (int i=0; i<smodecount; i++) {
		if (smodes[i].id==id) {
			return i;
		}
	}
	return -1;
}

void ht_search_dialog::handlemsg(htmsg *msg)
{
	if (msg->msg==msg_keypressed) {
		ht_dialog::handlemsg(msg);
		ht_listpopup_data data;
		ViewDataBuf vdb(search_mode_popup, &data, sizeof data);
		if ((int)data.cursor_pos != smodeidx) {
			smodeidx = data.cursor_pos;
			select_search_mode_bymodeidx();
		}
	} else ht_dialog::handlemsg(msg);
}

int ht_search_dialog::get_search_modeid()
{
	return smodes[smodeidx].id;
}

ht_view *ht_search_dialog::get_search_modeform()
{
	return smodes[smodeidx].view;
}

void ht_search_dialog::insert_search_mode(int id, const char *desc, ht_view *v)
{
	if (smodecount < MAX_SEARCH_DIALOG_MODES) {
		search_mode_xgroup->insert(v);
		search_mode_popup->insertstring(desc);
		smodes[smodecount].id=id;
		smodes[smodecount].view=v;
		smodecount++;
		focus(search_mode_xgroup);
	}
}

void ht_search_dialog::select_search_mode(int id)
{
	int i=find_search_mode(id);
	if (i!=-1) {
		smodeidx=i;
		select_search_mode_bymodeidx();
	}
}

void ht_search_dialog::select_search_mode_bymodeidx()
{
	ht_listpopup_data d;
	d.cursor_pos = smodeidx;
	d.cursor_string = NULL;
	search_mode_popup->databuf_set(&d, sizeof d);
	focus(smodes[smodeidx].view);
	sendmsg(msg_dirtyview, 0);
}

/*
 *	CLASS ht_replace_dialog
 */
void ht_replace_dialog::init(Bounds *b)
{
	ht_search_dialog::init(b, "replace");

	rmodecount=0;

	Bounds c;
	c.x=1;
	c.y=15;
	c.w=20;
	c.h=1;

	replace_mode_popup = new ht_listpopup();
	replace_mode_popup->init(&c);
	insert(replace_mode_popup);

	c.x=0;
	c.y=13;
	c.w=size.w-2;
	c.h=1;
	ht_hbar *hbar=new ht_hbar();
	hbar->init(&c, 0, NULL);
	insert(hbar);

	c.x=1;
	c.y=14;
	c.w=4;
	c.h=1;
	ht_label *mlabel=new ht_label();
	mlabel->init(&c, "~replace mode", replace_mode_popup);
	insert(mlabel);

	c.x=1;
	c.y=17;
	c.w=size.w-4;
	c.h=size.h-c.y;
	replace_mode_xgroup = new ht_xgroup();
	replace_mode_xgroup->init(&c, VO_SELECTABLE, "replace modes");
	insert(replace_mode_xgroup);
}

void ht_replace_dialog::done()
{
	ht_dialog::done();
}

int ht_replace_dialog::find_replace_mode(int id)
{
	for (int i=0; i<rmodecount; i++) {
		if (rmodes[i].id==id) {
			return i;
		}
	}
	return -1;
}

void ht_replace_dialog::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_keypressed) {
		ht_search_dialog::handlemsg(msg);
		ht_listpopup_data data;
		ViewDataBuf vdb(replace_mode_popup, &data, sizeof data);
		if ((int)data.cursor_pos != rmodeidx) {
			rmodeidx=data.cursor_pos;
			select_replace_mode_bymodeidx();
		}
	} else ht_search_dialog::handlemsg(msg);
}

int ht_replace_dialog::get_replace_modeid()
{
	return rmodes[rmodeidx].id;
}

ht_view *ht_replace_dialog::get_replace_modeform()
{
	return rmodes[rmodeidx].view;
}

void ht_replace_dialog::insert_replace_mode(int id, const char *desc, ht_view *v)
{
	if (rmodecount < MAX_REPLACE_DIALOG_MODES) {
		replace_mode_xgroup->insert(v);
		replace_mode_popup->insertstring(desc);
		rmodes[rmodecount].id=id;
		rmodes[rmodecount].view=v;
		rmodecount++;
//		focus(replace_mode_xgroup);
	}
}

void ht_replace_dialog::select_replace_mode(int id)
{
	int i=find_replace_mode(id);
	if (i!=-1) {
		rmodeidx=i;
		select_replace_mode_bymodeidx();
	}
}

void ht_replace_dialog::select_replace_mode_bymodeidx()
{
	ht_listpopup_data d;
	d.cursor_pos = rmodeidx;
	d.cursor_string = NULL;
	replace_mode_popup->databuf_set(&d, sizeof d);
//	focus(rmodes[rmodeidx].view);
	sendmsg(msg_dirtyview, 0);
}

/*
 *
 */
 
ht_search_result *linear_bin_search(ht_search_request *search, FileOfs start, FileOfs end, File *file, FileOfs fofs, FileOfs fsize)
{
	ht_fxbin_search_request *s = (ht_fxbin_search_request*)search;
		
	int fl = (search->flags & SFBIN_CASEINSENSITIVE) ? SFBIN_CASEINSENSITIVE : 0;
	if (start < fofs) start = fofs;
	if (end > fofs+fsize) end = fofs+fsize;
	if (fsize && start < end) {
		/* create result */
		bool search_success = false;
		FileOfs search_ofs;
		Object *ctx = create_search_bin_context(file, start, end-start, s->data, s->data_size, fl, &search_ofs, &search_success);
		if (execute_process(search_bin_process, ctx)) {
			delete ctx;
			if (search_success) {
				ht_physical_search_result *r=new ht_physical_search_result();
				r->offset = search_ofs;
				r->size = s->data_size;
				return r;
			}
		} else delete ctx;
	}
	return NULL;
}
