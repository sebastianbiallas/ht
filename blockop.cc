/*
 *	HT Editor
 *	blockop.cc
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

#include <stdlib.h>
#include <string.h>

#include "blockop.h"
#include "cmds.h"
#include "htatom.h"
#include "htctrl.h"
#include "htendian.h"
#include "htexcept.h"
#include "hthist.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htstring.h"
#include "process.h"
#include "snprintf.h"

#include "evalx.h"

/*
 *	CLASS ht_blockop_dialog
 */

void ht_blockop_dialog::init(bounds *b, FILEOFS pstart, FILEOFS pend, ht_list *history)
{
	ht_dialog::init(b, "operate on block", FS_TITLE | FS_KILLER | FS_MOVE);
	bounds c;

	bool prerange=(pend>pstart);

	ht_statictext *text;

	ht_label *s;
	
	ht_list *addrhist=(ht_list*)find_atom(HISTATOM_GOTO);
/* start */
	c=*b;
	c.h=1;
	c.w=13;
	c.x=7;
	c.y=1;
	start=new ht_strinputfield();
	start->init(&c, 64, addrhist);
	insert(start);
	if (prerange) {
		char t[16];
		ht_snprintf(t, sizeof t, "0x%x", pstart);
		ht_inputfield_data d;
		d.textlen=strlen(t);
		d.text=(byte*)t;
		start->databuf_set(&d);
	}

/* start_desc */
	c.x=1;
	c.w=6;
	s=new ht_label();
	s->init(&c, "~start", start);
	insert(s);
	
/* end */
	c=*b;
	c.h=1;
	c.w=13;
	c.x=27;
	c.y=1;
	end=new ht_strinputfield();
	end->init(&c, 64, addrhist);
	insert(end);
	if (prerange) {
		char t[16];
		ht_snprintf(t, sizeof t, "0x%x", pend);
		ht_inputfield_data d;
		d.textlen=strlen(t);
		d.text=(byte*)t;
		end->databuf_set(&d);
	}


/* end_desc */
	c.x=23;
	c.w=3;
	s=new ht_label();
	s->init(&c, "~end", end);
	insert(s);

/* mode */
	c=*b;
	c.h=1;
	c.w=16;
	c.x=7;
	c.y=3;
	mode=new ht_listpopup();
	mode->init(&c);
	mode->insertstring("byte (8-bit)");
	mode->insertstring("word (16-bit)");
	mode->insertstring("dword (32-bit)");
	mode->insertstring("string");
	insert(mode);

/* mode_desc */
	c.x=1;
	c.w=12;
	c.y=3;
	s=new ht_label();
	s->init(&c, "~mode", mode);
	insert(s);

/* action_expl */
	c=*b;
	c.x=1;
	c.y=5;
	c.w-=3;
	c.h=1;
	text=new ht_statictext();
	text->init(&c, "set each element to", align_left);
	insert(text);
	
/* action */
	ht_list *ehist=(ht_list*)find_atom(HISTATOM_EVAL_EXPR);

	c=*b;
	c.h=1;
	c.w=30;
	c.x=7;
	c.y=6;
	action=new ht_strinputfield();
	action->init(&c, 64, ehist);
	insert(action);

/* action_desc */
	c.x=1;
	c.w=27;
	c.y=6;
	s=new ht_label();
	s->init(&c, "e~xpr", action);
	insert(s);

/* help */
	c=*b;
	c.x=1;
	c.y=8;
	c.w-=c.x+2;
	c.h-=c.y+2;
	text=new ht_statictext();
	text->init(&c,
		"special vars:          special funcs:\n"
		"o - file offset        readbyte(ofs)\n"
		"i - iteration index    readstring(ofs, n)", align_left);
	insert(text);
}

void ht_blockop_dialog::done()
{
	ht_dialog::done();
}

struct ht_blockop_dialog_data {
	ht_inputfield_data start;
	ht_inputfield_data end;
	ht_listpopup_data mode;
	ht_inputfield_data action;
};

/*
 *   blockop_dialog
 */

static dword blockop_i;
static dword blockop_o;
static bool blockop_expr_is_const;

int blockop_symbol_eval(scalar_t *r, char *symbol)
{
	if (strcmp(symbol, "i")==0) {
		r->type=SCALAR_INT;
		r->scalar.integer.value=to_qword(blockop_i);
		r->scalar.integer.type=TYPE_UNKNOWN;
		blockop_expr_is_const=false;
		return 1;
	} else if (strcmp(symbol, "o")==0) {
		r->type=SCALAR_INT;
		r->scalar.integer.value=to_qword(blockop_o);
		r->scalar.integer.type=TYPE_UNKNOWN;
		blockop_expr_is_const=false;
		return 1;
	}
	return 0;
}

