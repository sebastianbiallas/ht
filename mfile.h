/*
 *	HT Editor
 *	mfile.h
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

#ifndef __MFILE_H__
#define __MFILE_H__

#include "stream.h"

/*
 *   File areas
 */
class FileArea: public Object {
public:
	FileOfs start;
	FileOfs size;

				FileArea(FileOfs start, FileOfs size);
	/* extends Object */
	virtual	int		compareTo(const Object *obj) const;
};

class ModifiedFileArea: public FileArea {
public:
	byte *buf;
				ModifiedFileArea(FileOfs start, FileOfs size);
	virtual			~ModifiedFileArea();
	/* extends Object */
	virtual	ObjectID	getObjectID() const;
};

class CopiedFileArea: public FileArea {
public:
	FileOfs src_start;
				CopiedFileArea(FileOfs start, FileOfs size, FileOfs src_start);
	/* extends Object */
	virtual	ObjectID	getObjectID() const;
};

/**
 *	File modification layer.
 *	This is a file modification layer. Ie. a file-layer that keeps track
 *	of all modifications made to it, without forwarding (or "flushing") them to
 *	the underlying (possibly physical) file.
 *	This is invisible however to the user of this object as all modifications are
 *	reflected back when retrieving information through the <b>File</b> interface.
 *	"Flushing" (ie. applying the modifications) must be done explicitly through
 *	a call to <b>fcntl(FCNTL_MODS_FLUSH)</b>.
 *
 *	This object is (esp. for big files) much more memory- and time-efficient
 *	compared to an "all-in-memory"- or even a "modpages-in-memory"-approach:
 *
 *	"all-in-memory" approach<br>
 *	========================<br>
 *	This can be achieved using the MemoryFile object (with little modification).
 *	The whole underlying <b>File</b> is read once on creation of this object and
 *	then kept in memory until destruction.
 *	Modifications received through the <b>File</b> interface are instantly applied to the
 *	this "in-memory image".
 *
 *	Performance analysis (performance grades are relative to the probability of the event):
 *		- read() and write() cause a single call to 'memcpy'. optimal performance.
 *		- truncate() and extend() cause a single call to 'realloc'. good performance.
 *		- insert() and cut() cause a single call to 'memmove'. bad performance.
 *		- memory consumption is constantly high. not memory-efficient.
 *	This approach is naive and very easy to implement, which is why:
 *	This is a very common approach for text- and hex-editors (although
 *	it is VERY BAD for huge files).<p>
 *
 *	"modpages-in-memory" approach<br>
 *	=============================<br>
 *	This approach divides the underlying <b>File</b> into seamless blocks of
 *	equal size. These blocks are called "pages". Two kinds of pages exist:
 *	modified and unmodified pages.
 *	Right after construction, all pages are unmodified. (We may use a tree
 *	that contains only modified pages. So this tree is empty right after
 *	construction).
 *	If a write-(or read-)operation spans multiple pages, it is treated as
 *	multiple discrete write-(or read-)operations not spanning multiple pages.
 *	Whenever an attempt is made to <b>write()</b> to a page, it is made sure
 *	that this page is modified. If it is not modified, "page-size" bytes of
 *	memory are allocated and filled with the corresponding bytes from the
 *	underlying file. The write is then performed in memory. 
 *	<b>read()</b>s are served from modified pages if possible.
 *	<b>extend()</b> and <b>truncate()</b> are treated like writes with zeros.
 *	But they must create new pages and/or change the size of the last page
 *	and <b>realloc()</b>.
 *	<b>insert()</b> and <b>cut()</b> have "naive" implementations, which
 *	translates them into a combination of extend()/read()/write() and
 *	read()/write()/truncate(), respectively.
 *
 *	Performance analysis (performance grades are relative to the probability of the event):
 *		- read() and write() cause little trouble. good performance.
 *		- truncate() and extend() cause little trouble. good performance.
 *		- insert() and cut() cause big trouble. bad performance.
 *		- memory consumption is high only for insert()/extend(). kind-of memory-efficient.
 *	This approach is easy to implement. It has been used extensively used
 *	in HT 0.7.x and its predecessors.<p>
 *
 *	"modareas-in-memory" approach<br>
 *	=============================<br>
 *	This is the approach used by a <b>FileModificator</b>. This is meant
 *	to be a more progressive approach compared to the above.
 *	As is well known from complexity-theory (and a programmer's all-day
 *	work) you can often save memory by wasting some more (CPU-)time and
 *	vice-versa.
 *	As pointed out in both analyses above the <b>insert()</b> and <b>cut()</b>
 *	operations cause these approaches to comsume very much memory (about as much
 *	as the file's size !).
 *	We will combine these two statements into a new approach:
 *	It's derived from the "modpages-in-memory" approach, with some exceptions:
 *	- "seamless blocks of equal size" become "seamless, dynamically sized blocks".
 *	  We will call them areas.
 *	- due to their dynamic size, areas also have dynamic start offsets.
 *
 *	Taking the notions from "modpages-in-memory" we have then also:
 *	"modified and unmodified areas".
 *	But we will now call the unmodified areas "copied areas".
 *	(And both copied and modified areas must to registered in a tree).
 *	<b>insert()</b> and <b>cut()</b> can now be implemented by simple
 *	operations on the areas. Memory consumption drops drastically on huge
 *	files (as compared to all other methods).
 *	This approach is hard to implement (the code written is complex).
 *	It will be used in (at least) HT 2.x.x .
 */
class FileModificator: public FileLayer {
protected:
	AVLTree mods;
	FileOfs newsize;
	FileOfs pos;
	int mcount;
	int inv_mcount;

		bool		cut1(ObjHandle h, FileOfs rstart, FileOfs size);
		ObjHandle	findArea(FileOfs o);
		void		flushMods();
		void		invalidateMods();
		bool		isModified() const;
		bool		isModifiedByte(FileOfs o);
		void 		makeAreaModified(ObjHandle h, FileOfs rstart, FileOfs size);
		void		read1(FileArea *a, FileOfs rstart, byte *buf, uint count);
		void		write1(FileArea *a, FileOfs rstart, const byte *buf, uint count);
public:
				FileModificator(File *file, bool own_file);
	/* <debug> */
		void		checkSanity();
		void		debug();
	/* </debug> */
	/* extends FileLayer */
	virtual	FileOfs		copyAllTo(Stream *stream);
	virtual	FileOfs		copyTo(Stream *stream, FileOfs count);
	virtual	void		cut(FileOfs size);
	virtual	void		extend(FileOfs newsize);
	virtual String &	getDesc(String &result) const;
	virtual	FileOfs		getSize() const;
	virtual	uint		read(void *buf, uint size);
	virtual	void		seek(FileOfs offset);
	virtual	FileOfs		tell() const;
	virtual	void		truncate(FileOfs newsize);
	virtual	int		vcntl(uint cmd, va_list vargs);
	virtual	uint		write(const void *buf, uint size);
	virtual	void		insert(const void *buf, FileOfs size);
};

#endif /* __MFILE_H__ */
