/*
 *	HT Editor
 *	htkeyb.cc (POSIX implementation)
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

#include "htkeyb.h"

#include "config.h"
#include CURSES_HDR

//#define HAVE_TEXTMODE_X11

#ifdef HAVE_TEXTMODE_X11
#define HT_META_KEY(c)    ((c) |   0x80000000)
#define HT_UNMETA_KEY(c)  ((c) & (~0x80000000))
#define HT_SHIFT_KEY(c)   ((c) |   0x40000000)
#define HT_UNSHIFT_KEY(c) ((c) & (~0x40000000))
#define HT_CTRL_KEY(c)    ((c) |   0x20000000)
#define HT_UNCTRL_KEY(c)  ((c) & (~0x20000000))
#else
#define HT_META_KEY(c)    ((c) |   0x80000000)
#define HT_UNMETA_KEY(c)  ((c) & (~0x80000000))
#define HT_SHIFT_KEY(c)   ((c) |   0x80000000)
#define HT_UNSHIFT_KEY(c) ((c) & (~0x80000000))
#define HT_CTRL_KEY(c)    ((((c)>='a') && ((c)<='z')) ? (c-'a'+1) : ((c) |   0x80000000))
#define HT_UNCTRL_KEY(c)  ((c) & (~0x80000000))
#endif

#ifdef HAVE_TEXTMODE_X11
#include <X11/Xlib.h>

static Display *x11_display;
static Window x11_window;
#endif /* HAVE_TEXTMODE_X11 */

int get_modifier(int key)
{
#ifdef HAVE_TEXTMODE_X11
	if (x11_display) {
		Window root, child;
		int root_x, root_y;
		int win_x, win_y;
		unsigned int mask;
		Bool b;
		int result = key;

		b = XQueryPointer(x11_display, x11_window, &root, &child,
				    &root_x, &root_y,
				    &win_x, &win_y,
				    &mask);

		if (mask & ShiftMask) result = HT_SHIFT_KEY(result);
		if (mask & ControlMask) result = HT_CTRL_KEY(result);
		return result;
	}
#endif
	return key;
}

UINT ht_escseq2rawkey(UINT r)
{
	switch (r) {
		case 'H': return KEY_HOME;
		case 'F': return KEY_END;
	}
	return K_INVALID;
}

ht_key ht_getkey()
{
	UINT r = ht_raw_getkey();
	ht_key k = K_INVALID;
	if (r == HT_META_KEY('[')) {		/* ^[ escape sequence */
		if (ht_keypressed()) {
			r = ht_raw_getkey();
			r = ht_escseq2rawkey(r);
		}		    
	}
	r = get_modifier(r);
	k = ht_rawkey2key(r);
	if ((k == K_INVALID) && (r <= 255)) return (ht_key)r;
	return k;
}

struct ht_key_keycode {
	ht_key key;
	int keycode;
};

// only works for A-Z !
//#define HT_CONTROL_KEY(c) (char)(c-'A'+1)

// my curses impl maps shift+f-x to f-(x+12), only works up to shift+f8...
//#define HT_SHIFT_FKEY(n) (KEY_F0+12+n)

