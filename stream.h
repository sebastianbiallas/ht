/* 
 *	HT Editor
 *	stream.h
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

#ifndef __STREAM_H__
#define __STREAM_H__

#include "common.h"
#include "global.h"
#include "htio.h"

#include <stdarg.h>
#include <stdio.h>

#ifndef HAVE_FILEOFS
#define HAVE_FILEOFS
typedef UINT FILEOFS;
#endif

/*
 *	CLASS ht_stream
 */

/* stream_error_func return values */
#define SERR_FAIL		0
#define SERR_RETRY		1

/* ht_stream.error */
#define STERR_SYSTEM		0x80000000

class ht_stream;

typedef int (*stream_error_func_ptr)(ht_stream *stream);

class ht_stream {
protected:
	stream_error_func_ptr stream_error_func;
	int	error;
	UINT	access_mode;

		   int		call_error_func();
public:
		   void		init();
	virtual void		done();
/* new */
	virtual void		copy_to(ht_stream *stream);
	virtual UINT		get_access_mode();
	virtual int		get_error();
	virtual const char *get_desc();
	virtual UINT		read(void *buf, UINT size);
	virtual bool		set_access_mode(UINT access_mode);
	virtual void		set_error(int error);
	virtual void		set_error_func(stream_error_func_ptr stream_error_func);
	virtual UINT		write(const void *buf, UINT size);
};

/*
 *	CLASS ht_layer_stream
 */

class ht_layer_stream: public ht_stream {
protected:
	ht_stream *stream;
	bool own_stream;
	
public:
	
		   void		init(ht_stream *stream, bool own_stream);
	virtual void		done();
/* overwritten */
	virtual void		copy_to(ht_stream *stream);
	virtual UINT		get_access_mode();
	virtual int		get_error();
	virtual const char *get_desc();
	virtual UINT		read(void *buf, UINT size);
	virtual bool		set_access_mode(UINT access_mode);
	virtual void		set_error(int error);
	virtual void		set_error_func(stream_error_func_ptr stream_error_func);
		   void		set_stream_ownership(bool own);
	virtual UINT		write(const void *buf, UINT size);
};

/*
 *	CLASS ht_object_stream
 */

class ht_object_stream: public ht_layer_stream {
public:
			void		init(ht_stream *s);
	virtual	void		done();
	
/* new */
	virtual	void *	getBinary(int size, char *desc) = 0;
	virtual	void		getBinary(void *p, int size, char *desc) = 0;
	virtual	bool		getBool(char *desc) = 0;
			int		getInt(int size, char *desc);
	virtual	int		getIntDec(int size, char *desc) = 0;
	virtual	int		getIntHex(int size, char *desc) = 0;
	virtual	qword	getQWordDec(int size, char *desc) = 0;
	virtual	qword	getQWordHex(int size, char *desc) = 0;
	virtual	Object *	getObject(char *name) = 0;
	virtual	void		getObject(Object *&o, char *name) = 0;
	virtual	void		getSeparator() = 0;
	virtual	char *	getString(char *desc) = 0;
	virtual	UINT		recordStart(UINT size);
	virtual	void		recordEnd(UINT);
	virtual	void		putBinary(void *mem, int size, char *desc) = 0;
	virtual	void		putBool(bool b, char *desc) = 0;
	virtual	void		putInfo(char *info) = 0;
			void		putInt(int a, int size, char *desc);
	virtual	void		putIntDec(int a, int size, char *desc) = 0;
	virtual	void		putIntHex(int a, int size, char *desc) = 0;
	virtual	void		putQWordDec(qword a, int size, char *desc) = 0;
	virtual	void		putQWordHex(qword a, int size, char *desc) = 0;
	virtual	void		putObject(Object *obj, char *name) = 0;
	virtual	void		putSeparator() = 0;
	virtual	void		putString(char *string, char *desc) = 0;
};

#define PUT_BINARY(st, d, size) st->putBinary(d, size, #d)
#define PUT_BOOL(st, d) st->putBool(d, #d)
#define PUT_INT(st, d) st->putInt(d, 4, #d)
#define PUT_INT_DEC(st, d) st->putIntDec(d, 4, #d)
#define PUT_INT_HEX(st, d) st->putIntHex(d, 4, #d)
#define PUT_QWORD_DEC(st, d) st->putQWordDec(d, 8, #d)
#define PUT_QWORD_HEX(st, d) st->putQWordHex(d, 8, #d)
#define PUT_OBJECT(st, o) st->putObject(o, #o)
#define PUT_STRING(st, d) st->putString(d, #d)

