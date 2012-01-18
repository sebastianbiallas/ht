/* 
 *	HT Editor
 *	sysfile.cc - file system functions for Win32
 *
 *	Copyright (C) 1999-2003 Sebastian Biallas (sb@biallas.net)
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

#include <cctype>
#include <errno.h>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>

#include "io/sys.h"
#include "io/file.h"
#include "data.h"
#include "strtools.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct winfindstate {
	HANDLE fhandle;
	WIN32_FIND_DATA find_data;
};

bool sys_is_path_delim(char c)
{
	return (c == '\\') /*|| (c == '/')*/;
}

inline bool sys_filename_is_absolute(const char *filename)
{
	return (isalpha(filename[0]) && (filename[1] == ':') &&
		sys_is_path_delim(filename[2]));
}

int sys_filename_cmp(const char *a, const char *b)
{
	while (*a && *b) {
		if (sys_is_path_delim(*a) && sys_is_path_delim(*b)) {
		} else if (tolower(*a) != tolower(*b)) {
			break;
/*		} else if (*a != *b) {
			break;*/
		}
		a++;
		b++;
	}
	return tolower(*a) - tolower(*b);
}

int sys_canonicalize(char **result, const char *filename)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	char *dunno;
	int res;
	int maxlen = 100;
	*result = (char*)malloc(maxlen);
	while ((res = GetFullPathName(filename, maxlen, *result, &dunno)) > 0) {
		if (res >= maxlen) {
			*result = (char*)realloc(*result, res);
			maxlen = res+1;
		} else {
			return 0;
		}
	}
	return ENOENT;
}

static uint filetime_to_ctime(FILETIME f)
{
	uint64 q;
	q = ((uint64)f.dwHighDateTime) << 32 | f.dwLowDateTime;
	q /= 10000000ULL;		// 100 nano-sec to full sec
	return q + 1240431886;	// MAGIC: this is 1.1.1970 minus 1.1.1601 in seconds
}

static void sys_findfill(pfind_t &pfind)
{
	/*DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD    nFileSizeHigh;
	DWORD    nFileSizeLow;
	DWORD    dwReserved0;
	DWORD    dwReserved1;
	TCHAR    cFileName[ MAX_PATH ];
	TCHAR    cAlternateFileName[ 14 ];*/
	winfindstate *wfs = (winfindstate*)pfind.findstate;
	pfind.name = (char *)&wfs->find_data.cFileName;
	pfind.stat.caps = pstat_ctime|pstat_mtime|pstat_atime|pstat_size|pstat_mode_type;

	pfind.stat.size = (wfs->find_data.nFileSizeLow) | (((uint64)wfs->find_data.nFileSizeHigh) << 32);

	pfind.stat.mode = (wfs->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? HT_S_IFDIR : HT_S_IFREG;
	pfind.stat.ctime = filetime_to_ctime(wfs->find_data.ftCreationTime);
	pfind.stat.mtime = filetime_to_ctime(wfs->find_data.ftLastWriteTime);
	pfind.stat.atime = filetime_to_ctime(wfs->find_data.ftLastAccessTime);
}

int sys_findfirst(pfind_t &pfind, const char *dirname)
{
	if (!sys_filename_is_absolute(dirname)) return ENOENT;
	char *Dirname = ht_malloc(strlen(dirname)+5);
	strcpy(Dirname, dirname);
	int dnl = strlen(dirname);
	if ((dirname[dnl-1] != '\\') && (dirname[dnl-1] != '/')) {
		Dirname[dnl] = '\\';
		Dirname[dnl+1] = 0;
	}
	char *s = Dirname;
	while ((s = strchr(s, '/'))) *s = '\\';
	strcat(Dirname, "*.*");

	pfind.findstate = malloc(sizeof (winfindstate));
	winfindstate *wfs = (winfindstate*)pfind.findstate;

	wfs->fhandle = FindFirstFile(Dirname, &wfs->find_data);
	free(Dirname);
	if (wfs->fhandle == INVALID_HANDLE_VALUE) {
		free(pfind.findstate);
		return ENOENT;
	}
	sys_findfill(pfind);
	return 0;
}

int sys_findnext(pfind_t &pfind)
{
	winfindstate *wfs = (winfindstate*)pfind.findstate;
	
	if (!FindNextFile(wfs->fhandle, &wfs->find_data)) {
		return ENOENT;
	}
	sys_findfill(pfind);
	return 0;
}

int sys_findclose(pfind_t &pfind)
{
	int r = FindClose(((winfindstate*)pfind.findstate)->fhandle);
	free(pfind.findstate);
	return r ? ENOENT : 0;
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
	char fn[HT_NAME_MAX];
	ht_strlcpy(fn, filename, sizeof fn);
	int flen = strlen(fn);
	if (flen && sys_is_path_delim(fn[flen-1]) && (flen !=3) || (fn[1]!=':')) fn[flen-1] = 0;
	if (!sys_filename_is_absolute(fn)) return ENOENT;
	struct stat st;
	int e = stat(fn, &st);
	if (e) return errno ? errno : ENOENT;
	stat_to_pstat_t(st, s);
	return 0;
}

int sys_pstat_fd(pstat_t &s, int fd)
{
	struct stat st;
	int e = fstat(fd, &st);
	if (e) return errno ? errno : ENOENT;
	stat_to_pstat_t(st, s);
	return 0;
}

int sys_pstat_filename(pstat_t &s, const char *filename)
{
}

int sys_pstat_file(pstat_t &s, SYS_FILE *file)
{
	return sys_pstat_fd(s, fileno((FILE*)file));
}

int sys_truncate(const char *filename, FileOfs ofs)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	HANDLE hfile = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return EIO;
	}
	if (SetFilePointer(hfile, ofs, NULL, FILE_BEGIN)==0xffffffff) {
		CloseHandle(hfile);
		return EIO;
	}
	if (!SetEndOfFile(hfile)) {
		CloseHandle(hfile);
		return EIO;
	}
	CloseHandle(hfile);
	return 0;
}

int sys_truncate_fd(int fd, FileOfs ofs)
{
	return ENOSYS;
}

int sys_deletefile(const char *filename)
{
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	if (DeleteFile(filename)) {
		return 0;
	}
	return EIO;
}

void sys_suspend()
{
	Sleep(42);
}

static const char *openmode2mode(FileOpenMode om, IOAccessMode am)
{
	char *mode = NULL;

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
	case SYS_SEEK_SET: r = fseek((FILE *)file, newofs, SEEK_SET); break;
	case SYS_SEEK_REL: r = fseek((FILE *)file, newofs, SEEK_CUR); break;
	case SYS_SEEK_END: r = fseek((FILE *)file, newofs, SEEK_END); break;
	default: return EINVAL;
	}
	return r ? errno : 0;
}

FileOfs	sys_ftell(SYS_FILE *file)
{
	return ftell((FILE *)file);
}

const char *sys_get_name()
{
	return "win32";
}

int sys_get_caps()
{
	return SYSCAP_NATIVE_CLIPBOARD;
}

/*
 *	INIT
 */

bool init_system()
{
	return true;
}

/*
 *	DONE
 */

void done_system()
{
}

