/*
 *	HT Editor
 *	htvfs.cc
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

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
	
#include "htctrl.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htreg.h"
#include "htvfs.h"
#include "httag.h"
#include "htstring.h"
#include "store.h"
#include "tools.h"

bool unix_is_path_delim(char c)
{
	return c == '/';
}

/*
 *	CLASS ht_vfs
 */

int ht_vfs::canonicalize(char *result, const char *filename, const char *cwd)
{
	return ENOSYS;
}

int ht_vfs::create_file(const char *filename, UINT createtype)
{
	return ENOSYS;
}

int ht_vfs::delete_file(const char *filename)
{
	return ENOSYS;
}

void *ht_vfs::enum_filetype(UINT *type, char **name, void *handle)
{
	return NULL;
}

int ht_vfs::filename_compare(const char *a, const char *b)
{
	return 0;
}

bool ht_vfs::findclose(pfind_t *f)
{
	return false;
}

bool ht_vfs::findfirst(const char *dirname, pfind_t *f)
{
	return false;
}

bool ht_vfs::findnext(pfind_t *f)
{
	return false;
}

int ht_vfs::get_caps()
{
	return 0;
}

char *ht_vfs::get_protocol_name()
{
	return NULL;
}

int ht_vfs::makedir(const char *dirname)
{
	return ENOSYS;
}

int ht_vfs::open(const char *filename, bool edit)
{
	return ENOSYS;
}

int ht_vfs::pstat(pstat_t *s, const char *filename)
{
	return ENOSYS;
}

int ht_vfs::rename_file(const char *filename, const char *newname)
{
	return ENOSYS;
}

int ht_vfs::streamfile_close(ht_streamfile *f)
{
	return ENOSYS;
}

int ht_vfs::streamfile_open(const char *filename, UINT access_mode, UINT open_mode, ht_streamfile **f)
{
	return ENOSYS;
}
	
/*
 *	CLASS ht_file_vfs
 */

void ht_file_vfs::init()
{
	ht_vfs::init();
}

void ht_file_vfs::done()
{
	ht_vfs::done();
}

int ht_file_vfs::canonicalize(char *result, const char *filename, const char *cwd)
{
	sys_common_canonicalize(result, filename, cwd, sys_is_path_delim);
	return 0;
}

int ht_file_vfs::delete_file(const char *filename)
{
/* filename must be absolute */
	if ((filename[0] != '/') && (filename[0] != '\\') &&
	((filename[1]!=':') && (filename[2]!='/') && (filename[2]!='\\')))
		return ENOENT;
	return remove(filename);
}

int ht_file_vfs::filename_compare(const char *a, const char *b)
{
/* FIXME: backslash & slash */
	if (strcmp(a, "..")==0) return -1;
	if (strcmp(b, "..")==0) return 1;
	return ht_stricmp(a, b);
}

bool ht_file_vfs::findfirst(const char *dirname, pfind_t *f)
{
	return (sys_findfirst(dirname, f)==0);
}

bool ht_file_vfs::findnext(pfind_t *f)
{
	return (sys_findnext(f)==0);
}

bool ht_file_vfs::findclose(pfind_t *f)
{
	return (sys_findclose(f)==0);
}

int ht_file_vfs::get_caps()
{
	return VFSCAP_WRITABLE;
}

char *ht_file_vfs::get_protocol_name()
{
	return "file";
}
	
int ht_file_vfs::makedir(const char *dirname)
{
	return ENOSYS;
//	return mkdir(dirname, S_IWUSR);
}

int ht_file_vfs::open(const char *filename, bool edit)
{
	return ENOSYS;
}

int ht_file_vfs::pstat(pstat_t *s, const char *filename)
{
	return sys_pstat(s, filename);
}

int ht_file_vfs::rename_file(const char *filename, const char *newname)
{
	return rename(filename, newname);
}

int ht_file_vfs::streamfile_close(ht_streamfile *f)
{
	f->done();
	int e=f->get_error();
	delete f;
	return e;
}

int ht_file_vfs::streamfile_open(const char *filename, UINT access_mode, UINT open_mode, ht_streamfile **f)
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
 *	CLASS ht_regnode_file
 */

#define REGNODE_FILE_MAGIC	"HTRG"

void ht_regnode_file::init(const char *nn, UINT am, UINT om)
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

void ht_regnode_file::done()
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

int ht_regnode_file::load_node(ht_object_stream *s, ht_registry_node_type *type, ht_registry_data **data)
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

void	ht_regnode_file::store_node(ht_object_stream *s, ht_registry_node_type type, ht_registry_data *data)
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

bool	ht_regnode_file::set_access_mode(UINT am)
{
	access_mode = access_mode0;
	return (am == access_mode0);
}

/*
 *	CLASS ht_reg_vfs
 */

void ht_reg_vfs::init()
{
	ht_vfs::init();
	enum_last = NULL;
	enum_dir = NULL;
}

void ht_reg_vfs::done()
{
	ht_vfs::done();
}

int ht_reg_vfs::canonicalize(char *result, const char *filename, const char *cwd)
{
	ht_registry_data *data;
	ht_registry_node_type type;
	
	sys_common_canonicalize(result, filename, cwd, unix_is_path_delim);
	return registry->find_data_entry(result, &data, &type, 0);
}

