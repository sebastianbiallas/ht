/*
 *	HT Editor
 *	htsearch.cc
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

#include "htapp.h"
#include "htatom.h"
#include "htctrl.h"
#include "hthist.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htsearch.h"
#include "htstring.h"
#include "process.h"

extern "C" {
#include "evalx.h"
}

/* FIXME: get rid of global vars */
UINT lastsearchmodeid=0;
UINT lastreplacemodeid=0;

union search_pos {
	FILEOFS offset;
	fmt_vaddress address;
};

typedef ht_view* (*create_form_func)(bounds *b, HT_ATOM histid);
typedef ht_search_request* (*create_request_func)(search_pos *ret_start, search_pos *ret_end, ht_view *form, ht_format_viewer *format, UINT search_class);
typedef void (*create_desc_func)(char *buf, int buflen, ht_view *form);

typedef ht_data* (*create_replace_context_func)(ht_streamfile *file, FILEOFS ofs, UINT len, ht_view *form, UINT *return_repllen);

struct ht_search_method {
	char *name;
	UINT search_class;			// SC_*
	UINT search_mode_mask;		// SEARCHMODE_*
	HT_ATOM histid;
	create_form_func create_form;
	create_request_func create_request;
	create_desc_func create_desc;
};

struct ht_replace_method {
	char *name;
	HT_ATOM histid;
	create_form_func create_form;
	create_replace_context_func create_replace_context;
	process_func replace_process;
};

#include <stdlib.h>
#include <string.h>

bool test_str_to_ofs(FILEOFS *ofs, byte *str, UINT strlen, ht_format_viewer *format, char *desc)
{
#define TEST_STR_TO_OFS_MAXSTRLEN       128
	if (strlen>TEST_STR_TO_OFS_MAXSTRLEN) {
		throw new ht_io_exception("%s: expression too long (len %d, max %d)", desc, strlen, TEST_STR_TO_OFS_MAXSTRLEN);
		return false;
	}
	if (strlen>0) {
		char s[TEST_STR_TO_OFS_MAXSTRLEN+1];
		bin2str(s, str, strlen);
		if (!format->string_to_offset(s, ofs)) {
			throw new ht_io_exception("%s: invalid expression: '%s'", desc, s);
			return false;
		}
	}
	return true;
}

bool test_str_to_addr(fmt_vaddress *addr, byte *str, UINT strlen, ht_format_viewer *format, char *desc)
{
#define TEST_STR_TO_ADDR_MAXSTRLEN      128
	if (strlen>TEST_STR_TO_ADDR_MAXSTRLEN) {
		throw new ht_io_exception("%s: expression too long (len %d, max %d)", desc, strlen, TEST_STR_TO_ADDR_MAXSTRLEN);
		return false;
	}
	if (strlen>0) {
		char s[TEST_STR_TO_ADDR_MAXSTRLEN+1];
		bin2str(s, str, strlen);
		if (!format->string_to_address(s, addr)) {
			throw new ht_io_exception("%s: invalid expression: '%s'", desc, s);
			return false;
		}
	}
	return true;
}

/*
 *	CLASS ht_fxbin_search_request
 */

ht_view* create_form_hexascii(bounds *b, HT_ATOM histid)
{
	ht_hexascii_search_form *form=new ht_hexascii_search_form();
	form->init(b, 0, (ht_list*)find_atom(histid));
	return form;
}

