/* 
 *	HT Editor
 *	code_analy.cc
 *
 * 	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License version 2 as
 * 	published by the Free Software Foundation.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program; if not, write to the Free Software
 * 	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "analy.h"
#include "analy_register.h"
#include "language.h"
#include "code_analy.h"
#include "htdebug.h"

void CodeAnalyser::init(Analyser *A)
{
}

int  CodeAnalyser::load(ht_object_stream *f)
{
	return 0;
}

void CodeAnalyser::done()
{
}


OBJECT_ID	CodeAnalyser::object_id() const
{
	return ATOM_CODE_ANALYSER;
}

void CodeAnalyser::store(ht_object_stream *f)
{
}