void ht_reg_vfs::create_pfind_t(pfind_t *f, const char *key, ht_registry_data *data, ht_registry_node_type type)
{
// FIXME: dunno what to do instead of typecast (drank alcohol...)
	f->name = (char*)key;
	f->stat.caps = pstat_mode_type | pstat_desc;
	f->stat.mode = 0;
	switch (type) {
		case RNT_SUBDIR:
			f->stat.mode |= HT_S_IFDIR;
			break;
		case RNT_SYMLINK:
			f->stat.mode |= HT_S_IFLNK;
			break;
		case RNT_RAW:
			f->stat.caps |= pstat_size;
			f->stat.size = ((ht_registry_data_raw *)data)->size;
		default:
			f->stat.mode |= HT_S_IFREG;
	}
	data->strvalue(f->stat.desc);		/* FIXME: possible buffer overflow !!! only 32 bytes... */
}

int ht_reg_vfs::create_file(const char *filename, UINT createtype)
{
	int e=registry->create_node(filename, createtype);
	htmsg m;
	m.msg=msg_config_changed;
	m.type=mt_broadcast;
	app->sendmsg(&m);
	return e;
}

int ht_reg_vfs::delete_file(const char *filename)
{
	int e=registry->delete_node(filename);
	htmsg m;
	m.msg=msg_config_changed;
	m.type=mt_broadcast;
	app->sendmsg(&m);
	return e;
}

void *ht_reg_vfs::enum_filetype(UINT *type, char **name, void *handle)
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

int ht_reg_vfs::filename_compare(const char *a, const char *b)
{
	if (strcmp(a, "..")==0) return -1;
	if (strcmp(b, "..")==0) return 1;
	return strcmp(a, b);
}

bool ht_reg_vfs::findfirst(const char *dirname, pfind_t *f)
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

bool ht_reg_vfs::findnext(pfind_t *f)
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

bool ht_reg_vfs::findclose(pfind_t *f)
{
	return true;
}

int ht_reg_vfs::get_caps()
{
	return VFSCAP_WRITABLE;
}

char *ht_reg_vfs::get_protocol_name()
{
	return "reg";
}

int ht_reg_vfs::makedir(const char *dirname)
{
	return registry->create_subdir(dirname);
}

int ht_reg_vfs::open(const char *filename, bool edit)
{
	ht_registry_data *data;
	ht_registry_node_type type;
	
	if (registry->find_data_entry(filename, &data, &type, 0)) {
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

int ht_reg_vfs::pstat(pstat_t *s, const char *filename)
{
	return ENOSYS;
}

int ht_reg_vfs::rename_file(const char *filename, const char *newname)
{
	return EXDEV;
}

int ht_reg_vfs::streamfile_close(ht_streamfile *f)
{
	f->done();
	int e=f->get_error();
	delete f;
	return e;
}

int ht_reg_vfs::streamfile_open(const char *filename, UINT access_mode, UINT open_mode, ht_streamfile **f)
{
	ht_regnode_file *file=new ht_regnode_file();
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
 *	CLASS ht_vfs_viewer
 */
 
void ht_vfs_viewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, ht_vfs_viewer_status *s)
{
	ht_uformat_viewer::init(b, desc, caps, file, format_group);
	status=s;
	avfsv=NULL;
	vfs_sub=NULL;
}

void ht_vfs_viewer::config_changed()
{
	ht_uformat_viewer::config_changed();
	sendsubmsg(msg_dirtyview);
}

char *ht_vfs_viewer::func(UINT i, bool execute)
{
	htmsg m;
	m.msg=execute ? msg_funcexec : msg_funcquery;
	m.type=mt_broadcast;
	m.data1.integer=i;
	
	sendsubmsg(&m);

	if (m.msg==msg_retval) {
		return m.data1.str;
	} else if (m.msg==msg_empty) {
		return NULL;
	}
	
	return ht_uformat_viewer::func(i, execute);
}

ht_vfs_viewer *ht_vfs_viewer::get_assoc_vfs_viewer()
{
	return avfsv;
}

void ht_vfs_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_complete_init:
			ht_uformat_viewer::handlemsg(msg);
			update_status();
			return;
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_Return:
					ref();
					clearmsg(msg);
					return;
				case K_Insert: {
					viewer_pos p;
					if (get_current_pos(&p)) {
						vfs_sub->select_direntry(p.u.line_id.id1, SDEM_INVERT);
						cursor_down(1);
						clearmsg(msg);
					}
					return;
				}
			}
			break;
	}
	return ht_uformat_viewer::handlemsg(msg);
}

void ht_vfs_viewer::insertsub(ht_sub *sub)
{
	ht_uformat_viewer::insertsub(sub);
	vfs_sub=(ht_vfs_sub*)sub;
}

int ht_vfs_viewer::ref()
{
	int r = ht_uformat_viewer::ref();
	update_status();
	return r;
}

void ht_vfs_viewer::set_assoc_vfs_viewer(ht_vfs_viewer *a)
{
	avfsv = a;
	vfs_sub->set_assoc_vfs_sub(a->vfs_sub);
}

void ht_vfs_viewer::update_status()
{
/* FIXME: cwd and cproto shouldn't be public, we should use
   messaging when available for ht_sub */
	if (cursor.sub) {
		char *cproto=((ht_vfs_sub*)cursor.sub)->cproto;
		char *cwd=((ht_vfs_sub*)cursor.sub)->cwd;
		if (status && cwd) status->setstatus(cwd, cproto);
	}
}

/*
 *	CLASS ht_vfs_viewer_status
 */
 
