/* 
 *	HT Editor
 *	io.cc
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

#include "htio.h"

#include "htsys.h"		// for sys_is_path_delim

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 *	COMMON SYS
 */

#ifndef S_IFMT
#define S_IFMT 0xf000
#endif

#ifndef S_ISREG
#	ifndef S_IFREG
#		define S_ISREG(m) (0)
#	else
#		define S_ISREG(m) (((m) & S_IFMT)==S_IFREG)
#	endif
#endif

#ifndef S_ISBLK
#	ifndef S_IFBLK
#		define S_ISBLK(m) (0)
#	else
#		define S_ISBLK(m) (((m) & S_IFMT)==S_IFBLK)
#	endif
#endif


#ifndef S_ISCHR
#	ifndef S_IFCHR
#		define S_ISCHR(m) (0)
#	else
#		define S_ISCHR(m) (((m) & S_IFMT)==S_IFCHR)
#	endif
#endif

#ifndef S_ISDIR
#	ifndef S_IFDIR
#		define S_ISDIR(m) (0)
#	else
#		define S_ISDIR(m) (((m) & S_IFMT)==S_IFDIR)
#	endif
#endif

#ifndef S_ISFIFO
#	ifndef S_IFFIFO
#		define S_ISFIFO(m) (0)
#	else
#		define S_ISFIFO(m) (((m) & S_IFMT)==S_IFFIFO)
#	endif
#endif

#ifndef S_ISLNK
#	ifndef S_IFLNK
#		define S_ISLNK(m) (0)
#	else
#		define S_ISLNK(m) (((m) & S_IFMT)==S_IFLNK)
#	endif
#endif

#ifndef S_ISSOCK
#	ifndef S_IFSOCK
#		define S_ISSOCK(m) (0)
#	else
#		define S_ISSOCK(m) (((m) & S_IFMT)==S_IFSOCK)
#	endif
#endif

#ifndef S_IRUSR
#define S_IRUSR 0
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif

#ifndef S_IWUSR
#define S_IWUSR 0
#endif
#ifndef S_IWGRP
#define S_IWGRP 0
#endif
#ifndef S_IWOTH
#define S_IWOTH 0
#endif

#ifndef S_IXUSR
#define S_IXUSR 0
#endif
#ifndef S_IXGRP
#define S_IXGRP 0
#endif
#ifndef S_IXOTH
#define S_IXOTH 0
#endif

int sys_basename(char *result, const char *filename)
{
// FIXME: use is_path_delim
	char *slash1 = strrchr(filename, '/');
	char *slash2 = strrchr(filename, '\\');
	char *slash=(slash1>slash2) ? slash1 : slash2;
	if (slash) {
		int l=strlen(filename);
		strncpy(result, slash+1, l-(slash-filename)-1);
		result[l-(slash-filename)-1]=0;
		return 0;
	}
	strcpy(result, filename);
	return 0;
}

int sys_dirname(char *result, const char *filename)
{
// FIXME: use is_path_delim
	char *slash1 = strrchr(filename, '/');
	char *slash2 = strrchr(filename, '\\');
	char *slash = (slash1>slash2) ? slash1 : slash2;
	if (slash) {
		strncpy(result, filename, slash-filename);
		result[slash-filename] = 0;
		return 0;
	}
	strcpy(result, ".");
	return 0;
}

/* filename and pathname must be canonicalized */
int sys_relname(char *result, const char *filename, const char *cwd)
{
	const char *f = filename, *p = cwd;
	while ((*f == *p) && (*f)) {
		f++;
		p++;
	}
	if (*f == '/') f++;
	const char *last = f, *h = f;
	while (*h) {
		if (*h == '/') {
			*(result++) = '.';
			*(result++) = '.';
			*(result++) = '/';
			last = h+1;
		}
		h++;
	}
	while (f<last) {
		*(result++) = *f;
		f++;
	}
	*result = 0;
	strcat(result, last);
	return 0;
}

