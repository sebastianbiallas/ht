/* 
 *	HT Editor
 *	coff_analy.h
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

#ifndef coff_analy_h
#define coff_analy_h

#include "analy.h"
#include "htcoff.h"


class CoffAnalyser: public Analyser {
public:
	ht_coff_shared_data 	*coff_shared;
	ht_streamfile 			*file;
	Area					*validarea;

			void			init(ht_coff_shared_data *Coff_shared, ht_streamfile *File);
			int 			load(ht_object_stream *f);
	virtual	void			done();
	virtual	OBJECT_ID		object_id() const;

	virtual	void			beginAnalysis();
	virtual	UINT			bufPtr(Address *Addr, byte *buf, int size);
			bool			convertAddressToRVA(Address *addr, RVA *r);
	virtual	Address		*createAddress();
			Address		*createAddress32(dword addr);
	virtual   Assembler 	*createAssembler();
	virtual	const char	*getName();
	virtual	char			*getSegmentNameByAddress(Address *Addr);
	virtual   const char	*getType();
	virtual	void 		initCodeAnalyser();
	virtual	void 		initUnasm();
	virtual	void 		log(const char *msg);
	virtual	Address		*nextValid(Address *Addr);
	virtual	void			store(ht_object_stream *f);
	virtual	int			queryConfig(int mode);
	virtual	Address		*fileofsToAddress(FILEOFS fileofs);
	virtual	FILEOFS		addressToFileofs(Address *Addr);
	virtual	bool 		validAddress(Address *Addr, tsectype action);
};

#endif
