/*
 *	HT Editor
 *	blockop.cc
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

#include <stdlib.h>
#include <string.h>

#include "blockop.h"
#include "cmds.h"
#include "atom.h"
#include "htctrl.h"
#include "endianess.h"
#include "hteval.h"
#include "except.h"
#include "hthist.h"
#include "htiobox.h"
#include "keyb.h"
#include "strtools.h"
#include "htprocess.h"
#include "snprintf.h"

#include "evalx.h"

/*
 *	CLASS ht_blockop_dialog
 */

void ht_blockop_dialog::init(Bounds *b, FileOfs pstart, FileOfs pend, List *history)
{
	ht_dialog::init(b, "operate on block", FS_TITLE | FS_KILLER | FS_MOVE);
	Bounds c;

	bool prerange = (pend > pstart);

	ht_statictext *text;

	ht_label *s;
	
	List *addrhist = (List*)getAtomValue(HISTATOM_GOTO);
	/* start */
	c = *b;
	c.h = 1;
	c.w = 13;
	c.x = 7;
	c.y = 1;
	start = new ht_strinputfield();
	start->init(&c, 64, addrhist);
	insert(start);
	if (prerange) {
		char t[32];
		ht_snprintf(t, sizeof t, "0x%qx", pstart);
		ht_inputfield_data d;
		d.textlen = strlen(t);
		d.text = (byte*)t;
		start->databuf_set(&d, sizeof d);
	}

	/* start_desc */
	c.x = 1;
	c.w = 6;
	s = new ht_label();
	s->init(&c, "~start", start);
	insert(s);
	
	/* end */
	c = *b;
	c.h = 1;
	c.w = 13;
	c.x = 27;
	c.y = 1;
	end=new ht_strinputfield();
	end->init(&c, 64, addrhist);
	insert(end);
	if (prerange) {
		char t[32];
		ht_snprintf(t, sizeof t, "0x%qx", pend);
		ht_inputfield_data d;
		d.textlen = strlen(t);
		d.text = (byte*)t;
		end->databuf_set(&d, sizeof d);
	}


	/* end_desc */
	c.x = 23;
	c.w = 3;
	s = new ht_label();
	s->init(&c, "~end", end);
	insert(s);

	/* mode */
	c = *b;
	c.h = 1;
	c.w = 16;
	c.x = 7;
	c.y = 3;
	mode = new ht_listpopup();
	mode->init(&c);
	mode->insertstring("byte (8-bit)");
	mode->insertstring("word (16-bit)");
	mode->insertstring("dword (32-bit)");
	mode->insertstring("qword (64-bit)");
	mode->insertstring("string");
	insert(mode);

	/* mode_desc */
	c.x = 1;
	c.w = 12;
	c.y = 3;
	s = new ht_label();
	s->init(&c, "~mode", mode);
	insert(s);

	/* action_expl */
	c = *b;
	c.x = 1;
	c.y = 5;
	c.w -= 3;
	c.h = 1;
	text = new ht_statictext();
	text->init(&c, "set each element to", align_left);
	insert(text);
	
	/* action */
	List *ehist = (List*)getAtomValue(HISTATOM_EVAL_EXPR);

	c = *b;
	c.h = 1;
	c.w = 40;
	c.x = 7;
	c.y = 6;
	action = new ht_strinputfield();
	action->init(&c, 4096, ehist);
	insert(action);

	/* action_desc */
	c.x = 1;
	c.w = 27;
	c.y = 6;
	s = new ht_label();
	s->init(&c, "e~xpr", action);
	insert(s);

	/* help */
	/*	c=*b;
	c.x=1;
	c.y=8;
	c.w-=c.x+2;
	c.h-=c.y+2;
	text=new ht_statictext();
	text->init(&c,
		"special vars:          special funcs:\n"
		"o - file offset        readbyte(ofs)\n"
		"i - iteration index    readstring(ofs, n)", align_left);
	insert(text);*/
	/* functions */
	ht_button *bhelp = new ht_button();
	c = *b;
	c.x = 1;
	c.y = 8;
	c.w = 12;
	c.h = 2;
	bhelp->init(&c, "~Functions", 100);
	insert(bhelp);
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

static FileOfs blockop_i;
static FileOfs blockop_o;
static bool blockop_expr_is_const;

static bool blockop_symbol_eval(eval_scalar *r, char *symbol)
{
	if (strcmp(symbol, "i") == 0) {
		r->type = SCALAR_INT;
		r->scalar.integer.value = blockop_i;
		r->scalar.integer.type = TYPE_UNKNOWN;
		blockop_expr_is_const = false;
		return true;
	} else if (strcmp(symbol, "o") == 0) {
		r->type = SCALAR_INT;
		r->scalar.integer.value = blockop_o;
		r->scalar.integer.type = TYPE_UNKNOWN;
		blockop_expr_is_const = false;
		return true;
	}
	return false;
}

static int func_readint(eval_scalar *result, eval_int *offset, int size, Endianess e)
{	
	File *f = (File*)eval_get_context();
	byte buf[8];
	try {
		f->seek(offset->value);
		f->readx(buf, size);
	} catch (const IOException&) {
		set_eval_error("i/o error (couldn't read %d bytes from ofs %qd (0x%qx))", size, offset->value, offset->value);
		return 0;
	}
	scalar_create_int_q(result, createHostInt64(buf, size, e));
	return 1;
}

static int func_readbyte(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 1, little_endian);
}

