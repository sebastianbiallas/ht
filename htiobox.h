/* 
 *	HT Editor
 *	htiobox.h
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

int msgbox(int buttonmask, const char *title, bool modal, statictext_align align, const char *format, ...);
int msgboxrect(Bounds *b, int buttonmask, const char *title, bool modal, statictext_align align, const char *format, ...);

#define errorbox(a...) msgbox(btmask_ok, "error", false, align_center, a)
#define infobox(a...) msgbox(btmask_ok, "information", false, align_center, a)
#define warnbox(a...) msgbox(btmask_ok, "warning", false, align_center, a)
#define confirmbox(a...) msgbox(btmask_yes+btmask_no, "confirmation", false, align_center, a)

#define errorbox_c(a...) msgbox(btmask_ok, "error", false, align_custom, a)
#define infobox_c(a...) msgbox(btmask_ok, "information", false, align_custom, a)
#define warnbox_c(a...) msgbox(btmask_ok, "warning", false, align_custom, a)
#define confirmbox_c(a...) msgbox(btmask_yes+btmask_no, "confirmation", false, align_custom, a)

#define errorbox_modal(a...) msgbox(btmask_ok, "error", true, align_center, a)
#define infobox_modal(a...) msgbox(btmask_ok, "information", true, align_center, a)
#define warnbox_modal(a...) msgbox(btmask_ok, "warning", true, align_center, a)
#define confirmbox_modal(a...) msgbox(btmask_yes+btmask_no, "confirmation", true, align_center, a)

bool inputbox(const char *title, const char *label, char *result, int limit, uint32 histid = 0);
bool inputboxrect(Bounds *b, const char *title, const char *label, char *result, int limit, uint32 histid = 0);

void get_std_progress_indicator_metrics(Bounds *b);

#endif /* __HTIOBOX_H__ */

