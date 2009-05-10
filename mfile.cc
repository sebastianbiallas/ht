/*
 *	HT Editor
 *	mfile.cc
 *
 *	Copyright (C) 1999-2003 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <cerrno>
#include <cstdlib>
#include <cstring>

// <debug>
#include "htdebug.h"
#include "snprintf.h"
// </debug>

#include "except.h"
#include "mfile.h"

#define OBJID_MFA MAGIC32("FAr\0")
#define OBJID_CFA MAGIC32("FAr\1")

// "min condition" value, see makeAreaModified(...)
#define MIN_COND_MFA_SIZE	(16)

// absolute maximum size of a MFA
#define MAX_MFA_SIZE		(128)

/*
 *	File area (FA)
 */
FileArea::FileArea(FileOfs Start, FileOfs Size)
{
	start = Start;
	size = Size;
}

int FileArea::compareTo(const Object *obj) const
{
	const FileArea *l, *r;
	int sign = 1;
	l = (FileArea*)obj;
	r = this;
	if (r->start < l->start) {
		const FileArea *t = l;
		l = r;
		r = t;
		sign = -sign;
	}

	if (r->start < l->start + l->size) return 0;
	return sign;
}

/*
 *	Modified file area (MFA)
 */
ModifiedFileArea::ModifiedFileArea(FileOfs start, FileOfs size)
: FileArea(start, size)
{
	buf = ht_malloc(size);
}

ModifiedFileArea::~ModifiedFileArea()
{
	free(buf);
}

ObjectID ModifiedFileArea::getObjectID() const
{
	return OBJID_MFA;
}

/*
 *	Copied file area (CFA)
 */
CopiedFileArea::CopiedFileArea(FileOfs start, FileOfs size, FileOfs Src_start)
: FileArea(start, size)
{
	src_start = Src_start;
}

ObjectID CopiedFileArea::getObjectID() const
{
	return OBJID_CFA;
}

/*
 *	File modification layer (FML)
 *
 *	FML is a FileLayer, which adds efficient in-memory file modification
 *	abilities.
 *	To achieve this, FML uses a tree containing FileArea (FA) objects, that
 *	describe the differences between the modified and the original file.
 *
 *	'mods' (tree which contains FAs) rules:
 *	(1) all FAs together must exactly cover the area [0, getSize()-1]
 *	(2) no two FAs may overlap
 *	(3) MFAs size may not be less than 1 (but not necessarily less than
 *	    MIN_COND_MFA_SIZE, as the name might suggest !)
 *	(4) MFAs size may not be greater than MAX_MFA_SIZE
 *	(5) two adjacent MFAs must have the sum of their sizes > MAX_MFA_SIZE
 *
 *	(general) minimize both the number of FAs in 'mods' and the space wasted
 *	          by each FA (ie. make MFAs at least 48 bytes in 'size',
 *	          because malloc will always alloc this many bytes for
 *	          MFA->buf (~16 bytes), the MFA itself (~16 bytes) and it's
 *	          entry in AVLTree (~16 bytes) on most architectures)
 */
FileModificator::FileModificator(File *file, bool own_file)
: FileLayer(file, own_file), mods(true)
{
	mcount = 0;
	inv_mcount = 0;
	invalidateMods();
}

void FileModificator::checkSanity()
{
	FileOfs s = 0;
	FileArea *prevx = NULL;
	foreach(FileArea, x, mods,
//		bool iscopy = dynamic_cast<CopiedFileArea*>(x) != NULL;
		bool ismod = dynamic_cast<ModifiedFileArea*>(x) != NULL;
		// rule (1) part 1 and rule (2)
		if (x->start != s)
			throw MsgfException("mfile-sanity: area starts at 0x%qx, should be 0x%qx", x->start, s);
		// rule (3)
		if (x->size < 1)
			throw MsgfException("mfile-sanity: area at 0x%qx, has size %qd (must be >= %d)", x->start, x->size, 1);
		// rule (4)
		if (x->size > MAX_MFA_SIZE && ismod)
			throw MsgfException("mfile-sanity: area at 0x%qx, has size %qd (must be <= %d)", x->start, x->size, MAX_MFA_SIZE);
		// rule (5)
/*		if (prevx && (dynamic_cast<ModifiedFileArea*>(prevx) != NULL)
		 && ismod && (((ModifiedFileArea*)prevx)->size + 
		((ModifiedFileArea*)x)->size <= MAX_MFA_SIZE))
			throw MsgException("mfile-sanity: two adjacent MFAs with sum of sizes <= MAX_MFA_SIZE = %d", MAX_MFA_SIZE);
*/		s += x->size;
		prevx = x;
	);
	// rule (1) part 2
	if (newsize != s)
		throw MsgfException("mfile-sanity: newsize = %qx != %qx = calculated_size", &newsize, &s);
}

