/* 
 *	HT Editor
 *	htkeyb.cc (WIN32 implementation)
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


HANDLE input_handle;
INPUT_RECORD key_event_record;
bool key_pending=false;
bool k_shift_state=false;
bool k_ctrl_state=false;
bool k_alt_state=false;

//#define KEY_DEBUG

#ifdef KEY_DEBUG
FILE *kd;
#endif

struct ht_key_keycode {
	ht_key key;
	int keycode;
};

#define HT_VK                  0x300
#define HT_VK_ALT              (0x1000)
#define HT_VK_SHIFT            (0x2000)
#define HT_VK_CTRL             (0x3000)

ht_key_keycode ht_win32_key_defs[] = {

	{K_Control_A,			(0x001+HT_VK_CTRL)},
	{K_Control_B,			(0x002+HT_VK_CTRL)},
	{K_Control_C,			(0x003+HT_VK_CTRL)},
	{K_Control_D,			(0x004+HT_VK_CTRL)},
	{K_Control_E,			(0x005+HT_VK_CTRL)},
	{K_Control_F,			(0x006+HT_VK_CTRL)},
	{K_Control_G,			(0x007+HT_VK_CTRL)},
	{K_Backspace,			0x008},
	{K_Control_H,			(0x008+HT_VK_CTRL)},
	{K_Tab,				0x009},
	{K_Control_I,			(0x009+HT_VK_CTRL)},
	{K_Control_J,			(0x00a+HT_VK_CTRL)},
	{K_Control_K,			(0x00b+HT_VK_CTRL)},
	{K_Control_L,			(0x00c+HT_VK_CTRL)},
	{K_Return,			0x00d},
	{K_Control_M,			(0x00d+HT_VK_CTRL)},
	{K_Control_N,			(0x00e + HT_VK_CTRL)},	// cpp-bug :-)
	{K_Control_O,			(0x00f+HT_VK_CTRL)},
	{K_Control_P,			(0x010+HT_VK_CTRL)},
	{K_Control_Q,			(0x011+HT_VK_CTRL)},
	{K_Control_R,			(0x012+HT_VK_CTRL)},
	{K_Control_S,			(0x013+HT_VK_CTRL)},
	{K_Control_T,			(0x014+HT_VK_CTRL)},
	{K_Control_U,			(0x015+HT_VK_CTRL)},
	{K_Control_V,			(0x016+HT_VK_CTRL)},
	{K_Control_W,			(0x017+HT_VK_CTRL)},
	{K_Control_X,			(0x018+HT_VK_CTRL)},
	{K_Control_Y,			(0x019+HT_VK_CTRL)},
	{K_Control_Z,			(0x01a+HT_VK_CTRL)},
	{K_Escape,			0x01b},
//	{K_Control_LBracket,	0x01b},
//	{K_Control_BackSlash,	0x01c},
//	{K_Control_RBracket,	0x01d},
//	{K_Control_Caret,		0x01e},
//	{K_Control_Underscore,	0x01f},
//	{K_Control_Backspace,	(K_Backspace+HT_VK_CTRL)},

	{K_Alt_Escape,			(K_Escape+HT_VK_ALT)},
//	{K_Control_At,			0x103},
	{K_Alt_Backspace,		(K_Backspace+HT_VK_ALT)},
	{K_BackTab,			HT_VK_SHIFT+0x0009},
	{K_Alt_Q,				('q'+HT_VK_ALT)},
	{K_Alt_W,				('w'+HT_VK_ALT)},
	{K_Alt_E,				('e'+HT_VK_ALT)},
	{K_Alt_R,				('r'+HT_VK_ALT)},
	{K_Alt_T,				('t'+HT_VK_ALT)},
	{K_Alt_Y,				('y'+HT_VK_ALT)},
	{K_Alt_U,				('u'+HT_VK_ALT)},
	{K_Alt_I,				('i'+HT_VK_ALT)},
	{K_Alt_O,				('o'+HT_VK_ALT)},
	{K_Alt_P,				('p'+HT_VK_ALT)},
//	{K_Alt_LBracket,		0x11a},
//	{K_Alt_RBracket,		0x11b},
//	{K_Alt_Return,			0x11c},
	{K_Alt_A,				('a'+HT_VK_ALT)},
	{K_Alt_S,				('s'+HT_VK_ALT)},
	{K_Alt_D,				('d'+HT_VK_ALT)},
	{K_Alt_F,				('f'+HT_VK_ALT)},
	{K_Alt_G,				('g'+HT_VK_ALT)},
	{K_Alt_H,				('h'+HT_VK_ALT)},
	{K_Alt_J,				('j'+HT_VK_ALT)},
	{K_Alt_K,				('k'+HT_VK_ALT)},
	{K_Alt_L,				('l'+HT_VK_ALT)},
//	{K_Alt_Semicolon,		0x127},
//	{K_Alt_Quote,			0x128},
//	{K_Alt_Backquote,		0x129},
//	{K_Alt_Backslash,		0x12b},
	{K_Alt_Z,				('z'+HT_VK_ALT)},
	{K_Alt_X,				('x'+HT_VK_ALT)},
	{K_Alt_C,				('c'+HT_VK_ALT)},
	{K_Alt_V,				('v'+HT_VK_ALT)},
	{K_Alt_B,				('b'+HT_VK_ALT)},
	{K_Alt_N,				('n'+HT_VK_ALT)},
	{K_Alt_M,				('m'+HT_VK_ALT)},
//	{K_Alt_Comma,			0x133},
//	{K_Alt_Period,			0x134},
//	{K_Alt_Slash,			0x135},
//	{K_Alt_KPStar,			0x137},
	{K_F1,				(VK_F1+HT_VK)},
	{K_F2,				(VK_F2+HT_VK)},
	{K_F3,				(VK_F3+HT_VK)},
	{K_F4,				(VK_F4+HT_VK)},
	{K_F5,				(VK_F5+HT_VK)},
	{K_F6,				(VK_F6+HT_VK)},
	{K_F7,				(VK_F7+HT_VK)},
	{K_F8,				(VK_F8+HT_VK)},
	{K_F9,				(VK_F9+HT_VK)},
	{K_F10,				(VK_F10+HT_VK)},
	{K_Home,				(VK_HOME+HT_VK)},
	{K_Up,				(VK_UP+HT_VK)},
	{K_PageUp,			(VK_PRIOR+HT_VK)},
//	{K_Alt_KPMinus,		0x14a},
	{K_Left,				(VK_LEFT+HT_VK)},
//	{K_Center,			0x14c},
	{K_Right,				(VK_RIGHT+HT_VK)},
//	{K_Alt_KPPlus,			0x14e},
	{K_End,				(VK_END+HT_VK)},
	{K_Down,				(VK_DOWN+HT_VK)},
	{K_PageDown,			(VK_NEXT+HT_VK)},
	{K_Insert,			(VK_INSERT+HT_VK)},
	{K_Delete,			(VK_DELETE+HT_VK)},
	{K_Shift_F1,			(VK_F1+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F2,			(VK_F2+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F3,			(VK_F3+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F4,			(VK_F4+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F5,			(VK_F5+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F6,			(VK_F6+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F7,			(VK_F7+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F8,			(VK_F8+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F9,			(VK_F9+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F10,			(VK_F10+HT_VK+HT_VK_SHIFT)},
	{K_Control_F1,			(VK_F1+HT_VK+HT_VK_CTRL)},
	{K_Control_F2,			(VK_F2+HT_VK+HT_VK_CTRL)},
	{K_Control_F3,			(VK_F3+HT_VK+HT_VK_CTRL)},
	{K_Control_F4,			(VK_F4+HT_VK+HT_VK_CTRL)},
	{K_Control_F5,			(VK_F5+HT_VK+HT_VK_CTRL)},
	{K_Control_F6,			(VK_F6+HT_VK+HT_VK_CTRL)},
	{K_Control_F7,			(VK_F7+HT_VK+HT_VK_CTRL)},
	{K_Control_F8,			(VK_F8+HT_VK+HT_VK_CTRL)},
	{K_Control_F9,			(VK_F9+HT_VK+HT_VK_CTRL)},
	{K_Control_F10,		(VK_F10+HT_VK+HT_VK_CTRL)},
	{K_Alt_F1,			(VK_F1+HT_VK+HT_VK_ALT)},
	{K_Alt_F2,			(VK_F2+HT_VK+HT_VK_ALT)},
	{K_Alt_F3,			(VK_F3+HT_VK+HT_VK_ALT)},
	{K_Alt_F4,			(VK_F4+HT_VK+HT_VK_ALT)},
	{K_Alt_F5,			(VK_F5+HT_VK+HT_VK_ALT)},
	{K_Alt_F6,			(VK_F6+HT_VK+HT_VK_ALT)},
	{K_Alt_F7,			(VK_F7+HT_VK+HT_VK_ALT)},
	{K_Alt_F8,			(VK_F8+HT_VK+HT_VK_ALT)},
	{K_Alt_F9,			(VK_F9+HT_VK+HT_VK_ALT)},
	{K_Alt_F10,			(VK_F10+HT_VK+HT_VK_ALT)},
//	{K_Control_Print		0x172},
	{K_Control_Left,		(VK_LEFT+HT_VK_CTRL+HT_VK)},
	{K_Control_Right,		(VK_RIGHT+HT_VK_CTRL+HT_VK)},
	{K_Control_End,		(VK_END+HT_VK_CTRL+HT_VK)},
	{K_Control_PageDown,	(VK_NEXT+HT_VK_CTRL+HT_VK)},
	{K_Control_Home,		(VK_HOME+HT_VK_CTRL+HT_VK)},
	{K_Alt_1,				('1'+HT_VK_ALT)},
	{K_Alt_2,				('2'+HT_VK_ALT)},
	{K_Alt_3,				('3'+HT_VK_ALT)},
	{K_Alt_4,				('4'+HT_VK_ALT)},
	{K_Alt_5,				('5'+HT_VK_ALT)},
	{K_Alt_6,				('6'+HT_VK_ALT)},
	{K_Alt_7,				('7'+HT_VK_ALT)},
	{K_Alt_8,				('8'+HT_VK_ALT)},
	{K_Alt_9,				('9'+HT_VK_ALT)},
	{K_Alt_0,				('0'+HT_VK_ALT)},
//	{K_Alt_Dash,			0x182},
//	{K_Alt_Equals,			0x183},
	{K_Control_PageUp,		(VK_PRIOR+HT_VK_CTRL+HT_VK)},
	{K_F11,				(VK_F11+HT_VK+HT_VK)},
	{K_F12,				(VK_F12+HT_VK+HT_VK)},
	{K_Shift_F11,			(VK_F11+HT_VK+HT_VK_SHIFT)},
	{K_Shift_F12,			(VK_F12+HT_VK+HT_VK_SHIFT)},
	{K_Control_F11,		(VK_F11+HT_VK+HT_VK_CTRL)},
	{K_Control_F12,		(VK_F12+HT_VK+HT_VK_CTRL)},
	{K_Alt_F11,			(VK_F11+HT_VK+HT_VK_ALT)},
	{K_Alt_F12,			(VK_F12+HT_VK+HT_VK_ALT)},
	{K_Control_Up,			(VK_UP+HT_VK_CTRL+HT_VK)},
//	{K_Control_KPDash,		0x18e},
//	{K_Control_Center,		0x18f},
//	{K_Control_KPPlus,		0x190},
	{K_Control_Down,		(VK_DOWN+HT_VK_CTRL+HT_VK)},
	{K_Control_Insert,		(VK_INSERT+HT_VK_CTRL+HT_VK)},
	{K_Control_Delete,		(VK_DELETE+HT_VK_CTRL+HT_VK)},
//	{K_Control_KPSlash,		0x195},
//	{K_Control_KPStar,		0x196},
	{K_Alt_Tab,			(0x0009+HT_VK_ALT+HT_VK)},
	{K_Alt_Enter,			0x1a6},

	{K_Shift_Up,			(HT_VK_SHIFT+VK_UP+HT_VK)},
	{K_Shift_Down,			(HT_VK_SHIFT+VK_DOWN+HT_VK)},
	{K_Shift_Left,			(HT_VK_SHIFT+VK_LEFT+HT_VK)},
	{K_Shift_Right,		(HT_VK_SHIFT+VK_RIGHT+HT_VK)},
	{K_Shift_PageUp,		(HT_VK_SHIFT+VK_PRIOR+HT_VK)},
	{K_Shift_PageDown,		(HT_VK_SHIFT+VK_NEXT+HT_VK)},

	{K_Shift_Home,			(HT_VK_SHIFT+VK_HOME+HT_VK)},
	{K_Shift_End,			(HT_VK_SHIFT+VK_END+HT_VK)},
	{K_Shift_Delete,		(HT_VK_SHIFT+VK_DELETE+HT_VK)},
	{K_Shift_Insert,		(HT_VK_SHIFT+VK_INSERT+HT_VK)},

	{K_INVALID,			-1}

};

bool ht_keypressed()
{
	DWORD read;
	if (key_pending) return true;
	while (1) {
		PeekConsoleInputA(input_handle, &key_event_record, 1, &read);
		if (!read) return false;
		ReadConsoleInputA(input_handle, &key_event_record, 1, &read);
		if (key_event_record.EventType & KEY_EVENT) {
			switch (key_event_record.Event.KeyEvent.wVirtualKeyCode) {
				case VK_CONTROL:
					k_ctrl_state=key_event_record.Event.KeyEvent.bKeyDown;
					break;
				case VK_MENU:
					k_alt_state=key_event_record.Event.KeyEvent.bKeyDown;
					break;
			}
			if (key_event_record.Event.KeyEvent.bKeyDown) {
				key_pending = true;
				return true;
			}
		}
	}
}

int ht_key_meta(bool shift, bool alt)
{
    return (((key_event_record.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) && (shift)) ? HT_VK_SHIFT : 0)
		+((key_event_record.Event.KeyEvent.dwControlKeyState &(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) ? HT_VK_CTRL : 0)
		+(((key_event_record.Event.KeyEvent.dwControlKeyState &(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) && (alt)) ? HT_VK_ALT : 0);
}

int ht_raw_getkey()
{
	if (!ht_keypressed()) return -1;
	key_pending = false;
#ifdef KEY_DEBUG
	if (key_event_record.Event.KeyEvent.uChar.AsciiChar) {
		fprintf(kd, "Ascii: %3d [%c]\n", key_event_record.Event.KeyEvent.uChar.AsciiChar + ht_key_meta(false, false), key_event_record.Event.KeyEvent.uChar.AsciiChar + ht_key_meta(false, false));
	} else {
		fprintf(kd, "VK: %3d  shift: %d, alt: %d, ctrl: %d\n", key_event_record.Event.KeyEvent.wVirtualKeyCode, k_shift_state, k_alt_state, k_ctrl_state);
	}
#endif
	if (key_event_record.Event.KeyEvent.uChar.AsciiChar && !(key_event_record.Event.KeyEvent.dwControlKeyState & 256)) {
		/*
		 *	Local keys, which can only be access via AltGr
		 *	[in german e.g. '\'=(altgr+'-') and '@'=(altgr+'q')]
		 *   are handled separately
		 */
		switch (key_event_record.Event.KeyEvent.uChar.AsciiChar) {
			case '\\':
			case '@':
			case '|':
			case 'ý':
			case '~':
			case '{':
			case '}':
			case '[':
			case ']':
				return ((unsigned char)key_event_record.Event.KeyEvent.uChar.AsciiChar) + ht_key_meta(false, false);
			default:
				return ((unsigned char)key_event_record.Event.KeyEvent.uChar.AsciiChar) + ht_key_meta(false, true);
		}
	} else {
//     	fprintf(kd, "returning %d, F10 is %d\n", key_event_record.Event.KeyEvent.wVirtualKeyCode + HT_VK + ht_key_meta(true, true),
		return key_event_record.Event.KeyEvent.wVirtualKeyCode + HT_VK + ht_key_meta(true, true);
	}
}

ht_key ht_getkey()
{
	UINT r=ht_raw_getkey();
	ht_key k=ht_rawkey2key(r);
	if ((k==K_INVALID) && (r<=255)) return (ht_key)r;
	return k;
}

bool init_keyb()
{
	input_handle = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(input_handle, 0); //ENABLE_PROCESSED_INPUT);
#ifdef KEY_DEBUG
	kd = fopen("_kd.kd", "wt");
#endif

	int i;
	
	for (i=0; i<K_COUNT; i++) {
		ht_set_key((ht_key)i, -1);
	}

	for (i=0; ht_win32_key_defs[i].key!=K_INVALID; i++) {
		ht_set_key(ht_win32_key_defs[i].key, ht_win32_key_defs[i].keycode);
	}
	
	return true;
}

void done_keyb()
{
#ifdef KEY_DEBUG
	fclose(kd);
#endif
}
