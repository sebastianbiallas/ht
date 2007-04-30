/*
 *	HT Editor
 *	vfsview.cc
 *
 *	Copyright (C) 1999-2003 Stefan Weyergraf
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
#include <string.h>
#include <stdlib.h>

#include "strtools.h"
#include "snprintf.h"
#include "vfsview.h"

static vfs_extra *make_vfs_extra(const char *name, pstat_t s)
{
	vfs_extra *e = ht_malloc(sizeof *e);
	e->stat = s;
	e->name = strdup(name);
	return e;
}

static void free_vfs_extra(vfs_extra *e)
{
	free(e->name);
	free(e);
}

static Vfs *vfslistbox_vfs;

static int vfslistbox_fncmp(const char *a, const char *b)
{
	if (strchr("/ *@=#+", *a) && (*b != *a)) {
		if (*a == '/') return -1;
		if (*b == '/') return 1;
		return *b-*a;
	}
	return vfslistbox_vfs->compareFilenames(a,b);
}

/*
 *	class VfsListbox
 */

#define VFSV_FORMAT_NAME	0
#define VFSV_FORMAT_SIZE	1
#define VFSV_FORMAT_BSIZE	2
#define VFSV_FORMAT_TYPE	3
#define VFSV_FORMAT_MTIME	4
#define VFSV_FORMAT_ATIME	5
#define VFSV_FORMAT_CTIME	6
#define VFSV_FORMAT_PERM	7
#define VFSV_FORMAT_MODE	8
#define VFSV_FORMAT_NLINK	9
#define VFSV_FORMAT_NGID	10
#define VFSV_FORMAT_NUID	11
#define VFSV_FORMAT_OWNER	12
#define VFSV_FORMAT_GROUP	13
#define VFSV_FORMAT_INODE	14
#define VFSV_FORMAT_SPACE	15
#define VFSV_FORMAT_MARK	16
#define VFSV_FORMAT_SEPARATOR	17
#define VFSV_FORMAT_DESC	18
#define VFSV_FORMAT_RMTIME	19
#define VFSV_FORMAT_RATIME	20
#define VFSV_FORMAT_RCTIME	21

static const char *format_property[VFSV_FORMAT_PROPERTIES]={
	"name",		"size",
	"bsize",	"type",
	"mtime",	"atime",
	"ctime",	"perm",
	"mode",		"nlink",
	"ngid",		"nuid",
	"owner",	"group",
	"inode",	"space",
	"mark",		"|",
	"desc",		"rmtime",
	"ratime",	"rctime"
};

#define MAKE_DISPLAY_FORMAT(type) MAKE_DISPLAY_FORMAT_MIN_WIDTH(type, 0)
#define MAKE_DISPLAY_FORMAT_MIN_WIDTH(type, width) (((width) << 16) | (type))
#define MAKE_DISPLAY_FORMAT_FIXED_WIDTH(type, width) ((((width) | 0x8000) << 16) | (type))

#define GET_DISPLAY_FORMAT_TYPE(dfmt) ((dfmt) & 0xffff)
#define GET_DISPLAY_FORMAT_SIZE(dfmt) (((dfmt)>>16) & 0x7fff)
#define GET_DISPLAY_FORMAT_IS_FIXED_SIZE(dfmt) ((dfmt) & 0x80000000)
#define GET_DISPLAY_FORMAT_IS_MIN_SIZE(dfmt) (!GET_DISPLAY_FORMAT_IS_FIXED_SIZE(dfmt))

void VfsListbox::init(Bounds *b, Container *vl, ht_text *sp)
{
	cvfs = NULL;
	show_pos = sp;
	cdir[0] = 0;
	cproto[0] = 0;
	vfs_list = vl;
	ht_itext_listbox::init(b);
	config_changed();
}

void VfsListbox::done()
{
	ht_itext_listbox::done();
}

int VfsListbox::changeDir(const char *dir)
{
	char url[VFS_URL_MAX];
	ht_snprintf(url, sizeof url, "%s:%s", cproto, dir);
	return changeURL(url);
}

