/* 
 *	HT Editor
 *	macho_analy.cc
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
#include "analy_arm.h"
#include "analy_ppc.h"
#include "analy_register.h"
#include "analy_x86.h"
#include "macho_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htmacho.h"
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
 
void MachoAnalyser::init(ht_macho_shared_data *Macho_shared, File *File)
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

	/*
	 *	entrypoints
	 */
	uint entrypoint_count = 0;
	MACHO_COMMAND_U **pp = macho_shared->cmds.cmds;
	for (uint i=0; i < macho_shared->cmds.count; i++) {
		if ((*pp)->cmd.cmd == LC_UNIXTHREAD || (*pp)->cmd.cmd == LC_THREAD) {
			MACHO_THREAD_COMMAND *s = (MACHO_THREAD_COMMAND*)*pp;
			uint64 e = 0;
			switch (macho_shared->header.cputype) {
			case MACHO_CPU_TYPE_ARM:
				e = s->state.state_arm.pc;
				break;
			case MACHO_CPU_TYPE_POWERPC:
				e = s->state.state_ppc.srr[0];
				break;
			case MACHO_CPU_TYPE_I386:
				e = s->state.state_i386.eip;
				break;
			case MACHO_CPU_TYPE_X86_64:
				e = s->state.state_x86_64.rip;
			case MACHO_CPU_TYPE_POWERPC64:
				e = s->state.state_ppc64.srr[0];
				break;
			default: assert(0);
			}
			char desc[128];
			Address *entry;
			if (macho_shared->_64) {
				 entry = createAddress64(e);
			} else {
				 entry = createAddress32(e);
			}
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
	for (uint i=0; i < macho_shared->cmds.count; i++) {
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

				ht_snprintf(blub, sizeof blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
				Address *secend_addr = secaddr->clone();

				secend_addr->add(s->vmsize);
				newLocation(secend_addr)->flags |= AF_FUNCTION_END;
				addComment(secend_addr, 0, "");
				addComment(secend_addr, 0, ";******************************************************************");
				addComment(secend_addr, 0, blub);
				addComment(secend_addr, 0, ";******************************************************************");

//				validarea->add(secaddr, secend_addr);

				delete secend_addr;
			}
			delete secaddr;
		}
		pp++;
	}

	/* symbols */
	pp = macho_shared->cmds.cmds;
	for (uint i=0; i < macho_shared->cmds.count; i++) {
		if ((*pp)->cmd.cmd == LC_SYMTAB) {
			MACHO_SYMTAB_COMMAND *s = (MACHO_SYMTAB_COMMAND*)*pp;
			int *entropy = random_permutation(s->nsyms);
			for (uint j=0; j < s->nsyms; j++) {
				Address *address = NULL;
				char *label = NULL;

				if (macho_shared->_64) {
					file->seek(s->symoff + entropy[j]*sizeof (MACHO_SYMTAB_NLIST_64));
					MACHO_SYMTAB_NLIST_64 nlist;
					if (file->read(&nlist, sizeof nlist) != sizeof nlist) break;
					createHostStruct(&nlist, MACHO_SYMTAB_NLIST_64_struct, macho_shared->image_endianess);
					if (nlist.strx && (nlist.type & MACHO_SYMBOL_N_TYPE) == MACHO_SYMBOL_TYPE_N_SECT) {
						file->seek(s->stroff + nlist.strx);
						label = file->fgetstrz();
						address = createAddress64(nlist.value);
					}
				} else {
					file->seek(s->symoff + entropy[j]*sizeof (MACHO_SYMTAB_NLIST));
					MACHO_SYMTAB_NLIST nlist;
					if (file->read(&nlist, sizeof nlist) != sizeof nlist) break;
					createHostStruct(&nlist, MACHO_SYMTAB_NLIST_struct, macho_shared->image_endianess);
					if (nlist.strx && (nlist.type & MACHO_SYMBOL_N_TYPE) == MACHO_SYMBOL_TYPE_N_SECT) {
						file->seek(s->stroff + nlist.strx);
						label = file->fgetstrz();
						address = createAddress32(nlist.value);
					}
				}
				if (address && validAddress(address, scvalid)) {
					char macho_buffer[1024];
					char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					if (!demangled) demangled = cplus_demangle_v3(label, DMGL_PARAMS | DMGL_ANSI | DMGL_TYPES);
					if (!demangled && label[0]) demangled = cplus_demangle_v3(label+1, DMGL_PARAMS | DMGL_ANSI | DMGL_TYPES);
					make_valid_name(label, label);
					ht_snprintf(macho_buffer, sizeof macho_buffer, "; function %s", (demangled) ? demangled : label);
					free(demangled);
					addComment(address, 0, "");
					addComment(address, 0, ";********************************************************");
					addComment(address, 0, macho_buffer);
					addComment(address, 0, ";********************************************************");
					pushAddress(address, address);
					assignSymbol(address, label, label_func);
				}
				delete address;
				free(label);
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
void MachoAnalyser::load(ObjectStream &f)
{
	GET_OBJECT(f, validarea);
	Analyser::load(f);
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

ObjectID MachoAnalyser::getObjectID() const
{
	return ATOM_MACHO_ANALYSER;
}

/*
 *
 */
uint MachoAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool MachoAnalyser::convertAddressToMACHOAddress(Address *addr, MACHOAddress *r)
{
	if (addr->getObjectID() == ATOM_ADDRESS_FLAT_64) {
		*r = ((AddressFlat64*)addr)->addr;
		return true;
	} else if (addr->getObjectID() == ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr;
		return true;
	} else if (addr->getObjectID() == ATOM_ADDRESS_X86_FLAT_32) {
		*r = ((AddressX86Flat32*)addr)->addr;
		return true;
	}
	return false;
}

Address *MachoAnalyser::createAddress()
{
	if (macho_shared->header.cputype == MACHO_CPU_TYPE_I386) {
		return new AddressX86Flat32();
	} else {
		if (macho_shared->_64) {
			return new AddressFlat64();
		} else {
			return new AddressFlat32();
		}		
	}
}

Address *MachoAnalyser::createAddress32(uint32 addr)
{
	switch (macho_shared->header.cputype) {
	case MACHO_CPU_TYPE_I386:
		return new AddressX86Flat32(addr);
	case MACHO_CPU_TYPE_ARM:
	case MACHO_CPU_TYPE_POWERPC:
	default:
		return new AddressFlat32(addr);
	}
}

Address *MachoAnalyser::createAddress64(uint64 addr)
{
	return new AddressFlat64(addr);
}

/*
 *
 */
Assembler *MachoAnalyser::createAssembler()
{
	switch (macho_shared->header.cputype) {
	case MACHO_CPU_TYPE_I386: {
		Assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
		a->init();
		return a;
	}
	case MACHO_CPU_TYPE_X86_64: {
		Assembler *a = new x86_64asm();
		a->init();
		return a;
	}
	}
	return NULL;
}

/*
 *
 */
FileOfs MachoAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FileOfs ofs;
		MACHOAddress ea;
		if (!convertAddressToMACHOAddress(Addr, &ea)) return INVALID_FILE_OFS;
		if (!macho_addr_to_ofs(macho_shared->sections, macho_shared->section_count, ea, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
const char *MachoAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char macho_sectionname[33];
	int i;
	MACHOAddress ea;
	if (!convertAddressToMACHOAddress(Addr, &ea)) return NULL;
	if (!macho_addr_to_section(macho_shared->sections, macho_shared->section_count, ea, &i)) return NULL;
	if (macho_shared->sections[i]._64) {
		ht_strlcpy(macho_sectionname, (char*)macho_shared->sections[i].s.sectname, sizeof macho_sectionname);
	} else {
		ht_strlcpy(macho_sectionname, (char*)macho_shared->sections[i].s64.sectname, sizeof macho_sectionname);
	}
	return macho_sectionname;
}

/*
 *
 */
String &MachoAnalyser::getName(String &res)
{
	return file->getDesc(res);
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
	uint machine = macho_shared->header.cputype;
	bool macho64 = macho_shared->_64;
	switch (machine) {
	case MACHO_CPU_TYPE_I386:
		analy_disasm = new AnalyX86Disassembler();
		((AnalyX86Disassembler*)analy_disasm)->init(this, macho64 ? ANALYX86DISASSEMBLER_FLAGS_FLAT64 : 0);
		break;
	case MACHO_CPU_TYPE_X86_64:
		if (macho64) {
			analy_disasm = new AnalyX86Disassembler();
			((AnalyX86Disassembler*)analy_disasm)->init(this, ANALYX86DISASSEMBLER_FLAGS_AMD64 | ANALYX86DISASSEMBLER_FLAGS_FLAT64);
		} else {
			errorbox("x86_64 cant be used in a 32-Bit Mach-O.");
		}
		break;
	case MACHO_CPU_TYPE_POWERPC:
	case MACHO_CPU_TYPE_POWERPC64:
		analy_disasm = new AnalyPPCDisassembler();
		((AnalyPPCDisassembler*)analy_disasm)->init(this, macho64 ? ANALY_PPC_64 : ANALY_PPC_32);
		break;
	case MACHO_CPU_TYPE_ARM:
		analy_disasm = new AnalyArmDisassembler();
		((AnalyArmDisassembler*)analy_disasm)->init(this);
		break;
	default:
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
void MachoAnalyser::store(ObjectStream &f) const
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
Address *MachoAnalyser::fileofsToAddress(FileOfs fileofs)
{
	MACHOAddress ea;
	if (macho_ofs_to_addr(macho_shared->sections, macho_shared->section_count, fileofs, &ea)) {
		if (macho_shared->_64) {
			return createAddress64(ea);
		} else {
			return createAddress32(ea);
		}
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool MachoAnalyser::validAddress(Address *Addr, tsectype action)
{
	int sec;
	MACHOAddress ea;
	if (!convertAddressToMACHOAddress(Addr, &ea)) return false;
	if (!macho_addr_to_section(macho_shared->sections, macho_shared->section_count, ea, &sec)) return false;
	MACHO_SECTION_U &s = macho_shared->sections[sec];
	uint32 flags;
	if (s._64) {
		flags = s.s64.flags;
	} else {
		flags = s.s.flags;
	}
	switch (action) {
		case scvalid:
			return true;
		case scread:
			return true;
		case scwrite:
		case screadwrite: {
			bool writable =
				(flags & MACHO_SECTION_TYPE) != MACHO_S_CSTRING_LITERALS &&
				(flags & MACHO_SECTION_TYPE) != MACHO_S_4BYTE_LITERALS &&
				(flags & MACHO_SECTION_TYPE) != MACHO_S_8BYTE_LITERALS
			;
			return writable;
		}
		case sccode:
			return (flags & MACHO_SECTION_TYPE) == MACHO_S_REGULAR
				|| (flags & MACHO_S_ATTR_PURE_INSTRUCTIONS)
				|| (flags & MACHO_S_ATTR_SOME_INSTRUCTIONS);
		case scinitialized:
			return (flags & MACHO_SECTION_TYPE) != MACHO_S_ZEROFILL;
	}
	return false;
}
