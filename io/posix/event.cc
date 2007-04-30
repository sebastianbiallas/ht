/* 
 *	HT Editor
 *	event.cc
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

#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io/display.h"
#include "io/event.h"

struct fd_listener {
	int fd;
	int fd_flags;
};

#define	MAX_FD_LISTENERS	32
static fd_listener fd_listener[MAX_FD_LISTENERS];
static int fd_listeners = 0;
static fd_set grfds, gwfds, gxfds;
static int g_highest_fd = 0;
static bool gTranslateKeys = true;

bool sys_next_event(sys_event_t &event, int wait_usec)
{
	if (sys_get_winch_flag()) {
		sys_set_winch_flag(false);
		if (sys_get_screen_size(event.screen_resize.w, event.screen_resize.h)) {
			event.type = SYSEV_SCREEN_RESIZE;
			return true;
		}
	}
	fd_set rfds = grfds, wfds = gwfds, xfds = gxfds;
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = wait_usec;
	int e = select(g_highest_fd+1, &rfds, &wfds, &xfds, &t);
	if (e > 0) {
		if (FD_ISSET(0, &rfds)) {
			if (gTranslateKeys) {
				return keyb_getevent(event);
			} else {
				event.type = SYSEV_FD_EVENT;
				event.fd_event.fd = 0;
				event.fd_event.fd_flags = FD_FLAG_READABLE;
				return true;
			}
		} else {
			for (int i=0; i<fd_listeners; i++) {
				int fl = 0;
				fl |= ((fd_listener[i].fd_flags & FD_FLAG_READABLE) &&
				FD_ISSET(fd_listener[i].fd, &rfds)) ? FD_FLAG_READABLE : 0;
				fl |= ((fd_listener[i].fd_flags & FD_FLAG_WRITABLE) &&
				FD_ISSET(fd_listener[i].fd, &wfds)) ? FD_FLAG_WRITABLE : 0;
				fl |= ((fd_listener[i].fd_flags & FD_FLAG_EXCEPTION) &&
				FD_ISSET(fd_listener[i].fd, &xfds)) ? FD_FLAG_EXCEPTION : 0;
				if (fl) {
					event.type = SYSEV_FD_EVENT;
					event.fd_event.fd = fd_listener[i].fd;
					event.fd_event.fd_flags = fl;
					return true;
				}
			}
		}
		// should not happen !
		fprintf(stderr, "argh: %s:%d", __FILE__, __LINE__);
		exit(1);
	}
	return false;
}

static void updateFDSets()
{
	FD_ZERO(&grfds);
	FD_ZERO(&gwfds);
	FD_ZERO(&gxfds);
	g_highest_fd = 0;
	for (int i=0; i<fd_listeners; i++) {
		if (fd_listener[i].fd > g_highest_fd) g_highest_fd = fd_listener[i].fd;
		if (fd_listener[i].fd_flags & FD_FLAG_READABLE)
			FD_SET(fd_listener[i].fd, &grfds);
		if (fd_listener[i].fd_flags & FD_FLAG_WRITABLE)
			FD_SET(fd_listener[i].fd, &gwfds);
		if (fd_listener[i].fd_flags & FD_FLAG_EXCEPTION)
			FD_SET(fd_listener[i].fd, &gxfds);
	}
	// always listen on stdin
	FD_SET(0, &grfds);
}

bool sys_listen_fd_event(int fd, int fd_flags)
{
	if (fd_listeners == MAX_FD_LISTENERS) return false;
	for (int i=0; i<fd_listeners; i++) {
		// already listening to fd ?
		if (fd_listener[i].fd == fd) return false;
	}
	fd_listener[fd_listeners].fd = fd;
	fd_listener[fd_listeners].fd_flags = fd_flags;
	fd_listeners++;
	updateFDSets();
	return true;
}

bool sys_unlisten_fd_event(int fd)
{
	for (int i=0; i<fd_listeners; i++) {
		if (fd_listener[i].fd == fd) {
			memmove(fd_listener+i, fd_listener+i+1, (fd_listeners-i-1)*sizeof fd_listener[0]);
			fd_listeners--;
			updateFDSets();
			return true;
		}
	}
	return false;
}

void sys_set_key_translation(bool translate_keys)
{
	gTranslateKeys = translate_keys;
}

bool initSysEvent()
{
	updateFDSets();
	return true;
}

void doneSysEvent()
{
}
