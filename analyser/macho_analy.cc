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
#include "analy_alpha.h"
#include "analy_names.h"
#include "analy_ia64.h"
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
//	Address *entry = NULL;
//	bool c32 = false;
/*	switch (macho_shared->ident.e_ident[MACHO_EI_CLASS]) {
		case MACHOCLASS32: {*/
/*			entry = createAddress32(macho_shared->header32.e_entry);
			c32 = true;
			break;*/
/*		}
		case MACHOCLASS64: {
			entry = createAddress64(macho_shared->header64.e_entry);
			c32 = false;
			break;
		}
	}*/

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);

	MACHO_COMMAND_U **pp;
	/*
	 *	entrypoints
	 */
	uint entrypoint_count = 0;
	pp = macho_shared->cmds.cmds;
	for (UINT i=0; i < macho_shared->cmds.count; i++) {
		if (((*pp)->cmd.cmd == LC_UNIXTHREAD) || ((*pp)->cmd.cmd == LC_THREAD)) {
			MACHO_THREAD_COMMAND *s = (MACHO_THREAD_COMMAND*)*pp;
			Address *entry = createAddress32(s->state.ppc.srr0);
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
/*		if (c32) {
			secaddr = createAddress32(s32->sh_addr);
		} else {
			secaddr = createAddress64(s64->sh_addr);
		}*/
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
/*			if (c32) {
				secend_addr->add(s32->sh_size);
			} else {
				secend_addr->add(s64->sh_size.lo);
			}*/
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
/*	if (c32) {
		for (UINT i=1; i<macho_shared->sheaders.count; i++) {
			if ((macho_shared->sheaders.sheaders32[i].sh_type==MACHO_SHT_SYMTAB) || (macho_shared->sheaders.sheaders32[i].sh_type==MACHO_SHT_DYNSYM)) {
				initInsertSymbols(i);
			}
		}
	} else {
		for (UINT i=1; i<macho_shared->sheaders.count; i++) {
			if ((macho_shared->sheaders.sheaders64[i].sh_type==MACHO_SHT_SYMTAB) || (macho_shared->sheaders.sheaders64[i].sh_type==MACHO_SHT_DYNSYM)) {
				initInsertSymbols(i);
			}
		}
	}*/
/*	assignSymbol(entry, "entrypoint", label_func);
	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	switch (c32 ? macho_shared->header32.e_type : macho_shared->header64.e_type) {
		case MACHO_ET_DYN:
			addComment(entry, 0, ";  dynamic executable entry point");
			break;
		case MACHO_ET_EXEC:
			addComment(entry, 0, ";  executable entry point");
			break;
		default:
			addComment(entry, 0, ";  entry point");               
	}
	addComment(entry, 0, ";****************************");*/

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
//	delete entry;
	
	Analyser::beginAnalysis();
}

/*
 *
 */
void MachoAnalyser::initInsertSymbols(int shidx)
{
#if 0
	char macho_buffer[1024];
//	if (macho_shared->ident.e_ident[MACHO_EI_CLASS] == MACHOCLASS32) {

		FILEOFS h=macho_shared->sheaders.sheaders32[shidx].sh_offset;
		FILEOFS sto=macho_shared->sheaders.sheaders32[macho_shared->sheaders.sheaders32[shidx].sh_link].sh_offset;
		UINT symnum=macho_shared->sheaders.sheaders32[shidx].sh_size / sizeof (MACHO_SYMBOL32);

		int *entropy = random_permutation(symnum);
		for (UINT i=0; i<symnum; i++) {
			MACHO_SYMBOL32 sym;
			if (entropy[i] == 0) continue;
			file->seek(h+entropy[i]*sizeof (MACHO_SYMBOL32));
			file->read(&sym, sizeof sym);
			create_host_struct(&sym, MACHO_SYMBOL32_struct, macho_shared->byte_order);

			file->seek(sto+sym.st_name);
			char *name = fgetstrz(file);
			if (!name) continue;

			switch (sym.st_shndx) {
				case MACHO_SHN_UNDEF:
					break;
				case MACHO_SHN_ABS:
					break;
				case MACHO_SHN_COMMON:
					break;
				default: {
					// sym.st_shndx
					break;
				}
			}

			char *bind;
			switch (MACHO32_ST_BIND(sym.st_info)) {
				case MACHO_STB_LOCAL:
					bind="local";
					break;
				case MACHO_STB_GLOBAL:
					bind="global";
					break;
				case MACHO_STB_WEAK:
					bind="weak";
					break;
				default:
					bind="?";
					break;
			}

			switch (MACHO32_ST_TYPE(sym.st_info)) {
				case MACHO_STT_NOTYPE:
				case MACHO_STT_FUNC: {
				char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress32(sym.st_value);
						if (validAddress(address, scvalid)) {
							char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);

							make_valid_name(label, label);

							ht_snprintf(macho_buffer, sizeof macho_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);

							if (demangled) free(demangled);

							addComment(address, 0, "");
							addComment(address, 0, ";********************************************************");
							addComment(address, 0, macho_buffer);
							addComment(address, 0, ";********************************************************");
							pushAddress(address, address);
							assignSymbol(address, label, label_func);
						}
						delete address;
					}
					break;
				}
				case MACHO_STT_OBJECT: {
					char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress32(sym.st_value);
						if (validAddress(address, scvalid)) {

							char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					
							make_valid_name(label, label);
					
							ht_snprintf(macho_buffer, sizeof macho_buffer, "; data object %s, size %d (%s)", (demangled) ? demangled : label, sym.st_size, bind);

							if (demangled) free(demangled);

							addComment(address, 0, "");
							addComment(address, 0, ";********************************************************");
							addComment(address, 0, macho_buffer);
							addComment(address, 0, ";********************************************************");
							assignSymbol(address, label, label_data);
						}
						delete address;
					}
					break;
				}
				case MACHO_STT_SECTION:
				case MACHO_STT_FILE:
					break;
			}
			free(name);
		}
		if (entropy) free(entropy);
