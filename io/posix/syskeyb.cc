/*
 *	HT Editor
 *	syskeyb.cc - keyboard access functions for POSIX
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
#include "io/event.h"
#include "io/keyb.h"
#include "io/types.h"

#ifdef HAVE_TEXTMODE_X11
#define META_KEY(c)    ((c) |   0x80000000)
#define UNMETA_KEY(c)  ((c) & (~0x80000000))
#define SHIFT_KEY(c)   ((c) |   0x40000000)
#define UNSHIFT_KEY(c) ((c) & (~0x40000000))
#define CTRL_KEY(c)    ((c) |   0x20000000)
#define UNCTRL_KEY(c)  ((c) & (~0x20000000))
#else
#define META_KEY(c)    ((c) |   0x80000000)
#define UNMETA_KEY(c)  ((c) & (~0x80000000))
#define SHIFT_KEY(c)   ((c) |   0x80000000)
#define UNSHIFT_KEY(c) ((c) & (~0x80000000))
#define CTRL_KEY(c)    ((((c)>='a') && ((c)<='z')) ? (c-'a'+1) : ((c) | 0x80000000))
#define UNCTRL_KEY(c)  ((c) & (~0x80000000))
#endif

#define CTRL_ALPHA_KEY(c) ((c)-'a'+1)

#ifdef HAVE_TEXTMODE_X11
#include <X11/Xlib.h>

static Display *Xdisplay;
static Window Xwindow;
#endif /* HAVE_TEXTMODE_X11 */

void sys_get_driver_desc(char *buf)
{
#ifdef HAVE_TEXTMODE_X11
	sprintf(buf, "POSIX/NCurses/X11 (X server %sconnected)", Xdisplay ? "" : "NOT ");
#else
	sprintf(buf, "POSIX/NCurses");
#endif	
}

static int get_modifier(int key)
{
#ifdef HAVE_TEXTMODE_X11
	if (Xdisplay) {
		Window root, child;
		int root_x, root_y;
		int win_x, win_y;
		unsigned int mask;
		Bool b;
		int result = key;

		b = XQueryPointer(Xdisplay, Xwindow, &root, &child,
				    &root_x, &root_y,
				    &win_x, &win_y,
				    &mask);

		if (mask & ShiftMask) result = SHIFT_KEY(result);
		if (mask & ControlMask) result = CTRL_KEY(result);
		return result;
	}
#endif
	return key;
}

static int escseq2rawkey(uint r)
{
	switch (r) {
		case 'H': return KEY_HOME;
		case 'F': return KEY_END;
		case 'P': return KEY_F(1);
		case 'Q': return KEY_F(2);
		case 'R': return KEY_F(3);
		case 'S': return KEY_F(4);
	}
	return -1;
}

/* From xterm's ctlseqs:
On button press or release, xterm sends CSI M CbCxCy. The low two bits of Cb encode button
information: 0=MB1 pressed, 1=MB2 pressed, 2=MB3 pressed, 3=release.
The next three bits encode the modifiers which were down when the button
was pressed and are added together:  4=Shift, 8=Meta, 16=Control.  Note
however that the shift and control bits are normally unavailable because
xterm uses the control modifier with mouse for popup menus, and the
shift modifier is used in the default translations for button events.
The Meta modifier recognized by xterm is the mod1 mask, and is not nec-
essarily the "Meta" key (see xmodmap).  Cx and Cy are the x and y coor-
dinates of the mouse event, encoded as in X10 mode.

Wheel mice may return buttons 4 and 5.  Those buttons are represented by
adding 64 to the event code.
*/

