/* 
 *	HT Editor
 *	cmds.h
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

#ifndef __CMDS_H__
#define __CMDS_H__

#include "global.h"

#define CMD	dword

#define msg_command					0x80000000
#define HT_COMMAND(c)				(msg_command+(c))

#define cmd_quit					HT_COMMAND(100)
#define cmd_about					HT_COMMAND(101)

#define cmd_file_open				HT_COMMAND(102)
#define cmd_file_new				HT_COMMAND(103)
#define cmd_file_save				HT_COMMAND(104)
#define cmd_file_saveas				HT_COMMAND(105)

#define cmd_edit_cut				HT_COMMAND(106)
#define cmd_edit_delete				HT_COMMAND(107)
#define cmd_edit_copy				HT_COMMAND(108)
#define cmd_edit_paste				HT_COMMAND(109)
#define cmd_edit_show_clipboard		HT_COMMAND(110)
#define cmd_edit_clear_clipboard		HT_COMMAND(111)
#define cmd_edit_copy_from_file		HT_COMMAND(112)
#define cmd_edit_paste_into_file		HT_COMMAND(113)

#define cmd_window_resizemove			HT_COMMAND(114)
#define cmd_window_switch_resizemove	HT_COMMAND(115)
#define cmd_window_close				HT_COMMAND(116)

#define cmd_popup_dialog_eval			HT_COMMAND(117)
#define cmd_popup_dialog_wlist		HT_COMMAND(118)
#define cmd_popup_window_log			HT_COMMAND(119)
#define cmd_popup_window_help			HT_COMMAND(120)
#define cmd_popup_window_options		HT_COMMAND(121)

#define cmd_analyser_save			HT_COMMAND(200)

/*
 *	htanaly  500- 999
 */

/*
 *	htformat 1000- 1499
 */
 
#define cmd_view_mode				HT_COMMAND(1000)
#define cmd_edit_mode				HT_COMMAND(1001)
#define cmd_view_mode_i				HT_COMMAND(1002)
#define cmd_edit_mode_i				HT_COMMAND(1003)
#define cmd_file_goto				HT_COMMAND(1004)
#define cmd_file_search				HT_COMMAND(1005)
#define cmd_file_replace				HT_COMMAND(1006)
#define cmd_file_blockop				HT_COMMAND(1007)
#define cmd_file_resize				HT_COMMAND(1008)
#define cmd_file_truncate			HT_COMMAND(1009)
#define cmd_file_extend				HT_COMMAND(1010)

/*
 *	hex 1500 - 1999
 */
 
#define cmd_hex_entropy				HT_COMMAND(1500)

/*
 *	disasm 2000 - 2499
 */
 
#define cmd_disasm_call_assembler		HT_COMMAND(2000)

typedef bool (*cmd_handler)(htmsg_param *data1, htmsg_param *data2);

struct cmd_rec {
	CMD cmd;
	bool enabled;
	bool dynamic;
	cmd_handler handler;
};

struct cmd_name {
	CMD cmd;
	char *name;
};

extern cmd_name cmd_names[];

#endif /* __CMDS_H__ */
