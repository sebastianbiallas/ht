/*
 *	HT Editor
 *	pe_analy.cc
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analy.h"
#include "analy_alpha.h"
#include "analy_ia64.h"
#include "analy_il.h"
#include "analy_names.h"
#include "analy_register.h"
#include "analy_ppc.h"
#include "analy_x86.h"
#include "analy_arm.h"
#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htpe.h"
#include "strtools.h"
#include "ilopc.h"
#include "pe_analy.h"
#include "pestruct.h"
#include "snprintf.h"
#include "x86asm.h"

void	PEAnalyser::init(ht_pe_shared_data *Pe_shared, File *File)
{
	pe_shared = Pe_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();
}


static char *string_func(uint32 ofs, void *context);
static char *token_func(uint32 token, void *context);

/*
 *
 */
void	PEAnalyser::load(ObjectStream &f)
{
	GET_OBJECT(f, validarea);
	Analyser::load(f);
}

/*
 *
 */
void	PEAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

/*
 *
 */
void	PEAnalyser::reinit(ht_pe_shared_data *Pe_shared, File *f)
{
	pe_shared = Pe_shared;
	file = f;
	if (disasm->getObjectID() == ATOM_DISASM_IL) {
		((ILDisassembler *)disasm)->initialize(string_func, token_func, pe_shared);
	}
}

/*
 *
 */
