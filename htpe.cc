/*
 *	HT Editor
 *	htpe.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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
#include "htendian.h"
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

static ht_view *htpe_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	byte pemagic[4];
	FILEOFS h=get_newexe_header_ofs(file);
	file->seek(h);
	file->read(pemagic, 4);
	if ((pemagic[0]!=PE_MAGIC0) || (pemagic[1]!=PE_MAGIC1) ||
	   (pemagic[2]!=PE_MAGIC2) || (pemagic[3]!=PE_MAGIC3)) return 0;

	ht_pe *g=new ht_pe();
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
void ht_pe::init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS header_ofs)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_PE, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_pe");

	LOG("%s: PE: found header at %08x", file->get_filename(), header_ofs);

	ht_pe_shared_data *pe_shared = (ht_pe_shared_data*)malloc(sizeof (ht_pe_shared_data));
	shared_data = pe_shared;
	pe_shared->header_ofs = header_ofs;

	pe_shared->exports.funcs = new ht_clist();
	pe_shared->exports.funcs->init();

	pe_shared->dimports.funcs = new ht_clist();
	pe_shared->dimports.funcs->init();

	pe_shared->dimports.libs = new ht_clist();
	pe_shared->dimports.libs->init();

	pe_shared->imports.funcs = new ht_clist();
	pe_shared->imports.funcs->init();

	pe_shared->imports.libs = new ht_clist();
	pe_shared->imports.libs->init();

	pe_shared->il = NULL;
	pe_shared->v_image = NULL;
	pe_shared->v_dimports = NULL;
	pe_shared->v_imports = NULL;
	pe_shared->v_exports = NULL;
	pe_shared->v_resources = NULL;
	pe_shared->v_header = NULL;

/* read header */
	file->seek(header_ofs+4);
	file->read(&pe_shared->coffheader, sizeof pe_shared->coffheader);
	create_host_struct(&pe_shared->coffheader, COFF_HEADER_struct, little_endian);
	file->read(&pe_shared->opt_magic, sizeof pe_shared->opt_magic);
	pe_shared->opt_magic = create_host_int(&pe_shared->opt_magic, sizeof pe_shared->opt_magic, little_endian);
	file->seek(header_ofs+4+sizeof pe_shared->coffheader);
	switch (pe_shared->opt_magic) {
		case COFF_OPTMAGIC_PE32: {
			file->read(&pe_shared->pe32.header, sizeof pe_shared->pe32.header);
			create_host_struct(&pe_shared->pe32.header, COFF_OPTIONAL_HEADER32_struct, little_endian);
			file->read(&pe_shared->pe32.header_nt, sizeof pe_shared->pe32.header_nt);
			create_host_struct(&pe_shared->pe32.header_nt, PE_OPTIONAL_HEADER32_NT_struct, little_endian);
			for (UINT i=0; i<PE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
				create_host_struct(&pe_shared->pe32.header_nt.directory[i], PE_DATA_DIRECTORY_struct, little_endian);
			}
			break;
		}
		case COFF_OPTMAGIC_PE64: {
			file->read(&pe_shared->pe64.header, sizeof pe_shared->pe64.header);
			create_host_struct(&pe_shared->pe64.header, COFF_OPTIONAL_HEADER64_struct, little_endian);
			file->read(&pe_shared->pe64.header_nt, sizeof pe_shared->pe64.header_nt);
			create_host_struct(&pe_shared->pe64.header_nt, PE_OPTIONAL_HEADER64_NT_struct, little_endian);
			for (UINT i=0; i<PE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
				create_host_struct(&pe_shared->pe64.header_nt.directory[i], PE_DATA_DIRECTORY_struct, little_endian);
			}
			break;
		}
	}

/* read section headers */
	int os=pe_shared->coffheader.optional_header_size;
	pe_shared->sections.section_count=pe_shared->coffheader.section_count;

	file->seek(header_ofs+os+sizeof(COFF_HEADER)+4/*magic*/);
	pe_shared->sections.sections=(COFF_SECTION_HEADER*)malloc(pe_shared->sections.section_count * sizeof *pe_shared->sections.sections);
	file->read(pe_shared->sections.sections, pe_shared->sections.section_count*sizeof *pe_shared->sections.sections);

	for (UINT i=0; i<pe_shared->sections.section_count; i++) {
		create_host_struct(&pe_shared->sections.sections[i], COFF_SECTION_HEADER_struct, little_endian);
	}

	shared_data = pe_shared;

	ht_format_group::init_ifs(ifs);
}

void ht_pe::done()
{
	ht_format_group::done();

	ht_pe_shared_data *pe_shared = (ht_pe_shared_data*)shared_data;

	if (pe_shared->exports.funcs) {
		pe_shared->exports.funcs->destroy();
		delete pe_shared->exports.funcs;
	}
	if (pe_shared->dimports.funcs) {
		pe_shared->dimports.funcs->destroy();
		delete pe_shared->dimports.funcs;
	}
	if (pe_shared->dimports.libs) {
		pe_shared->dimports.libs->destroy();
		delete pe_shared->dimports.libs;
	}
	if (pe_shared->imports.funcs) {
		pe_shared->imports.funcs->destroy();
		delete pe_shared->imports.funcs;
	}
	if (pe_shared->imports.libs) {
		pe_shared->imports.libs->destroy();
		delete pe_shared->imports.libs;
	}
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
		UINT l=sizeof (COFF_HEADER) + sh->coffheader.optional_header_size;
// go through directories
		for (UINT i=0; i<16; i++) {
			FILEOFS o;
			if (pe_rva_to_ofs(&sh->sections, sh->pe32.header_nt.directory[i].address, &o)) {
				UINT k=o+sh->pe32.header_nt.directory[i].size-sh->header_ofs;
				l=MAX(k, l);
			}
		}
// go through sections
		for (UINT i=0; i<sh->sections.section_count; i++) {
			UINT k=sh->sections.sections[i].data_offset+sh->sections.sections[i].data_size-sh->header_ofs;
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

bool pe_rva_to_ofs(pe_section_headers *section_headers, RVA rva, FILEOFS *ofs)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
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
	for (UINT i=0; i<section_headers->section_count; i++) {
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
	for (UINT i=0; i<section_headers->section_count; i++) {
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
	for (UINT i=0; i<section_headers->section_count; i++) {
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

bool pe_ofs_to_rva(pe_section_headers *section_headers, FILEOFS ofs, RVA *rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset) &&
		(ofs<s->data_offset+s->data_size)) {
			*rva=ofs-s->data_offset+s->data_address;
			return true;
		}
		s++;
	}
	return false;
}

bool pe_ofs_to_section(pe_section_headers *section_headers, FILEOFS ofs, int *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset) &&
		(ofs<s->data_offset+s->data_size)) {
			*section=i;
			return true;
		}
		s++;
	}
	return false;
}

bool pe_ofs_to_rva_and_section(pe_section_headers *section_headers, FILEOFS ofs, RVA *rva, int *section)
{
	bool r = pe_ofs_to_rva(section_headers, ofs, rva);
	if (r) {
		r = pe_ofs_to_section(section_headers, ofs, section);
	}
	return r;
}

bool pe_ofs_is_valid(pe_section_headers *section_headers, FILEOFS ofs)
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
	for (UINT i=0; i < section_headers->section_count; i++) {
		if (strncmp(name, (char*)&s->name, slen) == 0) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}
