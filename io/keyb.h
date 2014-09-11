/*
 *	HT Editor
 *	keyb.h
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

#ifndef __KEYB_H__
#define __KEYB_H__

enum ht_key {
K_FIRST = 0,

/* ASCII > 0 */

K_Backspace		= 1,
K_Shift_Tab		= 2,

K_Tab			= '\t',
K_Return		= '\n',
K_Escape		= '\e',

// Start of printables
K_Space			= ' ',
K_ExclamationPoint	= '!',
K_DoubleQuote		= '"',
K_Hash			= '#',
K_Dollar		= '$',
K_Percent		= '%',
K_Ampersand		= '&',
K_Quote			= '\'',
K_LParen		= '(',
K_RParen		= ')',
K_Star			= '*',
K_Plus			= '+',
K_Comma			= ',',
K_Dash			= '-',
K_Period		= '.',
K_Slash			= '/',
K_Colon			= ':',
K_SemiColon		= ';',
K_LAngle		= '<',
K_Equals		= '=',
K_RAngle		= '>',
K_QuestionMark		= '?',
K_At			= '@',

K_LBracket		= '[',
K_BackSlash		= '\'',
K_RBracket		= ']',
K_Caret			= '^',
K_UnderScore		= '_',
K_BackQuote		= '`',
K_LBrace		= '{',
K_Pipe			= '|',
K_RBrace		= '}',
K_Tilde			= '~',

K_0				= '0',
K_1				= '1',
K_2				= '2',
K_3				= '3',
K_4				= '4',
K_5				= '5',
K_6				= '6',
K_7				= '7',
K_8				= '8',
K_9				= '9',

K_A				= 'a',
K_B				= 'b',
K_C				= 'c',
K_D				= 'd',
K_E				= 'e',
K_F				= 'f',
K_G				= 'g',
K_H				= 'h',
K_I				= 'i',
K_J				= 'j',
K_K				= 'k',
K_L				= 'l',
K_M				= 'm',
K_N				= 'n',
K_O				= 'o',
K_P				= 'p',
K_Q				= 'q',
K_R				= 'r',
K_S				= 's',
K_T				= 't',
K_U				= 'u',
K_V				= 'v',
K_W				= 'w',
K_X				= 'x',
K_Y				= 'y',
K_Z				= 'z',

K_Shift_A			= 'A',
K_Shift_B			= 'B',
K_Shift_C			= 'C',
K_Shift_D			= 'D',
K_Shift_E			= 'E',
K_Shift_F			= 'F',
K_Shift_G			= 'G',
K_Shift_H			= 'H',
K_Shift_I			= 'I',
K_Shift_J			= 'J',
K_Shift_K			= 'K',
K_Shift_L			= 'L',
K_Shift_M			= 'M',
K_Shift_N			= 'N',
K_Shift_O			= 'O',
K_Shift_P			= 'P',
K_Shift_Q			= 'Q',
K_Shift_R			= 'R',
K_Shift_S			= 'S',
K_Shift_T			= 'T',
K_Shift_U			= 'U',
K_Shift_V			= 'V',
K_Shift_W			= 'W',
K_Shift_X			= 'X',
K_Shift_Y			= 'Y',
K_Shift_Z			= 'Z',
// end of printables

K_LASTASCII		= 0xff,

/* Special keys start here */

K_BackTick,

K_Meta_Escape,
K_Meta_Backspace,
K_Meta_Tab,
K_Meta_Enter,

K_Left,
K_Right,
K_Up,
K_Down,
K_PageUp,
K_PageDown,
K_Home,
K_End,
K_Insert,
K_Delete,

K_Control_Left,
K_Control_Right,
K_Control_Up,
K_Control_Down,
K_Control_PageUp,
K_Control_PageDown,
K_Control_Home,
K_Control_End,
K_Control_Insert,
K_Control_Delete,
K_Control_Tab,

K_Control_Shift_Left,
K_Control_Shift_Right,

K_Shift_Left,
K_Shift_Right,
K_Shift_Up,
K_Shift_Down,
K_Shift_PageUp,
K_Shift_PageDown,
K_Shift_Home,
K_Shift_End,
K_Shift_Insert,
K_Shift_Delete,