void PEAnalyser::beginAnalysis()
{
	char	buffer[1024];

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);

	bool pe32 = (pe_shared->opt_magic == COFF_OPTMAGIC_PE32);

	/*
	 *	entrypoint
	 */
	Address *entry;
	if (pe32) {
		entry = createAddress32(pe_shared->pe32.header.entrypoint_address+pe_shared->pe32.header_nt.image_base);
	} else {
		entry = createAddress64(pe_shared->pe64.header.entrypoint_address+pe_shared->pe64.header_nt.image_base);
	}
	pushAddress(entry, entry);

	/*
	 * give all sections a descriptive comment:
	 */

	/*struct PE_SECTION_HEADER {
		byte name[PE_SIZEOF_SHORT_NAME] __attribute__ ((packed));
		uint32 data_vsize __attribute__ ((packed));
		uint32 data_address __attribute__ ((packed));
		uint32 data_size __attribute__	((packed));
		uint32 data_offset __attribute__ ((packed));
		uint32 relocation_offset __attribute__ ((packed));
		uint32 linenumber_offset __attribute__ ((packed));
		uint16 relocation_count __attribute__ ((packed));
		uint16 linenumber_count __attribute__ ((packed));
		uint32 characteristics __attribute__ ((packed));
	};*/
	COFF_SECTION_HEADER *s=pe_shared->sections.sections;
	char blub[100];
	for (uint i=0; i<pe_shared->sections.section_count; i++) {
		Address *secaddr;
		if (pe32) {
			secaddr = createAddress32(s->data_address + pe_shared->pe32.header_nt.image_base);
		} else {
			secaddr = createAddress64(s->data_address + pe_shared->pe64.header_nt.image_base);
		}
		ht_snprintf(blub, sizeof blub, ";  section %d <%s>", i+1, getSegmentNameByAddress(secaddr));
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, blub);
		ht_snprintf(blub, sizeof blub, ";  virtual address  %08x  virtual size   %08x", s->data_address, s->data_vsize);
		addComment(secaddr, 0, blub);
		ht_snprintf(blub, sizeof blub, ";  file offset      %08x  file size      %08x", s->data_offset, s->data_size);
		addComment(secaddr, 0, blub);
		addComment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		ht_snprintf(blub, sizeof blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
		Address *secend_addr = secaddr->clone();
		secend_addr->add(MAX(s->data_size, s->data_vsize));
		newLocation(secend_addr)->flags |= AF_FUNCTION_END;
		addComment(secend_addr, 0, "");
		addComment(secend_addr, 0, ";******************************************************************");
		addComment(secend_addr, 0, blub);
		addComment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		Address *seciniaddr = secaddr->clone();
		seciniaddr->add(MIN(s->data_size, s->data_vsize));
		if (validAddress(secaddr, scinitialized) && validAddress(seciniaddr, scinitialized)) {
			initialized->add(secaddr, seciniaddr);
		}
		s++;
		delete secaddr;
		delete secend_addr;
		delete seciniaddr;
	}

	// exports

	int export_count=pe_shared->exports.funcs->count();
	int *entropy = random_permutation(export_count);
	for (int i=0; i<export_count; i++) {
		ht_pe_export_function *f=(ht_pe_export_function *)(*pe_shared->exports.funcs)[entropy[i]];
		Address *faddr;
		if (pe32) {
			faddr = createAddress32(f->address + pe_shared->pe32.header_nt.image_base);
		} else {
			faddr = createAddress64(f->address + pe_shared->pe64.header_nt.image_base);
		}
		if (validAddress(faddr, scvalid)) {
			char *label;
			if (f->byname) {
				ht_snprintf(buffer, sizeof buffer, "; exported function %s, ordinal %04x", f->name, f->ordinal);
			} else {
				ht_snprintf(buffer, sizeof buffer, "; unnamed exported function, ordinal %04x", f->ordinal);
			}
			label = export_func_name((f->byname) ? f->name : NULL, f->ordinal);
			addComment(faddr, 0, "");
			addComment(faddr, 0, ";********************************************************");
			addComment(faddr, 0, buffer);
			addComment(faddr, 0, ";********************************************************");
			pushAddress(faddr, faddr);
			assignSymbol(faddr, label, label_func);
			free(label);
		}
		delete faddr;
	}
	if (entropy) free(entropy);

	int import_count=pe_shared->imports.funcs->count();
	entropy = random_permutation(import_count);
	for (int i=0; i<import_count; i++) {
		ht_pe_import_function *f = (ht_pe_import_function *)(*pe_shared->imports.funcs)[entropy[i]];
		ht_pe_import_library *d = (ht_pe_import_library *)(*pe_shared->imports.libs)[f->libidx];
		char *label;
		label = import_func_name(d->name, (f->byname) ? f->name.name : NULL, f->ordinal);
		Address *faddr;
		if (pe32) {
			faddr = createAddress32(f->address + pe_shared->pe32.header_nt.image_base);
		} else {
			faddr = createAddress64(f->address + pe_shared->pe64.header_nt.image_base);
		}
		addComment(faddr, 0, "");
		if (!assignSymbol(faddr, label, label_func)) {
			// multiple import of a function (duplicate labelname)
			// -> mangle name a bit more
			addComment(faddr, 0, "; duplicate import");
			ht_snprintf(buffer, sizeof buffer, "%s_%x", label, f->address);
			assignSymbol(faddr, buffer, label_func);
		}
		if (pe32) {
			data->setIntAddressType(faddr, dst_idword, 4);
		} else {
			data->setIntAddressType(faddr, dst_iqword, 8);
		}
		free(label);
		delete faddr;
	}
	if (entropy) free(entropy);

	int dimport_count=pe_shared->dimports.funcs->count();
	entropy = random_permutation(dimport_count);
	for (int i=0; i<dimport_count; i++) {
		// FIXME: delay imports need work (push addr)
		ht_pe_import_function *f=(ht_pe_import_function *)(*pe_shared->dimports.funcs)[entropy[i]];
		ht_pe_import_library *d=(ht_pe_import_library *)(*pe_shared->dimports.libs)[f->libidx];
		if (f->byname) {
			ht_snprintf(buffer, sizeof buffer, "; delay import function loader for %s, ordinal %04x", f->name.name, f->ordinal);
		} else {
			ht_snprintf(buffer, sizeof buffer, "; delay import function loader for ordinal %04x", f->ordinal);
		}
		char *label;
		label = import_func_name(d->name, f->byname ? f->name.name : NULL, f->ordinal);
		Address *faddr;
		if (pe32) {
			faddr = createAddress32(f->address);
		} else {
			faddr = createAddress64(f->address);
		}
		addComment(faddr, 0, "");
		addComment(faddr, 0, ";********************************************************");
		addComment(faddr, 0, buffer);
		addComment(faddr, 0, ";********************************************************");
		assignSymbol(faddr, label, label_func);
		free(label);
		delete faddr;
	}
	if (entropy) free(entropy);

	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	if (pe_shared->coffheader.characteristics & COFF_DLL) {
		addComment(entry, 0, ";  dll entry point");
	} else {
		addComment(entry, 0, ";  program entry point");
	}
	addComment(entry, 0, ";****************************");
	assignSymbol(entry, "entrypoint", label_func);

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	delete entry;

	Analyser::beginAnalysis();
}

