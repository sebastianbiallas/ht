/* 
 *	HT Editor
 *	macho_analy.cc
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

#include "analy.h"
#include "analy_names.h"
#include "analy_ppc.h"
#include "analy_register.h"
#include "analy_x86.h"
#include "global.h"
#include "macho_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htmacho.h"
#include "htstring.h"
#include "pestruct.h"
#include "snprintf.h"
//#include "x86asm.h"

extern "C" {
#include "demangle.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
 
void MachoAnalyser::init(ht_macho_shared_data *Macho_shared, ht_streamfile *File)
{
	macho_shared = Macho_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();
}

void MachoAnalyser::beginAnalysis()
{
	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);

	MACHO_COMMAND_U **pp;
	/*
	 *	entrypoints
	 */
	UINT entrypoint_count = 0;
	pp = macho_shared->cmds.cmds;
	for (UINT i=0; i < macho_shared->cmds.count; i++) {
		if (((*pp)->cmd.cmd == LC_UNIXTHREAD) || ((*pp)->cmd.cmd == LC_THREAD)) {
			MACHO_THREAD_COMMAND *s = (MACHO_THREAD_COMMAND*)*pp;
			Address *entry;
			switch (macho_shared->header.cputype) {
			case MACHO_CPU_TYPE_POWERPC:
				entry = createAddress32(s->state.state_ppc.srr0);
				break;
			case MACHO_CPU_TYPE_I386:
				entry = createAddress32(s->state.state_i386.eip);
				break;
			default: assert(0);
			}
			char desc[128];
			sprintf(desc, "entrypoint%d", entrypoint_count++);
			pushAddress(entry, entry);
			assignSymbol(entry, desc, label_func);
			addComment(entry, 0, "");
			addComment(entry, 0, ";****************************");
			addComment(entry, 0, "; thread entrypoint");
			addComment(entry, 0, ";****************************");
			delete entry;
		}
		pp++;
	}
	
	/*
	 *	give all sections a descriptive comment:
	 */

	char blub[100];
	pp = macho_shared->cmds.cmds;
	for (UINT i=0; i < macho_shared->cmds.count; i++) {
		if ((*pp)->cmd.cmd == LC_SEGMENT) {
		MACHO_SEGMENT_COMMAND *s = (MACHO_SEGMENT_COMMAND*)*pp;

		Address *secaddr;

		secaddr = createAddress32(s->vmaddr);
		if (validAddress(secaddr, scvalid)) {
			ht_snprintf(blub, sizeof blub, ";  section %d <%s>", i+1, getSegmentNameByAddress(secaddr));
			addComment(secaddr, 0, "");
			addComment(secaddr, 0, ";******************************************************************");
			addComment(secaddr, 0, blub);
			ht_snprintf(blub, sizeof blub, ";  virtual address  %08x  virtual size   %08x", s->vmaddr, s->vmsize);
			addComment(secaddr, 0, blub);
			ht_snprintf(blub, sizeof blub, ";  file offset      %08x  file size      %08x", s->fileoff, s->filesize);
			addComment(secaddr, 0, blub);
			addComment(secaddr, 0, ";******************************************************************");

			// mark end of sections
			ht_snprintf(blub, sizeof blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
			Address *secend_addr = (Address *)secaddr->duplicate();

			secend_addr->add(s->vmsize);
			newLocation(secend_addr)->flags |= AF_FUNCTION_END;
			addComment(secend_addr, 0, "");
			addComment(secend_addr, 0, ";******************************************************************");
			addComment(secend_addr, 0, blub);
			addComment(secend_addr, 0, ";******************************************************************");

			validarea->add(secaddr, secend_addr);

			delete secend_addr;
		}
		delete secaddr;
		}
		pp++;
	}


	/* symbols */
	pp = macho_shared->cmds.cmds;
	for (UINT i=0; i < macho_shared->cmds.count; i++) {
		if ((*pp)->cmd.cmd == LC_SYMTAB) {
			MACHO_SYMTAB_COMMAND *s = (MACHO_SYMTAB_COMMAND*)*pp;
			int *entropy = random_permutation(s->nsyms);
			for (UINT j=0; j<s->nsyms; j++) {
				file->seek(s->symoff+entropy[j]*sizeof (MACHO_SYMTAB_NLIST));
				MACHO_SYMTAB_NLIST nlist;
				if (file->read(&nlist, sizeof nlist) != sizeof nlist) break;
				create_host_struct(&nlist, MACHO_SYMTAB_NLIST_struct, macho_shared->image_endianess);
				if (nlist.strx && (nlist.type & MACHO_SYMBOL_N_TYPE == MACHO_SYMBOL_TYPE_N_SECT)) {
					char macho_buffer[1024];
					file->seek(s->stroff+nlist.strx);
					char *label = fgetstrz(file);
//					fprintf(stderr, "symbol '%s' addr %08x\n", label, nlist.value);
					Address *address = createAddress32(nlist.value);
					if (validAddress(address, scvalid)) {
						char *demangled = NULL/*cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI)*/;
						make_valid_name(label, label);
						ht_snprintf(macho_buffer, sizeof macho_buffer, "; function %s", (demangled) ? demangled : label);
						if (demangled) free(demangled);
						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, macho_buffer);
						addComment(address, 0, ";********************************************************");
						pushAddress(address, address);
						assignSymbol(address, label, label_func);
					} else {
//						fprintf(stderr, "'%s' has invalid addr %08x\n", label, nlist.value);
					}
					delete address;
					free(label);
				}
			}
			free(entropy);
		}
		pp++;
	}
	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	
	Analyser::beginAnalysis();
}