void ht_vfs_viewer_status::init(bounds *b)
{
	ht_group::init(b, 0, 0);

	b->y=0;
	b->h=1;
	bounds c;

	c=*b;
	c.x=0;
//	c.w=10;
	url=new ht_statictext();
	url->init(&c, "?", align_left, false);
	url->growmode = MK_GM(GMH_FIT, GMV_TOP);
	insert(url);
}

void	ht_vfs_viewer_status::setstatus(char *cwd, char *cproto)
{
	int lcwd = strlen(cwd);
	int lcproto = strlen(cproto);
	char *u = (char*)malloc(lcwd+3+lcproto+1);
	sprintf(u, "%s://%s", cproto, cwd);
	url->settext(u);
	url->dirtyview();
	free(u);
}
			 
/*
 *	CLASS ht_vfs_sub
 */

#define VFSV_FORMAT_NAME		0
#define VFSV_FORMAT_SIZE		1
#define VFSV_FORMAT_BSIZE	2
#define VFSV_FORMAT_TYPE		3
#define VFSV_FORMAT_MTIME	4
#define VFSV_FORMAT_ATIME	5
#define VFSV_FORMAT_CTIME	6
#define VFSV_FORMAT_PERM		7
#define VFSV_FORMAT_MODE		8
#define VFSV_FORMAT_NLINK	9
#define VFSV_FORMAT_NGID		10
#define VFSV_FORMAT_NUID		11
#define VFSV_FORMAT_OWNER	12
#define VFSV_FORMAT_GROUP	13
#define VFSV_FORMAT_INODE	14
#define VFSV_FORMAT_SPACE	15
#define VFSV_FORMAT_MARK		16
#define VFSV_FORMAT_SEPARATOR	17
#define VFSV_FORMAT_DESC		18
#define VFSV_FORMAT_RMTIME	19
#define VFSV_FORMAT_RATIME	20
#define VFSV_FORMAT_RCTIME	21

char *format_property[VFSV_FORMAT_PROPERTIES]={
	"name",	"size",
	"bsize",	"type",
	"mtime",	"atime",
	"ctime",	"perm",
	"mode",	"nlink",
	"ngid",	"nuid",
	"owner",	"group",
	"inode",	"space",
	"mark",	"|",
	"desc",	"rmtime",
	"ratime",	"rctime"
};

#define MAKE_DISPLAY_FORMAT(type) MAKE_DISPLAY_FORMAT_MIN_WIDTH(type, 0)
#define MAKE_DISPLAY_FORMAT_MIN_WIDTH(type, width) (((width) << 16) | (type))
#define MAKE_DISPLAY_FORMAT_FIXED_WIDTH(type, width) ((((width) | 0x8000) << 16) | (type))

#define GET_DISPLAY_FORMAT_TYPE(dfmt) ((dfmt) & 0xffff)
#define GET_DISPLAY_FORMAT_SIZE(dfmt) (((dfmt)>>16) & 0x7fff)
#define GET_DISPLAY_FORMAT_IS_FIXED_SIZE(dfmt) ((dfmt) & 0x80000000)
#define GET_DISPLAY_FORMAT_IS_MIN_SIZE(dfmt) (!GET_DISPLAY_FORMAT_IS_FIXED_SIZE(dfmt))

ht_vfs *compare_keys_direntry_cvfs;

int compare_keys_direntry(ht_data *key_a, ht_data *key_b)
{
	ht_data_direntry *a=(ht_data_direntry*)key_a;
	ht_data_direntry *b=(ht_data_direntry*)key_b;
	int a_dir=(HT_S_ISDIR(a->stat.mode)!=0);
	int b_dir=(HT_S_ISDIR(b->stat.mode)!=0);
	if (a_dir==b_dir) {
		return compare_keys_direntry_cvfs->filename_compare(a->name, b->name);
	}
	return b_dir-a_dir;
}

void ht_vfs_sub::init(char *starturl)
{
	ht_sub::init(0);

	char *display_format = get_config_string("misc/vfs display format");
	set_display_format(display_format ? display_format : (char*)"name");
	if (display_format) free(display_format);

	dir = new ht_clist();
	((ht_clist*)dir)->init(compare_keys_direntry);
	dirsort = 0;
	cvfs = 0;
	cproto = 0;
	cwd = 0;
	avfss = NULL;
	churl(starturl);
}

void ht_vfs_sub::done()
{
	if (cwd) free(cwd);
	if (cproto) free(cproto);
	if (dirsort) free(dirsort);
	dir->destroy();
	delete dir;
	ht_sub::done();
}

