/*
 *	HT Editor
 *	log.cc
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

#include "htapp.h"
#include "log.h"
#include "snprintf.h"

void ht_logf(int color, const char *format, ...)
{
	char buf[256];
	va_list arg;
	va_start(arg, format);
	ht_vsnprintf(buf, sizeof buf, format, arg);
	va_end(arg);
	loglines->log(color, buf);

	ht_window *w = ((ht_app*)app)->get_window_by_type(AWT_LOG);
	if (w) w->sendmsg(msg_log_changed);
	app->sendmsg(msg_draw);
}

