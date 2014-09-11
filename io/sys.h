/*
 *	HT Editor
 *	sys.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __SYS_H__
#define __SYS_H__

/* system-dependant (implementation in $MYSYSTEM/ *.cc) */
/* Note: all functions only take absolute dir/filenames ! */
#define	SYSCAP_IPC			1
#define	SYSCAP_NBIPC			2
#define	SYSCAP_NATIVE_CLIPBOARD		4
int		sys_get_caps();

int		sys_ht_mode(int mode);

void		sys_get_driver_desc(char *buf, int len);

// return time slice to system
void		sys_suspend();

int		sys_native_clipboard_read(void *buf, int bufsize);
bool		sys_native_clipboard_write(const void *buf, int bufsize);
int		sys_native_clipboard_get_size();
const char *	sys_native_clipboard_name();

const char *	sys_get_name();

#if 1
class File;

#define		SPE_STDERR_TO_STDOUT	1

int sys_child_create(File **in, File **out, File **err,
		int &process_handle, int options,
		const char *filename, char* const* argv);
// FIXME: creates empty subshell if filename == NULL
int sys_child_create_in_subshell(char *ttyname_32bytes,
		int &tty_fd, int &process_handle,
		const char *filename, char* const *argv);
bool sys_child_is_running(int process_handle);
int  sys_child_terminate(int process_handle);

#endif

bool init_system();
void done_system();

#endif /* __SYS_H__ */
