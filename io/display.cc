/* 
 *	HT Editor
 *	display.cc
 *
 *	Copyright (C) 1999-2004 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <new>
#include <cstdarg>
#include <cstdlib>

#include "display.h"
#include "snprintf.h"

#if 0
static vc gInverseColors[8] = {
        VC_WHITE, VC_YELLOW, VC_MAGENTA, VC_RED,
	VC_CYAN, VC_GREEN, VC_BLUE, VC_BLACK
};
#define VC_GET_INVERSE(vc) gInverseColor[(vc)&7]
#else
#define VC_GET_INVERSE(vc) (7-((vc)&7))
#endif

inline vc mixSingleColor(vc base, vc layer)
{
	if (VC_GET_BASECOLOR(base) > 7) return layer;
	
	switch (VC_GET_BASECOLOR(layer)) {
	case VC_TRANSPARENT_EXCLUSIVE_DOM:
	case VC_TRANSPARENT_EXCLUSIVE:
	case VC_TRANSPARENT:	return base;

	case VC_DARKEN:		return VC_GET_BASECOLOR(base);
	case VC_LIGHTEN:	return VC_LIGHT(base);

	case VC_MONOCHROME: {
		switch (base) {
		case VC_CYAN:
		case VC_MAGENTA:
		case VC_YELLOW:
		case VC_WHITE:
			return VC_WHITE;
		case VC_RED:
		case VC_BLACK:
		case VC_BLUE:
		case VC_GREEN:
		default:
			return VC_BLACK;
		}
	}
	case VC_INVERSE:	return VC_GET_INVERSE(VC_GET_BASECOLOR(base));
	}
	
	return layer;
}

vcp mixColors(vcp base, vcp layer)
{
	vc fg = mixSingleColor(VCP_FOREGROUND(base), VCP_FOREGROUND(layer));
	vc bg = mixSingleColor(VCP_BACKGROUND(base), VCP_BACKGROUND(layer));

	if (fg == bg) {
		if (VC_GET_BASECOLOR(VCP_FOREGROUND(layer)) == VC_TRANSPARENT_EXCLUSIVE
		 || VC_GET_BASECOLOR(VCP_BACKGROUND(layer)) == VC_TRANSPARENT_EXCLUSIVE_DOM) {
			int fglight = VC_GET_LIGHT(fg);
			fg = VC_GET_INVERSE(VC_GET_BASECOLOR(bg)) | fglight;
			fg = VC_BLACK;
		} else if (VC_GET_BASECOLOR(VCP_BACKGROUND(layer)) == VC_TRANSPARENT_EXCLUSIVE
		 || VC_GET_BASECOLOR(VCP_FOREGROUND(layer)) == VC_TRANSPARENT_EXCLUSIVE_DOM) {
			int bglight = VC_GET_LIGHT(bg);
			bg = VC_GET_INVERSE(VC_GET_BASECOLOR(fg)) | bglight;
		}
	}
	return VCP(fg, bg);
}

/*
 *	Display
 */
void Display::assign(int x, int y, int w, int h)
{
	Bounds b(x, y, w, h);
	setBounds(b);
}

void Display::fillAll(vcp color, char chr, Codepage cp)
{
	fill(0, 0, w, h, color, chr, cp);
}

void Display::move(int deltax, int deltay)
{
	Bounds b(*this);
	b.move(deltax, deltay);
	setBounds(b);
}

void Display::resize(int deltaw, int deltah)
{
	Bounds b(*this);
	b.resize(deltaw, deltah);
	setBounds(b);
}

int Display::nprintf(int x, int y, vcp color, int maxstrlen, Codepage cp, const char *format, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, format);
	ht_vsnprintf(buf, MIN((int)sizeof buf, maxstrlen), format, ap);
	va_end(ap);
	return print(x, y, color, buf, cp);
}