/*	} else {
		// FIXME: 64 bit
		FILEOFS h=macho_shared->sheaders.sheaders64[shidx].sh_offset.lo;
		FILEOFS sto=macho_shared->sheaders.sheaders64[macho_shared->sheaders.sheaders64[shidx].sh_link].sh_offset.lo;
		UINT symnum=macho_shared->sheaders.sheaders64[shidx].sh_size.lo / sizeof (MACHO_SYMBOL64);

		int *entropy = random_permutation(symnum);
		for (UINT i=0; i<symnum; i++) {
			MACHO_SYMBOL64 sym;
			if (entropy[i] == 0) continue;
			file->seek(h+entropy[i]*sizeof (MACHO_SYMBOL64));
			file->read(&sym, sizeof sym);
			create_host_struct(&sym, MACHO_SYMBOL64_struct, macho_shared->byte_order);

			file->seek(sto+sym.st_name);
			char *name = fgetstrz(file);
			if (!name) continue;

			switch (sym.st_shndx) {
				case MACHO_SHN_UNDEF:
					break;
				case MACHO_SHN_ABS:
					break;
				case MACHO_SHN_COMMON:
					break;
				default: {
					// sym.st_shndx
					break;
				}
			}

			char *bind;
			switch (MACHO64_ST_BIND(sym.st_info)) {
				case MACHO_STB_LOCAL:
					bind="local";
					break;
				case MACHO_STB_GLOBAL:
					bind="global";
					break;
				case MACHO_STB_WEAK:
					bind="weak";
					break;
				default:
					bind="?";
					break;
			}

			switch (MACHO64_ST_TYPE(sym.st_info)) {
				case MACHO_STT_NOTYPE:
				case MACHO_STT_FUNC: {
					char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress64(sym.st_value);

						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);

						make_valid_name(label, label);

						ht_snprintf(macho_buffer, sizeof macho_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);

						if (demangled) free(demangled);

						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, macho_buffer);
						addComment(address, 0, ";********************************************************");
						pushAddress(address, address);
						assignSymbol(address, label, label_func);
						
						delete address;
					}
					break;
				}
				case MACHO_STT_OBJECT: {
					char *label = name;
					if (!getSymbolByName(label)) {
						Address *address = createAddress64(sym.st_value);

						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					
						make_valid_name(label, label);
					
						ht_snprintf(macho_buffer, sizeof macho_buffer, "; data object %s, size %d (%s)", (demangled) ? demangled : label, sym.st_size.lo, bind);

						if (demangled) free(demangled);

						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, macho_buffer);
						addComment(address, 0, ";********************************************************");
						assignSymbol(address, label, label_data);
						
						delete address;
					}
					break;
				}
				case MACHO_STT_SECTION:
				case MACHO_STT_FILE:
					break;
			}
			free(name);
		}
		if (entropy) free(entropy);
	}*/
#endif
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
	if (addr->object_id()==ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr;
		return true;
	} else {
		return false;
	}
}

Address *MachoAnalyser::createAddress()
{
#if 0
	switch (macho_shared->ident.e_ident[MACHO_EI_CLASS]) {
			case MACHOCLASS32: {
				switch (macho_shared->header32.e_machine) {
					case MACHO_EM_386:
						return new AddressX86Flat32();
				}
				return new AddressFlat32();
			}
			case MACHOCLASS64: {
/*				switch (macho_shared->header32.e_machine) {
					case MACHO_EM_386:
						return new AddressX86Flat32(0);
				}*/
				return new AddressFlat64();
			}
	}
#endif
	return new AddressFlat32();
}

