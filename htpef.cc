/* 
 *	HT Editor
 *	htpef.cc
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

#include "pefstruc.h"
#include "log.h"
#include "htpef.h"
#include "htpefhd.h"
#include "htpefimp.h"
#include "htpefimg.h"
#include "endianess.h"
#include "stream.h"
#include "tools.h"

#include <stdlib.h>

format_viewer_if *htpef_ifs[] = {
	&htpefheader_if,
	&htpefimports_if,
	&htpefimage_if,
	0
};

static ht_view *htpef_init(Bounds *b, File *file, ht_format_group *format_group)
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
void ht_pef::init(Bounds *b, File *f, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_PEF, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_pef");

	String fn;
	LOG("%y: PEF: found header at %08qx", &file->getFilename(fn), header_ofs);

	ht_pef_shared_data *pef_shared = ht_malloc(sizeof(ht_pef_shared_data));
	memset(pef_shared, 0, sizeof *pef_shared);
	
	shared_data = pef_shared;
	/* always big-endian */
	pef_shared->byte_order = big_endian;
	pef_shared->header_ofs = 0;

	pef_shared->imports.funcs = new Array(true);
	pef_shared->imports.funcs->init();

	pef_shared->imports.libs = new Array(true);

	pef_shared->v_imports = NULL;
	
	/* read (container) header */
	file->seek(header_ofs);
	file->read(&pef_shared->contHeader, sizeof pef_shared->contHeader);
	createHostStruct(&pef_shared->contHeader, PEF_CONTAINER_HEADER_struct, pef_shared->byte_order);

	if (memcmp(pef_shared->contHeader.architecture, "pwpc", 4) == 0) {
		pef_shared->arch = PEFARCH_PowerPC;
	} else if (memcmp(pef_shared->contHeader.architecture, "m68k", 4) == 0) {
		pef_shared->arch = PEFARCH_M68K;
	} else return;

	/* read section headers */
	pef_shared->sheaders.count = pef_shared->contHeader.sectionCount;
	if (pef_shared->sheaders.count) {
		pef_shared->sheaders.sheaders =
			ht_malloc(pef_shared->sheaders.count*sizeof (PEF_SECTION_HEADER));
		for (uint i=0; i<pef_shared->sheaders.count; i++) {
			file->read(&pef_shared->sheaders.sheaders[i], sizeof pef_shared->sheaders.sheaders[i]);
			createHostStruct(&pef_shared->sheaders.sheaders[i], PEF_SECTION_HEADER_struct, pef_shared->byte_order);
			// FIXME: hack
			pef_shared->sheaders.sheaders[i].defaultAddress = i*0x100000;
			if (!pef_shared->loader_info_header_ofs
			&& pef_shared->sheaders.sheaders[i].sectionKind == PEF_SK_Loader) {
				pef_shared->loader_info_header_ofs = pef_shared->sheaders.sheaders[i].containerOffset;
			}
		}
	}

	if (pef_shared->loader_info_header_ofs) {
		file->seek(pef_shared->loader_info_header_ofs);
		file->read(&pef_shared->loader_info_header, sizeof pef_shared->loader_info_header);
		createHostStruct(&pef_shared->loader_info_header, PEF_LOADER_INFO_HEADER_struct, pef_shared->byte_order);
	}

	/* init ifs */
	ht_format_group::init_ifs(ifs);
}

void ht_pef::done()
{
	ht_format_group::done();
	
	ht_pef_shared_data *pef_shared = (ht_pef_shared_data*)shared_data;
	delete pef_shared->imports.funcs;
	delete pef_shared->imports.libs;
	free(shared_data);
}

#define RELOC_BASE		0x10000000
#define RELOC_STEPPING	0x100000
#define RELOC_LIMIT		0xffffffff
			
/*static uint32 pef_invent_reloc_address(uint si, PEF_SECTION_HEADER *s, uint scount)
{
	elf32_addr a=RELOC_BASE;
	while (a<RELOC_LIMIT-s[si].sh_size) {
		bool ok=true;
		for (uint i=0; i<scount; i++) {
			if ((s[i].sh_type==ELF_SHT_PROGBITS) && (s[i].sh_addr) &&
			((a>=s[i].sh_addr) && (a<s[i].sh_addr+s[i].sh_size))) {
				ok=false;
				break;
			}
		}
		if (ok) return a;
		a+=RELOC_STEPPING;
	}
	return 0;
}*/

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

bool pef_addr_to_ofs(pef_section_headers *section_headers, PEFAddress addr, uint32 *ofs)
{
	PEF_SECTION_HEADER *s = section_headers->sheaders;
	for (uint i=0; i < section_headers->count; i++) {
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
	for (uint i = 0; i < section_headers->count; i++) {
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
	for (uint i=0; i<section_headers->count; i++) {
		if ((pef_valid_section(s)) &&
		(addr.a32 >= s->defaultAddress) && (addr.a32 < s->defaultAddress + s->totalSize)) {
			return true;
		}
		s++;
	}
	return false;
}

/*
 *	offset conversion routines
 */

bool pef_ofs_to_addr(pef_section_headers *section_headers, uint32 ofs, PEFAddress *addr)
{
	PEF_SECTION_HEADER *s = section_headers->sheaders;
	for (uint i = 0; i < section_headers->count; i++) {
		if ((pef_phys_and_mem_section(s)) &&
		(ofs>=s->containerOffset) && (ofs<s->containerOffset+s->packedSize)) {
			addr->a32 = ofs - s->containerOffset + s->defaultAddress;
			return true;
		}
		s++;
	}
	return false;
}

bool pef_ofs_to_section(pef_section_headers *section_headers, uint32 ofs, int *section)
{
	PEF_SECTION_HEADER *s=section_headers->sheaders;
	for (uint i=0; i<section_headers->count; i++) {
		if ((pef_valid_section(s)) &&
		(ofs >= s->containerOffset) && (ofs<s->containerOffset+s->packedSize)) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}

/*
 *	ht_pef_reloc_entry
 */
#include "relfile.h"
class ht_pef_reloc_entry: public Object {
public:
	
//	ht_elf32_reloc_entry(uint symtabidx, elf32_addr offset, uint type, uint symbolidx, elf32_addr addend, ht_elf_shared_data *data, File *file);
};

/*
 *	ht_pef_reloc_file
 */

class ht_pef_reloc_file: public ht_reloc_file {
protected:
	ht_pef_shared_data *data;
/* overwritten */
	virtual void	reloc_apply(Object *reloc, byte *data);
	virtual bool	reloc_unapply(Object *reloc, byte *data);
public:

ht_pef_reloc_file(File *s, bool own_s, ht_pef_shared_data *d)
	:ht_reloc_file(s, own_s)
{
	data = d;
}

};
