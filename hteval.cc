/* 
 *	HT Editor
 *	hteval.cc
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

#include <string.h>

#include "atom.h"
#include "htctrl.h"
#include "endianess.h"
#include "hthist.h"
#include "htiobox.h"
#include "htpal.h"
#include "strtools.h"
#include "str.h"
#include "snprintf.h"
#include "syntax.h"
#include "textedit.h"
#include "textfile.h"

extern "C" {
#include "evalx.h"
}


static eval_func_handler real_func_handler;
static eval_symbol_handler real_symbol_handler;
static bool have_last_result;
static eval_scalar last_result;

static bool symbol_eval(eval_scalar *r, char *symbol)
{
	if (strcmp(symbol, "_") == 0) {
		if (have_last_result) {
			 scalar_clone(r, &last_result);
			 return true;
		} else {
			set_eval_error("no previous result...");
			return false;
		}		
	}
	return real_symbol_handler ? real_symbol_handler(r, symbol) : false;
}

static bool func_eval(eval_scalar *r, char *name, eval_scalarlist *params)
{
	eval_func myfuncs[] = {
		{"_", 0, {SCALAR_ANY}, "last result"},
		{NULL},
	};
	if (std_eval_func_handler(r, name, params, myfuncs)) return true;
	return real_func_handler ? real_func_handler(r, name, params) : false;
}

/*
 *	eval help
 */
#define	FH_HEAD		1
#define	FH_DESC		2

// FIXME: disfunctional...
class ht_help_lexer: public ht_syntax_lexer {
public:
/* overwritten */
	virtual	vcp getcolor_syntax(uint pal_index)
	{
		return VCP(VC_BLUE, VC_TRANSPARENT);
	}

	virtual	lexer_state getinitstate()
	{
		return FH_HEAD;
	}

	virtual	lexer_token geterrortoken()
	{
		return 3;
	}

	virtual	const char *getname()
	{
		return "bla";
	}

	virtual	lexer_token gettoken(void *buf, uint buflen, text_pos p, bool start_of_line, lexer_state *ret_state, uint *ret_len)
	{
		*ret_len = buflen;
		lexer_token last = *ret_state;
		if (start_of_line && buflen == 0) {
			*ret_state = FH_HEAD;
		} else {
			*ret_state = FH_DESC;
		}
		return buflen ? last : 0;
	}

	virtual	vcp gettoken_color(lexer_token t)
	{
		switch (t) {
		case FH_HEAD:
			return VCP(VC_LIGHT(VC_WHITE), VC_TRANSPARENT);
		case FH_DESC:
			return VCP(VC_BLACK, VC_TRANSPARENT);
		}
		return VCP(VC_RED, VC_TRANSPARENT);
	}
};

static void dialog_fhelp(File *f)
{
	ht_help_lexer *l = new ht_help_lexer();
	l->init();

	ht_ltextfile *t = new ht_ltextfile(f, true, NULL);

	Bounds b, c;
	app->getbounds(&c);
	b = c;
	b.w = 70;
	b.h = 19;
	b.x = (c.w - b.w) / 2,
	b.y = (c.h - b.h) / 2;
	c = b;

	ht_dialog dialog;
	dialog.init(&b, "eval() - functions", FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);

	b.x = 0;
	b.y = 0;
	b.w -= 2;
	b.h -= 2;

	ht_text_viewer *v = new ht_text_viewer();
	v->init(&b, true, t, NULL);

	v->set_lexer(l, true);

	dialog.insert(v);

	b = c;
	b.x = b.w-2;
	b.y = 0;
	b.w = 1;
	b.h-=2;
	ht_scrollbar *hs=new ht_scrollbar();
	hs->init(&b, &dialog.pal, true);

	dialog.setvscrollbar(hs);

	dialog.setpalette(palkey_generic_cyan);

	dialog.run(0);
	dialog.done();
}

void dialog_eval_help(eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context)
{
	real_func_handler = func_handler;
	real_symbol_handler = symbol_handler;

	eval_scalar res;
	if (eval(&res, "help()", func_eval, symbol_eval, context)) {
		eval_str s;
		scalar_context_str(&res, &s);
		scalar_destroy(&res);

		ConstMemMapFile *f = new ConstMemMapFile(s.value, s.len);

		dialog_fhelp(f);

		string_destroy(&s);
	}
}

/*
 *
 */
static int sprint_base2(char *x, uint32 value, bool leading_zeros)
{
	char *ix = x;
	bool draw = leading_zeros;
	for (int i=0; i<32; i++) {
		bool v = value & (1<<(32-i-1));
		if (v) draw = true;
		if (draw) *x++ = v ? '1' : '0';
	}
	*x = 0;
	return x-ix;
}

static int sprint_base2_0(char *x, uint32 value, int zeros)
{
	char *ix = x;
	char vi = 0;
	uint32 m = 0x80000000;
	while (zeros < 32) {m >>= 1; zeros++;}
	do {
		if (value & m) {
			while (vi--) *(x++)='0';
			vi = 0;
			*x = '1';
			x++;
		} else {
			vi++;
		}
		m >>= 1;
	} while (m);
	if (!value) *(x++)='0';
	*x = 0;
	return x-ix;
}

static void nicify(char *dest, const char *src, int d)
{
	*dest = *src;
	int l = strlen(src);
	if (!l) return;
	dest++;
	src++;
	while (l--) {
		if ((l%d==0) && (l>1)) {
			*dest='\'';
			dest++;
		}
		*dest++ = *src++;
	}
	*dest=0;
}



