/*
 *	HT Editor
 *	store.h
 *
 *	Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@web-productions.de)
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

typedef object *(*object_builder)();

class ht_stream;

#include "stream.h"

class ht_object_stream_inter: public ht_object_stream {
public:
	virtual object	*get_object(char *name);
	virtual void	get_object(object *&o, char *name);
	virtual void	put_object(object *obj, char *name);
};

class ht_object_stream_bin: public ht_object_stream_inter {
public:
		   void	init(ht_stream *s);
/* overwritten */
	virtual void	*get_binary(int size, char *desc);
	virtual void	get_binary(void *p, int size, char *desc);
	virtual bool	get_bool(char *desc);
	virtual int	get_int_dec(int size, char *desc);
	virtual int	get_int_hex(int size, char *desc);
	virtual void	get_separator();
	virtual char	*get_string(char *desc);
	virtual void	put_binary(void *mem, int size, char *desc);
	virtual void	put_bool(bool b, char *desc);
	virtual void	put_info(char *info);
	virtual void	put_int_dec(int a, int size, char *desc);
	virtual void	put_int_hex(int a, int size, char *desc);
	virtual void	put_separator();
	virtual void	put_string(char *string, char *desc);
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
	virtual void	*get_binary(int size, char *desc);
	virtual void	get_binary(void *p, int size, char *desc);
	virtual bool	get_bool(char *desc);
	virtual int	get_int_dec(int size, char *desc);
	virtual int	get_int_hex(int size, char *desc);
	virtual void	get_object(object *&o, char *name);
	virtual void	get_separator();
	virtual char	*get_string(char *desc);
	virtual void	put_binary(void *mem, int size, char *desc);
	virtual void	put_bool(bool b, char *desc);
	virtual void	put_info(char *info);
	virtual void	put_int_dec(int a, int size, char *desc);
	virtual void	put_int_hex(int a, int size, char *desc);
	virtual void	put_object(object *obj, char *name);
	virtual void	put_separator();
	virtual void	put_string(char *string, char *desc);

		   void	set_syntax_error();
		   int	get_error_line();
/* io */
		   void	expect(char c);
		   void	skip_white();
		   char	read_char();
		   void	read_desc(char *desc);

		   
		   void	put_desc(char *desc);
		   void	put_indent();
		   void	put_char(char c);
		   void	put_s(char *s);
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
	virtual void	*get_binary(int size, char *desc);
	virtual void	get_binary(void *p, int size, char *desc);
	virtual int	get_int_dec(int size, char *desc);
	virtual int	get_int_hex(int size, char *desc);
	virtual char	*get_string(char *desc);
	virtual UINT	record_start(UINT size);
	virtual void	record_end(UINT);
	virtual void	put_binary(void *mem, int size, char *desc);
	virtual void	put_int_dec(int a, int size, char *desc);
	virtual void	put_int_hex(int a, int size, char *desc);
	virtual void	put_string(char *string, char *desc);
};

#endif
