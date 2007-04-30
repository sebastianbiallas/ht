/*
 *	HT Editor
 *	coff_analy.cc
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
#include "analy_names.h"
#include "analy_register.h"
#include "analy_ppc.h"
#include "analy_x86.h"
#include "analy_arm.h"
#include "coff_analy.h"
#include "coff_s.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htcoff.h"
#include "strtools.h"
#include "snprintf.h"
#include "x86asm.h"

/*
 *
 */
void	CoffAnalyser::init(ht_coff_shared_data *Coff_shared, File *File)
{
	coff_shared = Coff_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();

	/////////////

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);
}


/*
 *
 */
void	CoffAnalyser::load(ObjectStream &f)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	GET_OBJECT(f, validarea);
	Analyser::load(f);
}

/*
 *
 */
void	CoffAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

/*
 *
 */
void CoffAnalyser::beginAnalysis()
{
//	char	buffer[1024];

	/*
	 *	entrypoint
	 */
	Address *entry=createAddress32(coff_shared->coff32header.entrypoint_address);
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
	COFF_SECTION_HEADER *s=coff_shared->sections.sections;
	char blub[100];
	for (uint i=0; i<coff_shared->sections.section_count; i++) {
		Address *secaddr = createAddress32(s->data_address);
		sprintf(blub, ";  section %d <%s>", i+1, getSegmentNameByAddress(secaddr));
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  virtual address  %08x  size   %08x", s->data_address, s->data_size);
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  file offset      %08x", s->data_offset+coff_shared->hdr_ofs);
		addComment(secaddr, 0, blub);
		addComment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		sprintf(blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
		Address *secend_addr = secaddr->clone();
		secend_addr->add(MAX(s->data_size, s->data_vsize));
		newLocation(secend_addr)->flags |= AF_FUNCTION_END;
		addComment(secend_addr, 0, "");
		addComment(secend_addr, 0, ";******************************************************************");
		addComment(secend_addr, 0, blub);
		addComment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		Address *secini_addr = secaddr->clone();
		secini_addr->add(MIN(s->data_size, s->data_vsize));
		if (validAddress(secaddr, scinitialized) && validAddress(secini_addr, scinitialized)) {
			initialized->add(secaddr, secini_addr);
		}
		s++;
		delete secaddr;
		delete secend_addr;
		delete secini_addr;
	}

	// exports

/*	int export_count=coff_shared->exports.funcs->count();
	int *entropy = random_permutation(export_count);
	for (int i=0; i<export_count; i++) {
		ht_pe_export_function *f=(ht_pe_export_function *)pe_shared->exports.funcs->get(*(entropy+i));
		if (valid_addr(f->address, scvalid)) {
			char *label;
			if (f->byname) {
				sprintf(buffer, "; exported function %s, ordinal %04x", f->name, f->ordinal);
			} else {
				sprintf(buffer, "; unnamed exported function, ordinal %04x", f->ordinal);
			}
			label = export_func_name((f->byname) ? f->name : NULL, f->ordinal);
			add_comment(f->address, 0, "");
			add_comment(f->address, 0, ";********************************************************");
			add_comment(f->address, 0, buffer);
			add_comment(f->address, 0, ";********************************************************");
			push_addr(f->address, f->address);
			assign_label(f->address, label, label_func);
			free(label);
		}
	}
	if (entropy) free(entropy);*/

/*	int import_count=pe_shared->imports.funcs->count();
	entropy = random_permutation(import_count);
	for (int i=0; i<import_count; i++) {
		ht_pe_import_function *f=(ht_pe_import_function *)pe_shared->imports.funcs->get(*(entropy+i));
		ht_pe_import_library *d=(ht_pe_import_library *)pe_shared->imports.libs->get(f->libidx);
		char *label;
		label = import_func_name(d->name, (f->byname) ? f->name.name : NULL, f->ordinal);
		add_comment(f->address, 0, "");
		assign_label(f->address, label, label_func);
		data->set_int_addr_type(f->address, dst_idword, 4);
		free(label);
	}
	if (entropy) free(entropy);

	int dimport_count=pe_shared->dimports.funcs->count();
	entropy = random_permutation(dimport_count);
	for (int i=0; i<dimport_count; i++) {
		// FIXME: delay imports need work (push addr)
		ht_pe_import_function *f=(ht_pe_import_function *)pe_shared->dimports.funcs->get(*(entropy+i));
		ht_pe_import_library *d=(ht_pe_import_library *)pe_shared->dimports.libs->get(f->libidx);
		char *label;
		label = import_func_name(d->name, f->byname ? f->name.name : NULL, f->ordinal);
		add_comment(f->address, 0, "");
		assign_label(f->address, label, label_func);
//		data->setintaddrtype(f->address, dstidword, 4);
		free(label);
	}
	if (entropy) free(entropy);*/

	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	if (coff_shared->coffheader.characteristics & COFF_DLL) {
		addComment(entry, 0, ";  library entry point");
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
ObjectID	CoffAnalyser::getObjectID() const
{
	return ATOM_COFF_ANALYSER;
}

/*
 *
 */
uint CoffAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool CoffAnalyser::convertAddressToRVA(Address *addr, RVA *r)
{
	if (addr->getObjectID()==ATOM_ADDRESS_X86_FLAT_32) {
		*r = ((AddressX86Flat32*)addr)->addr;
	} else if (addr->getObjectID()==ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr;
	} else {
		return false;
	}
	return true;
}

Address *CoffAnalyser::createAddress()
{
	switch (coff_shared->coffheader.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
			return new AddressX86Flat32(0);
		default:
			return new AddressFlat32(0);
	}
}

Address *CoffAnalyser::createAddress32(uint32 addr)
{
	switch (coff_shared->coffheader.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
			return new AddressX86Flat32(addr);
		default:
			return new AddressFlat32(addr);
	}
}

/*
 *
 */
Assembler *CoffAnalyser::createAssembler()
{
	switch (coff_shared->coffheader.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
			Assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
			a->init();
			return a;
	}
	return NULL;
}


/*
 *
 */
FileOfs CoffAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FileOfs ofs;
//		Addr-=pe_shared->pe32.header_nt.image_base;
		RVA rva;
		if (!convertAddressToRVA(Addr, &rva)) return INVALID_FILE_OFS;
		if (!coff_rva_to_ofs(&coff_shared->sections, rva, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
const char *CoffAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char sectionname[9];
	coff_section_headers *sections=&coff_shared->sections;
	int i;
	RVA rva;
	if (!convertAddressToRVA(Addr, &rva)) return NULL;
	coff_rva_to_section(sections, rva, &i);
	COFF_SECTION_HEADER *s=sections->sections+i;
	if (!coff_rva_is_valid(sections, rva)) return NULL;
	memcpy(sectionname, s->name, 8);
	sectionname[8] = 0;
	return sectionname;
}

/*
 *
 */
String &CoffAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *CoffAnalyser::getType()
{
	return "COFF/Analyser";
}

/*
 *
 */
void CoffAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void CoffAnalyser::initUnasm()
{
	DPRINTF("coff_analy: ");
	switch (coff_shared->coffheader.machine) {
		case COFF_MACHINE_I386:	// Intel 386
		case COFF_MACHINE_I486:	// Intel 486
		case COFF_MACHINE_I586:	// Intel 586
			DPRINTF("initing analy_x86_disassembler\n");
			analy_disasm = new AnalyX86Disassembler();
			((AnalyX86Disassembler*)analy_disasm)->init(this, 0);
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
			analy_disasm = new AnalyPPCDisassembler();
			((AnalyPPCDisassembler*)analy_disasm)->init(this, ANALY_PPC_32);
			break;
		case COFF_MACHINE_ARM:
		case COFF_MACHINE_THUMB:
			analy_disasm = new AnalyArmDisassembler();
			((AnalyArmDisassembler*)analy_disasm)->init(this);
			break;
		case COFF_MACHINE_UNKNOWN:
		default:
			DPRINTF("no apropriate disassembler for machine %04x\n", coff_shared->coffheader.machine);
			warnbox("No disassembler for unknown machine type %04x!", coff_shared->coffheader.machine);
	}
}

/*
 *
 */
void CoffAnalyser::log(const char *msg)
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
Address *CoffAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void CoffAnalyser::store(ObjectStream &st) const
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	PUT_OBJECT(st, validarea);
	Analyser::store(st);
}

/*
 *
 */
int	CoffAnalyser::queryConfig(int mode)
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
Address *CoffAnalyser::fileofsToAddress(FileOfs fileofs)
{
	RVA a;
	if (coff_ofs_to_rva(&coff_shared->sections, fileofs, &a)) {
		return createAddress32(a);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool CoffAnalyser::validAddress(Address *Addr, tsectype action)
{
	coff_section_headers *sections=&coff_shared->sections;
	int sec;
	RVA rva;
	if (!convertAddressToRVA(Addr, &rva)) return false;
	if (!coff_rva_to_section(sections, rva, &sec)) return false;
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
			if (!coff_rva_is_physical(sections, rva)) return false;
			return (s->characteristics & (COFF_SCN_MEM_EXECUTE | COFF_SCN_CNT_CODE));
		case scinitialized:
			if (!coff_rva_is_physical(sections, rva)) return false;
			return !(s->characteristics & COFF_SCN_CNT_UNINITIALIZED_DATA);
	}
	return false;
}


