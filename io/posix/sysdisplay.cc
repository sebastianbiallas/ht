/* 
 *	HT Editor
 *	sysdisplay.cc - screen access functions for POSIX
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

#include "config.h"
#include CURSES_HDR

#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>

#include "io/display.h"
#include "io/types.h"

extern struct sigaction old_SIGTRAP;

uint mapToGraphical(char chr)
{
	switch (chr) {
	case GC_1VLINE:
		return ACS_VLINE;
	case GC_1HLINE:
		return ACS_HLINE;
	case GC_1CORNER0:
		return ACS_URCORNER;
	case GC_1CORNER1:
		return ACS_LRCORNER;
	case GC_1CORNER2:
		return ACS_LLCORNER;
	case GC_1CORNER3:
		return ACS_ULCORNER;
	/* for all 'T's: ACS nomenclature differs from ours */
	case GC_1UTEE:
		return ACS_BTEE;
	case GC_1LTEE:
		return ACS_RTEE;
	case GC_1DTEE:
		return ACS_TTEE;
	case GC_1RTEE:
		return ACS_LTEE;
	case GC_1CROSS:
		return ACS_PLUS;
	case GC_2VLINE:
		return ACS_VLINE;
	case GC_2HLINE:
		return ACS_HLINE;
	case GC_2CORNER0:
		return ACS_URCORNER;
	case GC_2CORNER1:
		return ACS_LRCORNER;
	case GC_2CORNER2:
		return ACS_LLCORNER;
	case GC_2CORNER3:
		return ACS_ULCORNER;
	case GC_LOW:
		return ACS_CKBOARD;
	case GC_MEDIUM:
		return ACS_CKBOARD;
	case GC_HIGH:
		return ACS_CKBOARD;
	case GC_FULL:
		return ACS_BLOCK;
	case GC_ARROW_UP:
		return ACS_UARROW;
	case GC_ARROW_DOWN:
		return ACS_DARROW;
	case GC_ARROW_LEFT:
		return (byte)'<';
	case GC_ARROW_RIGHT:
		return (byte)'>';
	case GC_SMALL_ARROW_UP:
		return ACS_UARROW;
	case GC_SMALL_ARROW_DOWN:
		return ACS_DARROW;
	case GC_FILLED_CIRCLE:
		return ACS_BULLET;
	case GC_FILLED_QUAD:
		return 'x';
	case GC_TRANSPARENT:
		return 0;
	case GC_FILLED_UPPER:
		return '^';
	case GC_FILLED_LOWER:
		return '_';
	}
	return '?';
}

#if 0
#define CHAR_LINEV ACS_VLINE
#define CHAR_LINEH ACS_HLINE
#define CHAR_LINEV_DBL ACS_VLINE
#define CHAR_LINEH_DBL ACS_HLINE
#define CHAR_BORDERTL ACS_LTEE
#define CHAR_BORDERTR ACS_RTEE
#define CHAR_BORDERTU ACS_TTEE
#define CHAR_BORDERTD ACS_BTEE
#define CHAR_BORDERTL_DBL ACS_LTEE
#define CHAR_BORDERTR_DBL ACS_RTEE
#define CHAR_BORDERTU_DBL ACS_TTEE
#define CHAR_BORDERTD_DBL ACS_BTEE
#define CHAR_CORNERUL ACS_ULCORNER
#define CHAR_CORNERLL ACS_LLCORNER
#define CHAR_CORNERUR ACS_URCORNER
#define CHAR_CORNERLR ACS_LRCORNER
#define CHAR_CORNERUL_DBL ACS_ULCORNER
#define CHAR_CORNERLL_DBL ACS_LLCORNER
#define CHAR_CORNERUR_DBL ACS_URCORNER
#define CHAR_CORNERLR_DBL ACS_LRCORNER
#define CHAR_FILLED_L ACS_CKBOARD	/* low */
#define CHAR_FILLED_M ACS_CKBOARD	/* medium */
#define CHAR_FILLED_H ACS_CKBOARD	/* high */
#define CHAR_FILLED_F ACS_BLOCK		/* full */
#define CHAR_FILLED_HU '#'
#define CHAR_FILLED_HL ' '
#define CHAR_QUAD_SMALL 'x'
#define CHAR_ARROW_UP ACS_UARROW
#define CHAR_ARROW_DOWN ACS_DARROW
#define CHAR_ARROWBIG_UP ACS_UARROW
#define CHAR_ARROWBIG_DOWN ACS_DARROW
#define CHAR_ARROWBIG_RIGHT '>'
#define CHAR_ARROWBIG_LEFT '<'
#define CHAR_RADIO ACS_BULLET
#endif

