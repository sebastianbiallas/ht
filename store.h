/*
 *	HT Editor
 *	store.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#ifndef STORE_H
#define STORE_H

#include "common.h"
#include "global.h"
#include "htdata.h"

typedef Object *(*object_builder)();

class ht_stream;

#include "stream.h"

class ht_object_stream_inter: public ht_object_stream {
public:
	virtual Object	*getObject(char *name);
	virtual void	getObject(Object *&o, char *name);
	virtual void	putObject(Object *obj, char *name);
};

class ht_object_stream_bin: public ht_object_stream_inter {
public:
		   void	init(ht_stream *s);
/* overwritten */
	virtual void	*getBinary(int size, char *desc);
	virtual void	getBinary(void *p, int size, char *desc);
	virtual bool	getBool(char *desc);
	virtual int	getIntDec(int size, char *desc);
	virtual int	getIntHex(int size, char *desc);
	virtual qword	getQWordDec(int size, char *desc);
	virtual qword	getQWordHex(int size, char *desc);
	virtual void	getSeparator();
	virtual char	*getString(char *desc);
	virtual void	putBinary(void *mem, int size, char *desc);
	virtual void	putBool(bool b, char *desc);
	virtual void	putInfo(char *info);
	virtual void	putIntDec(int a, int size, char *desc);
	virtual void	putIntHex(int a, int size, char *desc);
	virtual void	putQWordDec(qword a, int size, char *desc);
	virtual void	putQWordHex(qword a, int size, char *desc);
	virtual void	putSeparator();
	virtual void	putString(char *string, char *desc);
};

class ht_object_stream_txt: public ht_object_stream_inter {
protected:
	char		cur;
	int		line;
	int		errorline;
	int		indent;
public:
	   
		void	init(ht_stream *s);
/* overwritten */
	virtual void	*getBinary(int size, char *desc);
	virtual void	getBinary(void *p, int size, char *desc);
	virtual bool	getBool(char *desc);
	virtual int	getIntDec(int size, char *desc);
	virtual int	getIntHex(int size, char *desc);
	virtual qword	getQWordDec(int size, char *desc);
	virtual qword	getQWordHex(int size, char *desc);
	virtual void	getObject(Object *&o, char *name);
	virtual void	getSeparator();
	virtual char	*getString(char *desc);
	virtual void	putBinary(void *mem, int size, char *desc);
	virtual void	putBool(bool b, char *desc);
	virtual void	putInfo(char *info);
	virtual void	putIntDec(int a, int size, char *desc);
	virtual void	putIntHex(int a, int size, char *desc);
	virtual void	putQWordDec(qword a, int size, char *desc);
	virtual void	putQWordHex(qword a, int size, char *desc);
	virtual void	putObject(Object *obj, char *name);
	virtual void	putSeparator();
	virtual void	putString(char *string, char *desc);

		   void	setSyntaxError();
		   int	getErrorLine();
private:
/* io */
		   void	expect(char c);
		   void	skipWhite();
		   char	readChar();
		   void	readDesc(char *desc);

		   
		   void	putDesc(char *desc);
		   void	putIndent();
		   void	putChar(char c);
		   void	putS(char *s);
};

/*
 *   ht_object_stream_memmap dups strings + mem for set/getdata
 *	WARNING: not endian-neutral!!
 */
 
class ht_object_stream_memmap: public ht_object_stream_bin {
protected:
	bool duplicate;
	ht_clist	*allocd;

		   void	*duppa(void *p, int size);
public:
		   void	init(ht_stream *s, bool duplicate);
	virtual void	done();
	virtual void	*getBinary(int size, char *desc);
	virtual void	getBinary(void *p, int size, char *desc);
	virtual int	getIntDec(int size, char *desc);
	virtual int	getIntHex(int size, char *desc);
	virtual qword	getQWordDec(int size, char *desc);
	virtual qword	getQWordHex(int size, char *desc);
	virtual char	*getString(char *desc);
	virtual UINT	recordStart(UINT size);
	virtual void	recordEnd(UINT);
	virtual void	putBinary(void *mem, int size, char *desc);
	virtual void	putIntDec(int a, int size, char *desc);
	virtual void	putIntHex(int a, int size, char *desc);
	virtual void	putQWordDec(qword a, int size, char *desc);
	virtual void	putQWordHex(qword a, int size, char *desc);
	virtual void	putString(char *string, char *desc);
};

#endif
