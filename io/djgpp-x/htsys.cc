/* 
 *	HT Editor
 *	htsys.cc (DJGPP implementation)
 *
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

#include "htsys.h"

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
	s->caps=pstat_size | pstat_mtime | pstat_mode_type | pstat_mode_usr;
	s->size=f->ff_fsize;
	s->mtime=dostime2time_t(f->ff_ftime, f->ff_fdate);
	s->mode=HT_S_IRUSR;
	if (f->ff_attrib & FA_DIREC) {
		s->mode|=HT_S_IFDIR;
	} else {
		s->mode|=HT_S_IFREG;
	}
	if (!(f->ff_attrib & FA_RDONLY)) {
		s->mode|=HT_S_IWUSR;
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
	s->size=st.st_size;
	s->size_high=0;
	s->fsid=st.st_ino;
	return 0;
}

void sys_suspend()
{
	__dpmi_yield();
}

int sys_get_free_mem()
{
	_go32_dpmi_meminfo info;
	_go32_dpmi_get_free_memory_information(&info);
	return info.available_memory;
}

int sys_truncate(const char *filename, FILEOFS ofs)
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

int sys_ipc_exec(ht_streamfile **in, ht_streamfile **out, ht_streamfile **err, int *handle, const char *cmd)
{
	int save_stdout = dup(STDOUT_FILENO);
	int save_stderr = dup(STDERR_FILENO);
	int fdout = sys_tmpfile();
	int fderr = sys_tmpfile();
	dup2(fdout, STDOUT_FILENO);
	dup2(fderr, STDERR_FILENO);
	/*int r = */system(cmd);
	dup2(save_stdout, STDOUT_FILENO);
	dup2(save_stderr, STDERR_FILENO);
	close(save_stdout);
	close(save_stderr);
	lseek(fdout, 0, SEEK_SET);
	lseek(fderr, 0, SEEK_SET);
	ht_null_file *nf = new ht_null_file();
	nf->init();
	*in = nf;
	ht_sys_file *sf;
	sf = new ht_sys_file();
	sf->init(fdout, true, FAM_READ);
	*out = sf;
	sf = new ht_sys_file();
	sf->init(fderr, true, FAM_READ);
	*err = sf;
	return 0;
}

bool sys_ipc_is_valid(int handle)
{
// no multitasking, never valid
	return false;
}

int sys_ipc_terminate(int handle)
{
// do nothing
	return 0;
}

/*
 *	Clipboard functions
 */
 
static bool open_clipboard()
{
	__dpmi_regs r;
	r.x.ax = 0x1700;   // version
	__dpmi_int(0x2f, &r);
	if (r.x.ax == 0x1700) return false;
	r.x.ax = 0x1701;  // open
	__dpmi_int(0x2f, &r);     
	return (r.x.ax != 0);
}

static void close_clipboard()
{
	__dpmi_regs r;
	r.x.ax = 0x1708;
	__dpmi_int(0x2f, &r);
}

bool sys_write_data_to_native_clipboard(const void *data, int size)
{
	if (size > 0xffff) return false;
	if (!open_clipboard()) return false;
	int sel;
	word seg = __dpmi_allocate_dos_memory((size+15)>>4, &sel);
	if (seg == 0xffff) {
		close_clipboard();
		return false;
	}
	dosmemput(data, size, seg*16);
	
	__dpmi_regs r;
	r.x.ax = 0x1703;
	
	r.x.dx = 0x01; // text
	r.x.es = seg;
	r.x.bx = 0;
	r.x.si = size >> 16;
	r.x.cx = size & 0xffff;
	
	__dpmi_int(0x2f, &r);
	__dpmi_free_dos_memory(sel);
	close_clipboard();
	return (r.x.ax != 0);
}

int sys_get_native_clipboard_data_size()
{
	return 10000;
	if (!open_clipboard()) return 0;
	__dpmi_regs r;
	r.x.ax = 0x1704;
	r.x.dx = 0x07; // text
	__dpmi_int(0x2f, &r);
	close_clipboard();
	return ((dword)r.x.dx)<<16+r.x.ax;
}

bool sys_read_data_from_native_clipboard(void *data, int max_size)
{
	int dz = sys_get_native_clipboard_data_size();
	if (!open_clipboard()) return false;
	if (!dz) {
		close_clipboard();
		return false;
	}
	int sel;
	word seg = __dpmi_allocate_dos_memory((dz+15)>>4, &sel);
	if (seg == 0xffff) {
		close_clipboard();
		return false;
	}
	
	__dpmi_regs r;
	r.x.ax = 0x1705;
	r.x.dx = 0x1;
	r.x.es = seg;
	r.x.bx = 0;
	__dpmi_int(0x2f, &r);
	if (r.x.ax) {
		dosmemget(seg*16, MIN(max_size, dz), data);
	}
	__dpmi_free_dos_memory(sel);
	close_clipboard();
	return (r.x.ax != 0);
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

