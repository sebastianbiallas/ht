/* 
 *	HT Editor
 *	relfile.h
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

#ifndef __RELFILE_H__
#define __RELFILE_H__

#include "htdata.h"
#include "stream.h"

/*
 *	CLASS ht_reloc_file
 */

class ht_reloc_file: public ht_layer_streamfile {
protected:
	ht_tree *relocs;

/* new */
	virtual void	reloc_apply(ht_data *reloc, byte *data) = 0;
	virtual void	reloc_unapply(ht_data *reloc, byte *data) = 0;
public:
		   void	init(ht_streamfile *streamfile, bool own_streamfile);
	virtual void	done();
/* overwritten */
		   void	finalize();
		   void	insert_reloc(FILEOFS o, ht_data *reloc);
	virtual UINT	read(void *buf, UINT size);
	virtual UINT	write(void *buf, UINT size);
};

#endif /* __RELFILE_H__ */

