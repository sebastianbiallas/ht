/* 
 *	HT Editor
 *	cmds.cc
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

#include "cmds.h"

#define CMD_NAME(n) {n, #n}
/*
cmd_name cmd_names[] = {
CMD_NAME(cmd_quit),
CMD_NAME(cmd_file_open),
CMD_NAME(cmd_file_new),
CMD_NAME(cmd_file_save),
CMD_NAME(cmd_file_saveas),

CMD_NAME(cmd_edit_cut),
CMD_NAME(cmd_edit_delete),
CMD_NAME(cmd_edit_copy),
CMD_NAME(cmd_edit_paste),
CMD_NAME(cmd_edit_show_clipboard),
CMD_NAME(cmd_edit_clear_clipboard),
CMD_NAME(cmd_edit_copy_from_file),
CMD_NAME(cmd_edit_paste_into_file),

CMD_NAME(cmd_window_resizemove),
CMD_NAME(cmd_window_switch_resizemove),
CMD_NAME(cmd_window_close),

CMD_NAME(cmd_popup_dialog_eval),
CMD_NAME(cmd_popup_dialog_view_list),
CMD_NAME(cmd_popup_dialog_window_list),
CMD_NAME(cmd_popup_window_log),
CMD_NAME(cmd_popup_window_help),

CMD_NAME(cmd_view_mode),
CMD_NAME(cmd_edit_mode),
CMD_NAME(cmd_view_mode_i),
CMD_NAME(cmd_edit_mode_i),
CMD_NAME(cmd_file_goto),
CMD_NAME(cmd_file_search),
CMD_NAME(cmd_file_replace),
CMD_NAME(cmd_file_blockop),
CMD_NAME(cmd_file_resize),
CMD_NAME(cmd_file_truncate),
CMD_NAME(cmd_file_extend),
{0, 0}
};
*/
