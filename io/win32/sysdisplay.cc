/*
 *	HT Editor
 *	sysdisplay.cc - screen access functions for Win32
 *
 *	Copyright (C) 1999-2005 Sebastian Biallas (sb@biallas.net)
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

#include "io/display.h"
#include "io/types.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

uint mapToGraphical(char chr)
{
	switch (chr) {
	case GC_1VLINE: return 0x2502;
	case GC_1HLINE: return 0x2500;

	case GC_1CORNER0: return 0x2510;
	case GC_1CORNER1: return 0x2518;
	case GC_1CORNER2: return 0x2514;
	case GC_1CORNER3: return 0x250c;

	case GC_1UTEE: return 0x252c;
	case GC_1LTEE: return 0x251c;
	case GC_1DTEE: return 0x2534;
	case GC_1RTEE: return 0x2524;

	case GC_2VLINE: return 0x2551;
	case GC_2HLINE: return 0x2550;
	case GC_2CORNER0: return 0x2557;
	case GC_2CORNER1: return 0x255c;
	case GC_2CORNER2: return 0x255a;
	case GC_2CORNER3: return 0x2554;

	case GC_LOW: return 0x2591;
	case GC_MEDIUM: return 0x2592;
	case GC_HIGH: return 0x2593;
	case GC_FULL: return 0x2588;

	case GC_ARROW_UP: return 0x25b2;
	case GC_ARROW_DOWN: return 0x25bc;
	case GC_ARROW_LEFT: return 0x25c4;
	case GC_ARROW_RIGHT: return 0x25ba;

	case GC_SMALL_ARROW_UP: return 0x2191;
	case GC_SMALL_ARROW_DOWN: return 0x2193;

	case GC_FILLED_CIRCLE: return 0x25cf;
	case GC_FILLED_QUAD: return 0x25a0;

	case GC_FILLED_UPPER: return 0x2580;
	case GC_FILLED_LOWER: return 0x2584;
	}
	return '?';
}

uint mapCharToSystemCP(char chr, Codepage codepage)
{
	switch (codepage) {
	case CP_WINDOWS: 
	case CP_UNICODE: return chr;
	case CP_DEVICE: 
		if (byte(chr) > 127)  {
			char in[2] = {chr, 0};
			WCHAR out[2];
			OemToCharW(in, out);
			return out[0];
		} else {
			return chr;
		}
	case CP_GRAPHICAL: return mapToGraphical(chr);
	case CP_INVALID: return '?';
	}
	return chr;
}

bool sys_get_screen_size(int &w, int &h)
{
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen_info);
	w = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
	h = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
	return true;
}

/*
 *	Class Win32SystemDisplay
 */

class Win32SystemDisplay: public SystemDisplay {
public:
	CHAR_INFO *buf;
	CHAR_INFO *old_buf;
	vcp *colorbuf;

	HANDLE output;

	CursorMode cursor_mode;
	CursorMode last_cursor_mode;
	int cursorx, cursory;
	int dx, dy;

		void 			cursorBold();
		void 			cursorHide();
		void 			cursorNormal();
		bool			initConsole();
public:
					Win32SystemDisplay(const char *title);
	virtual				~Win32SystemDisplay();
	/* extends Display */
	virtual	void			copyFromDisplay(const Display &display, int x, int y, const Bounds &clipping);
	virtual	void			fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp = CP_DEVICE);
	virtual	void			getCursor(int &x, int &y) const;
	virtual	CursorMode		getCursorMode() const;
	virtual	int			nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage codepage = CP_DEVICE);
	virtual	bool			read(uint &rawchar, vcp &color, int x, int y) const;
	virtual	void			setBounds(const Bounds &b);
	virtual	void			setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL);
	virtual	void			setCursorMode(CursorMode mode = CURSOR_NORMAL);
	/* extends SystemDisplay */
	virtual	void			show();
	/* new */
        	void			putChar(int dest, uint rawchar, vcp vc);
		uint			vcpToSystem(vcp color);
};