int VfsListbox::changeURL(const char *url)
{
	int c = vfs_list->count();
	const char *pend = strchr(url, ':');
	Vfs *newVfs = NULL;
	const char *pathptr = url;
	if (pend) {
		/* find matching protocol */
		char protoname[VFS_PROTO_MAX+1];
		if (pend-url > VFS_PROTO_MAX) return EINVAL;
		ht_strlcpy(protoname, url, pend-url+1);
		for (int i = 0; i < c; i++) {
			Vfs *v = (Vfs*)(*vfs_list)[i];
			if (strcmp(protoname, v->getProtoName()) == 0) {
				newVfs = v;
				break;
			}
		}
		if (!newVfs) return EINVAL;
		pathptr = pend+1;
	} else {
		/* no proto mentioned, default to first in list */
		if (!c) return EINVAL;
		newVfs = (Vfs*)vfs_list->get(0);
	}
	char path[VFS_DIR_MAX+1];
	if (sys_common_canonicalize(path, pathptr, NULL, newVfs->isPathDelim()) !=0) return EINVAL;

	/* add trailing path delimiter if needed */
	int l = strlen(path)-1;
	if ((l==-1) || !newVfs->isPathDelim()(path[l])) {
		char delim = newVfs->isPathDelim()('/') ? '/' : '\\';
		if (l+2 >= (int)sizeof path) return EINVAL;
		path[l+1] = delim;
		path[l+2] = 0;
	}

	/* stat and find out if its a dir*/
	pstat_t s;
	int e;
	if ((e = newVfs->pstat(&s, path)) != 0) return e;
	if (!(s.caps & pstat_mode_type) || !(HT_S_ISDIR(s.mode))) return ENOTDIR;

	/* code to position cursor when doing "cd .." */
	char spath[VFS_DIR_MAX+1];
	char spath2[VFS_DIR_MAX+1];
	strcpy(spath, cdir);
	char *p = ht_strend(spath)-2;
	bool cdpp = false;
	while (p >= spath) {
		if (newVfs->isPathDelim()(*p)) {
			strcpy(spath2, p+1);
			*(ht_strend(spath2)-1) = 0;
			*(p+1) = 0;
			if (newVfs->compareFilenames(path, spath) == 0) {
				cdpp = true;
			}
			break;
		}
		p--;
	}
	
	/* everything ok, set current to this */
	ht_strlcpy(cproto, newVfs->getProtoName(), sizeof cproto);
	ht_strlcpy(cdir, path, sizeof cdir);
	cvfs = newVfs;

	reread();
	/**/
	update();
	if (dfmt_quickfind != -1) {
		ht_text_listbox_sort_order so[1];
		vfslistbox_vfs = cvfs;
		so[0].col = dfmt_quickfind;
		so[0].compare_func = vfslistbox_fncmp;
		sort(1, so);
	} /*else update();*/
	
	gotoItemByPosition(0);
	/* code to position cursor when doing "cd .." (part II) */
	if (cdpp) {
		ht_text_listbox_item *i = (ht_text_listbox_item*)getFirst();
		while (i) {
			vfs_extra *x = (vfs_extra*)i->extra_data;
			if (newVfs->compareFilenames(x->name, spath2) == 0) {
				gotoItemByEntry(i);
				break;
			}
			i = (ht_text_listbox_item*)getNext(i);
		}
	}
	rearrangeColumns();

	return 0;
}

void VfsListbox::config_changed()
{
	ht_text_listbox::config_changed();
	char *dfmt = get_config_string("misc/vfs display format");
	setDisplayFormat(dfmt ? dfmt : (char*)"name");
	free(dfmt);
}

int VfsListbox::cursorAdjust()
{
	return 1;
}

void VfsListbox::freeExtraData(void *extra_data)
{
	if (extra_data) {
		free_vfs_extra((vfs_extra*)extra_data);
	}
}

const char *VfsListbox::getCurDir()
{
	return cdir;
}

const char *VfsListbox::getCurProto()
{
	return cproto;
}

Vfs *VfsListbox::getCurVfs()
{
	return cvfs;
}

void VfsListbox::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_keypressed:
		switch (msg->data1.integer) {
		case K_Return:
			if (count && selectEntry(e_cursor)) {
				clearmsg(msg);
				return;
			}
			break;
		}
		break;
	}
	return ht_text_listbox::handlemsg(msg);
}

