/* 
 *	HT Editor
 *	sysdisplay.cc - screen access functions for Win32
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include "../display.h"
#include "../sysexcept.h"
#include "../types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

HANDLE output;

/* virtual color pairs (fg/bg) */

void put_vc(char *dest1, uint16 *dest2, char ch, int vc)
{
	int fg, bg;
	if (VC_GET_BASECOLOR(VCP_BACKGROUND(vc))==VC_TRANSPARENT) {
		bg=(byte)(((byte)*dest2)>>4);
	} else if (VC_GET_LIGHT(VCP_BACKGROUND(vc))) {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc))+8;
	} else {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc));
	}
	if (VC_GET_BASECOLOR(VCP_FOREGROUND(vc))==VC_TRANSPARENT) {
		fg=(byte)(((byte)*dest2)&0xf);
	} else if (VC_GET_LIGHT(VCP_FOREGROUND(vc))) {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc))+8;
	} else {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc));
	}
	*dest1=(unsigned char)ch;
	*dest2=(unsigned char)((bg<<4)|fg);
}

int mapCharToSystemCP(char chr, int codepage)
{
	switch (codepage) {
     	case CP_DEVICE: return chr;
          case CP_WINDOWS: {
          	char b[2];
               b[0] = chr;
               b[1] = 0;
               CharToOem(b, b);
          	return b[0];
		}
     }
	return chr;
}

/*
 *	Class Win32SystemDisplay
 */

class Win32SystemDisplay: public SystemDisplay {
protected:
     uint16 *buf_attr;
     char *buf_char;

     CursorMode cursor_mode;
     CursorMode last_cursor_mode;
     int cursorx, cursory;

			void 		cursorBold();
			void 		cursorHide();
			void 		cursorNormal();
			bool			initConsole();
public:
						Win32SystemDisplay(const char *title);
	virtual				~Win32SystemDisplay();
/* implements Display */
	virtual	void			fill(int x, int y, int w, int h, vcp color, char chr, int codepage = CP_DEVICE);
	virtual	int			nprint(int x, int y, vcp color, char *str, int strlen, int codepage = CP_DEVICE);
     virtual	bool			read(int &rawchar, vcp &color, int x, int y);
	virtual	void			setBounds(const Bounds &b);
/* new */
	virtual	void			copyFromDisplay(Display *display, int x, int y, const Bounds &clipping);
	virtual	void			getCursor(int &x, int &y);
	virtual	CursorMode	getCursorMode();
	virtual	void			setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL);
	virtual	void			setCursorMode(CursorMode mode = CURSOR_NORMAL);
	virtual	void			show();
};

Win32SystemDisplay::Win32SystemDisplay(const char *title)
: SystemDisplay()
{
//////////////////
	CONSOLE_SCREEN_BUFFER_INFO screen_info;

	buf_attr = NULL;
	buf_char = NULL;

	if (!initConsole()) throw new sys_exception("unable to init console");

	GetConsoleScreenBufferInfo(output, &screen_info);

	SetConsoleTitleA(title);

	assign(0, 0, screen_info.srWindow.Right - screen_info.srWindow.Left + 1,
		screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1);

	setCursor(0, 0, CURSOR_OFF);
     last_cursor_mode = CURSOR_NORMAL;

	show();
}

Win32SystemDisplay::~Win32SystemDisplay()
{
	if (buf_attr) free(buf_attr);
	if (buf_char) free(buf_char);
}

bool Win32SystemDisplay::initConsole()
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

void Win32SystemDisplay::fill(int x, int y, int w, int h, vcp color, char chr, int codepage)
{
     int rawchar = mapCharToSystemCP(chr, codepage);
	for (int i=0; i<h; i++) {
		uint16 *k_attr = buf_attr+x+(y+i)*this->w;
		char *k_char = buf_char+x+(y+i)*this->w;
		if (y+i>=this->h) break;
		if (y+i>=0)
		for (int j=0; j<w; j++) {
			if (x+j>=0) put_vc(k_char, k_attr, rawchar, color);
			if (x+j>=this->w-1) break;
			k_attr++;
               k_char++;
		}
	}
}

