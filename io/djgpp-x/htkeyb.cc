/*
 *	HT Editor
 *	htkeyb.cc (DJGPP implementation)
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

#include "htkeyb.h"

#include <pc.h>
#include <stdio.h>
#include <bios.h>

#define SHIFT_KEY(k)		 (k | 0x80000000)

bool ht_shift_pressed()
{
	return (bioskey(2) & 3);
}

bool ht_keypressed()
{
	return kbhit();
}

int ht_raw_getkey()
{
	int k=-1;
	if (ht_shift_pressed()) {
		k=getkey();
		switch (k) {
			case 0x14b:	/* K_Left */
			case 0x14d:	/* K_Right */
			case 0x148:	/* K_Up */
			case 0x150:	/* K_Down */
			case 0x149:	/* K_PageUp */
			case 0x151:	/* K_PageDown */
			case 0x147:	/* K_Home */
			case 0x14f:	/* K_End */
			case 0x152:	/* K_Insert */
			case 0x153:	/* K_Delete */
			case 0x173:	/* K_Control_Left */
			case 0x174:	/* K_Control_Right */
				k=SHIFT_KEY(k);
				break;
		}
	} else k=getkey();
	return k;
}

ht_key ht_getkey()
{
	UINT r=ht_raw_getkey();
	ht_key k=ht_rawkey2key(r);
	if ((k==K_INVALID) && (r<=255)) return (ht_key)r;
	return k;
}

struct ht_key_keycode {
	ht_key key;
	int keycode;
};

