/*
 *	HT Editor
 *	pe_analy.cc
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
#include "analy_x86.h"
#include "codeanaly.h"
#include "global.h"
#include "pe_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htpe.h"
#include "htstring.h"
#include "pestruct.h"
#include "x86asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
void	pe_analyser::init(ht_pe_shared_data *Pe_shared, ht_streamfile *File)
{
	pe_shared = Pe_shared;
	file = File;

	validarea = new area();
	validarea->init();

	analyser::init();

	/////////////

	set_addr_tree_optimize_threshold(100);
	set_label_tree_optimize_threshold(100);
}


/*
 *
 */
int	pe_analyser::load(ht_object_stream *f)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	GET_OBJECT(f, validarea);
	return analyser::load(f);
}

/*
 *
 */
void	pe_analyser::done()
{
	validarea->done();
	delete validarea;
	analyser::done();
}

/*
 *
 */
void pe_analyser::begin_analysis()
{
	char	buffer[1024];

	/*
	 *	entrypoint
	 */
	ADDR entry=pe_shared->pe32.header.entrypoint_address+pe_shared->pe32.header_nt.image_base;

	push_addr(entry, entry);
	
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
		add_comment(secaddr, 0, "");
		add_comment(secaddr, 0, ";******************************************************************");
		add_comment(secaddr, 0, blub);
		sprintf(blub, ";  virtual address  %08x  virtual size   %08x", s->data_address, s->data_vsize);
		add_comment(secaddr, 0, blub);
		sprintf(blub, ";  file offset      %08x  file size      %08x", s->data_offset, s->data_size);
		add_comment(secaddr, 0, blub);
		add_comment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		sprintf(blub, ";  end of section <%s>", get_addr_section_name(secaddr));
		ADDR secend_addr = secaddr + MAX(s->data_size, s->data_vsize);
		new_addr(secend_addr)->flags |= AF_FUNCTION_END;
		add_comment(secend_addr, 0, "");
		add_comment(secend_addr, 0, ";******************************************************************");
		add_comment(secend_addr, 0, blub);
		add_comment(secend_addr, 0, ";******************************************************************");

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
			add_comment(f->address, 0, "");
			add_comment(f->address, 0, ";********************************************************");
			add_comment(f->address, 0, buffer);
			add_comment(f->address, 0, ";********************************************************");
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
		add_comment(f->address, 0, "");
		if (!assign_label(f->address, label, label_func)) {
			// multiple import of a function (duplicate labelname)
			// -> mangle name a bit more
			add_comment(f->address, 0, "; duplicate import");
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
		add_comment(f->address, 0, "");
		add_comment(f->address, 0, ";********************************************************");
		add_comment(f->address, 0, buffer);
		add_comment(f->address, 0, ";********************************************************");
		assign_label(f->address, label, label_func);
		free(label);
	}
	if (entropy) free(entropy);

	add_comment(entry, 0, "");
	add_comment(entry, 0, ";****************************");
	if (pe_shared->coffheader.characteristics & COFF_DLL) {
		add_comment(entry, 0, ";  dll entry point");
	} else {
		add_comment(entry, 0, ";  program entry point");
	}
	add_comment(entry, 0, ";****************************");
	assign_label(entry, "entrypoint", label_func);

	set_addr_tree_optimize_threshold(1000);
	set_label_tree_optimize_threshold(1000);

	analyser::begin_analysis();
}

/*
 *
 */
OBJECT_ID	pe_analyser::object_id()
{
	return ATOM_PE_ANALYSER;
}

/*
 *
 */
UINT pe_analyser::bufptr(ADDR Addr, byte *buf, int size)
{
	FILEADDR ofs = file_addr(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

/*
 *
 */
assembler *pe_analyser::create_assembler()
{
	switch (pe_shared->coffheader.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
			assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
			a->init();
			return a;
	}
	return NULL;
}

/*
 *
 */
FILEADDR pe_analyser::file_addr(ADDR Addr)
{
	if (valid_addr(Addr, scinitialized)) {
		FILEOFS ofs;
		Addr-=pe_shared->pe32.header_nt.image_base;
		if (!pe_rva_to_ofs(&pe_shared->sections, Addr, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
char *pe_analyser::get_addr_section_name(ADDR Addr)
{
	static char sectionname[9];
	pe_section_headers *sections=&pe_shared->sections;
	int i;
	Addr-=pe_shared->pe32.header_nt.image_base;
	pe_rva_to_section(sections, Addr, &i);
	COFF_SECTION_HEADER *s=sections->sections+i;
	if (!pe_rva_is_valid(sections, Addr)) return NULL;
	memmove(sectionname, s->name, 8);
	sectionname[8]=0;
	return sectionname;
}

/*
 *
 */
char	*pe_analyser::get_name()
{
	return file->get_desc();
}

/*
 *
 */
char *pe_analyser::get_type()
{
	return "PE/Analyser";
}

/*
 *
 */
void pe_analyser::init_code_analyser()
{
	analyser::init_code_analyser();
	code->loaddefs("analyser/sign.def");
}

/*
 *
 */
void pe_analyser::init_unasm()
{
	DPRINTF("pe_analy: ");
	switch (pe_shared->coffheader.machine) {
		case COFF_MACHINE_I386:	// Intel 386
		case COFF_MACHINE_I486:	// Intel 486
		case COFF_MACHINE_I586:	// Intel 586
			DPRINTF("initing analy_x86_disassembler\n");
			analy_disasm = new analy_x86_disassembler();
			((analy_x86_disassembler*)analy_disasm)->init(this);
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
			analy_disasm = new analy_alpha_disassembler();
			analy_disasm->init(this);
			break;
		case COFF_MACHINE_POWERPC:	// IBM PowerPC Little-Endian
			DPRINTF("no apropriate disassembler for POWER PC\n");
			warnbox("No disassembler for POWER PC!");
			break;
		case COFF_MACHINE_UNKNOWN:
		default:
			DPRINTF("no apropriate disassembler for machine %04x\n", pe_shared->coffheader.machine);
			warnbox("No disassembler for unknown machine type %04x!", pe_shared->coffheader.machine);
	}
}

/*
 *
 */
void pe_analyser::log(char *msg)
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
ADDR pe_analyser::next_valid(ADDR Addr)
{
	return INVALID_ADDR; //validarea->findnext(Addr);
	// FIXME (hack while validarea isnt active):
//   taddr *a = enum_addrs(Addr);
//   return (a)?a->addr:INVALID_ADDR;
}

/*
 *
 */
void pe_analyser::store(ht_object_stream *st)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	PUT_OBJECT(st, validarea);
	analyser::store(st);
}

/*
 *
 */
int	pe_analyser::query_config(int mode)
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
ADDR pe_analyser::vaddr(FILEADDR fileaddr)
{
	ADDR a;
	if (pe_ofs_to_rva(&pe_shared->sections, fileaddr, &a)) {
		return (a+pe_shared->pe32.header_nt.image_base);
	} else {
		return INVALID_ADDR;
	}
}

/*
 *
 */
bool pe_analyser::valid_addr(ADDR Addr, tsectype action)
{
	pe_section_headers *sections=&pe_shared->sections;
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
	}
	return false;
}


