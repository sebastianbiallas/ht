/* 
 *	HT Editor
 *	pef_analy.cc
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

#include "analy.h"
#include "analy_names.h"
#include "analy_ppc.h"
#include "analy_register.h"
#include "pef_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htpef.h"
#include "strtools.h"
#include "pestruct.h"
#include "snprintf.h"
#include "x86asm.h"

extern "C" {
#include "demangle.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */

PEFAnalyser::PEFAnalyser()
{
}

void PEFAnalyser::init(ht_pef_shared_data *Pef_shared, File *File)
{
	pef_shared = Pef_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();
}

void PEFAnalyser::beginAnalysis()
{
//	Address *entry = NULL;

//	entry = createAddress32(pef_shared->header32.e_entry);

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);

	/*
	 *	entrypoint
	 */
//	pushAddress(entry, entry);
	
	/*
	 *	give all sections a descriptive comment:
	 */

	PEF_SECTION_HEADER *s32=pef_shared->sheaders.sheaders;
	char blub[100];
	for (uint i=0; i < pef_shared->sheaders.count; i++) {
		Address *secaddr;
		secaddr = createAddress32(s32->defaultAddress);
		if (validAddress(secaddr, scvalid)) {
			ht_snprintf(blub, sizeof blub, ";  section %d <%s>", i, getSegmentNameByAddress(secaddr));
			addComment(secaddr, 0, "");
			addComment(secaddr, 0, ";******************************************************************");
			addComment(secaddr, 0, blub);
			ht_snprintf(blub, sizeof blub, ";  virtual address  %08x  virtual size   %08x", s32->defaultAddress, s32->totalSize);
			addComment(secaddr, 0, blub);
			ht_snprintf(blub, sizeof blub, ";  file offset      %08x  file size      %08x", s32->containerOffset, s32->packedSize);
			addComment(secaddr, 0, blub);
			addComment(secaddr, 0, ";******************************************************************");

			// mark end of sections
			ht_snprintf(blub, sizeof blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
			Address *secend_addr = secaddr->clone();
			secend_addr->add(s32->totalSize);
			newLocation(secend_addr)->flags |= AF_FUNCTION_END;
			addComment(secend_addr, 0, "");
			addComment(secend_addr, 0, ";******************************************************************");
			addComment(secend_addr, 0, blub);
			addComment(secend_addr, 0, ";******************************************************************");

			validarea->add(secaddr, secend_addr);

			delete secend_addr;
		}
		delete secaddr;
		s32++;
	}

/* symbols */
/*	for (uint i=1; i<pef_shared->sheaders.count; i++) {
		if ((pef_shared->sheaders.sheaders32[i].sh_type==PEF_SHT_SYMTAB) || (pef_shared->sheaders.sheaders32[i].sh_type==PEF_SHT_DYNSYM)) {
			initInsertSymbols(i);
		}
	}*/
/*	assignSymbol(entry, "entrypoint", label_func);
	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	addComment(entry, 0, ";  executable entry point");
	addComment(entry, 0, ";****************************");*/

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
//	delete entry;
	
	Analyser::beginAnalysis();
}

/*
 *
 */