static bool keyb_getmouseevent(sys_event_t &event)
{
	event.type = SYSEV_MOUSE_EVENT;

	// CSI M is already parsed. now get Cb, Cx and Cy
	int b = getch();
	b -= 32;
	switch (b & 3) {
		case 0:
			if (b & 64) {
				// button 4 pressed (normally mouse wheel up)
				event.mouse_event.button_mask = MBM_BUTTON4;
			} else {
				// left button pressed
				event.mouse_event.button_mask = MBM_LEFT;
			}
			event.mouse_event.button_event = MBE_PRESSED;
			break;
		case 1:
			if (b & 64) {
				// button 5 pressed (normally mouse wheel down)
				event.mouse_event.button_mask = MBM_BUTTON5;
			} else {
				// middle button pressed
				event.mouse_event.button_mask = MBM_MIDDLE;
			}
			event.mouse_event.button_event = MBE_PRESSED;
			break;
		case 2:
			if (b & 64) {
				// button 6 pressed (whatever ?)
				event.mouse_event.button_mask = MBM_BUTTON6;
			} else {
				// right button pressed
				event.mouse_event.button_mask = MBM_RIGHT;
			}
			event.mouse_event.button_event = MBE_PRESSED;
			break;
		case 3:
			// all buttons released
			event.mouse_event.button_mask = MBM_LEFT | MBM_MIDDLE | MBM_RIGHT |
				MBM_BUTTON4 | MBM_BUTTON5 | MBM_BUTTON6;
			event.mouse_event.button_event = MBE_RELEASED;
			break;
	}
	int x = getch()-32;	// now 1-based
	int y = getch()-32;	// now 1-based
	event.mouse_event.x = x-1;	// now 0-based
	event.mouse_event.y = y-1;	// now 0-based
	return true;
}

bool keyb_getevent(sys_event_t &event)
{
	int r = keyb_getrawkey();
	if (r == KEY_MOUSE) return keyb_getmouseevent(event);
	ht_key k = K_INVALID;
	int r2 = UNMETA_KEY(UNCTRL_KEY(UNSHIFT_KEY(r)));
	if (META_KEY(r) == r && (r2 == '[' || r2 == 'O')) {/* escape seq */
		r2 = r;
		if (keyb_keypressed()) {
			r = keyb_getrawkey();
			r = escseq2rawkey(r);
#ifdef HAVE_TEXTMODE_X11
			if (CTRL_KEY(r2) == r2) r = CTRL_KEY(r);
			if (SHIFT_KEY(r2) == r2) r = SHIFT_KEY(r);
#endif
		}
	}
	k = keyb_rawkey2key(r);
	if ((k == K_INVALID) && ((unsigned int)r <= 255)) k = (ht_key)r;
	event.type = SYSEV_KEYPRESSED;
/*	if (k == 'a') {
		event.key = K_Control_PageDown;
	} else {*/
		event.key = k;
//	}
	return true;
}

ht_key keyb_getkey()
{
	int r = keyb_getrawkey();
	ht_key k = K_INVALID;
	int r2 = UNMETA_KEY(UNCTRL_KEY(UNSHIFT_KEY(r)));
	if (META_KEY(r) == r && ((r2 == '[') || (r2 == 'O'))) {/* escape seq */
		r2 = r;
		if (keyb_keypressed()) {
			r = keyb_getrawkey();
			r = escseq2rawkey(r);
#ifdef HAVE_TEXTMODE_X11
			if (CTRL_KEY(r2) == r2) r = CTRL_KEY(r);
			if (SHIFT_KEY(r2) == r2) r = SHIFT_KEY(r);
#endif
		}
	}
	k = keyb_rawkey2key(r);
	if (k == K_INVALID && (unsigned int)r <= 255) {
//	if (r == 'a') {
//		return K_Control_PageDown;
//	}
		return (ht_key)r;
	}
//debug:	if (k == K_INVALID) return (ht_key)r;
	return k;
}

struct key_keycode {
	ht_key key;
	int keycode;
};

bool keyb_keypressed()
{
	int i = getch();
	if (i != -1) ungetch(i);
	return (i != -1);
}

int keyb_getrawkey()
{
	int c = getch();
	if (c == '\e') {
		c = getch();
		if ((c == -1) || (c == '\e')) c = '\e'; else c = META_KEY(c);
	}
	if ((unsigned int)c>=0x100) c = get_modifier(c);
//	fprintf(stderr, "getrawkey() %d/0x%x\n", c, c);
	return c;
}