#define GET_BOOL(st, d) d=st->getBool(#d)
#define GET_INT(st, d) d=st->getInt(4, #d)
#define GET_INT_DEC(st, d) d=st->getIntDec(4, #d)
#define GET_INT_HEX(st, d) d=st->getIntHex(4, #d)
#define GET_QWORD_DEC(st, d) d=st->getQWordDec(8, #d)
#define GET_QWORD_HEX(st, d) d=st->getQwordHex(8, #d)
#define GET_OBJECT(st, o) st->getObject((Object *)o, #o)
#define GET_STRING(st, d) d=st->getString(#d)

/*
 *	CLASS ht_streamfile
 */

/* cntl cmd */
#define FCNTL_MODS_INVD				0x00000001
#define FCNTL_MODS_FLUSH				0x00000002
#define FCNTL_MODS_IS_DIRTY			0x00000003	// FILEOFS offset, UINT range, bool *isdirty
#define FCNTL_MODS_CLEAR_DIRTY		0x00000004
#define FCNTL_MODS_CLEAR_DIRTY_RANGE	0x00000005	// FILEOFS offset, UINT range
#define FCNTL_FLUSH_STAT				0x00000006
#define FCNTL_GET_RELOC				0x00000007	// bool *enabled
#define FCNTL_SET_RELOC				0x00000008	// bool enable

#define IS_DIRTY_SINGLEBIT			0x80000000

/* ht_file.access_mode */
#define FAM_NULL		0
#define FAM_READ		1
#define FAM_WRITE		2

/* ht_file open mode */
#define FOM_EXISTS		0
#define FOM_CREATE		1
#define FOM_APPEND		2

class ht_streamfile: public ht_stream {
public:
/* new */
		   int			cntl(UINT cmd, ...);
	virtual int			extend(UINT newsize);
	virtual const char *	get_filename();
	virtual ht_streamfile *	get_layered();
	virtual UINT			get_size();
	virtual void			pstat(pstat_t *s);
	virtual int			seek(FILEOFS offset);
	virtual void			set_layered(ht_streamfile *streamfile);
	virtual FILEOFS		tell();
	virtual int			truncate(UINT newsize);
	virtual int			vcntl(UINT cmd, va_list vargs);
};

/*
 *	CLASS ht_layer_streamfile
 */

class ht_layer_streamfile: public ht_streamfile {
protected:
	ht_streamfile *streamfile;
	bool own_streamfile;
public:
		   void	init(ht_streamfile *streamfile, bool own_streamfile);
	virtual void	done();
/* overwritten */
	virtual void			copy_to(ht_stream *stream);
	virtual int			extend(UINT newsize);
	virtual UINT			get_access_mode();
	virtual int			get_error();
	virtual const char *	get_desc();
	virtual const char *	get_filename();
	virtual ht_streamfile *	get_layered();
	virtual UINT			get_size();
	virtual void			pstat(pstat_t *s);
	virtual UINT			read(void *buf, UINT size);
	virtual int			seek(FILEOFS offset);
	virtual bool			set_access_mode(UINT access_mode);
	virtual void			set_error(int error);
	virtual void			set_error_func(stream_error_func_ptr stream_error_func);
	virtual void			set_layered(ht_streamfile *streamfile);
		   void			set_streamfile_ownership(bool own);
	virtual FILEOFS		tell();
	virtual int			truncate(UINT newsize);
	virtual int			vcntl(UINT cmd, va_list vargs);
	virtual UINT			write(const void *buf, UINT size);
};

/*
 *	CLASS ht_sys_file
 */

class ht_sys_file: public ht_streamfile {
protected:
	int fd;
	bool own_fd;

	FILEOFS offset;
public:

		   void		init(int fd, bool own_fd, UINT access_mode);
	virtual void		done();
/* overwritten */
	virtual int		extend(UINT newsize);
	virtual const char *get_desc();
	virtual UINT		get_size();
	virtual UINT		read(void *buf, UINT size);
	virtual int		seek(FILEOFS offset);
	virtual FILEOFS 	tell();
	virtual UINT		write(const void *buf, UINT size);
};

/*
 *	CLASS ht_stdio_file
 */