/*
 *
 */
ObjectID	PEAnalyser::getObjectID() const
{
	return ATOM_PE_ANALYSER;
}

/*
 *
 */
uint PEAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
/*	if (ofs == INVALID_FILE_OFS) {
		int as=0;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool PEAnalyser::convertAddressToRVA(Address *addr, RVA *r)
{
	ObjectID oid = addr->getObjectID();
	if (oid == ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr - pe_shared->pe32.header_nt.image_base;
		return true;
	} else if (oid == ATOM_ADDRESS_X86_FLAT_32) {
		*r = ((AddressX86Flat32*)addr)->addr - pe_shared->pe32.header_nt.image_base;
		return true;
	} else if (oid == ATOM_ADDRESS_FLAT_64) {
		uint64 q = ((AddressFlat64*)addr)->addr - pe_shared->pe64.header_nt.image_base;
		if (q >> 32) return false;
		*r = q;
		return true;
	}
	return false;
}

/*
 *
 */
Address *PEAnalyser::createAddress32(uint32 addr)
{
	switch (pe_shared->coffheader.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
			return new AddressX86Flat32(addr);
	}
	// fallback to standard-addrs
	return new AddressFlat32(addr);
}

/*
 *
 */
Address *PEAnalyser::createAddress64(uint64 addr)
{
	return new AddressFlat64(addr);
}

Address *PEAnalyser::createAddress()
{
	switch (pe_shared->coffheader.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
			if (pe_shared->opt_magic == COFF_OPTMAGIC_PE64) {
				return new AddressFlat64();
			} else {
				return new AddressX86Flat32();
			}
	}
	if (pe_shared->opt_magic == COFF_OPTMAGIC_PE64) {
		return new AddressFlat64();
	}
	return new AddressFlat32();
}

/*
 *
 */
Assembler *PEAnalyser::createAssembler()
{
	Assembler *a = NULL;
	switch (pe_shared->coffheader.machine) {
	case COFF_MACHINE_I386:
	case COFF_MACHINE_I486:
	case COFF_MACHINE_I586:
		a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
		a->init();
		return a;
	case COFF_MACHINE_AMD64:
		a = new x86_64asm();
		a->init();
		return a;
	}
	return a;
}

/*
 *
 */