bool ht_vfs_sub::create_file()
{
	bounds b, c;
	app->getbounds(&c);
	b=c;
	b.w/=2;
	b.h/=2;
	b.x=(c.w-b.w)/2;
	b.y=(c.h-b.h)/2;
	c=b;
	ht_dialog *d=new ht_dialog();
	d->init(&b, "create file", FS_TITLE);

	ht_label *l;

	b.x=1;
	b.y=1;
	b.w-=4;
	b.h=1;

	ht_strinputfield *s=new ht_strinputfield();
	s->init(&b, FILENAME_MAX);
	if (cwd) {
		ht_strinputfield_data sd;
		sd.text=(byte*)cwd;
		sd.textlen=strlen(cwd);
		s->databuf_set(&sd);
	}
	d->insert(s);

	b.y--;
	l=new ht_label();
	l->init(&b, "~name", s);
	d->insert(l);
	
	b=c;
	b.x=1;
	b.y=3;
	b.w-=4;
	b.h-=2+b.y;

	ht_text_listbox *tlb = new ht_text_listbox();
	tlb->init(&b);
	
	b.y--;
	b.h=1;
	l=new ht_label();
	l->init(&b, "~type", tlb);
	d->insert(l);
	
	void *h=NULL;
	UINT t;
	char *n;
	while ((h=cvfs->enum_filetype(&t, &n, h))) {
		tlb->insert_str(t, n);
	}
	tlb->update();
	d->insert(tlb);

	if (d->run(false)) {
		struct data {
			ht_strinputfield_data name;
			ht_listbox_data type;
		};
		data dd;
		d->databuf_get(&dd);
		char b[FILENAME_MAX];
		bin2str(b, dd.name.text, dd.name.textlen);
		
		int e=cvfs->create_file(b, dd.type.cursor_id);
		if (e) {
			errorbox("error creating file '%s': %s", b, strerror(e & ~STERR_SYSTEM));
			return false;
		}
		refreshdir();
	}
	
	return true;
}

bool ht_vfs_sub::chdir(char *dir)
{
	char dirname[HT_NAME_MAX];
	if (cvfs->canonicalize(dirname, dir, cwd)==0) {
		if (cwd) free(cwd);
		cwd=strdup(dirname);
		refreshdir();
		if (uformat_viewer) uformat_viewer->goto_offset(0, false);
		return true;
	}
	return false;
}

bool ht_vfs_sub::churl(char *url)
{
/* extract protocol identifier if available */
	if (cproto) free(cproto);
	cproto = extract_proto(&url, "file");
/* find the matching vfs */
	cvfs = find_vfs(cproto);
	if (!cvfs) return false;
/* change current dir */
	return chdir(url);
}

bool ht_vfs_sub::convert_ofs_to_id(FILEOFS offset, LINE_ID *line_id)
{
	if (offset<dir->count()) {
		line_id->id1 = offset;
		line_id->id2 = 0;
		return true;
	}
	return false;
}

char *ht_vfs_sub::extract_proto(char **url, char *default_proto)
{
	char *cp;
	char *p = *url;
	while (isalnum(*p) && *p) p++;
	if (strncmp(p, "://", 3)==0) {
		cp = (char*)malloc(p-*url+1);
		memmove(cp, *url, p-*url);
		cp[p-*url] = 0;
		*url += p-*url+3;
	} else {
		cp = ht_strdup(default_proto);
	}
	return cp;
}

ht_vfs *ht_vfs_sub::find_vfs(char *proto)
{
	UINT k = 0;
	ht_vfs *vfs;
	while ((vfs = (ht_vfs*)virtual_fs_list->get(k++))) {
		if (strcmp(vfs->get_protocol_name(), proto) == 0) {
			return vfs;
		}
	}
	return NULL;
}

void ht_vfs_sub::first_line_id(LINE_ID *line_id)
{
	line_id->id1 = 0;
	line_id->id2 = 0;
}

int vfs_copy(ht_vfs *svfs, char *sfilename, ht_vfs *dvfs, char *dfilename)
{
	int e, f;
	ht_streamfile *src, *dest;
	e = svfs->streamfile_open(sfilename, FAM_READ, FOM_EXISTS, &src);
	if (e) return e;

	e = dvfs->streamfile_open(dfilename, FAM_WRITE, FOM_CREATE, &dest);
	if (!e) {
		src->copy_to(dest);
		e = svfs->streamfile_close(dest);
	}

	f = svfs->streamfile_close(src);
	return e ? e : f;
}

int vfs_move(ht_vfs *svfs, char *sfilename, ht_vfs *dvfs, char *dfilename)
{
	int e;
	if ((strcmp(svfs->get_protocol_name(), dvfs->get_protocol_name()) != 0) ||
	((e = svfs->rename_file(sfilename, dfilename)) == EXDEV)) {
		e = vfs_copy(svfs, sfilename, dvfs, dfilename);
		if (e == 0) {
			e = svfs->delete_file(sfilename);
		}
	}
	return e;
}

void ht_vfs_sub::copy(char *rwd, ht_vfs *rvfs)
{
	char cfilename[260], rfilename[260];
	char *wd, *filename;
	int e=0;
	if (get_cfilename(&wd, &filename)) {
		make_filename(cfilename, wd, filename);
		make_filename(rfilename, rwd, filename);
		if (inputbox("copy", cfilename, rfilename, sizeof rfilename, 0) == button_ok) {
			e=vfs_copy(cvfs, cfilename, rvfs, rfilename);
			avfss->refreshdir();
		}
	} else {
		e=ENOSYS;
	}
	if (e) {
		char *se=ht_strerror(e);
		if (!se) se="?";
		errorbox("couldn't copy (%d):\n%s", e, se);
	}
}

void ht_vfs_sub::move(char *rwd, ht_vfs *rvfs)
{
	char cfilename[260], rfilename[260];
	char *wd, *filename;
	int e=0;
	if (get_cfilename(&wd, &filename)) {
		make_filename(cfilename, wd, filename);
		make_filename(rfilename, rwd, filename);
		if (inputbox("move", cfilename, rfilename, sizeof rfilename, 0) == button_ok) {
			char *rf = rfilename;
/* extract protocol identifier if available */
			char *rproto = extract_proto(&rf, cproto);
/* find the matching vfs */
			ht_vfs *rvfs = find_vfs(rproto);
			if (rvfs) {
				e = vfs_move(cvfs, cfilename, rvfs, rf);
				avfss->refreshdir();
				refreshdir();
			} else e = ENOSYS;
		}
	} else {
		e=ENOSYS;
	}
	if (e) {
		char *se=ht_strerror(e);
		if (!se) se="?";
		errorbox("couldn't copy (%d):\n%s", e, se);
	}
}

