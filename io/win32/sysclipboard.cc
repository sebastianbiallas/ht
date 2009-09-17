/* 
 *	HT Editor
 *	clipboard.cc - Win32-specific (windows-)clipboard functions
 *
 *	Copyright (C) 1999-2003 Sebastian Biallas (sb@biallas.net)
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
bool sys_native_clipboard_write(const void *data, int size)
{
	// FIXME:
	if (!OpenClipboard(NULL)) return false;
        HGLOBAL hdata;
        hdata = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, size);
        if (hdata) {
        	void *ptr = GlobalLock(hdata);
        	memcpy(ptr, data, size);
        	GlobalUnlock(hdata);
	        if (SetClipboardData(CF_OEMTEXT, hdata)) {
			CloseClipboard();
			return true;
		}
	}
	CloseClipboard();
       	return false;
}

int sys_native_clipboard_get_size()
{
	if (!OpenClipboard(NULL)) return false;
	HANDLE h = GetClipboardData(CF_OEMTEXT);
	int len = 0;
	if (h) {
		void *mem = GlobalLock(h);
		len = strlen((char*)mem);
		GlobalUnlock(h);		
	}
	CloseClipboard();
	return len;
}

#include "snprintf.h"
int sys_native_clipboard_read(void *data, int max_size)
{
//	ht_printf("sys_native_clipboard_read(%d)\n", max_size);
	if (!OpenClipboard(NULL)) return false;
	HANDLE hdata = GetClipboardData(CF_OEMTEXT);
	if (!hdata) {
		CloseClipboard();
		return 0;
	}
	int size = GlobalSize(hdata);
	void *ptr = GlobalLock(hdata);
	int r = MIN(size, max_size);
	memcpy(data, ptr, r);
	GlobalUnlock(hdata);
	CloseClipboard();
//	ht_printf("=%d\n", r);
	return r;
}

const char *sys_native_clipboard_name()
{
	return "Windows Clipboard";
}

