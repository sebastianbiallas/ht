/* 
 *	HT Editor
 *	relfile.h
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

#ifndef __RELFILE_H__
#define __RELFILE_H__

#include "data.h"
#include "stream.h"

/*
 *	CLASS ht_reloc_file
 */

class ht_reloc_file: public FileLayer {
protected:
	Container *relocs;
	bool enabled;

/* new */
	virtual void	reloc_apply(Object *reloc, byte *data) = 0;
	virtual bool	reloc_unapply(Object *reloc, byte *data) = 0;
public:
			ht_reloc_file(File *File, bool own_streamfile);
	virtual		~ht_reloc_file();
/* overwritten */
		   void	finalize();
		   void	insert_reloc(FileOfs o, Object *reloc);
	virtual uint	read(void *buf, uint size);
	virtual int	vcntl(uint cmd, va_list vargs);
	virtual uint	write(const void *buf, uint size);
};

#endif /* __RELFILE_H__ */

