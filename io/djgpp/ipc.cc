/* 
 *	HT Editor
 *   ipc.cc - DJGPP-specific inter-process communication (IPC) functions
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

#include <unistd.h>
#include <stdlib.h>

#include "stream.h"

int sys_subprocess_exec(StreamFile **in, StreamFile **out, StreamFile **err, int *handle, const char *cmd)
{
	int save_stdout = dup(STDOUT_FILENO);
	int save_stderr = dup(STDERR_FILENO);
	int fdout = sys_tmpfile_fd();
	int fderr = sys_tmpfile_fd();
	dup2(fdout, STDOUT_FILENO);
	dup2(fderr, STDERR_FILENO);
	/*int r = */system(cmd);
	dup2(save_stdout, STDOUT_FILENO);
	dup2(save_stderr, STDERR_FILENO);
	close(save_stdout);
	close(save_stderr);
	lseek(fdout, 0, SEEK_SET);
	lseek(fderr, 0, SEEK_SET);
	NullFile *nf = new NullFile();
	*in = nf;
	SysFile *sf;
	sf = new SysFile(fdout, true, FAM_READ);
	*out = sf;
	sf = new SysFile(fderr, true, FAM_READ);
	*err = sf;
	return 0;
}

bool sys_subprocess_still_running(int handle)
{
// no multitasking, always false
	return false;
}

int sys_subprocess_terminate(int handle)
{
// do nothing
	return 0;
}

