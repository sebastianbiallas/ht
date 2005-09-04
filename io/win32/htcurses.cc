/* 
 *	HT Editor
 *	htcurses.cc (WIN32 implementation)
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* stupid rsxntdj hack */
#ifdef __RSXNT__
extern "C" {
	int __dj_stderr;
}
#endif

HANDLE output;

/*
 *	CLASS screendrawbuf
 */
screendrawbuf::screendrawbuf(char *title)
{
	bounds b;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;

	buf = NULL;
	cursorx = 0;
	cursory = 0;

	init_console();

	GetConsoleScreenBufferInfo(output, &screen_info);

	SetConsoleTitleA(title);

	b.x = 0;
	b.y = 0;
/*	b.w = screen_info.dwSize.X;
	b.h = screen_info.dwSize.Y;*/
	b.w = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
	b.h = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
	b_setbounds(&b);

	cursor_visible = true;
	cursor_redraw = true;
	hidecursor();
	show();
}

screendrawbuf::~screendrawbuf()
{
	/* hack to keep prompt color white on black */
	b_printchar(size.w-1, size.h-1, VCP(VC_WHITE, VC_BLACK), ' ');

	setcursor(0, size.h-1);
	show();
	if (buf) free(buf);
}

bool screendrawbuf::init_console()
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	SMALL_RECT windowRect;

	output = GetStdHandle(STD_OUTPUT_HANDLE);

	if (output == INVALID_HANDLE_VALUE) return false;
	if (SetConsoleActiveScreenBuffer(output) == FALSE) return false;
	if (!GetConsoleScreenBufferInfo(output, &csbiInfo)) return false;
	windowRect.Left = 0;
	windowRect.Top = 0;
	windowRect.Right = csbiInfo.dwSize.X - 1;
	windowRect.Bottom = csbiInfo.dwSize.Y - 1;
	if (!SetConsoleWindowInfo(output, TRUE, &windowRect)) return false;
	if (SetConsoleMode(output, 0) == FALSE) return false;
	return true;
}

void screendrawbuf::drawbuffer(drawbuf *b, int x, int y, bounds *clipping)
{
	drawbufch *ch = b->buf;
	for (int iy=0; iy < b->size.h; iy++) {
		int dest = x+(iy+y)*size.w;
		if (y+iy >= clipping->y+clipping->h) break;
		if (y+iy >= clipping->y) {
			for (int ix=0; ix  <b->size.w; ix++) {
				if (x+ix < clipping->x+clipping->w && x+ix >= clipping->x) {
					put_vc(dest, ch->ch, ch->c);
				}
				dest++;
				ch++;
			}
		}
	}
}

void screendrawbuf::b_fill(int x, int y, int w, int h, int c, int ch)
{
	for (int i=0; i<h; i++) {
		int dest = x+(i+y)*size.w;
		if (y+i >= size.h) break;
		if (y+i >= 0)
		for (int j=0; j < w; j++) {
			if (x+j >= 0) put_vc(dest, ch, c);
			if (x+j >= size.w-1) break;
			dest++;
		}
	}
}

void screendrawbuf::b_printchar(int x, int y, int c, int ch)
{
	int dest = x+y*size.w;
	put_vc(dest, ch, c);
}

int screendrawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	int n=0;
	int dest = x+y*size.w;
	while (*text && n < l) {
		put_vc(dest, *text, c);
		dest++;
		text++;
		n++;
	}
	return n;
}

int screendrawbuf::b_lprintw(int x, int y, int c, int l, int *text)
{
	int n=0;
	int dest = x+y*size.w;
	while (*text && n < l) {
		put_vc(dest, *text, c);
		dest++;
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
	if (buf) free(buf);
	buf = (CHAR_INFO *)malloc(size.w * size.h * sizeof (CHAR_INFO));
	b_fill(size.x, size.y, size.w, size.h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void screendrawbuf::show()
{
	COORD xy, xy2;
	if (cursor_redraw) {
		xy.X = cursorx;
		xy.Y = cursory;
		SetConsoleCursorPosition(output, xy);
		cursor_redraw = false;
	}
	xy.X = 0;
	xy.Y  = 0;
	xy2.X = size.w;
	xy2.Y = size.h;
	SMALL_RECT sr;
	sr.Left = 0;
	sr.Top = 0;
	sr.Right = size.w-1;
	sr.Bottom = size.h-1;
	WriteConsoleOutputW(output, (CHAR_INFO *)buf, xy2, xy, &sr);
}

void screendrawbuf::getcursor(int *x, int *y)
{
	*x=cursorx;
	*y=cursory;
}

void screendrawbuf::hidecursor()
{
	if (cursor_visible) {
		COORD xy;
		xy.X=size.w-1;
		xy.Y=size.h-1;
		SetConsoleCursorPosition(output, xy);
		CONSOLE_CURSOR_INFO ci;
		ci.dwSize = 0xd;
		ci.bVisible = false;
		if (!SetConsoleCursorInfo(output, &ci)) {
		}
		cursor_visible = false;
	}
}

void screendrawbuf::setcursor(int x, int y)
{
	showcursor();
	cursorx=x;
	cursory=y;
	cursor_redraw = true;
}

void screendrawbuf::setcursormode(bool override)
{
}

void screendrawbuf::showcursor()
{
	if (!cursor_visible) {
		CONSOLE_CURSOR_INFO ci;
		ci.dwSize = 0xd;
		ci.bVisible = true;
		SetConsoleCursorInfo(output, &ci);
		cursor_visible = true;
	}
}

/* virtual color pairs (fg/bg) */

void screendrawbuf::put_vc(int dest, int ch, int vc)
{
	int fg, bg;
	if (VC_GET_BASECOLOR(VCP_BACKGROUND(vc))==VC_TRANSPARENT) {
		bg = ((byte)((CHAR_INFO *)buf)[dest].Attributes)>>4;
	} else if (VC_GET_LIGHT(VCP_BACKGROUND(vc))) {
		bg = VC_GET_BASECOLOR(VCP_BACKGROUND(vc))+8;
	} else {
		bg = VC_GET_BASECOLOR(VCP_BACKGROUND(vc));
	}
	if (VC_GET_BASECOLOR(VCP_FOREGROUND(vc))==VC_TRANSPARENT) {
		fg = (byte)(((CHAR_INFO *)buf)[dest].Attributes&0xf);
	} else if (VC_GET_LIGHT(VCP_FOREGROUND(vc))) {
		fg = VC_GET_BASECOLOR(VCP_FOREGROUND(vc)) + 8;
	} else {
		fg = VC_GET_BASECOLOR(VCP_FOREGROUND(vc));
	}
	if (ch > 127 && ch < 256) {
		WCHAR o;		
		MultiByteToWideChar(CP_OEMCP, 0, (char*)&ch, 1, &o, 1);
		ch = o;
	}
	((CHAR_INFO *)buf)[dest].Char.UnicodeChar = ch ? ch : ' ';
        ((CHAR_INFO *)buf)[dest].Attributes = (unsigned char)((bg<<4)|fg);
}
