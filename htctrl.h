/*
 *	HT Editor
 *	htctrl.h
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

#ifndef __HTCTRL_H__
#define __HTCTRL_H__

#include "data.h"
#include "htobj.h"

#define GLOBAL_ERROR_SIZE 1024
extern char *globalerror;

extern ht_view *app;
extern ht_view *baseview;
extern SystemDisplay *screen;
extern List *virtual_fs_list;
extern Object *project;
extern char appname[HT_NAME_MAX+1];
extern int some_analyser_active;
extern int num_ops_parsed;

#endif /* __HTCTRL_H__ */