int Display::print(int x, int y, vc color, const char *str, Codepage cp)
{
	return nprint(x, y, color, str, 0x7fffffff, cp);
}

int Display::printW(int x, int y, vcp color, const AbstractChar *widestr)
{
	const AbstractChar *owidestr = widestr;
	// FIXME: speed ?
	while (widestr->codepage != CP_INVALID) {
		if (!printChar(x++, y, color, widestr->chr, widestr->codepage)) break;
		widestr++;
	}
	return widestr-owidestr;
}

int Display::nprintW(int x, int y, vcp color, const AbstractChar *widestr, int maxstrlen)
{
	const AbstractChar *owidestr = widestr;
	// FIXME: speed ?
	while (widestr->codepage != CP_INVALID && maxstrlen--) {
		if (!printChar(x++, y, color, widestr->chr, widestr->codepage)) break;
		widestr++;
	}
	return widestr-owidestr;
}

int Display::printChar(int x, int y, vcp color, char chr, Codepage cp)
{
	// FIXME: speed ?
	fill(x, y, 1, 1, color, chr, cp);
	return 1;
}

int Display::printf(int x, int y, vcp color, Codepage cp, const char *format, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, format);
	ht_vsnprintf(buf, sizeof buf, format, ap);
	va_end(ap);
	return print(x, y, color, buf, cp);
}

void Display::setBounds(const Bounds &b)
{
	Bounds::assign(b.x, b.y, b.w, b.h);
}

/* graphical extension */
#if 0
void Display::line(int px1, int py1, int px2, int py2, uint color)
{
}

void Display::putPixel(int px, int py, uint color)
{
}

void Display::textToPixelCoord(int tx, int ty, int &px, int &py) const
{
	px = tx;
	py = ty;
}

void Display::pixelToTextCoord(int px, int py, int &tx, int &ty) const
{
	tx = px;
	ty = py;
}
#endif

/*
 *	NullDisplay
 */
NullRDisplay::NullRDisplay(const Bounds &b)
: RDisplay(b)
{
     setCursor(0, 0, CURSOR_OFF);
}

void NullRDisplay::fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp)
{
}

void NullRDisplay::getCursor(int &x, int &y) const
{
	x = cursorx;
	y = cursory;
}

CursorMode NullRDisplay::getCursorMode() const
{
	return cursorMode;
}

int NullRDisplay::nprint(int ix, int iy, vcp color, const char *str, int maxstrlen, Codepage cp)
{
	int i = 0;
	// FIXME: more efficient impl
	if (y < h) {
		while ((ix+i < w) && (*str) && (i<maxstrlen)) {
			i++;
		}
	}
	return i;
}

bool NullRDisplay::read(uint &rawchar, vcp &color, int x, int y) const
{
	if ((x >= w) || (y >= h)) return false;
	return true;
}

void NullRDisplay::setCursor(int x, int y, CursorMode mode)
{
	cursorx = x;
	cursory = y;
	setCursorMode(mode);
}

void NullRDisplay::setCursorMode(CursorMode mode)
{
	cursorMode = mode;
}

/*
 *	BufferedRDisplay
 */
BufferedRDisplay::BufferedRDisplay(const Bounds &b)
: RDisplay(b)
{
	buf = NULL;
	setBounds(b);
	setCursor(0, 0, CURSOR_OFF);
}

BufferedRDisplay::~BufferedRDisplay()
{
	free(buf);
}

void BufferedRDisplay::fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp)
{
	uint rawchar = mapCharToSystemCP(chr, cp);
	bool transparent = (cp == CP_GRAPHICAL && chr == GC_TRANSPARENT);
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (x+w > this->w) {
		w = this->w - x;
	}
	if (y+h > this->h) {
		h = this->h - y;
	}
	for (int iy = y; iy < y+h; iy++) {
		ColoredChar *b = buf+x+ iy * this->w;
		for (int ix = x; ix < x+w; ix++) {
			if (!transparent) b->rawchar = rawchar;
			b->color = mixColors(b->color, color);
			b++;
		}
	}
}

