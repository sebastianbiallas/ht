/* 
 *	HT Editor
 *   clipboard.cc - DJGPP-specific (windows-)clipboard functions
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

#include <dpmi.h>
#include <sys/movedata.h>

#include "types.h"

static bool open_clipboard()
{
	__dpmi_regs r;
	r.x.ax = 0x1700;   // version
	__dpmi_int(0x2f, &r);
	if (r.x.ax == 0x1700) return false;
	r.x.ax = 0x1701;  // open
	__dpmi_int(0x2f, &r);     
	return (r.x.ax != 0);
}

static void close_clipboard()
{
	__dpmi_regs r;
	r.x.ax = 0x1708;
	__dpmi_int(0x2f, &r);
}

bool sys_write_data_to_native_clipboard(const void *data, int size)
{
	if (size > 0xffff) return false;
	if (!open_clipboard()) return false;
	int sel;
	uint16 seg = __dpmi_allocate_dos_memory((size+15)>>4, &sel);
	if (seg == 0xffff) {
		close_clipboard();
		return false;
	}
	dosmemput(data, size, seg*16);
	
	__dpmi_regs r;
	r.x.ax = 0x1703;
	
	r.x.dx = 0x01; // text
	r.x.es = seg;
	r.x.bx = 0;
	r.x.si = size >> 16;
	r.x.cx = size & 0xffff;
	
	__dpmi_int(0x2f, &r);
	__dpmi_free_dos_memory(sel);
	close_clipboard();
	return (r.x.ax != 0);
}

int sys_get_native_clipboard_data_size()
{
	return 10000;
	if (!open_clipboard()) return 0;
	__dpmi_regs r;
	r.x.ax = 0x1704;
	r.x.dx = 0x07; // text
	__dpmi_int(0x2f, &r);
	close_clipboard();
	return ((uint32)r.x.dx)<<16+r.x.ax;
}

bool sys_read_data_from_native_clipboard(void *data, int max_size)
{
	int dz = sys_get_native_clipboard_data_size();
	if (!open_clipboard()) return false;
	if (!dz) {
		close_clipboard();
		return false;
	}
	int sel;
	uint16 seg = __dpmi_allocate_dos_memory((dz+15)>>4, &sel);
	if (seg == 0xffff) {
		close_clipboard();
		return false;
	}
	
	__dpmi_regs r;
	r.x.ax = 0x1705;
	r.x.dx = 0x1;
	r.x.es = seg;
	r.x.bx = 0;
	__dpmi_int(0x2f, &r);
	if (r.x.ax) {
		dosmemget(seg*16, MIN(max_size, dz), data);
	}
	__dpmi_free_dos_memory(sel);
	close_clipboard();
	return (r.x.ax != 0);
}