/*
 *
 */
void MachoAnalyser::initInsertSymbols(int shidx)
{
}

/*
 *
 */
int MachoAnalyser::load(ht_object_stream *f)
{
	GET_OBJECT(f, validarea);
	return Analyser::load(f);
}

/*
 *
 */
void MachoAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

OBJECT_ID MachoAnalyser::object_id() const
{
	return ATOM_MACHO_ANALYSER;
}

/*
 *
 */
UINT MachoAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FILEOFS ofs = addressToFileofs(Addr);
/*     if (ofs == INVALID_FILE_OFS) {
		int as = 1;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);;
}

bool MachoAnalyser::convertAddressToMACHOAddress(Address *addr, MACHOAddress *r)
{
/*	if (addr->object_id()==ATOM_ADDRESS_FLAT_32) {
		r->a32 = ((AddressFlat32*)addr)->addr;
		return true;
	} else if (addr->object_id()==ATOM_ADDRESS_X86_FLAT_32) {
		r->a32 = ((AddressX86Flat32*)addr)->addr;
		return true;
	} else if (addr->object_id()==ATOM_ADDRESS_FLAT_64) {
		r->a64.lo = ((AddressFlat64*)addr)->addr.lo;
		r->a64.hi = ((AddressFlat64*)addr)->addr.hi;
		return true;*/
	if (addr->object_id() == ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr;
		return true;
	} else if (addr->object_id() == ATOM_ADDRESS_X86_FLAT_32) {
		*r = ((AddressX86Flat32*)addr)->addr;
		return true;
	}
	return false;
}

Address *MachoAnalyser::createAddress()
{
	switch (macho_shared->header.cputype) {
			case MACHO_CPU_TYPE_I386: {
				return new AddressX86Flat32();
			}
			case MACHO_CPU_TYPE_POWERPC: {
				return new AddressFlat32();
			}
	}
	assert(0);
	return NULL;
}

Address *MachoAnalyser::createAddress32(dword addr)
{
	switch (macho_shared->header.cputype) {
			case MACHO_CPU_TYPE_I386: {
				return new AddressX86Flat32(addr);
			}
			case MACHO_CPU_TYPE_POWERPC: {
				return new AddressFlat32(addr);
			}
	}
	assert(0);
	return NULL;
}

/*
 *
 */
Assembler *MachoAnalyser::createAssembler()
{
/*	switch (macho_shared->header32.e_machine) {
		case MACHO_EM_386:
			Assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
			a->init();
			return a;
	}*/
	return NULL;
}

/*
 *
 */
