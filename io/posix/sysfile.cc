/* 
 *	HT Editor
 *	sysfile.cc - file system functions for POSIX
 *
 *	Copyright (C) 1999-2004 Stefan Weyergraf (stefan@weyergraf.de)
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "io/file.h"

struct posixfindstate {
	DIR *fhandle;
};

bool sys_filename_is_absolute(const char *filename)
{
	return sys_is_path_delim(filename[0]);
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

int sys_canonicalize(char **result, const char *filename)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	*result = (char*)malloc(HT_NAME_MAX);
	if (realpath(filename, *result)==*result) {
		return 0;
	} else {
		free(*result);
		return ENOENT;
	}
}

static char sys_find_dirname[HT_NAME_MAX];

int sys_findclose(pfind_t &pfind)
{
	int r = closedir(((posixfindstate*)pfind.findstate)->fhandle);
	free(pfind.findstate);
	return r;
}

int sys_findfirst(pfind_t &pfind, const char *dirname)
{
	if (!sys_filename_is_absolute(dirname)) return ENOENT;
	int r;
	pfind.findstate = malloc(sizeof (posixfindstate));
	posixfindstate *pfs = (posixfindstate*)pfind.findstate;
	if ((pfs->fhandle = opendir(dirname))) {
		strcpy(sys_find_dirname, dirname);
		char *s = sys_find_dirname+strlen(sys_find_dirname);
		if ((s > sys_find_dirname) && (*(s-1) != '/')) {
		    *(s++) = '/';
		    *s = 0;
		}
		r = sys_findnext(pfind);
	} else r = errno ? errno : ENOENT;
	if (r) free(pfind.findstate);
	return r;
}

int sys_findnext(pfind_t &pfind)
{
	posixfindstate *pfs = (posixfindstate*)pfind.findstate;
	struct dirent *d;
	if ((d = readdir(pfs->fhandle))) {
		pfind.name = d->d_name;
		char *s = sys_find_dirname+strlen(sys_find_dirname);
		strcpy(s, d->d_name);
		sys_pstat(pfind.stat, sys_find_dirname);
		*s = 0;
		return 0;
	}
	return ENOENT;
}

static void stat_to_pstat_t(const struct stat &st, pstat_t &s)
{
	s.caps = pstat_ctime|pstat_mtime|pstat_atime|pstat_uid|pstat_gid|pstat_mode_all|pstat_size|pstat_inode;
	s.ctime = st.st_ctime;
	s.mtime = st.st_mtime;
	s.atime = st.st_atime;
	s.gid = st.st_uid;
	s.uid = st.st_gid;
	s.mode = sys_file_mode(st.st_mode);
	s.size = st.st_size;
	s.fsid = st.st_ino;
}

int sys_pstat(pstat_t &s, const char *filename)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	struct stat st;
	errno = 0;
	int e = lstat(filename, &st);
	if (e) return errno ? errno : ENOENT;
	stat_to_pstat_t(st, s);
	return 0;
}

int sys_pstat_fd(pstat_t &s, int fd)
{
	struct stat st;
	errno = 0;
	int e = fstat(fd, &st);
	if (e) return errno ? errno : ENOENT;
	stat_to_pstat_t(st, s);
	return 0;
}

int sys_pstat_file(pstat_t &s, SYS_FILE *file)
{
	return sys_pstat_fd(s, fileno((FILE*)file));
}

int sys_truncate(const char *filename, FileOfs ofs)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	int fd = open(filename, O_RDWR, 0);
	if (fd < 0) return errno;
	if (ftruncate(fd, ofs) != 0) {
		close(fd);
		return errno;
	}
	if (close(fd)) return errno;
	return 0;
}

int sys_truncate_fd(int fd, FileOfs ofs)
{
	if (ftruncate(fd, ofs) != 0) return errno;
	return 0;
}

int sys_deletefile(const char *filename)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	return remove(filename);
}

void sys_suspend()
{
	timeval tm;
	fd_set zerofds;
	FD_ZERO(&zerofds);
	
	tm.tv_sec = 0;
	tm.tv_usec = 100;
	select(0, &zerofds, &zerofds, &zerofds, &tm);
}

static const char *openmode2mode(FileOpenMode om, IOAccessMode am)
{
	const char *mode = NULL;

	switch (om) {
	case FOM_APPEND:
		mode = "ab+";
		break;
	case FOM_CREATE:
		if (am & IOAM_WRITE) mode = "wb";
		if (am & IOAM_READ) mode = "wb+";
		break;
	case FOM_EXISTS:
		if (am & IOAM_READ) mode = "rb";
		if (am & IOAM_WRITE) mode = "rb+";
		break;
	}
	return mode;
}

SYS_FILE *sys_freopen(const char *filename, FileOpenMode openmode,
	IOAccessMode accessmode, SYS_FILE *file)
{
	return (SYS_FILE *)freopen(filename, openmode2mode(openmode, accessmode),
		(FILE*)file);
}

SYS_FILE *sys_fopen(const char *filename, FileOpenMode openmode, IOAccessMode accessmode)
{
	return (SYS_FILE *)fopen(filename, openmode2mode(openmode, accessmode));
}

void sys_fclose(SYS_FILE *file)
{
	fclose((FILE *)file);
}

int sys_fread(SYS_FILE *file, byte *buf, int size)
{
	return fread(buf, 1, size, (FILE *)file);
}

int sys_fwrite(SYS_FILE *file, byte *buf, int size)
{
	return fwrite(buf, 1, size, (FILE *)file);
}

int sys_fseek(SYS_FILE *file, FileOfs newofs, int seekmode)
{
	int r;
	switch (seekmode) {
	case SYS_SEEK_SET: r = fseeko((FILE *)file, newofs, SEEK_SET); break;
	case SYS_SEEK_REL: r = fseeko((FILE *)file, newofs, SEEK_CUR); break;
	case SYS_SEEK_END: r = fseeko((FILE *)file, newofs, SEEK_END); break;
	default: return EINVAL;
	}
	return r ? errno : 0;
}

FileOfs	sys_ftell(SYS_FILE *file)
{
	return ftello((FILE *)file);
}

const char *sys_get_name()
{
	return "POSIX";
}

int sys_get_caps()
{
	return 0;
}