static void do_eval(ht_strinputfield *s, ht_statictext *t, const char *b, eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context)
{
	eval_scalar r;
	String x;
	
	real_func_handler = func_handler;
	real_symbol_handler = symbol_handler;
	
	if (eval(&r, b, func_eval, symbol_eval, context)) {
		switch (r.type) {
			case SCALAR_INT: {
				char buf1[1024];
				char buf2[1024];
				ht_snprintf(buf1, sizeof buf1, "%qx", r.scalar.integer.value);
				nicify(buf2, buf1, 4);
				x.assignFormat("64bit integer:\nhex   %s\n", buf2);
				ht_snprintf(buf1, sizeof buf1, "%qu", r.scalar.integer.value);
				nicify(buf2, buf1, 3);
				x.appendFormat("dec   %s\n", buf2);
				if ((sint64)r.scalar.integer.value < 0) {
					ht_snprintf(buf1, sizeof buf1, "%qd", r.scalar.integer.value);
					nicify(buf2, buf1+1, 3);
					x.appendFormat("sdec  -%s\n", buf2);
				}
				ht_snprintf(buf1, sizeof buf1, "%qo", r.scalar.integer.value);
				nicify(buf2, buf1, 3);
				x.appendFormat("oct   %s\n", buf2);

				uint32 l = r.scalar.integer.value;
				ht_snprintf(buf1, sizeof buf1, "%032b", l);
				nicify(buf2, buf1, 8);
				x.appendFormat("binlo %s\n", buf2);
				if (r.scalar.integer.value >> 32) {
					l = r.scalar.integer.value >> 32;
					ht_snprintf(buf1, sizeof buf1, "%032b", l);
					nicify(buf2, buf1, 8);
					x.appendFormat("binhi %s\n", buf2);
				}
				byte bb[4];
				/* big-endian string */
				x += "string \"";
				createForeignInt(bb, r.scalar.integer.value, 4, big_endian);
				x.append(bb, 4);
				x += "\" 32bit big-endian (e.g. network)\n";
				/* little-endian string */
				x += "string \"";
				createForeignInt(bb, r.scalar.integer.value, 4, little_endian);
				x.append(bb, 4);
				x += "\" 32bit little-endian (e.g. x86)\n";
				break;
			}
			case SCALAR_STR: {
				char buf1[1024];
				/* c-escaped */
				x = "string:\nc-escaped \"";
				x.append((byte*)buf1, escape_special(buf1, sizeof buf1, r.scalar.str.value, r.scalar.str.len, NULL, true));
				/* raw */
				x += "'\nraw       '";
				x.append((byte*)r.scalar.str.value, r.scalar.str.len);
				x += "'\n";
				break;
			}
			case SCALAR_FLOAT: {
				char buf1[1024];
				sprintf(buf1, "val   %.20f\nnorm  %.20e", r.scalar.floatnum.value, r.scalar.floatnum.value);
				x += buf1;
				// FIXME: endianess/hardware format
				float ff = ((float)r.scalar.floatnum.value);
				uint32 f;
				memcpy(&f, &ff, 4);
				x += "\n-- IEEE-754, 32 bit --";
				x.appendFormat("\nhex   %08x\nbin   ", f);
				x.append((byte*)buf1, sprint_base2(buf1, f, true));
				x.appendFormat("\nsplit %c1.", (f>>31) ? '-' : '+');
				x.append((byte*)buf1, sprint_base2_0(buf1, f&((1<<23)-1), 23));
				x.appendFormat("b * 2^%d", ((f>>23)&255)-127);
				break;
			}
			default:
				x = "?";
		}
		if (have_last_result) scalar_destroy(&last_result);
		scalar_clone(&last_result, &r);
		have_last_result = true;
		
		scalar_destroy(&r);
	} else {
		const char *str="?";
		int pos=0;
		get_eval_error(&str, &pos);
		s->isetcursor(pos);
		x.assignFormat("error at pos %d: %s", pos+1, str);
	}
	String in;
	in.assign('\0');
	x.translate(in, " ");
	t->settext(x.contentChar());
}

#define BUTTON_HELP	100

void eval_dialog(eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context)
{
	Bounds b, c;
	app->getbounds(&c);
	b.w=70;
	b.h=17;
	b.x=(c.w-b.w)/2;
	b.y=(c.h-b.h)/2;
	ht_dialog *d=new ht_dialog();
	c=b;
	const char *hint="type integer, float or string expression to evaluate";

	d->init(&b, "evaluate", FS_TITLE | FS_MOVE | FS_RESIZE);

	List *ehist = (List*)getAtomValue(HISTATOM_EVAL_EXPR);

	/* input line */
	b.assign(1, 1, c.w-14, 1);
	ht_strinputfield *s = new ht_strinputfield();
	s->init(&b, 255, ehist);
	d->insert(s);
	/* help button */
	ht_button *bhelp = new ht_button();
	b.assign(c.w-12, 1, 10, 2);
	bhelp->init(&b, "~Functions", BUTTON_HELP);
	d->insert(bhelp);
	/* result text */
	b.assign(1, 3, c.w-4, c.h-5);
	ht_statictext *t = new ht_statictext();
	t->init(&b, hint, align_left);
	t->growmode = MK_GM(GMH_LEFT, GMV_FIT);
	d->insert(t);

	int button;
	while ((button = d->run(false)) != button_cancel) {
		switch (button) {
		case button_ok: {
			ht_strinputfield_data str;
			char b[1024];
			ViewDataBuf vdb(s, &str, sizeof str);
			if (str.textlen) {
				bin2str(b, str.text, str.textlen);
				insert_history_entry(ehist, b, 0);
				do_eval(s, t, b, func_handler, symbol_handler, context);
			} else {
				t->settext(hint);
			}
			break;
		}
		case BUTTON_HELP:
			dialog_eval_help(func_handler, symbol_handler, context);
			break;
		}
	}

	d->done();
	delete d;
}
