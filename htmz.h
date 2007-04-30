/* 
 *	HT Editor
 *	htmz.h
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

#ifndef __HTMZ_H__
#define __HTMZ_H__

#include "formats.h"

#include "mzstruct.h"

#define DESC_MZ "mz - dos exe"
#define DESC_MZ_HEADER "mz/header"
#define DESC_MZ_REL "mz/relocations"
#define DESC_MZ_IMAGE "mz/image"

extern format_viewer_if htmz_if;

struct ht_mz_shared_data {
	IMAGE_MZ_HEADER header;
};

class ht_mz: public ht_format_group {
protected:
	bool loc_enum;
public:

			void	init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual   void loc_enum_start();
	virtual   bool loc_enum_next(ht_format_loc *loc);
};

#endif /* !__HTMZ_H__ */