ht_key_keycode ht_curses_key_defs[] = {
/*	{K_Escape		'\e'
#define K_Return		0x0a
#define K_Space		 	0x20*/
	{K_Return,		'\n'},
	{K_Delete,		KEY_DC},
	{K_Insert,		KEY_IC},
	{K_BackSpace,		KEY_BACKSPACE},

	{K_F1,			KEY_F(1)},
	{K_F2,			KEY_F(2)},
	{K_F3,			KEY_F(3)},
	{K_F4,			KEY_F(4)},
	{K_F5,			KEY_F(5)},
	{K_F6,			KEY_F(6)},
	{K_F7,			KEY_F(7)},
	{K_F8,			KEY_F(8)},
	{K_F9,			KEY_F(9)},
	{K_F10,			KEY_F(10)},
	{K_F11,			KEY_F(11)},
	{K_F12,			KEY_F(12)},
	{K_Home,		KEY_HOME},
	{K_End,			KEY_END},
	{K_Up,			KEY_UP},
	{K_Down,		KEY_DOWN},
	{K_Left,		KEY_LEFT},
	{K_Right,		KEY_RIGHT},
	{K_PageUp,		KEY_PPAGE},
	{K_PageDown,		KEY_NPAGE},
	{K_Tab,			'\t'},
	{K_BackTab,		KEY_BTAB},
	{K_Control_PageUp,	HT_CTRL_KEY(KEY_PPAGE)},
	{K_Control_PageDown,	HT_CTRL_KEY(KEY_NPAGE)},
	{K_Control_Left,	HT_CTRL_KEY(KEY_LEFT)},
	{K_Control_Right,	HT_CTRL_KEY(KEY_RIGHT)},
	{K_Control_Up,		HT_CTRL_KEY(KEY_UP)},
	{K_Control_Down,	HT_CTRL_KEY(KEY_DOWN)},
	{K_Shift_Up,		HT_SHIFT_KEY(KEY_UP)},
	{K_Shift_Down,		HT_SHIFT_KEY(KEY_DOWN)},
	{K_Shift_Left,		HT_SHIFT_KEY(KEY_LEFT)},
	{K_Shift_Right,		HT_SHIFT_KEY(KEY_RIGHT)},
	{K_Shift_PageUp,	HT_SHIFT_KEY(KEY_PPAGE)},
	{K_Shift_PageDown,	HT_SHIFT_KEY(KEY_NPAGE)},
/*	{K_Shift_Delete,	KEY_SDC},
	{K_Shift_Insert,	KEY_SIC},*/
	{K_Shift_Delete,	HT_SHIFT_KEY(KEY_DC)},
	{K_Shift_Insert,	HT_SHIFT_KEY(KEY_IC)},
	{K_Shift_End,		HT_SHIFT_KEY(KEY_END)},
	{K_Shift_Home,		HT_SHIFT_KEY(KEY_HOME)},
/*	{K_Shift_End,		KEY_SEND},
	{K_Shift_Home,		KEY_SHOME},*/
//	{K_Control_F5,		HT_CTRL_KEY('V')},
/*	{K_Alt_F1,		-10},
	{K_Alt_F3,		-11},
	{K_Alt_F5,		-12},*/
	{K_Control_Insert,	HT_CTRL_KEY(KEY_IC)},
	{K_Control_Delete,	HT_CTRL_KEY(KEY_DC)},

	{K_Alt_1,               HT_META_KEY('1')},
	{K_Alt_2,               HT_META_KEY('2')},
	{K_Alt_3,               HT_META_KEY('3')},
	{K_Alt_4,               HT_META_KEY('4')},
	{K_Alt_5,               HT_META_KEY('5')},
	{K_Alt_6,               HT_META_KEY('6')},
	{K_Alt_7,               HT_META_KEY('7')},
	{K_Alt_8,               HT_META_KEY('8')},
	{K_Alt_9,               HT_META_KEY('9')},
	{K_Alt_0,               HT_META_KEY('0')},

	{K_Alt_A,               HT_META_KEY('a')},
	{K_Alt_B,               HT_META_KEY('b')},
	{K_Alt_C,	        HT_META_KEY('c')},
	{K_Alt_D,               HT_META_KEY('d')},
	{K_Alt_E,               HT_META_KEY('e')},
	{K_Alt_F,               HT_META_KEY('f')},
	{K_Alt_G,	        HT_META_KEY('g')},
	{K_Alt_H,               HT_META_KEY('h')},
	{K_Alt_I,               HT_META_KEY('i')},
	{K_Alt_J,               HT_META_KEY('j')},
	{K_Alt_K,	        HT_META_KEY('k')},
	{K_Alt_L,               HT_META_KEY('l')},
	{K_Alt_M,               HT_META_KEY('m')},
	{K_Alt_N,               HT_META_KEY('n')},
	{K_Alt_O,	        HT_META_KEY('o')},
	{K_Alt_P,               HT_META_KEY('p')},
	{K_Alt_Q,               HT_META_KEY('q')},
	{K_Alt_R,               HT_META_KEY('r')},
	{K_Alt_S,	        HT_META_KEY('s')},
	{K_Alt_T,               HT_META_KEY('t')},
	{K_Alt_U,               HT_META_KEY('u')},
	{K_Alt_V,               HT_META_KEY('v')},
	{K_Alt_W,	        HT_META_KEY('w')},
	{K_Alt_X,               HT_META_KEY('x')},
	{K_Alt_Y,               HT_META_KEY('y')},
	{K_Alt_Z,               HT_META_KEY('z')},

	{K_Control_A,		HT_CTRL_KEY('a')},
	{K_Control_B,		HT_CTRL_KEY('b')},
	{K_Control_C,		HT_CTRL_KEY('c')},
	{K_Control_D,		HT_CTRL_KEY('d')},
	{K_Control_E,		HT_CTRL_KEY('e')},
	{K_Control_F,		HT_CTRL_KEY('f')},
	{K_Control_G,		HT_CTRL_KEY('g')},
	{K_Control_H,		HT_CTRL_KEY('h')},
	{K_Control_I,		HT_CTRL_KEY('i')},
	{K_Control_J,		HT_CTRL_KEY('j')},
	{K_Control_K,		HT_CTRL_KEY('k')},
	{K_Control_L,		HT_CTRL_KEY('l')},
	{K_Control_M,		HT_CTRL_KEY('m')},
	{K_Control_N,		HT_CTRL_KEY('n')},
	{K_Control_O,		HT_CTRL_KEY('o')},
	{K_Control_P,		HT_CTRL_KEY('p')},
	{K_Control_Q,		HT_CTRL_KEY('q')},
	{K_Control_R,		HT_CTRL_KEY('r')},
	{K_Control_S,		HT_CTRL_KEY('s')},
	{K_Control_T,		HT_CTRL_KEY('t')},
	{K_Control_U,		HT_CTRL_KEY('u')},
	{K_Control_V,		HT_CTRL_KEY('v')},
	{K_Control_W,		HT_CTRL_KEY('w')},
	{K_Control_X,		HT_CTRL_KEY('x')},
	{K_Control_Y,		HT_CTRL_KEY('y')},
	{K_Control_Z,		HT_CTRL_KEY('z')},

	{K_Shift_F1,		HT_SHIFT_KEY(KEY_F(1))},
	{K_Shift_F2,		HT_SHIFT_KEY(KEY_F(2))},
	{K_Shift_F3,		HT_SHIFT_KEY(KEY_F(3))},
	{K_Shift_F4,		HT_SHIFT_KEY(KEY_F(4))},
	{K_Shift_F5,		HT_SHIFT_KEY(KEY_F(5))},
	{K_Shift_F6,		HT_SHIFT_KEY(KEY_F(6))},
	{K_Shift_F7,		HT_SHIFT_KEY(KEY_F(7))},
	{K_Shift_F8,		HT_SHIFT_KEY(KEY_F(8))},
	{K_Shift_F9,		HT_SHIFT_KEY(KEY_F(9))},
	{K_Shift_F10,		HT_SHIFT_KEY(KEY_F(10))},
	{K_Shift_F11,		HT_SHIFT_KEY(KEY_F(11))},
	{K_Shift_F12,		HT_SHIFT_KEY(KEY_F(12))},

	{K_Control_F1,		HT_CTRL_KEY(KEY_F(1))},
	{K_Control_F2,		HT_CTRL_KEY(KEY_F(2))},
	{K_Control_F3,		HT_CTRL_KEY(KEY_F(3))},
	{K_Control_F4,		HT_CTRL_KEY(KEY_F(4))},
	{K_Control_F5,		HT_CTRL_KEY(KEY_F(5))},
	{K_Control_F6,		HT_CTRL_KEY(KEY_F(6))},
	{K_Control_F7,		HT_CTRL_KEY(KEY_F(7))},
	{K_Control_F8,		HT_CTRL_KEY(KEY_F(8))},
	{K_Control_F9,		HT_CTRL_KEY(KEY_F(9))},
	{K_Control_F10,		HT_CTRL_KEY(KEY_F(10))},
	{K_Control_F11,		HT_CTRL_KEY(KEY_F(11))},
	{K_Control_F12,		HT_CTRL_KEY(KEY_F(12))}
};

bool ht_keypressed()
{
	int i=getch();
	if (i!=-1) ungetch(i);
	return (i!=-1);
}

int ht_raw_getkey()
{
	int c = getch();
	if (c=='\e') {
		c=getch();
		if ((c==-1) || (c=='\e')) c='\e'; else c=HT_META_KEY(c);
	}
	return c;
}

/*
 *	INIT
 */
 
bool init_keyb()
{
	UINT i;
	
#ifdef HAVE_TEXTMODE_X11
	x11_display = XOpenDisplay(0);
	if (x11_display) x11_window = DefaultRootWindow(x11_display);
#endif /* HAVE_TEXTMODE_X11 */

	for (i=0; i<K_COUNT; i++) {
		ht_set_key((ht_key)i, -1);
	}

	for (i=0; i<sizeof ht_curses_key_defs/ sizeof ht_curses_key_defs[0]; i++) {
		ht_set_key(ht_curses_key_defs[i].key, ht_curses_key_defs[i].keycode);
	}
	
	return true;
}

/*
 *	DONE
 */
 
void done_keyb()
{
#ifdef HAVE_TEXTMODE_X11
	if (x11_display) XCloseDisplay(x11_display);
#endif /* HAVE_TEXTMODE_X11 */
}
