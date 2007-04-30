/*
 *	HT Editor
 *	htiobox.cc
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

#include "atom.h"
#include "htctrl.h"
#include "htdialog.h"
#include "hthist.h"
#include "htiobox.h"
#include "snprintf.h"
#include "tools.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int imsgbox(Bounds *b, int buttonmask, const char *title, bool modal, statictext_align align, char *buf)
{
	ht_dialog *dialog=new ht_dialog();
	dialog->init(b, title, FS_KILLER | FS_TITLE | (modal ? 0 : FS_MOVE | FS_RESIZE));

	Bounds c;

	c.x=1;
	c.y=0;
	c.w=b->w-4;
	c.h=b->h-4;
	ht_statictext *text=new ht_statictext();
	text->init(&c, buf, align);
	text->growmode = MK_GM(GMH_FIT, GMV_FIT);

	dialog->insert(text);

	int buttons=0;
	if (buttonmask & btmask_ok) buttons++; else
	if (buttonmask & btmask_yes) buttons++;
	if (buttonmask & btmask_no) buttons++; else
	if (buttonmask & btmask_skip) buttons++;
	if (buttonmask & btmask_cancel) buttons++;
	if (buttonmask & btmask_all) buttons++;
	if (buttonmask & btmask_none) buttons++;

	int pos=0;
	if (buttonmask & btmask_ok) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "O~K", button_ok);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	} else if (buttonmask & btmask_yes) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "~Yes", button_yes);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	}
	if (buttonmask & btmask_no) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "~No", button_no);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	} else if (buttonmask & btmask_skip) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "~Skip", button_skip);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	}
	if (buttonmask & btmask_all) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "~All", button_all);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	}
	if (buttonmask & btmask_none) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "~None", button_none);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	}
	if (buttonmask & btmask_cancel) {
		c.x=b->w/2-5*(buttons-2*pos);
		c.y=b->h-4;
		c.w=9;
		c.h=2;
		ht_button *button=new ht_button();
		button->init(&c, "~Cancel", button_cancel);
		button->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		dialog->insert(button);
		pos++;
	}
	if (!buttons) {
		Bounds x;
		text->getbounds(&x);
		x.h+=2;
		text->setbounds(&x);
	}

	int r=dialog->run(modal);
	dialog->done();
	delete dialog;
	return r;
}

int msgbox(int buttonmask, const char *title, bool modal, statictext_align align, const char *format, ...)
{
	char buf[1024];
	va_list arg;
	va_start(arg, format);
	ht_vsnprintf(buf, 1024, format, arg);
	va_end(arg);

	char *s=buf;
	int ns=1;
	while ((s=strchr(s, '\n'))) {
		s++;
		ns++;
	}
	int strl=strlen(buf);

	Bounds b;
	app->getbounds(&b);
	b.w=55;
	b.h=MAX(strl/(b.w-4), ns)+6;
	b.x=(screen->w - b.w)/2;
	b.y=(screen->h - b.h)/2;
	return imsgbox(&b, buttonmask, title, modal, align, buf);
}

int msgboxrect(Bounds *b, int buttonmask, const char *title, bool modal, statictext_align align, const char *format, ...)
{
	char buf[1024];
	va_list arg;
	va_start(arg, format);
	ht_vsnprintf(buf, 1024, format, arg);
	va_end(arg);

	return imsgbox(b, buttonmask, title, modal, align, buf);
}

bool inputbox(const char *title, const char *label, char *result, int limit, uint32 histid)
{
	Bounds b;
	app->getbounds(&b);
	b.x = (b.w - 60) / 2,
	b.y = (b.h - 8) / 2;
	b.w = 60;
	b.h = 8;
	return inputboxrect(&b, title, label, result, limit, histid);
}

bool inputboxrect(Bounds *b, const char *title, const char *label, char *result, int limit, uint32 histid)
{
	ht_dialog *dialog=new ht_dialog();
	dialog->init(b, title, FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);

	ht_strinputfield *input;

	Bounds  b2;
	b2.x = 3 + strlen(label);
	b2.y = 1;
	b2.w = b->w - 3 - b2.x;
	b2.h = 1;

	List *hist = NULL;
	if (histid) hist = (List*)getAtomValue(histid);
	input = new ht_strinputfield();
	input->init(&b2, limit, hist);
	ht_inputfield_data d;
	d.text = (byte *)result;
	d.textlen = strlen((char*)d.text);
	input->databuf_set(&d, sizeof d);
	dialog->insert(input);

	if (label) {
		b2.x = 1;
		b2.y = 1;
		b2.w = 3 + strlen(label) - b2.x;
		b2.h = 1;

		ht_label *lab = new ht_label();
		lab->init(&b2, label, input);
		dialog->insert(lab);
	}

	b2.x = b->w - 25;
	b2.y = b->h - 5;
	b2.w = 10;
	b2.h = 2;

	ht_button *bok = new ht_button();
	bok->init(&b2, "O~k", button_ok);
	dialog->insert(bok);

	b2.x += 12;

	ht_button *bcancel = new ht_button();
	bcancel->init(&b2, "~Cancel", button_cancel);
	dialog->insert(bcancel);

	if (dialog->run(0)) {
		int dsize = input->datasize();
		ht_inputfield_data *data = ht_malloc(dsize);
		ViewDataBuf vdb(input, data, dsize);
		bin2str(result, data->text, data->textlen);
		free(data);
		if (hist) insert_history_entry(hist, result, 0);

		dialog->done();
		delete dialog;
		return true;
	}
	dialog->done();
	delete dialog;
	return false;
}

void get_std_progress_indicator_metrics(Bounds *b)
{
	app->getbounds(b);
	b->w=b->w*2/3;
	b->h=6;
	b->x=(screen->w - b->w)/2;
	b->y=(screen->h - b->h)/2;
}

