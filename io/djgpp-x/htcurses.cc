/* 
 *	HT Editor
 *	htcurses.cc (DJGPP-X (graphical) implementation)
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <bios.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <sys/farptr.h>
#include <sys/movedata.h>

static byte *font = NULL;

static int font_width;
static int font_height;
static int font_bytes;

static int lfb_addr;
static int lfb_size;
static int resx;
static int resy;
static int bytes_per_pixel;
static int vidmode = 0;
static byte *framebuffer;
static byte *framebuffer2;

void put_vc(int x, int y, unsigned short *dest, char ch, int vc)
{
	if (vc == 0xffff) {
		*dest = 0xffff;
		int o = y * resx * font_height + x * font_width;
		for (int i=0; i<font_height; i++) {
			memset(framebuffer2+o, 0, font_width*bytes_per_pixel);
			o += resx*bytes_per_pixel;
		}
	} else {
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
}

struct FFH {
	byte magic[4] HTPACKED;
	byte height HTPACKED;
	byte dist HTPACKED;
	byte res0[11] HTPACKED;
};

struct FCHAR {
	byte width;
	word data[0];
};

bool loadfont(char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f) return false;
	FFH hdr;
	fread(&hdr, 1, sizeof hdr, f);
	unsigned int c = 256 * (2 * hdr.height + 1);
	font = (byte*)malloc(c);
	memset(font, 0, c);
	font_width = 1;
	font_height = hdr.height;
	font_bytes = 2;
	if (fread(font, 1, c, f) != c) return false;
	for (int i=0; i<256; i++) {
		byte width = *(font + (font_height*2+1)*i) /*+1*/;
		if (width > font_width) font_width = width;
	}
	fclose(f);
	return true;
}

/*
 *	CLASS screendrawbuf
 */

struct VbeInfoBlock {
	byte magic[4] HTPACKED;
	word version HTPACKED;
	dword oem_string_ptr HTPACKED;
	dword caps HTPACKED;
	dword video_modes_ptr HTPACKED;
	word memory HTPACKED;			/* in 64k blocks */
/* VBE 2.0 */
	word revison HTPACKED;
	dword vendor_name_ptr HTPACKED;
	dword product_name_ptr HTPACKED;
	dword product_revision_ptr HTPACKED;
	byte res0[222] HTPACKED;
	byte res1[256] HTPACKED;
};

struct ModeInfoBlock {
/* VBE all versions */
	word mode_attrs;
	byte wina_attrs;
	byte winb_attrs;
	word win_granularity;
	word win_size;
	word wina_start;
	word winb_start;
	dword win_func_ptr;
	word bytes_per_scanline;
/* VBE 1.2+ */
	word resx;
	word resy;
	byte char_width;
	byte char_height;
	byte planes;
	byte bits_per_pixel;
	byte banks;
	byte memory_model;
	byte bank_size;
	byte image_pages;
	byte res0;
/* direct color fields */
	byte res1[9];
/* VBE 2.0 */
	dword linear_buffer_addr;
	dword off_screen_ofs;
	word off_screen_size;			/* in 1k blocks */
/* pad */
	byte pad[206];
};

#define PARAGRAPHS(size) (((size)+15)>>4)

bool prepare_vesa_mode(int mode)
{
	mode = 0x4000 | (mode & 0x1ff);
	__dpmi_regs regs;
	
	/* alloc VBE info block in DOS mem */
	int vbe_info_seg, vbe_info_sel;
	vbe_info_seg = __dpmi_allocate_dos_memory(PARAGRAPHS(sizeof (VbeInfoBlock)), &vbe_info_sel);
	if (vbe_info_seg == -1) return false;
	VbeInfoBlock vbe_info;

	/* alloc mode info block in DOS mem */
	int mode_info_seg, mode_info_sel;
	mode_info_seg = __dpmi_allocate_dos_memory(PARAGRAPHS(sizeof (ModeInfoBlock)), &mode_info_sel);
	if (mode_info_seg == -1) return false;
	ModeInfoBlock mode_info;

	/* fetch VBE 2.0 info block */
	char *VBE2 = "VBE2";
	movedata(_go32_my_ds(), (int)&VBE2, vbe_info_sel, 0, 4);

	memset(&regs, 0, sizeof regs);
	regs.d.eax = 0x4f00;
	regs.x.es = vbe_info_seg;
	if (__dpmi_int(0x10, &regs) == -1) return false;
	movedata(vbe_info_sel, 0, _go32_my_ds(), (int)&vbe_info, sizeof vbe_info);

	/* fetch mode info block */
	memset(&regs, 0, sizeof regs);
	regs.d.eax = 0x4f01;
	regs.d.ecx = mode;
	regs.x.es = mode_info_seg;
	if (__dpmi_int(0x10, &regs) == -1) return false;
	movedata(mode_info_sel, 0, _go32_my_ds(), (int)&mode_info, sizeof mode_info);

	/* create linear address from physical address */
	int memsize = mode_info.image_pages * mode_info.resx * mode_info.resy;

	__dpmi_meminfo addrmap;
	addrmap.address = mode_info.linear_buffer_addr;
	addrmap.size = memsize;

	if (__dpmi_physical_address_mapping(&addrmap) == -1) return false;

	lfb_addr = addrmap.address;
	lfb_size = addrmap.size;
	if (lfb_size == 0) return false;

	resx = mode_info.resx;
	resy = mode_info.resy;
	bytes_per_pixel = (mode_info.bits_per_pixel+7) >> 3;
	vidmode = mode;
	
	__dpmi_free_dos_memory(mode_info_sel);
	__dpmi_free_dos_memory(vbe_info_sel);

	return true;
}

