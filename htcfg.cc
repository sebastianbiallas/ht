/*
 *	HT Editor
 *	htcfg.cc
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
 
#include "cstream.h"
#include "htcfg.h"
#include "htctrl.h"
#include "htdebug.h"
#include "htendian.h"
#include "htreg.h"
#include "htstring.h"
#include "stream.h"
#include "store.h"
#include "htsys.h"
#include "tools.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* VERSION 2 (for ht-0.4.4 and later) */

/* NOTE: as of Version 2 ALL integers in HT config-files are
   stored in big-endian format... (non-intel) */
   
#define object_stream_bin			0
#define object_stream_txt			1
#define object_stream_bin_compressed	2

ht_object_stream *create_object_stream(ht_stream *f, int object_stream_type)
{
	ht_object_stream *s;
	switch (object_stream_type) {
		case object_stream_bin: {
			s=new ht_object_stream_bin();
			((ht_object_stream_bin*)s)->init(f);
			break;
		}
		case object_stream_txt: {
			s=new ht_object_stream_txt();
			((ht_object_stream_txt*)s)->init(f);
			break;
		}
		case object_stream_bin_compressed: {
			ht_compressed_stream *cs=new ht_compressed_stream();
			cs->init(f, false);
			s=new ht_object_stream_bin();
			((ht_object_stream_bin*)s)->init(cs);
			s->set_stream_ownership(true);
			break;
		}
		default: {
			return NULL;
		}
	}
	return s;
}

struct config_header {
	char magic[4] HTPACKED;
	char version[4] HTPACKED;
	char stream_type[2] HTPACKED;
};

/*
 *	system configs
 */

char *systemconfig_file;

/**/

loadstore_result save_systemconfig()
{
	ht_file *f = new ht_file();
	f->init(systemconfig_file, FAM_WRITE, FOM_CREATE);
	if (f->get_error()) {
		f->done();
		delete f;
		return LS_ERROR_WRITE;
	}
	
	/* write project config header */
	config_header h;

	memmove(h.magic, ht_systemconfig_magic, sizeof h.magic);

	char q[16];

	int system_ostream_type = get_config_dword("misc/config format");
	
	sprintf(q, "%04x", ht_systemconfig_fileversion);
	memmove(h.version, q, sizeof h.version);

	sprintf(q, "%02x", system_ostream_type);
	memmove(h.stream_type, q, sizeof h.stream_type);

	f->write(&h, sizeof h);
	
	/* write object stream type */
	ht_object_stream *d = create_object_stream(f, system_ostream_type);
	   
	switch (system_ostream_type) {
		case object_stream_bin:
			break;
		case object_stream_txt:
			f->write((void*)"\n#\n#\tThis is a generated file!\n#\n", 33);
			break;
	}
	/* write config */
	app->store(d);
	if (d->get_error()) return LS_ERROR_WRITE;
		
	d->done();
	delete d;

	f->done();
	delete f;
	
	return LS_OK;
}

bool load_systemconfig(loadstore_result *result, int *error_info)
{
	ht_file *f = new ht_file();
	f->init(systemconfig_file, FAM_READ, FOM_EXISTS);
	switch (f->get_error()) {
		case 0:break;
		case STERR_SYSTEM | ENOENT:
			f->done();
			delete f;
			*result = LS_ERROR_NOT_FOUND;
			return false;
		default:
			f->done();
			delete f;
			*result = LS_ERROR_READ;
			return false;
	}
	/* read project config header */
	config_header h;

	if (f->read(&h, sizeof h)!=sizeof h) {
		*result = LS_ERROR_MAGIC;
		f->done();
		delete f;
		return false;
	}
	
	if (memcmp(h.magic, ht_systemconfig_magic, sizeof h.magic)!=0) {
		*result = LS_ERROR_MAGIC;
		f->done();
		delete f;
		return false;
	}

	// FIXME: bad code, no conversion errors reported
	if (hexw((char*)h.version) != ht_systemconfig_fileversion) {
		*result = LS_ERROR_VERSION;
		*error_info = hexw((char*)h.version);
		f->done();
		delete f;
		return false;
	}

	dword object_stream_type = hexb((char*)h.stream_type);

	/* object stream type */
	ht_object_stream *d = create_object_stream(f, object_stream_type);
	if (!d) {
		*result = LS_ERROR_FORMAT;
		f->done();
		delete f;
		return false;
	}
	
	/* read config */
	if (app->load(d)!=0) {
		*result = LS_ERROR_CORRUPTED;
		*error_info = 0;
		if (d->get_error()) {
			if (object_stream_type==object_stream_txt)
				*error_info = ((ht_object_stream_txt*)d)->getErrorLine();
		}
		f->done();
		delete f;
		return false;
	}
	
	d->done();
	delete d;

	f->done();
	delete f;	

	*result = LS_OK;
	return true;
}