class ht_stdio_file: public ht_streamfile {
protected:
	FILE *file;
	bool own_file;

	FILEOFS offset;
public:

		   void		init(FILE *file, bool own_file, UINT access_mode);
	virtual void		done();
/* overwritten */
	virtual int		extend(UINT newsize);
	virtual const char *get_desc();
	virtual UINT		get_size();
	virtual UINT		read(void *buf, UINT size);
	virtual int		seek(FILEOFS offset);
	virtual FILEOFS 	tell();
	virtual UINT		write(const void *buf, UINT size);
};

/*
 *	CLASS ht_file
 */

class ht_file: public ht_stdio_file {
protected:
	char *filename;
	UINT open_mode;

	FILEOFS offset;

	bool	set_access_mode_internal(UINT access_mode);
public:

		   void		init(const char *filename, UINT access_mode, UINT open_mode);
	virtual void		done();
/* overwritten */
	virtual const char *get_desc();
	virtual const char *get_filename();
	virtual void		pstat(pstat_t *s);
	virtual bool		set_access_mode(UINT access_mode);
	virtual int		truncate(UINT newsize);
	virtual int		vcntl(UINT cmd, va_list vargs);
};

/*
 *	CLASS ht_temp_file
 */

class ht_temp_file: public ht_stdio_file {
public:
		   void		init(UINT access_mode);
	virtual void		done();
/* overwritten */
	virtual const char *get_desc();
	virtual void		pstat(pstat_t *s);
};

/*
 *	CLASS ht_memmap_file
 */

class ht_memmap_file: public ht_streamfile {
protected:
	FILEOFS pos;
	UINT size;
	byte *buf;

public:
		   void		init(byte *buf, UINT size);
	virtual void		done();
/* overwritten */
	virtual const char *get_desc();
	virtual UINT		get_size();
	virtual UINT		read(void *buf, UINT size);
	virtual int		seek(FILEOFS offset);
	virtual FILEOFS 	tell();
	virtual UINT		write(const void *buf, UINT size);
};

/*
 *	CLASS ht_null_file
 */

class ht_null_file: public ht_streamfile {
protected:
public:
		   void		init();
	virtual void		done();
/* overwritten */
	virtual int		extend(UINT newsize);
	virtual UINT		get_access_mode();
	virtual const char *get_desc();
	virtual UINT		get_size();
	virtual void		pstat(pstat_t *s);
	virtual UINT		read(void *buf, UINT size);
	virtual int		seek(FILEOFS offset);
	virtual bool		set_access_mode(UINT access_mode);
	virtual FILEOFS 	tell();
	virtual int		truncate(UINT newsize);
	virtual UINT		write(const void *buf, UINT size);
};

/*
 *	CLASS ht_mem_file
 */

class ht_mem_file: public ht_streamfile {
protected:
	FILEOFS ofs;
	FILEOFS pos;
	UINT bufsize, dsize, ibufsize;
	byte *buf;

	virtual UINT		extendbufsize(UINT bufsize);
	virtual UINT		shrinkbufsize(UINT bufsize);
		   void		extendbuf();
		   void		shrinkbuf();
		   void		resizebuf(UINT newsize);
public:
		   void		init();
		   void		init(FILEOFS ofs, UINT size, UINT access_mode);
	virtual void		done();
/* overwritten */
	virtual int		extend(UINT newsize);
	virtual UINT		get_access_mode();
	virtual const char *get_desc();
	virtual UINT		get_size();
	virtual void		pstat(pstat_t *s);
	virtual UINT		read(void *buf, UINT size);
	virtual int		seek(FILEOFS offset);
	virtual bool		set_access_mode(UINT access_mode);
	virtual FILEOFS 	tell();
	virtual int		truncate(UINT newsize);
	virtual UINT		write(const void *buf, UINT size);
/* new */
		   void*		bufptr();
};

/*
 *	string stream functions
 */

/* 0-terminated ASCII */
char *fgetstrz(ht_streamfile *file);
char *getstrz(ht_stream *stream);
void putstrz(ht_stream *stream, const char *str);

/* pascal-style ASCII */
char *getstrp(ht_stream *stream);
void putstrp(ht_stream *stream, const char *str);

/* 0-terminated wide/unicode */
char *getstrw(ht_stream *stream);
void putstrw(ht_stream *stream, const char *str);

char *ht_strerror(int error);

#endif /* __STREAM_H__ */

