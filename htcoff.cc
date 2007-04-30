/*
 *	HT Editor
 *	htcoff.cc
 *
 *	Copyright (C) 1999-2003 Stefan Weyergraf
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

#include "coff_s.h"
#include "log.h"
#include "htcoff.h"
#include "htcoffhd.h"
#include "htcoffimg.h"
#include "endianess.h"
#include "mzstruct.h"
#include "stream.h"

#include <stdlib.h>
#include <string.h>

format_viewer_if *htcoff_ifs[] = {
	&htcoffheader_if,
	&htcoffimage_if,
	0
};

static bool is_coff(File *file, Endianess &endian, FileOfs ofs)
{
	// unfortunately COFF has no magic (urgs). so we have to guess
	// a little bit.
	COFF_HEADER h;
	bool machine_found = false;
	Endianess end;

	// FIXME: I'm not sure little/big-endian assignment for CPUs is incorrect...

	// LITTLE-ENDIAN machines
	file->seek(ofs);
	if (file->read(&h, sizeof h) != sizeof h) return false;
	createHostStruct(&h, COFF_HEADER_struct, little_endian);
	switch (h.machine) {
		case COFF_MACHINE_I386:
		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:
		case COFF_MACHINE_R3000:
		case COFF_MACHINE_R4000:
		case COFF_MACHINE_R10000:
		case COFF_MACHINE_ALPHA:
		case COFF_MACHINE_SH3:
		case COFF_MACHINE_SH4:
		case COFF_MACHINE_ARM:
		case COFF_MACHINE_POWERPC_LE:
//		case COFF_MACHINE_IA64:		// COFF64 not supported
		case COFF_MACHINE_MIPS16:
		case COFF_MACHINE_68k:
//		case COFF_MACHINE_ALPHA_AXP_64:	// COFF64 not supported
		case COFF_MACHINE_MIPSf:
		case COFF_MACHINE_MIPS16f:
			end = little_endian;
			machine_found = true;
	}

	// BIG-ENDIAN machines
	if (!machine_found) {
		file->seek(ofs);
		if (file->read(&h, sizeof h)!=sizeof h) return 0;
		createHostStruct(&h, COFF_HEADER_struct, big_endian);
		switch (h.machine) {
			case COFF_MACHINE_R3000BE:
			case COFF_MACHINE_POWERPC_BE:
				end = big_endian;
				machine_found = true;
		}
	}
	if (!machine_found) return false;
	/* test symbol_table_offset */
//	if ((h.symbol_table_offset>=size) || (h.symbol_table_offset<ofs+sizeof h)) return 0;

	/* test size of optional header */
	switch (h.optional_header_size) {
		case COFF_OPTSIZE_0:
		case COFF_OPTSIZE_COFF32:
		case COFF_OPTSIZE_XCOFF32:
			break;
		default:
			return false;
	}

	/* all tests have completed successfully -> it's a COFF (probably) */
	endian = end;
	return true;
}

static ht_view *htcoff_init(Bounds *b, File *file, ht_format_group *format_group)
{
	FileOfs h;
	Endianess end;
	/* look for pure coff */
	if (!is_coff(file, end, h = 0)) {
		/* look for dj-coff */
		byte mz[2];
		file->seek(0);
		if (file->read(mz, 2) != 2) return 0;
		if ((mz[0] != IMAGE_MZ_MAGIC0) || (mz[1] != IMAGE_MZ_MAGIC1) ||
				(!is_coff(file, end, h = 0x800)))
				return 0;
	}

	ht_coff *g = new ht_coff();
	g->init(b, file, htcoff_ifs, format_group, h, end);
	return g;
}

format_viewer_if htcoff_if = {
	htcoff_init,
	0
};

/*
 *	CLASS ht_coff
 */
