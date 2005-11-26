/* 
 *	HT Editor
 *	htcurses.cc (POSIX implementation)
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

#include "config.h"
#include <unistd.h>

#include "global.h"
#include "htcurses.h"
#include "htdebug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* virtual color pairs (fg/bg) */

void put_vc(drawbufch *dest, int ch, int vc)
{
	int fg, bg;
	if (VC_GET_BASECOLOR(VCP_BACKGROUND(vc))==VC_TRANSPARENT) {
		bg=dest->c >> 4;
	} else if (VC_GET_LIGHT(VCP_BACKGROUND(vc))) {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc))+8;
	} else {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc));
	}
	if (VC_GET_BASECOLOR(VCP_FOREGROUND(vc))==VC_TRANSPARENT) {
		fg=dest->c & 0xf;
	} else if (VC_GET_LIGHT(VCP_FOREGROUND(vc))) {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc))+8;
	} else {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc));
	}

	dest->ch=ch;
	dest->c=(bg<<4) | fg;
}

/*
 *	CLASS screendrawbuf
 */
 
drawbufch *buf;
WINDOW *win;
int use_colors, use_high_colors;
int is_xterm;
int cursorx, cursory; 
int cursor_visible = 0;
int cursor_overwrite = 0;

short colormap[64];

//#define TEST

screendrawbuf::screendrawbuf(char *title)
{
#ifndef TEST
	bounds b;
	buf=0;
	int colors[8]={ COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE };

	cursorx=0;
	cursory=0;
	win=initscr();
	use_colors=0;
	use_high_colors=0;
	if (has_colors()) {
	    use_colors=1;
	    char *term=getenv("TERM");
	    int bold_support=0;
	    attr_t attrs;
	    short cur_color=1;
	    start_color();
/* FIXME: Does this work ???: test if the WA_BOLD attr can be set */
	    attr_on(WA_BOLD, 0);
	    attrs=WA_NORMAL;
	    attr_get(&attrs, &cur_color, 0);
	    bold_support=(attrs==WA_BOLD);
	    attr_off(WA_BOLD, 0);
	    
	    is_xterm=(strcmp(term, "linux") && strcmp(term, "console"));
	    if ( (!is_xterm) && (!bold_support)) {
		HT_WARN("terminal is of type '%s' (non-x-terminal) but bold_test fails !", term);
	    }
	    if (bold_support) {
		    use_high_colors=1;
	    } else {
		    HT_WARN("terminal only supports 8 foreground colors !");
	    }
	    for (int fg=0; fg<8; fg++) {
		    for (int bg=0; bg<8; bg++) {
			colormap[fg+bg*8]=fg+bg*8;
			init_pair(fg+bg*8, colors[fg], colors[bg]);
		    }
	    }
	    colormap[7]=0;
	    colormap[0]=7;
	    init_pair(7, COLOR_BLACK, COLOR_BLACK);
	} else {
	    HT_WARN("terminal lacks color support !");
	}
	wtimeout(win, 1);
	meta(win, 1);
	keypad(win, 1);
	nodelay(win, 1);
	noecho();
	cbreak();
	ESCDELAY=500;
	
	b.x=0;
	b.y=0;
	b.w=getmaxx(win);
	b.h=getmaxy(win);
	b_setbounds(&b);

	show();
#endif	
}

screendrawbuf::~screendrawbuf()
{
	endwin();
	delwin(win);
	free(buf);
}

void screendrawbuf::b_fill(int x, int y, int w, int h, int c, int chr)
{
	for (int i=0; i<h; i++) {
		drawbufch *ch=buf+x+(y+i)*size.w;
		for (int j=0; j<w; j++) {
			put_vc(ch, chr, c);
			ch++;
		}
	}
}

void screendrawbuf::b_printchar(int x, int y, int c, int ch)
{
	drawbufch *b=buf+x+y*size.w;
	put_vc(b, ch, c);
}

int screendrawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	int n=0;
	drawbufch *ch=buf+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(ch, *text, c);
		ch++;	
		text++;	
		n++;
	}
	return n;
}

int screendrawbuf::b_lprintw(int x, int y, int c, int l, int *text)
{
	int n=0;
	drawbufch *ch=buf+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(ch, *text, c);
		ch++;	
		text++;	
		n++;
	}
	return n;
}

void screendrawbuf::b_resize(int rx, int ry)
{
    /* screens are not resizeable yet */
}

void screendrawbuf::b_rmove(int rx, int ry)
{
    /* screens are not movable */
}

void screendrawbuf::b_setbounds(bounds *b)
{
	genericdrawbuf::b_setbounds(b);
	if (buf) delete buf;
	buf=(drawbufch *)malloc(sizeof *buf * size.w * size.h);
	b_fill(size.x, size.y, size.w, size.h, VCP(VC_BLACK, VC_BLACK), ' ');
}

void screendrawbuf::drawbuffer(drawbuf *b, int x, int y, bounds *clipping)
{
	drawbufch *ch=b->buf;
	for (int iy=0; iy<b->size.h; iy++) {
		drawbufch *k=buf+x+(iy+y)*size.w;
		if (y+iy>=clipping->y+clipping->h) break;
		if (y+iy>=clipping->y)
		for (int ix=0; ix<b->size.w; ix++) {
			if ((x+ix<clipping->x+clipping->w) && (x+ix>=clipping->x))
				put_vc(k, ch->ch, ch->c);
			k++;
			ch++;
		}
	}
}
/*void screendrawbuf::drawbuffer(drawbuf *b, int x, int y)
{
	for (int iy=0; iy<b->size.h; iy++) {
		drawbufch *src=b->buf+iy*b->size.w;
		drawbufch *dest=buf+x+(y+iy)*size.w;
		memmove(dest, src, b->size.w * sizeof *dest);
	}
}*/

void screendrawbuf::show()
{
#ifndef TEST
	drawbufch *ch=buf;
	int c=-1;
	for (int iy=0; iy<size.h; iy++) {
			move(iy+size.y, size.x);
			for (int ix=0; ix<size.w; ix++) {
				if ((use_colors) && (ch->c!=c)) {
					c=ch->c;
					if (use_high_colors) {
						if (is_xterm && (c==8)) {
					/* some terminals can't display dark grey, so we take light grey instead... */
						    attrset(A_NORMAL);
						    c=7;
						} else {
						    if (c&8) attrset(A_BOLD); else attrset(A_NORMAL);
						}
					}
					color_set( colormap[(c&7) | ((c&(7<<4))>>1)], 0);
				}
				if ((((unsigned char)ch->ch>=0x20) && ((unsigned char)ch->ch<=0x7e)) || ((unsigned int)ch->ch>0xff)) addch(ch->ch); else addch(32);
				ch++;
			}
	}
	curs_set(0);
	refresh();
	move(cursory, cursorx);
	if (cursor_visible) {
		if (cursor_overwrite) curs_set(2); else curs_set(1);
	}
#endif	
}

void screendrawbuf::getcursor(int *x, int *y)
{
	*x=cursorx;
	*y=cursory;
}

void screendrawbuf::hidecursor()
{
	cursor_visible=0;
}

void screendrawbuf::setcursormode(bool o)
{
	cursor_overwrite = o;
}

void screendrawbuf::setcursor(int x, int y)
{
	cursorx=x;
	cursory=y;
	cursor_visible=1;
}

void screendrawbuf::showcursor()
{
	cursor_visible=1;
}
