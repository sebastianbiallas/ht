/*
 *	HT Editor
 *	elf_analy.cc
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
#include "analy_alpha.h"
#include "analy_names.h"
#include "analy_ia64.h"
#include "analy_ppc.h"
#include "analy_register.h"
#include "analy_x86.h"
#include "analy_arm.h"
#include "analy_avr.h"
#include "elf_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htelf.h"
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

void ElfAnalyser::init(ht_elf_shared_data *Elf_shared, File *File)
{
	elf_shared = Elf_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();
}

void ElfAnalyser::beginAnalysis()
{
	Address *entry = NULL;
	bool c32 = false;
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
		case ELFCLASS32: {
			entry = createAddress32(elf_shared->header32.e_entry);
			c32 = true;
			break;
		}
		case ELFCLASS64: {
			entry = createAddress64(elf_shared->header64.e_entry);
			c32 = false;
			break;
		}
	}

	if (!validAddress(entry, scvalid)) {
		delete entry;
		entry = NULL;
	}

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);

	/*
	 *	give all sections a descriptive comment:
	 */
	ELF_SECTION_HEADER32 *s32=elf_shared->sheaders.sheaders32;
	ELF_SECTION_HEADER64 *s64=elf_shared->sheaders.sheaders64;
	char blub[100];
	for (uint i=0; i < elf_shared->sheaders.count; i++) {
		Address *secaddr;
		if (c32) {
//			fprintf(stderr, "desc sec %d, %08x\n", i, s32->sh_addr);
			secaddr = createAddress32(s32->sh_addr);
		} else {
			secaddr = createAddress64(s64->sh_addr);
		}
		if (validAddress(secaddr, scvalid)) {
//			fprintf(stderr, "valid!\n");
			ht_snprintf(blub, sizeof blub, ";  section %d <%s>", i, getSegmentNameByAddress(secaddr));
			addComment(secaddr, 0, "");
			addComment(secaddr, 0, ";******************************************************************");
			addComment(secaddr, 0, blub);
			if (c32) {
				ht_snprintf(blub, sizeof blub, ";  virtual address  %08x  virtual size   %08x", s32->sh_addr, s32->sh_size);
			} else {
				ht_snprintf(blub, sizeof blub, ";  virtual address  %08qx  virtual size   %08qx", s64->sh_addr, s64->sh_size);
			}
			addComment(secaddr, 0, blub);
			if (validAddress(secaddr, scinitialized)) {
				if (c32) {
					ht_snprintf(blub, sizeof blub, ";  file offset      %08x  file size      %08x", s32->sh_offset, s32->sh_size);
				} else {
					ht_snprintf(blub, sizeof blub, ";  file offset      %08qx  file size      %08qx", s64->sh_offset, s64->sh_size);
				}
			} else {
				ht_snprintf(blub, sizeof blub, ";  section is not in file");
			}
			addComment(secaddr, 0, blub);
			addComment(secaddr, 0, ";******************************************************************");

			// mark end of sections
			ht_snprintf(blub, sizeof blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
			Address *secend_addr = secaddr->clone();
			if (c32) {
				secend_addr->add(s32->sh_size);
			} else {
				secend_addr->add(s64->sh_size);
			}
			newLocation(secend_addr)->flags |= AF_FUNCTION_END;
			addComment(secend_addr, 0, "");
			addComment(secend_addr, 0, ";******************************************************************");
			addComment(secend_addr, 0, blub);
			addComment(secend_addr, 0, ";******************************************************************");

			validarea->add(secaddr, secend_addr);
			
			Address *seciniaddr = secaddr->clone();
			seciniaddr->add(c32 ? s32->sh_size : s64->sh_size);
			if (validAddress(secaddr, scinitialized) && validAddress(seciniaddr, scinitialized)) {
				initialized->add(secaddr, seciniaddr);
			}
			delete seciniaddr;
			delete secend_addr;
		}
		delete secaddr;
		s32++;
		s64++;
	}

	/* symbols */
	if (c32) {
		for (uint i=1; i < elf_shared->sheaders.count; i++) {
			if ((elf_shared->sheaders.sheaders32[i].sh_type==ELF_SHT_SYMTAB) || (elf_shared->sheaders.sheaders32[i].sh_type==ELF_SHT_DYNSYM)) {
				initInsertSymbols(i);
			}
		}
		initInsertFakeSymbols();
	} else {
		for (uint i=1; i < elf_shared->sheaders.count; i++) {
			if ((elf_shared->sheaders.sheaders64[i].sh_type==ELF_SHT_SYMTAB) || (elf_shared->sheaders.sheaders64[i].sh_type==ELF_SHT_DYNSYM)) {
				initInsertSymbols(i);
			}
		}
	}

	/*
	 *	entrypoint
	 */
	if (entry) {
		pushAddress(entry, entry);
		assignSymbol(entry, "entrypoint", label_func);
		addComment(entry, 0, "");
		addComment(entry, 0, ";****************************");
		switch (c32 ? elf_shared->header32.e_type : elf_shared->header64.e_type) {
		case ELF_ET_DYN:
			addComment(entry, 0, ";  dynamic executable entry point");
			break;
		case ELF_ET_EXEC:
			addComment(entry, 0, ";  executable entry point");
			break;
		default:
			addComment(entry, 0, ";  entry point");
		}
		addComment(entry, 0, ";****************************");
		delete entry;
	}

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);

	Analyser::beginAnalysis();
}

