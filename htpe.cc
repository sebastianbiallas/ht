/*
 *	HT Editor
 *	htpe.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#include "log.h"
#include "endianess.h"
#include "htnewexe.h"
#include "htpe.h"
#include "htpehead.h"
#include "htpeexp.h"
#include "htpeil.h"
#include "htpeimp.h"
#include "htpedimp.h"
#include "htpeimg.h"
#include "htperes.h"
#include "stream.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

static format_viewer_if *htpe_ifs[] = {
	&htpeheader_if,
	&htpeexports_if,
	&htpeimports_if,
	&htpedelayimports_if,
	&htperesources_if,
	&htpeil_if,
	&htpeimage_if,
	0
};

static ht_view *htpe_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte pemagic[4];
	FileOfs h = get_newexe_header_ofs(file);
	file->seek(h);
	;
	if (file->read(pemagic, 4) != 4 
	 || pemagic[0] != PE_MAGIC0 || pemagic[1] != PE_MAGIC1
	 || pemagic[2] != PE_MAGIC2 || pemagic[3] != PE_MAGIC3) return 0;

	ht_pe *g = new ht_pe();
	g->init(b, file, htpe_ifs, format_group, h);
	return g;
}

format_viewer_if htpe_if = {
	htpe_init,
	0
};

/*
 *	CLASS ht_pe
 */
void ht_pe::init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_PE, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_pe");

	String fn;
	LOG("%y: PE: found header at 0x%08qx", &file->getFilename(fn), header_ofs);
	ht_pe_shared_data *pe_shared = ht_malloc(sizeof (ht_pe_shared_data));
	shared_data = pe_shared;
	pe_shared->header_ofs = header_ofs;

	pe_shared->exports.funcs = new Array(true);
	pe_shared->dimports.funcs = new Array(true);
	pe_shared->dimports.libs = new Array(true);
	pe_shared->imports.funcs = new Array(true);
	pe_shared->imports.libs = new Array(true);

	pe_shared->il = NULL;
	pe_shared->v_image = NULL;
	pe_shared->v_dimports = NULL;
	pe_shared->v_imports = NULL;
	pe_shared->v_exports = NULL;
	pe_shared->v_resources = NULL;
	pe_shared->v_header = NULL;

	/* read header */
	file->seek(header_ofs+4);
	file->readx(&pe_shared->coffheader, sizeof pe_shared->coffheader);
	createHostStruct(&pe_shared->coffheader, COFF_HEADER_struct, little_endian);
	file->readx(&pe_shared->opt_magic, sizeof pe_shared->opt_magic);
	pe_shared->opt_magic = createHostInt(&pe_shared->opt_magic, sizeof pe_shared->opt_magic, little_endian);
	file->seek(header_ofs+4+sizeof pe_shared->coffheader);
	switch (pe_shared->opt_magic) {
	case COFF_OPTMAGIC_PE32:
		file->readx(&pe_shared->pe32.header, sizeof pe_shared->pe32.header);
		createHostStruct(&pe_shared->pe32.header, COFF_OPTIONAL_HEADER32_struct, little_endian);
		file->readx(&pe_shared->pe32.header_nt, sizeof pe_shared->pe32.header_nt);
		createHostStruct(&pe_shared->pe32.header_nt, PE_OPTIONAL_HEADER32_NT_struct, little_endian);
		for (uint i=0; i<PE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
			createHostStruct(&pe_shared->pe32.header_nt.directory[i], PE_DATA_DIRECTORY_struct, little_endian);
		}
		break;
	case COFF_OPTMAGIC_PE64:
		file->readx(&pe_shared->pe64.header, sizeof pe_shared->pe64.header);
		createHostStruct(&pe_shared->pe64.header, COFF_OPTIONAL_HEADER64_struct, little_endian);
		file->readx(&pe_shared->pe64.header_nt, sizeof pe_shared->pe64.header_nt);
		createHostStruct(&pe_shared->pe64.header_nt, PE_OPTIONAL_HEADER64_NT_struct, little_endian);
		for (uint i=0; i<PE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
			createHostStruct(&pe_shared->pe64.header_nt.directory[i], PE_DATA_DIRECTORY_struct, little_endian);
		}
		break;
	}

	/* read section headers */
	int os=pe_shared->coffheader.optional_header_size;
	pe_shared->sections.section_count=pe_shared->coffheader.section_count;

	file->seek(header_ofs+os+sizeof(COFF_HEADER)+4/*magic*/);
	pe_shared->sections.sections = ht_malloc(pe_shared->sections.section_count * sizeof *pe_shared->sections.sections);
	file->readx(pe_shared->sections.sections, pe_shared->sections.section_count*sizeof *pe_shared->sections.sections);

	for (uint i=0; i<pe_shared->sections.section_count; i++) {
		createHostStruct(&pe_shared->sections.sections[i], COFF_SECTION_HEADER_struct, little_endian);
		/*
		 *	To make those uninitialized/initialized flags
		 *	correct we guess a little
		 *
		if (pe_shared->sections.sections[i].data_size &&
			pe_shared->sections.sections[i].data_offset) {

			pe_shared->sections.sections[i].characteristics |= ;
			pe_shared->sections.sections[i].characteristics ~= ;
		} else {
			pe_shared->sections.sections[i].characteristics |= ;
			pe_shared->sections.sections[i].characteristics ~= ;
		}*/
	}

	shared_data = pe_shared;

	ht_format_group::init_ifs(ifs);
}

