/* 
 *	HT Editor
 *	htelf.cc
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

#include "elfstruc.h"
#include "log.h"
#include "htelf.h"
#include "htelfhd.h"
#include "htelfshs.h"
#include "htelfphs.h"
#include "htelfsym.h"
#include "htelfrel.h"
#include "htelfimg.h"
#include "endianess.h"
#include "except.h"
#include "htiobox.h"
#include "stream.h"
#include "tools.h"

#include "elfstruc.h"

#include <stdlib.h>

static format_viewer_if *htelf_ifs[] = {
	&htelfheader_if,
	&htelfsectionheaders_if,
	&htelfprogramheaders_if,
	&htelfimage_if,
	0
};

static ht_view *htelf_init(Bounds *b, File *file, ht_format_group *format_group)
{
	FileOfs header_ofs = 0;
	ELF_HEADER header;
	// read header
	try {
		file->seek(header_ofs);
		file->readx(&header, sizeof header);
	} catch (...) {
		return NULL;
	}
	// check for magic
	if ((header.e_ident[ELF_EI_MAG0]!=ELFMAG0) || (header.e_ident[ELF_EI_MAG1]!=ELFMAG1)
	||  (header.e_ident[ELF_EI_MAG2]!=ELFMAG2) || (header.e_ident[ELF_EI_MAG3]!=ELFMAG3))
		return NULL;
	switch (header.e_ident[ELF_EI_DATA]) {
		case ELFDATA2LSB:
		case ELFDATA2MSB:
			break;
		default:
			LOG_EX(LOG_WARN, "File seems to be ELF. But byte-order"
				" (ELF_EI_DATA) 0x%02x is unsupported. (byte at offset=0x%qx)",
				header.e_ident[ELF_EI_DATA], header_ofs+5);
			return NULL;
	}
	switch (header.e_ident[ELF_EI_CLASS]) {
		case ELFCLASS32:
		case ELFCLASS64:
			break;
		default:
			LOG_EX(LOG_WARN, "File seems to be ELF. But class-value"
				" (ELF_EI_CLASS) 0x%02x is unsupported. (byte at offset=0x%qx)",
				header.e_ident[ELF_EI_CLASS], header_ofs+4);
			return NULL;
	}

	try {
		ht_elf *g = new ht_elf();
		g->init(b, file, htelf_ifs, format_group, header_ofs);
		return g;
	} catch (const Exception &x) {
		errorbox("error while reading ELF: %y", &x);
		return NULL;
	}
}

format_viewer_if htelf_if = {
	htelf_init,
	0
};

/**/
int FakeAddr::compareTo(const Object *keyb) const
{
	FakeAddr *b = (FakeAddr*)keyb;
	if (secidx == b->secidx) return symidx - b->symidx;
	return secidx - b->secidx;
}

bool isValidELFSectionIdx(ht_elf_shared_data *elf_shared, int idx)
{
	return idx > 0 && (uint)idx < elf_shared->sheaders.count;
}

/*
 *	CLASS ht_elf
 */
