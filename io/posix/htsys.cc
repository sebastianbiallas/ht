/* 
 *	HT Editor
 *	htsys.cc (POSIX implementation)
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

#include "htsys.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
		 
int sys_canonicalize(char *result, const char *filename)
{
	return (realpath(filename, result)==result) ? 0 : ENOENT;
}

struct posixfindstate {
	DIR *fhandle;
};

char sys_find_dirname[HT_NAME_MAX];

int sys_findclose(pfind_t *pfind)
{
	int r=closedir(((posixfindstate*)pfind->findstate)->fhandle);
	free(pfind->findstate);
	return r;
}

int sys_findfirst(const char *dirname, pfind_t *pfind)
{
	int r;
	pfind->findstate=malloc(sizeof (posixfindstate));
	posixfindstate *pfs=(posixfindstate*)pfind->findstate;
	if ((pfs->fhandle=opendir(dirname))) {
		strcpy(sys_find_dirname, dirname);
		char *s=sys_find_dirname+strlen(sys_find_dirname);
		if ((s>sys_find_dirname) && (*(s-1)!='/')) {
		    *(s++)='/';
		    *s=0;
		}
		r=sys_findnext(pfind);
	} else r=errno ? errno : ENOENT;
	if (r) free(pfind->findstate);
	return r;
}

int sys_findnext(pfind_t *pfind)
{
	posixfindstate *pfs=(posixfindstate*)pfind->findstate;
	struct dirent *d;
	if ((d=readdir(pfs->fhandle))) {
		pfind->name=d->d_name;
		char *s=sys_find_dirname+strlen(sys_find_dirname);
		strcpy(s, d->d_name);
		sys_pstat(&pfind->stat, sys_find_dirname);
		*s=0;
		return 0;
	}
	return ENOENT;
}

int sys_pstat(pstat_t *s, const char *filename)
{
	struct stat st;
	int e=stat(filename, &st);
	if (e) return e;
	s->caps=pstat_ctime|pstat_mtime|pstat_atime|pstat_uid|pstat_gid|pstat_mode_all|pstat_size|pstat_inode;
	s->ctime=st.st_ctime;
	s->mtime=st.st_mtime;
	s->atime=st.st_atime;
	s->gid=st.st_uid;
	s->uid=st.st_gid;
	s->mode=sys_ht_mode(st.st_mode);
	s->size=st.st_size;
	s->size_high=0;
	s->fsid=st.st_ino;
	return 0;
}

void sys_suspend()
{
	timeval tm;
	fd_set zerofds;
	FD_ZERO(&zerofds);
	
	tm.tv_sec=0;
	tm.tv_usec=100;
	select(0, &zerofds, &zerofds, &zerofds, &tm);
}

int sys_get_free_mem()
{
	return 0;
}

int sys_truncate(const char *filename, FILEOFS ofs)
{
	return ENOSYS;
}

int sys_deletefile(const char *filename)
{
	return remove(filename);
}

bool sys_is_path_delim(char c)
{
	return c == '/';
}

int sys_filename_cmp(const char *a, const char *b)
{
	while (*a && *b) {
		if (sys_is_path_delim(*a) && sys_is_path_delim(*b)) {
		} else if (*a != *b) {
			break;
		}
		a++;
		b++;
	}
	return *a - *b;
}

/*
 *	INIT
 */

bool init_system()
{
	setuid( getuid() );
	return true;
}

/*
 *	DONE
 */

void done_system()
{
}
