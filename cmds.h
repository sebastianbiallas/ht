/* 
 *	HT Editor
 *	cmds.h
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

#ifndef __CMDS_H__
#define __CMDS_H__

#include "io/types.h"

#define CMD	uint32

#define msg_command				0x80000000
#define HT_COMMAND(c)				(msg_command+(c))

#define cmd_quit				HT_COMMAND(100)
#define cmd_about				HT_COMMAND(101)

#define cmd_file_open				HT_COMMAND(102)
#define cmd_file_new				HT_COMMAND(103)
#define cmd_file_save				HT_COMMAND(104)
#define cmd_file_saveas				HT_COMMAND(105)
#define cmd_file_exec_cmd			HT_COMMAND(106)

#define cmd_edit_cut				HT_COMMAND(120)
#define cmd_edit_delete				HT_COMMAND(121)
#define cmd_edit_copy				HT_COMMAND(122)
#define cmd_edit_paste				HT_COMMAND(123)
#define cmd_edit_show_clipboard			HT_COMMAND(124)
#define cmd_edit_clear_clipboard		HT_COMMAND(125)
#define cmd_edit_copy_from_file			HT_COMMAND(126)
#define cmd_edit_paste_into_file		HT_COMMAND(127)
#define cmd_edit_copy_native			HT_COMMAND(128)
#define cmd_edit_paste_native			HT_COMMAND(129)

#define cmd_window_resizemove			HT_COMMAND(140)
#define cmd_window_switch_resizemove		HT_COMMAND(141)
#define cmd_window_close			HT_COMMAND(142)
#define cmd_window_tile_vertical		HT_COMMAND(143)
#define cmd_window_tile_horizontal		HT_COMMAND(144)

#define cmd_project_open			HT_COMMAND(160)
#define cmd_project_close			HT_COMMAND(161)
#define cmd_project_add_item			HT_COMMAND(162)
#define cmd_project_remove_item			HT_COMMAND(163)
#define cmd_project_edit_item			HT_COMMAND(164)

#define cmd_popup_dialog_eval			HT_COMMAND(180)
#define cmd_popup_dialog_view_list		HT_COMMAND(181)
#define cmd_popup_dialog_window_list		HT_COMMAND(182)
#define cmd_popup_window_log			HT_COMMAND(183)
#define cmd_popup_window_help			HT_COMMAND(184)
#define cmd_popup_window_options		HT_COMMAND(185)
#define cmd_popup_window_project		HT_COMMAND(186)
#define cmd_popup_dialog_info_loader		HT_COMMAND(187)

#define cmd_vstate_restore			HT_COMMAND(190)

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
#define cmd_file_replace			HT_COMMAND(1006)
#define cmd_file_blockop			HT_COMMAND(1007)
#define cmd_file_resize				HT_COMMAND(1008)
#define cmd_file_truncate			HT_COMMAND(1009)
#define cmd_file_extend				HT_COMMAND(1010)

/*
 *	hex 1500 - 1999
 */
 
#define cmd_hex_entropy				HT_COMMAND(1500)
#define cmd_hex_display_bytes			HT_COMMAND(1501)
#define cmd_hex_display_disp			HT_COMMAND(1502)

/*
 *	disasm 2000 - 2499
 */
 
#define cmd_disasm_call_assembler		HT_COMMAND(2000)
#define cmd_disasm_toggle1632			HT_COMMAND(2001)

#endif /* __CMDS_H__ */