Win32SystemDisplay::Win32SystemDisplay(const char *title)
: SystemDisplay()
{
	CONSOLE_SCREEN_BUFFER_INFO screen_info;

	buf = NULL;
	colorbuf = NULL;
	old_buf = NULL;

	if (!initConsole()) ;//throw sys_exception("unable to init console");

	GetConsoleScreenBufferInfo(output, &screen_info);

	SetConsoleTitleA(title);

	assign(0, 0, screen_info.srWindow.Right - screen_info.srWindow.Left + 1,
		screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1);

	dx = screen_info.srWindow.Left;
	dy = screen_info.srWindow.Top;

	setCursor(dx, dy, CURSOR_OFF);
	last_cursor_mode = CURSOR_NORMAL;

/*	COORD xy;
	xy.X = dx;
	xy.Y = dy;
	SetConsoleCursorPosition(output, xy);
	xy.X = dx+w-1;
	xy.Y = dy+h-1;
	SetConsoleCursorPosition(output, xy);
	xy.X = dx;
	xy.Y = dy;
	SetConsoleCursorPosition(output, xy);
	ScrollConsoleScreenBuffer(output, );*/
	show();
}

Win32SystemDisplay::~Win32SystemDisplay()
{
	COORD xy;
	printChar(w-1, h-1, VCP(VC_WHITE, VC_BLACK), ' ');
	xy.X = dx;
	xy.Y = dy+h-1;
	SetConsoleCursorPosition(output, xy);
	free(buf);
	free(old_buf);
	free(colorbuf);
	cursorNormal();
}

bool Win32SystemDisplay::initConsole()
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

	SMALL_RECT windowRect;

	output = GetStdHandle(STD_OUTPUT_HANDLE);

	if (output == INVALID_HANDLE_VALUE) return false;
/*	if (!SetConsoleActiveScreenBuffer(output)) return false;
	if (!GetConsoleScreenBufferInfo(output, &csbiInfo)) return false;
	windowRect.Left = 0;
	windowRect.Top = 0;
	windowRect.Right = csbiInfo.dwSize.X - 1;
	windowRect.Bottom = csbiInfo.dwSize.Y - 1;
	if (!SetConsoleWindowInfo(output, TRUE, &windowRect)) return false;*/
	if (!SetConsoleMode(output, 0)) return false;
	return true;
}

void Win32SystemDisplay::fill(int x, int y, int w, int h, vcp color, char chr, Codepage codepage)
{
	uint rawchar = mapCharToSystemCP(chr, codepage);
	for (int i=0; i < h; i++) {
		if (y+i >= this->h) break;
		if (y+i < 0) continue;

		int dest = x + (y+i)*this->w;
		for (int j=0; j < w; j++) {
			if (x+j >= this->w) break;
			if (x+j >= 0) putChar(dest, rawchar, color);
			dest++;
		}
	}
}

int Win32SystemDisplay::nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage codepage)
{
	int n = 0;
	int dest = x + y*w;
	while (*str && n < maxstrlen && x+n < w) {
		if (x+n >= 0) putChar(dest, mapCharToSystemCP(*str, codepage), color);
		dest++;
		str++;
		n++;
	}
	return n;
}

bool Win32SystemDisplay::read(uint &rawchar, vcp &color, int x, int y) const
{
	rawchar = buf[x+y*w].Char.AsciiChar;
	color = colorbuf[x+y*w];
	return true;
}