char *ht_vfs_sub::func(UINT i, bool execute)
{
	int caps=cvfs->get_caps();
	int rcaps;
	if (avfss) rcaps=avfss->cvfs->get_caps(); else rcaps=0;
	switch (i) {
		case 4:
			if (caps & VFSCAP_WRITABLE) {
				if (execute) {
					create_file();
				}
				return "create";
			} else {
				return "~create";
			}				
		case 5:
			if (avfss && (rcaps & VFSCAP_WRITABLE)) {
				if (execute) {
					copy(avfss->cwd, avfss->cvfs);
				}
				return "copy";
			} else {
				return "~copy";
			}
		case 6:
			if (avfss && (rcaps & VFSCAP_WRITABLE)) {
				if (execute) {
					move(avfss->cwd, avfss->cvfs);
				}
				return "move";
			} else {
				return "~move";
			}
		case 7:
			if (caps & VFSCAP_WRITABLE) {
				if (execute) {
					char dir[261] = "";
					if (inputbox("make directory", "new directory: ", dir, sizeof dir-1) == button_ok) {
						makedir(dir);
					}
				}
				return "mkdir";
			} else {
				return "~mkdir";
			}
		case 8:
			if (caps & VFSCAP_WRITABLE) {
				if (execute) {
					unlink();
				}
				return "delete";
			} else {
				return "~delete";
			}
			break;
	}
	return NULL;
}

ht_vfs_sub *ht_vfs_sub::get_assoc_vfs_sub()
{
	return avfss;
}