void FileModificator::debug()
{
	FILE *f = stderr;
	fprintf(f, "<areas>\n");
	foreach(FileArea, x, mods,
		if (dynamic_cast<ModifiedFileArea*>(x)) {
			ModifiedFileArea *m = (ModifiedFileArea *)x;
			ht_fprintf(f, "\tmodify %08qx, %qd bytes", &x->start, &x->size);
			for (uint i=0; i < x->size; i++) {
				ht_fprintf(f, " %02x", m->buf[i]);
			}
			ht_fprintf(f, " '");
			for (uint i=0; i<x->size; i++) {
				byte c = m->buf[i];
				if ((c<32) || (c>0x80)) c = 32;
				ht_fprintf(f, "%c", c);
			}
			ht_fprintf(f, "'\n");
		} else {
			CopiedFileArea *c = (CopiedFileArea *)x;
			ht_fprintf(f, "\tcopy   %08qx, %qd bytes, orig start %08qx\n", &x->start, &x->size, &c->src_start);
		}
	);
	fprintf(f, "</areas>\n");
	fprintf(f, "sanity:\n");
	checkSanity();
}

bool FileModificator::cut1(ObjHandle h, FileOfs rstart, FileOfs size)
{
	FileArea *a = (FileArea*)mods.get(h);
	bool have_head_gap = rstart != 0;
	bool have_tail_gap = a->size > rstart+size;
	if (!have_head_gap && !have_tail_gap) {
		mods -= h;
		return true;
	}
	if (a->getObjectID() == OBJID_CFA) {
		CopiedFileArea *c = (CopiedFileArea*)a;
		if (have_head_gap) {
			FileOfs csize = c->size;
			c->size = rstart;
			if (have_tail_gap) {
				mods += new CopiedFileArea(c->start + rstart, csize - rstart - size, c->src_start +rstart+size);
			}
		} else {
			c->size -= size;
			c->src_start += size;
		}
	} else {
		assert(a->getObjectID() == OBJID_MFA);
		ModifiedFileArea *m = (ModifiedFileArea*)a;
		memmove(m->buf + rstart, m->buf + rstart+size, m->size-rstart-size);
		m->size -= size;
		m->buf = (byte*)realloc(m->buf, m->size);
/*		if (have_head_gap) {
			byte *newbuf = ht_malloc(m->size);
			memcpy(newbuf, m->buf, rstart);
			memcpy(newbuf+rstart, m->buf+rstart+size, m->size-rstart);
			free(m->buf);
			m->buf = newbuf;
		} else {
			memmove(m->buf, m->buf + size, m->size);
			m->buf = (byte*)realloc(m->buf, m->size);
		}
*/
	}
	return false;
}

/*
 *	NOTE 1 (aka changing keys of already-inserted Tree-elements, see below)
 *	======
 *	this is bad code because we change the key
 *	of an already-inserted Tree-element, but here it is both
 *	fast and functional...
 */
#define STREAM_COPYBUF_SIZE	(64*1024)
FileOfs FileModificator::copyAllTo(Stream *stream)
{
	byte *buf = new byte[STREAM_COPYBUF_SIZE];
	FileOfs result = 0;
	uint r, t;
	do {
		uint k = STREAM_COPYBUF_SIZE;
		r = read(buf, k);
		assert(r <= k);
		t = stream->write(buf, r);
		assert(t <= r);
		result += t;
		if (t != k) break;
	} while (t);
	delete[] buf;
	return result;
}