int func_readbyte(scalar_t *result, int_t *offset)
{
	ht_streamfile *f=(ht_streamfile*)eval_get_context();
	byte b;
	if ((f->seek(QWORD_GET_INT(offset->value))!=0) || (f->read(&b, 1)!=1)) {
		set_eval_error("i/o error (requested %d, read %d from ofs %08x)", 1, 0, offset->value);
		return 0;
	}
	scalar_create_int_c(result, b);
	return 1;
}

int func_readstring(scalar_t *result, int_t *offset, int_t *len)
{
	ht_streamfile *f=(ht_streamfile*)eval_get_context();

	UINT l=QWORD_GET_INT(len->value);
	void *buf=malloc(l);	/* FIXME: may be too slow... */

	if (buf) {
		str_t s;
		UINT c = 0;
		if ((f->seek(QWORD_GET_INT(offset->value))!=0) || ( (c=f->read(buf, l)) !=l)) {
			free(buf);
			set_eval_error("i/o error (requested %d, read %d from ofs %08x)", l, c, offset->value);
			return 0;
		}
		s.value=(char*)buf;
		s.len=l;
		scalar_create_str(result, &s);
		free(buf);
		return 1;
	}
	set_eval_error("out of memory");
	return 0;
}

int blockop_func_eval(scalar_t *result, char *name, scalarlist_t *params)
{
/* FIXME: non-constant funcs (e.g. rand()) should
   set blockop_expr_is_const to false */
	evalfunc_t myfuncs[] = {
		{"readbyte", (void*)&func_readbyte, {SCALAR_INT}},
		{"readstring", (void*)&func_readstring, {SCALAR_INT, SCALAR_INT}},
		{NULL}
	};
	
	blockop_expr_is_const=false;
		
	return std_eval_func_handler(result, name, params, myfuncs);
}

/*
 *	BLOCKOP STRING
 */

class ht_blockop_str_context: public ht_data {
public:
	ht_streamfile *file;

	FILEOFS ofs;
	UINT len;

	UINT size;
	bool netendian;

	char *action;

	UINT i;
	FILEOFS o;

	bool expr_const;
	str_t v;

	~ht_blockop_str_context()
	{
		free(action);
		if (expr_const) string_destroy(&v);
	}
};

ht_data *create_blockop_str_context(ht_streamfile *file, FILEOFS ofs, UINT len, UINT size, bool netendian, char *action)
{
	ht_blockop_str_context *ctx = new ht_blockop_str_context();
	ctx->file = file;
	ctx->ofs = ofs;
	ctx->len = len;
	ctx->size = size;
	ctx->netendian = netendian;
	ctx->action = strdup(action);

	ctx->i = 0;
	ctx->o = ofs;

	blockop_expr_is_const = true;

	scalar_t r;
	if (!eval(&r, action, blockop_func_eval, blockop_symbol_eval, file)) {
		char *s;
		int p;
		get_eval_error(&s, &p);
		throw new ht_io_exception("error evaluating '%s': %s at %d", action, s, p);
	}

	ctx->expr_const = blockop_expr_is_const;

	if (ctx->expr_const) {
		scalar_context_str(&r, &ctx->v);
	}
	scalar_destroy(&r);
	return ctx;
}

