/* 
 *	HT Editor
 *	htvfs.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTVFS_H__
#define __HTVFS_H__

#include "htdata.h"
#include "htobj.h"
#include "htreg.h"
#include "htsys.h"

#include "htformat.h"

/*
 *	CLASS ht_regnode_file
 */

class ht_regnode_file: public ht_mem_file {
protected:
	char *nodename;
     UINT access_mode0;
     UINT open_mode;
	
/* overwritten */
		   int load_node(ht_object_stream *s, ht_registry_node_type *type, ht_registry_data **data);
		   void store_node(ht_object_stream *s, ht_registry_node_type type, ht_registry_data *data);
public:
		   void init(const char *nodename, UINT am, UINT om);
	virtual void done();
/* overwritten */
	virtual bool set_access_mode(UINT access_mode);
};

/*
 *	CLASS ht_vfs
 */

#define VFSCAP_WRITABLE		1

class ht_vfs: public Object {
public:
/* new */
	virtual	int canonicalize(char *result, const char *filename, const char *cwd);
	virtual	int create_file(const char *filename, UINT createtype);
	virtual	int delete_file(const char *filename);
	virtual	void *enum_filetype(UINT *type, char **name, void *handle);
	virtual	int filename_compare(const char *a, const char *b);
	virtual	bool findfirst(const char *dirname, pfind_t *f);
	virtual	bool findnext(pfind_t *f);
	virtual	bool findclose(pfind_t *f);
	virtual	int get_caps();
	virtual	char *get_protocol_name();
	virtual	int makedir(const char *dirname);
	virtual	int open(const char *filename, bool edit);
	virtual	int pstat(pstat_t *s, const char *filename);
	virtual	int rename_file(const char *filename, const char *newname);
	virtual	int streamfile_close(ht_streamfile *f);
	virtual	int streamfile_open(const char *filename, UINT access_mode, UINT open_mode, ht_streamfile **f);
};

/*
 *	CLASS ht_file_vfs
 */

class ht_file_vfs: public ht_vfs {
public:
			void init();
	virtual	void done();
/* overwritten */
	virtual	int canonicalize(char *result, const char *filename, const char *cwd);
	virtual	int delete_file(const char *filename);
	virtual	int filename_compare(const char *a, const char *b);
	virtual	bool findfirst(const char *dirname, pfind_t *f);
	virtual	bool findnext(pfind_t *f);
	virtual	bool findclose(pfind_t *f);
	virtual	int get_caps();
	virtual	char *get_protocol_name();
	virtual	int makedir(const char *dirname);
	virtual	int open(const char *filename, bool edit);
	virtual	int pstat(pstat_t *s, const char *filename);
	virtual	int rename_file(const char *filename, const char *newname);
	virtual	int streamfile_close(ht_streamfile *f);
	virtual	int streamfile_open(const char *filename, UINT access_mode, UINT open_mode, ht_streamfile **f);
};

/*
 *	CLASS ht_reg_vfs
 */

class ht_reg_vfs: public ht_vfs {
protected:
	char *enum_last;
	char *enum_dir;

/* new */
			void create_pfind_t(pfind_t *f, const char *key, ht_registry_data *data, ht_registry_node_type type);
public:
			void init();
	virtual	void done();
/* overwritten */
	virtual	int canonicalize(char *result, const char *filename, const char *cwd);
	virtual	int create_file(const char *filename, UINT createtype);
	virtual	int delete_file(const char *filename);
	virtual	void *enum_filetype(UINT *type, char **name, void *handle);
	virtual	int filename_compare(const char *a, const char *b);
	virtual	bool findfirst(const char *dirname, pfind_t *f);
	virtual	bool findnext(pfind_t *f);
	virtual	bool findclose(pfind_t *f);
	virtual	int get_caps();
	virtual	char *get_protocol_name();
	virtual	int makedir(const char *dirname);
	virtual	int open(const char *filename, bool edit);
	virtual	int pstat(pstat_t *s, const char *filename);
	virtual	int rename_file(const char *filename, const char *newname);
	virtual	int streamfile_close(ht_streamfile *f);
	virtual	int streamfile_open(const char *filename, UINT access_mode, UINT open_mode, ht_streamfile **f);
};

/*
 *	CLASS ht_vfs_viewer_status
 */

class ht_vfs_viewer_status: public ht_group {
protected:
	ht_label *l;
	ht_statictext *url;
public:
			void	init(bounds *b);
/* new */
	virtual	void	setstatus(char *cwd, char *cproto);
};

/*
 *	CLASS ht_vfs_viewer
 */
 
class ht_vfs_sub;

class ht_vfs_viewer: public ht_uformat_viewer {
protected:
	ht_vfs_viewer_status *status;
	ht_vfs_viewer *avfsv;

	virtual	char	*func(UINT i, bool execute);
			void	update_status();
public:
	ht_vfs_sub *vfs_sub;

			void	init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, ht_vfs_viewer_status *status);
/* overwritten */
	virtual	void	config_changed();
	virtual	void	handlemsg(htmsg *msg);
	virtual	void insertsub(ht_sub *sub);
	virtual	int	ref();
/* new */
	virtual	ht_vfs_viewer *get_assoc_vfs_viewer();
	virtual	void set_assoc_vfs_viewer(ht_vfs_viewer *avfsv);
};

/*
 *	CLASS ht_data_direntry
 */

class ht_data_direntry: public ht_data {
public:
	char *name;
	pstat_t stat;
	bool selected;
	
	ht_data_direntry()
	{
		name = NULL;
		stat.caps = 0;
		selected = 0;
	}
	
	~ht_data_direntry()
	{
		if (name) free(name);
	}
};

/*
 *	CLASS ht_vfs_sub
 */

#define VFSV_FORMAT_MAX_LENGTH	64
#define VFSV_FORMAT_PROPERTIES	22

#define SDEM_SET				0
#define SDEM_CLEAR				1
#define SDEM_INVERT				2

class ht_vfs_sub: public ht_sub {
protected:
	ht_list *dir;
	int *dirsort;
	ht_vfs_sub *avfss;
	
	bool case_insensitive_names;

	int display_format_length;
	int display_format[VFSV_FORMAT_MAX_LENGTH];
	int max_prop_width[VFSV_FORMAT_PROPERTIES];
/* new */
			bool chdir(char *dir);
			bool churl(char *url);
			bool create_file();
			char *extract_proto(char **url, char *default_proto);
			ht_vfs *find_vfs(char *proto);
			bool get_cfilename(char **cwd, char **cfilename);
			void make_filename(char *buf, char *wd, char *filename);
			void set_display_format(char *fmt);
			int sortidx(int idx);
			char *translate_prop(char *fmt, int *type);
			void update_max_widths(ht_data_direntry *e);
public:
	char *cproto;
	char *cwd;
	
	ht_vfs *cvfs;
	
			void	init(char *starturl);
	virtual	void	done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
/* new */
			void copy(char *rwd, ht_vfs *rvfs);
			void move(char *rwd, ht_vfs *rvfs);
			char *func(UINT i, bool execute);
			void	refreshdir();
			void	unlink();
			void	makedir(char *name);
	virtual	ht_vfs_sub *get_assoc_vfs_sub();
	virtual	void select_direntry(UINT i, int mode);
	virtual	void set_assoc_vfs_sub(ht_vfs_sub *avfss);
};

#endif /* __HTVFS_H__ */
