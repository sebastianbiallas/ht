/* 
 *	HT Editor
 *	htdebug.cc
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

#include "htdebug.h"

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// FIXME: auto-detect ?
#define CPU_CLOCK 500000000

void ht_assert_failed(const char *file, int line, const char *assertion)
{
	fprintf(stderr, "in file %s, line %d: assertion failed: %s\n", file, line, assertion);
#ifndef WIN32
#if 1
	fprintf(stderr, "sending SIGTRAP...");
	raise(SIGTRAP);
#endif
#endif
	exit(1);
}

void ht_error(const char *file, int line, const char *format,...)
{
	va_list arg;
	va_start(arg, format);
	fprintf(stderr, "in file %s, line %d: error: ", file, line);
	vfprintf(stderr, format, arg);
	fputc('\n', stderr);
	va_end(arg);
	exit(1);
}

void ht_trace(char *file, int line, char *format,...)
{
	FILE *ht_debug_file = stderr;
	va_list arg;
	va_start(arg, format);
	fprintf(ht_debug_file, "TRACE: %s.%d: ", file, line);
	vfprintf(ht_debug_file, format, arg);
	fputc('\n', ht_debug_file);
	va_end(arg);
}

void ht_warn(char *file, int line, char *format,...)
{
	va_list arg;
	va_start(arg, format);
	fprintf(stderr, "in file %s, line %d: warning: ", file, line);
	vfprintf(stderr, format, arg);
	fputc('\n', stderr);
	va_end(arg);
}

#ifdef DJGPP

#define MAX_TIMERS 20

timepoint timer_start[MAX_TIMERS];
timepoint timer_end[MAX_TIMERS];
int handle[MAX_TIMERS];
int handle_count=0;

timer_handle getfreetimerhandle()
{
	timer_handle i;
	for (i=0; i<handle_count; i++) {
		if (!handle[i]) return i;
	}
	return -1;
}

timer_handle new_timer()
{
	int h=getfreetimerhandle();
	if (h==-1) {
		if (handle_count<MAX_TIMERS) {
			h=handle_count++;
		} else return -1;
	}
	handle[h]=1;
	timer_start[h] = to_qword(0);
	timer_end[h] = to_qword(0);
	return h;
}

void start_timer(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return;
	uint32 s0, s1;
	asm("rdtsc" : "=a" (s0), "=d" (s1));
	timer_start[h].hi=s1;
	timer_start[h].lo=s0;
}

void stop_timer(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return;
	uint32 e0, e1;
	asm("rdtsc" : "=a" (e0), "=d" (e1));
	timer_end[h].hi=e1;
	timer_end[h].lo=e0;
}

void delete_timer(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return;
	handle[h]=0;
}

void get_timer_tick_internal(timer_handle h, timepoint *p)
{
	if ((h<0) || (h>handle_count)) {
		*p = to_qword(0);
		return;
	}
	*p = timer_end[h] - timer_start[h];
}

uint32 get_timer_sec(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return 0;
	timepoint t;
	get_timer_tick_internal(h, &t);
	t = t / to_qword(CPU_CLOCK);
	if (t.hi) return 0xffffffff; else return t.lo;
}

uint32 get_timer_msec(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return 0;
	timepoint t;
	get_timer_tick_internal(h, &t);
	t = t / to_qword(CPU_CLOCK/1000);
	if (t.hi) return 0xffffffff; else return t.lo;
}

uint32 get_timer_tick(timer_handle h)
{
	timepoint t;
	get_timer_tick_internal(h, &t);
	return t.lo;
}

#else

timer_handle new_timer()
{
	return -1;
}

void start_timer(timer_handle handle)
{
}

void stop_timer(timer_handle handle)
{
}

void delete_timer(timer_handle handle)
{
}

uint32 get_timer_sec(timer_handle handle)
{
	return 0;
}

uint32 get_timer_msec(timer_handle handle)
{
	return 0;
}

uint32 get_timer_tick(timer_handle h)
{
	return 0;
}

#endif
