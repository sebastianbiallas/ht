/* 
 *	HT Editor
 *	htpef.cc
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

#include "pefstruc.h"
#include "log.h"
#include "htpef.h"
#include "htpefhd.h"
#include "htpefimg.h"
#include "htendian.h"
#include "stream.h"
#include "tools.h"

#include <stdlib.h>

format_viewer_if *htpef_ifs[] = {
	&htpefheader_if,
	&htpefimage_if,
	0
};

ht_view *htpef_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	byte id[12];
	file->seek(0);
	file->read(&id, sizeof id);
	if (memcmp(id, "Joy!peff", 8)) return NULL;
	PEF_ARCH arch;
	if (memcmp(id+8, "pwpc", 4) == 0) {
		arch = PEFARCH_PowerPC;
	} else if (memcmp(id+8, "m68k", 4) == 0) {
		arch = PEFARCH_M68K;
	} else return NULL;

	ht_pef *g = new ht_pef();
	g->init(b, file, htpef_ifs, format_group, 0);
	return g;
}

format_viewer_if htpef_if = {
	htpef_init,
	0
};

/*
 *	CLASS ht_pef
 */
void ht_pef::init(bounds *b, ht_streamfile *f, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS header_ofs)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_PEF, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_pef");

	LOG("%s: pef: found header at %08x", file->get_filename(), header_ofs);

	ht_pef_shared_data *pef_shared=(ht_pef_shared_data *)malloc(sizeof(ht_pef_shared_data));

	shared_data = pef_shared;
	/* always big-endian */
	pef_shared->byte_order = big_endian;
	pef_shared->header_ofs = 0;

	/* read (container) header */
	file->seek(header_ofs);
	file->read(&pef_shared->contHeader, sizeof pef_shared->contHeader);
	create_host_struct(&pef_shared->contHeader, PEF_CONTAINER_HEADER_struct, pef_shared->byte_order);

	if (memcmp(pef_shared->contHeader.architecture, "pwpc", 4) == 0) {
		pef_shared->arch = PEFARCH_PowerPC;
	} else if (memcmp(pef_shared->contHeader.architecture, "m68k", 4) == 0) {
		pef_shared->arch = PEFARCH_M68K;
	} else return;

	/* read section headers */
	pef_shared->sheaders.count = pef_shared->contHeader.sectionCount;
	pef_shared->sheaders.sheaders = (PEF_SECTION_HEADER*)
		malloc(pef_shared->sheaders.count*sizeof (PEF_SECTION_HEADER));
	for (uint i=0; i<pef_shared->sheaders.count; i++) {
		file->read(&pef_shared->sheaders.sheaders[i], sizeof pef_shared->sheaders.sheaders[i]);
		create_host_struct(&pef_shared->sheaders.sheaders[i], PEF_SECTION_HEADER_struct, pef_shared->byte_order);
		// FIXME: hack
		pef_shared->sheaders.sheaders[i].defaultAddress = i*0x100000;
	}

	/* init ifs */
	ht_format_group::init_ifs(ifs);
}

void ht_pef::done()
{
	ht_format_group::done();
}

/*
 *	address conversion routines
 */

bool pef_phys_and_mem_section(PEF_SECTION_HEADER *s)
{
	return (s->sectionKind <= 1) || (s->sectionKind == 3) || (s->sectionKind == 6);
}

bool pef_valid_section(PEF_SECTION_HEADER *s)
{
	return (s->sectionKind <= 3) || (s->sectionKind == 6);
}

bool pef_addr_to_ofs(pef_section_headers *section_headers, PEFAddress addr, dword *ofs)
{
	PEF_SECTION_HEADER *s = section_headers->sheaders;
	for (UINT i=0; i < section_headers->count; i++) {
		if ((pef_phys_and_mem_section(s)) &&
		(addr.a32 >= s->defaultAddress) && (addr.a32 < s->defaultAddress+s->packedSize)) {
			*ofs = addr.a32 - s->defaultAddress + s->containerOffset;
			return true;
		}
		s++;
	}
	return false;
}

bool pef_addr_to_section(pef_section_headers *section_headers, PEFAddress addr, int *section)
{
	PEF_SECTION_HEADER *s = section_headers->sheaders;
	for (UINT i = 0; i < section_headers->count; i++) {
		if ((pef_valid_section(s)) &&
		(addr.a32 >= s->defaultAddress) && (addr.a32 < s->defaultAddress + s->totalSize)) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}

bool pef_addr_is_valid(pef_section_headers *section_headers, PEFAddress addr)
{
	PEF_SECTION_HEADER *s = section_headers->sheaders;
	for (UINT i=0; i<section_headers->count; i++) {
		if ((pef_valid_section(s)) &&
		(addr.a32 >= s->defaultAddress) && (addr.a32 < s->defaultAddress + s->totalSize)) {
			return true;
		}
		s++;
	}
	return false;
}

/*bool pef_addr_is_physical(pef_section_headers *section_headers, PEFAddress addr)
{
	return false;
}*/


/*
 *	offset conversion routines
 */

bool pef_ofs_to_addr(pef_section_headers *section_headers, dword ofs, PEFAddress *addr)
{
	PEF_SECTION_HEADER *s = section_headers->sheaders;
	for (UINT i = 0; i < section_headers->count; i++) {
		if ((pef_phys_and_mem_section(s)) &&
		(ofs>=s->containerOffset) && (ofs<s->containerOffset+s->packedSize)) {
			addr->a32 = ofs - s->containerOffset + s->defaultAddress;
			return true;
		}
		s++;
	}
	return false;
}

bool pef_ofs_to_section(pef_section_headers *section_headers, dword ofs, int *section)
{
	PEF_SECTION_HEADER *s=section_headers->sheaders;
	for (UINT i=0; i<section_headers->count; i++) {
		if ((pef_valid_section(s)) &&
		(ofs >= s->containerOffset) && (ofs<s->containerOffset+s->packedSize)) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}

/*bool pef_ofs_to_addr_and_section(pef_section_headers *section_headers, dword ofs,
	PEFAddress *addr, int *section)
{
	return false;
}*/
