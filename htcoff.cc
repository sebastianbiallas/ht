/*
 *	HT Editor
 *	htcoff.cc
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "coff_analy.h"
#include "coff_s.h"
#include "htanaly.h"
#include "htapp.h"
#include "htcoff.h"
#include "htcoffhd.h"
#include "htcoffimg.h"
#include "htendian.h"
#include "mzstruct.h"
#include "stream.h"

#include <stdlib.h>
#include <string.h>

format_viewer_if *htcoff_ifs[] = {
	&htcoffheader_if,
	&htcoffimage_if,
	0
};

int is_coff(ht_streamfile *file, FILEOFS ofs)
{
	COFF_HEADER h;
	file->seek(ofs);
	if (file->read(&h, sizeof h)!=sizeof h) return 0;
	create_host_struct(&h, COFF_HEADER_struct, little_endian);
//     dword size=file->get_size();
/* test machine field */
	switch (h.machine) {
		case COFF_MACHINE_I386:		case COFF_MACHINE_I486:
		case COFF_MACHINE_I586:		case COFF_MACHINE_R3000BE:
		case COFF_MACHINE_R3000:		case COFF_MACHINE_R4000:
		case COFF_MACHINE_R10000:	case COFF_MACHINE_ALPHA:
		case COFF_MACHINE_SH3:		case COFF_MACHINE_SH4:
		case COFF_MACHINE_ARM:		case COFF_MACHINE_POWERPC:
		case COFF_MACHINE_IA64:		case COFF_MACHINE_MIPS16:
		case COFF_MACHINE_68k:		case COFF_MACHINE_ALPHA_AXP_64:
		case COFF_MACHINE_MIPSf:		case COFF_MACHINE_MIPS16f:
			break;
		default:
			return 0;
	}
/* test symbol_table_offset */
//	if ((h.symbol_table_offset>=size) || (h.symbol_table_offset<ofs+sizeof h)) return 0;
/* test size of optional header */
	if (h.optional_header_size>COFF_OPTHEADER_MAXSIZE) return 0;

/* all tests have completed successfully -> it's a COFF (probably) */
	return 1;
}

ht_view *htcoff_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	FILEOFS h;
/* look for pure coff */
	if (!is_coff(file, h=0)) {
/* look for dj-coff */
		byte mz[2];
		file->seek(0);
		file->read(mz, 2);
		if ((mz[0]!=IMAGE_MZ_MAGIC0) || (mz[1]!=IMAGE_MZ_MAGIC1) ||
				(!is_coff(file, h=0x800)))
				return 0;
	}

	ht_coff *g=new ht_coff();
	g->init(b, file, htcoff_ifs, format_group, h);
	return g;
}

format_viewer_if htcoff_if = {
	htcoff_init,
	0
};

/*
 *	CLASS ht_coff_aviewer
 */

void ht_coff_aviewer::init(bounds *b, char *desc, int caps, ht_streamfile *File, ht_format_group *format_group, analyser *Analyser, ht_coff_shared_data *Coff_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analyser);
	coff_shared = Coff_shared;
	file = File;
}

void ht_coff_aviewer::set_analyser(analyser *a)
{
	((coff_analyser *)a)->coff_shared = coff_shared;
	((coff_analyser *)a)->file = file;
	analy = a;
	analy_sub->set_analyser(a);
}

/*
 *	CLASS ht_coff
 */
void ht_coff::init(bounds *b, ht_streamfile *file, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS h)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_COFF, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_coff");

	LOG("%s: COFF: found header at %08x", file->get_filename(), h);
	coff_shared=new ht_coff_shared_data;
	coff_shared->hdr_ofs=h;
	coff_shared->v_image=NULL;
	coff_shared->v_header=NULL;

