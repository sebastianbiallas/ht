/*
 *	HT Editor
 *	stream.cc
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
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>	/* for mode definitions */
#include <unistd.h>

#include "htdebug.h"
#include "htexcept.h"
#include "htsys.h"
#include "stream.h"
#include "tools.h"

/*
 *	CLASS ht_stream
 */

#define STREAM_COPYBUF_SIZE	(64*1024)

ht_stream::~ht_stream()
{
}

void	ht_stream::init()
{
	stream_error_func = NULL;
	error = 0;
	access_mode = FAM_NULL;
}

void	ht_stream::done()
{
}

int	ht_stream::call_error_func()
{
	if (stream_error_func) return stream_error_func(this); else
		return SERR_FAIL;
}

void	ht_stream::copy_to(ht_stream *stream)
{
	const UINT bufsize=STREAM_COPYBUF_SIZE;
	byte *buf=(byte*)malloc(bufsize);
	UINT r;
	do {
		r=read(buf, bufsize);
		stream->write(buf, r);
	} while (r == bufsize);
	free(buf);
}

UINT	ht_stream::get_access_mode()
{
	return access_mode;
}

int	ht_stream::get_error()
{
	return error;
}

const char *ht_stream::get_desc()
{
	return NULL;
}

bool	ht_stream::set_access_mode(UINT a)
{
	access_mode=a;
	return true;
}

void	ht_stream::set_error(int e)
{
	error=e;
}

void	ht_stream::set_error_func(stream_error_func_ptr s)
{
	stream_error_func=s;
}

UINT	ht_stream::read(void *buf, UINT size)
{
	return 0;
}

UINT	ht_stream::write(const void *buf, UINT size)
{
	return 0;
}

/*
 *	ht_layer_stream
 */

void	ht_layer_stream::init(ht_stream *s, bool own)
{
	ht_stream::init();
	stream=s;
	own_stream=own;
}

void	ht_layer_stream::done()
{
	if (own_stream) {
		stream->done();
		delete stream;
	}
	ht_stream::done();
}

void	ht_layer_stream::copy_to(ht_stream *stream)
{
	return ht_stream::copy_to(stream);
}

UINT	ht_layer_stream::get_access_mode()
{
	return stream->get_access_mode();
}

int	ht_layer_stream::get_error()
{
	return stream->get_error();
}

const char *ht_layer_stream::get_desc()
{
	return stream->get_desc();
}

bool	ht_layer_stream::set_access_mode(UINT access_mode)
{
	return stream->set_access_mode(access_mode);
}

void	ht_layer_stream::set_error(int error)
{
	stream->set_error(error);
}

void	ht_layer_stream::set_error_func(stream_error_func_ptr stream_error_func)
{
	stream->set_error_func(stream_error_func);
}

void	ht_layer_stream::set_stream_ownership(bool own)
{
	own_stream=own;
}

UINT	ht_layer_stream::read(void *buf, UINT size)
{
	return stream->read(buf, size);
}

UINT	ht_layer_stream::write(const void *buf, UINT size)
{
	return stream->write(buf, size);
}

/*
 *	CLASS ht_object_stream
 */
 
void ht_object_stream::init(ht_stream *s)
{
	ht_layer_stream::init(s, false);
}

void ht_object_stream::done()
{
	ht_layer_stream::done();
}

int  ht_object_stream::getInt(int size, char *desc)
{
	return getIntHex(size, desc);
}

UINT ht_object_stream::recordStart(UINT size)
{
	return 0;
}

void ht_object_stream::recordEnd(UINT a)
{
}

void ht_object_stream::putInt(int a, int size, char *desc)
{
	putIntHex(a, size, desc);
}

/*
 *	CLASS ht_streamfile
 */

int	ht_streamfile::cntl(UINT cmd, ...)
{
	va_list vargs;
	va_start(vargs, cmd);
	int ret=vcntl(cmd, vargs);
	va_end(vargs);
	return ret;
}