int sys_ht_mode(int mode)
{
	int m = 0;
	if (S_ISREG(mode)) {
		m |= HT_S_IFREG;
	} else if (S_ISBLK(mode)) {
		m |= HT_S_IFBLK;
	} else if (S_ISCHR(mode)) {
		m |= HT_S_IFCHR;
	} else if (S_ISDIR(mode)) {
		m |= HT_S_IFDIR;
	} else if (S_ISFIFO(mode)) {
		m |= HT_S_IFFIFO;
	} else if (S_ISLNK(mode)) {
		m |= HT_S_IFLNK;
	} else if (S_ISSOCK(mode)) {
		m |= HT_S_IFSOCK;
	}
	if (mode & S_IRUSR) m |= HT_S_IRUSR;
	if (mode & S_IRGRP) m |= HT_S_IRGRP;
	if (mode & S_IROTH) m |= HT_S_IROTH;
	
	if (mode & S_IWUSR) m |= HT_S_IWUSR;
	if (mode & S_IWGRP) m |= HT_S_IWGRP;
	if (mode & S_IWOTH) m |= HT_S_IWOTH;
	
	if (mode & S_IXUSR) m |= HT_S_IXUSR;
	if (mode & S_IXGRP) m |= HT_S_IXGRP;
	if (mode & S_IXOTH) m |= HT_S_IXOTH;
	return m;
}

static char *next_delim(char *s, is_path_delim delim)
{
	while (*s) {
		s++;
		if (delim(*s)) return s;
	}
	return NULL;
}

static int flatten_path(char *path, is_path_delim delim)
{
	if (!path || !*path)
		return 0;
	char *q = next_delim(path, delim);
	int pp = flatten_path(q, delim);
	int ll = q ? (q-path-1) : strlen(path)-1;
	if ((ll == 2) && (strncmp(path+1, "..", 2) == 0)) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
		pp++;
	} else if ((ll == 1) && (strncmp(path+1, ".", 1) == 0)) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
	} else if (pp) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
		pp--;
	}
	return pp;
}

bool sys_path_is_absolute(const char *filename, is_path_delim delim)
{
	return delim(filename[0]) || (isalpha(filename[0]) && (filename[1] == ':'));
}

int sys_common_canonicalize(char *result, const char *filename, const char *cwd, is_path_delim delim)
{
	char *o = result;
	if (!sys_path_is_absolute(filename, delim)) {
		if (cwd) strcpy(o, cwd); else *o = 0;
		int ol = strlen(o);
		if (ol && !delim(o[ol-1])) {
			o[ol] = '/';
			o[ol+1] = 0;
		}
	} else *o = 0;
	strcat(o, filename);
	int k = flatten_path(o, delim);
	return (k==0) ? 0 : EINVAL;
}

char *sys_filename_suffix(const char *fn)
{
	const char *s = NULL;
	while (fn && *fn) {
		if (sys_is_path_delim(*fn)) s = fn+1;
		fn++;
	}
	char *p = s ? strrchr(s, '.') : NULL;
	return p ? p+1 : NULL;
}

int sys_tmpfile()
{
// FIXME: this might leak something...
    FILE *f = tmpfile();
    return fileno(f);
}

/*
 *	COMMON CURSES
 */

#define IS_FGTRANS(c) (VC_GET_BASECOLOR(VCP_FOREGROUND((c)))==VC_TRANSPARENT)
#define IS_BGTRANS(c) (VC_GET_BASECOLOR(VCP_BACKGROUND((c)))==VC_TRANSPARENT)

vcp vcp_mix(vcp base, vcp layer)
{
	int fg, bg;
	if (IS_FGTRANS(layer)) {
		fg=VCP_FOREGROUND(base);
	} else {
		fg=VCP_FOREGROUND(layer);
	}
	if (IS_BGTRANS(layer)) {
		bg=VCP_BACKGROUND(base);
	} else {
		bg=VCP_BACKGROUND(layer);
	}
	if (IS_FGTRANS(layer) && (fg != VC_TRANSPARENT) && (fg == bg)) fg=(fg+1)%8;
	if (IS_BGTRANS(layer) && (bg != VC_TRANSPARENT) && (fg == bg)) bg=(bg+1)%8;
	return VCP(fg, bg);
}

