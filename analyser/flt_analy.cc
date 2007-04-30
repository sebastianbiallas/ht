/* 
 *	HT Editor
 *	flt_analy.cc
 *
 *	Copyright (C) 1999-2003 Sebastian Biallas (sb@biallas.net)
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

#include "analy.h"
#include "analy_alpha.h"
#include "analy_names.h"
#include "analy_ia64.h"
#include "analy_ppc.h"
#include "analy_register.h"
#include "analy_x86.h"
#include "flt_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htflt.h"
#include "strtools.h"
#include "pestruct.h"
#include "snprintf.h"
//#include "x86asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
void FLTAnalyser::init(ht_flt_shared_data *Flt_shared, File *File)
{
	flt_shared = Flt_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();
}

void FLTAnalyser::beginAnalysis()
{
	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);
	
	/*
	 *	give all sections a descriptive comment:
	 */

	Address *secaddr;
	secaddr = createAddress32(flt_shared->code_start);
	if (validAddress(secaddr, scvalid)) {
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, "; start of code");
		addComment(secaddr, 0, ";******************************************************************");
	}
	delete secaddr;
	secaddr = createAddress32(flt_shared->data_start);
	if (validAddress(secaddr, scvalid)) {
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, "; start of data");
		addComment(secaddr, 0, ";******************************************************************");
	}
	delete secaddr;
	secaddr = createAddress32(flt_shared->bss_start);
	if (validAddress(secaddr, scvalid)) {
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, "; start of bss");
		addComment(secaddr, 0, ";******************************************************************");
	}
	delete secaddr;
	Address *secaddr1, *secaddr2;
	secaddr1 = createAddress32(flt_shared->code_start);
	secaddr2 = createAddress32(flt_shared->bss_end);
	validarea->add(secaddr1, secaddr2);
	delete secaddr1;
	delete secaddr2;

	/*
	 *	entrypoint
	 */
	 
	Address *entry;
	entry = createAddress32(flt_shared->header.entry);
	pushAddress(entry, entry);
	assignSymbol(entry, "entrypoint", label_func);
	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	addComment(entry, 0, "; entrypoint");
	addComment(entry, 0, ";****************************");
	delete entry;

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	
	Analyser::beginAnalysis();
}

/*
 *
 */
void FLTAnalyser::load(ObjectStream &f)
{
	GET_OBJECT(f, validarea);
	Analyser::load(f);
}

/*
 *
 */
void FLTAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

ObjectID FLTAnalyser::getObjectID() const
{
	return ATOM_FLT_ANALYSER;
}

/*
 *
 */
uint FLTAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
/*     if (ofs == INVALID_FILE_OFS) {
		int as = 1;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool FLTAnalyser::convertAddressToFLTAddress(Address *addr, FLTAddress *r)
{
	if (addr->getObjectID()==ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr;
		return true;
	} else {
		return false;
	}
}

Address *FLTAnalyser::createAddress()
{
	return new AddressFlat32();
}

Address *FLTAnalyser::createAddress32(uint32 addr)
{
	return new AddressFlat32(addr);
}

/*
 *
 */
Assembler *FLTAnalyser::createAssembler()
{
	return NULL;
}

/*
 *
 */
FileOfs FLTAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FLTAddress ea;
		if (!convertAddressToFLTAddress(Addr, &ea)) return INVALID_FILE_OFS;
		return (FileOfs)ea;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
const char *FLTAnalyser::getSegmentNameByAddress(Address *Addr)
{
	FLTAddress ea;
	if (!convertAddressToFLTAddress(Addr, &ea)) return NULL;
	if (ea >= flt_shared->code_start) {
     	if (ea >= flt_shared->data_start) {
			if (ea >= flt_shared->bss_start) {
				if (ea >= flt_shared->bss_end) {
					return NULL;
				}
				return "bss";
			}
			return "data";
		}
		return "code";
	}
	return NULL;
}

/*
 *
 */
String &FLTAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *FLTAnalyser::getType()
{
	return "FLAT/Analyser";
}

/*
 *
 */
void FLTAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void FLTAnalyser::initUnasm()
{
	DPRINTF("flt_analy: ");
//	DPRINTF("initing analy_ppc_disassembler\n");
	analy_disasm = NULL;
//	((AnalyPPCDisassembler*)analy_disasm)->init(this);
}

/*
 *
 */
void FLTAnalyser::log(const char *msg)
{
	/*
	 *	log() does to much traffic so dont log
	 *   perhaps we reactivate this later
	 *
	 */
/*	LOG(msg);*/
}

/*
 *
 */
Address *FLTAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void FLTAnalyser::store(ObjectStream &f) const
{
	PUT_OBJECT(f, validarea);
	Analyser::store(f);
}

/*
 *
 */
int FLTAnalyser::queryConfig(int mode)
{
	switch (mode) {
		case Q_DO_ANALYSIS:
		case Q_ENGAGE_CODE_ANALYSER:
		case Q_ENGAGE_DATA_ANALYSER:
			return true;
		default:
			return 0;
	}
}

/*
 *
 */
Address *FLTAnalyser::fileofsToAddress(FileOfs fileofs)
{
	FLTAddress ea = (FLTAddress)fileofs;
	if (ea >= flt_shared->code_start && ea < flt_shared->data_end) {
		return createAddress32(ea);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool FLTAnalyser::validAddress(Address *Addr, tsectype action)
{
	FLTAddress ea;
	if (!convertAddressToFLTAddress(Addr, &ea)) return false;
	if (ea >= flt_shared->code_start) {
     	if (ea >= flt_shared->data_start) {
			if (ea >= flt_shared->bss_start) {
				if (ea >= flt_shared->bss_end) {
					return false;
				}
				switch (action) {
				case scvalid:
				case scread:
				case scwrite:
				case screadwrite:
					return true;
				case sccode:
				case scinitialized:
					return false;
				}
			}
			switch (action) {
			case scvalid:
			case scread:
			case scwrite:
			case screadwrite:
			case scinitialized:
				return true;
			case sccode:
				return false;
			}
		}
		switch (action) {
		case scvalid:
		case scread:
		case scinitialized:
			return true;
		case scwrite:
		case screadwrite:
		case sccode:
			return false;
          }
	}
	return false;
}


