/* 
 *	HT Editor
 *	sysfile.cc - file system functions for DJGPP
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

#include "sys.h"

#include <ctype.h>
#include <dir.h>
#include <dpmi.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int sys_canonicalize(char *result, const char *filename)
{
	_fixpath(filename, result);
	return 0;
}

struct djfindstate {
	struct ffblk fstate;
};

char sys_find_name[HT_NAME_MAX];

int sys_findclose(pfind_t *pfind)
{
	free(pfind->findstate);
	return 0;
}

time_t dostime2time_t(unsigned short time, unsigned short date)
{
	struct tm t;
	t.tm_sec=(time & 0x1f) * 2;
	t.tm_min=(time >>  5) & 0x3f;
	t.tm_hour=(time >> 11) & 0x1f;
	t.tm_mday=date & 0x1f;
	t.tm_mon=((date >>  5) & 0x0f)-1;
	t.tm_year=((date >> 9) & 0x7f) + 80;
	t.tm_wday=1;
	t.tm_yday=1;
	t.tm_isdst=0;
	return mktime(&t);
}

void ff2pstat(struct ffblk *f, pstat_t *s)
{
	s->caps = pstat_size | pstat_mtime | pstat_mode_type | pstat_mode_usr;
	s->size = mkuint64(f->ff_fsize);
	s->mtime = dostime2time_t(f->ff_ftime, f->ff_fdate);
	s->mode = HT_S_IRUSR;
	if (f->ff_attrib & FA_DIREC) {
		s->mode |= HT_S_IFDIR;
	} else {
		s->mode |= HT_S_IFREG;
	}
	if (!(f->ff_attrib & FA_RDONLY)) {
		s->mode |= HT_S_IWUSR;
	}
	if ((f->lfn_magic[0]=='L') && (f->lfn_magic[1]=='F') && (f->lfn_magic[2]=='N') && 
	(f->lfn_magic[3]=='3') && (f->lfn_magic[4]=='2') && (f->lfn_magic[5]==0)) {
		s->caps|=pstat_ctime | pstat_atime;
		s->ctime=dostime2time_t(f->lfn_ctime, f->lfn_cdate);
		s->atime=dostime2time_t(f->lfn_atime, f->lfn_adate);
	}
}

int sys_findfirst(const char *dirname, pfind_t *pfind)
{
	int dnl=strlen(dirname);
	strcpy(sys_find_name, dirname);
	if ((dirname[dnl-1]!='\\') && (dirname[dnl-1]!='/')) {
		sys_find_name[dnl]='\\';
		sys_find_name[dnl+1]=0;
	}
	strcat(sys_find_name, "*.*");
	char *s=sys_find_name;
	while ((s=strchr(s, '/'))) *s='\\';
	
	pfind->findstate=malloc(sizeof (djfindstate));
	djfindstate *dfs=(djfindstate*)pfind->findstate;
	
	int e=findfirst(sys_find_name, &dfs->fstate, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH);
	if (e) {
		free(pfind->findstate);
		return e;
	}
	strcpy(sys_find_name, dfs->fstate.ff_name);
	pfind->name=sys_find_name;
	ff2pstat(&dfs->fstate, &pfind->stat);
	return 0;
}

int sys_findnext(pfind_t *pfind)
{
	djfindstate *dfs=(djfindstate*)pfind->findstate;
	int e=findnext(&dfs->fstate);
	if (e) return e;
	strcpy(sys_find_name, dfs->fstate.ff_name);
	pfind->name=sys_find_name;
	ff2pstat(&dfs->fstate, &pfind->stat);
	return 0;
}

int sys_pstat(pstat_t *s, const char *filename)
{
	struct stat st;
	int e=stat(filename, &st);
	if (e) return e;
	s->caps=pstat_mtime|pstat_mode_usr|pstat_mode_w|pstat_size/*|pstat_cluster*/|pstat_mode_type;
	s->mtime=st.st_mtime;
	s->mode=sys_ht_mode(st.st_mode);
	s->size=mkuint64(st.st_size);
	s->fsid=st.st_ino;
	return 0;
}

int sys_truncate(const char *filename, fileofs ofs)
{
	return truncate(filename, ofs);
}

int sys_deletefile(const char *filename)
{
	return unlink(filename);
}

bool sys_is_path_delim(const char c)
{
	return (c == '/') || (c == '\\');
}

int sys_filename_cmp(const char *a, const char *b)
{
	while (*a && *b) {
		if (sys_is_path_delim(*a) && sys_is_path_delim(*b)) {
		} else if (tolower(*a) != tolower(*b)) {
			break;
		} else if (*a != *b) {
			break;
		}
		a++;
		b++;
	}
	return tolower(*a) - tolower(*b);
}

// FIXME: not a file system function, place somewhere else (but where ?)
void sys_suspend()
{
	__dpmi_yield();
}

