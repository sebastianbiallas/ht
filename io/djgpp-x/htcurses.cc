/* 
 *	HT Editor
 *	htcurses.cc (DJGPP implementation)
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

#include "global.h"
#include "htcurses.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>

void put_vc(unsigned short *dest, char ch, int vc)
{
	int fg, bg;
	if (VC_GET_BASECOLOR(VCP_BACKGROUND(vc))==VC_TRANSPARENT) {
		bg=(((unsigned char*)dest)[1])>>4;
	} else if (VC_GET_LIGHT(VCP_BACKGROUND(vc))) {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc))+8;
	} else {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc));
	}
	if (VC_GET_BASECOLOR(VCP_FOREGROUND(vc))==VC_TRANSPARENT) {
		fg=(((unsigned char*)dest)[1])&0xf;
	} else if (VC_GET_LIGHT(VCP_FOREGROUND(vc))) {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc))+8;
	} else {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc));
	}
	*dest=( ((bg<<4)|fg) <<8)|((unsigned char)ch);
}

/*
 *	CLASS screendrawbuf
 */

screendrawbuf::screendrawbuf(char *title)
{
	bounds b;
	buf=0;
	cursorx=0;
	cursory=0;
	cursorhidden = false;
	cursoroverwrite = false;
	hidecursor();

	b.x=0;
	b.y=0;
	b.w=ScreenCols();
	b.h=ScreenRows();
	b_setbounds(&b);

	screensel=__dpmi_allocate_ldt_descriptors(1);
	__dpmi_set_descriptor_access_rights(screensel, 0xc0f3);
	__dpmi_set_segment_base_address(screensel, 0xb8000);
	__dpmi_set_segment_limit(screensel, 0x7fff);

	show();
}

screendrawbuf::~screendrawbuf()
{
/* hack to keep prompt color white on black */
	b_printchar(size.w-1, size.h-1, VCP(VC_WHITE, VC_BLACK), ' ');
	
	setcursormode(false);
	setcursor(0, size.h-1);
	show();
	if (buf) {
		delete buf;
	}
}

void screendrawbuf::drawbuffer(drawbuf *b, int x, int y, bounds *clipping)
{
	drawbufch *ch=b->buf;
	for (int iy=0; iy<b->size.h; iy++) {
		unsigned short *k=buf+x+(iy+y)*size.w;
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

void screendrawbuf::b_fill(int x, int y, int w, int h, int c, int ch)
{
	for (int i=0; i<h; i++) {
		unsigned short *b=buf+x+(y+i)*size.w;
		if (y+i>=size.h) break;
		if (y+i>=0)
		for (int j=0; j<w; j++) {
			if (x+j>=0) put_vc(b, ch, c);
			if (x+j>=size.w-1) break;
			b++;
		}
	}
}

void screendrawbuf::b_printchar(int x, int y, int c, int ch)
{
	unsigned short*b=buf+x+y*size.w;
	put_vc(b, ch, c);
}

int screendrawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	int n=0;
	unsigned short *b=buf+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(b, (unsigned char)*text, c);
		b++;
		text++;
		n++;
	}
	return n;
}

int screendrawbuf::b_lprintw(int x, int y, int c, int l, int *text)
{
	int n=0;
	unsigned short *b=buf+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(b, (unsigned char)*text, c);
		b++;
		text++;
		n++;
	}
	return n;
}

void screendrawbuf::b_resize(int rw, int rh)
{
    /* screens are not sizeable (?) */
}

void screendrawbuf::b_rmove(int rx, int ry)
{
    /* screens are not movable */
}

void screendrawbuf::b_setbounds(bounds *b)
{
	genericdrawbuf::b_setbounds(b);
	if (buf) delete buf;
	buf=(unsigned short *)malloc(sizeof *buf * size.w * size.h);
	b_fill(size.x, size.y, size.w, size.h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void screendrawbuf::show()
{
	gotoxy(cursorx+1, cursory+1);
/* FIXME: */
	movedata(_go32_my_ds(), (int)buf, screensel, 0, size.w*size.h*2);
}

void screendrawbuf::getcursor(int *x, int *y)
{
	*x=cursorx;
	*y=cursory;
}

void screendrawbuf::hidecursor()
{
	if (!cursorhidden) {
		__dpmi_regs r;
		r.h.ah = 1;
		r.h.ch = 31;
		r.h.cl = 30;
		__dpmi_int(0x10, &r);

		cursorhidden = true;
	}
}

void screendrawbuf::setcursor(int x, int y)
{
	showcursor();
	cursorx=x;
	cursory=y;
}

void screendrawbuf::setcursormode(bool overwrite)
{
	if (cursoroverwrite != overwrite) {
		cursoroverwrite = overwrite;
		if (!cursorhidden) {
			hidecursor();
			showcursor();
		}
	}
}

void screendrawbuf::showcursor()
{
	if (cursorhidden) {
		__dpmi_regs r;
		r.h.ah = 1;
		if (cursoroverwrite) {
			r.h.ch = 0;
			r.h.cl = 31;
		} else {
			r.h.ch = 30;
			r.h.cl = 31;
		}
		__dpmi_int(0x10, &r);

		cursorhidden = false;
	}
}

/*void waitretrace()
{
	asm ("
		mov	$0x3da, %edx
waitretrace_eot:
		in	%dx, %al
		test	$8, %al
		jnz	waitretrace_eot
waitretrace_eor:
		in	%dx, %al
		test $8, %al
		jz	waitretrace_eor");
}*/