FileOfs FileModificator::copyTo(Stream *stream, FileOfs count)
{
	byte *buf = new byte[STREAM_COPYBUF_SIZE];
	FileOfs result = 0;
	while (count) {
		FileOfs k = STREAM_COPYBUF_SIZE;
		if (k > count) k = count;
		uint r = read(buf, k);
		assert(r <= k);
		uint t = stream->write(buf, r);
		assert(t <= r);
		count -= t;
		result += t;
		if (t != k) break;
	}
	delete[] buf;
	return result;
}

void FileModificator::cut(FileOfs size)
{
	if (!(getAccessMode() & IOAM_WRITE)) throw IOException(EACCES);

	FileOfs o = tell();
	if (o + size > newsize) throw IOException(EINVAL);
	ObjHandle h = findArea(o);
	FileOfs ssize = size;

//	int i = 1;
	while (size) {
//		printf("it %d\n", i++);
		assert(h != invObjHandle)
		FileArea *a = (FileArea*)mods.get(h);
		FileOfs s = o - a->start;
		FileOfs z = a->size - s;
		if (z > size) z = size;
		ObjHandle hnext = mods.findNext(h);
		bool deleted = cut1(h, s, z);

		if (!deleted && o == a->start)
			a->start -= ssize-size;// see NOTE 1 above
		o += z;
		size -= z;
		newsize -= z;
		if (deleted) {
			h = findArea(o);
		} else {
			h = hnext;
		}			
	}
	while (h != invObjHandle) {
		FileArea *a = (FileArea*)mods.get(h);
		h = mods.findNext(h);
		a->start -= ssize;	// see NOTE 1 above
	}
	mcount++;
}

void FileModificator::extend(FileOfs Newsize)
{
	if (Newsize == newsize) return;
	if (Newsize < newsize) throw IOException(EINVAL);
	if (!(getAccessMode() & IOAM_WRITE)) throw IOException(EACCES);

	seek(0);
	// find last area
	ObjHandle h = (newsize!=0) ? findArea(newsize-1) : invObjHandle;
	FileOfs c = Newsize-newsize;
	if (h != invObjHandle) {
		FileArea *a = (FileArea*)mods.get(h);
		ModifiedFileArea *m = dynamic_cast<ModifiedFileArea*>(a);
		if (m) {
			// if its a modification, merge with the following modifications
			if (m->size < MAX_MFA_SIZE) {
				FileOfs s = m->size + c;
				if (s > MAX_MFA_SIZE) s = MAX_MFA_SIZE;
				m->buf = (byte*)realloc(m->buf, s);
				FileOfs d = s - m->size;
				memset(m->buf+m->size, 0, d);
				newsize += d;
				c -= d;
				m->size = s;
			}
		}
	}
	while (c != 0) {
		FileOfs s = c;
		if (s > MAX_MFA_SIZE) s = MAX_MFA_SIZE;
		ModifiedFileArea *a = new ModifiedFileArea(newsize, s);
		memset(a->buf, 0, s);
		mods += a;
		newsize += s;
		c -= s;
	}
	mcount++;
}

ObjHandle FileModificator::findArea(FileOfs o)
{
	ObjHandle h;
//	if (o != 0) {
		FileArea a(o, 1);
		h = mods.find(&a);
/*	} else {
		h = mods.findFirst();
	}*/
	return h;
}

String &FileModificator::getDesc(String &result) const
{
	mFile->getDesc(result);
	if (isModified()) result.prepend("in-memory modified ");
	return result;
}

FileOfs FileModificator::getSize() const
{
	return newsize;
}