FILEOFS MachoAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		uint32 ofs;
		MACHOAddress ea;
		if (!convertAddressToMACHOAddress(Addr, &ea)) return INVALID_FILE_OFS;
		if (!macho_addr_to_ofs(&macho_shared->sections, 0, ea, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
static char macho_sectionname[33];

char *MachoAnalyser::getSegmentNameByAddress(Address *Addr)
{
	macho_sections *sections = &macho_shared->sections;
	int i;
	MACHOAddress ea;
	if (!convertAddressToMACHOAddress(Addr, &ea)) return NULL;
	if (!macho_addr_to_section(sections, 0, ea, &i)) return NULL;
	strncpy(macho_sectionname, (char*)sections->sections[i].sectname, 16);
	macho_sectionname[16] = 0;
	return macho_sectionname;
}

/*
 *
 */
const char *MachoAnalyser::getName()
{
	return file->get_desc();
}

/*
 *
 */
const char *MachoAnalyser::getType()
{
	return "Mach-O/Analyser";
}

/*
 *
 */
void MachoAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void MachoAnalyser::initUnasm()
{
	DPRINTF("macho_analy: ");
	uint machine = macho_shared->header.cputype;
	bool macho64 = false;
	switch (machine) {
		case MACHO_CPU_TYPE_I386: // Intel x86
			DPRINTF("initing analy_x86_disassembler\n");
			analy_disasm = new AnalyX86Disassembler();
			((AnalyX86Disassembler*)analy_disasm)->init(this, macho64 ? ANALYX86DISASSEMBLER_FLAGS_FLAT64 : 0);
			break;
		case MACHO_CPU_TYPE_POWERPC:	// PowerPC
			DPRINTF("initing analy_ppc_disassembler\n");
			analy_disasm = new AnalyPPCDisassembler();
			((AnalyPPCDisassembler*)analy_disasm)->init(this);
			break;
		default:
			DPRINTF("no apropriate disassembler for machine %04x\n", machine);
			warnbox("No disassembler for unknown machine type %04x!", machine);
	}
}

/*
 *
 */
void MachoAnalyser::log(const char *msg)
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
Address *MachoAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void MachoAnalyser::store(ht_object_stream *f)
{
	PUT_OBJECT(f, validarea);
	Analyser::store(f);
}

/*
 *
 */
int MachoAnalyser::queryConfig(int mode)
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
Address *MachoAnalyser::fileofsToAddress(FILEOFS fileofs)
{
	MACHOAddress ea;
	if (macho_ofs_to_addr(&macho_shared->sections, 0, fileofs, &ea)) {
/*		switch (macho_shared->ident.e_ident[MACHO_EI_CLASS]) {          
			case MACHOCLASS32: return createAddress32(ea.a32);
			case MACHOCLASS64: return createAddress64(ea.a64);
		}
		return new InvalidAddress();*/
		return createAddress32(ea);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool MachoAnalyser::validAddress(Address *Addr, tsectype action)
{
	macho_sections *sections = &macho_shared->sections;
	int sec;
	byte cls = 0/*macho_shared->ident.e_ident[MACHO_EI_CLASS]*/;
	MACHOAddress ea;
	if (!convertAddressToMACHOAddress(Addr, &ea)) return false;
	if (!macho_addr_to_section(sections, cls, ea, &sec)) return false;
/*	switch (cls) {
		case MACHOCLASS32: {*/
			MACHO_SECTION *s = &sections->sections[sec];
			switch (action) {
				case scvalid:
					return true;
				case scread:
					return true;
				case scwrite:
				case screadwrite: {
					bool writable =
						((s->flags & MACHO_SECTION_TYPE) != MACHO_S_CSTRING_LITERALS) &&
						((s->flags & MACHO_SECTION_TYPE) != MACHO_S_4BYTE_LITERALS) &&
						((s->flags & MACHO_SECTION_TYPE) != MACHO_S_8BYTE_LITERALS)
					;
					return writable;
				}
				case sccode:
					return ((s->flags & MACHO_SECTION_TYPE) == MACHO_S_REGULAR);
				case scinitialized:
					return ((s->flags & MACHO_SECTION_TYPE) != MACHO_S_ZEROFILL);
			}
			return false;
/*		}
		case MACHOCLASS64: {
			MACHO_SECTION_HEADER64 *s=sections->sheaders64+sec;
			switch (action) {
				case scvalid:
					return true;
				case scread:
					return true;
				case scwrite:
				case screadwrite:
					return s->sh_flags.lo & MACHO_SHF_WRITE;
				case sccode:
					return (s->sh_flags.lo & MACHO_SHF_EXECINSTR) && (s->sh_type==MACHO_SHT_PROGBITS);
				case scinitialized:
					return s->sh_type==MACHO_SHT_PROGBITS;
			}
			return false;
		}
	}*/
	return false;
}