uint mapCharToSystemCP(char chr, Codepage cp)
{
	switch (cp) {
	case CP_DEVICE: return (byte)chr;
//	case CP_WINDOWS: return (byte)chr;
	case CP_GRAPHICAL: return mapToGraphical(chr);
	default: break;
	}
	return (byte)chr;
}

/*
 *	Class CursesSystemDisplay
 */
 
struct CursesChar {
	uint rawchar;
	vcp color;
}; 

class CursesSystemDisplay: public SystemDisplay {
protected:
	CursorMode cursor_mode;
	int cursorx, cursory; 
	
	CursesChar *buf;
	WINDOW *win;
	SCREEN *terminal;
	bool use_colors, use_high_colors, is_xterm;

	short colormap[64];

			void 		cursorBold();
			void 		cursorHide();
			void 		cursorNormal();
			void		putChar(CursesChar *dest, uint rawchar, vcp vc);
public:
					CursesSystemDisplay(const char *title);
	virtual				~CursesSystemDisplay();
	/* extends Display */
	virtual	void			copyFromDisplay(const Display &display, int x, int y, const Bounds &clipping);
	virtual	void			fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp = CP_DEVICE);
	virtual	void			getCursor(int &x, int &y) const;
	virtual	CursorMode		getCursorMode() const;
	virtual	int			nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp = CP_DEVICE);
	virtual	bool			read(uint &rawchar, vcp &color, int x, int y) const;
	virtual	void			setBounds(const Bounds &b);
	virtual	void			setCursor(int x, int y, CursorMode mode = CURSOR_NORMAL);
	virtual	void			setCursorMode(CursorMode mode = CURSOR_NORMAL);
	/* extends SystemDisplay */
	virtual	void			show();
	/* debug */	
		void			doShowCursor();
		void			term_on();
		void			term_off();
};

CursesSystemDisplay *gDisplay = NULL;

extern struct sigaction old_SIGWINCH;	// from sysinit.cc

// dont work, dont know why...
void SIGTRAP_sigaction(int i, siginfo_t *info, void *v)
{
	// set cursor for gdb
	if (gDisplay) gDisplay->doShowCursor();
	old_SIGTRAP.sa_sigaction(i, info, v);
}

sig_atomic_t gWinChFlag = 0;

void SIGWINCH_sigaction(int i, siginfo_t *info, void *v)
{
	// FIXME: really, really, really possible??? (or just Valgrind's fault)
//	if (!info) return;
	gWinChFlag = true;
	if (old_SIGWINCH.sa_sigaction) old_SIGWINCH.sa_sigaction(i, info, v);
}

bool sys_get_winch_flag()
{
	return gWinChFlag;
}

void sys_set_winch_flag(bool f)
{
	gWinChFlag = f;
}

bool sys_get_screen_size(int &w, int &h)
{
#if defined TIOCGWINSZ && !defined SCO_FLAVOR
	struct winsize winsz;

	winsz.ws_col = winsz.ws_row = 0;
	/* Ioctl on the STDIN_FILENO */
	ioctl(0, TIOCGWINSZ, &winsz);
	if (winsz.ws_col && winsz.ws_row) {
	        w = winsz.ws_col;
		h = winsz.ws_row;
    		resizeterm(winsz.ws_row, winsz.ws_col);
	        clearok(stdscr, TRUE);
	        return true;
        }
#endif /* TIOCGWINSZ && !SCO_FLAVOR */
	return false;
}