int	ht_streamfile::extend(UINT newsize)
{
	return ENOSYS;
}

const char *ht_streamfile::get_filename()
{
	return NULL;
}

UINT	ht_streamfile::get_size()
{
	return 0;
}

void	ht_streamfile::pstat(pstat_t *s)
{
	s->caps=0;
}

int ht_streamfile::seek(FILEOFS offset)
{
	return ENOSYS;
}

FILEOFS ht_streamfile::tell()
{
	return 0;
}

int	ht_streamfile::truncate(UINT newsize)
{
	return ENOSYS;
}

int	ht_streamfile::vcntl(UINT cmd, va_list vargs)
{
	return ENOSYS;
}

ht_streamfile *ht_streamfile::get_layered()
{
	return NULL;
}

void ht_streamfile::set_layered(ht_streamfile *s)
{
}

/*
 *	CLASS ht_layer_streamfile
 */

void	ht_layer_streamfile::init(ht_streamfile *s, bool own)
{
	ht_streamfile::init();
	streamfile=s;
	own_streamfile=own;
}

void	ht_layer_streamfile::done()
{
	if (own_streamfile) {
		streamfile->done();
		delete streamfile;
	}
	ht_streamfile::done();
}

void	ht_layer_streamfile::copy_to(ht_stream *stream)
{
	return ht_streamfile::copy_to(stream);
}

int	ht_layer_streamfile::extend(UINT newsize)
{
	return streamfile->extend(newsize);
}

UINT	ht_layer_streamfile::get_access_mode()
{
	return streamfile->get_access_mode();
}

int	ht_layer_streamfile::get_error()
{
	return streamfile->get_error();
}

ht_streamfile *ht_layer_streamfile::get_layered()
{
	ht_streamfile *q = streamfile->get_layered();
	return q ? q : streamfile;
}

const char *ht_layer_streamfile::get_desc()
{
	return streamfile->get_desc();
}

const char *ht_layer_streamfile::get_filename()
{
	return streamfile->get_filename();
}

UINT	ht_layer_streamfile::get_size()
{
	return streamfile->get_size();
}

void	ht_layer_streamfile::pstat(pstat_t *s)
{
	streamfile->pstat(s);
}

UINT	ht_layer_streamfile::read(void *buf, UINT size)
{
	return streamfile->read(buf, size);
}

int	ht_layer_streamfile::seek(FILEOFS offset)
{
	return streamfile->seek(offset);
}

bool	ht_layer_streamfile::set_access_mode(UINT access_mode)
{
	return streamfile->set_access_mode(access_mode);
}

void	ht_layer_streamfile::set_error(int error)
{
	streamfile->set_error(error);
}

void	ht_layer_streamfile::set_error_func(stream_error_func_ptr stream_error_func)
{
	streamfile->set_error_func(stream_error_func);
}

void ht_layer_streamfile::set_layered(ht_streamfile *s)
{
	ht_streamfile *q = streamfile->get_layered();
	if (q) streamfile->set_layered(s); else streamfile = s;
}
	
void	ht_layer_streamfile::set_streamfile_ownership(bool own)
{
	own_streamfile=own;
}

FILEOFS ht_layer_streamfile::tell()
{
	return streamfile->tell();
}

int	ht_layer_streamfile::truncate(UINT newsize)
{
	return streamfile->truncate(newsize);
}

int	ht_layer_streamfile::vcntl(UINT cmd, va_list vargs)
{
	return streamfile->vcntl(cmd, vargs);
}

UINT	ht_layer_streamfile::write(const void *buf, UINT size)
{
	return streamfile->write(buf, size);
}

/*
 *	CLASS ht_sys_file
 */

void	ht_sys_file::init(int f, bool ofd, UINT am)
{
	ht_streamfile::init();
	fd = f;
	offset = 0;
	own_fd = ofd;
	set_access_mode(am);
}

void	ht_sys_file::done()
{
	if (own_fd && (fd>=0)) close(fd);
	ht_streamfile::done();
}