static key_keycode curses_key_defs[] = {
	{K_Return,		'\n'},
	{K_Delete,		KEY_DC},
	{K_Insert,		KEY_IC},
	{K_Backspace,		KEY_BACKSPACE},
	{K_Backspace,		8},
	{K_Backspace,		127},
	{K_Meta_Backspace,	META_KEY(KEY_BACKSPACE)},
	{K_Meta_Backspace,	META_KEY(8)},
	{K_Meta_Backspace,	META_KEY(127)},

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

	{K_Meta_Up,		META_KEY(KEY_UP)},
	{K_Meta_Down,		META_KEY(KEY_DOWN)},
	{K_Meta_Left,		META_KEY(KEY_LEFT)},
	{K_Meta_Right,		META_KEY(KEY_RIGHT)},
	{K_Meta_PageUp,		META_KEY(KEY_PPAGE)},
	{K_Meta_PageDown,	META_KEY(KEY_NPAGE)},
	{K_Meta_Insert,		META_KEY(KEY_IC)},
	{K_Meta_Delete,		META_KEY(KEY_DC)},
	{K_Meta_Home,		META_KEY(KEY_HOME)},
	{K_Meta_End,		META_KEY(KEY_END)},

	{K_Control_Up,		CTRL_KEY(KEY_UP)},
	{K_Control_Down,	CTRL_KEY(KEY_DOWN)},
	{K_Control_Left,	CTRL_KEY(KEY_LEFT)},
	{K_Control_Right,	CTRL_KEY(KEY_RIGHT)},
	{K_Control_PageUp,	CTRL_KEY(KEY_PPAGE)},
	{K_Control_PageDown,	CTRL_KEY(KEY_NPAGE)},
	{K_Control_Insert,	CTRL_KEY(KEY_IC)},
	{K_Control_Delete,	CTRL_KEY(KEY_DC)},
	{K_Control_Home,	CTRL_KEY(KEY_HOME)},
	{K_Control_End,		CTRL_KEY(KEY_END)},

	{K_Shift_Tab,		SHIFT_KEY('\t')},
	{K_Shift_Up,		SHIFT_KEY(KEY_UP)},
	{K_Shift_Down,		SHIFT_KEY(KEY_DOWN)},
	{K_Shift_Left,		SHIFT_KEY(KEY_LEFT)},
	{K_Shift_Right,		SHIFT_KEY(KEY_RIGHT)},
	{K_Shift_PageUp,	SHIFT_KEY(KEY_PPAGE)},
	{K_Shift_PageDown,	SHIFT_KEY(KEY_NPAGE)},
	{K_Shift_Delete,	SHIFT_KEY(KEY_DC)},
	{K_Shift_Insert,	SHIFT_KEY(KEY_IC)},
	{K_Shift_Home,		SHIFT_KEY(KEY_HOME)},
	{K_Shift_End,		SHIFT_KEY(KEY_END)},

	{K_Meta_1,		META_KEY('1')},
	{K_Meta_2,		META_KEY('2')},
	{K_Meta_3,		META_KEY('3')},
	{K_Meta_4,		META_KEY('4')},
	{K_Meta_5,		META_KEY('5')},
	{K_Meta_6,		META_KEY('6')},
	{K_Meta_7,		META_KEY('7')},
	{K_Meta_8,		META_KEY('8')},
	{K_Meta_9,		META_KEY('9')},
	{K_Meta_0,		META_KEY('0')},

	{K_Control_A,		CTRL_ALPHA_KEY('a')},
	{K_Control_B,		CTRL_ALPHA_KEY('b')},
	{K_Control_C,		CTRL_ALPHA_KEY('c')},
	{K_Control_D,		CTRL_ALPHA_KEY('d')},
	{K_Control_E,		CTRL_ALPHA_KEY('e')},
	{K_Control_F,		CTRL_ALPHA_KEY('f')},
	{K_Control_G,		CTRL_ALPHA_KEY('g')},
	{K_Control_H,		CTRL_ALPHA_KEY('h')},
	{K_Control_I,		CTRL_ALPHA_KEY('i')},
	{K_Control_J,		CTRL_ALPHA_KEY('j')},
	{K_Control_K,		CTRL_ALPHA_KEY('k')},
	{K_Control_L,		CTRL_ALPHA_KEY('l')},
	{K_Control_M,		CTRL_ALPHA_KEY('m')},
	{K_Control_N,		CTRL_ALPHA_KEY('n')},
	{K_Control_O,		CTRL_ALPHA_KEY('o')},
	{K_Control_P,		CTRL_ALPHA_KEY('p')},
	{K_Control_Q,		CTRL_ALPHA_KEY('q')},
	{K_Control_R,		CTRL_ALPHA_KEY('r')},
	{K_Control_S,		CTRL_ALPHA_KEY('s')},
	{K_Control_T,		CTRL_ALPHA_KEY('t')},
	{K_Control_U,		CTRL_ALPHA_KEY('u')},
	{K_Control_V,		CTRL_ALPHA_KEY('v')},
	{K_Control_W,		CTRL_ALPHA_KEY('w')},
	{K_Control_X,		CTRL_ALPHA_KEY('x')},
	{K_Control_Y,		CTRL_ALPHA_KEY('y')},
	{K_Control_Z,		CTRL_ALPHA_KEY('z')},

	{K_Meta_A,		META_KEY('a')},
	{K_Meta_B,		META_KEY('b')},
	{K_Meta_C,		META_KEY('c')},
	{K_Meta_D,		META_KEY('d')},
	{K_Meta_E,		META_KEY('e')},
	{K_Meta_F,		META_KEY('f')},
	{K_Meta_G,		META_KEY('g')},
	{K_Meta_H,		META_KEY('h')},
	{K_Meta_I,		META_KEY('i')},
	{K_Meta_J,		META_KEY('j')},
	{K_Meta_K,		META_KEY('k')},
	{K_Meta_L,		META_KEY('l')},
	{K_Meta_M,		META_KEY('m')},
	{K_Meta_N,		META_KEY('n')},
	{K_Meta_O,		META_KEY('o')},
	{K_Meta_P,		META_KEY('p')},
	{K_Meta_Q,		META_KEY('q')},
	{K_Meta_R,		META_KEY('r')},
	{K_Meta_S,		META_KEY('s')},
	{K_Meta_T,		META_KEY('t')},
	{K_Meta_U,		META_KEY('u')},
	{K_Meta_V,		META_KEY('v')},
	{K_Meta_W,		META_KEY('w')},
	{K_Meta_X,		META_KEY('x')},
	{K_Meta_Y,		META_KEY('y')},
	{K_Meta_Z,		META_KEY('z')},

/*	{K_Shift_F1,		SHIFT_KEY(KEY_F(1))},
	{K_Shift_F2,		SHIFT_KEY(KEY_F(2))},
	{K_Shift_F3,		SHIFT_KEY(KEY_F(3))},
	{K_Shift_F4,		SHIFT_KEY(KEY_F(4))},
	{K_Shift_F5,		SHIFT_KEY(KEY_F(5))},
	{K_Shift_F6,		SHIFT_KEY(KEY_F(6))},
	{K_Shift_F7,		SHIFT_KEY(KEY_F(7))},
	{K_Shift_F8,		SHIFT_KEY(KEY_F(8))},
	{K_Shift_F9,		SHIFT_KEY(KEY_F(9))},
	{K_Shift_F10,		SHIFT_KEY(KEY_F(10))},
	{K_Shift_F11,		SHIFT_KEY(KEY_F(11))},
	{K_Shift_F12,		SHIFT_KEY(KEY_F(12))},*/
	{K_Shift_F1,		SHIFT_KEY(KEY_F(1+12))},
	{K_Shift_F2,		SHIFT_KEY(KEY_F(2+12))},
	{K_Shift_F3,		SHIFT_KEY(KEY_F(3+12))},
	{K_Shift_F4,		SHIFT_KEY(KEY_F(4+12))},
	{K_Shift_F5,		SHIFT_KEY(KEY_F(5+12))},
	{K_Shift_F6,		SHIFT_KEY(KEY_F(6+12))},
	{K_Shift_F7,		SHIFT_KEY(KEY_F(7+12))},
	{K_Shift_F8,		SHIFT_KEY(KEY_F(8+12))},
	{K_Shift_F9,		SHIFT_KEY(KEY_F(9+12))},
	{K_Shift_F10,		SHIFT_KEY(KEY_F(10+12))},
	{K_Shift_F11,		SHIFT_KEY(KEY_F(11+12))},
	{K_Shift_F12,		SHIFT_KEY(KEY_F(12+12))},

	{K_Control_Shift_F1,	CTRL_KEY(SHIFT_KEY(KEY_F(1+12)))},
	{K_Control_Shift_F2,	CTRL_KEY(SHIFT_KEY(KEY_F(2+12)))},
	{K_Control_Shift_F3,	CTRL_KEY(SHIFT_KEY(KEY_F(3+12)))},
	{K_Control_Shift_F4,	CTRL_KEY(SHIFT_KEY(KEY_F(4+12)))},
	{K_Control_Shift_F5,	CTRL_KEY(SHIFT_KEY(KEY_F(5+12)))},
	{K_Control_Shift_F6,	CTRL_KEY(SHIFT_KEY(KEY_F(6+12)))},
	{K_Control_Shift_F7,	CTRL_KEY(SHIFT_KEY(KEY_F(7+12)))},
	{K_Control_Shift_F8,	CTRL_KEY(SHIFT_KEY(KEY_F(8+12)))},
	{K_Control_Shift_F9,	CTRL_KEY(SHIFT_KEY(KEY_F(9+12)))},
	{K_Control_Shift_F10,	CTRL_KEY(SHIFT_KEY(KEY_F(10+12)))},
	{K_Control_Shift_F11,	CTRL_KEY(SHIFT_KEY(KEY_F(11+12)))},
	{K_Control_Shift_F12,	CTRL_KEY(SHIFT_KEY(KEY_F(12+12)))},

	{K_Control_F1,		CTRL_KEY(KEY_F(1))},
	{K_Control_F2,		CTRL_KEY(KEY_F(2))},
	{K_Control_F3,		CTRL_KEY(KEY_F(3))},
	{K_Control_F4,		CTRL_KEY(KEY_F(4))},
	{K_Control_F5,		CTRL_KEY(KEY_F(5))},
	{K_Control_F6,		CTRL_KEY(KEY_F(6))},
	{K_Control_F7,		CTRL_KEY(KEY_F(7))},
	{K_Control_F8,		CTRL_KEY(KEY_F(8))},
	{K_Control_F9,		CTRL_KEY(KEY_F(9))},
	{K_Control_F10,		CTRL_KEY(KEY_F(10))},
	{K_Control_F11,		CTRL_KEY(KEY_F(11))},
	{K_Control_F12,		CTRL_KEY(KEY_F(12))},

	{K_Meta_F1,		META_KEY(KEY_F(1))},
	{K_Meta_F2,		META_KEY(KEY_F(2))},
	{K_Meta_F3,		META_KEY(KEY_F(3))},
	{K_Meta_F4,		META_KEY(KEY_F(4))},
	{K_Meta_F5,		META_KEY(KEY_F(5))},
	{K_Meta_F6,		META_KEY(KEY_F(6))},
	{K_Meta_F7,		META_KEY(KEY_F(7))},
	{K_Meta_F8,		META_KEY(KEY_F(8))},
	{K_Meta_F9,		META_KEY(KEY_F(9))},
	{K_Meta_F10,		META_KEY(KEY_F(10))},
	{K_Meta_F11,		META_KEY(KEY_F(11))},
	{K_Meta_F12,		META_KEY(KEY_F(12))},
};