/*
 *
 */
void ElfAnalyser::initInsertFakeSymbols()
{
	if (!elf_shared->undefined2fakeaddr) return;

	foreach(FakeAddr, fa, *elf_shared->undefined2fakeaddr, {
		Address *address = createAddress32(fa->addr);
		FileOfs h = elf_shared->sheaders.sheaders32[fa->secidx].sh_offset;
		ELF_SYMBOL32 sym;
		file->seek(h + fa->symidx * sizeof (ELF_SYMBOL32));
		file->readx(&sym, sizeof sym);
		createHostStruct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);

		FileOfs sto = elf_shared->sheaders.sheaders32[
			elf_shared->sheaders.sheaders32[fa->secidx].sh_link].sh_offset;
		file->seek(sto + sym.st_name);
		char *name = file->fgetstrz();
		char buf[1024];
		ht_snprintf(buf, sizeof buf, "undef_%s", name);
		free(name);
		make_valid_name(buf, buf);
		assignSymbol(address, buf, label_func);
	})
}

void ElfAnalyser::initInsertSymbols(int shidx)
{
	char elf_buffer[1024];
	if (elf_shared->ident.e_ident[ELF_EI_CLASS] == ELFCLASS32) {
		FileOfs h = elf_shared->sheaders.sheaders32[shidx].sh_offset;
		FileOfs sto = elf_shared->sheaders.sheaders32[elf_shared->sheaders.sheaders32[shidx].sh_link].sh_offset;
		uint symnum = elf_shared->sheaders.sheaders32[shidx].sh_size / sizeof (ELF_SYMBOL32);

		int *entropy = random_permutation(symnum);
		for (uint i = 0; i < symnum; i++) {
			ELF_SYMBOL32 sym;
			if (entropy[i] == 0) continue;
			file->seek(h+entropy[i]*sizeof (ELF_SYMBOL32));
			file->read(&sym, sizeof sym);
			createHostStruct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);

			file->seek(sto+sym.st_name);
			char *name = file->fgetstrz();
			if (!name) continue;

			switch (sym.st_shndx) {
			case ELF_SHN_UNDEF:
				break;
			case ELF_SHN_ABS:
				break;
			case ELF_SHN_COMMON:
				break;
			default:
				// sym.st_shndx
				break;
			}

			const char *bind;
			switch (ELF32_ST_BIND(sym.st_info)) {
			case ELF_STB_LOCAL:
				bind="local";
				break;
			case ELF_STB_GLOBAL:
				bind="global";
				break;
			case ELF_STB_WEAK:
				bind="weak";
				break;
			default:
				bind="?";
				break;
			}

			switch (ELF32_ST_TYPE(sym.st_info)) {
			case ELF_STT_NOTYPE:
			case ELF_STT_FUNC: {
				char *label = name;
				if (!getSymbolByName(label)) {
					elf32_addr sym_addr = sym.st_value;
					if (elf_shared->shrelocs && sym.st_shndx > 0 && sym.st_shndx < elf_shared->sheaders.count
					&& elf_shared->shrelocs[sym.st_shndx].relocAddr) {
						sym_addr += elf_shared->shrelocs[sym.st_shndx].relocAddr;
					}
					Address *address = createAddress32(sym_addr);
					if (validAddress(address, scvalid)) {
						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
						if (!demangled) demangled = cplus_demangle_v3(label, DMGL_PARAMS | DMGL_ANSI | DMGL_TYPES);
						make_valid_name(label, label);
						ht_snprintf(elf_buffer, sizeof elf_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);
						free(demangled);
						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, elf_buffer);
						addComment(address, 0, ";********************************************************");
						pushAddress(address, address);
						assignSymbol(address, label, label_func);
					}
					delete address;
				}
				break;
			}
			case ELF_STT_OBJECT: {
				char *label = name;
				if (!getSymbolByName(label)) {
					Address *address = createAddress32(sym.st_value);
					if (validAddress(address, scvalid)) {
						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);

						make_valid_name(label, label);

						ht_snprintf(elf_buffer, sizeof elf_buffer, "; data object %s, size %d (%s)", (demangled) ? demangled : label, sym.st_size, bind);

						if (demangled) free(demangled);

						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, elf_buffer);
						addComment(address, 0, ";********************************************************");
						assignSymbol(address, label, label_data);
					}
					delete address;
				}
				break;
			}
			case ELF_STT_SECTION:
			case ELF_STT_FILE:
				break;
			}
			free(name);
		}
		free(entropy);
	} else {
		// FIXME: 64 bit
		FileOfs h = elf_shared->sheaders.sheaders64[shidx].sh_offset;
		FileOfs sto = elf_shared->sheaders.sheaders64[elf_shared->sheaders.sheaders64[shidx].sh_link].sh_offset;
		uint symnum = elf_shared->sheaders.sheaders64[shidx].sh_size / sizeof (ELF_SYMBOL64);

		int *entropy = random_permutation(symnum);
		for (uint i=0; i<symnum; i++) {
			ELF_SYMBOL64 sym;
			if (entropy[i] == 0) continue;
			file->seek(h+entropy[i]*sizeof (ELF_SYMBOL64));
			file->read(&sym, sizeof sym);
			createHostStruct(&sym, ELF_SYMBOL64_struct, elf_shared->byte_order);

			file->seek(sto+sym.st_name);
			char *name = file->fgetstrz();
			if (!name) continue;

			switch (sym.st_shndx) {
			case ELF_SHN_UNDEF:
				break;
			case ELF_SHN_ABS:
				break;
			case ELF_SHN_COMMON:
				break;
			default: {
				// sym.st_shndx
				break;
			}
			}

			const char *bind;
			switch (ELF64_ST_BIND(sym.st_info)) {
			case ELF_STB_LOCAL:
				bind="local";
				break;
			case ELF_STB_GLOBAL:
				bind="global";
				break;
			case ELF_STB_WEAK:
				bind="weak";
				break;
			default:
				bind="?";
				break;
			}

			switch (ELF64_ST_TYPE(sym.st_info)) {
			case ELF_STT_NOTYPE:
			case ELF_STT_FUNC: {
				char *label = name;
				if (!getSymbolByName(label)) {
					Address *address = createAddress64(sym.st_value);
					if (validAddress(address, scvalid)) {
						char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
						if (!demangled) demangled = cplus_demangle_v3(label, DMGL_PARAMS | DMGL_ANSI | DMGL_TYPES);
						make_valid_name(label, label);
						ht_snprintf(elf_buffer, sizeof elf_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);
						free(demangled);
						addComment(address, 0, "");
						addComment(address, 0, ";********************************************************");
						addComment(address, 0, elf_buffer);
						addComment(address, 0, ";********************************************************");
						pushAddress(address, address);
						assignSymbol(address, label, label_func);
					}
					delete address;
				}
				break;
			}
			case ELF_STT_OBJECT: {
				char *label = name;
				if (!getSymbolByName(label)) {
					Address *address = createAddress64(sym.st_value);
					char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					if (!demangled) demangled = cplus_demangle_v3(label, DMGL_PARAMS | DMGL_ANSI | DMGL_TYPES);

					make_valid_name(label, label);

					ht_snprintf(elf_buffer, sizeof elf_buffer, "; data object %s, size %qd (%s)", (demangled) ? demangled : label, sym.st_size, bind);

					free(demangled);

					addComment(address, 0, "");
					addComment(address, 0, ";********************************************************");
					addComment(address, 0, elf_buffer);
					addComment(address, 0, ";********************************************************");
					assignSymbol(address, label, label_data);

					delete address;
				}
				break;
			}
			case ELF_STT_SECTION:
			case ELF_STT_FILE:
				break;
			}
			free(name);
		}
		free(entropy);
	}
}