#define EXTEND_BUFSIZE 1024
int ht_sys_file::extend(UINT newsize)
{
	int r=0;

	UINT oldmode=get_access_mode();
	if (!(oldmode & FAM_WRITE)) set_access_mode(oldmode | FAM_WRITE);

	UINT s=get_size();
	char buf[EXTEND_BUFSIZE];
	memset(buf, 0, sizeof buf);
	newsize-=s;
	seek(s);
	while (newsize) {
		UINT k=MIN(sizeof buf, newsize);
		UINT l=write(buf, k);
		if (l!=k) {
			r=EIO;
			break;
		}
		newsize-=l;
	}

	if (!(oldmode & FAM_WRITE)) set_access_mode(oldmode);
	return r;
}

const char *ht_sys_file::get_desc()
{
	return "file descriptor based file";
}

UINT	ht_sys_file::get_size()
{
	int t = tell();
	UINT r = lseek(fd, 0, SEEK_END);
	lseek(fd, t, SEEK_SET);
	return r;
}

UINT	ht_sys_file::read(void *buf, UINT size)
{
	if (!(access_mode & FAM_READ)) return 0;
	UINT r = ::read(fd, buf, size);
	offset += r;
	return r;
}

int	ht_sys_file::seek(FILEOFS o)
{
	if (o == offset) return 0;
	off_t r = lseek(fd, o, SEEK_SET);
	offset = r;
	return (offset == o) ? 0 : EIO;
}

FILEOFS ht_sys_file::tell()
{
	return offset;
}

UINT	ht_sys_file::write(const void *buf, UINT size)
{
	if (!(access_mode & FAM_WRITE)) return 0;
	UINT r = ::write(fd, buf, size);
	offset += r;
	return r;
}

/*
 *	CLASS ht_stdio_file
 */

void	ht_stdio_file::init(FILE *f, bool ofile, UINT am)
{
	ht_streamfile::init();
	file = f;
	offset = 0;
	own_file = ofile;
	set_access_mode(am);
}

void	ht_stdio_file::done()
{
	if (own_file && file) fclose(file);
	ht_streamfile::done();
}

#define EXTEND_BUFSIZE 1024
int ht_stdio_file::extend(UINT newsize)
{
	int r=0;

	UINT oldmode=get_access_mode();
	if (!(oldmode & FAM_WRITE)) set_access_mode(oldmode | FAM_WRITE);

	UINT s=get_size();
	char buf[EXTEND_BUFSIZE];
	memset(buf, 0, sizeof buf);
	newsize-=s;
	seek(s);
	while (newsize) {
		UINT k=MIN(sizeof buf, newsize);
		UINT l=write(buf, k);
		if (l!=k) {
			r=EIO;
			break;
		}
		newsize-=l;
	}

	if (!(oldmode & FAM_WRITE)) set_access_mode(oldmode);
	return r;
}

const char *ht_stdio_file::get_desc()
{
	return "FILE based file";
}

UINT	ht_stdio_file::get_size()
{
	int t = tell();
	fseek(file, 0, SEEK_END);	/* zero is allowed */
	int r = ftell(file);
	fseek(file, t, SEEK_SET);
	return r;
}

UINT	ht_stdio_file::read(void *buf, UINT size)
{
	if (!(access_mode & FAM_READ)) return 0;
	UINT r = fread(buf, 1, size, file);
	offset += r;
	return r;
}

int	ht_stdio_file::seek(FILEOFS o)
{
	if (o == offset) return 0;
	int r = fseek(file, o, SEEK_SET);
	if (r == 0) offset = o;
	return r;
}

FILEOFS ht_stdio_file::tell()
{
	return offset;
}

UINT	ht_stdio_file::write(const void *buf, UINT size)
{
	if (!(access_mode & FAM_WRITE)) return 0;
	UINT r = fwrite(buf, 1, size, file);
	offset += r;
	return r;
}