void FileModificator::insert(const void *buf, FileOfs size)
{
	if (!(getAccessMode() & IOAM_WRITE)) throw IOException(EACCES);

	ObjHandle h;

	if (!size) return;
	FileOfs ssize = size;
	// (1) insert/merge or append MFAs
	FileOfs t = tell();
//	ht_printf("tell %qx\n", t);
	ObjHandle h_a = findArea(t);
	FileArea *a = (FileArea*)mods.get(h_a);

	// really insert or append ?
	if (a) {
		// yes, insert.

		// relocate all FAs after 'a'
		h = mods.findLast();
		while (h != h_a) {
			FileArea *a = (FileArea*)mods.get(h);
			h = mods.findPrev(h);
			a->start += ssize;	// see NOTE 1 above
		}
		if (a->start == t) {
			a->start += ssize;
			FileOfs start = t;
			while (ssize != 0) {
				FileOfs k = MAX_MFA_SIZE;
				if (k > ssize) k = ssize;
				mods += new ModifiedFileArea(start, k);
				ssize -= k;
				start += k;
			}
		} else if (a->getObjectID() == OBJID_CFA) {
			// split CFA at offset t, and relocate high (offset) part
			CopiedFileArea *c = (CopiedFileArea*)a;
			FileOfs d = t - a->start;
			// b becomes 'high' (offset) a
			FileArea *b = new CopiedFileArea(c->start + d + ssize, c->size -d, c->src_start+d);
			// a becomes 'low' (offset) a
			a->size = d;
			mods += b;

			// create new MFAs between 'a' and 'b'
			FileOfs start = t;
			while (ssize != 0) {
				FileOfs k = MAX_MFA_SIZE;
				if (k > ssize) k = ssize;
				mods += new ModifiedFileArea(start, k);
				ssize -= k;
				start += k;
			}
		} else {
			ModifiedFileArea *m = (ModifiedFileArea*)a;
			// FIXME: should merge existing MFA with new one(s)

			// split MFA at offset t and relocate high (offset) part
			FileOfs d = t - a->start;
			ModifiedFileArea *b = new ModifiedFileArea(m->start + d + ssize, m->size -d);
			memcpy(b->buf, m->buf+d, b->size);
			m->size = d;
			m->buf = (byte*)realloc(m->buf, d);

			mods += b;

			// create new MFAs between 'a' and 'b'
			FileOfs start = t;
			while (ssize != 0) {
				FileOfs k = MAX_MFA_SIZE;
				if (k > ssize) k = ssize;
				mods += new ModifiedFileArea(start, k);
				ssize -= k;
				start += k;
			}
		}
	} else {
		// no, append.
		FileOfs start = t;
		while (ssize != 0) {
			FileOfs k = MAX_MFA_SIZE;
			if (k > ssize) k = ssize;
			mods += new ModifiedFileArea(start, k);
			ssize -= k;
			start += k;
		}
	}
	
	// (2) write data
	const byte *b = (const byte*)buf;
	FileOfs o = t;
	h = findArea(o);
	while (size && (h != invObjHandle)) {
		FileArea *a = (FileArea*)mods.get(h);
		FileOfs s = o - a->start;
		FileOfs z = a->size - s;
		if (z > size) z = size;
		if (z > a->size) z = a->size;
//		ht_printf("%qx, now write %qx\n", a->start, z);
		write1(a, s, b, z);
		o += z;
		size -= z;
		b += z;
		h = mods.findNext(h);
	}
	newsize += o-t;
	seek(o);
	mcount++;
}

