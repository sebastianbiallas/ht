/* 
 *	HT Editor
 *	htsys.h (DJGPP implementation)
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

#ifndef __HTSYS_H__
#define __HTSYS_H__

#include "htio.h"
#include "global.h"

#define HT_SYS_NAME			"DJGPP"

#define SYS_SUPPORT_NATIVE_CLIPBOARD
#define SYS_NATIVE_CLIPBOARD_NAME "Windows Clipboard"

int sys_canonicalize(char *result, const char *filename);
int sys_findclose(pfind_t *pfind);
int sys_findfirst(const char *dirname, pfind_t *pfind);
int sys_findnext(pfind_t *pfind);
int sys_pstat(pstat_t *s, const char *filename);
void sys_suspend();
int sys_get_free_mem();
int sys_truncate(const char *filename, FILEOFS ofs);
int sys_deletefile(const char *filename);
bool sys_is_path_delim(char c);
int sys_filename_cmp(const char *a, const char *b);

bool sys_write_data_to_native_clipboard(const void *data, int size);
int sys_get_native_clipboard_data_size();
bool sys_read_data_from_native_clipboard(void *data, int max_size);

#include "stream.h"	// FIXME: ARGH
int sys_ipc_exec(ht_streamfile **in, ht_streamfile **out, ht_streamfile **err, int *handle, const char *cmd);
bool sys_ipc_is_valid(int handle);
int sys_ipc_terminate(int handle);

/*
 *	INIT
 */

bool init_system();

/*
 *	DONE
 */

void done_system();

#endif /* __HTSYS_H__ */
