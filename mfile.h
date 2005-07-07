/* 
 *	HT Editor
 *	mfile.h
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

#ifndef __MFILE_H__
#define __MFILE_H__

#include "data.h"
#include "stream.h"

typedef FileOfs PAGEOFS;

/*
 *	ht_mod_page
 */
#define DEFAULT_MOD_PAGE_SIZE 8*1024	/* MUST BE 2^n */

class ht_mod_page: public Object {
public:
	uint size;
	byte *data;

			ht_mod_page(uint size);
			~ht_mod_page();
	/* new */
	virtual uint	read(PAGEOFS pofs, byte *buf, uint len);
	virtual uint	write(PAGEOFS pofs, const byte *buf, uint len);
};

/*
 *	ht_streamfile_modifier
 */
class ht_streamfile_modifier: public ht_layer_streamfile {
protected:
	ht_tree *mod_pages;

	bool modified;

	FileOfs offset;
	uint size;

	uint page_granularity;
	uint page_mask;

	bool active;
	/* new */
	virtual void	mod_pages_create();
	virtual void	mod_pages_destroy();

		   void	mod_pages_flush();
		   void	mod_pages_invd();
			 
	virtual ht_mod_page *mod_page_create(FileOfs offset);
	virtual void	mod_page_destroy(FileOfs offset);
	virtual ht_mod_page *mod_page_find(FileOfs offset);
	virtual void	mod_page_flush(FileOfs offset);

		   void	cleardirtybyte(FileOfs offset);
		   
		   bool	isdirty(FileOfs offset, uint range);

		   bool	isdirtybit(FileOfs offset, uint bit);
		   bool	isdirtybyte(FileOfs offset);
		   bool	readbyte(FileOfs offset, byte *b);
		   bool	writebyte(FileOfs offset, byte b);
public:
		   void	init(File *streamfile, int own_streamfile, uint page_granularity = DEFAULT_MOD_PAGE_SIZE);
	virtual void	done();
	/* overwritten */
	virtual int	extend(uint newsize);
	virtual uint	get_size();
	virtual uint	read(void *buf, uint size);
	virtual int	seek(FileOfs offset);
	virtual bool	set_access_mode(uint access_mode);
	virtual FileOfs tell();
	virtual int	truncate(uint newsize);
	virtual int	vcntl(uint cmd, va_list vargs);
	virtual uint	write(const void *buf, uint size);
};

#endif /* __MFILE_H__ */
