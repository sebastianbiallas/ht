/* 
 *	HT Editor
 *	class_analy.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#ifndef class_analy_h
#define class_analy_h

#include "analy.h"
#include "class.h"


class ClassAnalyser: public Analyser {
public:
	ht_class_shared_data 	*class_shared;
	ht_streamfile 		*file;
	Area				*validarea;

			void		init(ht_class_shared_data *Class_shared, ht_streamfile *File);
			int 		load(ht_object_stream *f);
	virtual	void		done();
	virtual	OBJECT_ID	object_id();

	virtual	void		beginAnalysis();
	virtual	UINT		bufPtr(Address *Addr, byte *buf, int size);
			bool		convertAddressToRVA(Address *addr, dword *r);
	virtual	Address	*createAddress();
			Address	*createAddress32(dword addr);
	virtual   Assembler *createAssembler();
	virtual	char		*getName();
	virtual   char		*getType();
	virtual	void 	initCodeAnalyser();
	virtual	void 	initUnasm();
	virtual	void 	log(const char *msg);
	virtual	Address	*nextValid(Address *Addr);
	virtual	void		store(ht_object_stream *f);
	virtual	int		queryConfig(int mode);
	virtual	bool 	validAddress(Address *Addr, tsectype action);
	virtual	Address	*fileofsToAddress(FILEOFS fileofs);
	virtual	FILEOFS	addressToFileofs(Address *Addr);
	virtual	char		*getSegmentNameByAddress(Address *Addr);
};

#endif