/*
 *
 */
void ElfAnalyser::load(ObjectStream &f)
{
	GET_OBJECT(f, validarea);
	Analyser::load(f);
}

/*
 *
 */
void ElfAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

ObjectID ElfAnalyser::getObjectID() const
{
	return ATOM_ELF_ANALYSER;
}

/*
 *
 */
uint ElfAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
/*     if (ofs == INVALID_FILE_OFS) {
		int as = 1;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool ElfAnalyser::convertAddressToELFAddress(Address *addr, ELFAddress *r)
{
	if (addr->getObjectID()==ATOM_ADDRESS_FLAT_32) {
		r->a32 = ((AddressFlat32*)addr)->addr;
		return true;
	} else if (addr->getObjectID()==ATOM_ADDRESS_X86_FLAT_32) {
		r->a32 = ((AddressX86Flat32*)addr)->addr;
		return true;
	} else if (addr->getObjectID()==ATOM_ADDRESS_FLAT_64) {
		r->a64 = ((AddressFlat64*)addr)->addr;
		return true;
	} else {
		return false;
	}
}

Address *ElfAnalyser::createAddress()
{
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
			case ELFCLASS32: {
				switch (elf_shared->header32.e_machine) {
					case ELF_EM_386:
						return new AddressX86Flat32();
				}
				return new AddressFlat32();
			}
			case ELFCLASS64: {
/*				switch (elf_shared->header32.e_machine) {
					case ELF_EM_386:
						return new AddressX86Flat32(0);
				}*/
				return new AddressFlat64();
			}
	}
	return new AddressFlat32();
}