void BufferedRDisplay::getCursor(int &x, int &y) const
{
	x = cursorx;
	y = cursory;
}

CursorMode BufferedRDisplay::getCursorMode() const
{
	return cursorMode;
}

int BufferedRDisplay::nprint(int ix, int iy, vcp color, const char *str, int maxstrlen, Codepage cp)
{
	int i = 0;
	ColoredChar *b = buf+ix+ iy * w;
	if (iy < h) {
		while (ix+i < w && str[i] && i < maxstrlen) {
			bool transparent = (cp == CP_GRAPHICAL && str[i] == GC_TRANSPARENT);
			if (!transparent) b->rawchar = mapCharToSystemCP(str[i], cp);
			b->color = mixColors(b->color, color);
			i++;
			b++;
		}
	}
	return i;
}

bool BufferedRDisplay::read(uint &rawchar, vcp &color, int x, int y) const
{
	if (!buf) return false;
	if ((x >= w) || (y >= h)) return false;
	ColoredChar *b = buf + x + y*w;
	rawchar = b->rawchar;
	color = b->color;
	return true;
}

void BufferedRDisplay::setBounds(const Bounds &b)
{
	Bounds oldb = *(Bounds*)this;
	RDisplay::setBounds(b);
	ColoredChar *bufnew;
	if (w * h) {
		bufnew = ht_malloc(sizeof *buf * w * h);
		if (!bufnew) throw std::bad_alloc();
		ColoredChar *bb = bufnew;
		for (int iy = 0; iy < h; iy++) {
			for (int ix = 0; ix < w; ix++) {
				if ((ix < oldb.w) && (iy < oldb.h) && buf) {
					*bb = buf[ix+iy*oldb.w];
				} else {
					bb->rawchar = ' ';
					bb->color = VCP(VC_TRANSPARENT, VC_TRANSPARENT);
				}
				bb++;
			}
		}
	} else bufnew = NULL;
	free(buf);
	buf = bufnew;
}

void BufferedRDisplay::setCursor(int x, int y, CursorMode mode)
{
	cursorx = x;
	cursory = y;
	setCursorMode(mode);
}

void BufferedRDisplay::setCursorMode(CursorMode mode)
{
	cursorMode = mode;
}

/*
 *	SystemDisplay
 */
SystemDisplay::SystemDisplay()
{
}

/*
 *	SystemRDisplay
 */
SystemRDisplay::SystemRDisplay(SystemDisplay *System_display, const Bounds &b)
: RDisplay(b)
{
	system_display = System_display;
}

SystemRDisplay::~SystemRDisplay()
{
}

void SystemRDisplay::fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp)
{
	x += this->x;
	y += this->y;
	system_display->fill(x, y, w, h, color, chr, cp);
}

void SystemRDisplay::getCursor(int &x, int &y) const
{
	system_display->getCursor(x, y);
	x -= this->x;
	y -= this->y;
}

CursorMode SystemRDisplay::getCursorMode() const
{
	return system_display->getCursorMode();
}

int SystemRDisplay::nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp)
{
	x += this->x;
	y += this->y;
	return system_display->nprint(x, y, color, str, maxstrlen, cp);
}

bool SystemRDisplay::read(uint &rawchar, vcp &color, int x, int y) const
{
	x += this->x;
	y += this->y;
	return system_display->read(rawchar, color, x, y);
}

void SystemRDisplay::setBounds(const Bounds &b)
{
	RDisplay::setBounds(b);
}

void SystemRDisplay::setCursor(int x, int y, CursorMode mode)
{
	x += this->x;
	y += this->y;
	system_display->setCursor(x, y, mode);
}

void SystemRDisplay::setCursorMode(CursorMode mode)
{
	system_display->setCursorMode(mode);
}