/* headers */
	file->seek(h);
	file->read(&coff_shared->coffheader, sizeof coff_shared->coffheader);
	create_host_struct(&coff_shared->coffheader, COFF_HEADER_struct, little_endian);
	coff_shared->opt_magic=0;
	if (coff_shared->coffheader.optional_header_size>=2) {
		file->read(&coff_shared->opt_magic, sizeof coff_shared->opt_magic);
		file->seek(h+sizeof coff_shared->coffheader);
		switch (coff_shared->opt_magic) {
			case COFF_OPTMAGIC_COFF32:
				file->read(&coff_shared->coff32header, sizeof coff_shared->coff32header);
				create_host_struct(&coff_shared->coff32header, COFF_OPTIONAL_HEADER32_struct, little_endian);
				break;
		}
	}

/* read section headers */
	int os=coff_shared->coffheader.optional_header_size;
	coff_shared->sections.section_count=coff_shared->coffheader.section_count;
	coff_shared->sections.base_ofs = 0x800;

	h-=4;
	
	file->seek(h+os+24);
	coff_shared->sections.sections=(COFF_SECTION_HEADER*)malloc(coff_shared->sections.section_count * sizeof *coff_shared->sections.sections);
	file->read(coff_shared->sections.sections, coff_shared->sections.section_count*sizeof *coff_shared->sections.sections);
	for (UINT i=0; i<coff_shared->sections.section_count; i++) {
		create_host_struct(&coff_shared->sections.sections[i], COFF_SECTION_HEADER_struct, little_endian);
	}

	shared_data=coff_shared;

	ht_format_group::init_ifs(ifs);
}

void ht_coff::done()
{
	ht_format_group::done();
}

/*
 *	rva conversion routines
 */

int coff_rva_to_section(coff_section_headers *section_headers, ADDR rva, int *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
// FIXME: what about data_vsize in COFFs ?
//		(rva<s->data_address+MAX(s->data_size, s->data_vsize))) {
		(rva<s->data_address+s->data_size)) {
			*section=i;
			return 1;
		}
		s++;
	}
	return 0;
}

int coff_rva_to_ofs(coff_section_headers *section_headers, ADDR rva, dword *ofs)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
		(rva<s->data_address+s->data_size)) {
			*ofs=rva-s->data_address+s->data_offset+section_headers->base_ofs;
			return 1;
		}
		s++;
	}
	return 0;
}

int coff_rva_is_valid(coff_section_headers *section_headers, ADDR rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
// FIXME: what about data_vsize in COFFs ?
//		(rva<s->data_address+MAX(s->data_size, s->data_vsize))) {
		(rva<s->data_address+s->data_size)) {
			return 1;
		}
		s++;
	}
	return 0;
}

int coff_rva_is_physical(coff_section_headers *section_headers, ADDR rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((rva>=s->data_address) &&
		(rva<s->data_address+s->data_size)) {
			return 1;
		}
		s++;
	}
	return 0;
}

/*
 *	ofs conversion routines
 */

int coff_ofs_to_rva(coff_section_headers *section_headers, dword ofs, ADDR *rva)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset+section_headers->base_ofs) &&
		(ofs<s->data_offset+section_headers->base_ofs+s->data_size)) {
			*rva=ofs-s->data_offset+section_headers->base_ofs+s->data_address;
			return 1;
		}
		s++;
	}
	return 0;
}

int coff_ofs_to_section(coff_section_headers *section_headers, dword ofs, ADDR *section)
{
	COFF_SECTION_HEADER *s=section_headers->sections;
	for (UINT i=0; i<section_headers->section_count; i++) {
		if ((ofs>=s->data_offset+section_headers->base_ofs) &&
		(ofs<s->data_offset+section_headers->base_ofs+s->data_size)) {
			*section=i;
			return 1;
		}
		s++;
	}
	return 0;
}

int coff_ofs_to_rva_and_section(coff_section_headers *section_headers, dword ofs, ADDR *rva, ADDR *section)
{
	int r=coff_ofs_to_rva(section_headers, ofs, rva);
	if (r) {
		r=coff_ofs_to_section(section_headers, ofs, section);
	}
	return r;
}

int coff_ofs_is_valid(coff_section_headers *section_headers, dword ofs)
{
	ADDR rva;
	return coff_ofs_to_rva(section_headers, ofs, &rva);
}

