/* 
 *	HT Editor
 *	htiobox.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTIOBOX_H__
#define __HTIOBOX_H__

#include "htdialog.h"

/* buttonmask */
#define btmask_ok			1
#define btmask_cancel		2
#define btmask_yes			4
#define btmask_no			8
#define btmask_skip			16
#define btmask_all			32
#define btmask_none			64

int msgbox(int buttonmask, char *title, bool modal, int align, char *format, ...);
int msgboxrect(bounds *b, int buttonmask, char *title, bool modal, int align, bounds *b, char *format, ...);

#define errorbox(a...) msgbox(btmask_ok, "error", 0, align_center, a)
#define infobox(a...) msgbox(btmask_ok, "information", 0, align_center, a)
#define warnbox(a...) msgbox(btmask_ok, "warning", 0, align_center, a)
#define confirmbox(a...) msgbox(btmask_yes+btmask_no, "confirmation", 0, align_center, a)

#define errorbox_c(a...) msgbox(btmask_ok, "error", 0, align_custom, a)
#define infobox_c(a...) msgbox(btmask_ok, "information", 0, align_custom, a)
#define warnbox_c(a...) msgbox(btmask_ok, "warning", 0, align_custom, a)
#define confirmbox_c(a...) msgbox(btmask_yes+btmask_no, "confirmation", 0, align_custom, a)

#define errorbox_modal(a...) msgbox(btmask_ok, "error", 1, align_center, a)
#define infobox_modal(a...) msgbox(btmask_ok, "information", 1, align_center, a)
#define warnbox_modal(a...) msgbox(btmask_ok, "warning", 1, align_center, a)
#define confirmbox_modal(a...) msgbox(btmask_yes+btmask_no, "confirmation", 1, align_center, a)

int inputbox(char *title, char *Label, char *result, int limit, dword histid=0);
int inputboxrect(bounds *b, char *title, char *Label, char *result, int limit, dword histid=0);

void get_std_progress_indicator_metrics(bounds *b);

#endif /* __HTIOBOX_H__ */

