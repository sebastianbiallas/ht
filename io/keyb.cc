/* 
 *	HT Editor
 *	keyb.cc
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

#include <cstdio>
#include <cstring>

#include "keyb.h"
#include "strtools.h"

ht_key keyb_unmetakey(ht_key key)
{
	switch (key) {
		case K_Meta_A: return K_A;
		case K_Meta_B: return K_B;
		case K_Meta_C: return K_C;
		case K_Meta_D: return K_D;
		case K_Meta_E: return K_E;
		case K_Meta_F: return K_F;
		case K_Meta_G: return K_G;
		case K_Meta_H: return K_H;
		case K_Meta_I: return K_I;
		case K_Meta_J: return K_J;
		case K_Meta_K: return K_K;
		case K_Meta_L: return K_L;
		case K_Meta_M: return K_M;
		case K_Meta_N: return K_N;
		case K_Meta_O: return K_O;
		case K_Meta_P: return K_P;
		case K_Meta_Q: return K_Q;
		case K_Meta_R: return K_R;
		case K_Meta_S: return K_S;
		case K_Meta_T: return K_T;
		case K_Meta_U: return K_U;
		case K_Meta_V: return K_V;
		case K_Meta_W: return K_W;
		case K_Meta_X: return K_X;
		case K_Meta_Y: return K_Y;
		case K_Meta_Z: return K_Z;
		default: return K_INVALID;
	}
}

static ht_key keyb_lmetakey(ht_key key)
{
	switch (key) {
		case K_A: return K_Meta_A;
		case K_B: return K_Meta_B;
		case K_C: return K_Meta_C;
		case K_D: return K_Meta_D;
		case K_E: return K_Meta_E;
		case K_F: return K_Meta_F;
		case K_G: return K_Meta_G;
		case K_H: return K_Meta_H;
		case K_I: return K_Meta_I;
		case K_J: return K_Meta_J;
		case K_K: return K_Meta_K;
		case K_L: return K_Meta_L;
		case K_M: return K_Meta_M;
		case K_N: return K_Meta_N;
		case K_O: return K_Meta_O;
		case K_P: return K_Meta_P;
		case K_Q: return K_Meta_Q;
		case K_R: return K_Meta_R;
		case K_S: return K_Meta_S;
		case K_T: return K_Meta_T;
		case K_U: return K_Meta_U;
		case K_V: return K_Meta_V;
		case K_W: return K_Meta_W;
		case K_X: return K_Meta_X;
		case K_Y: return K_Meta_Y;
		case K_Z: return K_Meta_Z;
		default: return K_INVALID;
	}
}

ht_key keyb_metakey(ht_key key)
{
	if ((key>=K_A) && (key<=K_Z)) {
		return keyb_lmetakey(key);
/*	} else if ((key>=K_Shift_A) && (key<=K_Shift_Z)) {
		return ht_lmetakey( (ht_key) ((int)key-(int)K_Meta_A+(int)K_A));*/
	}
	return K_INVALID;
}

static int ht_keys1[K_COUNT];
static int ht_keys2[K_COUNT];
static int ht_keys3[K_COUNT];

ht_key keyb_rawkey2key(int rawkey)
{
	for (int i=0; i<K_COUNT; i++) {
		if (ht_keys1[i]==(int)rawkey) return (ht_key)i;
		if (ht_keys2[i]==(int)rawkey) return (ht_key)i;
		if (ht_keys3[i]==(int)rawkey) return (ht_key)i;
	}
	return K_INVALID;
}

void keyb_setkey(ht_key key, int rawkey)
{
	int i = (int)key;
	if (i<K_COUNT) {
		if (rawkey == -1) {
			ht_keys1[i] = -1;
			ht_keys2[i] = -1;
			ht_keys3[i] = -1;
		}
		else if ((ht_keys1[i] == -1) || (ht_keys1[i] == rawkey)) ht_keys1[i] = rawkey;
		else if ((ht_keys2[i] == -1) || (ht_keys2[i] == rawkey)) ht_keys2[i] = rawkey;
		else if ((ht_keys3[i] == -1) || (ht_keys3[i] == rawkey)) ht_keys3[i] = rawkey;
	}
}

#if defined(WIN32) || defined(__WIN32__)
#define META_KEY_NAME "Alt"
#else
#define META_KEY_NAME "Meta"
#endif