bool switch_to_vesa_mode()
{
	if (!vidmode) return false;

	/* switch to desired video mode */
	__dpmi_regs regs;
	memset(&regs, 0, sizeof regs);
	regs.d.eax = 0x4f02;
	regs.d.ebx = vidmode;
	if (__dpmi_int(0x10, &regs) == -1) return false;

// FIXME: return regs.d.eax or sth.
	return true;
}

//#define JUST_320_200

screendrawbuf::screendrawbuf(char *title)
{
	bounds b;
	buf = 0;
	cursorx = 0;
	cursory = 0;
	cursorhidden = false;
	cursoroverwrite = false;
	hidecursor();

	if (!loadfont("P:\\src\\enew\\src\\io\\djgpp-x\\fnt\\system2.fnt"))
		HT_ERROR("couldn't load a font");

#ifndef JUST_320_200
	prepare_vesa_mode(0x101);
#else
	resx = 320;
	resy = 200;
	bytes_per_pixel = 1;
	lfb_addr = 0xa0000;
	lfb_size = 0xffff;
#endif

	screensel = __dpmi_allocate_ldt_descriptors(1);
	if (screensel == -1) HT_ERROR("couldn't allocate selector for frame buffer");
	if (__dpmi_set_descriptor_access_rights(screensel, 0xc0f3) == -1)
		HT_ERROR("couldn't set access rights of frame buffer selector");
	if (__dpmi_set_segment_base_address(screensel, lfb_addr) == -1)
		HT_ERROR("couldn't set base address of frame buffer selector");
	int lfb_size2 = (lfb_size & 0xfffff000) | 0xfff;
	if (lfb_size2 > lfb_size) lfb_size2 -= 0x1000;
	if (__dpmi_set_segment_limit(screensel, lfb_size2) == -1)
		HT_ERROR("couldn't set limit of frame buffer selector");

	int fbs = resx * resy * bytes_per_pixel;
	framebuffer = (byte*)malloc(fbs);
	memset(framebuffer, 0, fbs);
	
	framebuffer2 = (byte*)malloc(resx * resy * bytes_per_pixel);
	memset(framebuffer2, 0, fbs);

#ifndef JUST_320_200
	switch_to_vesa_mode();
#else
	asm(
		"mov $0x0013, %eax\n"
		"int $0x10\n"
	);
#endif

	b.x = 0;
	b.y = 0;
	b.w = resx / font_width;
	b.h = resy / font_height;
	b_setbounds(&b);

	show();
}

screendrawbuf::~screendrawbuf()
{
/* hack to keep prompt color white on black */
/*	b_printchar(size.w-1, size.h-1, VCP(VC_WHITE, VC_BLACK), ' ');
	
	setcursormode(false);
	setcursor(0, size.h-1);
	show();*/
	asm(
		"mov $0x0003, %eax\n"
		"int $0x10\n"
	);
	if (buf) delete buf;
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
				put_vc(x+ix, y+iy, k, ch->ch, ch->c);
			k++;
			ch++;
		}
	}
}