bool ht_vfs_sub::getline(char *line, LINE_ID line_id)
{
	ht_data_direntry *e = (ht_data_direntry*)dir->get(sortidx(line_id.id1));
	if (e) {
		if (e->selected) {
			line = tag_make_color(line, VCP(VC_LIGHT(VC_YELLOW), VC_TRANSPARENT));
		}
		for (int i = 0; i < display_format_length; i++) {
			int timei = 0;
			bool alignright = 1;
			bool link_it = 0;
			char *start = line;
			switch (GET_DISPLAY_FORMAT_TYPE(display_format[i])) {
				case VFSV_FORMAT_NAME:
					strcpy(line, e->name);
					line += strlen(e->name);
					alignright = 0;
					link_it = 1;
					break;
				case VFSV_FORMAT_SIZE:
					if (e->stat.caps & pstat_size) {
						line += sprintf(line, "%u", e->stat.size);
					}
					break;
				case VFSV_FORMAT_BSIZE:
					if (HT_S_ISDIR(e->stat.mode)) {
						if (strcmp(e->name, "..")==0) {
							strcpy(line, "<UP-DIR>");
							line+=8;
						} else {
							strcpy(line, "<SUB-DIR>");
							line+=9;
						}
					} else if (e->stat.caps & pstat_size) {
						line+=sprintf(line, "%u", e->stat.size);
					}
					break;
				case VFSV_FORMAT_TYPE:
					if (e->stat.caps & pstat_mode_type) {
						if (HT_S_ISDIR(e->stat.mode)) {
							*(line++)='/';
						} else if (HT_S_ISBLK(e->stat.mode)) {
							*(line++)='+';
						} else if (HT_S_ISCHR(e->stat.mode)) {
							*(line++)='#';
						} else if (HT_S_ISFIFO(e->stat.mode)) {
							*(line++)='|';
						} else if (HT_S_ISLNK(e->stat.mode)) {
							*(line++)='@';
						} else if (HT_S_ISSOCK(e->stat.mode)) {
							*(line++)='=';
						} else if (e->stat.mode & HT_S_IXUSR) {
							*(line++)='*';
						} else {
							*(line++)=' ';
						}
					}
					break;
				case VFSV_FORMAT_MTIME:timei++;
				case VFSV_FORMAT_ATIME:timei++;
				case VFSV_FORMAT_CTIME:timei++;
				case VFSV_FORMAT_RMTIME:timei++;
				case VFSV_FORMAT_RATIME:timei++;
				case VFSV_FORMAT_RCTIME: {
					time_t q;
					bool avail=0;
					bool reltime=0;
								
					switch (timei) {
						case 2: reltime=1;
						case 5:
							if (e->stat.caps & pstat_mtime) {
								q=e->stat.mtime;
								avail=1;
							}
							break;
						case 1: reltime=1;
						case 4:
							if (e->stat.caps & pstat_atime) {
								q=e->stat.atime;
								avail=1;
							}
							break;
						case 0: reltime=1;
						case 3:
							if (e->stat.caps & pstat_ctime) {
								q=e->stat.ctime;
								avail=1;
							}
							break;
					}
					if (avail) {
						tm t = *gmtime(&q);

						time_t ct;
						time(&ct);
						tm c = *gmtime(&ct);
									   
						char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
						if ((reltime) && ((UINT)ct-q<=60*60*24*28)) {
							if ((UINT)ct-q<=60*60*24) {
								line+=sprintf(line, "today ");
							} else if ((UINT)ct-q<=60*60*24*2) {
								line+=sprintf(line, "yestrd");
							} else {
								line+=sprintf(line, "%s-%uday", ((UINT)(ct-q)/(60*60*24)>10) ? "" : " ", (UINT)(ct-q)/(60*60*24));
							}
						} else {
							line+=sprintf(line, "%s %02d", months[t.tm_mon], t.tm_mday);
						}
						if (t.tm_year==c.tm_year) {
							if ((reltime) && ((UINT)ct-q<=60*60*24)) {
								if ((UINT)ct-q<=60*60) {
									line+=sprintf(line, "-%umin", (UINT)(ct-q)/60);
								} else {
									line+=sprintf(line, "-%um:%u", (UINT)(ct-q)/60/60, (UINT)(ct-q)/60%60);
								}
							} else {
								line+=sprintf(line, " %02d:%02d", t.tm_hour, t.tm_min);
							}
						} else {
							line+=sprintf(line, "  %04d", t.tm_year+1900);
						}
					}
					break;
				}
				case VFSV_FORMAT_PERM:
					if (e->stat.caps & pstat_mode_type) {
						*(line++)=HT_S_ISDIR(e->stat.mode) ? 'd' : '-';
					}
					
					if (e->stat.caps & pstat_mode_oth) {
						*(line++)=(e->stat.mode & HT_S_IROTH) ? 'r' : '-';
						*(line++)=(e->stat.mode & HT_S_IWOTH) ? 'w' : '-';
						*(line++)=(e->stat.mode & HT_S_IXOTH) ? 'x' : '-';
					}
					
					if (e->stat.caps & pstat_mode_grp) {
						*(line++)=(e->stat.mode & HT_S_IRGRP) ? 'r' : '-';
						*(line++)=(e->stat.mode & HT_S_IWGRP) ? 'w' : '-';
						*(line++)=(e->stat.mode & HT_S_IXGRP) ? 'x' : '-';
					}
					
					if (e->stat.caps & pstat_mode_usr) {
						*(line++)=(e->stat.mode & HT_S_IRUSR) ? 'r' : '-';
						*(line++)=(e->stat.mode & HT_S_IWUSR) ? 'w' : '-';
						*(line++)=(e->stat.mode & HT_S_IXUSR) ? 'x' : '-';
					}
					break;
				case VFSV_FORMAT_MODE:
					if (e->stat.caps & pstat_mode_all) {
						line+=sprintf(line, "%o", e->stat.mode & ((1<<9)-1));
					}
					break;
				case VFSV_FORMAT_NLINK:
					break;
				case VFSV_FORMAT_NGID:
					if (e->stat.caps & pstat_gid) {
						line+=sprintf(line, "%u", e->stat.gid);
					}
					break;
				case VFSV_FORMAT_NUID:
					if (e->stat.caps & pstat_uid) {
						line+=sprintf(line, "%u", e->stat.uid);
					}
					break;
				case VFSV_FORMAT_OWNER:
					break;
				case VFSV_FORMAT_GROUP:
					break;
				case VFSV_FORMAT_INODE:
					if (e->stat.caps & pstat_inode) {
						line+=sprintf(line, "%u", e->stat.inode);
					}
					break;
				case VFSV_FORMAT_SPACE:
					*(line++)=' ';
					break;
				case VFSV_FORMAT_MARK:
					break;
				case VFSV_FORMAT_SEPARATOR:
					*(line++)='|';
					break;
				case VFSV_FORMAT_DESC:
					if (e->stat.caps & pstat_desc) {
						tag_strcpy(line, e->stat.desc);
						line+=tag_strlen(line);
						alignright=0;
					}
					break;
			}

			*line=0;
			int vvsize=tag_strvlen(start);
			int msize=GET_DISPLAY_FORMAT_SIZE(display_format[i]);
			if (GET_DISPLAY_FORMAT_IS_FIXED_SIZE(display_format[i])) {
				int m=line-start;
				if (m>msize) m=msize;
				if (alignright) {
					memmove(start+msize-m, line-m, m);
					memset(start, ' ', msize-m);
				} else {
					if (msize>line-start) memset(line, ' ', msize-(line-start));
				}
			} else {
				int csize=line-start;
				if (csize>msize) msize=csize;
				int vsize=max_prop_width[GET_DISPLAY_FORMAT_TYPE(display_format[i])]+(line-start)-vvsize;
				if (vsize>msize) msize=vsize;
				if (alignright) {
					memmove(start+msize-(line-start), start, line-start);
					if (msize>line-start) memset(start, ' ', msize-(line-start));
				} else {
					if (msize>line-start) memset(line, ' ', msize-(line-start));
				}
			}
			line=start+msize;
			if (link_it) {
				char p[512], *q;
				q=tag_make_ref_len(p, line_id.id1, 0, 0, 0, start, msize);
				memmove(start, p, q-p);
				line=start+(q-p);
			}
		}
		*line=0;
		return 1;
	}
	return 0;
}

void ht_vfs_sub::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_dirtyview:
			refreshdir();
			break;
		case msg_funcexec:
			if (func(msg->data1.integer, 1)) {
				clearmsg(msg);
				return;
			}
			break;
		case msg_funcquery: {
			char *s=func(msg->data1.integer, 0);
			if (s) {
				msg->msg=msg_retval;
				msg->data1.str=s;
			}
			break;
		}
	}
	ht_sub::handlemsg(msg);
}

void ht_vfs_sub::last_line_id(LINE_ID *line_id)
{
	line_id->id1 = dir->count();
	line_id->id2 = 0;
}

