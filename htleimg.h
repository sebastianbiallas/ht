/* 
 *	HT Editor
 *	htleimg.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "stream.h"

#include "formats.h"

/*
 *	CLASS ht_le_page_file
 */

class ht_le_page_file: public ht_layer_streamfile {
protected:
	ht_le_pagemap *pagemap;
	dword pagemapsize;
	dword page_size;
	FILEOFS ofs;
/* new */
		   int map_ofs(dword ofs, FILEOFS *offset, dword *maxsize);
public:
		   void init(ht_streamfile *file, bool own_file, ht_le_pagemap *pagemap, dword pagemapsize, dword page_size);
/* overwritten */
	virtual bool isdirty(FILEOFS offset, UINT range);
	virtual UINT read(void *buf, dword size);
	virtual int seek(FILEOFS offset);
	virtual FILEOFS tell();
	virtual UINT write(const void *buf, UINT size);
};

extern format_viewer_if htleimage_if;

#endif /* !__HTLEIMG_H__ */

