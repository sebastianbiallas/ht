/* 
 *	HT Editor
 *	htsys.cc (WIN32 implementation)
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct winfindstate {
	HANDLE fhandle;
	WIN32_FIND_DATA find_data;
};

int sys_canonicalize(char *filename, const char *fullfilename)
{
	char *dunno;
	return (GetFullPathName(filename, HT_NAME_MAX, fullfilename, &dunno) > 0) ? 0 : ENOENT;
}

void sys_findfill(pfind_t *pfind)
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
	winfindstate *wfs=(winfindstate*)pfind->findstate;
	pfind->name = (char *)&wfs->find_data.cFileName;
	pfind->stat.caps = pstat_ctime|pstat_mtime|pstat_atime|pstat_size|pstat_mode_type;
	pfind->stat.size = wfs->find_data.nFileSizeLow;
	pfind->stat.size_high = wfs->find_data.nFileSizeHigh;
	pfind->stat.mode = (wfs->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? HT_S_IFDIR : HT_S_IFREG;
	pfind->stat.ctime = 0;
	pfind->stat.mtime = 0;
	pfind->stat.atime = 0;
}

int sys_findfirst(const char *dirname, pfind_t *pfind)
{
	char *Dirname = (char *)malloc(strlen(dirname)+5);
	strcpy(Dirname, dirname);
	int dnl=strlen(dirname);
	if ((dirname[dnl-1]!='\\') && (dirname[dnl-1]!='/')) {
		Dirname[dnl]='\\';
		Dirname[dnl+1]=0;
	}
	char *s=Dirname;
	while ((s=strchr(s, '/'))) *s='\\';
	strcat(Dirname, "*.*");
	
	pfind->findstate=malloc(sizeof (winfindstate));
	winfindstate *wfs=(winfindstate*)pfind->findstate;
	
	wfs->fhandle = FindFirstFile(Dirname, &wfs->find_data);
	free(Dirname);
	if (wfs->fhandle == INVALID_HANDLE_VALUE) {
		free(pfind->findstate);
		return ENOENT;
	}
	sys_findfill(pfind);
	return 0;
}

int sys_findnext(pfind_t *pfind)
{
	winfindstate *wfs=(winfindstate*)pfind->findstate;
	
	if (!FindNextFile(wfs->fhandle, &wfs->find_data)) {
		return ENOENT;
	}
	sys_findfill(pfind);
	return 0;
}

int sys_findclose(pfind_t *pfind)
{
	int r=FindClose(((winfindstate*)pfind->findstate)->fhandle);
	free(pfind->findstate);
	return r ? ENOENT : 0;
}

int sys_pstat(pstat_t *s, const char *filename)
{
	struct stat st;
	int e=stat(filename, &st);
	if (e) return ENOENT;
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
	Sleep(0);
}

int sys_get_free_mem()
{
	return 0;
}

int sys_truncate(const char *filename, FILEOFS ofs)
{
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

int sys_deletefile(const char *filename)
{
	if (DeleteFile(filename)) {
		return 0;
	}
	return EIO;
}

bool sys_is_path_delim(char c)
{
	return (c == '\\');
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

int sys_ipc_exec(int *in, int *out, int *err, int *handle, const char *cmd)
{
	return ENOSYS;
}

bool sys_ipc_is_valid(int handle)
{
	return false;
}

int sys_ipc_terminate(int handle)
{
	return 0;
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