void PEFAnalyser::initInsertSymbols(int shidx)
{
/*	char pef_buffer[1024];
	if (pef_shared->ident.e_ident[PEF_EI_CLASS] == PEFCLASS32) {

		FileOfs h=pef_shared->sheaders.sheaders32[shidx].sh_offset;
		FileOfs sto=pef_shared->sheaders.sheaders32[pef_shared->sheaders.sheaders32[shidx].sh_link].sh_offset;
		uint symnum=pef_shared->sheaders.sheaders32[shidx].sh_size / sizeof (PEF_SYMBOL32);

		int *entropy = random_permutation(symnum);
		for (uint i=0; i<symnum; i++) {
			PEF_SYMBOL32 sym;
			if (entropy[i] == 0) continue;
			file->seek(h+entropy[i]*sizeof (PEF_SYMBOL32));
			file->read(&sym, sizeof sym);
			createHostStruct(&sym, PEF_SYMBOL32_struct, pef_shared->byte_order);

			file->seek(sto+sym.st_name);
			char *name = fgetstrz(*file);
			if (!name) continue;

			switch (sym.st_shndx) {
				case PEF_SHN_UNDEF:
					break;
				case PEF_SHN_ABS:
					break;
				case PEF_SHN_COMMON:
					break;
				default: {
					// sym.st_shndx
					break;
				}
			}

			char *bind;
			switch (PEF32_ST_BIND(sym.st_info)) {
				case PEF_STB_LOCAL:
					bind="local";
					break;
				case PEF_STB_GLOBAL:
					bind="global";
					break;
				case PEF_STB_WEAK:
					bind="weak";
					break;
				default:
					bind="?";
					break;
			}

			switch (PEF32_ST_TYPE(sym.st_info)) {
				case PEF_STT_NOTYPE:
				case PEF_STT_FUNC: {
				char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress32(sym.st_value);
						if (validAddress(address, scvalid)) {
							char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);

							make_valid_name(label, label);

							ht_snprintf(pef_buffer, sizeof pef_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);

							if (demangled) free(demangled);

							addComment(address, 0, "");
							addComment(address, 0, ";********************************************************");
							addComment(address, 0, pef_buffer);
							addComment(address, 0, ";********************************************************");
							pushAddress(address, address);
							assignSymbol(address, label, label_func);
						}
						delete address;
					}
					break;
				}
				case PEF_STT_OBJECT: {
					char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress32(sym.st_value);
						if (validAddress(address, scvalid)) {

							char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					
							make_valid_name(label, label);
					
							ht_snprintf(pef_buffer, sizeof pef_buffer, "; data object %s, size %d (%s)", (demangled) ? demangled : label, sym.st_size, bind);

							if (demangled) free(demangled);

							addComment(address, 0, "");
							addComment(address, 0, ";********************************************************");
							addComment(address, 0, pef_buffer);
							addComment(address, 0, ";********************************************************");
							assignSymbol(address, label, label_data);
						}
						delete address;
					}
					break;
				}
				case PEF_STT_SECTION:
				case PEF_STT_FILE:
					break;
			}
			free(name);
		}
		if (entropy) free(entropy);
	} else {
		// FIXME: 64 bit
		FileOfs h=pef_shared->sheaders.sheaders64[shidx].sh_offset.lo;
		FileOfs sto=pef_shared->sheaders.sheaders64[pef_shared->sheaders.sheaders64[shidx].sh_link].sh_offset.lo;
		uint symnum=pef_shared->sheaders.sheaders64[shidx].sh_size.lo / sizeof (PEF_SYMBOL64);

		int *entropy = random_permutation(symnum);
		for (uint i=0; i<symnum; i++) {
			PEF_SYMBOL64 sym;
			if (entropy[i] == 0) continue;
			file->seek(h+entropy[i]*sizeof (PEF_SYMBOL64));
			file->read(&sym, sizeof sym);
			createHostStruct(&sym, PEF_SYMBOL64_struct, pef_shared->byte_order);

			file->seek(sto+sym.st_name);
			char *name = fgetstrz(file);
			if (!name) continue;

			switch (sym.st_shndx) {
				case PEF_SHN_UNDEF:
					break;
				case PEF_SHN_ABS:
					break;
				case PEF_SHN_COMMON:
					break;
				default: {
					// sym.st_shndx
					break;
				}
			}

			char *bind;
			switch (PEF64_ST_BIND(sym.st_info)) {
				case PEF_STB_LOCAL:
					bind="local";
					break;
				case PEF_STB_GLOBAL:
					bind="global";
					break;
				case PEF_STB_WEAK:
					bind="weak";
					break;
				default:
					bind="?";
					break;
			}

			switch (PEF64_ST_TYPE(sym.st_info)) {
				case PEF_STT_NOTYPE:
				case PEF_STT_FUNC: {
					char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress64(sym.st_value);

						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);

						make_valid_name(label, label);

						ht_snprintf(pef_buffer, sizeof pef_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);

						if (demangled) free(demangled);

						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, pef_buffer);
						addComment(address, 0, ";********************************************************");
						pushAddress(address, address);
						assignSymbol(address, label, label_func);
						
						delete address;
					}
					break;
				}
				case PEF_STT_OBJECT: {
					char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress64(sym.st_value);

						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					
						make_valid_name(label, label);
					
						ht_snprintf(pef_buffer, sizeof pef_buffer, "; data object %s, size %d (%s)", (demangled) ? demangled : label, sym.st_size.lo, bind);

						if (demangled) free(demangled);

						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, pef_buffer);
						addComment(address, 0, ";********************************************************");
						assignSymbol(address, label, label_data);
						
						delete address;
					}
					break;
				}
				case PEF_STT_SECTION:
				case PEF_STT_FILE:
					break;
			}
			free(name);
		}
		if (entropy) free(entropy);
	}*/
}

