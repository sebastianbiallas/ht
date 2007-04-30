/* 
 *	HT Editor
 *	sysinit.cc - POSIX-specific initialization
 *
 *	Copyright (C) 1999-2004 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "io/sys.h"

bool initKeyb();
void doneKeyb();

bool initSysEvent();
void doneSysEvent();

struct sigaction old_SIGTRAP;
struct sigaction old_SIGWINCH;

void SIGCHLD_sigaction(int i, siginfo_t *info, void *v);
void SIGTRAP_sigaction(int i, siginfo_t *info, void *v);
void SIGWINCH_sigaction(int i, siginfo_t *info, void *v);

bool init_system()
{
	setuid(getuid());
	struct sigaction sa;
#if 0

	sa.sa_sigaction = SIGCHLD_sigaction;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_sigaction = SIGTRAP_sigaction;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGTRAP, &sa, &old_SIGTRAP);
#endif
	sa.sa_sigaction = SIGWINCH_sigaction;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGWINCH, &sa, &old_SIGWINCH);
	return true;
}

void done_system()
{
}
