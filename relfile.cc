/* 
 *	HT Editor
 *	relfile.cc
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

#include <string.h>

#include "relfile.h"
#include "tools.h"

#define MAX_RELOC_TOKEN_LEN 8

/*
 *	CLASS ht_reloc_file
 */

void	ht_reloc_file::init(ht_streamfile *s, bool os)
{
	ht_layer_streamfile::init(s, os);
	relocs = new ht_stree();
	((ht_stree*)relocs)->init(compare_keys_uint_delinear);
}

void	ht_reloc_file::done()
{
	relocs->destroy();
	delete relocs;
	ht_layer_streamfile::done();
}

void ht_reloc_file::finalize()
{
	relocs->set_compare_keys(compare_keys_uint);
}

void	ht_reloc_file::insert_reloc(FILEOFS o, ht_data *reloc)
{
	relocs->insert(new ht_data_uint(o), reloc);
}

UINT	ht_reloc_file::read(void *buf, UINT size)
{
	FILEOFS o = tell();
	UINT ret = ht_layer_streamfile::read(buf, size), c = ret;
	ht_data_uint q;
/* FIXME: relocs wiil not work before ca. offset 8 */
	ht_data *r;
	ht_data_uint *k = &q;
	k->value = o-MAX_RELOC_TOKEN_LEN-1;
	while ((k = (ht_data_uint*)relocs->enum_next((ht_data**)&r, k))) {
		UINT s = (k->value < o) ? o - k->value : 0;
		UINT e = (k->value > o) ? k->value - o : 0;
		if (e >= c) break;
		byte b[MAX_RELOC_TOKEN_LEN];
		memset(b, 0, sizeof b);
		UINT mm = MIN(c - e, sizeof b - s);
		memmove(b+s, ((byte*)buf)+e, mm);
		reloc_apply(r, b);
		memmove(((byte*)buf)+e, b+s, mm);
	}
	return ret;
}

UINT	ht_reloc_file::write(void *buf, UINT size)
{
	FILEOFS o = tell();
	UINT c = size;
	ht_data_uint q;
/* FIXME: relocs wiil not work before ca. offset 8 */
	ht_data *r;
	ht_data_uint *k = &q;
	k->value = o-MAX_RELOC_TOKEN_LEN-1;
	while ((k = (ht_data_uint*)relocs->enum_next((ht_data**)&r, k))) {
		UINT s = (k->value < o) ? o - k->value : 0;
		UINT e = (k->value > o) ? k->value - o : 0;
		if (e >= c) break;
		byte b[MAX_RELOC_TOKEN_LEN];
		memset(b, 0, sizeof b);
		UINT mm = MIN(c - e, sizeof b - s);
		memmove(b+s, ((byte*)buf)+e, mm);
		reloc_unapply(r, b);
		memmove(((byte*)buf)+e, b+s, mm);
	}
	return ht_layer_streamfile::write(buf, size);
}