bool ht_vfs_sub::get_cfilename(char **wd, char **filename)
{
	viewer_pos p;
	if (uformat_viewer->get_current_pos(&p)) {
		ht_data_direntry *e = (ht_data_direntry*)dir->get(p.u.line_id.id1);
		if (e) {
			*filename=e->name;
			*wd=cwd;
			return true;
		}
	}
	return false;
}

void ht_vfs_sub::make_filename(char *buf, char *wd, char *filename)
{
	strcpy(buf, wd);
	if (buf[strlen(buf)-1]!='/') strcat(buf, "/");
	strcat(buf, filename);
}

int ht_vfs_sub::next_line_id(LINE_ID *line_id, int n)
{
	int r = n;
	int c = dir->count();
	ID i1 = line_id->id1;
	i1 += n;
	if ((int)i1 > c-1) {
		r -= i1-c+1;
		i1 = c-1;
	}
	if (r) line_id->id1 = i1;
	return r;
}

int ht_vfs_sub::prev_line_id(LINE_ID *line_id, int n)
{
	int r;
	ID i1 = line_id->id1;
	if (i1 < (dword)n) {
		r = i1;
		i1 = 0;
	} else {
		r = n;
		i1 -= n;
	}
	if (r) line_id->id1 = i1;
	return r;
}

bool ht_vfs_sub::ref(LINE_ID *id)
{
	ht_data_direntry *e=(ht_data_direntry*)dir->get(id->id1);
	if (e) {
		char d[256], f[256];	/* FIXME: possible buffer overflow ! */
		int dnl=strlen(cwd);
		strcpy(d, cwd);
		if ((d[dnl-1]!='\\') && (d[dnl-1]!='/')) {
			d[dnl]='/';
			d[dnl+1]=0;
		}
		strcat(d, e->name);
		if (HT_S_ISDIR(e->stat.mode)) {
			bool is_pp=(strcmp(e->name, "..")==0);
			f[0]=0;
			if (is_pp) {
				char *last_slash1 = strrchr(cwd, '/');
				char *last_slash2 = strrchr(cwd, '\\');
				char *last_slash = (last_slash1>last_slash2) ? last_slash1 : last_slash2;
				if (last_slash) {
					last_slash++;
					strcpy(f, last_slash);
				}
			}
			chdir(d);
			if (is_pp) {
				int i=0;
				ht_data_direntry *e;
				while ((e=(ht_data_direntry*)dir->get(i))) {
					if (cvfs->filename_compare(e->name, f)==0) {
						if (uformat_viewer) uformat_viewer->goto_offset(i, false);
						break;
					}
					i++;
				}
			}
		} else {
			cvfs->open(d, 1);
		}
	}
	return 0;
}

void ht_vfs_sub::refreshdir()
{
	dir->empty();
/* stat all entries & calculate max visual width of fields */
	for (int i=0; i<VFSV_FORMAT_PROPERTIES; i++) {
		max_prop_width[i]=0;
	}
/* read dir entries */
	pfind_t f;
	if (cwd && cvfs->findfirst(cwd, &f)) {
		do {
			if (strcmp(f.name, ".")!=0) {
				ht_data_direntry *e=new ht_data_direntry();
				e->name=strdup(f.name);
				e->stat=f.stat;
				dir->insert(e);
				update_max_widths(e);
			}
		} while (cvfs->findnext(&f));
		cvfs->findclose(&f);
	}
/* sort entries */
	compare_keys_direntry_cvfs=cvfs;
	dir->sort();
	if (uformat_viewer) uformat_viewer->sendmsg(msg_dirtyview);
}

void ht_vfs_sub::set_display_format(char *fmt)
{
	int type;
	display_format_length=0;
	while ((fmt=translate_prop(fmt, &type))) {
		if (*fmt==':') {
			fmt++;
			unsigned int width=strtoul(fmt, &fmt, 10);
			if (*fmt=='+') {
				fmt++;
				display_format[display_format_length]=MAKE_DISPLAY_FORMAT_MIN_WIDTH(type, width);
			} else {
				display_format[display_format_length]=MAKE_DISPLAY_FORMAT_FIXED_WIDTH(type, width);
			}
		} else {
			display_format[display_format_length]=MAKE_DISPLAY_FORMAT(type);
		}

		while ((*fmt==',') || (*fmt==' ')) fmt++;
		if (++display_format_length==VFSV_FORMAT_MAX_LENGTH) break;
	}
}

void ht_vfs_sub::select_direntry(UINT i, int mode)
{
	ht_data_direntry *e = (ht_data_direntry*)dir->get(i);
	if (!e) return;
	switch (mode) {
		case SDEM_SET:
			e->selected = true;
			break;
		case SDEM_CLEAR:
			e->selected = false;
			break;
		case SDEM_INVERT:
			e->selected = !e->selected;
			break;
	}
}

void ht_vfs_sub::set_assoc_vfs_sub(ht_vfs_sub *a)
{
	avfss=a;
}

int ht_vfs_sub::sortidx(int idx)
{
	if (dirsort) return dirsort[idx]; else return idx;
}

char *ht_vfs_sub::translate_prop(char *fmt, int *type)
{
	for (int i=0; i<VFSV_FORMAT_PROPERTIES; i++) {
		int l=strlen(format_property[i]);
		if (strncmp(fmt, format_property[i], l)==0) {
			*type=i;
			return fmt+l;
		}
	}
	return 0;
}