void ht_pe::done()
{
	ht_format_group::done();

	ht_pe_shared_data *pe_shared = (ht_pe_shared_data*)shared_data;

	delete pe_shared->exports.funcs;
	delete pe_shared->dimports.funcs;
	delete pe_shared->dimports.libs;
	delete pe_shared->imports.funcs;
	delete pe_shared->imports.libs;

	free(pe_shared->sections.sections);
	free(shared_data);
}

void ht_pe::loc_enum_start()
{
/*
	ht_pe_shared_data *sh=(ht_pe_shared_data*)shared_data;
	if (sh->opt_magic==COFF_OPTMAGIC_PE32) {
		loc_enum=1;
	} else {
		loc_enum=0;
	}
*/
}

bool ht_pe::loc_enum_next(ht_format_loc *loc)
{
#if 0
	ht_pe_shared_data *sh=(ht_pe_shared_data*)shared_data;
	if (loc_enum) {
		loc->name="pe";
		loc->start=sh->header_ofs;
		// calc pe size
		uint l=sizeof (COFF_HEADER) + sh->coffheader.optional_header_size;
		// go through directories
		for (uint i=0; i<16; i++) {
			FileOfs o;
			if (pe_rva_to_ofs(&sh->sections, sh->pe32.header_nt.directory[i].address, &o)) {
				uint k=o+sh->pe32.header_nt.directory[i].size-sh->header_ofs;
				l=MAX(k, l);
			}
		}
		// go through sections
		for (uint i=0; i<sh->sections.section_count; i++) {
			uint k=sh->sections.sections[i].data_offset+sh->sections.sections[i].data_size-sh->header_ofs;
			l=MAX(k, l);
		}
		loc->length=l;
		
		loc_enum=0;
		return true;
	}
	return false;
#endif
	return false;
}

/*
 *	rva conversion routines
 */

bool pe_rva_to_ofs(pe_section_headers *section_headers, RVA rva, FileOfs *ofs)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
		(rva<s->data_address+s->data_size)) {
			*ofs=rva-s->data_address+s->data_offset;
			return true;
		}
		s++;
	}
	return false;
}

bool pe_rva_to_section(pe_section_headers *section_headers, RVA rva, int *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
		(rva<s->data_address+MAX(s->data_size, s->data_vsize))) {
			*section=i;
			return true;
		}
		s++;
	}
	return false;
}

bool pe_rva_is_valid(pe_section_headers *section_headers, RVA rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
		(rva<s->data_address+MAX(s->data_size, s->data_vsize))) {
			return true;
		}
		s++;
	}
	return false;
}

bool pe_rva_is_physical(pe_section_headers *section_headers, RVA rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
		(rva<s->data_address+s->data_size)) {
			return true;
		}
		s++;
	}
	return false;
}

/*
 *	ofs conversion routines
 */

bool pe_ofs_to_rva(pe_section_headers *section_headers, FileOfs ofs, RVA *rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset) &&
		(ofs<s->data_offset+s->data_size)) {
			*rva=ofs-s->data_offset+s->data_address;
			return true;
		}
		s++;
	}
	return false;
}

bool pe_ofs_to_section(pe_section_headers *section_headers, FileOfs ofs, int *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset) &&
		(ofs<s->data_offset+s->data_size)) {
			*section=i;
			return true;
		}
		s++;
	}
	return false;
}

bool pe_ofs_to_rva_and_section(pe_section_headers *section_headers, FileOfs ofs, RVA *rva, int *section)
{
	bool r = pe_ofs_to_rva(section_headers, ofs, rva);
	if (r) {
		r = pe_ofs_to_section(section_headers, ofs, section);
	}
	return r;
}

bool pe_ofs_is_valid(pe_section_headers *section_headers, FileOfs ofs)
{
	RVA rva;
	return pe_ofs_to_rva(section_headers, ofs, &rva);
}

/*
 *
 */
 
bool pe_section_name_to_section(pe_section_headers *section_headers, const char *name, int *section)
{
	COFF_SECTION_HEADER *s = section_headers->sections;
	int slen = strlen(name);
	slen = MIN(slen, COFF_SIZEOF_SHORT_NAME);
	for (uint i=0; i < section_headers->section_count; i++) {
		if (ht_strncmp(name, (char*)&s->name, slen) == 0) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}
