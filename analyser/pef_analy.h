/* 
 *	HT Editor
 *	pef_analy.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#ifndef pef_analy_h
#define pef_analy_h

#include "analy.h"
#include "htpef.h"

//test
#include "pefstruc.h"

class PEFAnalyser: public Analyser {
public:
	ht_pef_shared_data 	*pef_shared;
	File			*file;
	Area			*validarea;

				PEFAnalyser();
				PEFAnalyser(BuildCtorArg&a): Analyser(a) {};
		void		init(ht_pef_shared_data *pef_shared, File *File);
		void 		load(ObjectStream &f);
	virtual	void		done();
	virtual	ObjectID	getObjectID() const;

	virtual	void		beginAnalysis();
	virtual	uint		bufPtr(Address *Addr, byte *buf, int size);
		bool		convertAddressToPEFAddress(Address *addr, PEFAddress *r);
	virtual	Address		*createAddress();
		Address		*createAddress32(uint32 addr);
		Address		*createAddress64(uint64 addr);
	virtual	Assembler 	*createAssembler();
	virtual	String &	getName(String &res);
	virtual	const char	*getType();
	virtual	void 		initCodeAnalyser();
		void		initInsertSymbols(int shidx);
	virtual	void 		initUnasm();
	virtual	void 		log(const char *msg);
	virtual	Address		*nextValid(Address *Addr);
	virtual	void		store(ObjectStream &f) const;
	virtual	int		queryConfig(int mode);
	virtual	bool 		validAddress(Address *Addr, tsectype action);
	virtual	Address		*fileofsToAddress(FileOfs fileofs);
	virtual	FileOfs		addressToFileofs(Address *Addr);
	virtual	const char	*getSegmentNameByAddress(Address *Addr);
};

#endif
