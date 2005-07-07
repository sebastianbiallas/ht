/*
 *	HT Editor
 *	vfs.cc
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

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
	
#include "htctrl.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htreg.h"
#include "htsys.h"
#include "httag.h"
#include "htstring.h"
#include "snprintf.h"
#include "store.h"
#include "tools.h"
#include "vfs.h"

static bool unix_is_path_delim(char c)
{
	return c == '/';
}

/*
 *	class LocalFs
 */

void LocalFs::init()
{
	Vfs::init();
}

void LocalFs::done()
{
	Vfs::done();
}

int LocalFs::canonicalize(char *result, const char *filename, const char *cwd)
{
	sys_common_canonicalize(result, filename, cwd, sys_is_path_delim);
	return 0;
}

int LocalFs::compareFilenames(const char *a, const char *b)
{
	/* FIXME: backslash & slash */
	if (strcmp(a, "..")==0) return -1;
	if (strcmp(b, "..")==0) return 1;
	return ht_stricmp(a, b);
}

int LocalFs::createFile(const char *filename, uint createtype)
{
	// FIXME
	return ENOSYS;
}

int LocalFs::deleteFile(const char *filename)
{
	/* filename must be absolute */
	if ((filename[0] != '/') && (filename[0] != '\\') &&
	((filename[1]!=':') && (filename[2]!='/') && (filename[2]!='\\')))
		return ENOENT;
	return remove(filename);
}

void *LocalFs::enumFiletype(uint *type, char **name, void *handle)
{
	*type = 0;
	*name = "file";
	return NULL;
}

bool LocalFs::findFirst(const char *dirname, pfind_t *f)
{
	return (sys_findfirst(dirname, f)==0);
}

bool LocalFs::findNext(pfind_t *f)
{
	return (sys_findnext(f)==0);
}

bool LocalFs::findClose(pfind_t *f)
{
	return (sys_findclose(f)==0);
}

int LocalFs::getCaps()
{
	return VFSCAP_WRITABLE;
}

const char *LocalFs::getProtoName()
{
	return "local";
}

is_path_delim LocalFs::isPathDelim()
{
	return sys_is_path_delim;
}

int LocalFs::makeDir(const char *dirname)
{
	return ENOSYS;
//	return mkdir(dirname, S_IWUSR);
}

int LocalFs::open(const char *filename, bool edit)
{
	return ENOSYS;
}

int LocalFs::pstat(pstat_t *s, const char *filename)
{
	return sys_pstat(s, filename);
}

int LocalFs::renameFile(const char *filename, const char *newname)
{
	return rename(filename, newname);
}

int LocalFs::fileClose(ht_streamfile *f)
{
	f->done();
	int e=f->get_error();
	delete f;
	return e;
}

int LocalFs::fileOpen(const char *filename, uint access_mode, uint open_mode, ht_streamfile **f)
{
	ht_file *file=new ht_file();
	file->init(filename, access_mode, open_mode);
	int e=file->get_error();
	if (e) {
		file->done();
		delete file;
		return e;
	}
	*f=file;
	return 0;
}
	
/*
 *	class RegNodeFile
 */

#define REGNODE_FILE_MAGIC	"HTRG"

void RegNodeFile::init(const char *nn, uint am, uint om)
{
	access_mode0 = am;
	ht_mem_file::init(0, 1024, am);
	open_mode = om;
	nodename = ht_strdup(nn);
	if ((access_mode & FAM_READ) && (access_mode & FAM_WRITE)) {
		set_error(EINVAL);
		return;
	}
	
	ht_registry_node_type type;
	ht_registry_data *data;
	if (!(open_mode & FOM_CREATE)) {
		if (!registry->find_data_entry(nodename, &data, &type, false)) {
			set_error(ENOENT);
			return;
		}
	}
	if (access_mode & FAM_READ) {
		if (open_mode & FOM_CREATE) {
			set_error(EINVAL);
			return;
		}
		ht_object_stream_bin *o = new ht_object_stream_bin();
		o->init(this);
			
		store_node(o, type, data);
			
		o->done();
		delete o;

		seek(0);
	}
}

