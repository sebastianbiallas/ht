/* 
 *	HT Editor
 *	htpeil.h
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

#ifndef __HTPEIL_H__
#define __HTPEIL_H__

#include "data.h"
#include "formats.h"
#include "ilstruct.h"

extern format_viewer_if htpeil_if;

class ht_il_metadata_entry: public Object {
public:
	char *name;
	uint32 offset;
	uint32 size;
	ht_il_metadata_entry(const char *name, uint32 offset, uint32 size);
	~ht_il_metadata_entry();
};

class ht_pe_il: public Object {
public:
	PE_IL_DIRECTORY dir;
	IL_METADATA_SECTION metadata;
	Container *entries;
	uint32 string_pool_size;
	char *string_pool;
};

/*
 *	CLASS ht_pe_header_viewer
 */

class ht_pe_il_viewer: public ht_uformat_viewer {
public:
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *group);
	virtual void done();
};

int ILunpackDword(uint32 &result, const byte *buf, int len);
int ILunpackToken(uint32 &result, const byte *buf, int len);

#endif /* !__HTPEIL_H__ */