void FileModificator::makeAreaModified(ObjHandle h, FileOfs rstart, FileOfs size)
{
	FileArea *a = (FileArea*)mods.get(h);
	if (a->getObjectID() == OBJID_CFA) {
		// not marked modified, do something
		CopiedFileArea *c = (CopiedFileArea*)a;
		FileOfs csrc_start = c->src_start;
		FileOfs cstart = c->start;
		FileOfs csize = c->size;
		bool have_head_gap = rstart != 0;
		bool have_tail_gap = csize > rstart+size;
		ModifiedFileArea *prev_m = (ModifiedFileArea*)mods.get(mods.findPrev(h));
//		ASSERT(!prev_m || (prev_m->getObjectID() == OBJID_MFA));
		if (prev_m && (prev_m->getObjectID() != OBJID_MFA)) prev_m = NULL;
		ModifiedFileArea *next_m = (ModifiedFileArea*)mods.get(mods.findNext(h));
		if (next_m && (next_m->getObjectID() != OBJID_MFA)) next_m = NULL;
//		ASSERT(!next_m || (next_m->getObjectID() == OBJID_MFA));
		// "min condition"
		// ===============
		// trick for FA packing: we simply make modified areas bigger
		// than they would have to be. (either at least MIN_COND_MFA_SIZE
		// big or adjacent/mergable with another MFA)
		bool min_condition = (size < MIN_COND_MFA_SIZE) && (!prev_m || have_head_gap) && (!next_m || have_tail_gap);
		FileOfs min_addsize;
		FileOfs min_ofs;
		FileOfs min_src_ofs;
		if (min_condition) {
			// if the MFA we will create would be too small, make it bigger
			// (compare gap sizes, then try to reach boundary to
			// reduce number of FAs)
			if (rstart < csize-rstart-size) {
				// head gap is smaller than tail gap
				FileOfs z = rstart;
				if (z+size > MIN_COND_MFA_SIZE) z = MIN_COND_MFA_SIZE-size;
				min_addsize = z;
				min_ofs = cstart+rstart-z;
				min_src_ofs = csrc_start+rstart-z;
				rstart -= z;
			} else {
				// tail gap is smaller than head gap
				FileOfs z = csize-rstart-size;
				if (z+size > MIN_COND_MFA_SIZE) z = MIN_COND_MFA_SIZE-size;
				min_addsize = z;
				min_ofs = cstart+rstart+size;
				min_src_ofs = csrc_start+rstart+size;
			}
			size += min_addsize;
			have_head_gap = rstart != 0;
			have_tail_gap = csize > rstart+size;
		}
		if (have_head_gap) {
			c->size = rstart;
			cstart += rstart;
			csrc_start += rstart;
			csize -= rstart;
		} else {
			mods -= h;
		}			
		if (next_m && !have_tail_gap) {
			// merge with next MFA (successor)
			FileOfs nnsize = next_m->size + size;
			if (nnsize > MAX_MFA_SIZE) nnsize = MAX_MFA_SIZE;
			mods.removeObj(next_m);
			FileOfs rnnsize = nnsize - next_m->size;
			next_m->buf = (byte*)realloc(next_m->buf, nnsize);
			next_m->start -= rnnsize;
			memmove(next_m->buf+rnnsize, next_m->buf, next_m->size);
			next_m->size = nnsize;
			mods += next_m;
			csize -= rnnsize;
			size -= rnnsize;
		}
		if (prev_m && !have_head_gap) {
			// merge with previous MFA (predecessor)
			FileOfs pnsize = prev_m->size + size;
			if (pnsize > MAX_MFA_SIZE) pnsize = MAX_MFA_SIZE;
			FileOfs rpnsize = pnsize - prev_m->size;
			prev_m->size = pnsize;
			prev_m->buf = (byte*)realloc(prev_m->buf, pnsize);
			csrc_start += rpnsize;
			cstart += rpnsize;
			csize -= rpnsize;
			size -= rpnsize;
		}
		while (size != 0) {
			FileOfs k = MAX_MFA_SIZE;
			if (k > size) k = size;
			mods += new ModifiedFileArea(cstart, k);
			size -= k;
			csize -= k;
			cstart += k;
			csrc_start += k;
		}
		if (have_tail_gap) {
			mods += new CopiedFileArea(cstart, csize, csrc_start);
		}
		if (min_condition) {
			byte *buf = ht_malloc(min_addsize);
			mFile->seek(min_src_ofs);
			mFile->readx(buf, min_addsize);
			ObjHandle ha = findArea(min_ofs);
			while (min_addsize != 0) {
				FileArea *a = (FileArea*)mods.get(ha);
				assert(dynamic_cast<ModifiedFileArea*>(a));
				FileOfs k = min_addsize;
				if (k > a->size- (min_ofs-a->start)) k = a->size- (min_ofs-a->start);
				write1(a, min_ofs - a->start, buf, k);
				min_addsize -= k;
				min_ofs += k;
				ha = mods.findNext(ha);
			}
			free(buf);
		}
	}
}

void FileModificator::read1(FileArea *a, FileOfs rstart, byte *buf, uint count)
{
	CopiedFileArea *c;
	ModifiedFileArea *m;
	if ((m = dynamic_cast<ModifiedFileArea*>(a))) {
		memcpy(buf, m->buf + rstart, count);
	} else if ((c = dynamic_cast<CopiedFileArea*>(a))) {
		mFile->seek(c->src_start+rstart);
		mFile->read(buf, count);
	} else assert(0);
}