/*
 *
 */
void PEFAnalyser::load(ObjectStream &f)
{
	GET_OBJECT(f, validarea);
	return Analyser::load(f);
}

/*
 *
 */
void PEFAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

ObjectID PEFAnalyser::getObjectID() const
{
	return ATOM_PEF_ANALYSER;
}

/*
 *
 */
uint PEFAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
/*     if (ofs == INVALID_FILE_OFS) {
		int as = 1;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);;
}

bool PEFAnalyser::convertAddressToPEFAddress(Address *addr, PEFAddress *r)
{
	if (addr->getObjectID()==ATOM_ADDRESS_FLAT_32) {
		r->a32 = ((AddressFlat32*)addr)->addr;
		return true;
	} else {
		return false;
	}
}

Address *PEFAnalyser::createAddress()
{
	return new AddressFlat32();
}

Address *PEFAnalyser::createAddress32(uint32 addr)
{
	return new AddressFlat32(addr);
}

Address *PEFAnalyser::createAddress64(uint64 addr)
{
	return new AddressFlat64(addr);
}

/*
 *
 */
Assembler *PEFAnalyser::createAssembler()
{
	return NULL;
}

/*
 *
 */
FileOfs PEFAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		uint32 ofs;
		PEFAddress ea;
		if (!convertAddressToPEFAddress(Addr, &ea)) return INVALID_FILE_OFS;
		if (!pef_addr_to_ofs(&pef_shared->sheaders, ea, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
const char *PEFAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char pef_sectionname[33];
	pef_section_headers *sections=&pef_shared->sheaders;
	int i;
	PEFAddress ea;
	if (!convertAddressToPEFAddress(Addr, &ea)) return NULL;
	if (!pef_addr_to_section(sections, ea, &i)) return NULL;
	if (pef_shared->sheaders.sheaders[i].nameOffset == 0xffffffff) {
		ht_snprintf(pef_sectionname, sizeof pef_sectionname, "unnamed%d", i);
	} else {
//		ht_strlcpy(pef_sectionname, pef_shared->shnames[i], 32);
		strcpy(pef_sectionname, "nyi");
	}
	return pef_sectionname;
}

/*
 *
 */
String &PEFAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *PEFAnalyser::getType()
{
	return "PEF/Analyser";
}

/*
 *
 */
void PEFAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void PEFAnalyser::initUnasm()
{
	DPRINTF("pef_analy: ");
	switch (pef_shared->arch) {
	case PEFARCH_PowerPC:
		DPRINTF("initing analy_ppc_disassembler\n");
		analy_disasm = new AnalyPPCDisassembler();
		((AnalyPPCDisassembler*)analy_disasm)->init(this, ANALY_PPC_32);
		break;
	case PEFARCH_M68K:
		DPRINTF("no disassembler for machine 'm68k'\n");
		warnbox("No disassembler for machine 'm68k'!");
		break;
	default:
		break;
	}
}

/*
 *
 */
void PEFAnalyser::log(const char *msg)
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
Address *PEFAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void PEFAnalyser::store(ObjectStream &f) const
{
	PUT_OBJECT(f, validarea);
	Analyser::store(f);
}

/*
 *
 */
int PEFAnalyser::queryConfig(int mode)
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
Address *PEFAnalyser::fileofsToAddress(FileOfs fileofs)
{
	PEFAddress ea;
	if (pef_ofs_to_addr(&pef_shared->sheaders, fileofs, &ea)) {
		return createAddress32(ea.a32);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool PEFAnalyser::validAddress(Address *Addr, tsectype action)
{
	pef_section_headers *sections=&pef_shared->sheaders;
	int sec;
	PEFAddress ea;
	if (!convertAddressToPEFAddress(Addr, &ea)) return false;
	if (!pef_addr_to_section(sections, ea, &sec)) return false;
	PEF_SECTION_HEADER *s = sections->sheaders + sec;
	switch (action) {
		case scvalid:
			return true;
		case scread:
			return true;
		case scwrite:
		case screadwrite:
			return (s->sectionKind == 1) ||
				(s->sectionKind == 2) ||
				(s->sectionKind == 6);
		case sccode:
			return (s->sectionKind == 0);
		case scinitialized:
			return (s->sectionKind == 0) ||
				(s->sectionKind == 1) ||
				(s->sectionKind == 3) ||
				(s->sectionKind == 6)	/* FIXME: is 6 really initialized ? */;
	}
	return false;
}