void ht_elf::init(Bounds *b, File *f, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_ELF, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_elf");

	String fn;
	LOG("%y: ELF: found header at 0x%08qx", &file->getFilename(fn), header_ofs);

	ht_elf_shared_data *elf_shared = ht_malloc(sizeof(ht_elf_shared_data));

	shared_data = elf_shared;
	elf_shared->header_ofs = header_ofs;
	elf_shared->shnames = NULL;
	elf_shared->symtables = 0;
	elf_shared->reloctables = 0;
	elf_shared->v_image = NULL;
	elf_shared->shrelocs = NULL;
	elf_shared->fake_undefined_shidx = 0;
	elf_shared->undefined2fakeaddr = NULL;

	/* read header */
	file->seek(header_ofs);
	file->readx(&elf_shared->ident, sizeof elf_shared->ident);

	switch (elf_shared->ident.e_ident[ELF_EI_DATA]) {
	case ELFDATA2LSB:
		elf_shared->byte_order = little_endian;
		break;
	case ELFDATA2MSB:
		elf_shared->byte_order = big_endian;
		break;
	}

	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
	case ELFCLASS32: {
		file->readx(&elf_shared->header32, sizeof elf_shared->header32);
		createHostStruct(&elf_shared->header32, ELF_HEADER32_struct, elf_shared->byte_order);

		/* read section headers */
		elf_shared->sheaders.count = elf_shared->header32.e_shnum;
		if (!elf_shared->sheaders.count) {
			elf_shared->sheaders.sheaders32 = NULL;
		} else {
			elf_shared->sheaders.sheaders32 =
				ht_malloc(elf_shared->sheaders.count * sizeof *elf_shared->sheaders.sheaders32);
			file->seek(header_ofs + elf_shared->header32.e_shoff);
			file->readx(elf_shared->sheaders.sheaders32, elf_shared->sheaders.count*sizeof *elf_shared->sheaders.sheaders32);
			for (uint i=0; i < elf_shared->sheaders.count; i++) {
				ELF_SECTION_HEADER32 a = elf_shared->sheaders.sheaders32[i];
				createHostStruct(elf_shared->sheaders.sheaders32+i, ELF_SECTION_HEADER32_struct, elf_shared->byte_order);
			}
		}

		/* read program headers */
		elf_shared->pheaders.count = elf_shared->header32.e_phnum;
//		if (!elf_shared->pheaders.count) throw MsgException("Zero count in program section headers");
		if (!elf_shared->pheaders.count) {
			elf_shared->pheaders.pheaders32 = NULL;
		} else {
			elf_shared->pheaders.pheaders32 = ht_malloc(elf_shared->pheaders.count*sizeof *elf_shared->pheaders.pheaders32);
			file->seek(header_ofs + elf_shared->header32.e_phoff);
			file->readx(elf_shared->pheaders.pheaders32, elf_shared->pheaders.count*sizeof *elf_shared->pheaders.pheaders32);
			for (uint i=0; i < elf_shared->pheaders.count; i++) {
				createHostStruct(elf_shared->pheaders.pheaders32+i, ELF_PROGRAM_HEADER32_struct, elf_shared->byte_order);
			}
			// if file is relocatable, relocate it
			if (elf_shared->header32.e_type == ELF_ET_REL) {
				/* create a fake section for undefined symbols */
				try {
					fake_undefined_symbols32();
				} catch (const Exception &x) {
					errorbox("error while faking undefined ELF symbols: %y", &x);
				}

				/* create streamfile layer for relocations */
				try {
					auto_relocate32();
				} catch (const Exception &x) {
					errorbox("error while auto-relocating ELF symbols: %y", &x);
				}
			}
		}
		break;
	}
	case ELFCLASS64: {
		file->readx(&elf_shared->header64, sizeof elf_shared->header64);
		createHostStruct(&elf_shared->header64, ELF_HEADER64_struct, elf_shared->byte_order);

		/* read section headers */
		elf_shared->sheaders.count=elf_shared->header64.e_shnum;
		if (!elf_shared->sheaders.count) throw MsgException("Zero count for section headers");
		elf_shared->sheaders.sheaders64 = ht_malloc(elf_shared->sheaders.count*sizeof *elf_shared->sheaders.sheaders64);
		file->seek(header_ofs + elf_shared->header64.e_shoff);
		file->readx(elf_shared->sheaders.sheaders64, elf_shared->sheaders.count*sizeof *elf_shared->sheaders.sheaders64);
		for (uint i=0; i < elf_shared->sheaders.count; i++) {
			ELF_SECTION_HEADER64 a = elf_shared->sheaders.sheaders64[i];
			createHostStruct(elf_shared->sheaders.sheaders64+i, ELF_SECTION_HEADER64_struct, elf_shared->byte_order);
		}

		/* read program headers */
		elf_shared->pheaders.count = elf_shared->header64.e_phnum;
//		if (!elf_shared->pheaders.count) throw MsgException("Zero count in program section headers");
		if (!elf_shared->pheaders.count) {
			elf_shared->pheaders.pheaders64 = NULL;
		} else {
			elf_shared->pheaders.pheaders64 = ht_malloc(elf_shared->pheaders.count*sizeof *elf_shared->pheaders.pheaders64);
			/* FIXME: 64-bit */
			file->seek(header_ofs + elf_shared->header64.e_phoff);
			file->readx(elf_shared->pheaders.pheaders64, elf_shared->pheaders.count*sizeof *elf_shared->pheaders.pheaders64);

			for (uint i=0; i<elf_shared->pheaders.count; i++) {
				createHostStruct(elf_shared->pheaders.pheaders64+i, ELF_PROGRAM_HEADER64_struct, elf_shared->byte_order);
			}
			/* create a fake section for undefined symbols */
			//fake_undefined_symbols();

			/* create streamfile layer for relocations */
			//auto_relocate();
		}
		break;
		}
	}
	/* init ifs */
	ht_format_group::init_ifs(ifs);
	while (init_if(&htelfsymboltable_if)) elf_shared->symtables++;
	while (init_if(&htelfreloctable_if)) elf_shared->reloctables++;
}

