/* 
 *	HT Editor
 *	ipc.cc - Win32-specific inter-process communication (IPC) functions
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

#include <errno.h>

#include "stream.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/*
 *	Win32 IPC
 */

class Win32File: public StreamFile {
private:
	HANDLE h;
public:
	       		Win32File(HANDLE h);
/* overwritten */
	virtual	uint	read(void *buf, uint size);
	virtual	uint	write(const void *buf, uint size);
	virtual	void	extend(fileofs newsize) {}
	virtual	void	truncate(fileofs newsize) {}
};

Win32File::Win32File(HANDLE H)
{
	h = H;
}

uint Win32File::read(void *buf, uint size)
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

uint Win32File::write(const void *buf, uint size)
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

int sys_subprocess_exec(StreamFile **in, StreamFile **out, StreamFile **err, int *handle, const char *cmd, int options)
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

	Win32File *wf;

	wf = new Win32File(inw_dup);
	*in = wf;

	wf = new Win32File(outr_dup);
	*out = wf;

	wf = new Win32File(outr_dup);
	*err = wf;

	return 0;
}

bool sys_subprocess_still_running(int handle)
{
	HANDLE h = (HANDLE)handle;
	if (h == myPID) {
		return (WaitForSingleObject(h, 0) != WAIT_OBJECT_0);
	}
	return false;
}

int sys_subprocess_terminate(int handle)
{
	if (sys_subprocess_still_running(handle)) {
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
	return SYSCAP_IPC | SYSCAP_NBIPC /*| SYSCAP_NATIVE_CLIPBOARD*/;
}
