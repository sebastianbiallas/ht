/*
 *	HT Editor
 *	mfile.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

ht_mod_page::ht_mod_page(UINT s)
{
	size = s;
	data = (byte*)malloc(size);
}

ht_mod_page::~ht_mod_page()
{
	free(data);
}

UINT ht_mod_page::read(PAGEOFS pofs, byte *buf, UINT len)
{
	UINT s = len;
	if (pofs+s > size) s = size-pofs;
	memmove(buf, data+pofs, s);
	return s;
}

UINT ht_mod_page::write(PAGEOFS pofs, const byte *buf, UINT len)
{
	UINT s = len;
	if (pofs+s > size) s = size-pofs;
	memmove(data+pofs, buf, s);
	return s;
}

/*
 *	CLASS ht_streamfile_modifier
 */

int compare_keys_file_delinear(ht_data *key_a, ht_data *key_b)
{
	FILEOFS a = (FILEOFS)delinearize((dword)((ht_data_uint*)key_a)->value);
	FILEOFS b = (FILEOFS)delinearize((dword)((ht_data_uint*)key_b)->value);
	return a-b;
}

void	ht_streamfile_modifier::init(ht_streamfile *s, int own_s, UINT pgran)
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

bool	ht_streamfile_modifier::isdirty(FILEOFS offset, UINT range)
{
	if (range & IS_DIRTY_SINGLEBIT) {
		return isdirtybit(offset, range & 7);
	} else {
		while (range--) if (isdirtybyte(offset++)) return true;
	}
	return false;
}

bool	ht_streamfile_modifier::isdirtybit(FILEOFS offset, UINT bit)
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

	UINT lsize = ht_layer_streamfile::get_size();
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
	
	UINT s = size - offset;
	if (s > page_granularity) s = page_granularity;
	
	ht_mod_page *m = new ht_mod_page(page_granularity);
	UINT lsize = ht_layer_streamfile::get_size();
	UINT rsize = s;
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
	UINT s = m->size;
	if (offset+s > size) s = size-offset;
	ht_layer_streamfile::seek(offset);
	ht_layer_streamfile::write(m->data, s);
}

int ht_streamfile_modifier::extend(UINT newsize)
{
	// must be opened writable to extend
	if (!active) return EIO;
	UINT osize = size;

	if (size != newsize) modified = true;
	size = newsize;
	for (FILEOFS o = osize & page_mask; o < size; o += page_granularity) {
		if (!mod_page_find(o)) mod_page_create(o);
	}
/**/
	return 0;
//	return ht_layer_streamfile::extend(newsize);
}

UINT ht_streamfile_modifier::get_size()
{
	if (!active) return ht_layer_streamfile::get_size();
	return size;
}

UINT ht_streamfile_modifier::read(void *buf, UINT s)
{
	if (!active) return ht_layer_streamfile::read(buf, s);
	UINT c = 0;
	byte *b = (byte*)buf;
	FILEOFS o = tell();
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
			UINT lc = page_granularity - (o & (~page_mask));
			if (lc > s) lc = s;
			if (o + lc > size) lc = size - o;
			if (!lc) break;

			UINT k;
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

bool	ht_streamfile_modifier::set_access_mode(UINT access_mode)
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

FILEOFS ht_streamfile_modifier::tell()
{
	if (!active) return ht_layer_streamfile::tell();
	return offset;
}

int ht_streamfile_modifier::truncate(UINT newsize)
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

int ht_streamfile_modifier::vcntl(UINT cmd, va_list vargs)
{
	if (active) switch (cmd) {
		case FCNTL_MODS_INVD:
			mod_pages_invd();
			return 0;
		case FCNTL_MODS_FLUSH:
			mod_pages_flush();
			return 0;
		case FCNTL_MODS_CLEAR_DIRTY_RANGE: {
			FILEOFS o=va_arg(vargs, FILEOFS);
			UINT s=va_arg(vargs, UINT);
			UINT i=0;
			while (s--) {
				cleardirtybyte(o+i);
				i++;
			}
			return 0;
		}
		case FCNTL_MODS_IS_DIRTY: {
			FILEOFS o=va_arg(vargs, FILEOFS);
			UINT s=va_arg(vargs, UINT);
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

UINT ht_streamfile_modifier::write(const void *buf, UINT s)
{
	if (!active) return ht_layer_streamfile::write(buf, s);
	FILEOFS o = tell();
	byte *b = (byte*)buf;
	UINT c = 0;

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
			UINT lc = page_granularity - (o & (~page_mask));
			if (lc > s) lc = s;

			UINT k;
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
	modified = (t!=b);
	return (m->write(offset % page_granularity, &b, 1)==1);
}