void VfsListbox::setDisplayFormat(char *fmt)
{
/*	int dfmt_cols;
	int dfmt_props;
	int dfmt_prop2colidx[VFSV_FORMAT_MAX_COLS];
	int dfmt_prop[VFSV_FORMAT_MAX_COLS];*/
	int type;
	dfmt_cols = 0;
	dfmt_props = 0;
	dfmt_quickfind = -1;
	while ((fmt = translateProp(fmt, &type))) {
		if (type == VFSV_FORMAT_SEPARATOR) {
			if (++dfmt_cols == VFSV_FORMAT_MAX_COLS) break;
		} else {
			if (*fmt == ':') {
				fmt++;
				unsigned int width = strtoul(fmt, &fmt, 10);
				if (*fmt == '+') {
					fmt++;
					dfmt_prop[dfmt_props] = MAKE_DISPLAY_FORMAT_MIN_WIDTH(type, width);
				} else {
					dfmt_prop[dfmt_props] = MAKE_DISPLAY_FORMAT_FIXED_WIDTH(type, width);
				}
			} else {
				dfmt_prop[dfmt_props] = MAKE_DISPLAY_FORMAT(type);
			}
			dfmt_prop2colidx[dfmt_props] = dfmt_cols;
			if (++dfmt_props == VFSV_FORMAT_MAX_COLS) break;
		}

		while ((*fmt == ',') || (*fmt == ' ')) fmt++;

		if ((type == VFSV_FORMAT_NAME) && (dfmt_quickfind == -1)) {
			dfmt_quickfind = dfmt_cols;
		}
	}
	++dfmt_cols;
	cols = dfmt_cols;
	if (dfmt_quickfind != -1) keycol = dfmt_quickfind;
		else keycol = 0;
	rearrangeColumns();
}

bool VfsListbox::selectEntry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item*)entry;
	if (i->extra_data) {
		vfs_extra *e = (vfs_extra*)i->extra_data;
		if (e->stat.caps & pstat_mode_type) {
			if (HT_S_ISDIR(e->stat.mode)) {
				char d[VFS_DIR_MAX];
				ht_snprintf(d, sizeof d, "%s%s", cdir, e->name);
				changeDir(d);
				update();
				return true;
			}
		}
	}
	return false;
}

char *VfsListbox::translateProp(char *fmt, int *type)
{
	for (int i=0; i<VFSV_FORMAT_PROPERTIES; i++) {
		int l = strlen(format_property[i]);
		if (ht_strncmp(fmt, format_property[i], l)==0) {
			*type = i;
			return fmt+l;
		}
	}
	return 0;
}

void VfsListbox::reread()
{
#define VFSV_FORMAT_MAX_LENGTH 256
	if (!cvfs) return;
	clearAll();
	char *strs[VFSV_FORMAT_MAX_COLS];
	for (int i=0; i<dfmt_cols; i++) {
		strs[i] = ht_malloc(VFSV_FORMAT_MAX_LENGTH);
	}
	pfind_t f;
	int k = 0;
	if (cvfs->findFirst(cdir, &f)) {
		do {
			if (strcmp(f.name, ".") != 0) {
				for (int i=0; i<dfmt_cols; i++) {
					*(strs[i]) = 0;
				}
				for (int i=0; i<dfmt_props; i++) {
					int z = dfmt_prop2colidx[i];
					int l = strlen(strs[z]);
					renderEntry(strs[z]+l, VFSV_FORMAT_MAX_LENGTH-l, dfmt_prop[i], f.name, f.stat);
				}
				insert_str_extra(k++, make_vfs_extra(f.name, f.stat), (const char**)strs);
			}
		} while (cvfs->findNext(&f));
	}
	cvfs->findClose(&f);
	for (int i=0; i<dfmt_cols; i++) {
		free(strs[i]);
	}
}

void *VfsListbox::quickfind(const char *s)
{
	ht_text_listbox_item *item = (ht_text_listbox_item *)e_cursor;
	for (int j=0; j<2; j++) {
		int slen = strlen(s);
		char *i = NULL;
		if (item) {
			i = item->data[keycol];
			i = (i && *i) ? i+1 : i;
		}
		while (item && (compare_strn(i, s, slen)!=0)) {
			item = item->next;
			if (item) {
				i = item->data[keycol];
				i = (i && *i) ? i+1 : i;
			}
		}
		if (item) return item;
		item = first;
	}
	return NULL;
}

