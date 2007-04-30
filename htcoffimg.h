/* 
 *	HT Editor
 *	htcoffimg.h
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

#ifndef __HTCOFFIMG_H__
#define __HTCOFFIMG_H__

#include "formats.h"
#include "htanaly.h"
#include "htcoff.h"

extern format_viewer_if htcoffimage_if;

/*
 *	CLASS ht_coff_aviewer
 */

class ht_coff_aviewer: public ht_aviewer {
public:
	ht_coff_shared_data *coff_shared;
	File *file;
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Analyser *analyser, ht_coff_shared_data *coff_shared);
	virtual void setAnalyser(Analyser *a);
};
#endif /* !__HTCOFFIMG_H__ */
