/* 
 *	HT Editor
 *	htcurses.h (POSIX implementation)
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTCURSES_H__
#define __HTCURSES_H__

#include "config.h"

#include CURSES_HDR
#undef clear
#undef move
#undef getstr

#include "htio.h"
#include "common.h"

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

class screendrawbuf: public genericdrawbuf {
public:
	screendrawbuf(char *title);
	~screendrawbuf();
	virtual void b_fill(int x, int y, int w, int h, int c, int ch);
	virtual void b_printchar(int x, int y, int c, int ch);
	virtual int b_lprint(int x, int y, int c, int l, char *text);
	virtual int b_lprintw(int x, int y, int c, int l, int *text);
	virtual void b_resize(int rw, int rh);
	virtual void b_rmove(int rx, int ry);
	virtual void b_setbounds(bounds *b);
/* new */
	void drawbuffer(drawbuf *buf, int x, int y, bounds *clipping);
	void show();
	void getcursor(int *x, int *y);
	void hidecursor();
	void showcursor();
	void setcursor(int x, int y);
	void setcursormode(bool override);
};

extern int xres, yres;

#endif /* !__HTCURSES_H__ */

