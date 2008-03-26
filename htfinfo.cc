/* 
 *	HT Editor
 *	htfinfo.cc
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

#include "htfinfo.h"
#include "snprintf.h"

#include <sys/stat.h>
#include <string.h>

ht_view *htfinfo_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_finfo_text *v = new ht_finfo_text();
	v->init(b, file);
	return v;
}

format_viewer_if htfinfo_if = {
	htfinfo_init,
	0
};

/*
 *   ht_finfo_text
 */
 
void	ht_finfo_text::init(Bounds *b, File *f)
{
	ht_statictext::init(b, FINFO_DESC, align_left, 1);
	options |= VO_BROWSABLE;
//	olddesc = desc;
//	desc =;
	file = f;
}

void	ht_finfo_text::done()
{
//	desc = olddesc;
	ht_statictext::done();
}

#define FINFO_IDENTIFIER_WIDTH 24
#define FINFO_IDENTIFIER_WIDTH_STR "24"

int print_time(char *f, int max_len, const char *prefix, time_t time)
{
	tm tt;
	memcpy(&tt, localtime(&time), sizeof tt);
	return ht_snprintf(f, max_len, "%-"FINFO_IDENTIFIER_WIDTH_STR"s%s", prefix, asctime(&tt));
}

int ht_finfo_text::gettext(char *text, int max_len)
{
	if (max_len <= 0) return 0;

	char *t = text;
	pstat_t s;
	file->pstat(s);
	
	if (s.caps & pstat_ctime) t += print_time(t, max_len-(t-text), "time of creation", s.ctime);
	if (s.caps & pstat_mtime) t += print_time(t, max_len-(t-text), "time of modification", s.mtime);
	if (s.caps & pstat_atime) t += print_time(t, max_len-(t-text), "time of last access", s.atime);
	if (s.caps & pstat_uid) t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s%d\n", "user id", s.uid);
	if (s.caps & pstat_gid) t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s%d\n", "group id", s.gid);

	if (s.caps & pstat_size) {
		t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s%qd (%.2f KiB, %.2f MiB)"
		" / 0x%08qx\n", "size", s.size, ((float)s.size)/1024,
		((float)s.size)/1024/1024, s.size);
	}	    

	if (s.caps & pstat_inode) t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s%d\n", "inode", s.fsid);
	else if (s.caps & pstat_cluster) t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s%d\n", "cluster (?)", s.fsid);
	else if (s.caps & pstat_fsid) t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s%d\n", "fsid", s.fsid);
	
	if (s.caps & pstat_mode_all) {
		uint32 am[3][3]={
			{HT_S_IRUSR, HT_S_IWUSR, HT_S_IXUSR},
			{HT_S_IRGRP, HT_S_IWGRP, HT_S_IXGRP},
			{HT_S_IROTH, HT_S_IWOTH, HT_S_IXOTH}
		};
	
		uint32 ulm[3]={pstat_mode_usr, pstat_mode_grp, pstat_mode_oth};
		const char *uls[3]={"user", "group", "world"};
	
		uint32 alm[3]={pstat_mode_r, pstat_mode_w, pstat_mode_x};
		const char *als[3]={"read", "write", "execute"};
		
		if (max_len-(t-text) > 1) *t++ = '\n';

		int cols=0;	
		t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s", "");
		for (int u=0; u<3; u++) {
			if (s.caps & ulm[u]) {
				t += ht_snprintf(t, max_len-(t-text), " %-8s", uls[u]);
				cols++;
			}
		}		
		if (max_len-(t-text) > 1) *t++ = '\n';
		
		for (int q=0; q < FINFO_IDENTIFIER_WIDTH+cols*9; q++) if (max_len-(t-text) > 1) *t++ = '-';

		if (max_len-(t-text) > 1) *t++ = '\n';

				
		for (int a=0; a<3; a++) {
			if (s.caps & alm[a]) {
				t += ht_snprintf(t, max_len-(t-text), "%-"FINFO_IDENTIFIER_WIDTH_STR"s", als[a]);
				for (int u=0; u<3; u++) {
					if (s.caps & ulm[u]) {
						t += ht_snprintf(t, max_len-(t-text), " %-8s", (s.mode & am[u][a]) ? "[x]" : "[ ]");
					}
				}
				if (max_len-(t-text) > 1) *t++ = '\n';
			}
		}
		*t = 0;
	}
	
	return t-text;
}