/*
 *	CLASS genericdrawbuf
 */

genericdrawbuf::genericdrawbuf()
{
}

genericdrawbuf::~genericdrawbuf()
{
}

void genericdrawbuf::b_line(int px1, int py1, int px2, int py2, int c)
{
}

void genericdrawbuf::b_putpixel(int px, int py, int c)
{
}

void genericdrawbuf::text_to_pixel_coord(int tx, int ty, int *px, int *py)
{
	*px = tx;
	*py = ty;
}

void genericdrawbuf::pixel_to_text_coord(int px, int py, int *tx, int *ty)
{
	*tx = px;
	*ty = py;
}

void genericdrawbuf::b_fill(int x, int y, int w, int h, int c, int ch)
{
}

int genericdrawbuf::b_printf(int x, int y, int c, char *format, ...)
{
	char buf[1024];
	va_list arg;
	va_start(arg, format);
	vsprintf(buf, format, arg);
	va_end(arg);
	return b_print(x, y, c, buf);
}

int genericdrawbuf::b_print(int x, int y, int c, char *text)
{
	return b_lprint(x, y, c, 0x7fffffff, text);
}

int genericdrawbuf::b_printw(int x, int y, int c, int *text)
{
	return b_lprintw(x, y, c, 0x7fffffff, text);
}

void genericdrawbuf::b_printchar(int x, int y, int c, int ch)
{
}

int genericdrawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	return 0;
}

int genericdrawbuf::b_lprintw(int x, int y, int c, int l, int *text)
{
	return 0;
}

void genericdrawbuf::b_resize(int rw, int rh)
{
	size.w+=rw;
	size.h+=rh;
	b_setbounds(&size);
}

void genericdrawbuf::b_rmove(int rx, int ry)
{
	size.x+=rx;
	size.y+=ry;
}

void genericdrawbuf::b_setbounds(bounds *b)
{
	size=*b;
}

/*
 *	CLASS drawbuf
 */
 
drawbuf::drawbuf(bounds *b)
{
	buf=0;
	b_setbounds(b);
}

drawbuf::~drawbuf()
{
	if (buf) {
		delete buf;
	}
}

void drawbuf::b_fill(int x, int y, int w, int h, int c, int ch)
{
	x-=size.x;
	y-=size.y;
	for (int iy=y; iy<y+h; iy++) {
		if (iy<size.h) {
			drawbufch *b=buf+x+iy*size.w;
			for (int ix=x; ix<x+w; ix++) {
				b->ch=ch;
				b->c=vcp_mix(b->c, c);
				b++;
			}
		}
	}
}

void drawbuf::b_printchar(int x, int y, int c, int ch)
{
	drawbufch *b=buf+(x-size.x)+(y-size.y)*size.w;
	b->ch=ch;
	b->c=vcp_mix(b->c, c);
}

int drawbuf::b_lprint(int x, int y, int c, int l, char *text)
{
	int ox=0;
	x-=size.x;
	y-=size.y;
	drawbufch *b=buf+x+y*size.w;
	if (y<size.h) {
		while ((ox<size.w) && (*text) && (ox<l)) {
			b->ch=(byte)*text;
			b->c=vcp_mix(b->c, c);
			text++;
			ox++;
			b++;
		}
	}
	return ox;
}

int drawbuf::b_lprintw(int x, int y, int c, int l, int *text)
{
	int ox=0;
	x-=size.x;
	y-=size.y;
	drawbufch *b=buf+x+y*size.w;
	if (y<size.h) {
		while ((ox<size.w) && (*text) && (ox<l)) {
			b->ch=(byte)*text;
			b->c=vcp_mix(b->c, c);
			text++;
			ox++;
			b++;
		}
	}
	return ox;
}