void ht_elf::done()
{
	ht_format_group::done();
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)shared_data;
	if (elf_shared->shnames) {
		for (uint i=0; i < elf_shared->sheaders.count; i++)
			free(elf_shared->shnames[i]);
		free(elf_shared->shnames);
	}		
	free(elf_shared->shrelocs);
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
	case ELFCLASS32:
		free(elf_shared->sheaders.sheaders32);
		free(elf_shared->pheaders.pheaders32);
		break;
	case ELFCLASS64:
		free(elf_shared->sheaders.sheaders64);
		free(elf_shared->pheaders.pheaders64);
		break;
	}
	delete elf_shared->undefined2fakeaddr;
	free(elf_shared);
}

uint ht_elf::find_reloc_section_for(uint si)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)shared_data;

	ELF_SECTION_HEADER32 *s=elf_shared->sheaders.sheaders32;
	for (uint i=0; i < elf_shared->sheaders.count; i++) {
		if ((s->sh_type == ELF_SHT_REL || s->sh_type == ELF_SHT_RELA)
		 && s->sh_info == si) {
			return i;
		}
		s++;
	}
	return 0;
}

#define INVENT_BASE	0x100000
#define INVENT_STEPPING	0x100000
#define INVENT_LIMIT	0xffffffff

static elf32_addr elf32_invent_address(uint si, ELF_SECTION_HEADER32 *s, uint scount, elf32_addr base = INVENT_BASE)
{
	elf32_addr a = base;
	assert(s[si].sh_addr == 0);
	while (a < INVENT_LIMIT - s[si].sh_size) {
		bool ok = true;
		for (uint i=0; i < scount; i++) {
			if (a >= s[i].sh_addr
			 && a < s[i].sh_addr+s[i].sh_size) {
				ok = false;
				break;
			}
		}
		if (ok) return a;
		a += INVENT_STEPPING;
	}
	return 0;
}

void ht_elf::relocate_section(ht_reloc_file *f, uint si, uint rsi, elf32_addr a)
{
	// relocate section si (using section rsi) to address a
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)shared_data;
	ELF_SECTION_HEADER32 *s=elf_shared->sheaders.sheaders32;

	FileOfs relh = s[rsi].sh_offset;

	uint symtabidx = s[rsi].sh_link;
	if (!isValidELFSectionIdx(elf_shared, symtabidx)) throw MsgfException("invalid symbol table index %d", symtabidx);
	FileOfs symh = elf_shared->sheaders.sheaders32[symtabidx].sh_offset;

	if (s[rsi].sh_type != ELF_SHT_REL) throw MsgfException(
		"invalid section type for section %d (expecting %d)",
		rsi, ELF_SHT_REL);

	uint relnum = s[rsi].sh_size / sizeof (ELF_REL32);
	for (uint i=0; i < relnum; i++) {
		// read ELF_REL32
		ELF_REL32 r;
		file->seek(relh + i * sizeof r);
		file->readx(&r, sizeof r);
		createHostStruct(&r, ELF_REL32_struct, elf_shared->byte_order);

		// read ELF_SYMBOL32
		uint symbolidx = ELF32_R_SYM(r.r_info);
		ELF_SYMBOL32 sym;
		file->seek(symh+symbolidx*sizeof (ELF_SYMBOL32));
		file->read(&sym, sizeof sym);
		createHostStruct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);

		// calc reloc vals
		uint32 A = 0;
		uint32 P = r.r_offset+s[si].sh_addr;
		uint32 S;
		if ((sym.st_shndx > 0) && (sym.st_shndx < elf_shared->sheaders.count)) {
			S = sym.st_value + elf_shared->shrelocs[sym.st_shndx].relocAddr;
		} else if (elf_shared->fake_undefined_shidx >= 0) {
			FakeAddr s(symtabidx, symbolidx, 0);
			FakeAddr *fa = (FakeAddr *)
				elf_shared->undefined2fakeaddr->get(
				elf_shared->undefined2fakeaddr->find(&s));
			if (fa) {
				S = fa->addr;
			} else continue;
		} else {
			// skip this one
			// FIXME: nyi
			continue;
		}
		Object *z = new ht_elf32_reloc_entry(ELF32_R_TYPE(r.r_info), A, P, S);
		f->insert_reloc(r.r_offset+s[si].sh_offset, z);
	}
}