bool sys_set_screen_size(int w, int h)
{
#if defined TIOCSWINSZ && !defined SCO_FLAVOR
        struct winsize tty_size;

	tty_size.ws_row = h;
	tty_size.ws_col = w;
	tty_size.ws_xpixel = tty_size.ws_ypixel = 0;

	return ioctl(0, TIOCSWINSZ, &tty_size) == 0;
#endif
	return false;
}

CursesSystemDisplay::CursesSystemDisplay(const char *title)
: SystemDisplay()
{
	buf = NULL;

	cursorx = 0;
	cursory = 0;

	terminal = NULL;

	term_on();
	show();
}

CursesSystemDisplay::~CursesSystemDisplay()
{
	term_off();
	free(buf);
}

void CursesSystemDisplay::term_off()
{
//	if (!terminal) return;
	::erase();
	::refresh();
	::endwin();
//	::delscreen(terminal);
	terminal = NULL;
}

void CursesSystemDisplay::term_on()
{
	if (terminal) return;
	int colors[8] = { COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE };

	setCursor(0, 0, CURSOR_OFF);

	::setlocale(LC_ALL, "");

//	terminal = ::newterm(NULL, stdout, stdin);
	win = ::initscr();
//	win = stdscr;

//	::setlocale(LC_ALL, "C");

	use_colors = false;
	use_high_colors = false;
	if (::has_colors()) {
		use_colors = true;
		char *term = getenv("TERM");
		bool bold_support = false;
		attr_t attrs;
		short cur_color = 1;
		::start_color();
		/* FIXME: Does this work ???: test if the WA_BOLD attr can be set */
		::attr_on(WA_BOLD, 0);
		attrs = WA_NORMAL;
		attr_get(&attrs, &cur_color, 0);
		bold_support = (attrs==WA_BOLD);
		::attr_off(WA_BOLD, 0);

		is_xterm = (term && strcmp(term, "linux") && strcmp(term, "console"));
		if (!is_xterm && !bold_support) {
			fprintf(stderr, "warning: terminal is of type '%s' (non-x-terminal) but bold_test fails!", term);
		}
		if (bold_support) {
			use_high_colors = true;
		} else {
			fprintf(stderr, "warning: terminal only supports 8 foreground colors!");
		}
		for (int fg=0; fg < 8; fg++) {
			for (int bg=0; bg < 8; bg++) {
				colormap[fg+bg*8] = fg+bg*8;
				::init_pair(fg+bg*8, colors[fg], colors[bg]);
			}
		}
		colormap[7] = 0;
		colormap[0] = 7;
		::init_pair(7, COLOR_BLACK, COLOR_BLACK);
	} else {
		fprintf(stderr, "warning: terminal lacks color support!");
	}
	::wtimeout(win, 1);
	::meta(win, 1);
	::keypad(win, 1);
	::nodelay(win, 1);
	::noecho();
	::cbreak();
	ESCDELAY = 500;

	assign(0, 0, getmaxx(win), getmaxy(win));
}

void CursesSystemDisplay::copyFromDisplay(const Display &d, int x, int y, const Bounds &clipping)
{
	CursorMode cm = d.getCursorMode();
	int cx, cy;
	d.getCursor(cx,cy);
	cx += d.x;
	cy += d.y;
//	setCursor(cx, cy, cm);
	for (int iy = 0; iy < d.h; iy++) {
		CursesChar *k = buf+x+(iy+y)*w;
		if (y+iy >= clipping.y+clipping.h) break;
		if (y+iy >= clipping.y)
		for (int ix=0; ix < d.w; ix++) {
			uint rawchar;
			vcp color;
			d.read(rawchar, color, ix, iy);
			if (x+ix < clipping.x+clipping.w && x+ix >= clipping.x)
				putChar(k, rawchar, color);
			k++;
		}
	}
}

void CursesSystemDisplay::fill(int x, int y, int w, int h, vcp color, char chr, Codepage cp)
{
	uint rawchar = mapCharToSystemCP(chr, cp);
	for (int i=0; i<h; i++) {
		CursesChar *b = buf+x+(y+i)*this->w;
		if (y+i >= this->h) break;
		if (y+i >= 0)
		for (int j=0; j<w; j++) {
			if (x+j >= this->w) break;
			if (x+j >= 0) putChar(b, rawchar, color);
			b++;
		}
	}
}

