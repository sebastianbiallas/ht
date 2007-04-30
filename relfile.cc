/* 
 *	HT Editor
 *	relfile.cc
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

#include <string.h>

#include "htdebug.h"
#include "relfile.h"
#include "tools.h"

#define MAX_RELOC_ITEM_LEN 8

/*
 *	ht_reloc_file
 */
ht_reloc_file::ht_reloc_file(File *s, bool os)
	: FileLayer(s, os)
{
	relocs = new AVLTree(true);
	enabled = true;
}

ht_reloc_file::~ht_reloc_file()
{
	delete relocs;
}

void ht_reloc_file::finalize()
{
//	relocs->set_compare_keys(compare_keys_uint);
}

int ht_reloc_file::vcntl(uint cmd, va_list vargs)
{
	switch (cmd) {
		case FCNTL_GET_RELOC: {
			bool *e = va_arg(vargs, bool*);
			*e = enabled;
			return 0;
		}
		case FCNTL_SET_RELOC: {
			enabled = (bool)(va_arg(vargs, int));
			return 0;
		}
	}
	return FileLayer::vcntl(cmd, vargs);
}

void ht_reloc_file::insert_reloc(FileOfs o, Object *reloc)
{
	relocs->insert(new KeyValue(new UInt64(o), reloc));
}

uint ht_reloc_file::read(void *buf, uint size)
{
	FileOfs o = tell();
	/* read fine data. */
	uint ret = FileLayer::read(buf, size);
	uint c = ret;
	if (enabled) {
		ObjHandle oh;
		if ((MAX_RELOC_ITEM_LEN+1) <= o) {
			KeyValue kv(new UInt64(o - (MAX_RELOC_ITEM_LEN+1)), NULL);
			oh = relocs->findG(&kv);
		} else {
			oh = relocs->findFirst();
		}
		/* enum through 'relocs' - the tree that contains all our
		 * dear relocations - starting some bytes before the current
		 * (stream) offset to get all the relocation items that may
		 * intersect with our fine read data. */
		while (oh != invObjHandle) {
			KeyValue *kv = (KeyValue *)relocs->get(oh);
			UInt64 *k = (UInt64 *)kv->mKey;

			/* buffer to apply relocation to */
			byte b[MAX_RELOC_ITEM_LEN];
			/* stop if the item is "behind" the area this function
			 * should work on. */
			if (k->value >= o+c) break;

			/* if relocation item intersects with the beginning of
			 * this read, copy buf to b+s */
			uint s = (k->value < o) ? o - k->value : 0;
			/* if relocation item intersects with the end of
			 * this read, copy buf+e to b */
			uint e = (k->value > o) ? k->value - o : 0;

			/* complicated calculation to get the size of the
			 * intended intersection (=: mm) of the read and b. */
			uint l = (k->value + sizeof b > o+c) ?
				k->value + sizeof b - o - c : 0;
			uint mm = MIN(sizeof b - l, sizeof b - s);

			/* probably cleaner to clear it all before we start
			 * because if the read is smaller then the reloc item
			 * we'd have some undefined bytes in b. */
			memset(b, 0, sizeof b);

			/* never memmove beyond Bounds of b. (maybe you didn't call finalize()?) */
			assert(mm+s <= sizeof b);

			/* move read data to b as good as we can. */
			memmove(b+s, ((byte*)buf)+e, mm);
			/* apply complete relocation item. */
			reloc_apply(kv->mValue, b);
			/* overwrite read with relocated/read data as good as we can. */
			memcpy(((byte*)buf)+e, b+s, mm);

			oh = relocs->findG(kv);
		}
	}
	return ret;
}

uint ht_reloc_file::write(const void *buf, uint size)
{
	/* documentation: see read(). */
	FileOfs o;
	if (enabled) {
		o = tell();
		uint c = size;
		ObjHandle oh;
		if ((MAX_RELOC_ITEM_LEN+1) <= o) {
			KeyValue kv(new UInt64(o - (MAX_RELOC_ITEM_LEN+1)), NULL); 
			oh = relocs->findG(&kv);
		} else {
			oh = relocs->findFirst();
		}

		while (oh != invObjHandle) {
			KeyValue *kv = (KeyValue *)relocs->get(oh);
			UInt64 *k = (UInt64 *)kv->mKey;

			byte b[MAX_RELOC_ITEM_LEN];
			if (k->value >= o+c) break;

			uint s = (k->value < o) ? o - k->value : 0;
			uint e = (k->value > o) ? k->value - o : 0;

			uint l = (k->value+sizeof b > o+c) ?
				k->value + sizeof b - o - c : 0;

			memset(b, 0, sizeof b);
			uint mm = MIN(sizeof b - l, sizeof b - s);

			assert(mm+s <= sizeof b);
			memmove(b+s, ((byte*)buf)+e, mm);
			// FIXME: return here ???
			if (!reloc_unapply(kv->mValue, b)) /*return 0*/;
			// FIXME: violation of function declaration "const void *buf"
			memcpy(((byte*)buf)+e, b+s, mm);
			oh = relocs->findG(kv);
		}
	}
	return FileLayer::write(buf, size);
}
