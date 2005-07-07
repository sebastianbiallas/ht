/*
 *	HT Editor
 *	mfile.cc
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mfile.h"
#include "tools.h"

/*
 *	CLASS ht_mod_page
 */

ht_mod_page::ht_mod_page(uint s)
{
	size = s;
	data = (byte*)malloc(size);
}

ht_mod_page::~ht_mod_page()
{
	free(data);
}

uint ht_mod_page::read(PAGEOFS pofs, byte *buf, uint len)
{
	uint s = len;
	if (pofs+s > size) s = size-pofs;
	memmove(buf, data+pofs, s);
	return s;
}

uint ht_mod_page::write(PAGEOFS pofs, const byte *buf, uint len)
{
	uint s = len;
	if (pofs+s > size) s = size-pofs;
	memmove(data+pofs, buf, s);
	return s;
}

/*
 *	CLASS ht_streamfile_modifier
 */

int compare_keys_file_delinear(ht_data *key_a, ht_data *key_b)
{
	FileOfs a = (FILEOFS)delinearize((uint32)((ht_data_uint*)key_a)->value);
	FileOfs b = (FILEOFS)delinearize((uint32)((ht_data_uint*)key_b)->value);
	return a-b;
}

void	ht_streamfile_modifier::init(ht_streamfile *s, int own_s, uint pgran)
{
	ht_layer_streamfile::init(s, own_s);
	
	page_granularity = pgran;
	page_mask = ~(page_granularity-1);

	offset = 0;
	active = get_access_mode() & FAM_WRITE;

	modified = false;
	mod_pages_create();
}

void	ht_streamfile_modifier::done()
{
	mod_pages_destroy();

	ht_layer_streamfile::done();
}

bool	ht_streamfile_modifier::isdirty(FILEOFS offset, uint range)
{
	if (range & IS_DIRTY_SINGLEBIT) {
		return isdirtybit(offset, range & 7);
	} else {
		while (range--) if (isdirtybyte(offset++)) return true;
	}
	return false;
}

bool	ht_streamfile_modifier::isdirtybit(FILEOFS offset, uint bit)
{
	ht_mod_page *m = mod_page_find(offset);
	if (m) {
		byte b1, b2;
		ht_layer_streamfile::seek(offset);
		if (!ht_layer_streamfile::read(&b1, 1)) return true;
		if (!m->read(offset % page_granularity, &b2, 1)) return true;
		return (((b1 >> bit) &1) != ((b2 >> bit) &1));
	}
	return false;
}

bool	ht_streamfile_modifier::isdirtybyte(FILEOFS offset)
{
	ht_mod_page *m = mod_page_find(offset);
	if (m) {
		byte b1, b2;
		ht_layer_streamfile::seek(offset);
		if (!ht_layer_streamfile::read(&b1, 1)) return true;
		if (!m->read(offset % page_granularity, &b2, 1)) return true;
		return (b1 != b2);
	}
	return false;
}

void	ht_streamfile_modifier::cleardirtybyte(FILEOFS offset)
{
	ht_mod_page *m = mod_page_find(offset);
	if (m) {
		byte b1;
		ht_layer_streamfile::seek(offset);
		if (ht_layer_streamfile::read(&b1, 1)) {
			m->write(offset % page_granularity, &b1, 1);
		}
	}
}

void ht_streamfile_modifier::mod_pages_create()
{
#if 1
	ht_dtree *m=new ht_dtree();
	m->init(compare_keys_file_delinear, 250, 250);
#else
	ht_stree *m=new ht_stree();
	m->init(compare_keys_file_delinear);
#endif
	mod_pages=m;
	size = ht_layer_streamfile::get_size();
}

void ht_streamfile_modifier::mod_pages_destroy()
{
	mod_pages->destroy();
	delete mod_pages;
	modified = false;
}

void ht_streamfile_modifier::mod_pages_flush()
{
	ht_data_uint *key=NULL;
	ht_data *value;

	while ((key=(ht_data_uint*)mod_pages->enum_next(&value, key))) {
		mod_page_flush(key->value);
	}

	uint lsize = ht_layer_streamfile::get_size();
	if (size > lsize) {
		ht_layer_streamfile::extend(size);
	} else if (size < lsize) {
		ht_layer_streamfile::truncate(size);
	}

	modified = false;
}

void ht_streamfile_modifier::mod_pages_invd()
{
	mod_pages_destroy();
	mod_pages_create();
}

ht_mod_page *ht_streamfile_modifier::mod_page_create(FILEOFS offset)
{
	offset &= page_mask;
	if (offset > size) return NULL;
	
	uint s = size - offset;
	if (s > page_granularity) s = page_granularity;
	
	ht_mod_page *m = new ht_mod_page(page_granularity);
	uint lsize = ht_layer_streamfile::get_size();
	uint rsize = s;
	if (offset > lsize) rsize = 0; else
		if (rsize > lsize - offset) rsize = lsize - offset;
	ht_layer_streamfile::seek(offset);
	ht_layer_streamfile::read(m->data, rsize);
	memset(m->data+rsize, 0, page_granularity-rsize);
	mod_pages->insert(new ht_data_uint(offset), m);
	return m;
}

void ht_streamfile_modifier::mod_page_destroy(FILEOFS offset)
{
	ht_data_uint o(offset);
	mod_pages->del(&o);
}

ht_mod_page *ht_streamfile_modifier::mod_page_find(FILEOFS offset)
{
	ht_data_uint o(offset & page_mask);
	return (ht_mod_page*)mod_pages->get(&o);
}

void ht_streamfile_modifier::mod_page_flush(FILEOFS offset)
{
	ht_mod_page *m = mod_page_find(offset);
	uint s = m->size;
	if (offset+s > size) s = size-offset;
	ht_layer_streamfile::seek(offset);
	ht_layer_streamfile::write(m->data, s);
}

