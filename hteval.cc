/* 
 *	HT Editor
 *	hteval.cc
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
#include "hthist.h"
#include "htiobox.h"
#include "htstring.h"

#include <string.h>

extern "C" {
#include "evalx.h"
}

int sprintf_base2(char *x, dword value)
{
	char *ix=x;
	dword m=0x80000000;
/* skip zeros */
	while (m) {
		if (value & m) break;
		m>>=1;
	}
	do {
		if (value & m) *x='1'; else *x='0';
		x++;
		m>>=1;
	} while (m);
	*x=0;
	return x-ix;
}

void eval_dialog()
{
	bounds b, c;
	app->getbounds(&c);
	b.w=c.w/2+1;
	b.h=c.h/2+1;
	b.x=(c.w-b.w)/2;
	b.y=(c.h-b.h)/2;
	ht_dialog *d=new ht_dialog();
	c=b;
	char *hint="type an integer, float or string expression to evaluate";
	
	d->init(&b, "evaluate", FS_TITLE | FS_MOVE | FS_RESIZE);

	ht_list *ehist=(ht_list*)find_atom(HISTATOM_EVAL_EXPR);

/* input line */
	BOUNDS_ASSIGN(b, 1, 1, c.w-4, 1);
	ht_strinputfield *s=new ht_strinputfield();
	s->init(&b, 255, ehist);
	d->insert(s);
/* result text */
	BOUNDS_ASSIGN(b, 1, 3, c.w-4, c.h-5);
	ht_statictext *t=new ht_statictext();
	t->init(&b, hint, align_left);
	t->growmode=GM_VDEFORM;
	d->insert(t);

	while (d->run(0)!=button_cancel) {
		ht_strinputfield_data str;
		scalar_t r;
		char b[1024];
		char c[256];
		s->databuf_get(&str);
		if (str.textlen) {
			bin2str(b, str.text, str.textlen);
		
			insert_history_entry(ehist, b, 0);

			if (eval(&r, b, NULL, NULL, NULL)) {
				switch (r.type) {
					case SCALAR_INT: {
						char *x=b;
						x+=sprintf(x, "integer:\n");
						x+=sprintf(x, "hex  %x\n", r.scalar.integer.value);
						x+=sprintf(x, "dec  %u\n", r.scalar.integer.value);
						x+=sprintf(x, "sdec %d\n", r.scalar.integer.value);
						x+=sprintf(x, "oct  %o\n", r.scalar.integer.value);
						x+=sprintf(x, "bin  ");
						x+=sprintf_base2(x, r.scalar.integer.value);
						*(x++)='\n';
						*x=0;
						break;
					}
					case SCALAR_STR: {
						char *x=b;
						x+=sprintf(x, "string:\n");
						/* c-escaped */
						escape_special(c, sizeof c, r.scalar.str.value, r.scalar.str.len, NULL, true);
						x+=sprintf(x, "c-escaped \"%s\"\n", c);
						/* raw */
						bin2str(c, r.scalar.str.value, MIN((UINT)r.scalar.str.len, sizeof c-1));
						x+=sprintf(x, "raw       '%s'\n", c);
						break;
					}
					case SCALAR_FLOAT:
						sprintf(b, "float:\n%.20f\n%.20e", r.scalar.floatnum.value, r.scalar.floatnum.value);
						break;
					default:
						strcpy(b, "?");
				}
				scalar_destroy(&r);
			} else {
				char *str="?";
				int pos=0;
				get_eval_error(&str, &pos);
				s->isetcursor(pos);
				sprintf(b, "error at pos %d: %s", pos, str);
			}
			t->settext(b);
		} else {
			t->settext(hint);
		}
	}

	d->done();
	delete d;
}