/*
 *	CLASS ht_file
 */

void	ht_file::init(const char *fn, UINT am, UINT om)
{
	filename = strdup(fn);
	open_mode = om;
	ht_stdio_file::init(NULL, true, am);
	open_mode = FOM_EXISTS;
}

void	ht_file::done()
{
	if (filename) free(filename);
	ht_stdio_file::done();
}

const char *ht_file::get_desc()
{
	return filename;
}

const char *ht_file::get_filename()
{
	return filename;
}

void	ht_file::pstat(pstat_t *s)
{
	sys_pstat(s, filename);
}

bool	ht_file::set_access_mode(UINT am)
{
	UINT orig_access_mode = access_mode;
	bool r = set_access_mode_internal(am);
	if (!r && !set_access_mode_internal(orig_access_mode))
		throw new ht_io_exception("fatal error: couldn't restore file access mode. %s possibly damaged...", get_filename());
	return r;
}

bool	ht_file::set_access_mode_internal(UINT am)
{
RETRY:
	if (access_mode == am) return true;
	if (file) {
		fclose(file);
		file = NULL;
	}
	access_mode = FAM_NULL;
	char *mode = NULL;

	if (open_mode & FOM_APPEND) {
		mode = "ab+";
	} else if (open_mode & FOM_CREATE) {
		if (am & FAM_WRITE) mode = "wb";
		if (am & FAM_READ) mode = "wb+";
	} else {
		if (am & FAM_READ) mode = "rb";
		if (am & FAM_WRITE) mode = "rb+";
	}

	bool retval = true;
	if (am != FAM_NULL) {
		int e = 0;
		if (open_mode != FOM_CREATE) {
			pstat_t s;
			e = sys_pstat(&s, filename);
			if (!e) {
				if ((!(s.caps & pstat_mode_type) || !HT_S_ISREG(s.mode)) && (am & FAM_WRITE)) {
					// disable write-access to non-regular files
					e = EACCES;
				} else if (HT_S_ISDIR(s.mode)) {
					e = EISDIR;
				} else if (!HT_S_ISREG(s.mode) && !HT_S_ISBLK(s.mode)) {
					e = EINVAL;
				}
			}
		}
		if (!e) {
			file = fopen(filename, mode);
			if (!file) e = errno;
		}
		if (e) {
			set_error(e | STERR_SYSTEM);
			if ((stream_error_func) && (stream_error_func(this) == SERR_RETRY))
				goto RETRY;
			retval = false;
		}
	}
	return retval && ht_streamfile::set_access_mode(am);
}

int	ht_file::truncate(UINT newsize)
{
	int e;
	int old_access_mode = access_mode;
	if (set_access_mode(FAM_NULL)) {
		e = sys_truncate(filename, newsize);
	} else {
		e = EACCES;
	}

	set_access_mode(old_access_mode);
	return e;
}

int	ht_file::vcntl(UINT cmd, va_list vargs)
{
	switch (cmd) {
		case FCNTL_FLUSH_STAT: {
			UINT m = get_access_mode();
			set_access_mode(FAM_NULL);
			set_access_mode(m);
			return 0;
		}
	}
	return ht_streamfile::vcntl(cmd, vargs);
}

/*
 *	CLASS ht_temp_file
 */

void	ht_temp_file::init(UINT am)
{
	ht_stdio_file::init(tmpfile(), true, am);
}

void	ht_temp_file::done()
{
	ht_stdio_file::done();
}

const char *ht_temp_file::get_desc()
{
	return "temporary file";
}

void	ht_temp_file::pstat(pstat_t *s)
{
	s->caps = pstat_size;
	s->size = get_size();
	s->size_high = 0;
}

/*
 *	CLASS ht_memmap_file
 */

void ht_memmap_file::init(byte *b, UINT s)
{
	ht_streamfile::init();
	buf = b;
	pos = 0;
	size = s;
}

void ht_memmap_file::done()
{
}

const char *ht_memmap_file::get_desc()
{
	return "memmap";
}

