/*
 *	HT Editor
 *	log.h
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

#ifndef __LOG_H__
#define __LOG_H__

void ht_logf(int color, const char *format, ...);

#define LOG_NORMAL 0
#define LOG_WARN 1
#define LOG_ERROR 2

#define LOG(a...) ht_logf(LOG_NORMAL, a);
#define LOG_EX(c, a...) ht_logf(c, a);

#endif /* __LOG_H__ */