bool init_keyb()
{
	uint i;

#ifdef HAVE_TEXTMODE_X11
	Xdisplay = XOpenDisplay(0);
	if (Xdisplay) Xwindow = DefaultRootWindow(Xdisplay);
#endif /* HAVE_TEXTMODE_X11 */

	for (i=0; i<K_COUNT; i++) {
		keyb_setkey((ht_key)i, -1);
	}

	for (i=0; i<sizeof curses_key_defs/ sizeof curses_key_defs[0]; i++) {
		int kc = curses_key_defs[i].keycode;
#ifdef HAVE_TEXTMODE_X11
		if (!Xdisplay) {
			if (kc & 0x40000000) kc = (kc & ~0x40000000) | 0x80000000;
			if (kc & 0x20000000) kc = (kc & ~0x20000000) | 0x80000000;
		}
#endif /* HAVE_TEXTMODE_X11 */
		keyb_setkey(curses_key_defs[i].key, kc);
//		fprintf(stderr, "%x - %x\n", curses_key_defs[i].key, kc);
	}
//	fprintf(stderr, "%d/%x\n", KEY_F(1), KEY_F(1));

	/* Mouse support */

	// xterm (VT200-like)
//	printf("\e[?1001s");	// save old highlight mouse tracking
//	printf("\e[?1000h");	// enable mouse tracking aka. receive mouse events through stdin

	return true;
}

void done_keyb()
{
#ifdef HAVE_TEXTMODE_X11
	if (Xdisplay) XCloseDisplay(Xdisplay);
#endif /* HAVE_TEXTMODE_X11 */

	fflush(stdout);
	fflush(stdin);

	/* Mouse support */

	// xterm (VT200-like)
//	printf("\e[?1000l");	// disable mouse tracking
//	printf("\e[?1001r");	// restore old highlight mouse tracking
}
