/* 
 *	HT Editor
 *	htleimg.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTLEIMG_H__
#define __HTLEIMG_H__

#include "htanaly.h"
#include "htle.h"
#include "formats.h"

extern format_viewer_if htleimage_if;

/*
 *	CLASS ht_le_aviewer
 */

class ht_le_aviewer: public ht_aviewer {
public:
	ht_le_shared_data *le_shared;
	ht_streamfile *file;
	
		   void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, Analyser *Analyser, ht_le_shared_data *le_shared);
/* overwritten */
	virtual char *func(UINT i, bool execute);
	virtual void setAnalyser(Analyser *a);
};

#endif /* !__HTLEIMG_H__ */

