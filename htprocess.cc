/*
 *	HT Editor
 *	process.cc
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

#include "htprocess.h"
#include "htctrl.h"
#include "htdialog.h"
#include "htiobox.h"
#include "keyb.h"

bool execute_process(process_func pp, Object *context)
{
	Bounds b;
	get_std_progress_indicator_metrics(&b);
	
	ht_progress_indicator pi;
	pi.init(&b, "ESC to cancel");

	bool cancelled=false;
	bool p=true;
	
	while (p) {
		p = pp(context, pi.text);
		if (keyb_keypressed()) {
			if (keyb_getkey() == K_Escape) {
				cancelled = true;
				break;
			}
		}
		pi.sendmsg(msg_draw, 0);
		screen->show();
	}

	pi.done();

	return !cancelled;
}

void execute_process_bg(process_func pp, Object *context)
{
	/* FIXME: nyi */
}

