/* 
 *	HT Editor
 *	htcfg.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTCFG_H__
#define __HTCFG_H__

#include "common.h"
#include "stream.h"

#if defined(WIN32) || defined(__WIN32__) || defined(MSDOS) || defined(DJGPP)
#define PROJECT_CONFIG_FILE_NAME "ht.cfg"
#else
#define PROJECT_CONFIG_FILE_NAME ".htcfg"
#endif

enum loadstore_result {
	LS_OK,
	LS_ERROR_NOT_FOUND,
	LS_ERROR_READ,
	LS_ERROR_WRITE,
	LS_ERROR_MAGIC,
	LS_ERROR_VERSION,             // sets error_info to version
	LS_ERROR_FORMAT,
	LS_ERROR_CORRUPTED
};

/*	PROJECT CONFIG FILE VERSION HISTORY
 *	Version 2: HT 0.4.4
 *	Version 3: HT 0.4.5
 *	Version 4: HT 0.5.0
 *	Version 5: HT 0.5.1
 */

#define ht_projectconfig_magic			"HTCP"
#define ht_projectconfig_fileversion		5

/*	FILE CONFIG FILE VERSION HISTORY
 *	Version 1: HT 0.5.0
 *	Version 2: HT 0.5.1
 */

#define ht_fileconfig_magic				"HTCF"
#define ht_fileconfig_fileversion			2

extern char *projectconfig_file;
loadstore_result save_projectconfig();
bool load_projectconfig(loadstore_result *result, int *error_info);

typedef int (*load_fcfg_func)(ht_object_stream *f, void *context);
typedef void (*store_fcfg_func)(ht_object_stream *f, void *context);

loadstore_result save_fileconfig(char *fileconfig_file, store_fcfg_func store_func, void *context);
loadstore_result load_fileconfig(char *fileconfig_file, load_fcfg_func load_func, void *context, int *error_info);

/*
 *	INIT
 */

bool init_cfg();

/*
 *	DONE
 */

void done_cfg();

#endif /* !__HTCFG_H__ */