K_Meta_Left,
K_Meta_Right,
K_Meta_Up,
K_Meta_Down,
K_Meta_PageUp,
K_Meta_PageDown,
K_Meta_Home,
K_Meta_End,
K_Meta_Insert,
K_Meta_Delete,

K_Meta_A,
K_Meta_B,
K_Meta_C,
K_Meta_D,
K_Meta_E,
K_Meta_F,
K_Meta_G,
K_Meta_H,
K_Meta_I,
K_Meta_J,
K_Meta_K,
K_Meta_L,
K_Meta_M,
K_Meta_N,
K_Meta_O,
K_Meta_P,
K_Meta_Q,
K_Meta_R,
K_Meta_S,
K_Meta_T,
K_Meta_U,
K_Meta_V,
K_Meta_W,
K_Meta_X,
K_Meta_Y,
K_Meta_Z,

K_Meta_1,
K_Meta_2,
K_Meta_3,
K_Meta_4,
K_Meta_5,
K_Meta_6,
K_Meta_7,
K_Meta_8,
K_Meta_9,
K_Meta_0,

K_F1,
K_F2,
K_F3,
K_F4,
K_F5,
K_F6,
K_F7,
K_F8,
K_F9,
K_F10,
K_F11,
K_F12,

K_Shift_F1,
K_Shift_F2,
K_Shift_F3,
K_Shift_F4,
K_Shift_F5,
K_Shift_F6,
K_Shift_F7,
K_Shift_F8,
K_Shift_F9,
K_Shift_F10,
K_Shift_F11,
K_Shift_F12,

K_Control_F1,
K_Control_F2,
K_Control_F3,
K_Control_F4,
K_Control_F5,
K_Control_F6,
K_Control_F7,
K_Control_F8,
K_Control_F9,
K_Control_F10,
K_Control_F11,
K_Control_F12,

K_Control_Shift_F1,
K_Control_Shift_F2,
K_Control_Shift_F3,
K_Control_Shift_F4,
K_Control_Shift_F5,
K_Control_Shift_F6,
K_Control_Shift_F7,
K_Control_Shift_F8,
K_Control_Shift_F9,
K_Control_Shift_F10,
K_Control_Shift_F11,
K_Control_Shift_F12,

K_Control_A,
K_Control_B,
K_Control_C,
K_Control_D,
K_Control_E,
K_Control_F,
K_Control_G,
K_Control_H,
K_Control_I,
K_Control_J,
K_Control_K,
K_Control_L,
K_Control_M,
K_Control_N,
K_Control_O,
K_Control_P,
K_Control_Q,
K_Control_R,
K_Control_S,
K_Control_T,
K_Control_U,
K_Control_V,
K_Control_W,
K_Control_X,
K_Control_Y,
K_Control_Z,

K_Meta_F1,
K_Meta_F2,
K_Meta_F3,
K_Meta_F4,
K_Meta_F5,
K_Meta_F6,
K_Meta_F7,
K_Meta_F8,
K_Meta_F9,
K_Meta_F10,
K_Meta_F11,
K_Meta_F12,

K_LAST,
K_INVALID = -1
};

#define K_COUNT			((int)K_LAST)

/* system-independant (implementation in keyb.cc) */
ht_key keyb_metakey(ht_key key);		/* generate ht_key for M+key from key */
ht_key keyb_unmetakey(ht_key metakey);	/* generate ht_key for key from M+key */

void keyb_setkey(ht_key key, int rawkey);

ht_key keyb_rawkey2key(int rawkey);

bool keyb_getkeydesc(char *buf, int bufsize, ht_key k);

//#include "event.h"

struct sys_event_t;

/* system-dependant (implementation in $MYSYSTEM/ *.cc) */
bool keyb_getevent(sys_event_t &event);

/* deprecated interface */
bool keyb_keypressed();
ht_key keyb_getkey();
int keyb_getrawkey();

bool init_keyb();
void done_keyb();

#endif /* __KEYB_H__ */
