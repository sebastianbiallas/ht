/* 
 *	HT Editor
 *	htexcept.cc
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

#include <stdio.h>
#include <stdarg.h>

#include "htexcept.h"
#include "snprintf.h"

/*
 *	CLASS ht_exception
 */

ht_exception::ht_exception()
{
}

const char *ht_exception::what()
{
	return NULL;
}

/*
 *	CLASS ht_msg_exception
 */

ht_msg_exception::ht_msg_exception(char *e,...)
{
	va_list va;
	va_start(va, e);
	ht_vsnprintf(estr, sizeof estr, e, va);
	va_end(va);
}

const char *ht_msg_exception::what()
{
	return estr;
}