void RegNodeFile::done()
{
	if (access_mode & FAM_WRITE) {
		ht_registry_node_type type;
		ht_registry_data *data;

		seek(0);

		ht_object_stream_bin *o=new ht_object_stream_bin();
		o->init(this);

		int e=load_node(o, &type, &data);
		
		if (e==0) {
			if (open_mode & FOM_CREATE) {
				if ((e = registry->create_node(nodename, type))) {
					set_error(e);
				}
			}
			registry->set_node(nodename, type, data);

			htmsg m;
			m.msg = msg_config_changed;
			m.type = mt_broadcast;
			app->sendmsg(&m);
		} else set_error(e);

		o->done();
		delete o;
	}
	free(nodename);
	ht_mem_file::done();
}

int RegNodeFile::load_node(ObjectStream &s, ht_registry_node_type *type, ht_registry_data **data)
{
	byte magic[4];
	int n = s->read(magic, sizeof magic);
	if ((n != sizeof magic) || memcmp(magic, REGNODE_FILE_MAGIC, 4)==0) {
		*type = s->getIntDec(4, NULL);
		*data = (ht_registry_data*)s->getObject(NULL);
		return s->get_error();
	}
	
	ht_mem_file *g = new ht_mem_file();
	g->init();
	g->write(magic, n);
	s->copy_to(g);
	ht_registry_data_raw *d = new ht_registry_data_raw(g->bufptr(), g->get_size());
	g->done();
	delete g;
	
	*type=RNT_RAW;
	*data=d;
	return s->get_error();
}

void	RegNodeFile::store_node(ObjectStream &s, ht_registry_node_type type, ht_registry_data *data)
{
	if (type==RNT_RAW) {
		ht_registry_data_raw *d=(ht_registry_data_raw*)data;
		s->write(d->value, d->size);
	} else {
		s->write((void*)REGNODE_FILE_MAGIC, 4);
		s->putIntDec(type, 4, NULL);
		s->putObject(data, NULL);
	}
}

bool	RegNodeFile::set_access_mode(uint am)
{
	access_mode = access_mode0;
	return (am == access_mode0);
}

/*
 *	class RegistryFs
 */

void RegistryFs::init()
{
	Vfs::init();
	enum_last = NULL;
	enum_dir = NULL;
}

void RegistryFs::done()
{
	Vfs::done();
}

int RegistryFs::canonicalize(char *result, const char *filename, const char *cwd)
{
	ht_registry_data *data;
	ht_registry_node_type type;
	
	sys_common_canonicalize(result, filename, cwd, unix_is_path_delim);
	return registry->find_data_entry(result, &data, &type, 0);
}

void RegistryFs::create_pfind_t(pfind_t *f, const char *key, ht_registry_data *data, ht_registry_node_type type)
{
	f->name = key;
	f->stat.caps = pstat_mode_type | pstat_desc;
	create_pstat_t(&f->stat, data, type);
	data->strvalue(f->stat.desc);		/* FIXME: possible buffer overflow !!! only 32 bytes... */
}

void RegistryFs::create_pstat_t(pstat_t *s, ht_registry_data *data, ht_registry_node_type type)
{
	s->caps = pstat_mode_type | pstat_desc;
	s->mode = 0;
	switch (type) {
		case RNT_SUBDIR:
			s->mode |= HT_S_IFDIR;
			break;
		case RNT_SYMLINK:
			s->mode |= HT_S_IFLNK;
			break;
		case RNT_RAW:
			s->caps |= pstat_size;
			s->size = ((ht_registry_data_raw *)data)->size;
		default:
			s->mode |= HT_S_IFREG;
	}
}

int RegistryFs::createFile(const char *filename, uint createtype)
{
	int e=registry->create_node(filename, createtype);
	htmsg m;
	m.msg=msg_config_changed;
	m.type=mt_broadcast;
	app->sendmsg(&m);
	return e;
}

