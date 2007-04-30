/* 
 *	HT Editor
 *	htxeximg.h
 *
 *	Copyright (C) 2006 Sebastian Biallas (sb@biallas.net)
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

#ifndef __HTXEXIMG_H__
#define __HTXEXIMG_H__

#include "htanaly.h"
#include "htxex.h"
#include "formats.h"

extern format_viewer_if htxeximage_if;

/*
 *	CLASS ht_xex_aviewer
 */

class ht_xex_aviewer: public ht_aviewer {
public:
	ht_xex_shared_data *xex_shared;
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Analyser *Analyser, ht_xex_shared_data *xex_shared);
//	virtual bool func_handler(eval_scalar *result, char *name, eval_scalarlist *params);
	virtual void setAnalyser(Analyser *a);
};

#endif /* !__HTXEXIMG_H__ */