void ht_coff::init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs h, Endianess end)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_COFF, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_coff");

	String fn;
	LOG("%y: COFF: found header at 0x%08qx", &file->getFilename(fn), h);
	coff_shared = ht_malloc(sizeof(*coff_shared));
	coff_shared->hdr_ofs = h;
	coff_shared->sections.hdr_ofs = h;
	coff_shared->v_image = NULL;
	coff_shared->v_header = NULL;
	coff_shared->endian = end;

	/* headers */
	file->seek(h);
	file->readx(&coff_shared->coffheader, sizeof coff_shared->coffheader);
	createHostStruct(&coff_shared->coffheader, COFF_HEADER_struct, end);
	coff_shared->opt_magic = 0;
	if (coff_shared->coffheader.optional_header_size >= 2) {
		file->readx(&coff_shared->opt_magic, sizeof coff_shared->opt_magic);
		file->seek(h + sizeof coff_shared->coffheader);
		coff_shared->opt_magic = createHostInt(&coff_shared->opt_magic, 2, end);
		switch (coff_shared->opt_magic) {
			case COFF_OPTMAGIC_COFF32:
				file->readx(&coff_shared->coff32header, sizeof coff_shared->coff32header);
				createHostStruct(&coff_shared->coff32header, COFF_OPTIONAL_HEADER32_struct, end);
				break;
		}
	}

	/* read section headers */
	int os = coff_shared->coffheader.optional_header_size;
	coff_shared->sections.section_count = coff_shared->coffheader.section_count;

	h -= 4;

	file->seek(h+os+24);
	if (coff_shared->sections.section_count) {
		coff_shared->sections.sections = ht_malloc(coff_shared->sections.section_count * sizeof *coff_shared->sections.sections);
		file->read(coff_shared->sections.sections, coff_shared->sections.section_count*sizeof *coff_shared->sections.sections);
		for (uint i=0; i<coff_shared->sections.section_count; i++) {
			createHostStruct(&coff_shared->sections.sections[i], COFF_SECTION_HEADER_struct, end);
		}
	} /* CHECK - sufficient */
	shared_data = coff_shared;

	ht_format_group::init_ifs(ifs);
}

void ht_coff::done()
{
	ht_format_group::done();
	free(shared_data);
}

/*
 *	rva conversion routines
 */

bool coff_rva_to_section(coff_section_headers *section_headers, RVA rva, int *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((rva >= s->data_address) && (rva < s->data_address+s->data_size)) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}

bool coff_rva_to_ofs(coff_section_headers *section_headers, RVA rva, FileOfs *ofs)
{
	COFF_SECTION_HEADER *s = section_headers->sections;
	for (uint i = 0; i < section_headers->section_count; i++) {
		if (s->data_offset && rva >= s->data_address 
		&& rva < s->data_address+s->data_size) {
			*ofs = rva-s->data_address + s->data_offset + section_headers->hdr_ofs;
			return true;
		}
		s++;
	}
	return false;
}

bool coff_rva_is_valid(coff_section_headers *section_headers, RVA rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((rva >= s->data_address) && (rva < s->data_address+s->data_size)) {
			return true;
		}
		s++;
	}
	return false;
}

bool coff_rva_is_physical(coff_section_headers *section_headers, RVA rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if (s->data_offset && (rva >= s->data_address) &&
		(rva < s->data_address+s->data_size)) {
			return true;
		}
		s++;
	}
	return false;
}

/*
 *	ofs conversion routines
 */

bool coff_ofs_to_rva(coff_section_headers *section_headers, uint32 ofs, RVA *rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset+section_headers->hdr_ofs) &&
		(ofs<s->data_offset+section_headers->hdr_ofs+s->data_size)) {
			*rva=ofs-(s->data_offset+section_headers->hdr_ofs)+s->data_address;
			return true;
		}
		s++;
	}
	return false;
}

bool coff_ofs_to_section(coff_section_headers *section_headers, uint32 ofs, uint *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (uint i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset+section_headers->hdr_ofs) &&
		(ofs<s->data_offset+section_headers->hdr_ofs+s->data_size)) {
			*section=i;
			return true;
		}
		s++;
	}
	return false;
}

int coff_ofs_to_rva_and_section(coff_section_headers *section_headers, uint32 ofs, RVA *rva, uint *section)
{
	int r = coff_ofs_to_rva(section_headers, ofs, rva);
	if (r) {
		r = coff_ofs_to_section(section_headers, ofs, section);
	}
	return r;
}

bool coff_ofs_is_valid(coff_section_headers *section_headers, uint32 ofs)
{
	RVA rva;
	return coff_ofs_to_rva(section_headers, ofs, &rva);
}