ht_search_request* create_request_hexascii(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, UINT search_class)
{
	ht_hexascii_search_form *form=(ht_hexascii_search_form*)f;
	ht_hexascii_search_form_data d;
	form->databuf_get(&d);
	
	ht_fxbin_search_request *request;
	
	if (!d.str.textlen) {
		throw new ht_io_exception("%s: string is empty", "hex/ascii");
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
	form->databuf_get(&d);

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

ht_view* create_form_evalstr(bounds *b, HT_ATOM histid)
{
	ht_evalstr_search_form *form=new ht_evalstr_search_form();
	form->init(b, 0, (ht_list*)find_atom(histid));
	return form;
}

ht_search_request* create_request_evalstr(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, UINT search_class)
{
#define EVALSTR_MAXSTRLEN		256
	ht_evalstr_search_form *form=(ht_evalstr_search_form*)f;
	ht_evalstr_search_form_data d;
	form->databuf_get(&d);
	
	ht_fxbin_search_request *request = NULL;
		
	if (!d.str.textlen) {
		throw new ht_io_exception("%s: string is empty", "eval str");
	} else if (d.str.textlen<=EVALSTR_MAXSTRLEN) {
		char strbuf[EVALSTR_MAXSTRLEN+1];
		bin2str(strbuf, d.str.text, d.str.textlen);

		scalar_t r;
		str_t s;
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
			char *str;
			int pos;
			get_eval_error(&str, &pos);
			throw new ht_io_exception("eval error at pos %d: %s", pos, str);
		}
	} else {
		throw new ht_io_exception("%s: expression too long (len %d, max %d)", "eval str", d.str.textlen, EVALSTR_MAXSTRLEN);
	}
	return request;
}

void create_desc_evalstr(char *buf, int buflen, ht_view *f)
{
	ht_evalstr_search_form *form=(ht_evalstr_search_form*)f;
	ht_evalstr_search_form_data d;
	form->databuf_get(&d);

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

ht_fxbin_search_request::ht_fxbin_search_request(UINT search_class, UINT flags, UINT ds, byte *d) :
	ht_search_request(search_class, ST_FXBIN, flags)
{
	data_size=ds;
	data=(byte*)malloc(data_size);
	memmove(data, d, data_size);
}

ht_fxbin_search_request::~ht_fxbin_search_request()
{
	free(data);
}

object *ht_fxbin_search_request::duplicate()
{
	byte *dt=(byte*)malloc(data_size);
	memmove(dt, data, data_size);
	return new ht_fxbin_search_request(search_class, flags, data_size, dt);
}

/*
 *	CLASS ht_regex_search_request
 */
 
ht_view* create_form_vregex(bounds *b, HT_ATOM histid)
{
	ht_vregex_search_form *form=new ht_vregex_search_form();
	form->init(b, 0, (ht_list*)find_atom(histid));
	return form;
}

ht_search_request* create_request_vregex(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, UINT search_class)
{
#define VREGEX_MAXSTRLEN		256
	ht_vregex_search_form *form=(ht_vregex_search_form*)f;
	ht_vregex_search_form_data d;
	form->databuf_get(&d);

	ht_regex_search_request *request=NULL;

	if (!d.str.textlen) {
		throw new ht_io_exception("%s: string is empty", "regex");
	} else if (d.str.textlen <= VREGEX_MAXSTRLEN) {
		char strbuf[VREGEX_MAXSTRLEN+1];
		bin2str(strbuf, d.str.text, d.str.textlen);

		if (test_str_to_addr(&start->address, d.start.text, d.start.textlen, format, "start-address")
		&& test_str_to_addr(&end->address, d.end.text, d.end.textlen, format, "end-address")) {
			request = new ht_regex_search_request(search_class, d.options.state & 1 ? SF_REGEX_CASEINSENSITIVE : 0, strbuf);
		}
	} else {
		throw new ht_io_exception("%s: expression too long (size %d, max %d)", "regex", strlen, VREGEX_MAXSTRLEN);
	}
	return request;
}

void create_desc_vregex(char *buf, int buflen, ht_view *f)
{
	ht_vregex_search_form *form=(ht_vregex_search_form*)f;
	ht_vregex_search_form_data d;
	form->databuf_get(&d);

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

ht_regex_search_request::ht_regex_search_request(UINT search_class, UINT flags, char *regex) :
	ht_search_request(search_class, ST_REGEX, flags)
{
	rx_str=ht_strdup(regex);
	int r=regcomp(&rx, rx_str, REG_EXTENDED | ((flags & SF_REGEX_CASEINSENSITIVE) ? REG_ICASE : 0));
	if (r) throw new ht_regex_search_exception(r, &rx);
}

ht_regex_search_request::~ht_regex_search_request()
{
	regfree(&rx);
}

object *ht_regex_search_request::duplicate()
{
	return new ht_regex_search_request(search_class, flags, rx_str);
}

/*
 *	CLASS ht_regex_search_request
 */
 
ht_regex_search_exception::ht_regex_search_exception(int e, regex_t *r)
{
	errorcode=e;
	regex=r;
	
	char s[128];
	regerror(errorcode, regex, s, sizeof s-32);
	sprintf(rxerr, "error compiling regex: %s", s);
}
	
const char* ht_regex_search_exception::what()
{
	return rxerr;
}

/*
 *	CLASS ht_expr_search_request
 */

ht_view* create_form_expr(bounds *b, HT_ATOM histid)
{
	ht_expr_search_form *form = new ht_expr_search_form();
	form->init(b, 0, (ht_list*)find_atom(histid));
	return form;
}

ht_search_request* create_request_expr(search_pos *start, search_pos *end, ht_view *f, ht_format_viewer *format, UINT search_class)
{
#define EXPR_MAXSTRLEN		256
	ht_expr_search_form *form=(ht_expr_search_form*)f;
	ht_expr_search_form_data d;
	form->databuf_get(&d);
	
	ht_expr_search_request *request = NULL;

	if (!d.str.textlen) {
		throw new ht_io_exception("%s: string is empty", "expr");
	} else if (d.str.textlen <= EXPR_MAXSTRLEN) {
		char strbuf[EXPR_MAXSTRLEN+1];
		bin2str(strbuf, d.str.text, d.str.textlen);
		
		if (test_str_to_ofs(&start->offset, d.start.text, d.start.textlen, format, "start-offset")
		&& test_str_to_ofs(&end->offset, d.end.text, d.end.textlen, format, "end-offset")) {
			request = new ht_expr_search_request(search_class, 0, strbuf);
		}
	} else {
		throw new ht_io_exception("%s: expression too long (size %d, max %d)", "expr", strlen, EXPR_MAXSTRLEN);
	}
	return request;
}

void create_desc_expr(char *buf, int buflen, ht_view *f)
{
	ht_vregex_search_form *form=(ht_vregex_search_form*)f;
	ht_vregex_search_form_data d;
	form->databuf_get(&d);

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

ht_expr_search_request::ht_expr_search_request(UINT search_class, UINT flags, char *Expr) :
	ht_search_request(search_class, ST_EXPR, flags)
{
	expr = ht_strdup(Expr);
}

ht_expr_search_request::~ht_expr_search_request()
{
	if (expr) delete expr;
}

object *ht_expr_search_request::duplicate()
{
	return new ht_expr_search_request(search_class, flags, expr);
}

/*
 *	binary search
 */

void bufdowncase(byte *buf, dword len)
{
	for (dword i=0; i<len; i++) {
		if ((buf[i]>='A') && (buf[i]<='Z')) buf[i]+=32;
	}
}

#define SEARCH_BUF_SIZE 256*1024

ht_search_bin_context::~ht_search_bin_context()
{
	free(buf);
	free(pat);
}

ht_data* create_search_bin_context(ht_streamfile *file, FILEOFS ofs, UINT len, byte *pat, UINT patlen, UINT flags, UINT *return_ofs, bool *return_success)
{
	if (patlen > SEARCH_BUF_SIZE) return NULL;
	
	ht_search_bin_context *ctx = new ht_search_bin_context();
	ctx->file = file;
	ctx->file_end = false;
	ctx->ofs = ofs;
	ctx->flags = flags;
	ctx->len = len;
	ctx->pat = (byte*)malloc(patlen);
	memmove(ctx->pat, pat, patlen);
	ctx->patlen = patlen;

	ctx->o = ofs;

	if (ctx->flags & SFBIN_CASEINSENSITIVE) bufdowncase(ctx->pat, ctx->patlen);

	ctx->buf = (byte*)malloc(SEARCH_BUF_SIZE);

	ctx->return_ofs = return_ofs;
	ctx->return_success = return_success;

	file->seek(ctx->o);
	ctx->c = file->read(ctx->buf, SEARCH_BUF_SIZE);
	ctx->bufptr = ctx->buf;
	ctx->file_end = (ctx->c != SEARCH_BUF_SIZE);
	if (ctx->flags & SFBIN_CASEINSENSITIVE) bufdowncase(ctx->buf, ctx->c);

	return ctx;
}

bool search_bin_process(ht_data *context, ht_text *progress_indicator)
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

	int p = (ctx->o - ctx->ofs)*100/ctx->len;
	
	char status[64];
	sprintf(status, "%d %%", p);
	progress_indicator->settext(status);
	
	return true;
}

/*
 *	CLASS ht_hexascii_search_form
 */

void ht_hexascii_search_form::init(bounds *b, int options, ht_list *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_hexascii_search_form");

	bounds c;
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
	ht_string_list *opts=new ht_string_list();
	opts->init();
	opts->insert_string("~case insensitive");
//	opts->insert_string("find all occurencies (~idle)");
	option_boxes=new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state=options;
	option_boxes->databuf_set(&d);
	insert(option_boxes);
}

/*
 *	CLASS ht_evalstr_search_form
 */

void ht_evalstr_search_form::init(bounds *b, int options, ht_list *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_evalstr_search_form");

	bounds c;
/* string */
	c.x=0;
	c.y=1;
	c.w=40;
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
	strlabel->init(&c, "eval ~string", str);
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
	c.w=35;
	c.y=8;
	c.h=2;
	ht_string_list *opts=new ht_string_list();
	opts->init();
	opts->insert_string("~case insensitive");
//	opts->insert_string("find all occurencies (~idle)");
	option_boxes=new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state=options;
	option_boxes->databuf_set(&d);
	insert(option_boxes);
}

/*
 *	CLASS ht_vregex_search_form
 */

void ht_vregex_search_form::init(bounds *b, int options, ht_list *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_text_search_form");

	bounds c;
/* string */
	c.x=0;
	c.y=1;
	c.w=40;
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
	strlabel->init(&c, "~regular expression", str);
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
	c.w=35;
	c.y=8;
	c.h=2;
	ht_string_list *opts=new ht_string_list();
	opts->init();
	opts->insert_string("~case insensitive");
//	opts->insert_string("find all occurencies (~idle)");
	option_boxes=new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state=options;
	option_boxes->databuf_set(&d);
	insert(option_boxes);
}

/*
 *	CLASS ht_expr_search_form
 */

void	ht_expr_search_form::init(bounds *b, int options, ht_list *history=0)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_expr_search_form");

	bounds c;
/* string */
	c.x=0;
	c.y=1;
	c.w=40;
	c.h=1;
	str=new ht_strinputfield();
	str->init(&c, 256, history);
	insert(str);
/* string label */
	c.x=0;
	c.y=0;
	c.w=12;
	c.h=1;
	ht_label *strlabel=new ht_label();
	strlabel->init(&c, "~expression", str);
	insert(strlabel);
/* hint */
	c.x=0;
	c.y=2;
	c.w=b->w-2;
	c.h=1;
	ht_statictext *hint=new ht_statictext();
	hint->init(&c, "stops if expression evaluates to non-zero", 0);
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
	c.w=35;
	c.y=8;
	c.h=2;
	ht_string_list *opts=new ht_string_list();
	opts->init();
//	opts->insert_string("~case insensitive");
//	opts->insert_string("find all occurencies (~idle)");
	option_boxes=new ht_checkboxes();
	option_boxes->init(&c, opts);
	ht_checkboxes_data d;
	d.state=options;
	option_boxes->databuf_set(&d);
	insert(option_boxes);
}

/*
 *	CLASS ht_replace_hexascii_search_form
 */

ht_replace_bin_context::~ht_replace_bin_context()
{
	free(repl);
}

ht_view* create_form_replace_hexascii(bounds *b, HT_ATOM histid)
{
	ht_replace_hexascii_search_form *form=new ht_replace_hexascii_search_form();
	form->init(b, 0, (ht_list*)find_atom(histid));
	return form;
}

ht_data* create_replace_hexascii_context(ht_streamfile *file, FILEOFS ofs, UINT len, ht_view *form, UINT *return_repllen)
{
	ht_replace_hexascii_search_form_data d;
	form->databuf_get(&d);
	
	ht_replace_bin_context *ctx = (ht_replace_bin_context*)
	create_replace_bin_context(file, ofs, len, d.str.text, d.str.textlen, return_repllen);

	return ctx;
}

void ht_replace_hexascii_search_form::init(bounds *b, int options, ht_list *history)
{
	ht_group::init(b, VO_SELECTABLE, NULL);
	VIEW_DEBUG_NAME("ht_replace_hexascii_search_form");

	bounds c;
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

ht_search_method search_methods[] =
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

ht_search_request *search_dialog(ht_format_viewer *format, UINT searchmodes)
{
	ht_search_request *result = NULL;
	bounds b;
	b.w = 50;
	b.h = 15;
	b.x = (screen->size.w-b.w)/2;
	b.y = (screen->size.h-b.h)/2;
	ht_search_dialog *dialog = new ht_search_dialog();
	dialog->init(&b);

	bounds k;
	dialog->search_mode_xgroup->getbounds(&k);

	k.x = 0;
	k.y = 0;

	int modes = 0;
	int i = 0;
	ht_search_method *q = search_methods;
	while (q->name) {
		if (q->search_mode_mask & searchmodes) {
			bounds v = k;
			ht_view *form = q->create_form(&v, q->histid);
			dialog->insert_search_mode(i, q->name, form);
			modes++;
		}
		q++;
		i++;
	}
	
	dialog->select_search_mode(lastsearchmodeid);
	
	if (dialog->run(0)) {
		int modeid = dialog->get_search_modeid();
		lastsearchmodeid = modeid;

		ht_search_method *s = &search_methods[modeid];
		ht_view *form = dialog->get_search_modeform();

		search_pos start, end;

		try {
/* create history entry */
			if (s->create_desc) {
				char hist_desc[1024];	/* FIXME: possible buffer overflow */
				s->create_desc(hist_desc, sizeof hist_desc, form);
				insert_history_entry((ht_list*)find_atom(s->histid), hist_desc, form);
			}
/* search */
			switch (s->search_class) {
				case SC_PHYSICAL:
					start.offset=0;
					end.offset=0xffffffff;
					format->get_current_offset(&start.offset);
					break;
				case SC_VISUAL:
					start.address=0;
					end.address=0xffffffff;
					format->get_current_address(&start.address);
					break;
			}
			result = s->create_request(&start, &end, form, format, s->search_class);
		} catch (ht_exception *e) {
			errorbox("error: %s", e->what());
		}
	}
	dialog->done();
	delete dialog;
	return result;
}

ht_replace_method replace_methods[] =
{
	{ "bin: hex/ascii", 0, create_form_replace_hexascii,
		create_replace_hexascii_context, replace_bin_process },
	{ NULL }
};

void replace_dialog(ht_format_viewer *format, UINT searchmodes)
{
	bounds b;
	b.w = 50;
	b.h = 22;
	b.x = (screen->size.w-b.w)/2;
	b.y = (screen->size.h-b.h)/2;
	ht_replace_dialog *dialog = new ht_replace_dialog();
	dialog->init(&b);

	bounds k;
	dialog->search_mode_xgroup->getbounds(&k);

	k.x = 0;
	k.y = 0;

	int i;

	i = 0;
	ht_search_method *q = search_methods;
	while (q->name) {
		if ((q->search_mode_mask & searchmodes) &&
		(q->search_class == SC_PHYSICAL)) {
			bounds v = k;
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
		bounds v = k;
		ht_view *form = w->create_form(&v, w->histid);
		dialog->insert_replace_mode(i, w->name, form);
		w++;
		i++;
	}

	dialog->select_search_mode(lastsearchmodeid);
	dialog->select_replace_mode(lastreplacemodeid);

	if (dialog->run(0)) {
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
				char hist_desc[1024];	/* FIXME: possible buffer overflow */
				s->create_desc(hist_desc, sizeof hist_desc, sform);
				insert_history_entry((ht_list*)find_atom(s->histid), hist_desc, sform);
			}
/* search */
			start.offset=0;
			end.offset=0xffffffff;
			format->get_current_offset(&start.offset);

			request = s->create_request(&start, &end, sform, format, s->search_class);
		} catch (ht_exception *e) {
			errorbox("error: %s", e->what());
		}
		
		if (request) {
			UINT n = 0;
			FILEOFS so = start.offset, eo = end.offset;
			ht_physical_search_result *result;

			format->push_vs_history(format);
				
			try {
				bool replace_all = false;
				while ((result = (ht_physical_search_result*)format->psearch(request, so, eo))) {
					UINT irepllen = 0;
					bool do_replace = false;

					if (!replace_all) {
						format->show_search_result(result);
						int r = msgbox(btmask_yes+btmask_no+btmask_all+btmask_cancel, "confirmation", 0, align_center, "replace ?");
						format->pop_vs_history();
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
							delete result;
							break;
						}
					}

					if (replace_all || do_replace) {
						ht_data *rctx = r->create_replace_context(format->get_file(), result->offset, result->size, rform, &irepllen);
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
						n++;
					} else {
						so = result->offset + result->size;
					}
					delete result;
				}
			} catch (ht_exception *e) {
				errorbox("error: %s", e->what());
			}
		
			format->pop_vs_history();
			infobox("%d replacement(s) made", n);
		}
				
		if (request) delete request;
	}
	dialog->done();
	delete dialog;
}

#define REPLACE_COPY_BUF_SIZE	64*1024
ht_data* create_replace_bin_context(ht_streamfile *file, FILEOFS ofs, UINT len, byte *repl, UINT repllen, UINT *return_repllen)
{
	ht_replace_bin_context *ctx = new ht_replace_bin_context();
	ctx->file = file;
	ctx->ofs = ofs;
	ctx->len = len;
	ctx->repl = (byte*)malloc(repllen);
	memmove(ctx->repl, repl, repllen);
	ctx->repllen = repllen;
	if (repllen > len) {
		ctx->o = file->get_size();
	} else if (len > repllen) {
		ctx->o = ofs+len;
	}
	ctx->z = REPLACE_COPY_BUF_SIZE;
	if (len != repllen)
		ctx->buf = (byte*)malloc(REPLACE_COPY_BUF_SIZE);
	ctx->return_repllen = return_repllen;
	return ctx;
}

bool replace_bin_process(ht_data *context, ht_text *progress_indicator)
{
	char status[128];
	sprintf(status, "replacing...\n");
	progress_indicator->settext(status);
	
	ht_replace_bin_context *c = (ht_replace_bin_context*)context;
	if (c->repllen > c->len) {
/* grow */
		UINT size = c->file->get_size();
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
		c->file->read(c->buf, c->z);
		c->file->seek(c->o+c->repllen-c->len);
		c->file->write(c->buf, c->z);
			
		if (c->o > c->ofs + c->len) return true;
		
		c->file->seek(c->ofs);
		c->file->write(c->repl, c->repllen);
		free(c->buf);
	} else if (c->repllen < c->len) {
/* shrink */
		UINT size = c->file->get_size();
		if (c->o == c->ofs + c->len) {
			c->file->seek(c->ofs);
			c->file->write(c->repl, c->repllen);
		}
		
		if (c->z > size - c->o) {
			c->z = size - c->o;
		}
		c->file->seek(c->o);
		c->file->read(c->buf, c->z);
		c->file->seek(c->o - (c->len - c->repllen));
		c->file->write(c->buf, c->z);
		c->o += REPLACE_COPY_BUF_SIZE;
		
		if (c->z == REPLACE_COPY_BUF_SIZE) return true;
		
		c->file->truncate(size - (c->len - c->repllen));
		free(c->buf);
	} else {
		c->file->seek(c->ofs);
		c->file->write(c->repl, c->repllen);
	}
	if (c->return_repllen) *c->return_repllen = c->repllen;
	return false;
}

/*
 *	CLASS ht_search_dialog
 */

void ht_search_dialog::init(bounds *b)
{
	ht_dialog::init(b, "search", FS_KILLER | FS_TITLE | FS_MOVE);
	VIEW_DEBUG_NAME("ht_search_dialog");

	smodecount=0;

	bounds c;
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

	c.x=1;
	c.y=3;
	c.w=size.w-4;
	c.h=MIN(10, size.h-4);
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
		search_mode_popup->databuf_get(&data);
		if ((int)data.cursor_id!=smodeidx) {
			smodeidx=data.cursor_id;
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

void ht_search_dialog::insert_search_mode(int id, char *desc, ht_view *v)
{
	if (smodecount<MAX_SEARCH_DIALOG_MODES) {
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
	d.cursor_id=smodeidx;
	search_mode_popup->databuf_set(&d);
	focus(smodes[smodeidx].view);
	sendmsg(msg_dirtyview, 0);
}

/*
 *	CLASS ht_replace_dialog
 */

void ht_replace_dialog::init(bounds *b)
{
	ht_search_dialog::init(b);

	rmodecount=0;

	bounds c;
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
	if (msg->msg==msg_keypressed) {
		ht_search_dialog::handlemsg(msg);
		ht_listpopup_data data;
		replace_mode_popup->databuf_get(&data);
		if ((int)data.cursor_id!=rmodeidx) {
			rmodeidx=data.cursor_id;
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

void ht_replace_dialog::insert_replace_mode(int id, char *desc, ht_view *v)
{
	if (rmodecount<MAX_REPLACE_DIALOG_MODES) {
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
	d.cursor_id=rmodeidx;
	replace_mode_popup->databuf_set(&d);
//	focus(rmodes[rmodeidx].view);
	sendmsg(msg_dirtyview, 0);
}

