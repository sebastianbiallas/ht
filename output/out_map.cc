/*
 *	HT Editor
 *	out_map.cc
 *
 *	Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@biallas.net)
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
#include "out_map.h"

#define STREAM_WRITE_STR(s, str) s->write((void*)str, strlen(str))

bool output_map_file(ht_stream *s, Analyser *analy)
{
	assert(s);
	assert(analy);
	/* header */

	char c=' ';
	s->write(&c, 1);
	STREAM_WRITE_STR(s, analy->get_name());
	c='\n';
	s->write(&c, 1);
	s->write(&c, 1);
	STREAM_WRITE_STR(s, " Start         Length     Name\n");
	STREAM_WRITE_STR(s, " 0000:00000000 00004800H CODE\n\n");
	
	tlabel *sym = analy->enum_labels(NULL);
	STREAM_WRITE_STR(s, "  Address         Publics by Value\n");
	
}
 
