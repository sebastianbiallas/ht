/* 
 *	HT Editor
 *	cstream.cc
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

#include "cstream.h"
#include "htdebug.h"
#include "htendian.h"
#include "minilzo.h"
#include "tools.h"

#include <string.h>

void	ht_compressed_stream::init(ht_stream *stream, bool own_stream, UINT granularity=COMPRESSED_STREAM_DEFAULT_GRANULARITY)
{
	ht_layer_stream::init(stream, own_stream);
	if ((get_access_mode() & (FAM_READ | FAM_WRITE)) == (FAM_READ | FAM_WRITE)) {
		// ht_compressed_stream cant be used for read and write access simultaneously
		assert(0);
	}
	buffer = (byte *)smalloc(granularity);
	bufferpos = 0;
	buffersize = granularity;
}

void	ht_compressed_stream::done()
{
	if (get_access_mode() & FAM_WRITE) {
		flush_compressed();
	}
	if (buffer) free(buffer);
	ht_layer_stream::done();
}

bool ht_compressed_stream::flush_compressed()
{
	if (bufferpos) {
		byte *cbuf = (byte *)smalloc(bufferpos + bufferpos / 64 + 16 + 3);
		byte *workbuf = (byte *)smalloc(LZO1X_1_MEM_COMPRESS);
		lzo_uint cbuf_len;
		byte n[4];
		
		lzo1x_1_compress(buffer, bufferpos, cbuf, &cbuf_len, workbuf);

		free(workbuf);

		create_foreign_int(n, bufferpos, 4, big_endian);
		if (stream->write(n, 4)!=4) {
			free(cbuf);
			return false;
		}			
		create_foreign_int(n, cbuf_len, 4, big_endian);
		if (stream->write(n, 4)!=4) {
			free(cbuf);
			return false;
		}			
		if (stream->write(cbuf, cbuf_len)!=cbuf_len) {
			free(cbuf);
			return false;
		}			

		free(cbuf);
		
		bufferpos = 0;
	}
	return true;
}

bool ht_compressed_stream::flush_uncompressed()
{
	if (bufferpos==0) {
		free(buffer);
		buffer = NULL;

		UINT cbuf_len;
		UINT uncompressed_len;
		byte n[4];

		if (stream->read(n, 4)!=4) return false;
		uncompressed_len = create_host_int(n, 4, big_endian);
		if (stream->read(n, 4)!=4) return false;
		cbuf_len = create_host_int(n, 4, big_endian);

		buffer = (byte *)smalloc(uncompressed_len);
		byte *cbuf = (byte *)smalloc(cbuf_len);
		if (stream->read(cbuf, cbuf_len)!=cbuf_len) {
			free(cbuf);
			return false;
		}

		lzo_uint dummy;
		lzo1x_decompress(cbuf, cbuf_len, buffer, &dummy, NULL);
		assert(dummy == uncompressed_len);

		free(cbuf);

		buffersize = uncompressed_len;
		bufferpos = uncompressed_len;          
	}
	return true;
}

UINT	ht_compressed_stream::read(void *buf, UINT size)
{
	UINT ssize = size;
	while (size >= bufferpos) {
		memcpy(buf, buffer+buffersize-bufferpos, bufferpos);
		((byte *)buf) += bufferpos;
		size -= bufferpos;
		bufferpos = 0;
		if (size) {
			if (!flush_uncompressed()) return ssize - size;
		} else break;
	}
	if (size) {
		memcpy(buf, buffer+buffersize-bufferpos, size);
		bufferpos -= size;
	}
	return ssize;
}

UINT	ht_compressed_stream::write(void *buf, UINT size)
{
	UINT ssize = size;
	while (bufferpos+size >= buffersize) {
		memcpy(buffer+bufferpos, buf, buffersize-bufferpos);
		size -= buffersize-bufferpos;
		((byte *)buf) += buffersize-bufferpos;
		bufferpos = buffersize;
		if (size) {
			if (!flush_compressed()) return ssize - size;
		} else break;
	}
	if (size) {
		memcpy(buffer+bufferpos, buf, size);
		bufferpos += size;
	}
	return ssize;
}
