/* 
 *	HT Editor
 *	htcoffhd.h
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

#ifndef __HTCOFFHD_H__
#define __HTCOFFHD_H__

#include "httag.h"
#include "formats.h"

extern ht_mask_ptable coffheader[];
extern ht_mask_ptable coff32header[];
extern ht_mask_ptable coff_section[];

extern int_hash coff_machines[];
extern ht_tag_flags_s coff_characteristics[];
extern ht_tag_flags_s coff_section_characteristics[];

extern format_viewer_if htcoffheader_if;

#endif /* !__HTCOFFHD_H__ */
