/* 
 *	HT Editor
 *	code_analy.h
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

#ifndef CODE_ANALY_H
#define CODE_ANALY_H

#include "global.h"
#include "common.h"
#include "stream.h"
#include "analy.h"

class Analyser;

class CodeAnalyser: public Object {
public:
	Analyser		*a;
			void      	init(Analyser *A);
			int 			load(ht_object_stream *f);
	virtual	void 		done();
	virtual	OBJECT_ID		object_id();

	virtual	void			store(ht_object_stream *f);
};

#endif
