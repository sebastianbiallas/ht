/* 
 *	HT Editor
 *	cstream.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#include "cstream.h"
#include "htdebug.h"
#include "except.h"
#include "endianess.h"
# ifdef USE_MINILZO
#  include "minilzo/minilzo.h"
# elif HAVE_LZO_LZO1X_H
#  include <lzo/lzo1x.h>
# elif HAVE_LZO1X_H
#  include <lzo1x.h>
# endif
#include "tools.h"

#include <string.h>

CompressedStream::CompressedStream(Stream *stream, bool own_stream)
	: StreamLayer(stream, own_stream)
{
	if ((stream->getAccessMode() & (IOAM_READ | IOAM_WRITE)) == (IOAM_READ | IOAM_WRITE)) {
		// ht_compressed_stream cant be used for read and write access simultaneously
		assert(0);
	}
	bufferpos = 0;
	buffersize = COMPRESSED_STREAM_DEFAULT_GRANULARITY;
	buffer = ht_malloc(buffersize);
}

CompressedStream::~CompressedStream()
{
	if (getAccessMode() & IOAM_WRITE) {
		flush_compressed();
	}
	free(buffer);
}

void CompressedStream::flush_compressed()
{
	if (bufferpos) {
		byte cbuf[bufferpos + bufferpos / 64 + 16 + 3];
		byte workbuf[LZO1X_1_MEM_COMPRESS];
		lzo_uint cbuf_len;
		byte n[4];

		memset(workbuf, 0, sizeof workbuf);		
		lzo1x_1_compress(buffer, bufferpos, cbuf, &cbuf_len, workbuf);

		createForeignInt(n, bufferpos, 4, big_endian);
		mStream->writex(n, 4);
		createForeignInt(n, cbuf_len, 4, big_endian);
		mStream->writex(n, 4);
		mStream->writex(cbuf, cbuf_len);

		bufferpos = 0;
	}
}

void CompressedStream::flush_uncompressed()
{
	if (bufferpos == 0) {
		free(buffer);
		buffer = NULL;

		uint cbuf_len;
		uint uncompressed_len;
		byte n[4];

		mStream->readx(n, 4);
		uncompressed_len = createHostInt(n, 4, big_endian);
		mStream->readx(n, 4);
		cbuf_len = createHostInt(n, 4, big_endian);
		
		if (!uncompressed_len || uncompressed_len > COMPRESSED_STREAM_DEFAULT_GRANULARITY
		 || !cbuf_len || cbuf_len > 2*COMPRESSED_STREAM_DEFAULT_GRANULARITY) throw IOException(EIO);

		buffer = ht_malloc(uncompressed_len);
		byte cbuf[cbuf_len];

		mStream->readx(cbuf, cbuf_len);
		lzo_uint dummy = uncompressed_len;
		
		lzo1x_decompress_safe(cbuf, cbuf_len, buffer, &dummy, NULL);
		if (dummy != uncompressed_len) throw IOException(EIO);

		buffersize = uncompressed_len;
		bufferpos = uncompressed_len;          
	}
}

uint CompressedStream::read(void *aBuf, uint size)
{
	uint ssize = size;
	byte *buf = (byte *)aBuf;
	while (size >= bufferpos) {
		memcpy(buf, buffer+buffersize-bufferpos, bufferpos);
		buf += bufferpos;
		size -= bufferpos;
		bufferpos = 0;
		if (size) {
			try {
				flush_uncompressed();
			} catch (const EOFException &) {
				return ssize - size;
			}
		} else break;
	}
	if (size) {
		memcpy(buf, buffer+buffersize-bufferpos, size);
		bufferpos -= size;
	}
	return ssize;
}

uint CompressedStream::write(const void *aBuf, uint size)
{
	uint ssize = size;
	const byte *buf = (const byte *)aBuf;
	while (bufferpos+size >= buffersize) {
		memcpy(buffer+bufferpos, buf, buffersize-bufferpos);
		size -= buffersize-bufferpos;
		buf += buffersize-bufferpos;
		bufferpos = buffersize;
		if (size) {
			try {
				flush_compressed();
			} catch (const EOFException &) {
				return ssize - size;
			}
		} else break;
	}
	if (size) {
		memcpy(buffer+bufferpos, buf, size);
		bufferpos += size;
	}
	return ssize;
}
