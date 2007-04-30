/* 
 *	HT Editor
 *	code_analy.h
 *
 * 	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#include "data.h"
#include "stream.h"
#include "analy.h"

class Analyser;

class CodeAnalyser: public Object {
public:	
	Analyser	*a;
	
				CodeAnalyser();
				CodeAnalyser(BuildCtorArg&a): Object(a) {};
		void      	init(Analyser *A);
	virtual	void		load(ObjectStream &s);
	virtual	void 		done();
	virtual	ObjectID	getObjectID() const;

	virtual	void		store(ObjectStream &s) const;
};

#endif