Address *ElfAnalyser::createAddress32(uint32 addr)
{
	switch (elf_shared->header32.e_machine) {
		case ELF_EM_386:
			return new AddressX86Flat32(addr);
	}
	return new AddressFlat32(addr);
}

Address *ElfAnalyser::createAddress64(uint64 addr)
{
	return new AddressFlat64(addr);
}

/*
 *
 */
Assembler *ElfAnalyser::createAssembler()
{
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
	case ELFCLASS32:
		switch (elf_shared->header32.e_machine) {
		case ELF_EM_386:
			Assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
			a->init();
			return a;
		}
	case ELFCLASS64:
		switch (elf_shared->header64.e_machine) {
		case ELF_EM_X86_64:
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
FileOfs ElfAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FileOfs ofs;
		ELFAddress ea;
		if (!convertAddressToELFAddress(Addr, &ea)) return INVALID_FILE_OFS;
		if (!elf_addr_to_ofs(&elf_shared->sheaders, elf_shared->ident.e_ident[ELF_EI_CLASS], ea, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */

const char *ElfAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char elf_sectionname[33];
	elf_section_headers *sections=&elf_shared->sheaders;
	int i;
	ELFAddress ea;
	if (!convertAddressToELFAddress(Addr, &ea)) return NULL;
	if (!elf_addr_to_section(sections, elf_shared->ident.e_ident[ELF_EI_CLASS], ea, &i)) return NULL;
	if (i == elf_shared->fake_undefined_shidx) {
		strcpy(elf_sectionname, "$$HT_FAKE$$");
	} else {
		ht_strlcpy(elf_sectionname, elf_shared->shnames[i], sizeof elf_sectionname);
	}
	return elf_sectionname;
}

/*
 *
 */
String &ElfAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *ElfAnalyser::getType()
{
	return "ELF/Analyser";
}

/*
 *
 */
void ElfAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void ElfAnalyser::initUnasm()
{
	DPRINTF("elf_analy: ");
	int machine = 0;
	bool elf64 = false;
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
		case ELFCLASS32: machine = elf_shared->header32.e_machine; break;
		case ELFCLASS64: machine = elf_shared->header64.e_machine; elf64 = true; break;
	}
	switch (machine) {
	case ELF_EM_386:
		DPRINTF("initing analy_x86_disassembler\n");
		analy_disasm = new AnalyX86Disassembler();
		((AnalyX86Disassembler*)analy_disasm)->init(this, elf64 ? ANALYX86DISASSEMBLER_FLAGS_FLAT64 : 0);
		break;
	case ELF_EM_X86_64:
		if (elf_shared->ident.e_ident[ELF_EI_CLASS] != ELFCLASS64) {
			errorbox("x86_64 cant be used in a 32-Bit ELF.");
		} else {
			analy_disasm = new AnalyX86Disassembler();
			((AnalyX86Disassembler*)analy_disasm)->init(this, ANALYX86DISASSEMBLER_FLAGS_AMD64 | ANALYX86DISASSEMBLER_FLAGS_FLAT64);
		}
		break;
	case ELF_EM_IA_64: // Intel ia64
		if (elf_shared->ident.e_ident[ELF_EI_CLASS] != ELFCLASS64) {
			errorbox("Intel IA64 cant be used in a 32-Bit ELF.");
		} else {
			analy_disasm = new AnalyIA64Disassembler();
			((AnalyIA64Disassembler*)analy_disasm)->init(this);
		}
		break;
	case ELF_EM_PPC: // PowerPC
		if (elf_shared->ident.e_ident[ELF_EI_CLASS] != ELFCLASS32) {
			errorbox("PowerPC32 cant be used in a 64-Bit ELF.");
		} else {
			DPRINTF("initing analy_ppc_disassembler\n");
			analy_disasm = new AnalyPPCDisassembler();
			((AnalyPPCDisassembler*)analy_disasm)->init(this, ANALY_PPC_32);
		}
		break;
	case ELF_EM_PPC64: // PowerPC64
		if (elf_shared->ident.e_ident[ELF_EI_CLASS] != ELFCLASS64) {
			errorbox("PowerPC64 cant be used in a 32-Bit ELF.");
		} else {
			DPRINTF("initing analy_ppc_disassembler\n");
			analy_disasm = new AnalyPPCDisassembler();
			((AnalyPPCDisassembler*)analy_disasm)->init(this, ANALY_PPC_64);
		}
		break;
	case ELF_EM_ARM:
		if (elf_shared->ident.e_ident[ELF_EI_CLASS] != ELFCLASS32) {
			errorbox("ARM cant be used in a 64-Bit ELF.");
		} else {
			DPRINTF("initing analy_arm_disassembler\n");
			analy_disasm = new AnalyArmDisassembler();
			((AnalyArmDisassembler*)analy_disasm)->init(this);
		}
		break;
	case ELF_EM_AVR:
		if (elf_shared->ident.e_ident[ELF_EI_CLASS] != ELFCLASS32) {
			errorbox("AVR cant be used in a 64-Bit ELF.");
		} else {
			DPRINTF("initing analy_avr_disassembler\n");
			analy_disasm = new AnalyAVRDisassembler();
			((AnalyAVRDisassembler*)analy_disasm)->init(this);
		}
		break;
	default:
		DPRINTF("no apropriate disassembler for machine 0x%04x\n", machine);
		warnbox("No disassembler for unknown machine type 0x%04x!", machine);
	}
}

/*
 *
 */
Address *ElfAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void ElfAnalyser::store(ObjectStream &f) const
{
	PUT_OBJECT(f, validarea);
	Analyser::store(f);
}

/*
 *
 */
int ElfAnalyser::queryConfig(int mode)
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
Address *ElfAnalyser::fileofsToAddress(FileOfs fileofs)
{
	ELFAddress ea;
	if (elf_ofs_to_addr(&elf_shared->sheaders, elf_shared->ident.e_ident[ELF_EI_CLASS], fileofs, &ea)) {
		switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
			case ELFCLASS32: return createAddress32(ea.a32);
			case ELFCLASS64: return createAddress64(ea.a64);
		}
		return new InvalidAddress();
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool ElfAnalyser::validAddress(Address *Addr, tsectype action)
{
	elf_section_headers *sections=&elf_shared->sheaders;
	int sec;
	byte cls = elf_shared->ident.e_ident[ELF_EI_CLASS];
	ELFAddress ea;
	if (!convertAddressToELFAddress(Addr, &ea)) return false;
	if (!elf_addr_to_section(sections, cls, ea, &sec)) return false;
	switch (cls) {
	case ELFCLASS32: {
		ELF_SECTION_HEADER32 *s = sections->sheaders32 + sec;
		switch (action) {
		case scvalid:
			return true;
		case scread:
			return true;
		case scwrite:
		case screadwrite:
			return s->sh_flags & ELF_SHF_WRITE;
		case sccode:
			return (s->sh_flags & ELF_SHF_EXECINSTR) && (s->sh_type == ELF_SHT_PROGBITS);
		case scinitialized:
			return s->sh_type==ELF_SHT_PROGBITS;
		}
		return false;
	}
	case ELFCLASS64: {
		ELF_SECTION_HEADER64 *s = sections->sheaders64 + sec;
		switch (action) {
		case scvalid:
			return true;
		case scread:
			return true;
		case scwrite:
		case screadwrite:
			return s->sh_flags & ELF_SHF_WRITE;
		case sccode:
			return (s->sh_flags & ELF_SHF_EXECINSTR) && (s->sh_type == ELF_SHT_PROGBITS);
		case scinitialized:
			return s->sh_type==ELF_SHT_PROGBITS;
		}
		return false;
	}
	}
	return false;
}