void CursesSystemDisplay::getCursor(int &x, int &y) const
{
	x = cursorx;
	y = cursory;
}

CursorMode CursesSystemDisplay::getCursorMode() const
{
	return cursor_mode;
}

int CursesSystemDisplay::nprint(int x, int y, vcp color, const char *str, int maxstrlen, Codepage cp)
{
	int n = 0;
	CursesChar *b=buf+x+y*w;
	while (n < maxstrlen && *str && x+n < w) {
		uint rawchar = mapCharToSystemCP(*str, cp);
		if (x+n>=0) putChar(b, rawchar, color);
		b++;
		str++;
		n++;
	}
	return n;
}

bool CursesSystemDisplay::read(uint &rawchar, vcp &color, int x, int y) const
{
	CursesChar *b = buf+x+y*w;
	rawchar = b->rawchar;
	color = b->color;
	return true;
}

void CursesSystemDisplay::setBounds(const Bounds &b)
{
	SystemDisplay::setBounds(b);
	free(buf);
	buf = ht_malloc(sizeof *buf * w * h);
	memset(buf, 0, sizeof *buf * w * h);
	fill(x, y, w, h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void CursesSystemDisplay::setCursor(int x, int y, CursorMode mode)
{
	cursorx = x;
	cursory = y;
	setCursorMode(mode);
}

void CursesSystemDisplay::setCursorMode(CursorMode mode)
{
	cursor_mode = mode;
}

void CursesSystemDisplay::show()
{
	CursesChar *ch = buf;
	vcp c = -1;
	for (int iy=0; iy < h; iy++) {
		::move(iy+y, x);
		for (int ix=0; ix < w; ix++) {
			if (use_colors && ch->color != c) {
				vc fg = VCP_FOREGROUND(ch->color);
				vc bg = VCP_BACKGROUND(ch->color);
				if (use_high_colors) {
					if (0 && is_xterm && fg == VC_LIGHT(VC_BLACK)) {
						/* some terminals can't display dark grey (=light black), so we take light grey instead... */
						attrset(A_NORMAL);
						fg = VC_WHITE;
					} else {
						if (VC_GET_LIGHT(fg)) attrset(A_BOLD); else attrset(A_NORMAL);
					}
				}
/*				fg = VC_GET_BASECOLOR(fg);
				bg = VC_GET_BASECOLOR(bg);*/
				fg &= 7;
				bg &= 7;
				color_set(colormap[bg*8+fg], 0);
			}
			uint chr = (uint)ch->rawchar;
			if ((chr >= 0x20 && chr < 0x7f) || chr > 0xff) {
				addch(chr);
			} else {
				attrset(A_BOLD);
				color_set(colormap[VC_RED*8+VC_WHITE], 0);
				addch('?');
			}
			ch++;
		}
	}

	::curs_set(0);

	::refresh();

	::move(cursory, cursorx);
	switch (cursor_mode) {
	case CURSOR_OFF:
		::curs_set(0); break;
	case CURSOR_NORMAL:
		::curs_set(1); break;
	case CURSOR_BOLD:
		if (::curs_set(2) == ERR) ::curs_set(1); 
		break;
	}
	::refresh();
}

void CursesSystemDisplay::doShowCursor()
{
/*	::move(0,0);
	curs_set(1);
	refresh();*/
}

void CursesSystemDisplay::putChar(CursesChar *dest, uint rawchar, vcp vc)
{
	if (dest >= buf+w*h || dest < buf) return;
	if (rawchar) dest->rawchar = rawchar;
	dest->color = mixColors(dest->color, vc);
}

static int sysdisplay_count = 0;
	
SystemDisplay *allocSystemDisplay(const char *title)
{
	if (sysdisplay_count) return NULL;
	sysdisplay_count++;
	gDisplay = new CursesSystemDisplay(title);
	return gDisplay;
}

/*
void sys_display_enter()
{
	gDisplay->term_on();
}

void sys_display_leave()
{
	gDisplay->term_off();
}
*/
