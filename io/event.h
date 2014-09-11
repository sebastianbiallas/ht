/*
 *	HT Editor
 *	event.h
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

#ifndef __EVENT_H__
#define __EVENT_H__

#include "keyb.h"

enum sys_event_type {
	SYSEV_NONE,
	SYSEV_KEYPRESSED,
	SYSEV_MOUSE_EVENT,
	SYSEV_SCREEN_RESIZE,
	SYSEV_FD_EVENT
};

#define FD_FLAG_READABLE	1
#define FD_FLAG_WRITABLE	2
#define FD_FLAG_EXCEPTION	4

struct fd_event_t {
	int fd;
	int fd_flags;
};

struct screen_resize_t {
	int w, h;
};

enum mouse_button_event_t {
	MBE_PRESSED,
	MBE_RELEASED
};

typedef int mouse_button_mask_t;
enum mouse_button_t {
	MBM_LEFT = 1,
	MBM_MIDDLE = 2,
	MBM_RIGHT = 4,
	MBM_BUTTON4 = 8,
	MBM_BUTTON5 = 16,
	MBM_BUTTON6 = 32
};

struct mouse_event_t {
	mouse_button_event_t button_event;
	mouse_button_mask_t button_mask;
	int x, y;
};

struct sys_event_t {
	sys_event_type type;
	union {
		ht_key key;
		screen_resize_t screen_resize;
		fd_event_t fd_event;
		mouse_event_t mouse_event;
	};
};

/* system-dependant (implementation in $MYSYSTEM/ *.cc) */
bool	sys_next_event(sys_event_t &event, int wait_usec);
bool	sys_listen_fd_event(int fd, int fd_flags);
bool	sys_unlisten_fd_event(int fd);
// if set, sys_next_event() translates events on stdin (fd 0) to SYSEV_KEYPRESSED
// if not set, sys_next_event() translates events on stdin (fd 0) to SYSEV_FD_EVENT
void	sys_set_key_translation(bool translate_keys);

#endif /* __EVENT_H__ */
