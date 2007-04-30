/*
 *	HT Editor
 *	blockop.h
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

#ifndef __BLOCKOP_H__
#define __BLOCKOP_H__

#include "htdialog.h"
#include "htformat.h"

/*
 *	CLASS ht_blockop_dialog
 */

class ht_blockop_dialog: public ht_dialog {
protected:
	ht_strinputfield *start;
	ht_strinputfield *end;
	ht_listpopup *mode;
	ht_strinputfield *action;
public:
		void init(Bounds *b, FileOfs pstart, FileOfs pend, List *history = NULL);
	virtual	void done();
};

void blockop_dialog(ht_format_viewer *format, FileOfs pstart, FileOfs pend);

#endif /* __BLOCKOP_H__ */