void Win32SystemDisplay::setBounds(const Bounds &b)
{
	SystemDisplay::setBounds(b);
	free(buf);
	free(old_buf);
	free(colorbuf);
	buf = ht_malloc(w * h * sizeof(CHAR_INFO));
	old_buf = ht_malloc(w * h * sizeof(CHAR_INFO));
	colorbuf = ht_malloc(w * h * sizeof(vcp));
	memset(old_buf, 0, w * h * sizeof(CHAR_INFO));
	fill(x, y, w, h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void Win32SystemDisplay::copyFromDisplay(const Display &d, int x, int y, const Bounds &clipping)
{
	for (int iy=0; iy < d.h; iy++) {
		if (y+iy >= clipping.y+clipping.h) break;
		if (y+iy < clipping.y) break;
		int dest = x+(iy+y)*w;
		for (int ix=0; ix < d.w; ix++) {
			uint rawchar;
			vcp color;
			d.read(rawchar, color, ix, iy);
			if ((x+ix < clipping.x+clipping.w) && (x+ix >= clipping.x))
				putChar(dest, rawchar, color);
			dest++;
		}
	}
}

void Win32SystemDisplay::getCursor(int &x, int &y) const
{
	x = cursorx;
	y = cursory;
}

CursorMode Win32SystemDisplay::getCursorMode() const
{
	return cursor_mode;
}

void Win32SystemDisplay::putChar(int dest, uint rawchar, vcp vc)
{
	if (dest >= w*h || dest < 0) return;
	buf[dest].Char.UnicodeChar = rawchar ? rawchar : ' ';
	colorbuf[dest] = mixColors(colorbuf[dest], vc);
	buf[dest].Attributes = vcpToSystem(colorbuf[dest]);
}

void Win32SystemDisplay::setCursor(int x, int y, CursorMode mode)
{
	cursorx = x;
	cursory = y;
	if (cursorx < 0) cursorx = 0;
	if (cursory < 0) cursory = 0;
	if (cursorx >= w) cursorx = w-1;
	if (cursory >= h) cursory = h-1;
	setCursorMode(mode);
}

void Win32SystemDisplay::setCursorMode(CursorMode mode)
{
	cursor_mode = mode;
}

void Win32SystemDisplay::show()
{
	int first = h, last = 0;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (buf[y*w+x].Char.UnicodeChar != old_buf[y*w+x].Char.UnicodeChar
			 || buf[y*w+x].Attributes != old_buf[y*w+x].Attributes) {
				if (first == h) {
					first = y;
				}
				last = y;
				break;
			}
		}
	}
	
	COORD xy, xy2;

	if (cursor_mode != CURSOR_OFF) {
		xy.X = cursorx + dx;
		xy.Y = cursory + dy;
	} else {
		xy.X = dx;
		xy.Y = dy;
	}
	SetConsoleCursorPosition(output, xy);
	if (last_cursor_mode != cursor_mode) {
		switch (cursor_mode) {
		case CURSOR_OFF:
			cursorHide(); break;
		case CURSOR_NORMAL:
			cursorNormal(); break;
		case CURSOR_BOLD:
			cursorBold(); break;
		}
		last_cursor_mode = cursor_mode;
	}

	if (first > last) return;
	xy.X = 0;
	xy.Y  = first;
	xy2.X = w;
	xy2.Y = h;
	SMALL_RECT sr;
	sr.Left = dx;
	sr.Top = dy+first;
	sr.Right = dx+w-1;
	sr.Bottom = dy+last;
	WriteConsoleOutputW(output, buf, xy2, xy, &sr);
	memcpy(old_buf, buf, w * h * sizeof(CHAR_INFO));
return;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
//			if (buf[y*w+x].Char.UnicodeChar != old_buf[y*w+x].Char.UnicodeChar
//			 || buf[y*w+x].Attributes != old_buf[y*w+x].Attributes) {
				xy.X = x;
				xy.Y  = y;
				xy2.X = w;
				xy2.Y = h;
				SMALL_RECT sr;
				sr.Left = dx+x;
				sr.Top = dy+y;
				sr.Right = dx+x+1;
				sr.Bottom = dy+y+1;
				WriteConsoleOutputW(output, buf, xy2, xy, &sr);
//			}
		}
	}
	memcpy(old_buf, buf, w * h * sizeof(CHAR_INFO));
}

void Win32SystemDisplay::cursorHide()
{
	COORD xy;
	xy.X = dx+w-1;
	xy.Y = dy+h-1;
	SetConsoleCursorPosition(output, xy);
	CONSOLE_CURSOR_INFO ci;
	ci.dwSize = 1;
	ci.bVisible = false;
	SetConsoleCursorInfo(output, &ci);
}

void Win32SystemDisplay::cursorBold()
{
	CONSOLE_CURSOR_INFO ci;
	ci.dwSize = 99;
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

uint Win32SystemDisplay::vcpToSystem(vcp vc)
{
	byte fg, bg;
	if (VC_GET_LIGHT(VCP_BACKGROUND(vc))) {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc))+8;
	} else {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc));
	}
	if (VC_GET_LIGHT(VCP_FOREGROUND(vc))) {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc))+8;
	} else {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc));
	}
	return (bg<<4|fg);
}

static int sysdisplay_count = 0;
	
SystemDisplay *allocSystemDisplay(const char *title)
{
	if (sysdisplay_count) return NULL;
	sysdisplay_count++;
	return new Win32SystemDisplay(title);
}

void sys_display_enter()
{
}

void sys_display_leave()
{
}
