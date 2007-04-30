/* 
 *	HT Editor
 *	htdebug.h
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

#ifndef __HTDEBUG_H__
#define __HTDEBUG_H__

#include <stdio.h>

#include "io/types.h"

/****************************************************************************/
#define HTDEBUG
/****************************************************************************/

void ht_assert_failed(const char *file, int line, const char *assertion);
void ht_error(const char *file, int line, const char *format,...);
void ht_trace(const char *file, int line, const char *format,...);
void ht_warn(const char *file, int line, const char *format,...);

typedef uint64 timepoint;

typedef int timer_handle;

timer_handle new_timer();
void start_timer(timer_handle handle);
void stop_timer(timer_handle handle);
void delete_timer(timer_handle handle);

uint get_timer_sec(timer_handle handle);
uint get_timer_msec(timer_handle handle);
uint get_timer_tick(timer_handle h);

#define HT_ERROR(a...) ht_error(__FILE__, __LINE__, a)
#define HT_WARN(a...) ht_warn(__FILE__, __LINE__, a)

#ifdef HTDEBUG
#define HT_TRACE(a...) ht_trace(__FILE__, __LINE__, a)
#else
#define HT_TRACE(a...) ((void)0)
#endif

#ifdef assert
#undef assert
#endif

#ifdef HTDEBUG
#	define assert(a) if (!(a)) ht_assert_failed(__FILE__, __LINE__, (#a));
#else
#	define assert(a) ((void)0)
#endif

#define ASSERT(a) assert(a)

#define HERE __FILE__, __LINE__

#endif /* !__HTDEBUG_H__ */