Address *MachoAnalyser::createAddress32(dword addr)
{
/*	switch (macho_shared->header32.e_machine) {
		case MACHO_EM_386:
			return new AddressX86Flat32(addr);
	}*/
	return new AddressFlat32(addr);
}

Address *MachoAnalyser::createAddress64(qword addr)
{
	return new AddressFlat64(addr);
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
		dword ofs;
		MACHOAddress ea;
		if (!convertAddressToMACHOAddress(Addr, &ea)) return INVALID_FILE_OFS;
		if (!macho_addr_to_ofs(&macho_shared->cmds, 0, ea, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
char macho_sectionname[33];

char *MachoAnalyser::getSegmentNameByAddress(Address *Addr)
{
	macho_commands *cmds = &macho_shared->cmds;
	int i;
	MACHOAddress ea;
	if (!convertAddressToMACHOAddress(Addr, &ea)) return NULL;
	if (!macho_addr_to_section(cmds, 0, ea, &i)) return NULL;
	strncpy(macho_sectionname, (char*)cmds->cmds[i]->segment.segname, 16);
	macho_sectionname[32]=0;
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
	return "MACHO/Analyser";
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
	int machine = 0;
	bool macho64 = false;
/*	switch (macho_shared->ident.e_ident[MACHO_EI_CLASS]) {
		case MACHOCLASS32: machine = macho_shared->header32.e_machine; break;
		case MACHOCLASS64: machine = macho_shared->header64.e_machine; macho64 = true; break;
	}*/
/*	switch (machine) {
		case MACHO_EM_386: // Intel
			DPRINTF("initing analy_x86_disassembler\n");
			analy_disasm = new AnalyX86Disassembler();
			((AnalyX86Disassembler*)analy_disasm)->init(this, macho64 ? ANALYX86DISASSEMBLER_FLAGS_FLAT64 : 0);
			break;
		case MACHO_EM_IA_64: // Intel ia64
			if (macho_shared->ident.e_ident[MACHO_EI_CLASS] != MACHOCLASS64) {
				errorbox("Intel IA64 cant be used in a 32-Bit MACHO.");
			} else {
				analy_disasm = new AnalyIA64Disassembler();
				((AnalyIA64Disassembler*)analy_disasm)->init(this);
			}
			break;
		case MACHO_EM_PPC: // PowerPC
			if (macho_shared->ident.e_ident[MACHO_EI_CLASS] != MACHOCLASS32) {
				errorbox("Intel PowerPC cant be used in a 64-Bit MACHO.");
			} else {
				DPRINTF("initing analy_ppc_disassembler\n");
				analy_disasm = new AnalyPPCDisassembler();
				((AnalyPPCDisassembler*)analy_disasm)->init(this);
			}
			break;
		case MACHO_EM_PPC64: // PowerPC64
			if (macho_shared->ident.e_ident[MACHO_EI_CLASS] != MACHOCLASS64) {
				errorbox("Intel PowerPC64 cant be used in a 32-Bit MACHO.");
			} else {
				DPRINTF("initing analy_ppc_disassembler\n");
				analy_disasm = new AnalyPPCDisassembler();
				((AnalyPPCDisassembler*)analy_disasm)->init(this);
			}
			break;
		default:
			DPRINTF("no apropriate disassembler for machine %04x\n", machine);
			warnbox("No disassembler for unknown machine type %04x!", machine);
	}*/
	DPRINTF("initing analy_ppc_disassembler\n");
	analy_disasm = new AnalyPPCDisassembler();
	((AnalyPPCDisassembler*)analy_disasm)->init(this);
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
	if (macho_ofs_to_addr(&macho_shared->cmds, 0, fileofs, &ea)) {
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
	macho_commands *cmds=&macho_shared->cmds;
	int sec;
	byte cls = 0/*macho_shared->ident.e_ident[MACHO_EI_CLASS]*/;
	MACHOAddress ea;
	if (!convertAddressToMACHOAddress(Addr, &ea)) return false;
	if (!macho_addr_to_section(cmds, cls, ea, &sec)) return false;
/*	switch (cls) {
		case MACHOCLASS32: {*/
			MACHO_SEGMENT_COMMAND *s = &cmds->cmds[sec]->segment;
			switch (action) {
				case scvalid:
					return true;
				case scread:
					return true;
				case scwrite:
				case screadwrite:
					return true/*s->sh_flags & MACHO_SHF_WRITE*/;
				case sccode:
					return true/*(s->flags & MACHO_SHF_EXECINSTR) && (s->sh_type==MACHO_SHT_PROGBITS)*/;
				case scinitialized:
					return true/*s->sh_type==MACHO_SHT_PROGBITS*/;
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


