/*
 *	HT Editor
 *	vfs.h
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

#ifndef __VFS_H__
#define __VFS_H__

#include "htobj.h"
#include "htreg.h"
#include "stream.h"

#define VFS_DIR_MAX      256
#define VFS_PROTO_MAX	16
#define VFS_URL_MAX		(VFS_DIR_MAX+VFS_PROTO_MAX+1)

/*
 *	class RegNodeFile
 */

class RegNodeFile: public MemoryFile {
protected:
	char *nodename;
	uint access_mode0;
	uint open_mode;

/* overwritten */
		   int		load_node(ObjectStream &s, ht_registry_node **node);
		   void		store_node(ObjectStream &s, ht_registry_node *node);
public:
		RegNodeFile(const char *nodename, uint am, uint om);
	virtual ~RegNodeFile();
};

/*
 *   class Vfs
 */

#define VFSCAP_WRITABLE		1

class Vfs: public Object {
public:
/* new */
	virtual	int		canonicalize(char *result, const char *filename, const char *cwd) = 0;
	virtual	int		createFile(const char *filename, uint createtype) = 0;
	virtual	int		deleteFile(const char *filename) = 0;
	virtual	void *		enumFiletype(uint *type, char **name, void *handle) = 0;
	virtual	int		compareFilenames(const char *a, const char *b) = 0;
	virtual	bool		findFirst(const char *dirname, pfind_t *f) = 0;
	virtual	bool		findNext(pfind_t *f) = 0;
	virtual	bool		findClose(pfind_t *f) = 0;
	virtual	int		getCaps() = 0;
	virtual	const char *	getProtoName() = 0;
	virtual	is_path_delim	isPathDelim() = 0;
	virtual	int		makeDir(const char *dirname) = 0;
	virtual	int		open(const char *filename, bool edit) = 0;
	virtual	int		pstat(pstat_t *s, const char *filename) = 0;
	virtual	int		renameFile(const char *filename, const char *newname) = 0;
	virtual	int		fileClose(File *f) = 0;
	virtual	int		fileOpen(const char *filename, IOAccessMode access_mode, FileOpenMode open_mode, File **f) = 0;
};

/*
 *	class LocalFs
 */

class LocalFs: public Vfs {
public:
		void		init();
	virtual	void		done();
/* overwritten */
	virtual	int		canonicalize(char *result, const char *filename, const char *cwd);
	virtual	int		compareFilenames(const char *a, const char *b);
	virtual	int		createFile(const char *filename, uint createtype);
	virtual	int		deleteFile(const char *filename);
	virtual	void *		enumFiletype(uint *type, char **name, void *handle);
	virtual	bool		findFirst(const char *dirname, pfind_t *f);
	virtual	bool		findNext(pfind_t *f);
	virtual	bool		findClose(pfind_t *f);
	virtual	int		getCaps();
	virtual	const char *	getProtoName();
	virtual	is_path_delim	isPathDelim();
	virtual	int		makeDir(const char *dirname);
	virtual	int		open(const char *filename, bool edit);
	virtual	int		pstat(pstat_t *s, const char *filename);
	virtual	int		renameFile(const char *filename, const char *newname);
	virtual	int		fileClose(File *f);
	virtual	int		fileOpen(const char *filename, IOAccessMode access_mode, FileOpenMode open_mode, File **f);
};                      

/*
 *	class RegistryFs
 */

class RegistryFs: public Vfs {
protected:
	ht_registry_node *enum_last;
	char *enum_dir;

/* new */
		void		create_pfind_t(pfind_t *f, const ht_registry_node *node);
		void		create_pstat_t(pstat_t *s, ht_registry_data *data, ht_registry_node_type type);
public:
		void		init();
	virtual	void		done();
/* overwritten */
	virtual	int		canonicalize(char *result, const char *filename, const char *cwd);
	virtual	int		createFile(const char *filename, uint createtype);
	virtual	int		deleteFile(const char *filename);
	virtual	void *		enumFiletype(uint *type, char **name, void *handle);
	virtual	int		compareFilenames(const char *a, const char *b);
	virtual	bool		findFirst(const char *dirname, pfind_t *f);
	virtual	bool		findNext(pfind_t *f);
	virtual	bool		findClose(pfind_t *f);
	virtual	int		getCaps();
	virtual	const char *	getProtoName();
	virtual	is_path_delim	isPathDelim();
	virtual	int		makeDir(const char *dirname);
	virtual	int		open(const char *filename, bool edit);
	virtual	int		pstat(pstat_t *s, const char *filename);
	virtual	int		renameFile(const char *filename, const char *newname);
	virtual	int		fileClose(File *f);
	virtual	int		fileOpen(const char *filename, IOAccessMode access_mode, FileOpenMode open_mode, File **f);
};                      
#endif /* __VFS_H__ */