void drawbuf::b_setbounds(bounds *b)
{
	genericdrawbuf::b_setbounds(b);
	if (buf) delete buf;
	if (size.w * size.h) {
		buf=(drawbufch*)malloc(sizeof *buf * size.w * size.h);
		drawbufch *bb=buf;
		for (int iy=0; iy<size.h; iy++) {
			for (int ix=0; ix<size.w; ix++) {
				bb->ch=' ';
				bb->c=VCP(VC_TRANSPARENT, VC_TRANSPARENT);
				bb++;
			}
		}
	} else buf=0;
}

/*
 *	COMMON KEYB
 */
 
ht_key ht_unmetakey(ht_key key)
{
	switch (key) {
		case K_Alt_A: return K_A;
		case K_Alt_B: return K_B;
		case K_Alt_C: return K_C;
		case K_Alt_D: return K_D;
		case K_Alt_E: return K_E;
		case K_Alt_F: return K_F;
		case K_Alt_G: return K_G;
		case K_Alt_H: return K_H;
		case K_Alt_I: return K_I;
		case K_Alt_J: return K_J;
		case K_Alt_K: return K_K;
		case K_Alt_L: return K_L;
		case K_Alt_M: return K_M;
		case K_Alt_N: return K_N;
		case K_Alt_O: return K_O;
		case K_Alt_P: return K_P;
		case K_Alt_Q: return K_Q;
		case K_Alt_R: return K_R;
		case K_Alt_S: return K_S;
		case K_Alt_T: return K_T;
		case K_Alt_U: return K_U;
		case K_Alt_V: return K_V;
		case K_Alt_W: return K_W;
		case K_Alt_X: return K_X;
		case K_Alt_Y: return K_Y;
		case K_Alt_Z: return K_Z;
		default: return K_INVALID;
	}
}

ht_key ht_lmetakey(ht_key key)
{
	switch (key) {
		case K_A: return K_Alt_A;
		case K_B: return K_Alt_B;
		case K_C: return K_Alt_C;
		case K_D: return K_Alt_D;
		case K_E: return K_Alt_E;
		case K_F: return K_Alt_F;
		case K_G: return K_Alt_G;
		case K_H: return K_Alt_H;
		case K_I: return K_Alt_I;
		case K_J: return K_Alt_J;
		case K_K: return K_Alt_K;
		case K_L: return K_Alt_L;
		case K_M: return K_Alt_M;
		case K_N: return K_Alt_N;
		case K_O: return K_Alt_O;
		case K_P: return K_Alt_P;
		case K_Q: return K_Alt_Q;
		case K_R: return K_Alt_R;
		case K_S: return K_Alt_S;
		case K_T: return K_Alt_T;
		case K_U: return K_Alt_U;
		case K_V: return K_Alt_V;
		case K_W: return K_Alt_W;
		case K_X: return K_Alt_X;
		case K_Y: return K_Alt_Y;
		case K_Z: return K_Alt_Z;
		default: return K_INVALID;
	}
}

ht_key ht_metakey(ht_key key)
{
	if ((key>=K_A) && (key<=K_Z)) {
		return ht_lmetakey(key);
/*	} else if ((key>=K_Shift_A) && (key<=K_Shift_Z)) {
		return ht_lmetakey( (ht_key) ((int)key-(int)K_Alt_A+(int)K_A));*/
	}
	return K_INVALID;
}

int ht_keys[K_COUNT];

ht_key ht_rawkey2key(int rawkey)
{
	for (int i=0; i<K_COUNT; i++) {
		if (ht_keys[i]==(int)rawkey) return (ht_key)i;
	}
	return K_INVALID;
}

void ht_set_key(ht_key key, int rawkey)
{
	if ((int)key<K_COUNT) {
		ht_keys[(int)key] = rawkey;
	}
}

