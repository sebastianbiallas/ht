/*
 *	HT Editor
 *	class_analy.cc
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
#include "analy_register.h"
#include "analy_java.h"
#include "global.h"
#include "class_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htstring.h"
#include "pestruct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
void	ClassAnalyser::init(ht_class_shared_data *Class_shared, ht_streamfile *File)
{
	class_shared = Class_shared;
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
int	ClassAnalyser::load(ht_object_stream *f)
{
	GET_OBJECT(f, validarea);
	return Analyser::load(f);
}

/*
 *
 */
void	ClassAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

/*
 *
 */
void ClassAnalyser::beginAnalysis()
{
#if 0
	char	buffer[1024];

	/*
	 *	entrypoint
	 */
	Address entry=createAddress32(pe_shared->pe32.header.entrypoint_address+pe_shared->pe32.header_nt.image_base);

	pushAddress(entry, entry);
	
	/*
	 * give all sections a descriptive comment:
	 */

	/*struct PE_SECTION_HEADER {
		byte name[PE_SIZEOF_SHORT_NAME] __attribute__ ((packed));
		dword data_vsize __attribute__ ((packed));
		dword data_address __attribute__ ((packed));
		dword data_size __attribute__	((packed));
		dword data_offset __attribute__ ((packed));
		dword relocation_offset __attribute__ ((packed));
		dword linenumber_offset __attribute__ ((packed));
		word relocation_count __attribute__ ((packed));
		word linenumber_count __attribute__ ((packed));
		dword characteristics __attribute__ ((packed));
	};*/
	COFF_SECTION_HEADER *s=pe_shared->sections.sections;
	char blub[100];
	for (UINT i=0; i<pe_shared->sections.section_count; i++) {
		ADDR secaddr = s->data_address+pe_shared->pe32.header_nt.image_base;
		sprintf(blub, ";  section %d <%s>", i+1, get_addr_section_name(secaddr));
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  virtual address  %08x  virtual size   %08x", s->data_address, s->data_vsize);
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  file offset      %08x  file size      %08x", s->data_offset, s->data_size);
		addComment(secaddr, 0, blub);
		addComment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		sprintf(blub, ";  end of section <%s>", get_addr_section_name(secaddr));
		ADDR secend_addr = secaddr + MAX(s->data_size, s->data_vsize);
		new_addr(secend_addr)->flags |= AF_FUNCTION_END;
		addComment(secend_addr, 0, "");
		addComment(secend_addr, 0, ";******************************************************************");
		addComment(secend_addr, 0, blub);
		addComment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		if (valid_addr(secaddr, scinitialized) && valid_addr(secaddr + MIN(s->data_size, s->data_vsize), scinitialized)) {
			initialized->add(secaddr, secaddr + MIN(s->data_size, s->data_vsize));
		}
		s++;
	}

	// exports

	int export_count=pe_shared->exports.funcs->count();
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
			addComment(f->address, 0, "");
			addComment(f->address, 0, ";********************************************************");
			addComment(f->address, 0, buffer);
			addComment(f->address, 0, ";********************************************************");
			push_addr(f->address, f->address);
			assign_label(f->address, label, label_func);
			free(label);
		}
	}
	if (entropy) free(entropy);

	int import_count=pe_shared->imports.funcs->count();
	entropy = random_permutation(import_count);
	for (int i=0; i<import_count; i++) {
		ht_pe_import_function *f=(ht_pe_import_function *)pe_shared->imports.funcs->get(*(entropy+i));
		ht_pe_import_library *d=(ht_pe_import_library *)pe_shared->imports.libs->get(f->libidx);
		char *label;
		label = import_func_name(d->name, (f->byname) ? f->name.name : NULL, f->ordinal);
		addComment(f->address, 0, "");
		if (!assign_label(f->address, label, label_func)) {
			// multiple import of a function (duplicate labelname)
			// -> mangle name a bit more
			addComment(f->address, 0, "; duplicate import");
			sprintf(buffer, "%s_%x", label, f->address);
			assign_label(f->address, buffer, label_func);
		}
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
		if (f->byname) {
			sprintf(buffer, "; delay import function loader for %s, ordinal %04x", f->name.name, f->ordinal);
		} else {
			sprintf(buffer, "; delay import function loader for ordinal %04x", f->ordinal);
		}
		char *label;
		label = import_func_name(d->name, f->byname ? f->name.name : NULL, f->ordinal);
		addComment(f->address, 0, "");
		addComment(f->address, 0, ";********************************************************");
		addComment(f->address, 0, buffer);
		addComment(f->address, 0, ";********************************************************");
		assign_label(f->address, label, label_func);
		free(label);
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
	assign_label(entry, "entrypoint", label_func);
#endif
	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);

	Analyser::beginAnalysis();
}