uint FileModificator::read(void *buf, uint size)
{
	if (!(getAccessMode() & IOAM_READ)) throw IOException(EACCES);

	FileOfs t = tell();
	if (t == 0x00000000ULL) {
		int a = 1;
	}
	FileOfs o = t;
	ObjHandle h = findArea(o);
	byte *b = (byte*)buf;

	while (size && h != invObjHandle) {
		FileArea *a = (FileArea*)mods.get(h);
		FileOfs s = o - a->start;
		FileOfs z = a->size - s;
		if (z > size) z = size;
		h = mods.findNext(h);
		read1(a, s, b, z);
		o += z;
		size -= z;
		b += z;
	}
	seek(o);
	return o - t;
}

void FileModificator::seek(FileOfs offset)
{
	if (offset > newsize) throw IOException(EINVAL);
	pos = offset;
}

FileOfs FileModificator::tell() const
{
	return pos;
}

void FileModificator::truncate(FileOfs Newsize)
{
	if (Newsize == newsize) return;
	if (Newsize > newsize) throw IOException(EINVAL);
	if (!(getAccessMode() & IOAM_WRITE)) throw IOException(EACCES);
	seek(0);
	ObjHandle h = findArea(Newsize);
	FileArea *a = (FileArea*)mods.get(h);
	if (a->start < Newsize) {
		if (a->getObjectID() == OBJID_CFA) {
			CopiedFileArea *c = (CopiedFileArea*)a;
			c->size = Newsize - a->start;
		} else {
			ModifiedFileArea *m = (ModifiedFileArea*)a;
			m->size = Newsize - a->start;
			m->buf = (byte*)realloc(m->buf, m->size);
		}
		h = mods.findNext(h);
	}
	while (h != invObjHandle) {
		ObjHandle hnext = mods.findNext(h);
		FileArea *a = (FileArea*)mods.get(hnext);
		mods -= h;
		if (!a) break;
		h = findArea(a->start);
	}
	newsize = Newsize;
	mcount++;
}

void FileModificator::flushMods()
{
	// make sure we can read and write
	int e;
	IOAccessMode old_am = mFile->getAccessMode();
	e = mFile->setAccessMode(IOAM_READ | IOAM_WRITE);
	if (e) throw IOException(e);
	// start work
	if (newsize > mFile->getSize()) mFile->extend(newsize);
	// store CFAs with start > src_start in descending order
	foreachbwd(FileArea, fa, mods,
		if (fa->getObjectID() == OBJID_CFA) {
			CopiedFileArea *cfa = (CopiedFileArea*)fa;
			if (cfa->start > cfa->src_start) {
				mFile->move(cfa->src_start, cfa->start, cfa->size);
			}
		}
	);
	// store CFAs with start < src_start in ascending order
	foreach(FileArea, fa, mods,
		if (fa->getObjectID() == OBJID_CFA) {
			CopiedFileArea *cfa = (CopiedFileArea*)fa;
			if (cfa->start < cfa->src_start) {
				mFile->move(cfa->src_start, cfa->start, cfa->size);
			}
		}
	);
	// store MFAs
	foreach(FileArea, fa, mods,
		if (fa->getObjectID() == OBJID_MFA) {
			ModifiedFileArea *mfa = (ModifiedFileArea*)fa;
			mFile->seek(mfa->start);
			mFile->writex(mfa->buf, mfa->size);
		}
	);
	//
	if (newsize < mFile->getSize()) mFile->truncate(newsize);
	// restore old access mode
	e = mFile->setAccessMode(old_am);
	if (e) throw IOException(e);
	//
	inv_mcount = mcount;
	invalidateMods();
}

void FileModificator::invalidateMods()
{
	mods.delAll();
	newsize = mFile->getSize();
	if (newsize != 0) mods += new CopiedFileArea(0, newsize, 0);
	pos = 0;
	mcount = inv_mcount;
}

