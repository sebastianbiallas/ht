/* 
 *	HT Editor
 *	htdebug.cc
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

#include "global.h"
#include "htdebug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define CPU_CLOCK 500000000

void ht_assert_failed(char *file, int line, char *assertion)
{
	fprintf(stderr, "in file %s, line %d: assertion failed: %s\n", file, line, assertion);
	exit(1);
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

void ht_error(char *file, int line, char *format,...)
{
	va_list arg;
	va_start(arg, format);
	fprintf(stderr, "in file %s, line %d: error: ", file, line);
	vfprintf(stderr, format, arg);
	fputc('\n', stderr);
	va_end(arg);
	exit(1);
}

#define QWORD_ZERO(c) { c.hi=0; c.lo=0; }
#define QWORD_ISZERO(c) (!(c.hi || c.lo))

#define QWORD_ASSIGN32(c, n) { c.hi=0; c.lo=n; }

#define QWORD_EVEN(c) (!QWORD_ODD(c))
#define QWORD_ODD(c) (c.lo & 1)

int qword_cmp(qword *a, qword *b)
{
	int r=a->hi - b->hi;
	if (r) return r;
	return a->lo - b->lo;
}

void qword_shl(qword *result, qword *a, int b)
{
	result->hi=a->hi << b;
	result->hi|=a->lo >> (32-b);
	result->lo=a->lo << b;
}

void qword_shr(qword *result, qword *a, int b)
{
	result->hi=a->hi >> b;
	result->lo|=a->hi << (32-b);
	result->lo=a->lo >> b;
}

void qword_add(qword *result, qword *a, qword *b)
{
	qword r;
	r.lo = a->lo + b->lo;
	r.hi = a->hi + b->hi + ((r.lo < a->lo) ? 1 : 0);
	*result=r;
}

void qword_sub(qword *result, qword *a, qword *b)
{
	qword r;
	r.hi = a->hi - b->hi - ((a->lo < b->lo) ? 1 : 0);
	r.lo = a->lo - b->lo;
	*result=r;
}

void qword_mul(qword *result, qword *a, qword *b)
{
	qword x, y, z;
	x=*a;
	y=*b;
	QWORD_ZERO(z);
	while (!QWORD_ISZERO(y)) {
		if (QWORD_ODD(y)) qword_add(&z, &z, &x);
		qword_shl(&x, &x, 1);
		qword_shr(&y, &y, 1);
	}
	*result=z;
}

void qword_div(qword *result, qword *a, qword *b)
{
	qword x, y, q, t;
	x=*a;
	y=*b;
	QWORD_ZERO(q);
	QWORD_ASSIGN32(t, 1);
	while (qword_cmp(&y, &x) < 0) {
		qword_shl(&y, &y, 1);
		qword_shl(&t, &t, 1);
	}
	while (!QWORD_ISZERO(t)) {
		if (qword_cmp(&x, &y) >= 0) {
			qword_sub(&x, &x, &y);
			qword_add(&q, &q, &t);
		}
		qword_shr(&y, &y, 1);
		qword_shr(&t, &t, 1);
	}
	*result=q;
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
	timer_start[h].hi=0;
	timer_start[h].lo=0;
	timer_end[h].hi=0;
	timer_end[h].lo=0;
	return h;
}

void start_timer(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return;
	dword s0, s1;
	asm("rdtsc" : "=a" (s0), "=d" (s1));
	timer_start[h].hi=s1;
	timer_start[h].lo=s0;
}

void stop_timer(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return;
	dword e0, e1;
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
		p->hi=0;
		p->lo=0;
		return;
	}
	qword_sub(p, &timer_end[h], &timer_start[h]);
}

dword get_timer_sec(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return 0;
	timepoint t;
	get_timer_tick_internal(h, &t);
	qword d;
	d.hi=0;
	d.lo=CPU_CLOCK;
	qword_div(&t, &t, &d);
	if (t.hi) return 0xffffffff; else return t.lo;
}

dword get_timer_msec(timer_handle h)
{
	if ((h<0) || (h>handle_count)) return 0;
	timepoint t;
	get_timer_tick_internal(h, &t);
	qword d;
	d.hi=0;
	d.lo=CPU_CLOCK/1000;
	qword_div(&t, &t, &d);
	if (t.hi) return 0xffffffff; else return t.lo;
}

dword get_timer_tick(timer_handle h)
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

dword get_timer_sec(timer_handle handle)
{
	return 0;
}

dword get_timer_msec(timer_handle handle)
{
	return 0;
}

dword get_timer_tick(timer_handle h)
{
	return 0;
}

#endif