char *VfsListbox::quickfindCompletition(const char *s)
{
	ht_text_listbox_item *item = first;
	char *res = NULL;
	int slen = strlen(s);
	char *i = NULL;
	if (item) {
		i = item->data[keycol];
		i = (i && *i) ? i+1 : i;
	}
	while (item) {
		if (compare_strn(i, s, slen)==0) {
			if (!res) {
				res = ht_strdup(item->data[keycol]+1);
			} else {
				int a = compare_ccomm(item->data[keycol]+1, res);
				res[a] = 0;
			}
		}
		item = item->next;
		if (item) {
			i = item->data[keycol];
			i = (i && *i) ? i+1 : i;
		}
	}
	return res;
}

void VfsListbox::update()
{
	if (show_pos) {
		char curl[VFS_URL_MAX];
		ht_snprintf(curl, sizeof curl, "%s:%s", cproto, cdir);
		show_pos->settext(curl);
	}

	ht_text_listbox::update();
}

void VfsListbox::renderEntry(char *buf, int bufsize, int dfmt, const char *filename, pstat_t stat)
{
	buf[0] = 0;
	int timei = 0;
	switch (GET_DISPLAY_FORMAT_TYPE(dfmt)) {
		case VFSV_FORMAT_NAME: {
			ht_snprintf(buf, bufsize, "%s", filename);
			break;
		}
		case VFSV_FORMAT_SIZE:
			if (stat.caps & pstat_size) {
				ht_snprintf(buf, bufsize, "%qu", stat.size);
			}
			break;
		case VFSV_FORMAT_BSIZE:
			if (HT_S_ISDIR(stat.mode)) {
				if (strcmp(filename, "..")==0) {
					ht_snprintf(buf, bufsize,"<UP-DIR>", stat.size);
				} else {
					ht_snprintf(buf, bufsize,"<SUB-DIR>", stat.size);
				}
			} else if (stat.caps & pstat_size) {
				ht_snprintf(buf, bufsize, "%qu", stat.size);
			}
			break;
		case VFSV_FORMAT_TYPE:
			if (bufsize > 1) {
				if (stat.caps & pstat_mode_type) {
					if (HT_S_ISDIR(stat.mode)) {
						buf[0] = '/';
					} else if (HT_S_ISBLK(stat.mode)) {
						buf[0] = '+';
					} else if (HT_S_ISCHR(stat.mode)) {
						buf[0] = '#';
					} else if (HT_S_ISFIFO(stat.mode)) {
						buf[0] = '|';
					} else if (HT_S_ISLNK(stat.mode)) {
						buf[0] = '@';
					} else if (HT_S_ISSOCK(stat.mode)) {
						buf[0] = '=';
					} else if (stat.mode & HT_S_IXUSR) {
						buf[0] = '*';
					} else {
						buf[0] = ' ';
					}
					buf[1] = 0;
				}
			}
			break;
		case VFSV_FORMAT_MTIME: timei++;
		case VFSV_FORMAT_ATIME: timei++;
		case VFSV_FORMAT_CTIME: timei++;
		case VFSV_FORMAT_RMTIME: timei++;
		case VFSV_FORMAT_RATIME: timei++;
		case VFSV_FORMAT_RCTIME: {
			time_t q;
			bool avail = false;
			bool reltime = false;

			switch (timei) {
				case 2: reltime = true;
				case 5:
					if (stat.caps & pstat_mtime) {
						q = stat.mtime;
						avail = true;
					}
					break;
				case 1: reltime = true;
				case 4:
					if (stat.caps & pstat_atime) {
						q = stat.atime;
						avail = true;
					}
					break;
				case 0: reltime = true;
				case 3:
					if (stat.caps & pstat_ctime) {
						q = stat.ctime;
						avail = true;
					}
					break;
			}
			if (avail) {
				tm *pt = gmtime(&q);
				if (!pt) {
					q = 0;
					pt = gmtime(&q);
				}
				tm t = *pt;

				time_t ct;
				time(&ct);
				tm *pc = gmtime(&ct);
				if (!pc) {
					ct = 0;
					pc = gmtime(&ct);
				}
				tm c = *pc;
				char *line = buf;
									   
				const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
				if ((reltime) && ((uint)ct-q<=60*60*24*28)) {
					if ((uint)ct-q<=60*60*24) {
						line += ht_snprintf(line, bufsize-(line-buf), "now ");
					} else if ((uint)ct-q<=60*60*24*2) {
						line += ht_snprintf(line, bufsize-(line-buf), "now ");
					} else {
						line += ht_snprintf(line, bufsize-(line-buf), "%s-%uday", ((uint)(ct-q)/(60*60*24)>10) ? "" : " ", (uint)(ct-q)/(60*60*24));
					}
				} else {
					line += ht_snprintf(line, bufsize-(line-buf), "%s %02d", months[t.tm_mon], t.tm_mday);
				}
				if (t.tm_year==c.tm_year) {
					if ((reltime) && ((uint)ct-q<=60*60*24)) {
						if ((uint)ct-q<=60*60) {
							line += ht_snprintf(line, bufsize-(line-buf), "-%umin", (uint)(ct-q)/60);
						} else {
							line += ht_snprintf(line, bufsize-(line-buf), "-%um:%u", (uint)(ct-q)/60/60, (uint)(ct-q)/60%60);
						}
					} else {
						line += ht_snprintf(line, bufsize-(line-buf), " %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
					}
				} else {
					line += ht_snprintf(line, bufsize-(line-buf), "  %04d", t.tm_year+1900);
					line += ht_snprintf(line, bufsize-(line-buf), " %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
				}
			}
			break;
		}
		case VFSV_FORMAT_PERM: {
			if (bufsize>3) {
				char *line = buf;
				if (stat.caps & pstat_mode_type) {
					*(line++)=HT_S_ISDIR(stat.mode) ? 'd' : '-';
				}
					
				if (stat.caps & pstat_mode_oth) {
					*(line++)=(stat.mode & HT_S_IROTH) ? 'r' : '-';
					*(line++)=(stat.mode & HT_S_IWOTH) ? 'w' : '-';
					*(line++)=(stat.mode & HT_S_IXOTH) ? 'x' : '-';
				}
					
				if (stat.caps & pstat_mode_grp) {
					*(line++)=(stat.mode & HT_S_IRGRP) ? 'r' : '-';
					*(line++)=(stat.mode & HT_S_IWGRP) ? 'w' : '-';
					*(line++)=(stat.mode & HT_S_IXGRP) ? 'x' : '-';
				}
					
				if (stat.caps & pstat_mode_usr) {
					*(line++)=(stat.mode & HT_S_IRUSR) ? 'r' : '-';
					*(line++)=(stat.mode & HT_S_IWUSR) ? 'w' : '-';
					*(line++)=(stat.mode & HT_S_IXUSR) ? 'x' : '-';
				}
				*line = 0;
			}
			break;
		}
		case VFSV_FORMAT_MODE:
			if (stat.caps & pstat_mode_all) {
				ht_snprintf(buf, bufsize, "%o", stat.mode & ((1<<9)-1));
			}
			break;
		case VFSV_FORMAT_NLINK:
			break;
		case VFSV_FORMAT_NGID:
			if (stat.caps & pstat_gid) {
				ht_snprintf(buf, bufsize, "%u", stat.gid);
			}
			break;
		case VFSV_FORMAT_NUID:
			if (stat.caps & pstat_uid) {
				ht_snprintf(buf, bufsize, "%u", stat.uid);
			}
			break;
		case VFSV_FORMAT_OWNER:
			break;
		case VFSV_FORMAT_GROUP:
			break;
		case VFSV_FORMAT_INODE:
			if (stat.caps & pstat_inode) {
				ht_snprintf(buf, bufsize, "%u", stat.inode);
			}
			break;
		case VFSV_FORMAT_SPACE:
			if (bufsize>1) {
				buf[0] = ' ';
				buf[1] = 0;
			}
			break;
		case VFSV_FORMAT_MARK:
			break;
		case VFSV_FORMAT_DESC:
			if (stat.caps & pstat_desc) {
/*				tag_strcpy(line, stat.desc);
				line+=tag_strlen(line);
				alignright=0;*/
			}
			break;
	}
}

/*
 *	class VfsListbox2
 */

bool VfsListbox2::selectEntry(void *entry)
{
	if (VfsListbox::selectEntry(entry)) return true;
	ht_text_listbox_item *i = (ht_text_listbox_item*)entry;
	if (i->extra_data) {
		vfs_extra *e = (vfs_extra*)i->extra_data;
		char path[VFS_URL_MAX];
		ht_snprintf(path, sizeof path, "%s%s", cdir, e->name);
		cvfs->open(path, 1);
		return true;
	}
	return false;
}