void screendrawbuf::b_line(int x1, int y1, int x2, int y2, int c)
{
	UINT fbsize = resx*resy*bytes_per_pixel;

	int d = 0;
	int dx = x2-x1;
	int dy = y2-y1;
	int ix = 1;
	int iy = 1;
	int maxx = resx;
	int maxy = resy;
	if (dx < 0) {
		ix = -ix;
		dx = -dx;
	}
	if (dy < 0) {
		iy = -iy;
		dy = -dy;
	}

	bool swapxy = false;
	if (dx < dy) {
		swapxy = true;
		int t;
		t = x1;
		x1 = y1;
		y1 = t;
		
		t = x2;
		x2 = y2;
		y2 = t;

		t = ix;
		ix = iy;
		iy = t;

		t = dx;
		dx = dy;
		dy = t;

		t = maxx;
		maxx = maxy;
		maxy = t;
	}
		if (dx == 0) return;
		if (x1>x2) {
			int tx=x1, ty=y1;
			x1 = x2;
			y1 = y2;
			x2 = tx;
			y2 = ty;
			iy = -iy;
		}
		int n = y1 - dy*x1/dx;
		if (x1 < 0) {
			x1 = 0;
			y1 = n;
		}
		if (y1 < 0) {
			if (dy == 0) return;
			x1 = -n*dx/dy;
			y1 = 0;
		}
		if (x2 > maxx) {
			x2 = maxx;
			y2 = dy*maxx/dx+n;
		}
		if (y2 > maxy) {
			if (dy == 0) return;
			x2 = (maxy-n)*dx/dy;
			y2 = maxy;
		}
		int y = y1;
		for (int x = x1; x<x2; x++) {
			UINT a;
			if (swapxy) {
				a = y + x*resx*bytes_per_pixel;
			} else {
				a = x + y*resx*bytes_per_pixel;
			}
			if (a < fbsize) framebuffer2[a] = c;
			d += dy;
			while (d > dx) {
				d -= dx;
				y += iy;
			}
		}
}

void screendrawbuf::b_putpixel(int x, int y, int c)
{
	UINT fbsize = resx*resy*bytes_per_pixel;
	UINT a = x + y*resx*bytes_per_pixel;
	if (a < fbsize) framebuffer2[a] = c;
}

void screendrawbuf::text_to_pixel_coord(int tx, int ty, int *px, int *py)
{
	*px = tx * font_width;
	*py = ty * font_height;
}

void screendrawbuf::pixel_to_text_coord(int px, int py, int *tx, int *ty)
{
	*tx = px / font_width;
	*ty = py / font_height;
}


void screendrawbuf::b_fill(int x, int y, int w, int h, int c, int ch)
{
	for (int i=0; i<h; i++) {
		unsigned short *b=buf+x+(y+i)*size.w;
		if (y+i>=size.h) break;
		if (y+i>=0)
		for (int j=0; j<w; j++) {
			if (x+j>=0) put_vc(x+j, y+i, b, ch, c);
			if (x+j>=size.w-1) break;
			b++;
		}
	}
}

void screendrawbuf::b_printchar(int x, int y, int c, int ch)
{
	unsigned short*b=buf+x+y*size.w;
	put_vc(x, y, b, ch, c);
}

int screendrawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	int n=0;
	unsigned short *b=buf+x+y*size.w;
	while ((*text) && (n<l)) {
		put_vc(x+n, y, b, (unsigned char)*text, c);
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
		put_vc(x+n, y, b, (unsigned char)*text, c);
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
	if (!font) return;
	gotoxy(cursorx+1, cursory+1);

	unsigned short *b = buf;
	UINT fbsize = resx*resy*bytes_per_pixel;
	for (int y = 0; y<size.h; y++) {
		UINT o = y * resx * font_height;
		for (int x = 0; x<size.w; x++) {
			if (*b == 0xffff) {
				for (int i=0; i<font_height; i++) {
					int a = o+i*resx*bytes_per_pixel;
					memmove(framebuffer+a, framebuffer2+a, font_width*bytes_per_pixel);
				}
			} else {
			
			int fg = (*b>>8) & 0xf;
			int bg = (*b>>12) & 0xf;
			unsigned char ch = *b & 0xff;
			/* put font character */
			byte width = *(font + (font_height*2+1)*ch);
			int jshift = (font_width-width)/2;
			for (int i=0; i<font_height; i++) {
				byte *chr = font + (font_height*2+1)*ch + i*2 + 1;
				word cdata = (chr[1] << 8) | chr[0];
				UINT a = o+i*resx*bytes_per_pixel;
				for (int j=0; j<font_width; j++) {
					if (a < fbsize) {
						if ((cdata << jshift) & (1 << (j))) {
							framebuffer[a] = fg;
//     	     				_farpokeb(screensel, a, fg);
						} else {
							framebuffer[a] = bg;
//     	     				_farpokeb(screensel, a, bg);
						}
					}
					a++;
				}
			}
			
			}
			b++;
			o += font_width;
		}
	}
	movedata(_go32_my_ds(), (int)framebuffer, screensel, 0, resx*resy*bytes_per_pixel);
}

void screendrawbuf::getcursor(int *x, int *y)
{
	*x = cursorx;
	*y = cursory;
}

void screendrawbuf::hidecursor()
{
	if (!cursorhidden) {
/*		__dpmi_regs r;
		r.h.ah = 1;
		r.h.ch = 31;
		r.h.cl = 30;
		__dpmi_int(0x10, &r);*/

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
/*		__dpmi_regs r;
		r.h.ah = 1;
		if (cursoroverwrite) {
			r.h.ch = 0;
			r.h.cl = 31;
		} else {
			r.h.ch = 30;
			r.h.cl = 31;
		}
		__dpmi_int(0x10, &r);*/

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

