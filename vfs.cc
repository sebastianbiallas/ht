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
#include "keyb.h"
#include "htreg.h"
#include "sys.h"
#include "httag.h"
#include "strtools.h"
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
	*name = (char*)"file";
	return NULL;
}

bool LocalFs::findFirst(const char *dirname, pfind_t *f)
{
	return (sys_findfirst(*f, dirname) == 0);
}

bool LocalFs::findNext(pfind_t *f)
{
	return (sys_findnext(*f) == 0);
}

bool LocalFs::findClose(pfind_t *f)
{
	return (sys_findclose(*f) == 0);
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
	return sys_pstat(*s, filename);
}

int LocalFs::renameFile(const char *filename, const char *newname)
{
	return rename(filename, newname);
}

int LocalFs::fileClose(File *f)
{
	delete f;
	return 0;
}

int LocalFs::fileOpen(const char *filename, IOAccessMode access_mode, FileOpenMode open_mode, File **f)
{
	try {
		LocalFile *file=new LocalFile(filename, access_mode, open_mode);
		*f=file;
		return 0;
	} catch (const IOException &e) {
		return EIO;
	}
}
	
/*
 *	class RegNodeFile
 */

#define REGNODE_FILE_MAGIC	"HTRG"

RegNodeFile::RegNodeFile(const char *nn, uint am, uint om)
	: MemoryFile(0, 1024, am)
{
	access_mode0 = am;
	open_mode = om;
	nodename = ht_strdup(nn);
	if ((am & IOAM_READ) && (am & IOAM_WRITE)) {
		throw IOException(EINVAL);
	}
	
	ht_registry_node *node;
	if (!(om & FOM_CREATE)) {
		if (!registry->find_data_entry(nodename, &node, false)) {
			throw IOException(ENOENT);
		}
	}
	if (am & IOAM_READ) {
		if (om & FOM_CREATE) {
			throw IOException(EINVAL);
		}
		ObjectStreamBin o(this, false);

		store_node(o, node);

		seek(0);
	}
}

RegNodeFile::~RegNodeFile()
{
	if (getAccessMode() & IOAM_WRITE) {
		ht_registry_node *node;

		seek(0);

		ObjectStreamBin o(this, false);

		int e = load_node(o, &node);
		
		if (e==0) {
			if (open_mode & FOM_CREATE) {
				if ((e = registry->create_node(nodename, node->type))) {
//					set_error(e);
				}
			}
			registry->set_node(nodename, node->type, node->data);

			htmsg m;
			m.msg = msg_config_changed;
			m.type = mt_broadcast;
			app->sendmsg(&m);
		}
	}
	free(nodename);
}

int RegNodeFile::load_node(ObjectStream &s, ht_registry_node **node)
{
	byte magic[4];
	ht_registry_node_type type;
	ht_registry_data *data;
	int n = s.read(magic, sizeof magic);
	if (n != sizeof magic || memcmp(magic, REGNODE_FILE_MAGIC, 4)==0) {
		type = s.getInt(4, NULL);
		data = s.getObject(NULL);
	} else {
		MemoryFile g;
		g.write(magic, n);
		s.copyAllTo(&g);
		ht_registry_data_raw *d = new ht_registry_data_raw(g.getBufPtr(), g.getSize());

		type = RNT_RAW;
		data = d;
	}
	*node = new ht_registry_node(type, NULL, data);
	return 0;
}

void	RegNodeFile::store_node(ObjectStream &s, ht_registry_node *node)
{
	if (node->type == RNT_RAW) {
		ht_registry_data_raw *d = (ht_registry_data_raw*)node->data;
		s.write(d->value, d->size);
	} else {
		s.write((void*)REGNODE_FILE_MAGIC, 4);
		s.putInt(node->type, 4, NULL);
		s.putObject(node->data, NULL);
	}
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
	ht_registry_node *node;
	
	sys_common_canonicalize(result, filename, cwd, unix_is_path_delim);
	return registry->find_data_entry(result, &node, 0);
}

void RegistryFs::create_pfind_t(pfind_t *f, const ht_registry_node *node)
{
	f->name = node->name;
	f->stat.caps = pstat_mode_type | pstat_desc;
	create_pstat_t(&f->stat, node->data, node->type);
	node->data->strvalue(f->stat.desc);		/* FIXME: possible buffer overflow !!! only 32 bytes... */
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
	int e = registry->create_node(filename, createtype);
	htmsg m;
	m.msg = msg_config_changed;
	m.type = mt_broadcast;
	app->sendmsg(&m);
	return e;
}

int RegistryFs::deleteFile(const char *filename)
{
	int e = registry->delete_node(filename);
	htmsg m;
	m.msg = msg_config_changed;
	m.type = mt_broadcast;
	app->sendmsg(&m);
	return e;
}

void *RegistryFs::enumFiletype(uint *type, char **name, void *handle)
{
	ObjHandle oh = registry->node_types->find((Object*)handle);
	ht_registry_node_type_desc *n;
	while ((n = (ht_registry_node_type_desc *)registry->node_types->get(oh))
		&& n->type == RNT_SUBDIR) {
		oh = registry->node_types->findNext(oh);
	}
	if (n) {
		*type = n->type;
		*name = n->name;
		return n;
	}
	return NULL;
}

int RegistryFs::compareFilenames(const char *a, const char *b)
{
	if (strcmp(a, "..") == 0) return -1;
	if (strcmp(b, "..") == 0) return 1;
	return strcmp(a, b);
}

bool RegistryFs::findFirst(const char *dirname, pfind_t *f)
{
	ht_registry_node *node;
	
	free(enum_dir);
	enum_last = NULL;
	enum_dir = NULL;
	if (strcmp(dirname, "/")==0 || strcmp(dirname, "")==0) {
		if ((node = registry->enum_next(dirname, NULL))) {
			enum_last = node;
			enum_dir = ht_strdup(dirname);
			create_pfind_t(f, node);
			return true;
		}
	} else {
		enum_dir = ht_strdup(dirname);
		f->name = "..";
		f->stat.caps = pstat_mode_type;
		f->stat.mode = HT_S_IFDIR;
		return true;
	}
	return false;
}

bool RegistryFs::findNext(pfind_t *f)
{
	ht_registry_node *node;
	
	if ((node = registry->enum_next(enum_dir, enum_last))) {
		enum_last = node;
		create_pfind_t(f, node);
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
	ht_registry_node *node;
	
	if (registry->find_data_entry(filename, &node, false)) {
		if (node->data->editdialog(filename)) {
			htmsg m;
			m.msg = msg_config_changed;
			m.type = mt_broadcast;
			app->sendmsg(&m);
		}
		return 0;
	}
	return ENOSYS;
}

int RegistryFs::pstat(pstat_t *s, const char *filename)
{
	ht_registry_node *node;

	char key[VFS_DIR_MAX];
	ht_snprintf(key, sizeof key, "%s", filename);
	int l = strlen(key)-1;
	if (l >= 0 && key[l] == '/') key[l] = 0;
	if (registry->find_any_entry(key, &node)) {
		create_pstat_t(s, node->data, node->type);
		return 0;
	}
	return EINVAL;
}

int RegistryFs::renameFile(const char *filename, const char *newname)
{
	return EXDEV;
}

int RegistryFs::fileClose(File *f)
{
	delete f;
	return 0;
}

int RegistryFs::fileOpen(const char *filename, IOAccessMode access_mode, FileOpenMode open_mode, File **f)
{
	try {
		*f = new RegNodeFile(filename, access_mode, open_mode);
		return 0;
	} catch (const IOException &e) {
		return e.mPosixErrno;
	}
}