void ht_vfs_sub::unlink()
{
	char filename[260];

	viewer_pos p;
	uformat_viewer->get_current_pos(&p);
	ht_data_direntry *e=(ht_data_direntry*)dir->get(p.u.line_id.id1);
	
	char *wd, *fn;
	if (!get_cfilename(&wd, &fn)) return;
	
	if (strcmp(fn, "..")==0) {
		errorbox("can't delete \"..\" !");
		return;
	}
	
	make_filename(filename, wd, fn);
	
	if (HT_S_ISDIR(e->stat.mode)) {
		if (confirmbox("Really delete directory '%s' ?", filename)==button_ok) {
			int e=cvfs->delete_file(filename);
			if (e) {
				char *se=ht_strerror(e);
				if (!se) se="?";
				errorbox("couldn't delete directory '%s', (%d):\n%s", filename, e, se);
			}
			refreshdir();
		}
	} else {
		char *t=NULL;
		if (HT_S_ISREG(e->stat.mode)) {
			t="file";
		} else if (HT_S_ISLNK(e->stat.mode)) {
			t="symbolic link";
		} else if (HT_S_ISBLK(e->stat.mode)) {
			t="block device";
		} else if (HT_S_ISCHR(e->stat.mode)) {
			t="character device";
		} else if (HT_S_ISFIFO(e->stat.mode)) {
			t="fifo";
		} else if (HT_S_ISSOCK(e->stat.mode)) {
			t="socket";
		}
		if (t) {
			if (confirmbox("Really delete %s '%s' ?", t, filename)==button_ok) {
				int e=cvfs->delete_file(filename);
				if (e) {
					char *se=ht_strerror(e);
					if (!se) se="?";
					errorbox("couldn't delete %s '%s', (%d):\n%s", t, filename, e, se);
				}
				refreshdir();
			}
		} else {
			errorbox("unlink() not supported for this type of file...");
		}
	}
}

void ht_vfs_sub::makedir(char *name)
{
	if (name) {
		char dirname[260];
		strcpy(dirname, cwd);
		strcat(dirname, "/");
		strcat(dirname, name);
		int e=cvfs->makedir(dirname);
		if (e) {
			char *se=ht_strerror(e);
			if (!se) se="?";
			errorbox("couldn't create directory (%d):\n%s", e, se);
		}
		refreshdir();
	}
}


int intstrlen(int a)
{
	int l=a ? 1 : 0;
	while (a/=10) l++;
	return l;
}

void ht_vfs_sub::update_max_widths(ht_data_direntry *e)
{
#define SATURATE(a, b) { int c=b; if (c>a) a=c; }
	SATURATE(max_prop_width[VFSV_FORMAT_NAME], tag_strlen(e->name));
	if (e->stat.caps & pstat_size) SATURATE(max_prop_width[VFSV_FORMAT_SIZE], intstrlen(e->stat.size));
	if (e->stat.caps & pstat_mode_type) max_prop_width[VFSV_FORMAT_TYPE]=1;
	if ((e->stat.caps & pstat_size) || (HT_S_ISDIR(e->stat.mode))) {
		// FIXME: involve strings somehow ("<UP-DIR>", "<SUB-DIR>")
		int q=intstrlen(e->stat.size);
		max_prop_width[VFSV_FORMAT_BSIZE]=MAX(q, 9);
	}
	
	if (e->stat.caps & pstat_mtime) max_prop_width[VFSV_FORMAT_MTIME]=12;
	if (e->stat.caps & pstat_atime) max_prop_width[VFSV_FORMAT_ATIME]=12;
	if (e->stat.caps & pstat_ctime) max_prop_width[VFSV_FORMAT_CTIME]=12;
	if (e->stat.caps & pstat_mtime) max_prop_width[VFSV_FORMAT_RMTIME]=12;
	if (e->stat.caps & pstat_atime) max_prop_width[VFSV_FORMAT_RATIME]=12;
	if (e->stat.caps & pstat_ctime) max_prop_width[VFSV_FORMAT_RCTIME]=12;
	
	max_prop_width[VFSV_FORMAT_PERM]=0;
	if (e->stat.caps & pstat_mode_type) max_prop_width[VFSV_FORMAT_PERM]++;
	if (e->stat.caps & pstat_mode_usr) max_prop_width[VFSV_FORMAT_PERM]+=3;
	if (e->stat.caps & pstat_mode_grp) max_prop_width[VFSV_FORMAT_PERM]+=3;
	if (e->stat.caps & pstat_mode_oth) max_prop_width[VFSV_FORMAT_PERM]+=3;
	
	if (e->stat.caps & pstat_mode_all) max_prop_width[VFSV_FORMAT_MODE]=3;
//	SATURATE(max_prop_width[VFSV_FORMAT_NLINK], intstrlen(e->stat.size));
	if (e->stat.caps & pstat_gid) SATURATE(max_prop_width[VFSV_FORMAT_NGID], intstrlen(e->stat.gid));
	if (e->stat.caps & pstat_uid) SATURATE(max_prop_width[VFSV_FORMAT_NUID], intstrlen(e->stat.uid));
// FIXME: owner...
// FIXME: group...
	if (e->stat.caps & pstat_inode) SATURATE(max_prop_width[VFSV_FORMAT_INODE], intstrlen(e->stat.inode));
// FIXME: space...
	max_prop_width[VFSV_FORMAT_MARK]=1;
	if (e->stat.caps & pstat_desc) SATURATE(max_prop_width[VFSV_FORMAT_DESC], tag_strvlen(e->stat.desc));
}