ht_key_keycode ht_dj_key_defs[] = {
	{K_BackSpace,			0x008},
	{K_BackTab,			0x10f},
	
	{K_Tab,				0x009},
	{K_Return,			0x00d},
	{K_Escape,			0x01b},
	
	{K_Control_A,			0x001},
	{K_Control_B,			0x002},
	{K_Control_C,			0x003},
	{K_Control_D,			0x004},
	{K_Control_E,			0x005},
	{K_Control_F,			0x006},
	{K_Control_G,			0x007},
	{K_Control_H,			0x008},
	{K_Control_I,			0x009},
	{K_Control_J,			0x00a},
	{K_Control_K,			0x00b},
	{K_Control_L,			0x00c},
	{K_Control_M,			0x00d},
	{K_Control_N,			0x00e},
	{K_Control_O,			0x00f},
	{K_Control_P,			0x010},
	{K_Control_Q,			0x011},
	{K_Control_R,			0x012},
	{K_Control_S,			0x013},
	{K_Control_T,			0x014},
	{K_Control_U,			0x015},
	{K_Control_V,			0x016},
	{K_Control_W,			0x017},
	{K_Control_X,			0x018},
	{K_Control_Y,			0x019},
	{K_Control_Z,			0x01a},
	
	{K_Alt_Escape,			0x101},
	{K_Alt_Backspace,		0x10e},
	{K_Alt_Tab,			0x1a5},
	{K_Alt_Enter,			0x1a6},
	
	{K_Alt_A,				0x11e},
	{K_Alt_B,				0x130},
	{K_Alt_C,				0x12e},
	{K_Alt_D,				0x120},
	{K_Alt_E,				0x112},
	{K_Alt_F,				0x121},
	{K_Alt_G,				0x122},
	{K_Alt_H,				0x123},
	{K_Alt_I,				0x117},
	{K_Alt_J,				0x124},
	{K_Alt_K,				0x125},
	{K_Alt_L,				0x126},
	{K_Alt_M,				0x132},
	{K_Alt_N,				0x131},
	{K_Alt_O,				0x118},
	{K_Alt_P,				0x119},
	{K_Alt_Q,				0x110},
	{K_Alt_R,				0x113},
	{K_Alt_S,				0x11f},
	{K_Alt_T,				0x114},
	{K_Alt_U,				0x116},
	{K_Alt_V,				0x12f},
	{K_Alt_W,				0x111},
	{K_Alt_X,				0x12d},
	{K_Alt_Y,				0x115},
	{K_Alt_Z,				0x12c},
	
	{K_F1,				0x13b},
	{K_F2,				0x13c},
	{K_F3,				0x13d},
	{K_F4,				0x13e},
	{K_F5,				0x13f},
	{K_F6,				0x140},
	{K_F7,				0x141},
	{K_F8,				0x142},
	{K_F9,				0x143},
	{K_F10,				0x144},
	{K_F11,				0x185},
	{K_F12,				0x186},
	
	{K_Shift_F1,			0x154},
	{K_Shift_F2,			0x155},
	{K_Shift_F3,			0x156},
	{K_Shift_F4,			0x157},
	{K_Shift_F5,			0x158},
	{K_Shift_F6,			0x159},
	{K_Shift_F7,			0x15a},
	{K_Shift_F8,			0x15b},
	{K_Shift_F9,			0x15c},
	{K_Shift_F10,			0x15d},
	{K_Shift_F11,			0x187},
	{K_Shift_F12,			0x188},
	
	{K_Control_F1,			0x15e},
	{K_Control_F2,			0x15f},
	{K_Control_F3,			0x160},
	{K_Control_F4,			0x161},
	{K_Control_F5,			0x162},
	{K_Control_F6,			0x163},
	{K_Control_F7,			0x164},
	{K_Control_F8,			0x165},
	{K_Control_F9,			0x166},
	{K_Control_F10,		0x167},
	{K_Control_F11,		0x189},
	{K_Control_F12,		0x18a},
	
	{K_Alt_F1,			0x168},
	{K_Alt_F2,			0x169},
	{K_Alt_F3,			0x16a},
	{K_Alt_F4,			0x16b},
	{K_Alt_F5,			0x16c},
	{K_Alt_F6,			0x16d},
	{K_Alt_F7,			0x16e},
	{K_Alt_F8,			0x16f},
	{K_Alt_F9,			0x170},
	{K_Alt_F10,			0x171},
	{K_Alt_F11,			0x18b},
	{K_Alt_F12,			0x18c},
	
	{K_Alt_1,				0x178},
	{K_Alt_2,				0x179},
	{K_Alt_3,				0x17a},
	{K_Alt_4,				0x17b},
	{K_Alt_5,				0x17c},
	{K_Alt_6,				0x17d},
	{K_Alt_7,				0x17e},
	{K_Alt_8,				0x17f},
	{K_Alt_9,				0x180},
	{K_Alt_0,				0x181},
		
	{K_Left,				0x14b},
	{K_Right,				0x14d},
	{K_Up,				0x148},
	{K_Down,				0x150},
	{K_PageUp,			0x149},
	{K_PageDown,			0x151},
	{K_Home,				0x147},
	{K_End,				0x14f},
	{K_Insert,			0x152},
	{K_Delete,			0x153},
	
	{K_Control_Left,		0x173},
	{K_Control_Right,		0x174},
	{K_Control_Up,			0x18d},
	{K_Control_Down,		0x191},
	{K_Control_PageUp,		0x184},
	{K_Control_PageDown,	0x176},
	{K_Control_Home,		0x177},
	{K_Control_End,		0x175},
	{K_Control_Insert,		0x192},
	{K_Control_Delete,		0x193},
	
	{K_Shift_Left,			SHIFT_KEY(0x14b)},
	{K_Shift_Right,		SHIFT_KEY(0x14d)},
	{K_Shift_Up,			SHIFT_KEY(0x148)},
	{K_Shift_Down,			SHIFT_KEY(0x150)},
	{K_Shift_PageUp,		SHIFT_KEY(0x149)},
	{K_Shift_PageDown,		SHIFT_KEY(0x151)},
	{K_Shift_Home,			SHIFT_KEY(0x147)},
	{K_Shift_End,			SHIFT_KEY(0x14f)},
	{K_Shift_Insert,		SHIFT_KEY(0x152)},
	{K_Shift_Delete,		SHIFT_KEY(0x153)},
	{K_Control_Shift_Left,	SHIFT_KEY(0x173)},
	{K_Control_Shift_Right,	SHIFT_KEY(0x174)}
};

/*
 *	INIT
 */
 
bool init_keyb()
{
	UINT i;
	
	for (i=0; i<K_COUNT; i++) {
		ht_set_key((ht_key)i, -1);
	}

	for (i=0; i<sizeof ht_dj_key_defs/sizeof ht_dj_key_defs[0]; i++) {
		ht_set_key(ht_dj_key_defs[i].key, ht_dj_key_defs[i].keycode);
	}
	
	return true;
}

/*
 *	DONE
 */
 
void done_keyb()
{
}

