/* 
 *	HT Editor
 *	file.cc
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

#include "file.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

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


#if defined(WIN32) || defined(__WIN32__)
# include <windows.h>
# include <winreg.h>
#endif
//-----------------------------------------------------------------------------
static char *_str_strip(char *str, int len)
{
	char *pr =  ht_malloc(len + 1);
	pr[len] = 0;
	memcpy(pr, str, len);
	return pr;
}
//-----------------------------------------------------------------------------
/*
 1. char *s            - old string
 2. unsigned int s_len - old string len
 3. char *r            - new string
 4. unsigned int r_len - new string len
 5. unsigned int at    - offset in old string
 6. unsigned int len   - len of substring for delete ( at - start)
*/
static char *_substr_replace(char *s, unsigned int s_len, char *r, unsigned int r_len,\
                                            unsigned int at, unsigned int len)
{
	if (!s_len || !r_len) return NULL;
	if (at > s_len) at = 0;
	unsigned int nlen = s_len + r_len - len;
	char *out = ht_malloc(nlen+1);
	out[nlen] = 0; 
	memcpy(out, s, at);
	memcpy(out+at, r, r_len);
	while (s_len-- > (at + len)) {
		out[--nlen] = s[s_len];
	}
	return out;
}
//-----------------------------------------------------------------------------
static char *_env_replace(char *str, unsigned int len)
{
	bool e = false;
	unsigned int cc = 0; // count
	unsigned int sp = 0; // start pos
	char *p = NULL;
	char *prt = NULL;
	char *t;
	char *t2;
	while (cc != len) {
		t2 = prt?prt:str;
#if defined(WIN32) || defined(__WIN32__) || defined(DJGPP) || defined(MSDOC)
		if (t2[cc] == '%' || t2[cc] == ' ') {
#else
		if (t2[cc] == '$' || t2[cc] == ' ') {
#endif
			if (e) {
				free(p);
				p = _str_strip(t2 + sp, cc - sp);
				if ((t = getenv(p)) != NULL) {
					prt = _substr_replace(t2, prt?strlen(prt):strlen(str), t, strlen(t),
                                                          sp-1, (cc - sp) + 2);
					if (t2 != str) free(t2);
					len = strlen(prt);
					cc = sp;
				}
				e = false;
			} else {
				if (t2[cc] != ' ') {
					sp = cc + 1;
					e = true;
				}
			}
		}
		cc++;
	}
	if (prt == NULL) {
		prt = ht_malloc(len+1);
		prt[len] = 0;	
		memcpy(prt, str, len);
	}
	return prt;
}
//-----------------------------------------------------------------------------
int sys_basename(char *result, const char *filename)
{
	// FIXME: use is_path_delim
	const char *slash1 = strrchr(filename, '/');
	const char *slash2 = strrchr(filename, '\\');
	const char *slash = (slash1 > slash2) ? slash1 : slash2;
	if (slash) {
		int l = strlen(filename);
		ht_strlcpy(result, slash+1, l-(slash-filename)+1);
		return 0;
	}
	strcpy(result, filename);
	return 0;
}

char *sys_dirname(char *path)
{
	char *r;           // result
	char *ptr = path;
	int c = 0;
	int last = 0;
	do {
		c++;
		if (*ptr == '/' || *ptr == '\\') {
			last = c;
		} else if (*ptr == 0) {
			r = ht_malloc(last + 1);
			strncpy(r, path, last);
			r[last] = 0;
			return r;
		}
	} while (*ptr++);
	return NULL;
}

/* filename and pathname must be canonicalized */
int sys_relname(char *result, const char *filename, const char *cwd)
{
	const char *f = filename, *p = cwd;
	while ((*f == *p) && (*f)) {
		f++;
		p++;
	}
	if (*f == '/') f++;
	const char *last = f, *h = f;
	while (*h) {
		if (*h == '/') {
			*(result++) = '.';
			*(result++) = '.';
			*(result++) = '/';
			last = h+1;
		}
		h++;
	}
	while (f<last) {
		*(result++) = *f;
		f++;
	}
	*result = 0;
	strcat(result, last);
	return 0;
}

char *sys_get_home_dir(void)
{
#if defined(WIN32) || defined(__WIN32__)
	HKEY hKey;
	char path[HT_NAME_MAX];
	/*
	 LONG len = HT_NAME_MAX;
	 if(RegOpenKey(HKEY_CURRENT_USER,
	 "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",
	                                                      &hKey) == ERROR_SUCCESS){
	  if(RegQueryValue(hKey, "Personal", path, &len) == ERROR_SUCCESS){
	   RegCloseKey(hKey);
	   if(len > HT_NAME_MAX) return NULL;
	   else return _env_replace(path, len);
	  }
	 }
	*/
	// not work on Windows 3.1 ?
	DWORD len = HT_NAME_MAX;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",
                0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS) {
		
		if (RegQueryValueEx(hKey, "Personal", NULL, NULL, (LPBYTE)path, &len) ==
                                                                ERROR_SUCCESS) {
			RegCloseKey(hKey);
			if (len > HT_NAME_MAX) {
				return NULL;
			} else {
				return _env_replace(path, len);
			}
		}
	}
#elif defined(MSDOS) 
	// -?
#elif defined(DJGPP)
	// unknown...
#else 
	char *path = getenv("HOME");
	if (path) return _env_replace(path, strlen(path));
#endif
	return NULL;
}

int sys_file_mode(int mode)
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
	if (ll == 2 && ht_strncmp(path+1, "..", 2) == 0) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
		pp++;
	} else if (ll == 1 && ht_strncmp(path+1, ".", 1) == 0) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
	} else if (pp) {
		if (q) memmove(path, q, strlen(q)+1); else *path = 0;
		pp--;
	}
	return pp;
}

bool sys_path_is_absolute(const char *filename, is_path_delim delim)
{
	return delim(filename[0]) || (isalpha(filename[0]) && (filename[1] == ':'));
}

int sys_common_canonicalize(char *result, const char *filename, const char *cwd, is_path_delim delim)
{
	char *o = result;
	if (!sys_path_is_absolute(filename, delim)) {
		if (cwd) strcpy(o, cwd); else return EINVAL;
		int ol = strlen(o);
		if (ol && !delim(o[ol-1])) {
			o[ol] = '/';
			o[ol+1] = 0;
		}
	} else *o = 0;
	strcat(o, filename);
	int k = flatten_path(o, delim);
	return (k == 0) ? 0 : EINVAL;
}

const char *sys_filename_suffix(const char *fn)
{
	const char *s = NULL;
	while (fn && *fn) {
		if (sys_is_path_delim(*fn)) s = fn+1;
		fn++;
	}
	const char *p = s ? strrchr(s, '.') : NULL;
	return p ? p+1 : NULL;
}

int sys_tmpfile_fd()
{
#if 1
	// FIXME: this might leak something...
	FILE *f = tmpfile();
	return fileno(f);
#else
	// is this better ?
	return open(tmpnam(NULL), O_RDWR | O_EXCL | O_CREAT);
#endif
}
