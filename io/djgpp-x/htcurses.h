/* 
 *	HT Editor
 *	htcurses.h (DJGPP implementation)
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

#ifndef __HTCURSES_H__
#define __HTCURSES_H__

#include "htio.h"
#include "common.h"

#define CHAR_LINEV (unsigned char)'\xb3'
#define CHAR_LINEH (unsigned char)'\xc4'
#define CHAR_LINEV_DBL (unsigned char)'\xba'
#define CHAR_LINEH_DBL (unsigned char)'\xcd'
#define CHAR_LINEH_BU (unsigned char)'\xdf'
#define CHAR_LINEH_BL (unsigned char)'\xdc'
#define CHAR_BORDERTL (unsigned char)'\xc3'
#define CHAR_BORDERTR (unsigned char)'\xb4'
#define CHAR_BORDERTU (unsigned char)'\xc2'
#define CHAR_BORDERTD (unsigned char)'\xc1'
#define CHAR_CORNERUL (unsigned char)'\xda'
#define CHAR_CORNERLL (unsigned char)'\xc0'
#define CHAR_CORNERUR (unsigned char)'\xbf'
#define CHAR_CORNERLR (unsigned char)'\xd9'
#define CHAR_CORNERUL_DBL (unsigned char)'\xc9'
#define CHAR_CORNERLL_DBL (unsigned char)'\xc8'
#define CHAR_CORNERUR_DBL (unsigned char)'\xbb'
#define CHAR_CORNERLR_DBL (unsigned char)'\xbc'
#define CHAR_FILLED_L (unsigned char)'\xb0'	/* FILLED, low density */
#define CHAR_FILLED_M (unsigned char)'\xb1'	/* FILLED, medium density */
#define CHAR_FILLED_H (unsigned char)'\xb2'  /* FILLED, high density */
#define CHAR_FILLED_F (unsigned char)'\xdb'  /* FILLED, filled entirely */
#define CHAR_FILLED_HU (unsigned char)'\xdf' /* FILLED, upper half - filled entirely */
#define CHAR_FILLED_HL (unsigned char)'\xdc' /* FILLED, lower half - filled entirely */
#define CHAR_QUAD_SMALL (unsigned char)'\xfe'
#define CHAR_ARROW_UP (unsigned char)'\x18'
#define CHAR_ARROW_DOWN (unsigned char)'\x19'
#define CHAR_ARROWBIG_UP (unsigned char)'\x1e'
#define CHAR_ARROWBIG_DOWN (unsigned char)'\x1f'
#define CHAR_ARROWBIG_LEFT (unsigned char)'\x11'
#define CHAR_ARROWBIG_RIGHT (unsigned char)'\x10'
#define CHAR_RADIO	(unsigned char)'\x7'

class screendrawbuf: public genericdrawbuf {
protected:
	unsigned short *buf;
	int screensel;
	int cursorx, cursory;
	bool cursorhidden;
	bool cursoroverwrite;
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
	void	show();
	void getcursor(int *x, int *y);
	void hidecursor();
	void showcursor();
	void setcursor(int x, int y);
	void setcursormode(bool override);
};

#endif /* !__HTCURSES_H__ */
