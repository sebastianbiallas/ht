/* 
 *	HT Editor
 *	htxbeimg.h
 *
 *	Copyright (C) 2003 Stefan Esser
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

#ifndef __HTXBEIMG_H__
#define __HTXBEIMG_H__

#include "htanaly.h"
#include "htxbe.h"
#include "formats.h"

extern format_viewer_if htxbeimage_if;

/*
 *	ht_xbe_aviewer
 */
class ht_xbe_aviewer: public ht_aviewer {
public:
	ht_xbe_shared_data *xbe_shared;

		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Analyser *Analyser, ht_xbe_shared_data *xbe_shared);
	virtual bool func_handler(eval_scalar *result, char *name, eval_scalarlist *params);
	virtual void setAnalyser(Analyser *a);
};

#endif /* !__HTXBEIMG_H__ */