#define BLOCKOP_STR_MAX_ITERATIONS 1024
bool blockop_str_process(ht_data *context, ht_text *progress_indicator)
{
	char status[64];
	ht_blockop_str_context *ctx = (ht_blockop_str_context*)context;
	if (ctx->expr_const) {
		ht_snprintf(status, sizeof status, "operating (constant string)... %d%% complete", (ctx->o-ctx->ofs) * 100 / ctx->len);
		progress_indicator->settext(status);
		for (UINT i=0; i < BLOCKOP_STR_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			UINT s = ctx->v.len;
			if (ctx->o + s > ctx->ofs + ctx->len) s = ctx->ofs + ctx->len - ctx->o;
			
			ctx->file->seek(ctx->o);
			if (ctx->file->write(ctx->v.value, s)!=s) {
				throw new ht_io_exception("blockop_str(): write error at pos %08x, size %08x", ctx->o, s);
			}
			ctx->o += s;
		} else {
			return false;
		}
	} else {
		ht_snprintf(status, sizeof status, "operating (variable string)... %d%% complete", (ctx->o-ctx->ofs) * 100 / ctx->len);
		progress_indicator->settext(status);
		scalar_t r;
		str_t sr;
		for (UINT i=0; i < BLOCKOP_STR_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			blockop_i = ctx->i;
			blockop_o = ctx->o;
			if (!eval(&r, ctx->action, blockop_func_eval, blockop_symbol_eval, ctx->file)) {
				char *s;
				int p;
				get_eval_error(&s, &p);
				throw new ht_io_exception("error evaluating '%s': %s at %d", ctx->action, s, p);
			}
			scalar_context_str(&r, &sr);
			scalar_destroy(&r);

			UINT s = sr.len;
			if (ctx->o+s > ctx->ofs+ctx->len) s = ctx->ofs+ctx->len-ctx->o;

			ctx->file->seek(ctx->o);
			if (ctx->file->write(sr.value, s)!=s) {
				throw new ht_io_exception("blockop_str(): write error at pos %08x, size %08x", ctx->o, s);
			}
			string_destroy(&sr);
			ctx->o += s;
			ctx->i++;
		} else {
			return false;
		}
	}
	return true;
}

/*
 *	BLOCKOP INTEGER
 */

class ht_blockop_int_context: public ht_data {
public:
	ht_streamfile *file;

	FILEOFS ofs;
	UINT len;

	UINT size;
	endianess endian;

	char *action;

	UINT i;
	FILEOFS o;

	bool expr_const;
	UINT v;

	~ht_blockop_int_context()
	{
		free(action);
	}
};

ht_data *create_blockop_int_context(ht_streamfile *file, FILEOFS ofs, UINT len, UINT size, endianess endian, char *action)
{
	ht_blockop_int_context *ctx = new ht_blockop_int_context();
	ctx->file = file;
	ctx->ofs = ofs;
	ctx->len = len;
	ctx->size = size;
	ctx->endian = endian;
	ctx->action = ht_strdup(action);

	ctx->i = 0;
	ctx->o = ofs;

	blockop_expr_is_const = true;
	
	scalar_t r;
	int_t ir;
	if (!eval(&r, action, blockop_func_eval, blockop_symbol_eval, file)) {
		char *s;
		int p;
		get_eval_error(&s, &p);
		throw new ht_io_exception("error evaluating '%s': %s at %d", action, s, p);
	}

	ctx->expr_const = blockop_expr_is_const;
	
	if (ctx->expr_const) {
		scalar_context_int(&r, &ir);
		ctx->v = QWORD_GET_INT(ir.value);
	}
	scalar_destroy(&r);
	return ctx;
}

#define BLOCKOP_INT_MAX_ITERATIONS	1024
bool blockop_int_process(ht_data *context, ht_text *progress_indicator)
{
	ht_blockop_int_context *ctx = (ht_blockop_int_context*)context;
	char status[64];
	if (ctx->expr_const) {
		ht_snprintf(status, sizeof status, "operating (constant integer)... %d%% complete", (ctx->o-ctx->ofs) * 100 / ctx->len);
		progress_indicator->settext(status);
		byte ibuf[4];
		create_foreign_int(ibuf, ctx->v, ctx->size, ctx->endian);
		ctx->file->seek(ctx->o);
		for (UINT i=0; i < BLOCKOP_INT_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			UINT s = ctx->size;
			if (ctx->o + s > ctx->ofs + ctx->len) s = ctx->ofs + ctx->len - ctx->o;
			if (ctx->file->write(ibuf, s)!=s) {
				throw new ht_io_exception("blockop_int(): write error at pos %08x, size %08x", ctx->o, s);
			}
			ctx->o += s;
		} else {
			return false;
		}
	} else {
		ht_snprintf(status, sizeof status, "operating (variable integer)... %d%% complete", (ctx->o-ctx->ofs) * 100 / ctx->len);
		progress_indicator->settext(status);
		scalar_t r;
		int_t ir;
		for (UINT i=0; i < BLOCKOP_INT_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			blockop_o = ctx->o;
			blockop_i = ctx->i;
			if (!eval(&r, ctx->action, blockop_func_eval, blockop_symbol_eval, ctx->file)) {
				char *s;
				int p;
				get_eval_error(&s, &p);
				throw new ht_io_exception("error evaluating '%s': %s at %d", ctx->action, s, p);
			}
			scalar_context_int(&r, &ir);
			scalar_destroy(&r);
			ctx->v=QWORD_GET_INT(ir.value);

			UINT s = ctx->size;
			if (ctx->o+s > ctx->ofs+ctx->len) s = ctx->ofs+ctx->len-ctx->o;

			byte ibuf[4];
			create_foreign_int(ibuf, ctx->v, ctx->size, ctx->endian);
			if ((ctx->file->seek(ctx->o) != 0) || (ctx->file->write(ibuf, s)!=s)) {
				throw new ht_io_exception("blockop_int(): write error at pos %08x, size %08x", ctx->o, s);
			}
			ctx->o += s;
			ctx->i++;
		} else {
			return false;
		}
	}
	return true;
}