static int func_read16le(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 2, little_endian);
}

static int func_read32le(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 4, little_endian);
}

static int func_read64le(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 8, little_endian);
}

static int func_read16be(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 2, big_endian);
}

static int func_read32be(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 4, big_endian);
}

static int func_read64be(eval_scalar *result, eval_int *offset)
{
	return func_readint(result, offset, 8, big_endian);
}

static int func_readstring(eval_scalar *result, eval_int *offset, eval_int *len)
{
	File *f = (File*)eval_get_context();

	uint l = len->value;
	void *buf = malloc(l);	/* FIXME: may be too slow... */

	if (buf) {
		eval_str s;
		uint c = 0;
		try {
			f->seek(offset->value);
			f->readx(buf, l);
		} catch (const IOException&) {
			free(buf);
			set_eval_error("i/o error (couldn't read %d bytes from ofs %d (0x%qx))", l, c, offset->value, offset->value);
			return 0;
		}
		s.value = (char*)buf;
		s.len = l;
		scalar_create_str(result, &s);
		free(buf);
		return 1;
	}
	set_eval_error("out of memory");
	return 0;
}

static bool blockop_func_eval(eval_scalar *result, char *name, eval_scalarlist *params)
{
	/* FIXME: non-constant funcs (e.g. rand()) should
	   set blockop_expr_is_const to false */
	eval_func myfuncs[] = {
		{"i", 0, {SCALAR_INT}, "iteration index"},
		{"o", 0, {SCALAR_INT}, "current offset"},
		{"readbyte", (void*)&func_readbyte, {SCALAR_INT}, "read byte from offset"},
		{"read16le", (void*)&func_read16le, {SCALAR_INT}, "read little endian 16 bit word from offset"},
		{"read32le", (void*)&func_read32le, {SCALAR_INT}, "read little endian 32 bit word from offset"},
		{"read64le", (void*)&func_read64le, {SCALAR_INT}, "read little endian 64 bit word from offset"},
		{"read16be", (void*)&func_read16be, {SCALAR_INT}, "read big endian 16 bit word from offset"},
		{"read32be", (void*)&func_read32be, {SCALAR_INT}, "read big endian 32 bit word from offset"},
		{"read64be", (void*)&func_read64be, {SCALAR_INT}, "read big endian 64 bit word from offset"},
		{"readstring", (void*)&func_readstring, {SCALAR_INT, SCALAR_INT}, "read string (offset, length)"},
		{NULL}
	};
	
	blockop_expr_is_const = false;
		
	return std_eval_func_handler(result, name, params, myfuncs);
}

/*
 *	BLOCKOP STRING
 */

class ht_blockop_str_context: public Object {
public:
	File *file;

	FileOfs ofs;
	uint len;

	uint size;
	bool netendian;

	char *action;

	uint i;
	FileOfs o;

	bool expr_const;
	eval_str v;

	~ht_blockop_str_context()
	{
		free(action);
		if (expr_const) string_destroy(&v);
	}
};

Object *create_blockop_str_context(File *file, FileOfs ofs, uint len, uint size, bool netendian, char *action)
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

	// test if first eval works
	blockop_i = ctx->i;
	blockop_o = ctx->o;
	eval_scalar r;
	if (!eval(&r, action, blockop_func_eval, blockop_symbol_eval, file)) {
		const char *s;
		int p;
		get_eval_error(&s, &p);
		throw MsgfException("error evaluating '%s': %s at %d", action, s, p);
	}

	ctx->expr_const = blockop_expr_is_const;

	if (ctx->expr_const) {
		scalar_context_str(&r, &ctx->v);
	}
	scalar_destroy(&r);
	return ctx;
}