int RegistryFs::deleteFile(const char *filename)
{
	int e=registry->delete_node(filename);
	htmsg m;
	m.msg=msg_config_changed;
	m.type=mt_broadcast;
	app->sendmsg(&m);
	return e;
}

void *RegistryFs::enumFiletype(uint *type, char **name, void *handle)
{
	ht_data_string *key = (ht_data_string*)handle;
	ht_registry_node_type_desc *value;
	do {
		key = (ht_data_string *)registry->node_types->enum_next((ht_data**)&value, key);
	} while (value && (value->type==RNT_SUBDIR));
	if (key && value) {
		*type=value->type;
		*name=key->value;
		return key;
	}
	return NULL;
}

int RegistryFs::compareFilenames(const char *a, const char *b)
{
	if (strcmp(a, "..")==0) return -1;
	if (strcmp(b, "..")==0) return 1;
	return strcmp(a, b);
}

bool RegistryFs::findFirst(const char *dirname, pfind_t *f)
{
	const char *key;
	ht_registry_data *data;
	ht_registry_node_type type;
	
	if (enum_last) free(enum_last);
	enum_last = NULL;
	if (enum_dir) free(enum_dir);
	enum_dir = NULL;
	if ((strcmp(dirname, "/")==0) || (strcmp(dirname, "")==0)){
		if ((key = registry->enum_next(&data, &type, dirname, NULL))) {
			enum_last = strdup(key);
			enum_dir = strdup(dirname);
			create_pfind_t(f, key, data, type);
			return true;
		}
	} else {
		enum_dir = strdup(dirname);
		f->name="..";
		f->stat.caps=pstat_mode_type;
		f->stat.mode=HT_S_IFDIR;
		return true;
	}
	return false;
}

bool RegistryFs::findNext(pfind_t *f)
{
	const char *key;
	ht_registry_data *data;
	ht_registry_node_type type;
	
	if ((key = registry->enum_next(&data, &type, enum_dir, enum_last))) {
		if (enum_last) free(enum_last);
		enum_last = strdup(key);
		create_pfind_t(f, key, data, type);
		return true;
	}
	return false;
}

bool RegistryFs::findClose(pfind_t *f)
{
	return true;
}

int RegistryFs::getCaps()
{
	return VFSCAP_WRITABLE;
}

const char *RegistryFs::getProtoName()
{
	return "reg";
}

is_path_delim RegistryFs::isPathDelim()
{
	return unix_is_path_delim;
}

int RegistryFs::makeDir(const char *dirname)
{
	return registry->create_subdir(dirname);
}

int RegistryFs::open(const char *filename, bool edit)
{
	ht_registry_data *data;
	ht_registry_node_type type;
	
	if (registry->find_data_entry(filename, &data, &type, false)) {
		if (data->editdialog(filename)) {
			htmsg m;
			m.msg=msg_config_changed;
			m.type=mt_broadcast;
			app->sendmsg(&m);
		}
		return 0;
	}
	return ENOSYS;
}

int RegistryFs::pstat(pstat_t *s, const char *filename)
{
	ht_registry_data *data;
	ht_registry_node_type type;

	char key[VFS_DIR_MAX];
	ht_snprintf(key, sizeof key, "%s", filename);
	int l = strlen(key)-1;
	if ((l>=0) && (key[l] == '/')) key[l]=0;
	if (registry->find_any_entry(key, &data, &type)) {
		create_pstat_t(s, data, type);
		return 0;
	}
	return EINVAL;
}

int RegistryFs::renameFile(const char *filename, const char *newname)
{
	return EXDEV;
}

int RegistryFs::fileClose(ht_streamfile *f)
{
	f->done();
	int e=f->get_error();
	delete f;
	return e;
}

int RegistryFs::fileOpen(const char *filename, uint access_mode, uint open_mode, ht_streamfile **f)
{
	RegNodeFile *file=new RegNodeFile();
	file->init(filename, access_mode, open_mode);
	int e=file->get_error();
	if (e) {
		file->done();
		delete file;
		return e;
	}
	*f=file;
	return 0;
}

