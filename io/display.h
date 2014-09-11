/*
 *	HT Editor
 *	display.h
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

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "bounds.h"
#include "types.h"
#include "data.h"

/* codepages */

enum Codepage {
	CP_INVALID = 0,
	CP_DEVICE,
	CP_GRAPHICAL,
	CP_WINDOWS,
	CP_UNICODE,
};

/* "graphical" chars (ie. lines, corners and patterns like in ASCII) */

// if a char has all 4 orthogonally possible alignments, we suffix its name
// by a number 0..3, counting clockwise (mathematically positive)

#define	GC_TRANSPARENT		'0'		// transparent

#define	GC_1VLINE		'|'		// single vertical line
#define	GC_1HLINE		'-'		// single horizontal line
#define	GC_1CORNER0		'l'		// right-upper corner, single
#define	GC_1CORNER1		'j'		// right-lower corner, single
#define	GC_1CORNER2		'L'		// left-lower corner, single
#define	GC_1CORNER3		'F'		// left-upper corner, single

#define	GC_1UTEE		0x01		// 'T', with "nose" pointing up
#define	GC_1LTEE		0x02		// 'T', with "nose" pointing left
#define	GC_1DTEE		0x03		// 'T', with "nose" pointing down
#define	GC_1RTEE		0x04		// 'T', with "nose" pointing right

#define	GC_1CROSS		0x05		// a cross like in '+', but bigger to fit with other line-drawing chars

#define	GC_2VLINE		'H'		// double vertical line
#define	GC_2HLINE		'='		// double horizontal line
#define	GC_2CORNER0		0x06		// right-upper corner, double
#define	GC_2CORNER1		0x07		// right-lower corner, double
#define	GC_2CORNER2		0x08		// left-lower corner, double
#define	GC_2CORNER3		0x09		// left-upper corner, double

#define GC_LOW			0x0a		// regular pattern, density: low
#define GC_MEDIUM		0x0b		// regular pattern, density: medium
#define GC_HIGH			0x0c		// regular pattern, density: high
#define GC_FULL			0x0d		// regular pattern, density: full

#define GC_ARROW_UP		'^'		// a filled triangle, points up
#define GC_ARROW_DOWN		'v'		// a filled triangle, points down
#define GC_ARROW_LEFT		'<'		// a filled triangle, points left
#define GC_ARROW_RIGHT		'>'		// a filled triangle, points right

#define GC_SMALL_ARROW_UP	'A'		// an arrow up
#define GC_SMALL_ARROW_DOWN	'V'		// an arrow down

#define GC_FILLED_CIRCLE	'o'		// a filled and centered circle
#define GC_FILLED_QUAD		'x'		// a filled and centered quad

#define GC_FILLED_UPPER		0x0e		// upper half filled
#define GC_FILLED_LOWER		0x0f		// lower half filled

/* virtual colors */

typedef int vc;

#define NUM_VCS				15

// real colors
#define VC_BLACK			0
#define VC_BLUE				1
#define VC_GREEN			2
#define VC_CYAN				3
#define VC_RED				4
#define VC_MAGENTA			5
#define VC_YELLOW			6
#define VC_WHITE			7
// functional colors
#define VC_TRANSPARENT			8
#define VC_LIGHTEN			9
#define VC_DARKEN			10
#define VC_MONOCHROME			11
#define VC_INVERSE			12
// like VC_TRANSPARENT, but always change 'this color' if it would equal the 'other color'
#define VC_TRANSPARENT_EXCLUSIVE	13
// like VC_TRANSPARENT, but always change the 'other color' if it would equal 'this color'
#define VC_TRANSPARENT_EXCLUSIVE_DOM	14

#define VC_LIGHT(vc) ((vc) | 0x80)

#define VC_GET_LIGHT(vc) ((vc) & 0x80)
#define VC_GET_BASECOLOR(vc) ((vc) & 0x7f)

/* virtual color pairs (fg/bg) */

typedef int vcp;

#define VCP_INVALID	-1
#define VCP(vc_fg, vc_bg) (vcp)((vc_bg) | ((vc_fg)<<8))
#define VCP_BACKGROUND(v) ((v) & 0xff)
#define VCP_FOREGROUND(v) ((v>>8) & 0xff)

vcp mixColors(vcp base, vcp layer);

/*
 *	Display (absolute/screen coordinates)
 */

struct AbstractChar {
	Codepage codepage;
	uint32 chr;
};

struct AbstractColoredChar {
	Codepage codepage;
	vcp color;
	uint32 chr;
};

enum CursorMode { CURSOR_OFF, CURSOR_NORMAL, CURSOR_BOLD };

