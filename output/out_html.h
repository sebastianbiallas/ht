/*
 *	out_html.h
 */

/*	Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@web-productions.de)
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

#ifndef out_html_h
#define out_html_h

#include "analy.h"
#include "common.h"
#include "global.h"
#include "stream.h"

#define HTML_OUTPUT_OK 0
#define HTML_OUTPUT_ERR_GENERIC 1
#define HTML_OUTPUT_ERR_STREAM 2
#define HTML_OUTPUT_ERR_ANALY_NOT_FINISHED 3

int generate_html_output(analyser *analy, ht_stream *stream, ADDR from, ADDR to);

#endif