#define BLOCKOP_STR_MAX_ITERATIONS 1024
bool blockop_str_process(Object *context, ht_text *progress_indicator)
{
	char status[64];
	ht_blockop_str_context *ctx = (ht_blockop_str_context*)context;
	if (ctx->expr_const) {
		ht_snprintf(status, sizeof status, "operating (constant string)... %d%% complete", (int)(((double)(ctx->o-ctx->ofs)) * 100 / ctx->len));
		progress_indicator->settext(status);
		for (uint i=0; i < BLOCKOP_STR_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			uint s = ctx->v.len;
			if (ctx->o + s > ctx->ofs + ctx->len) s = ctx->ofs + ctx->len - ctx->o;
			
			ctx->file->seek(ctx->o);
			ctx->file->writex(ctx->v.value, s);
/*			!=s) {
				throw ht_io_exception("blockop_str(): write error at pos %08qx, size %08qx", ctx->o, s);
			}*/
			ctx->o += s;
		} else {
			return false;
		}
	} else {
		ht_snprintf(status, sizeof status, "operating (variable string)... %d%% complete", (int)(((double)(ctx->o-ctx->ofs)) * 100 / ctx->len));
		progress_indicator->settext(status);
		eval_scalar r;
		eval_str sr;
		for (uint i=0; i < BLOCKOP_STR_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			blockop_i = ctx->i;
			blockop_o = ctx->o;
			if (!eval(&r, ctx->action, blockop_func_eval, blockop_symbol_eval, ctx->file)) {
				const char *s;
				int p;
				get_eval_error(&s, &p);
				throw MsgfException("error evaluating '%s': %s at %d", ctx->action, s, p);
			}
			scalar_context_str(&r, &sr);
			scalar_destroy(&r);

			uint s = sr.len;
			if (ctx->o+s > ctx->ofs+ctx->len) s = ctx->ofs+ctx->len-ctx->o;

			ctx->file->seek(ctx->o);
			ctx->file->writex(sr.value, s);
/*			!=s) {
				throw ht_io_exception("blockop_str(): write error at pos %08x, size %08x", ctx->o, s);
			}*/
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

class ht_blockop_int_context: public Object {
public:
	File *file;

	FileOfs ofs;
	uint len;

	uint size;
	Endianess endian;

	char *action;

	uint i;
	FileOfs o;

	bool expr_const;
	uint64 v;

	~ht_blockop_int_context()
	{
		free(action);
	}
};

Object *create_blockop_int_context(File *file, FileOfs ofs, uint len, uint size, Endianess endian, char *action)
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

	// test if first eval works
	blockop_i = ctx->i;
	blockop_o = ctx->o;
	eval_scalar r;
	eval_int ir;
	if (!eval(&r, action, blockop_func_eval, blockop_symbol_eval, file)) {
		const char *s;
		int p;
		get_eval_error(&s, &p);
		throw MsgfException("error evaluating '%s': %s at %d", action, s, p);
	}

	ctx->expr_const = blockop_expr_is_const;
	
	if (ctx->expr_const) {
		scalar_context_int(&r, &ir);
		ctx->v = ir.value;
	}
	scalar_destroy(&r);
	return ctx;
}

#define BLOCKOP_INT_MAX_ITERATIONS	1024
bool blockop_int_process(Object *context, ht_text *progress_indicator)
{
	ht_blockop_int_context *ctx = (ht_blockop_int_context*)context;
	char status[64];
	if (ctx->expr_const) {		
		ht_snprintf(status, sizeof status, "operating (constant integer)... %d%% complete", (int)(((double)(ctx->o-ctx->ofs)) * 100 / ctx->len));
		progress_indicator->settext(status);
		byte ibuf[8];
		createForeignInt64(ibuf, ctx->v, ctx->size, ctx->endian);
		ctx->file->seek(ctx->o);
		for (uint i=0; i < BLOCKOP_INT_MAX_ITERATIONS; i++) {
			if (ctx->o < ctx->ofs + ctx->len) {
				uint s = ctx->size;
				if (ctx->o + s > ctx->ofs + ctx->len) s = ctx->ofs + ctx->len - ctx->o;
				ctx->file->writex(ibuf, s);
				ctx->o += s;
			} else {
				return false;
			}
		}
		
	} else {
		ht_snprintf(status, sizeof status, "operating (variable integer)... %d%% complete", (int)(((double)(ctx->o-ctx->ofs)) * 100 / ctx->len));
		progress_indicator->settext(status);
		eval_scalar r;
		eval_int ir;
		for (uint i=0; i < BLOCKOP_INT_MAX_ITERATIONS; i++)
		if (ctx->o < ctx->ofs + ctx->len) {
			blockop_o = ctx->o;
			blockop_i = ctx->i;
			if (!eval(&r, ctx->action, blockop_func_eval, blockop_symbol_eval, ctx->file)) {
				const char *s;
				int p;
				get_eval_error(&s, &p);
				throw MsgfException("error evaluating '%s': %s at %d", ctx->action, s, p);
			}
			scalar_context_int(&r, &ir);
			scalar_destroy(&r);
			ctx->v = ir.value;

			uint s = ctx->size;
			if (ctx->o+s > ctx->ofs+ctx->len) s = ctx->ofs + ctx->len - ctx->o;

			byte ibuf[8];
			createForeignInt64(ibuf, ctx->v, ctx->size, ctx->endian);
			ctx->file->seek(ctx->o);
			ctx->file->writex(ibuf, s);
			ctx->o += s;
			ctx->i++;
		} else {
			return false;
		}
	}
	return true;
}

bool format_string_to_offset_if_avail(ht_format_viewer *format, byte *string, int stringlen, const char *string_desc, FileOfs *ofs)
{
	if (string && *string && stringlen < 64) {
		char str[64];
		memcpy(str, string, stringlen);
		str[stringlen] = 0;
		if (!format->string_to_offset(str, ofs)) {
			errorbox("%s: '%s' doesn't seem to be a valid offset", string_desc, str);
			return false;
		}
		return true;
	}
	return false;
}
		
		
void blockop_dialog(ht_format_viewer *format, FileOfs pstart, FileOfs pend)
{
	Bounds b;
	b.w = 65;
	b.h = 15;
	b.x = (screen->w - b.w)/2;
	b.y = (screen->h - b.h)/2;
	
	ht_blockop_dialog *d=new ht_blockop_dialog();
	d->init(&b, pstart, pend, 0);
	bool run = true;
	int r;
	while (run && (r = d->run(false)) != button_cancel) {
		switch (r) {
		case 100:
			dialog_eval_help(blockop_func_eval, blockop_symbol_eval, NULL);
			break;
		default: 
		{
		ht_blockop_dialog_data t;
		ViewDataBuf vdb(d, &t, sizeof t);
		
		File *file = format->get_file();

		baseview->sendmsg(cmd_edit_mode_i, file, NULL);
		
		if (file->getAccessMode() & IOAM_WRITE) {
			FileOfs start = pstart, end = pend;

			if (format_string_to_offset_if_avail(format, t.start.text, t.start.textlen, "start", &start)
			 && format_string_to_offset_if_avail(format, t.end.text, t.end.textlen, "end", &end)) {
				if (end > start) {
					int esize = 0;
					int esizes[4] = {8, 4, 2, 1};
					switch (t.mode.cursor_pos) {
						/* element type: byte */
						case 0: esize++;
						/* element type: uint16 */
						case 1: esize++;
						/* element type: uint32 */
						case 2: esize++;
						/* element type: uint64 */
						case 3: {
							char a[4096];
							bin2str(a, t.action.text, MIN(sizeof a, t.action.textlen));
							insert_history_entry((List*)getAtomValue(HISTATOM_EVAL_EXPR), a, NULL);
						
							char addr[128];
							bin2str(addr, t.start.text, MIN(sizeof addr, t.start.textlen));
							insert_history_entry((List*)getAtomValue(HISTATOM_GOTO), addr, NULL);
							bin2str(addr, t.end.text, MIN(sizeof addr, t.end.textlen));
							insert_history_entry((List*)getAtomValue(HISTATOM_GOTO), addr, NULL);
						
							esize = esizes[esize];
							Object *ctx = NULL;
							try {
								ctx = create_blockop_int_context(file, start, end-start, esize, little_endian, a);
								if (ctx) {
									/*bool b = */execute_process(blockop_int_process, ctx);
								}
							} catch (const Exception &e) {
								errorbox("error: %y", &e);
							}
							delete ctx;
							break;
						}
						/* element type: string */
						case 4: {
							char a[256];
							bin2str(a, t.action.text, MIN(sizeof a, t.action.textlen));
							insert_history_entry((List*)getAtomValue(HISTATOM_EVAL_EXPR), a, NULL);

							char addr[128];
							bin2str(addr, t.start.text, MIN(sizeof addr, t.start.textlen));
							insert_history_entry((List*)getAtomValue(HISTATOM_GOTO), addr, NULL);
							bin2str(addr, t.end.text, MIN(sizeof addr, t.end.textlen));
							insert_history_entry((List*)getAtomValue(HISTATOM_GOTO), addr, NULL);

							Object *ctx = NULL;
							try {
								ctx = create_blockop_str_context(file, start, end-start, esize, little_endian, a);
								if (ctx) {
									/*bool b = */execute_process(blockop_str_process, ctx);
								}
							} catch (const Exception &e) {
								errorbox("error: %y", &e);
							}
							delete ctx;
							break;
						}
						default:
							errorbox("mode %d not supported", t.mode.cursor_pos);
					}							
				} else {
					errorbox("end offset must be greater than start offset");
				}
			}
		}
		run = false;
		}
		}
	}

	d->done();
	delete d;
}

