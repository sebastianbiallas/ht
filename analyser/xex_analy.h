/* 
 *	HT Editor
 *	xex_analy.h
 *
 *	Copyright (C) 2006 Sebastian Biallas (sb@biallas.net)
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

#ifndef xex_analy_h
#define xex_analy_h

#include "analy.h"
#include "htxex.h"


class XEXAnalyser: public Analyser {
public:
	ht_xex_shared_data 	*xex_shared;
	File 			*file;
	Area			*validarea;

				XEXAnalyser() {};
				XEXAnalyser(BuildCtorArg&a): Analyser(a) {};
		void		init(ht_xex_shared_data *XEX_shared, File *File);
		void 		load(ObjectStream &f);
		void		reinit(ht_xex_shared_data *XEX_shared, File *File);
	virtual	void		done();
	virtual	ObjectID	getObjectID() const;

	virtual	void		beginAnalysis();
	virtual	uint		bufPtr(Address *Addr, byte *buf, int size);
		bool		convertAddressToRVA(Address *addr, RVA *r);
	virtual	Address		*createAddress();
		Address		*createAddress32(uint32 addr);
	virtual	String &	getName(String &res);
	virtual const char	*getType();
	virtual	void 		initUnasm();
	virtual	Address		*nextValid(Address *Addr);
	virtual	void		store(ObjectStream &f) const;
	virtual	bool 		validAddress(Address *Addr, tsectype action);
	virtual	Address		*fileofsToAddress(FileOfs fileofs);
	virtual	FileOfs		addressToFileofs(Address *Addr);
	virtual	const char	*getSegmentNameByAddress(Address *Addr);
};

#endif