UINT ht_memmap_file::get_size()
{
	return size;
}

UINT ht_memmap_file::read(void *b, UINT s)
{
	if (pos+s > size) s = size-pos;
	memmove(b, (char*)buf+pos, s);
	pos += s;
	return s;
}

int ht_memmap_file::seek(FILEOFS offset)
{
	pos = offset;
	if (pos > size) pos = size;
	return 0;
}

FILEOFS ht_memmap_file::tell()
{
	return pos;
}

UINT ht_memmap_file::write(const void *b, UINT s)
{
	if (pos+s > size) s = size-pos;
	memmove(((byte*)buf)+pos, b, s);
	pos += s;
	return s;
}

/*
 *	CLASS ht_null_file
 */

void ht_null_file::init()
{
	ht_streamfile::init();
}

void ht_null_file::done()
{
	ht_streamfile::done();
}

int ht_null_file::extend(UINT newsize)
{
	return newsize ? EINVAL : 0;
}

UINT ht_null_file::get_access_mode()
{
	return access_mode;
}

const char *ht_null_file::get_desc()
{
	return "null device";
}

UINT ht_null_file::get_size()
{
	return 0;
}

void ht_null_file::pstat(pstat_t *s)
{
	s->caps = pstat_size;
	s->size = get_size();
	s->size_high = 0;
}

UINT ht_null_file::read(void *buf, UINT size)
{
	return 0;
}

int ht_null_file::seek(FILEOFS offset)
{
	return offset ? EINVAL : 0;
}

bool ht_null_file::set_access_mode(UINT am)
{
	return (am == access_mode);
}

FILEOFS ht_null_file::tell()
{
	return 0;
}

int ht_null_file::truncate(UINT newsize)
{
	return newsize ? EINVAL : 0;
}

UINT ht_null_file::write(const void *buf, UINT size)
{
	return 0;
}

/*
 *	CLASS ht_mem_file
 */

#define HTMEMFILE_INITIAL_SIZE		1024
#define HTMEMFILE_GROW_FACTOR_NUM		4
#define HTMEMFILE_GROW_FACTOR_DENOM	3

void ht_mem_file::init()
{
	ht_mem_file::init(0, HTMEMFILE_INITIAL_SIZE, FAM_READ | FAM_WRITE);
}

void ht_mem_file::init(FILEOFS o, UINT size, UINT am)
{
	ht_streamfile::init();
	ofs=o;
	buf=(byte*)malloc(size ? size : 1);
	ibufsize=size;
	bufsize=size;
	memset(buf, 0, size);
	pos=0;
	dsize=0;
	access_mode=0;
	if (!set_access_mode(am)) throw new ht_io_exception("unable to open memfile");
}

void ht_mem_file::done()
{
	free(buf);
	ht_stream::done();
}

void *ht_mem_file::bufptr()
{
	return buf;
}

int ht_mem_file::extend(UINT newsize)
{
	while (bufsize<newsize) extendbuf();
	dsize=newsize;
	return 0;
}

void ht_mem_file::extendbuf()
{
	resizebuf(extendbufsize(bufsize));
}

UINT ht_mem_file::extendbufsize(UINT bufsize)
{
	return bufsize * HTMEMFILE_GROW_FACTOR_NUM / HTMEMFILE_GROW_FACTOR_DENOM;
}

UINT	ht_mem_file::get_access_mode()
{
	return ht_stream::get_access_mode();
}

const char *ht_mem_file::get_desc()
{
	return "memfile";
}

UINT ht_mem_file::get_size()
{
	return dsize;
}

void ht_mem_file::pstat(pstat_t *s)
{
	s->caps = pstat_size;
	s->size = get_size();
	s->size_high = 0;
}

UINT ht_mem_file::read(void *b, UINT size)
{
	if (pos+size>dsize) {
		if (pos >= dsize) return 0;
		size=dsize-pos;
	}
	memmove(b, buf+pos, size);
	pos+=size;
	return size;
}

