/* 
 *	HT Editor
 *	htsys.cc (WIN32 implementation)
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include "htsys.h"
#include "qword.h"

#include <ctype.h>
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

int sys_canonicalize(char *result, const char *filename)
{
	char *dunno;
	return (GetFullPathName(filename, HT_NAME_MAX, result, &dunno) > 0) ? 0 : ENOENT;
}

dword filetime_to_ctime(FILETIME f)
{
	qword q;
	QWORD_SET_LO(q, f.dwLowDateTime);
	QWORD_SET_HI(q, f.dwHighDateTime);
	q = q / int_to_qword(10000000);		// 100 nano-sec to full sec
	return QWORD_GET_LO(q) + 1240431886;	// MAGIC: this is 1.1.1970 minus 1.1.1601 in seconds
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
	pfind->stat.ctime = filetime_to_ctime(wfs->find_data.ftCreationTime);
	pfind->stat.mtime = filetime_to_ctime(wfs->find_data.ftLastWriteTime);
	pfind->stat.atime = filetime_to_ctime(wfs->find_data.ftLastAccessTime);
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

/*
 *	Win32 IPC
 */

class ht_win32_file: public ht_streamfile {
private:
	HANDLE h;
public:
		   void init(HANDLE h);
/* overwritten */
	virtual UINT		read(void *buf, UINT size);
	virtual UINT		write(const void *buf, UINT size);
};

void ht_win32_file::init(HANDLE H)
{
	ht_streamfile::init();
	h = H;
}

UINT ht_win32_file::read(void *buf, UINT size)
{
	DWORD avail, unread;
	if (!PeekNamedPipe(h, NULL, 0, NULL, &avail, &unread)) return 0;
	DWORD n = 0;
	if (avail) {
		int c = (avail > size) ? size : avail;
		if (!ReadFile(h, buf, c, &n, NULL)) n = 0;
	}
	return n;
}

UINT ht_win32_file::write(const void *buf, UINT size)
{
	DWORD n = 0;
	if (!WriteFile(h, buf, size, &n, NULL)) n = 0;
	return n;
}

/**/

BOOL CreateChildProcess(DWORD *pid, const char *cmd, HANDLE in, HANDLE out, HANDLE err)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = in;
	si.hStdOutput = out;
	si.hStdError = err;

	TCHAR shellCmd[HT_NAME_MAX];
	strcpy(shellCmd, cmd);
	BOOL ret = CreateProcess(NULL, shellCmd, NULL, NULL, TRUE, 0/*DETACHED_PROCESS*/,
		NULL, NULL, &si, &pi);

	if (ret) *pid = pi.dwProcessId;
	return ret;
}

static HANDLE myPID = NULL;

int sys_ipc_exec(ht_streamfile **in, ht_streamfile **out, ht_streamfile **err, int *handle, const char *cmd, int options)
{
	if (myPID != NULL) return EBUSY;
	HANDLE old_out = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE outr, outw, outr_dup;
	if (!CreatePipe(&outr, &outw, NULL, 0)) return EIO;
	if (!SetStdHandle(STD_OUTPUT_HANDLE, outw)) return EIO;
	if (!DuplicateHandle(GetCurrentProcess(), outr,	GetCurrentProcess(),
		&outr_dup, 0, FALSE, DUPLICATE_SAME_ACCESS)) return EIO;
	CloseHandle(outr);

	HANDLE old_in = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE inr, inw, inw_dup;
	if (!CreatePipe(&inr, &inw, NULL, 0)) return EIO;
	if (!SetStdHandle(STD_INPUT_HANDLE, inr)) return EIO;
	if (!DuplicateHandle(GetCurrentProcess(), inw, GetCurrentProcess(),
		&inw_dup, 0, FALSE, DUPLICATE_SAME_ACCESS)) return EIO;
	CloseHandle(inw);

	DWORD child_pid;
	if (!CreateChildProcess(&child_pid, cmd, inr, outw, outw))
		return EIO;

	SetStdHandle(STD_OUTPUT_HANDLE, old_out);
	SetStdHandle(STD_INPUT_HANDLE, old_in);

	HANDLE ph = OpenProcess(PROCESS_ALL_ACCESS, false, child_pid);
	if (ph == NULL) return EIO;

	myPID = ph;
	*handle = (int)ph;

	ht_win32_file *wf;

	wf = new ht_win32_file();
	wf->init(inw_dup);
	*in = wf;

	wf = new ht_win32_file();
	wf->init(outr_dup);
	*out = wf;

	wf = new ht_win32_file();
	wf->init(outr_dup);
	*err = wf;

	return 0;
}

bool sys_ipc_is_valid(int handle)
{
	HANDLE h = (HANDLE)handle;
	if (h == myPID) {
		return (WaitForSingleObject(h, 0) != WAIT_OBJECT_0);
	}
	return false;
}

int sys_ipc_terminate(int handle)
{
	if (sys_ipc_is_valid(handle)) {
		HANDLE h = (HANDLE)handle;
		int r = (TerminateProcess(h, 1)) ? 0 : EIO;
		CloseHandle(h);
		myPID = NULL;
		return r;
	}
	myPID = NULL;
	return EINVAL;
}

int sys_get_caps()
{
	return SYSCAP_NONBLOCKING_IPC;
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