/*
 *
 */
OBJECT_ID	ClassAnalyser::object_id()
{
	return ATOM_CLASS_ANALYSER;
}

/*
 *
 */
UINT ClassAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FILEOFS ofs = addressToFileofs(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

/*
 *
 */
Address *ClassAnalyser::createAddress()
{
	return new AddressFlat32(0);
}

/*
 *
 */
Address *ClassAnalyser::createAddress32(dword addr)
{
	return new AddressFlat32(addr);
}

/*
 *
 */
Assembler *ClassAnalyser::createAssembler()
{
	return NULL;
}

/*
 *
 */
FILEOFS ClassAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		return ((AddressFlat32*)Addr)->addr;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
char *ClassAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char sectionname[9];
	strcpy(sectionname, "test");
	return sectionname;
}

/*
 *
 */
char	*ClassAnalyser::getName()
{
	return file->get_desc();
}

/*
 *
 */
char *ClassAnalyser::getType()
{
	return "Java-Class/Analyser";
}

/*
 *
 */
void ClassAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void ClassAnalyser::initUnasm()
{
	DPRINTF("class_analy: ");
	analy_disasm = new AnalyJavaDisassembler();
	((AnalyJavaDisassembler*)analy_disasm)->init(this);
}

/*
 *
 */
void ClassAnalyser::log(char *msg)
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
Address *ClassAnalyser::nextValid(Address *Addr)
{
	return new InvalidAddress(); //validarea->findnext(Addr);
	// FIXME (hack while validarea isnt active):
//   taddr *a = enum_addrs(Addr);
//   return (a)?a->addr:INVALID_ADDR;
}

/*
 *
 */
void ClassAnalyser::store(ht_object_stream *st)
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
int	ClassAnalyser::queryConfig(int mode)
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
Address *ClassAnalyser::fileofsToAddress(FILEOFS fileaddr)
{
//	ADDR a;
/*	if (pe_ofs_to_rva(&pe_shared->sections, fileaddr, &a)) {
		return (a+pe_shared->pe32.header_nt.image_base);
	} else {
		return INVALID_ADDR;
	}*/
//     return fileaddr;
	return NULL;
}

/*
 *
 */
bool ClassAnalyser::validAddress(Address *Addr, tsectype action)
{
/*	pe_section_headers *sections=&pe_shared->sections;
	int sec;
	Addr-=pe_shared->pe32.header_nt.image_base;
	if (!pe_rva_to_section(sections, Addr, &sec)) return false;
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
			if (!pe_rva_is_physical(sections, Addr)) return false;
			return (s->characteristics & (COFF_SCN_MEM_EXECUTE | COFF_SCN_CNT_CODE));
		case scinitialized:
			if (!pe_rva_is_physical(sections, Addr)) return false;
			return !(s->characteristics & COFF_SCN_CNT_UNINITIALIZED_DATA);
	}*/
	if (!Addr->isValid()) return false;
	return (((AddressFlat32*)Addr)->addr <= 427);
}