#define	FAKE_SECTION_BASEADDR	0x4acc0000
/* "resolve" undefined references by creating fake section and fake addresses */
void ht_elf::fake_undefined_symbols32()
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)shared_data;
	// create a fake section
	elf_shared->fake_undefined_shidx = elf_shared->sheaders.count;
	elf_shared->sheaders.count++;
	elf_shared->sheaders.sheaders32 = (ELF_SECTION_HEADER32*)
		realloc(elf_shared->sheaders.sheaders32, sizeof (ELF_SECTION_HEADER32)* elf_shared->sheaders.count);

	ELF_SECTION_HEADER32 *s = elf_shared->sheaders.sheaders32;
	s[elf_shared->fake_undefined_shidx].sh_name = 0;
	s[elf_shared->fake_undefined_shidx].sh_type = ELF_SHT_NOBITS;
	s[elf_shared->fake_undefined_shidx].sh_flags = ELF_SHF_WRITE | ELF_SHF_ALLOC;
	s[elf_shared->fake_undefined_shidx].sh_addr = 0;
	s[elf_shared->fake_undefined_shidx].sh_offset = 0;
	s[elf_shared->fake_undefined_shidx].sh_size = 0;	// filled in below
	s[elf_shared->fake_undefined_shidx].sh_link = 0;
	s[elf_shared->fake_undefined_shidx].sh_info = 0;
	s[elf_shared->fake_undefined_shidx].sh_addralign = 0;
	s[elf_shared->fake_undefined_shidx].sh_entsize = 0;
	elf32_addr a = elf32_invent_address(elf_shared->fake_undefined_shidx,
		s, elf_shared->sheaders.count, FAKE_SECTION_BASEADDR);
	s[elf_shared->fake_undefined_shidx].sh_addr = a;
	LOG("fake section %d", elf_shared->fake_undefined_shidx);

	// allocate fake addresses
	elf_shared->undefined2fakeaddr = new AVLTree(true);
	uint32 nextFakeAddr = FAKE_SECTION_BASEADDR;
	for (uint secidx = 0; secidx < elf_shared->sheaders.count; secidx++) {
		if (elf_shared->sheaders.sheaders32[secidx].sh_type == ELF_SHT_SYMTAB) {
			FileOfs symh = elf_shared->sheaders.sheaders32[secidx].sh_offset;
			uint symnum = elf_shared->sheaders.sheaders32[secidx].sh_size / sizeof (ELF_SYMBOL32);
			for (uint symidx = 1; symidx < symnum; symidx++) {
				ELF_SYMBOL32 sym;
				file->seek(symh+symidx*sizeof (ELF_SYMBOL32));
				file->read(&sym, sizeof sym);
				createHostStruct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);
				if (sym.st_shndx == ELF_SHN_UNDEF) {
					elf_shared->undefined2fakeaddr->insert(
						new FakeAddr(secidx, symidx, nextFakeAddr)
					);
					nextFakeAddr += 4;
				}
			}
		}
	}
	elf_shared->fake_undefined_size = nextFakeAddr-FAKE_SECTION_BASEADDR;
	s[elf_shared->fake_undefined_shidx].sh_size = elf_shared->fake_undefined_size;
}

void ht_elf::auto_relocate32()
{
	ht_elf32_reloc_file *rf = new ht_elf32_reloc_file(file, false, (ht_elf_shared_data*)shared_data);

	bool reloc_needed = false;

	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)shared_data;

	ELF_SECTION_HEADER32 *s=elf_shared->sheaders.sheaders32;
	if (!elf_shared->sheaders.count) {
		String fn;
		LOG("%y: ELF: segment header count is zero", &file->getFilename(fn));
	} else {
		elf_shared->shrelocs = ht_malloc(elf_shared->sheaders.count * sizeof (ht_elf_reloc_section32));
	}

	/* relocate sections */
	for (uint i=0; i < elf_shared->sheaders.count; i++) {
		elf_shared->shrelocs[i].relocAddr = 0;
		if ((s[i].sh_type == ELF_SHT_PROGBITS) && (s[i].sh_addr == 0)) {
			uint j = find_reloc_section_for(i);
			if (j) {
				elf32_addr a = elf32_invent_address(i, s, elf_shared->sheaders.count);
				if (a) {
					reloc_needed = true;
					// update section header entry
					s[i].sh_addr = a;
					elf_shared->shrelocs[i].relocAddr = a;
					elf_shared->shrelocs[i].relocShIdx = j;
				}
			}
		}
	}

	/* apply relocations to section descriptors */
	for (uint i=0; i < elf_shared->sheaders.count; i++) {
		if (elf_shared->shrelocs[i].relocAddr) {
			//LOG("section %d to %08x", i, elf_shared->shrelocs[i].relocAddr);
			try {
				relocate_section(rf, i, elf_shared->shrelocs[i].relocShIdx, elf_shared->shrelocs[i].relocAddr);
			} catch (const Exception &x) {
				LOG("error while relocating section %d: %y", i, &x);
			}
		}
	}

	if (reloc_needed) {
		rf->finalize();
		own_file = true;
		file = rf;
		String fn;
		LOG("%y: ELF: relocation layer enabled (invented relocation addresses)", &file->getFilename(fn));
	} else {
		free(elf_shared->shrelocs);
		elf_shared->shrelocs = NULL;
		delete rf;
	}
}