int Win32SystemDisplay::nprint(int x, int y, vcp color, char *str, int strlen, int codepage)
{
	int n = 0;
	uint16 *k_attr = buf_attr+x+y*this->w;
	char *k_char = buf_char+x+y*this->w;
	while (*str && (n<strlen) && (x+n<w)) {
		put_vc(k_char, k_attr, mapCharToSystemCP(*str, codepage), color);
		k_attr++;
		k_char++;
		str++;
		n++;
	}
	return n;
}

bool Win32SystemDisplay::read(int &rawchar, vcp &color, int x, int y)
{
	uint16 *k_attr = buf_attr+x+y*w;
	char *k_char = buf_char+x+y*w;
     rawchar = *k_char;
     color = *k_attr;
	return true;
}

void Win32SystemDisplay::setBounds(const Bounds &b)
{
	SystemDisplay::setBounds(b);
	if (buf_attr) free(buf_attr);
	if (buf_char) free(buf_char);
	buf_attr = (uint16 *)malloc(w * h * sizeof *buf_attr);
	buf_char = (char *)malloc(w * h * sizeof *buf_char);
	fill(x, y, w, h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void	Win32SystemDisplay::copyFromDisplay(Display *d, int x, int y, const Bounds &clipping)
{
	for (int iy=0; iy< d->h; iy++) {
		uint16 *k_attr = buf_attr+x+(iy+y)*w;
		char *k_char = buf_char+x+(iy+y)*w;
		if (y+iy >= clipping.y+clipping.h) break;
		if (y+iy >= clipping.y)
		for (int ix=0; ix < d->w; ix++) {
          	int rawchar;
               int color;
               d->read(rawchar, color, ix, iy);
			if ((x+ix<clipping.x+clipping.w) && (x+ix>=clipping.x))
				put_vc(k_char, k_attr, rawchar, color);
			k_attr++;
			k_char++;
		}
	}
}

void	Win32SystemDisplay::getCursor(int &x, int &y)
{
	x = cursorx;
	y = cursory;
}

CursorMode Win32SystemDisplay::getCursorMode()
{
	return cursor_mode;
}

void	Win32SystemDisplay::setCursor(int x, int y, CursorMode mode)
{
	cursorx = x;
	cursory = y;
	setCursorMode(mode);
}

void	Win32SystemDisplay::setCursorMode(CursorMode mode)
{
     cursor_mode = mode;
}

void	Win32SystemDisplay::show()
{
	COORD xy;
	DWORD wrr;

	if (cursor_mode != CURSOR_OFF) {
		xy.X = cursorx;
		xy.Y = cursory;
		SetConsoleCursorPosition(output, xy);
	}
     if (last_cursor_mode != cursor_mode) {
     	switch (cursor_mode) {
          	case CURSOR_OFF:
               	cursorHide(); break;
          	case CURSOR_NORMAL:
               	cursorNormal(); break;
          	case CURSOR_BOLD:
               	cursorBold(); break;
          }
     }
	xy.X=0;
	for (xy.Y=0; xy.Y<h; xy.Y++) {
		WriteConsoleOutputAttribute(output, buf_attr+(xy.Y*w), w, xy, &wrr);
		WriteConsoleOutputCharacter(output, buf_char+(xy.Y*w), w, xy, &wrr);
	}
}

void Win32SystemDisplay::cursorHide()
{
	COORD xy;
	xy.X = w-1;
	xy.Y = h-1;
	SetConsoleCursorPosition(output, xy);
	CONSOLE_CURSOR_INFO ci;
	ci.dwSize = 1;
	ci.bVisible = false;
	SetConsoleCursorInfo(output, &ci);
}

void Win32SystemDisplay::cursorBold()
{
	CONSOLE_CURSOR_INFO ci;
	ci.dwSize = 100;
	ci.bVisible = true;
	SetConsoleCursorInfo(output, &ci);
}

void Win32SystemDisplay::cursorNormal()
{
	CONSOLE_CURSOR_INFO ci;
	ci.dwSize = 13;
	ci.bVisible = true;
	SetConsoleCursorInfo(output, &ci);
}

static int sysdisplay_count = 0;
     
SystemDisplay *allocSystemDisplay(const char *title)
{
	if (sysdisplay_count) return NULL;
     sysdisplay_count++;
	return new Win32SystemDisplay(title);
}
///////////////////////////////////////////////////

#if 0
screendrawbuf::screendrawbuf(char *title)
{
	bounds b;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;

	buf_attr=NULL;
	buf_char=NULL;
	cursorx=0;
	cursory=0;

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
	if (buf_attr) free(buf_attr);
	if (buf_char) free(buf_char);
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
	drawbufch *ch=b->buf;
	for (int iy=0; iy<b->size.h; iy++) {
		word *k_attr = buf_attr+x+(iy+y)*size.w;
		char *k_char = buf_char+x+(iy+y)*size.w;
		if (y+iy>=clipping->y+clipping->h) break;
		if (y+iy>=clipping->y)
		for (int ix=0; ix<b->size.w; ix++) {
			if ((x+ix<clipping->x+clipping->w) && (x+ix>=clipping->x)) {
				put_vc(k_char, k_attr, ch->ch, ch->c);
			}
			k_attr++;
			k_char++;
			ch++;
		}
	}
}

void screendrawbuf::b_fill(int x, int y, int w, int h, int c, int ch)
{
	for (int i=0; i<h; i++) {
		word *k_attr = buf_attr+x+(i+y)*size.w;
		char *k_char = buf_char+x+(i+y)*size.w;
		if (y+i>=size.h) break;
		if (y+i>=0)
		for (int j=0; j<w; j++) {
			if (x+j>=0) put_vc(k_char, k_attr, ch, c);
			if (x+j>=size.w-1) break;
			k_attr++;
			k_char++;
		}
	}
}

void screendrawbuf::b_printchar(int x, int y, int c, int ch)
{
	word *k_attr = buf_attr+x+y*size.w;
	char *k_char = buf_char+x+y*size.w;
	put_vc(k_char, k_attr, ch, c);
}

int screendrawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	int n=0;
	word *k_attr = buf_attr+x+y*size.w;
	char *k_char = buf_char+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(k_char, k_attr, (unsigned char)*text, c);
		k_attr++;
		k_char++;
		text++;
		n++;
	}
	return n;
}

int screendrawbuf::b_lprintw(int x, int y, int c, int l, int *text)
{
	int n=0;
	word *k_attr = buf_attr+x+y*size.w;
	char *k_char = buf_char+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(k_char, k_attr, (unsigned char)*text, c);
		k_attr++;
		k_char++;
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
	if (buf_attr) free(buf_attr);
	if (buf_char) free(buf_char);
	buf_attr = (word *)malloc(size.w * size.h * sizeof(word));
	buf_char = (char *)malloc(size.w * size.h * sizeof(byte));
	b_fill(size.x, size.y, size.w, size.h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void screendrawbuf::show()
{
	COORD xy;
	DWORD wrr;

	if (cursor_redraw) {
		xy.X=cursorx;
		xy.Y=cursory;
		SetConsoleCursorPosition(output, xy);
		cursor_redraw = false;
	}
	xy.X=0;
	for (xy.Y=0; xy.Y<size.h; xy.Y++) {
		WriteConsoleOutputAttribute(output, buf_attr+(xy.Y*size.w), size.w, xy, &wrr);
		WriteConsoleOutputCharacter(output, buf_char+(xy.Y*size.w), size.w, xy, &wrr);
	}
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
#endif