bool FileModificator::isModified() const
{
	if (newsize != mFile->getSize()) return true;
	foreach(FileArea, fa, mods,
		CopiedFileArea *cfa = dynamic_cast<CopiedFileArea*>(fa);
		if (!cfa || (cfa->start != cfa->src_start)) return true;
	);
	return false;
}

bool FileModificator::isModifiedByte(FileOfs o)
{
	ObjHandle h = findArea(o);
	FileArea *fa = (FileArea*)mods.get(h);
	if (!fa) return true;
	if (fa->getObjectID() == OBJID_MFA) {
		ModifiedFileArea *mfa = (ModifiedFileArea*)fa;
		int pg_ofs = o - fa->start;
		byte b;
		mFile->seek(o);
		mFile->readx(&b, 1);
		return mfa->buf[pg_ofs] != b;
	} else if (fa->getObjectID() == OBJID_CFA) {
		CopiedFileArea *cfa = (CopiedFileArea*)fa;
		if (cfa->src_start == cfa->start) return false;
		int pg_ofs = o - fa->start;
		byte b1, b2;
		mFile->seek(o);
		mFile->readx(&b1, 1);
		mFile->seek(cfa->src_start + pg_ofs);
		mFile->readx(&b2, 1);
		return (b1 != b2);
	}
	return false;
}

int FileModificator::vcntl(uint cmd, va_list vargs)
{
	switch (cmd) {
	case FCNTL_MODS_INVD:
		invalidateMods();
		return 0;
	case FCNTL_MODS_FLUSH:
		flushMods();
		return 0;
	case FCNTL_MODS_CLEAR_DIRTY_RANGE: {
/*
		// BEFORE-IMPLEMENT: decl changed !
		FileOfs o = va_arg(vargs, FileOfs);
		FileOfs s = va_arg(vargs, FileOfs);
		while (s--) {
			cleardirtybyte(o++);
		}
*/
		return 0;
	}
	case FCNTL_MODS_IS_DIRTY: {
		// const FileOfs offset, const FileOfs range, bool &isdirty
		FileOfs o = va_arg(vargs, FileOfs);
		FileOfs s = va_arg(vargs, FileOfs);
		bool &b = (bool&)*va_arg(vargs, bool*);
		if (o == 0 && s == newsize) {
			b = isModified();
		} else if (s <= 16) {
			try {
				bool bb = false;
				while (s--) {
					bb |= isModifiedByte(o++);
				}
				b = bb;
			} catch (const IOException &x) {
				return EIO;
			}
		} else {
			return ENOSYS;
		}
		return 0;
	}
	case FCNTL_GET_MOD_COUNT: {	// int &mcount
		int *mc = va_arg(vargs, int *);
		*mc = mcount;
		return 0;
	}		
	}
	return mFile->vcntl(cmd, vargs);
}

void FileModificator::write1(FileArea *a, FileOfs rstart, const byte *buf, uint count)
{
	assert(a->getObjectID() == OBJID_MFA);
//	ObjectID id = a->getObjectID();
	ModifiedFileArea *m = (ModifiedFileArea*)a;
	memcpy(m->buf+rstart, buf, count);
}

uint FileModificator::write(const void *buf, uint size)
{
	if (!(getAccessMode() & IOAM_WRITE)) throw IOException(EACCES);
	// (1) make areas modified
	FileOfs t = tell();
	FileOfs o = t;
	uint osize = size;
	while (size) {
		ObjHandle h = findArea(o);
		if (h == invObjHandle) break;
		FileArea *a = (FileArea*)mods.get(h);
		FileOfs s = o - a->start;
		FileOfs z = a->size - s;
		if (z > size) z = size;
		makeAreaModified(h, s, z);
		o += z;
		size -= z;
	}
	// (2) write data
	const byte *b = (const byte*)buf;
	o = t;
	size = osize;
	ObjHandle h = findArea(o);
	while (size && (h != invObjHandle)) {
		FileArea *a = (FileArea*)mods.get(h);
		FileOfs s = o - a->start;
		FileOfs z = a->size - s;
		if (z > size) z = size;
		if (z > a->size) z = a->size;
		write1(a, s, b, z);
		o += z;
		size -= z;
		b += z;
		h = mods.findNext(h);
	}
	seek(o);
	mcount++;
	return o - t;
}