FileOfs PEAnalyser::addressToFileofs(Address *Addr)
{
/*     char tbuf[1024];
	Addr->stringify(tbuf, 1024, 0);
	printf("ADDR=%s", tbuf);*/
	if (validAddress(Addr, scinitialized)) {
//     	printf(" v1\n");
		FileOfs ofs;
		RVA r;
		if (!convertAddressToRVA(Addr, &r)) return INVALID_FILE_OFS;
		if (!pe_rva_to_ofs(&pe_shared->sections, r, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
//     	printf(" IV1\n");
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
const char *PEAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char sectionname[9];
	pe_section_headers *sections=&pe_shared->sections;
	int i;
	RVA r;
//	Addr-=pe_shared->pe32.header_nt.image_base;
	if (!convertAddressToRVA(Addr, &r)) return NULL;
	pe_rva_to_section(sections, r, &i);
	COFF_SECTION_HEADER *s=sections->sections+i;
	if (!pe_rva_is_valid(sections, r)) return NULL;
	memcpy(sectionname, s->name, 8);
	sectionname[8] = 0;
	return sectionname;
}

/*
 *
 */
String &PEAnalyser::getName(String &s)
{
	return file->getDesc(s);
}

/*
 *
 */
const char *PEAnalyser::getType()
{
	return "PE/Analyser";
}

/*
 *
 */
void PEAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

static char *string_func(uint32 ofs, void *context)
{
	char str[1024];
	static char str2[1024];
	ht_pe_shared_data *pe = (ht_pe_shared_data*)context;
	if (ofs < pe->il->string_pool_size) {
		uint32 length;
		uint32 o = ILunpackDword(length, (byte*)&pe->il->string_pool[ofs], 10);
		wide_char_to_multi_byte(str, (byte*)&pe->il->string_pool[ofs+o], length/2+1);
		escape_special_str(str2, sizeof str2, str, "\"");
		return str2;
	} else {
		return NULL;
	}
}

static char *token_func(uint32 token, void *context)
{
	static char tokenstr[1024];
//	ht_pe_shared_data *pe = (ht_pe_shared_data*)context;
	switch (token & IL_META_TOKEN_MASK) {
	case IL_META_TOKEN_TYPE_REF:
	case IL_META_TOKEN_TYPE_DEF: {
		sprintf(tokenstr, "typedef");
		break;
	}
	case IL_META_TOKEN_FIELD_DEF: {
		sprintf(tokenstr, "fielddef");
		break;
	}
	case IL_META_TOKEN_METHOD_DEF: {
		sprintf(tokenstr, "methoddef");
		break;
	}
	case IL_META_TOKEN_MEMBER_REF: {
		sprintf(tokenstr, "memberref");
		break;
	}
	case IL_META_TOKEN_TYPE_SPEC: {
		sprintf(tokenstr, "typespec");
		break;
	}
	default:
		return NULL;
	}
	return tokenstr;
}

/*
 *
 */
void PEAnalyser::initUnasm()
{
	bool pe64 = false;
	if (pe_shared->opt_magic == COFF_OPTMAGIC_PE64) {
		pe64 = true;
	}
	DPRINTF("pe_analy: ");
	if (pe_shared->il) {
		analy_disasm = new AnalyILDisassembler();
		((AnalyILDisassembler *)analy_disasm)->init(this, string_func, token_func, pe_shared);
	} else {
		switch (pe_shared->coffheader.machine) {
		case COFF_MACHINE_I386:	// Intel 386
		case COFF_MACHINE_I486:	// Intel 486
		case COFF_MACHINE_I586:	// Intel 586
			if (pe64) {
				errorbox("x86 cant be used in PE64 format.");
			} else {
				DPRINTF("initing analy_x86_disassembler\n");
				analy_disasm = new AnalyX86Disassembler();
				((AnalyX86Disassembler *)analy_disasm)->init(this, 0);
			}
			break;
		case COFF_MACHINE_AMD64:
			if (!pe64) {
				errorbox("x86_64 cant be used in PE32 format.");
			} else {
				analy_disasm = new AnalyX86Disassembler();
				((AnalyX86Disassembler *)analy_disasm)->init(this, ANALYX86DISASSEMBLER_FLAGS_AMD64);
			}
			break;
		case COFF_MACHINE_R3000:	// MIPS little-endian, 0x160 big-endian
			DPRINTF("no apropriate disassembler for MIPS\n");
			warnbox("No disassembler for MIPS!");
			break;
		case COFF_MACHINE_R4000:	// MIPS little-endian
			DPRINTF("no apropriate disassembler for MIPS\n");
			warnbox("No disassembler for MIPS!");
			break;
		case COFF_MACHINE_R10000:	// MIPS little-endian
			DPRINTF("no apropriate disassembler for MIPS\n");
			warnbox("No disassembler for MIPS!");
			break;
		case COFF_MACHINE_ALPHA:	// Alpha_AXP
			DPRINTF("initing alpha_axp_disassembler\n");
			analy_disasm = new AnalyAlphaDisassembler();
			((AnalyAlphaDisassembler *)analy_disasm)->init(this);
			break;
		case COFF_MACHINE_POWERPC_LE:	// IBM PowerPC Little-Endian
			DPRINTF("no apropriate disassembler for POWER PC\n");
			warnbox("No disassembler for little endian POWER PC!");
			break;
		case COFF_MACHINE_POWERPC_BE:
		case COFF_MACHINE_POWERPC64_BE:
			analy_disasm = new AnalyPPCDisassembler();
			((AnalyPPCDisassembler*)analy_disasm)->init(this, pe64 ? ANALY_PPC_64 : ANALY_PPC_32);
			break;
		case COFF_MACHINE_IA64:
			if (!pe64) {
				errorbox("Intel IA64 cant be used in PE32 format.");
			} else {
				analy_disasm = new AnalyIA64Disassembler();
				((AnalyIA64Disassembler*)analy_disasm)->init(this);
			}
			break;
		case COFF_MACHINE_ARM: // ARM
		case COFF_MACHINE_THUMB: // Thumb
			DPRINTF("initing arm_disassembler\n");
			analy_disasm = new AnalyArmDisassembler();
			((AnalyArmDisassembler *)analy_disasm)->init(this);
                        break;
		case COFF_MACHINE_UNKNOWN:
		default:
			DPRINTF("no apropriate disassembler for machine %04x\n", pe_shared->coffheader.machine);
			warnbox("No disassembler for unknown machine type %04x!", pe_shared->coffheader.machine);
		}
	}
}

/*
 *
 */
void PEAnalyser::log(const char *msg)
{
	/*
	 *	log() creates to much traffic so dont log
	 *   perhaps we reactivate this later
	 *
	 */
/*	LOG(msg);*/
}

/*
 *
 */
Address *PEAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void PEAnalyser::store(ObjectStream &st) const
{
	PUT_OBJECT(st, validarea);
	Analyser::store(st);
}

/*
 *
 */
int	PEAnalyser::queryConfig(int mode)
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
Address *PEAnalyser::fileofsToAddress(FileOfs fileofs)
{
	RVA r;
	if (pe_ofs_to_rva(&pe_shared->sections, fileofs, &r)) {
		if (pe_shared->opt_magic == COFF_OPTMAGIC_PE32) {
			return createAddress32(r + pe_shared->pe32.header_nt.image_base);
		} else {
			return createAddress64(r + pe_shared->pe64.header_nt.image_base);
		}
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool PEAnalyser::validAddress(Address *Addr, tsectype action)
{
	pe_section_headers *sections=&pe_shared->sections;
	int sec;
	RVA r;
	if (!convertAddressToRVA(Addr, &r)) return false;
	if (!pe_rva_to_section(sections, r, &sec)) return false;
	COFF_SECTION_HEADER *s=sections->sections+sec;
	switch (action) {
	case scvalid:
		return true;
	case scread:
		return s->characteristics & COFF_SCN_MEM_READ;
	case scwrite:
		return s->characteristics & COFF_SCN_MEM_WRITE;
	case screadwrite:
		return s->characteristics & COFF_SCN_MEM_WRITE;
	case sccode:
		// FIXME: EXECUTE vs. CNT_CODE ?
		if (!pe_rva_is_physical(sections, r)) return false;
		return (s->characteristics & (COFF_SCN_MEM_EXECUTE | COFF_SCN_CNT_CODE));
	case scinitialized:
		if (!pe_rva_is_physical(sections, r)) return false;
		return true;
		// !(s->characteristics & COFF_SCN_CNT_UNINITIALIZED_DATA);
	}
	return false;
}


