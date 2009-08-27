/* 
 *	HT Editor
 *	htctrl.cc
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

#include "htctrl.h"

static char sglobalerror[GLOBAL_ERROR_SIZE];
char *globalerror = sglobalerror;

ht_view *app;
ht_view *baseview;
SystemDisplay *screen;
char appname[HT_NAME_MAX+1];
List *virtual_fs_list;
Object *project;
int some_analyser_active = 0;
int num_ops_parsed = 0;

