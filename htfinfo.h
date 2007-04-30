/* 
 *	HT Editor
 *	htfinfo.h
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

#ifndef __HTFINFO_H__
#define __HTFINFO_H__

#include "formats.h"
#include "htdialog.h"

#define FINFO_DESC "file info"

extern format_viewer_if htfinfo_if;

/*
 *   ht_finfo_text
 */
 
class ht_finfo_text: public ht_statictext {
protected:
	File *file;
	const char *olddesc;
	
public:
		void	init(Bounds *b, File *file);
	virtual	void	done();
/* overwritten */
	virtual	int     gettext(char *text, int max_len);
};

#endif /* __HTFINFO_H__ */