bool ht_elf::loc_enum_next(ht_format_loc *loc)
{
	ht_elf_shared_data *sh = (ht_elf_shared_data*)shared_data;
	if (loc_enum) {
		loc->name = "elf";
		loc->start = sh->header_ofs;
		loc->length = file->getSize() - loc->start;	/* FIXME: ENOTOK */

		loc_enum = false;
		return true;
	}
	return false;
}

void ht_elf::loc_enum_start()
{
	loc_enum = true;
}

/*
 *	address conversion routines
 */
bool elf_phys_and_mem_section(elf_section_header *sh, uint elfclass)
{
	switch (elfclass) {
	case ELFCLASS32: {
		ELF_SECTION_HEADER32 *s = &sh->sheaders32;
		return s->sh_type == ELF_SHT_PROGBITS && s->sh_addr;
	}
	case ELFCLASS64: {
		ELF_SECTION_HEADER64 *s = &sh->sheaders64;
		return s->sh_type==ELF_SHT_PROGBITS && s->sh_addr;
	}
	}
	return false;
}

bool elf_valid_section(elf_section_header *sh, uint elfclass)
{
	switch (elfclass) {
	case ELFCLASS32: {
		ELF_SECTION_HEADER32 *s = &sh->sheaders32;
		return (s->sh_type==ELF_SHT_PROGBITS || s->sh_type==ELF_SHT_NOBITS) && s->sh_addr;
	}
	case ELFCLASS64: {
		ELF_SECTION_HEADER64 *s = &sh->sheaders64;
		return (s->sh_type==ELF_SHT_PROGBITS || s->sh_type==ELF_SHT_NOBITS) && s->sh_addr;
	}
	}
	return false;
}

