/* 
 *	HT Editor
 *	sysdisplay.cc - screen access functions for DJGPP
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

#include <conio.h>
#include <dpmi.h>
#include <pc.h>
#include <go32.h>
#include <stdlib.h>
#include <sys/movedata.h>

#include "display.h"
#include "types.h"
#include "sysexcept.h"

void put_vc(uint16 *dest, int rawchar, vcp vc)
{

	int fg, bg;
	if (VC_GET_BASECOLOR(VCP_BACKGROUND(vc))==VC_TRANSPARENT) {
		bg=(((byte*)dest)[1])>>4;
	} else if (VC_GET_LIGHT(VCP_BACKGROUND(vc))) {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc))+8;
	} else {
		bg=VC_GET_BASECOLOR(VCP_BACKGROUND(vc));
	}
	if (VC_GET_BASECOLOR(VCP_FOREGROUND(vc))==VC_TRANSPARENT) {
		fg=(((byte*)dest)[1])&0xf;
	} else if (VC_GET_LIGHT(VCP_FOREGROUND(vc))) {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc))+8;
	} else {
		fg=VC_GET_BASECOLOR(VCP_FOREGROUND(vc));
	}
	*dest=( ((bg<<4)|fg) <<8)|((byte)rawchar);
}

/*
 *	Class DjgppSystemDisplay
 */

class DjgppSystemDisplay: public SystemDisplay {
protected:
	int screensel;
     uint16 *buf;

     CursorMode cursor_mode;
     CursorMode last_cursor_mode;
     int cursorx, cursory;

			void 		cursorBold();
			void 		cursorHide();
			void 		cursorNormal();
public:
						DjgppSystemDisplay(const char *title);
	virtual				~DjgppSystemDisplay();
/* implements Display */
	virtual	void			fill(int x, int y, int w, int h, vcp color, char chr, int codepage = CP_SYSTEM);
	virtual	int			nprint(int x, int y, vcp color, char *str, int strlen, int codepage = CP_SYSTEM);
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

DjgppSystemDisplay::DjgppSystemDisplay(const char *title)
: SystemDisplay()
{
	buf = NULL;
	setCursor(0, 0, CURSOR_OFF);
     last_cursor_mode = CURSOR_NORMAL;

     assign(0, 0, ScreenCols(), ScreenRows());

	if ((screensel = __dpmi_allocate_ldt_descriptors(1)) == -1)
     	throw new sys_exception("Unable to allocate selector");
	if (__dpmi_set_descriptor_access_rights(screensel, 0xc0f3) == -1)
     	throw new sys_exception("Unable to set selector access rights");
	if (__dpmi_set_segment_base_address(screensel, 0xb8000) == -1)
     	throw new sys_exception("Unable to set selector base address");
	if (__dpmi_set_segment_limit(screensel, 0x7fff) == -1)
     	throw new sys_exception("Unable to set selector limit");

	show();
}

DjgppSystemDisplay::~DjgppSystemDisplay()
{
/* hack to keep prompt color white on black */
	print(w-1, h-1, VCP(VC_WHITE, VC_BLACK), " ");

     setCursor(0, h-1, CURSOR_NORMAL);
	show();
     __dpmi_free_ldt_descriptor(screensel);
	if (buf) free(buf);
}

void DjgppSystemDisplay::fill(int x, int y, int w, int h, vcp color, char chr, int codepage)
{
	for (int i=0; i<h; i++) {
		uint16 *b = buf+x+(y+i)*this->w;
		if (y+i>=this->h) break;
		if (y+i>=0)
		for (int j=0; j<w; j++) {
			if (x+j>=0) put_vc(b, chr, color);
			if (x+j>=this->w-1) break;
			b++;
		}
	}
}

int DjgppSystemDisplay::nprint(int x, int y, vcp color, char *str, int strlen, int codepage)
{
	int n = 0;
	uint16 *b=buf+x+y*this->w;
	while (*str && (n<strlen) && (x+n<w)) {
		put_vc(b, (byte)*str, color);
		b++;
		str++;
		n++;
	}
	return n;
}

bool DjgppSystemDisplay::read(int &rawchar, vcp &color, int x, int y)
{
	uint16 *b = buf+x+y*w;
     rawchar = *b&0xff;
     color = *b>>8;
	return true;
}

void DjgppSystemDisplay::setBounds(const Bounds &b)
{
	SystemDisplay::setBounds(b);
	if (buf) free(buf);
	buf = (uint16*)malloc(sizeof *buf * w * h);
	fill(x, y, w, h, VCP(VC_WHITE, VC_BLACK), ' ');
}

void	DjgppSystemDisplay::copyFromDisplay(Display *d, int x, int y, const Bounds &clipping)
{
	for (int iy=0; iy< d->h; iy++) {
		uint16 *k = buf+x+(iy+y)*w;
		if (y+iy >= clipping.y+clipping.h) break;
		if (y+iy >= clipping.y)
		for (int ix=0; ix < d->w; ix++) {
          	int rawchar;
               int color;
               d->read(rawchar, color, ix, iy);
			if ((x+ix<clipping.x+clipping.w) && (x+ix>=clipping.x))
				put_vc(k, rawchar, color);
			k++;
		}
	}
}

void	DjgppSystemDisplay::getCursor(int &x, int &y)
{
	x = cursorx;
	y = cursory;
}

CursorMode DjgppSystemDisplay::getCursorMode()
{
	return cursor_mode;
}

void	DjgppSystemDisplay::setCursor(int x, int y, CursorMode mode)
{
	cursorx = x;
	cursory = y;
	setCursorMode(mode);
}

void	DjgppSystemDisplay::setCursorMode(CursorMode mode)
{
     cursor_mode = mode;
}

void	DjgppSystemDisplay::show()
{
	if (cursor_mode != CURSOR_OFF) gotoxy(cursorx+1, cursory+1);
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
	movedata(_go32_my_ds(), (int)buf, screensel, 0, w*h*2);
     last_cursor_mode = cursor_mode;
}

void DjgppSystemDisplay::cursorHide()
{
	__dpmi_regs r;
	r.h.ah = 1;
	r.h.ch = 31;
	r.h.cl = 30;
	__dpmi_int(0x10, &r);
}

void DjgppSystemDisplay::cursorBold()
{
	__dpmi_regs r;
	r.h.ah = 1;
	r.h.ch = 0;
	r.h.cl = 31;

	__dpmi_int(0x10, &r);
}

void DjgppSystemDisplay::cursorNormal()
{
	__dpmi_regs r;
	r.h.ah = 1;
	r.h.ch = 30;
	r.h.cl = 31;

	__dpmi_int(0x10, &r);
}

static int sysdisplay_count = 0;
     
SystemDisplay *allocSystemDisplay(const char *title)
{
	if (sysdisplay_count) return NULL;
     sysdisplay_count++;
	return new DjgppSystemDisplay(title);
}

// those were the days... R.I.P. waitretrace()

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