bool keyb_getkeydesc(char *buf, int bufsize, ht_key k)
{
	char b2[64];
	const char *b;
	bool r = true;
	switch (k) {
		case K_Backspace: b = "Backspace"; break;
		case K_Meta_Backspace: b = "Alt+Backspace"; break;
		case K_Return: b = "Enter"; break;
		case K_Tab: b = "Tab"; break;
		case K_Escape: b = "Escape"; break;

		case K_Left: b = "Left"; break;
		case K_Right: b = "Right"; break;
		case K_Up: b = "Up"; break;
		case K_Down: b = "Down"; break;
		case K_PageUp: b = "PgUp"; break;
		case K_PageDown: b = "PgDn"; break;
		case K_Insert: b = "Insert"; break;
		case K_Delete: b = "Delete"; break;
		case K_Home: b = "Home"; break;
		case K_End: b = "End"; break;

		case K_Meta_Left: b = META_KEY_NAME"+Left"; break;
		case K_Meta_Right: b = META_KEY_NAME"+Right"; break;
		case K_Meta_Up: b = META_KEY_NAME"+Up"; break;
		case K_Meta_Down: b = META_KEY_NAME"+Down"; break;
		case K_Meta_PageUp: b = META_KEY_NAME"+PgUp"; break;
		case K_Meta_PageDown: b = META_KEY_NAME"+PgDn"; break;
		case K_Meta_Insert: b = META_KEY_NAME"+Insert"; break;
		case K_Meta_Delete: b = META_KEY_NAME"+Delete"; break;
		case K_Meta_Home: b = META_KEY_NAME"+Home"; break;
		case K_Meta_End: b = META_KEY_NAME"+End"; break;

		case K_Control_Up: b = "Ctrl+Up"; break;
		case K_Control_Down: b = "Ctrl+Down"; break;
		case K_Control_Left: b = "Ctrl+Left"; break;
		case K_Control_Right: b = "Ctrl+Right"; break;
		case K_Control_PageUp: b = "Ctrl+PgUp"; break;
		case K_Control_PageDown: b = "Ctrl+PgDn"; break;
		case K_Control_Insert: b = "Ctrl+Insert"; break;
		case K_Control_Delete: b = "Ctrl+Delete"; break;
		case K_Control_Home: b = "Ctrl+Home"; break;
		case K_Control_End: b = "Ctrl+End"; break;
		case K_Control_Tab: b = "Ctrl+Tab"; break;

		case K_Shift_Left: b = "Shift+Left"; break;
		case K_Shift_Right: b = "Shift+Right"; break;
		case K_Shift_Up: b = "Shift+Up"; break;
		case K_Shift_Down: b = "Shift+Down"; break;
		case K_Shift_PageUp: b = "Shift+PgUp"; break;
		case K_Shift_PageDown: b = "Shift+PgDn"; break;
		case K_Shift_Insert: b = "Shift+Insert"; break;
		case K_Shift_Delete: b = "Shift+Delete"; break;
		case K_Shift_Home: b = "Shift+Home"; break;
		case K_Shift_End: b = "Shift+End"; break;
		case K_Shift_Tab: b = "Shift+Tab"; break;
		default: {
			char kk = (char)k;
			b = b2;
			if (!(k & ~0xff) &&
			(((kk>='a') && (kk<='z'))
			|| ((kk>='A') && (kk<='Z'))
			|| ((kk>='0') && (kk<='9'))
			|| ((strchr("[]{}()/\\'\"$%&?!.:,;-_=*+-#~<>|", kk) != NULL)))) {
				b2[0] = kk;
				b2[1] = 0;
			} else if ((k>=K_F1) && (k<=K_F12)) {
				sprintf(b2, "F%d", (int)k-(int)K_F1+1);
			} else if ((k>=K_Shift_F1) && (k<=K_Shift_F12)) {
				sprintf(b2, "Shift-F%d", (int)k-(int)K_Shift_F1+1);
			} else if ((k>=K_Control_F1) && (k<=K_Control_F12)) {
				sprintf(b2, "Ctrl-F%d", (int)k-(int)K_Control_F1+1);
			} else if ((k>=K_Control_Shift_F1) && (k<=K_Control_Shift_F12)) {
				sprintf(b2, "Ctrl-Shift-F%d", (int)k-(int)K_Control_Shift_F1+1);
			} else if ((k>=K_Meta_F1) && (k<=K_Meta_F12)) {
				sprintf(b2, META_KEY_NAME"-F%d", (int)k-(int)K_Meta_F1+1);
			} else if ((k>=K_Meta_1) && (k<=K_Meta_0)) {
				sprintf(b2, META_KEY_NAME"-%d", ((int)k-(int)K_Meta_1+1)%10);
			} else if ((k>=K_Control_A) && (k<=K_Control_Z)) {
				sprintf(b2, "Ctrl-%c", (int)k-(int)K_Control_A+'A');
			} else if ((k>=K_Meta_A) && (k<=K_Meta_Z)) {
				sprintf(b2, META_KEY_NAME"-%c", (int)k-(int)K_Meta_A+'A');
			} else {
				if ((unsigned int)k>255) {
					sprintf(b2, "key <%x>", (int)k);
				} else {
					sprintf(b2, "key <%x> '%c'", (int)k, (int)k);
				}
				r = false;
			}
		}			
	}
	if (bufsize > 0) {
		ht_strlcpy(buf, b, bufsize);
	}
	return r;
}