int ht_streamfile_modifier::extend(uint newsize)
{
	// must be opened writable to extend
	if (!active) return EIO;
	uint osize = size;

	if (size != newsize) modified = true;
	size = newsize;
	for (FILEOFS o = osize & page_mask; o < size; o += page_granularity) {
		if (!mod_page_find(o)) mod_page_create(o);
	}
/**/
	return 0;
//	return ht_layer_streamfile::extend(newsize);
}

uint ht_streamfile_modifier::get_size()
{
	if (!active) return ht_layer_streamfile::get_size();
	return size;
}

uint ht_streamfile_modifier::read(void *buf, uint s)
{
	if (!active) return ht_layer_streamfile::read(buf, s);
	uint c = 0;
	byte *b = (byte*)buf;
	FileOfs o = tell();
#if 0
	while (s--) {
		if (o+c >= size) break;
		if (!readbyte(o+c, &b[c])) {
			set_error(EIO);
			break;
		}
		c++;
	}
#else
	if (s >= 8) {
		ht_mod_page *m;
		while (s) {
			m = mod_page_find(o);
			uint lc = page_granularity - (o & (~page_mask));
			if (lc > s) lc = s;
			if (o + lc > size) lc = size - o;
			if (!lc) break;

			uint k;
			if (m) {
				k = m->read(o & (~page_mask), b, lc);
			} else {
				ht_layer_streamfile::seek(o);
				k = ht_layer_streamfile::read(b, lc);
			}
			if (!k) break;
			s -= k;
			o += k;
			c += k;
			b += k;
		}
	} else {
		while (s--) {
			if (o+c >= size) break;
			if (!readbyte(o+c, &b[c])) {
				set_error(EIO);
				break;
			}
			c++;
		}
	}
#endif
	offset+=c;
	return c;
}

bool ht_streamfile_modifier::readbyte(FILEOFS offset, byte *b)
{
	ht_mod_page *m = mod_page_find(offset);
	if (m) {
		return (m->read(offset % page_granularity, b, 1)==1);
	}
	ht_layer_streamfile::seek(offset);
	return (ht_layer_streamfile::read(b, 1)==1);
}

int ht_streamfile_modifier::seek(FILEOFS o)
{
	if (!active) return ht_layer_streamfile::seek(o);
	offset = o;
	if (o >= size) return EINVAL;
	return 0;
}

bool	ht_streamfile_modifier::set_access_mode(uint access_mode)
{
	bool b=ht_layer_streamfile::set_access_mode(access_mode);
	if (get_access_mode() & FAM_WRITE) {
		active = true;
	} else {
		if (active) mod_pages_invd();
		active = false;
	}
	return b;
}

FileOfs ht_streamfile_modifier::tell()
{
	if (!active) return ht_layer_streamfile::tell();
	return offset;
}

int ht_streamfile_modifier::truncate(uint newsize)
{
	// must be opened writable to truncate
	if (!active) return EIO;
	for (FILEOFS o = (newsize+page_granularity-1) & page_mask; o < size;
	o += page_granularity) {
		mod_page_destroy(o);
	}
	if (size != newsize) modified = true;
	size = newsize;
/**/
	return 0;
}

int ht_streamfile_modifier::vcntl(uint cmd, va_list vargs)
{
	if (active) switch (cmd) {
		case FCNTL_MODS_INVD:
			mod_pages_invd();
			return 0;
		case FCNTL_MODS_FLUSH:
			mod_pages_flush();
			return 0;
		case FCNTL_MODS_CLEAR_DIRTY_RANGE: {
			FileOfs o=va_arg(vargs, FILEOFS);
			uint s=va_arg(vargs, UINT);
			uint i=0;
			while (s--) {
				cleardirtybyte(o+i);
				i++;
			}
			return 0;
		}
		case FCNTL_MODS_IS_DIRTY: {
			FileOfs o=va_arg(vargs, FILEOFS);
			uint s=va_arg(vargs, UINT);
			bool *b=va_arg(vargs, bool*);
			if ((o==0) && (s==size)) {
				*b = modified;
			} else {
				*b = isdirty(o, s);
			}
			return 0;
		}
	}
	return ht_layer_streamfile::vcntl(cmd, vargs);
}

uint ht_streamfile_modifier::write(const void *buf, uint s)
{
	if (!active) return ht_layer_streamfile::write(buf, s);
	FileOfs o = tell();
	byte *b = (byte*)buf;
	uint c = 0;

#if 0
	while (s--) {
		if (o+c >= size) break;
		if (!writebyte(o+c, b[c])) {
			set_error(EIO);
			break;
		}
		c++;
	}
#else
	if (s >= 8) {
		ht_mod_page *m;
		while (s) {
			m = mod_page_find(o);
			if (!m) m = mod_page_create(o);
			uint lc = page_granularity - (o & (~page_mask));
			if (lc > s) lc = s;

			uint k;
			// FIXME: not the right thing
			modified = true;
			k = m->write(o & (~page_mask), b, lc);
			if (!k) break;
			s -= k;
			o += k;
			c += k;
			b += k;
		}
	} else {
		while (s--) {
			if (o+c >= size) break;
			if (!writebyte(o+c, b[c])) {
				set_error(EIO);
				break;
			}
			c++;
		}
	}
#endif
	offset+=c;
	return c;
}

bool ht_streamfile_modifier::writebyte(FILEOFS offset, byte b)
{
	ht_mod_page *m = mod_page_find(offset);
	if (!m) m = mod_page_create(offset);
	byte t = b;
	m->read(offset % page_granularity, &t, 1);
	modified |= (t!=b);
	return (m->write(offset % page_granularity, &b, 1)==1);
}