class Display: public Bounds {
public:
				Display() {};
				Display(const Bounds &b) : Bounds(b) {};
	virtual			~Display() {};
	/* extends Bounds */
	virtual	void		assign(int x, int y, int w, int h);
	virtual	void		move(int deltax, int deltay);
	virtual	void		resize(int deltaw, int deltah);
	/* new */
	virtual	void		fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp = CP_DEVICE) = 0;
     		void		fillAll(vcp color, char chr, Codepage cp = CP_DEVICE);
	virtual	void		getCursor(int &x, int &y) const = 0;
	virtual	CursorMode	getCursorMode() const = 0;
	virtual	int		nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp = CP_DEVICE) = 0;
		int		nprintW(int x, int y, vcp color, const AbstractChar *widestr, int maxstrlen);
		int		nprintf(int x, int y, vcp color, int maxstrlen, Codepage cp, const char *format, ...);
		int		print(int x, int y, vcp color, const char *str, Codepage cp = CP_DEVICE);
		int		printW(int x, int y, vcp color, const AbstractChar *widestr);
		int		printChar(int x, int y, vcp color, char chr, Codepage cp = CP_DEVICE);
		int		printf(int x, int y, vcp color, Codepage cp, const char *format, ...);
	virtual	bool		read(uint &rawchar, vcp &color, int x, int y) const = 0;
	virtual	void		setBounds(const Bounds &b);
	virtual	void		setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL) = 0;
	virtual	void		setCursorMode(CursorMode mode = CURSOR_NORMAL) = 0;
#if 0
	/* graphical extension */
	virtual	void		line(int px1, int py1, int px2, int py2, uint color);
	virtual	void		putPixel(int px, int py, uint color);
	virtual	void		textToPixelCoord(int tx, int ty, int &px, int &py) const;
	virtual	void		pixelToTextCoord(int px, int py, int &tx, int &ty) const;
#endif
};

/*
 *	RDisplay (relative coords)
 */
typedef Display RDisplay;

/*
 *	NullRDisplay
 */
class NullRDisplay: public RDisplay {
protected:
	uint cursorx, cursory;
	CursorMode cursorMode;
public:
				NullRDisplay(const Bounds &b);
	/* extends Display */
	virtual	void		fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp = CP_DEVICE);
	virtual	void		getCursor(int &x, int &y) const;
	virtual	CursorMode	getCursorMode() const;
	virtual	int		nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp = CP_DEVICE);
	virtual	bool		read(uint &rawchar, vcp &color, int x, int y) const;
	virtual	void		setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL);
	virtual	void		setCursorMode(CursorMode mode = CURSOR_NORMAL);
};

/*
 *   BufferedRDisplay
 */
struct ColoredChar {
	uint rawchar;
	vcp color;
};

class BufferedRDisplay: public RDisplay {
protected:
	int cursorx, cursory;
	CursorMode cursorMode;
public:
	ColoredChar *buf;

				BufferedRDisplay(const Bounds &b);
	virtual			~BufferedRDisplay();
	/* extends RDisplay */
	virtual	void		fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp = CP_DEVICE);
	virtual	void		getCursor(int &x, int &y) const;
	virtual	CursorMode	getCursorMode() const;
	virtual	int		nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp = CP_DEVICE);
	virtual	bool		read(uint &rawchar, vcp &color, int x, int y) const;
	virtual	void		setBounds(const Bounds &b);
	virtual	void		setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL);
	virtual	void		setCursorMode(CursorMode mode = CURSOR_NORMAL);
};

/* system-dependant (implementation in $MYSYSTEM/ *.cc) */
uint	mapCharToSystemCP(char chr, Codepage cp);
bool	sys_get_screen_size(int &w, int &h);
bool	sys_set_screen_size(int w, int h);
bool	sys_get_winch_flag();
void	sys_set_winch_flag(bool f);

class SystemDisplay: public Display {
public:
				SystemDisplay();
	/* new */
	virtual	void		copyFromDisplay(const Display &display, int x, int y, const Bounds &clipping) = 0;
	virtual	void		show() = 0;
};

SystemDisplay *allocSystemDisplay(const char *title);

/*
 *   SystemRDisplay
 */
class SystemRDisplay: public RDisplay {
public:
	SystemDisplay *system_display;

				SystemRDisplay(SystemDisplay *system_display, const Bounds &b);
	virtual			~SystemRDisplay();
	/* extends Display */
	virtual	void		fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp = CP_DEVICE);
	virtual	void		getCursor(int &x, int &y) const;
	virtual	CursorMode	getCursorMode() const;
	virtual	int		nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp = CP_DEVICE);
	virtual	bool		read(uint &rawchar, vcp &color, int x, int y) const;
	virtual	void		setBounds(const Bounds &b);
	virtual	void		setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL);
	virtual	void		setCursorMode(CursorMode mode = CURSOR_NORMAL);
};

#endif /* __DISPLAY_H__ */
