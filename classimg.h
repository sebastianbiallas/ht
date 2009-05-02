/* 
 *	HT Editor
 *	classimg.h
 *
 *	Copyright (C) 2002 Stefan Weyergraf
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

#ifndef __CLASSIMG_H__
#define __CLASSIMG_H__

#include "class.h"
#include "formats.h"
#include "htanaly.h"

extern format_viewer_if htclassimage_if;

/*
 *	CLASS ht_class_aviewer
 */

class ht_class_aviewer: public ht_aviewer {
public:
	ht_class_shared_data *class_shared;
	File *file;
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Analyser *Analy, ht_class_shared_data *class_shared);
	virtual void setAnalyser(Analyser *a);
};

#endif