/*
 *
 */

bool format_string_to_offset_if_avail(ht_format_viewer *format, byte *string, int stringlen, char *string_desc, FILEOFS *ofs)
{
	if (string && *string && stringlen<64) {
		char str[64];
		memmove(str, string, stringlen);
		str[stringlen]=0;
		if (!format->string_to_offset(str, ofs)) {
			errorbox("%s: '%s' doesn't seem to be a valid offset", string_desc, &str);
			return false;
		}
		return true;
	}
	return false;
}
		
		
void blockop_dialog(ht_format_viewer *format, FILEOFS pstart, FILEOFS pend)
{
	bounds b;
	b.w=50;
	b.h=15;
	b.x=(screen->size.w-b.w)/2;
	b.y=(screen->size.h-b.h)/2;
	
	ht_blockop_dialog *d=new ht_blockop_dialog();
	d->init(&b, pstart, pend, 0);
	if (d->run(false)) {
		ht_blockop_dialog_data t;
		d->databuf_get(&t);
		
		ht_streamfile *file=format->get_file();

		baseview->sendmsg(cmd_edit_mode_i, file, NULL);
		
		if (file->get_access_mode() & FAM_WRITE) {
			FILEOFS start=pstart, end=pend;

			if (format_string_to_offset_if_avail(format, t.start.text, t.start.textlen, "start", &start) &&
			format_string_to_offset_if_avail(format, t.end.text, t.end.textlen, "end", &end)) {
				int esize=0;
				int esizes[3]={4, 2, 1};
				switch (t.mode.cursor_id) {
/* element type: byte */
					case 0: esize++;
/* element type: word */
					case 1: esize++;
/* element type: dword */
					case 2: {
						char a[256];
						bin2str(a, t.action.text, MIN(sizeof a, t.action.textlen));
						insert_history_entry((ht_list*)find_atom(HISTATOM_EVAL_EXPR), a, NULL);
						
						char addr[128];
						bin2str(addr, t.start.text, MIN(sizeof addr, t.start.textlen));
						insert_history_entry((ht_list*)find_atom(HISTATOM_GOTO), addr, NULL);
						bin2str(addr, t.end.text, MIN(sizeof addr, t.end.textlen));
						insert_history_entry((ht_list*)find_atom(HISTATOM_GOTO), addr, NULL);
						
						esize = esizes[esize];
						ht_data *ctx = NULL;
						try {
							ctx = create_blockop_int_context(file, start, end-start, esize, little_endian, a);
							if (ctx) {
								/*bool b = */execute_process(blockop_int_process, ctx);
							}
						} catch (ht_exception *e) {
							errorbox("error: %s", e->what());
						}
						if (ctx) delete ctx;
						break;
					}
/* element type: string */
					case 3: {
						char a[256];
						bin2str(a, t.action.text, MIN(sizeof a, t.action.textlen));
						insert_history_entry((ht_list*)find_atom(HISTATOM_EVAL_EXPR), a, NULL);

						char addr[128];
						bin2str(addr, t.start.text, MIN(sizeof addr, t.start.textlen));
						insert_history_entry((ht_list*)find_atom(HISTATOM_GOTO), addr, NULL);
						bin2str(addr, t.end.text, MIN(sizeof addr, t.end.textlen));
						insert_history_entry((ht_list*)find_atom(HISTATOM_GOTO), addr, NULL);
						
						ht_data *ctx = NULL;
						try {
							ctx = create_blockop_str_context(file, start, end-start, esize, little_endian, a);
							if (ctx) {
								/*bool b = */execute_process(blockop_str_process, ctx);
							}
						} catch (ht_exception *e) {
							errorbox("error: %s", e->what());
						}
						if (ctx) delete ctx;
						break;
					}
					default:
						errorbox("mode %d not supported", t.mode.cursor_id);
				}
			}
		}
	}

	d->done();
	delete d;
}