void ht_mem_file::resizebuf(UINT newsize)
{
	bufsize=newsize;

	assert(dsize <= bufsize);

	byte *t=(byte*)malloc(bufsize ? bufsize : 1);
	memset(t, 0, bufsize);
	memmove(t, buf, dsize);
	free(buf);
	buf=t;
}

int ht_mem_file::seek(FILEOFS o)
{
	if (o<ofs) return EINVAL;
	pos=o-ofs;
	return 0;
}

bool	ht_mem_file::set_access_mode(UINT access_mode)
{
	if (ht_stream::set_access_mode(access_mode)) {
		if (seek(ofs) != 0) return false;
		return true;
	}
	return false;
}

UINT ht_mem_file::shrinkbufsize(UINT bufsize)
{
	return bufsize * HTMEMFILE_GROW_FACTOR_DENOM / HTMEMFILE_GROW_FACTOR_NUM;
}

void ht_mem_file::shrinkbuf()
{
	resizebuf(shrinkbufsize(bufsize));
}

FILEOFS ht_mem_file::tell()
{
	return pos+ofs;
}

int ht_mem_file::truncate(UINT newsize)
{
	dsize=newsize;

	UINT s=ibufsize;
	while (s<dsize) s=extendbufsize(s);
	
	resizebuf(s);
	
	return 0;
}

UINT ht_mem_file::write(const void *b, UINT size)
{
	while (pos+size>=bufsize) extendbuf();
	memmove(((byte*)buf)+pos, b, size);
	pos+=size;
	if (pos>dsize) dsize=pos;
	return size;
}

/*
 *	string stream functions
 */

char *fgetstrz(ht_streamfile *file)
{
	FILEOFS o=file->tell();
/* get string size */
	char buf[64];
	int s, z=0;
	int found=0;
	while (!found) {
		if (!(s=file->read(buf, 64))) return NULL;
		for (int i=0; i<s; i++) {
			z++;
			if (!buf[i]) {
				found=1;
				break;
			}
		}
	}
/* read string */
	char *str=(char *)malloc(z);
	file->seek(o);
	file->read(str, z);
	return str;
}

// FIXME: more dynamical solution appreciated
#define REASONABLE_STRING_LIMIT	1024
 
char *getstrz(ht_stream *stream)
{
/* get string size */
	char buf[REASONABLE_STRING_LIMIT];
	int s, z=0;
	while (1) {
		if ((s=stream->read(buf+z, 1))!=1) return NULL;
		z++;
		if (z>=REASONABLE_STRING_LIMIT) {
			z=REASONABLE_STRING_LIMIT;
			break;
		}
		if (!buf[z-1]) break;
	}
	if (!z) return NULL;
	char *str=(char *)malloc(z);
	memmove(str, buf, z-1);
	str[z-1]=0;
	return str;
}

void putstrz(ht_stream *stream, const char *str)
{
	stream->write(str, strlen(str)+1);
}

char *getstrp(ht_stream *stream)
{
	unsigned char l;
	stream->read(&l, 1);
	char *str=(char*)malloc(l+1);
	stream->read(str, l);
	*(str+l)=0;
	return str;
}

void putstrp(ht_stream *stream, const char *str)
{
	unsigned char l=strlen(str);
	stream->write(&l, 1);
	stream->write(str, l);
}

char *getstrw(ht_stream *stream)
{
	short t;
	byte lbuf[2];
	stream->read(lbuf, 2);
	int l = lbuf[0] | lbuf[1] << 8;
	char	*a=(char*)malloc(l+1);
	for (int i=0; i<l; i++) {
		stream->read(&t, 2);
		*(a+i)=(char)t;
	}
	*(a+l)=0;
	return a;
}

void putstrw(ht_stream *stream, const char *str)
{
	/* FIXME: someone implement me ? */
}

char *ht_strerror(int error)
{
	return strerror(error & (~STERR_SYSTEM));
}

