/*
 *	HT Editor
 *	htsys.cc (POSIX implementation)
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

#include "htsys.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
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
	int r = closedir(((posixfindstate*)pfind->findstate)->fhandle);
	free(pfind->findstate);
	return r;
}

int sys_findfirst(const char *dirname, pfind_t *pfind)
{
	int r;
	pfind->findstate = malloc(sizeof (posixfindstate));
	posixfindstate *pfs = (posixfindstate*)pfind->findstate;
	if ((pfs->fhandle = opendir(dirname))) {
		strcpy(sys_find_dirname, dirname);
		char *s = sys_find_dirname+strlen(sys_find_dirname);
		if ((s > sys_find_dirname) && (*(s-1) != '/')) {
		    *(s++) = '/';
		    *s = 0;
		}
		r = sys_findnext(pfind);
	} else r = errno ? errno : ENOENT;
	if (r) free(pfind->findstate);
	return r;
}

int sys_findnext(pfind_t *pfind)
{
	posixfindstate *pfs = (posixfindstate*)pfind->findstate;
	struct dirent *d;
	if ((d = readdir(pfs->fhandle))) {
		pfind->name = d->d_name;
		char *s = sys_find_dirname+strlen(sys_find_dirname);
		strcpy(s, d->d_name);
		sys_pstat(&pfind->stat, sys_find_dirname);
		*s = 0;
		return 0;
	}
	return ENOENT;
}

int sys_pstat(pstat_t *s, const char *filename)
{
	struct stat st;
	errno = 0;
	int e = stat(filename, &st);
	if (e) return errno ? errno : ENOENT;
	s->caps = pstat_ctime|pstat_mtime|pstat_atime|pstat_uid|pstat_gid|pstat_mode_all|pstat_size|pstat_inode;
	s->ctime = st.st_ctime;
	s->mtime = st.st_mtime;
	s->atime = st.st_atime;
	s->gid = st.st_uid;
	s->uid = st.st_gid;
	s->mode = sys_ht_mode(st.st_mode);
	s->size = st.st_size;
	s->size_high = 0;
	s->fsid = st.st_ino;
	return 0;
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

int sys_get_free_mem()
{
	return 0;
}

int sys_truncate(const char *filename, FILEOFS ofs)
{
	int fd = open(filename, O_RDWR, 0);
	if (fd < 0) return errno;
	if (ftruncate(fd, ofs) != 0) return errno;
	return close(fd);
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
 * 	POSIX IPC
 */
 
static int child_pid = -1;

void SIGCHLD_signal(int i)
{
	int j;
	waitpid(child_pid, &j, WNOHANG);
	child_pid = -1;
}

int sys_ipc_exec(ht_streamfile **in, ht_streamfile **out, ht_streamfile **err, int *handle, const char *cmd, int options)
{
	if (child_pid != -1) return EBUSY;
	int in_fds[2];
	int out_fds[2];
	int err_fds[2];
	int pid;
	if (pipe(in_fds)) return errno;
	if (pipe(out_fds)) return errno;
	if (pipe(err_fds)) return errno;
	pid = fork();
	if (pid > 0) {
		/* parent process */
		close(in_fds[0]);
		close(out_fds[1]);
		close(err_fds[1]);
		int in_fd = in_fds[1];
		int out_fd = out_fds[0];
		int err_fd = err_fds[0];
		ht_sys_file *f;
		f = new ht_sys_file();
		f->init(in_fd, true, FAM_WRITE);
		*in = f;
		f = new ht_sys_file();
		f->init(out_fd, true, FAM_READ);
		*out = f;
		f = new ht_sys_file();
		f->init(err_fd, true, FAM_READ);
		*err = f;
		*handle = pid;
		if (fcntl(out_fd, F_SETFL, O_NONBLOCK) ||
		fcntl(err_fd, F_SETFL, O_NONBLOCK)) return errno;
		child_pid = pid;
		return 0;
	} else if (pid == 0) {
		/* child process */
		close(in_fds[1]);
		close(out_fds[0]);
		close(err_fds[0]);
		dup2(in_fds[0], STDIN_FILENO);
		dup2(out_fds[1], STDOUT_FILENO);
		dup2(err_fds[1], STDERR_FILENO);
		close(in_fds[0]);
		close(out_fds[1]);
		close(err_fds[1]);
		execl(cmd, cmd, NULL);
		exit(1);
	} else return errno;
}

bool sys_ipc_is_valid(int handle)
{
	return child_pid == handle;
}

int sys_ipc_terminate(int handle)
{
	if (child_pid == handle) {
		kill(handle, SIGTERM);
		child_pid = -1;
		return 0;
	}		
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
	setuid( getuid() );
	(void)signal(SIGCHLD, SIGCHLD_signal);
	return true;
}

/*
 *	DONE
 */

void done_system()
{
}
