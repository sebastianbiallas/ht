/* 
 *	HT Editor
 *	sys.cc
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
#include <errno.h>
#include <limits.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#include "file.h"
#include "strtools.h"

/*
 *	COMMON SYS
 */

#ifndef S_IFMT
#define S_IFMT 0xf000
#endif

#ifndef S_ISREG
#	ifndef S_IFREG
#		define S_ISREG(m) (0)
#	else
#		define S_ISREG(m) (((m) & S_IFMT)==S_IFREG)
#	endif
#endif

#ifndef S_ISBLK
#	ifndef S_IFBLK
#		define S_ISBLK(m) (0)
#	else
#		define S_ISBLK(m) (((m) & S_IFMT)==S_IFBLK)
#	endif
#endif


#ifndef S_ISCHR
#	ifndef S_IFCHR
#		define S_ISCHR(m) (0)
#	else
#		define S_ISCHR(m) (((m) & S_IFMT)==S_IFCHR)
#	endif
#endif

#ifndef S_ISDIR
#	ifndef S_IFDIR
#		define S_ISDIR(m) (0)
#	else
#		define S_ISDIR(m) (((m) & S_IFMT)==S_IFDIR)
#	endif
#endif

#ifndef S_ISFIFO
#	ifndef S_IFFIFO
#		define S_ISFIFO(m) (0)
#	else
#		define S_ISFIFO(m) (((m) & S_IFMT)==S_IFFIFO)
#	endif
#endif

#ifndef S_ISLNK
#	ifndef S_IFLNK
#		define S_ISLNK(m) (0)
#	else
#		define S_ISLNK(m) (((m) & S_IFMT)==S_IFLNK)
#	endif
#endif

#ifndef S_ISSOCK
#	ifndef S_IFSOCK
#		define S_ISSOCK(m) (0)
#	else
#		define S_ISSOCK(m) (((m) & S_IFMT)==S_IFSOCK)
#	endif
#endif

#ifndef S_IRUSR
#define S_IRUSR 0
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif

#ifndef S_IWUSR
#define S_IWUSR 0
#endif
#ifndef S_IWGRP
#define S_IWGRP 0
#endif
#ifndef S_IWOTH
#define S_IWOTH 0
#endif

#ifndef S_IXUSR
#define S_IXUSR 0
#endif
#ifndef S_IXGRP
#define S_IXGRP 0
#endif
#ifndef S_IXOTH
#define S_IXOTH 0
#endif

int sys_ht_mode(int mode)
{
	int m = 0;
	if (S_ISREG(mode)) {
		m |= HT_S_IFREG;
	} else if (S_ISBLK(mode)) {
		m |= HT_S_IFBLK;
	} else if (S_ISCHR(mode)) {
		m |= HT_S_IFCHR;
	} else if (S_ISDIR(mode)) {
		m |= HT_S_IFDIR;
	} else if (S_ISFIFO(mode)) {
		m |= HT_S_IFFIFO;
	} else if (S_ISLNK(mode)) {
		m |= HT_S_IFLNK;
	} else if (S_ISSOCK(mode)) {
		m |= HT_S_IFSOCK;
	}
	if (mode & S_IRUSR) m |= HT_S_IRUSR;
	if (mode & S_IRGRP) m |= HT_S_IRGRP;
	if (mode & S_IROTH) m |= HT_S_IROTH;
	
	if (mode & S_IWUSR) m |= HT_S_IWUSR;
	if (mode & S_IWGRP) m |= HT_S_IWGRP;
	if (mode & S_IWOTH) m |= HT_S_IWOTH;
	
	if (mode & S_IXUSR) m |= HT_S_IXUSR;
	if (mode & S_IXGRP) m |= HT_S_IXGRP;
	if (mode & S_IXOTH) m |= HT_S_IXOTH;
	return m;
}

static char *next_delim(char *s, is_path_delim delim)
{
	while (*s) {
		s++;
		if (delim(*s)) return s;
	}
	return NULL;
}

static int flatten_path(char *path, is_path_delim delim)
{
	if (!path || !*path)
		return 0;
	char *q = next_delim(path, delim);
	int pp = flatten_path(q, delim);
	int ll = q ? (q-path-1) : strlen(path)-1;
	if ((ll == 2) && (ht_strncmp(path+1, "..", 2) == 0)) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
		pp++;
	} else if ((ll == 1) && (ht_strncmp(path+1, ".", 1) == 0)) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
	} else if (pp) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
		pp--;
	}
	return pp;
}