/**/

loadstore_result save_fileconfig(char *fileconfig_file, const char *magic, UINT version, store_fcfg_func store_func, void *context)
{
	ht_file *f=new ht_file();
	f->init(fileconfig_file, FAM_WRITE, FOM_CREATE);
	if (f->get_error()) {
		f->done();
		delete f;
		return LS_ERROR_WRITE;
	}
	
	/* write file config header */
	config_header h;

	memmove(h.magic, magic, sizeof h.magic);

	char q[16];

	int file_ostream_type = get_config_dword("misc/config format");
	
	sprintf(q, "%04x", version);
	memmove(h.version, q, sizeof h.version);

	sprintf(q, "%02x", file_ostream_type);
	memmove(h.stream_type, q, sizeof h.stream_type);

	f->write(&h, sizeof h);

	/* object stream type */
	ht_object_stream *d = create_object_stream(f, file_ostream_type);
	   
	switch (file_ostream_type) {
		case object_stream_bin:
			break;
		case object_stream_txt:
			f->write((void*)"\n#\n#\tThis is a generated file!\n#\n", 33);
			break;
	}
	/* write config */
	store_func(d, context);

	d->done();
	delete d;

	f->done();
	delete f;
		
	return LS_OK;
}

loadstore_result load_fileconfig(char *fileconfig_file, const char *magic, UINT version, load_fcfg_func load_func, void *context, int *error_info)
{
	ht_file *f=new ht_file();
	f->init(fileconfig_file, FAM_READ, FOM_EXISTS);
	switch (f->get_error()) {
		case 0:break;
		case STERR_SYSTEM | ENOENT:
			f->done();
			delete f;
			return LS_ERROR_NOT_FOUND;
		default:
			f->done();
			delete f;
			return LS_ERROR_READ;
	}
	/* read file config header */
	config_header h;

	if (f->read(&h, sizeof h)!=sizeof h) {
		f->done();
		delete f;
		return LS_ERROR_MAGIC;
	}
	
	if (memcmp(h.magic, magic, sizeof h.magic)!=0) return LS_ERROR_MAGIC;

	// FIXME: bad code, no conversion errors reported
	if (hexw((char*)h.version) != version) {
		f->done();
		delete f;
		*error_info = hexw((char*)h.version);
		return LS_ERROR_VERSION;
	}
	
	dword object_stream_type=hexb((char*)h.stream_type);

	/* object stream type */
	ht_object_stream *d = create_object_stream(f, object_stream_type);
	if (!d) {
		f->done();
		delete f;
		return LS_ERROR_FORMAT;
	}		
	   
	/* read config */
	if (load_func(d, context)) {
		*error_info = 0;
		if (d->get_error()) {
			if (object_stream_type==object_stream_txt)
				*error_info = ((ht_object_stream_txt*)d)->getErrorLine();
		}
		f->done();
		delete f;
		return LS_ERROR_CORRUPTED;
	}
	
	d->done();
	delete d;

	f->done();
	delete f;
	
	return LS_OK;
}

/*
 *	INIT
 */

bool init_cfg()
{
#if defined(WIN32) || defined(__WIN32__) || defined(MSDOS) || defined(DJGPP)
	char d[1024];	/* FIXME: !!!! */
	sys_dirname(d, appname);
	char *b = "/"SYSTEM_CONFIG_FILE_NAME;
	systemconfig_file = (char*)malloc(strlen(d)+strlen(b)+1);
	strcpy(systemconfig_file, d);
	strcat(systemconfig_file, b);
#else
	char *home = getenv("HOME");
	char *b = "/"SYSTEM_CONFIG_FILE_NAME;
	if (!home) home = "";
	systemconfig_file = (char*)malloc(strlen(home)+strlen(b)+1);
	strcpy(systemconfig_file, home);
	strcat(systemconfig_file, b);
#endif
	return true;
}

/*
 *	DONE
 */

void done_cfg()
{
	free(systemconfig_file);
}