bool elf_addr_to_ofs(elf_section_headers *section_headers, uint elfclass, ELFAddress addr, FileOfs *ofs)
{
	switch (elfclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = section_headers->sheaders32;
			for (uint i=0; i < section_headers->count; i++) {
				if ((elf_phys_and_mem_section((elf_section_header*)s, elfclass)) && (addr.a32 >= s->sh_addr) && (addr.a32 < s->sh_addr+s->sh_size)) {
					*ofs = addr.a32 - s->sh_addr + s->sh_offset;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = section_headers->sheaders64;
			for (uint i=0; i < section_headers->count; i++) {
				if ((elf_phys_and_mem_section((elf_section_header*)s, elfclass)) && addr.a64 >= s->sh_addr && (addr.a64 < s->sh_addr + s->sh_size)) {
					*ofs = addr.a64 - s->sh_addr + s->sh_offset;
					return true;
				}
				s++;
			}
			break;
		}
	}
	return false;
}

bool elf_addr_to_section(elf_section_headers *section_headers, uint elfclass, ELFAddress addr, int *section)
{
	switch (elfclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = section_headers->sheaders32;
			for (uint i = 0; i < section_headers->count; i++) {
				if ((elf_valid_section((elf_section_header*)s, elfclass)) && (addr.a32 >= s->sh_addr) && (addr.a32 < s->sh_addr + s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = section_headers->sheaders64;
			for (uint i = 0; i < section_headers->count; i++) {
				if ((elf_valid_section((elf_section_header*)s, elfclass)) && addr.a64 >= s->sh_addr && (addr.a64 < s->sh_addr + s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
	}
	return false;
}

bool elf_addr_is_valid(elf_section_headers *section_headers, uint elfclass, ELFAddress addr)
{
	switch (elfclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = section_headers->sheaders32;
			for (uint i=0; i<section_headers->count; i++) {
				if ((elf_valid_section((elf_section_header*)s, elfclass)) && (addr.a32 >= s->sh_addr) && (addr.a32 < s->sh_addr + s->sh_size)) {
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = section_headers->sheaders64;
			for (uint i=0; i<section_headers->count; i++) {
				if ((elf_valid_section((elf_section_header*)s, elfclass)) && addr.a64  >= s->sh_addr && (addr.a64 < s->sh_addr + s->sh_size)) {
					return true;
				}
				s++;
			}
			break;
		}
	}
	return false;
}

bool elf_addr_is_physical(elf_section_headers *section_headers, uint elfclass, ELFAddress addr)
{
	return false;
}


/*
 *	offset conversion routines
 */

bool elf_ofs_to_addr(elf_section_headers *section_headers, uint elfclass, FileOfs ofs, ELFAddress *addr)
{
	switch (elfclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = section_headers->sheaders32;
			for (uint i = 0; i < section_headers->count; i++) {
				if ((elf_phys_and_mem_section((elf_section_header*)s, elfclass)) && (ofs >= s->sh_offset) && (ofs<s->sh_offset+s->sh_size)) {
					addr->a32 = ofs - s->sh_offset + s->sh_addr;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = section_headers->sheaders64;
			uint64 qofs = ofs;
			for (uint i = 0; i < section_headers->count; i++) {
				if ((elf_phys_and_mem_section((elf_section_header*)s, elfclass)) && qofs >= s->sh_offset && (qofs < s->sh_offset + s->sh_size)) {
					addr->a64 = qofs - s->sh_offset + s->sh_addr;
					return true;
				}
				s++;
			}
			break;
		}
	}
	return false;
}

bool elf_ofs_to_section(elf_section_headers *section_headers, uint elfclass, uint32 ofs, int *section)
{
	switch (elfclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s=section_headers->sheaders32;
			for (uint i=0; i<section_headers->count; i++) {
				if ((elf_valid_section((elf_section_header*)s, elfclass)) && (ofs >= s->sh_offset) && (ofs<s->sh_offset+s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = section_headers->sheaders64;
			uint64 qofs = ofs;
			for (uint i=0; i < section_headers->count; i++) {
				if ((elf_valid_section((elf_section_header*)s, elfclass)) && qofs >= s->sh_offset && (qofs < s->sh_offset + s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
	}
	return false;
}

bool elf_ofs_to_addr_and_section(elf_section_headers *section_headers, uint elfclass, uint32 ofs, ELFAddress *addr, int *section)
{
	return false;
}

/*
 *	ht_elf32_reloc_entry
 */
//ht_elf32_reloc_entry::ht_elf32_reloc_entry(uint symtabidx, elf32_addr addr, uint t, uint symbolidx, elf32_addr addend, ht_elf_shared_data *data, File *file)
ht_elf32_reloc_entry::ht_elf32_reloc_entry(uint t, uint32 A, uint32 P, uint32 S)
{
	type = t;
	switch (type) {
	case ELF_R_386_32:
		relocs.r_32 = S+A;
		break;
	case ELF_R_386_PC32:
		relocs.r_pc32 = S+A-P;
		break;
	}
}

/*
 *	ht_elf32_reloc_file
 */

ht_elf32_reloc_file::ht_elf32_reloc_file(File *s, bool os, ht_elf_shared_data *d)
	: ht_reloc_file(s, os), data(d)
{
}

void	ht_elf32_reloc_file::reloc_apply(Object *reloc, byte *buf)
{
	ht_elf32_reloc_entry *e=(ht_elf32_reloc_entry*)reloc;

	switch (e->type) {
	case ELF_R_386_32: {
		uint32 v = createHostInt(buf, 4, data->byte_order);
		v += e->relocs.r_32;
		createForeignInt(buf, v, 4, data->byte_order);
		break;
	}
	case ELF_R_386_PC32: {
		uint32 v = createHostInt(buf, 4, data->byte_order);
		v += e->relocs.r_pc32;
		createForeignInt(buf, v, 4, data->byte_order);
		break;
	}
	}
}

bool	ht_elf32_reloc_file::reloc_unapply(Object *reloc, byte *data)
{
	return false;
//	ht_elf32_reloc_entry *e=(ht_elf32_reloc_entry*)reloc;
}
