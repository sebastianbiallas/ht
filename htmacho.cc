/* 
 *	HT Editor
 *	htmacho.cc
 *
 *	Copyright (C) 2003 Stefan Weyergraf (stefan@weyergraf.de)
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
#include "htmacho.h"
#include "htmachohd.h"
#include "htmachoimg.h"
#include "htendian.h"
#include "stream.h"
#include "tools.h"

#include "machostruc.h"

#include <stdlib.h>

format_viewer_if *htmacho_ifs[] = {
	&htmachoheader_if,
	&htmachoimage_if,
	0
};

ht_view *htmacho_init(bounds *b, ht_streamfile *file, ht_format_group *format_group)
{
	byte magic[4];
	file->seek(0);
	file->read(&magic, sizeof magic);
	if (memcmp(magic, "\xfe\xed\xfa\xce", 4) != 0) return NULL;
		
	ht_macho *g = new ht_macho();
	g->init(b, file, htmacho_ifs, format_group, 0);
	return g;
}

format_viewer_if htmacho_if = {
	htmacho_init,
	0
};

/*
 *	CLASS ht_macho
 */
void ht_macho::init(bounds *b, ht_streamfile *f, format_viewer_if **ifs, ht_format_group *format_group, FILEOFS header_ofs)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_MACHO, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_macho");

	LOG("%s: Mach-O: found header at %08x", file->get_filename(), header_ofs);
	
	ht_macho_shared_data *macho_shared=(ht_macho_shared_data *)malloc(sizeof(ht_macho_shared_data));

	shared_data = macho_shared;
	macho_shared->header_ofs = header_ofs;
	macho_shared->cmds.count = 0;
	macho_shared->cmds.cmds = NULL;
/*	macho_shared->shnames=NULL;
	macho_shared->symtables=0;
	macho_shared->reloctables=0;
	macho_shared->v_image=NULL;
	macho_shared->htrelocs=NULL;
	macho_shared->fake_undefined_section=0;*/

	/* read header */
	file->seek(header_ofs);
	file->read(&macho_shared->header, sizeof macho_shared->header);
	create_host_struct(&macho_shared->header, MACHO_HEADER_struct, big_endian);

	/* read commands */
	FILEOFS ofs = file->tell();
	macho_shared->cmds.count = macho_shared->header.ncmds;
	macho_shared->cmds.cmds = (MACHO_COMMAND_U**)malloc(sizeof (MACHO_COMMAND_U*) * macho_shared->header.ncmds);
	for (uint i=0; i<macho_shared->cmds.count; i++) {
		MACHO_COMMAND cmd;
		file->seek(ofs);
		file->read(&cmd, sizeof cmd);
		create_host_struct(&cmd, MACHO_COMMAND_struct, big_endian);
		// FIXME: improve this logic
		assert(cmd.cmdsize<=1024);
		macho_shared->cmds.cmds[i] = (MACHO_COMMAND_U*)malloc(cmd.cmdsize);
		file->seek(ofs);
		file->read(macho_shared->cmds.cmds[i], cmd.cmdsize);
		switch (cmd.cmd) {
			case LC_SEGMENT:
				create_host_struct(macho_shared->cmds.cmds[i], MACHO_SEGMENT_COMMAND_struct, big_endian);
				break;
			case LC_THREAD:
			case LC_UNIXTHREAD: {
				MACHO_THREAD_COMMAND *c = (MACHO_THREAD_COMMAND*)macho_shared->cmds.cmds[i];
				create_host_struct(macho_shared->cmds.cmds[i], MACHO_THREAD_COMMAND_struct, big_endian);
				switch (c->flavor) {
					case FLAVOR_PPC_THREAD_STATE:
						create_host_struct(&c->state, MACHO_PPC_THREAD_STATE_struct, big_endian);
						break;
				}
				break;
			}
			default:
				create_host_struct(macho_shared->cmds.cmds[i], MACHO_COMMAND_struct, big_endian);
		}
		ofs += cmd.cmdsize;
	}
#if 0
	switch (macho_shared->ident.e_ident[ELF_EI_DATA]) {
		case ELFDATA2LSB:
			macho_shared->byte_order = little_endian;
			break;
		case ELFDATA2MSB:
			macho_shared->byte_order = big_endian;
			break;
	}

	switch (macho_shared->ident.e_ident[ELF_EI_CLASS]) {
		case ELFCLASS32: {
			file->read(&macho_shared->header32, sizeof macho_shared->header32);
			create_host_struct(&macho_shared->header32, ELF_HEADER32_struct, macho_shared->byte_order);
			/* read section headers */
			macho_shared->sheaders.count=macho_shared->header32.e_shnum;
			macho_shared->sheaders.sheaders32=(ELF_SECTION_HEADER32*)malloc(macho_shared->sheaders.count*sizeof *macho_shared->sheaders.sheaders32);
			file->seek(header_ofs+macho_shared->header32.e_shoff);
			file->read(macho_shared->sheaders.sheaders32, macho_shared->sheaders.count*sizeof *macho_shared->sheaders.sheaders32);
			for (UINT i=0; i<macho_shared->sheaders.count; i++) {
				ELF_SECTION_HEADER32 a = macho_shared->sheaders.sheaders32[i];
				create_host_struct(macho_shared->sheaders.sheaders32+i, ELF_SECTION_HEADER32_struct, macho_shared->byte_order);
			}
	
			/* read program headers */
			macho_shared->pheaders.count=macho_shared->header32.e_phnum;
			macho_shared->pheaders.pheaders32=(ELF_PROGRAM_HEADER32*)malloc(macho_shared->pheaders.count*sizeof *macho_shared->pheaders.pheaders32);
			file->seek(header_ofs+macho_shared->header32.e_phoff);
			file->read(macho_shared->pheaders.pheaders32, macho_shared->pheaders.count*sizeof *macho_shared->pheaders.pheaders32);
			for (UINT i=0; i<macho_shared->pheaders.count; i++) {
				create_host_struct(macho_shared->pheaders.pheaders32+i, ELF_PROGRAM_HEADER32_struct, macho_shared->byte_order);
			}
			/* create a fake section for undefined symbols */
//			fake_undefined_symbols();

			/* create streamfile layer for relocations */
			auto_relocate();
			break;
		}
		case ELFCLASS64: {
			file->read(&macho_shared->header64, sizeof macho_shared->header64);
			create_host_struct(&macho_shared->header64, ELF_HEADER64_struct, macho_shared->byte_order);
			/* read section headers */
			macho_shared->sheaders.count=macho_shared->header64.e_shnum;
			macho_shared->sheaders.sheaders64=(ELF_SECTION_HEADER64*)malloc(macho_shared->sheaders.count*sizeof *macho_shared->sheaders.sheaders64);
/* FIXME: 64-bit */
			file->seek(header_ofs+macho_shared->header64.e_shoff.lo);
			file->read(macho_shared->sheaders.sheaders64, macho_shared->sheaders.count*sizeof *macho_shared->sheaders.sheaders64);
			for (UINT i=0; i<macho_shared->sheaders.count; i++) {
				ELF_SECTION_HEADER64 a = macho_shared->sheaders.sheaders64[i];
				create_host_struct(macho_shared->sheaders.sheaders64+i, ELF_SECTION_HEADER64_struct, macho_shared->byte_order);
			}

			/* read program headers */
			macho_shared->pheaders.count=macho_shared->header64.e_phnum;
			macho_shared->pheaders.pheaders64=(ELF_PROGRAM_HEADER64*)malloc(macho_shared->pheaders.count*sizeof *macho_shared->pheaders.pheaders64);
/* FIXME: 64-bit */
			file->seek(header_ofs+macho_shared->header64.e_phoff.lo);
			file->read(macho_shared->pheaders.pheaders64, macho_shared->pheaders.count*sizeof *macho_shared->pheaders.pheaders64);
			for (UINT i=0; i<macho_shared->pheaders.count; i++) {
				create_host_struct(macho_shared->pheaders.pheaders64+i, ELF_PROGRAM_HEADER64_struct, macho_shared->byte_order);
			}
			/* create a fake section for undefined symbols */
//			fake_undefined_symbols();

			/* create streamfile layer for relocations */
//			auto_relocate();
			break;
		}
	}
	while (init_if(&htelfsymboltable_if)) macho_shared->symtables++;
	while (init_if(&htelfreloctable_if)) macho_shared->reloctables++;
#endif
	/* init ifs */
	ht_format_group::init_ifs(ifs);
}

void ht_macho::done()
{
	ht_format_group::done();
/*	ht_macho_shared_data *macho_shared=(ht_macho_shared_data *)shared_data;
	if (macho_shared->shnames) {
		for (UINT i=0; i < macho_shared->sheaders.count; i++)
			free(macho_shared->shnames[i]);
		free(macho_shared->shnames);
	}		
	if (macho_shared->htrelocs) free(macho_shared->htrelocs);
	switch (macho_shared->ident.e_ident[ELF_EI_CLASS]) {
		case ELFCLASS32:
			if (macho_shared->sheaders.sheaders32) free(macho_shared->sheaders.sheaders32);
			if (macho_shared->pheaders.pheaders32) free(macho_shared->pheaders.pheaders32);
			break;
		case ELFCLASS64:
			if (macho_shared->sheaders.sheaders64) free(macho_shared->sheaders.sheaders64);
			if (macho_shared->pheaders.pheaders64) free(macho_shared->pheaders.pheaders64);
			break;
	}
	free(macho_shared);*/
}

/*
 *	address conversion routines
 */

bool macho_phys_and_mem_section(MACHO_SEGMENT_COMMAND *s, UINT machoclass)
{
	return (s->cmd == LC_SEGMENT);
}

bool macho_valid_section(MACHO_SEGMENT_COMMAND *s, UINT machoclass)
{
/*	switch (machoclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = (ELF_SECTION_HEADER32*)sh;
			return (((s->sh_type==ELF_SHT_PROGBITS) || (s->sh_type==ELF_SHT_NOBITS)) && (s->sh_addr!=0));
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = (ELF_SECTION_HEADER64*)sh;
			return (((s->sh_type==ELF_SHT_PROGBITS) || (s->sh_type==ELF_SHT_NOBITS)) && (s->sh_addr.lo!=0) && (s->sh_addr.hi!=0));
		}
	}*/
	return (s->cmd == LC_SEGMENT);
}

bool macho_addr_to_ofs(macho_commands *command_headers, UINT machoclass, MACHOAddress addr, dword *ofs)
{
/*	switch (machoclass) {
		case ELFCLASS32: {
			MACHO_COMMAND *s = command_headers->sheaders32;
			for (UINT i=0; i < command_headers->count; i++) {
				if ((macho_phys_and_mem_section((macho_section_header*)s, machoclass)) && (addr.a32 >= s->sh_addr) && (addr.a32 < s->sh_addr+s->sh_size)) {
					*ofs = addr.a32 - s->sh_addr + s->sh_offset;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = command_headers->sheaders64;
			for (UINT i=0; i < command_headers->count; i++) {
				if ((macho_phys_and_mem_section((macho_section_header*)s, machoclass)) && (qword_cmp(addr.a64, s->sh_addr) >= 0) && (addr.a64 < s->sh_addr + s->sh_size)) {
					qword qofs = addr.a64 - s->sh_addr + s->sh_offset;
					*ofs = qofs.lo;
					return true;
				}
				s++;
			}
			break;
		}
	}*/
	MACHO_COMMAND_U **pp = command_headers->cmds;
	for (UINT i=0; i < command_headers->count; i++) {
		if ((*pp)->cmd.cmd == LC_SEGMENT) {
			MACHO_SEGMENT_COMMAND *s = (MACHO_SEGMENT_COMMAND*)*pp;
			if (macho_phys_and_mem_section(s, machoclass) &&
			(addr >= s->vmaddr) && (addr < s->vmaddr+s->vmsize)) {
				*ofs = addr - s->vmaddr + s->fileoff;
				return true;
			}
		}
		pp++;
	}
	return false;
}


bool macho_addr_to_section(macho_commands *command_headers, UINT machoclass, MACHOAddress addr, int *section)
{
/*	switch (machoclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = command_headers->sheaders32;
			for (UINT i = 0; i < command_headers->count; i++) {
				if ((macho_valid_section((macho_section_header*)s, machoclass)) && (addr.a32 >= s->sh_addr) && (addr.a32 < s->sh_addr + s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = command_headers->sheaders64;
			for (UINT i = 0; i < command_headers->count; i++) {
				if ((macho_valid_section((macho_section_header*)s, machoclass)) && (qword_cmp(addr.a64, s->sh_addr) >= 0) && (addr.a64 < s->sh_addr + s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
	}*/
	MACHO_COMMAND_U **pp = command_headers->cmds;
	for (UINT i=0; i < command_headers->count; i++) {
		if ((*pp)->cmd.cmd == LC_SEGMENT) {
			MACHO_SEGMENT_COMMAND *s = (MACHO_SEGMENT_COMMAND*)*pp;
			if ((macho_valid_section(s, machoclass)) && (addr >= s->vmaddr) && (addr < s->vmaddr + s->vmsize)) {
				*section = i;
				return true;
			}
		}
		pp++;
	}
	return false;
}

bool macho_addr_is_valid(macho_commands *command_headers, UINT machoclass, MACHOAddress addr)
{
/*	switch (machoclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = command_headers->sheaders32;
			for (UINT i=0; i<command_headers->count; i++) {
				if ((macho_valid_section((macho_section_header*)s, machoclass)) && (addr.a32 >= s->sh_addr) && (addr.a32 < s->sh_addr + s->sh_size)) {
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = command_headers->sheaders64;
			for (UINT i=0; i<command_headers->count; i++) {
				if ((macho_valid_section((macho_section_header*)s, machoclass)) && (qword_cmp(addr.a64, s->sh_addr) >= 0) && (addr.a64 < s->sh_addr + s->sh_size)) {
					return true;
				}
				s++;
			}
			break;
		}
	}*/
	MACHO_COMMAND_U **pp = command_headers->cmds;
	for (UINT i=0; i < command_headers->count; i++) {
		if ((*pp)->cmd.cmd == LC_SEGMENT) {
			MACHO_SEGMENT_COMMAND *s = (MACHO_SEGMENT_COMMAND*)*pp;
			if ((macho_valid_section(s, machoclass)) && (addr >= s->vmaddr) && (addr < s->vmaddr + s->vmsize)) {
				return true;
			}
		}
		pp++;
	}
	return false;
}

/*bool macho_addr_is_physical(macho_command_headers *command_headers, UINT machoclass, ELFAddress addr)
{
	return false;
}*/

/*
 *	offset conversion routines
 */

bool macho_ofs_to_addr(macho_commands *command_headers, UINT machoclass, dword ofs, MACHOAddress *addr)
{
/*	switch (machoclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s = command_headers->sheaders32;
			for (UINT i = 0; i < command_headers->count; i++) {
				if ((macho_phys_and_mem_section((macho_section_header*)s, machoclass)) && (ofs>=s->sh_offset) && (ofs<s->sh_offset+s->sh_size)) {
					addr->a32 = ofs - s->sh_offset + s->sh_addr;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = command_headers->sheaders64;
			qword qofs = to_qword(ofs);
			for (UINT i = 0; i < command_headers->count; i++) {
				if ((macho_phys_and_mem_section((macho_section_header*)s, machoclass)) && (qword_cmp(qofs, s->sh_offset)>=0) && (qofs < s->sh_offset + s->sh_size)) {
					addr->a64 = qofs - s->sh_offset + s->sh_addr;
					return true;
				}
				s++;
			}
			break;
		}
	}*/
	MACHO_COMMAND_U **pp = command_headers->cmds;
	for (UINT i=0; i < command_headers->count; i++) {
		if ((*pp)->cmd.cmd == LC_SEGMENT) {
			MACHO_SEGMENT_COMMAND *s = (MACHO_SEGMENT_COMMAND*)*pp;
			if ((macho_phys_and_mem_section(s, machoclass)) && (ofs>=s->fileoff) && (ofs<s->fileoff+s->filesize)) {
				*addr = ofs - s->fileoff + s->vmaddr;
				return true;
			}
		}
		pp++;
	}
	return false;
}

bool macho_ofs_to_section(macho_commands *command_headers, UINT machoclass, dword ofs, dword *section)
{
/*	switch (machoclass) {
		case ELFCLASS32: {
			ELF_SECTION_HEADER32 *s=command_headers->sheaders32;
			for (UINT i=0; i<command_headers->count; i++) {
				if ((macho_valid_section((macho_section_header*)s, machoclass)) && (ofs >= s->sh_offset) && (ofs<s->sh_offset+s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
		case ELFCLASS64: {
			ELF_SECTION_HEADER64 *s = command_headers->sheaders64;
			qword qofs;
			qofs.hi = 0; qofs.lo = ofs;
			for (UINT i=0; i < command_headers->count; i++) {
				if ((macho_valid_section((macho_section_header*)s, machoclass)) && (qword_cmp(qofs, s->sh_offset)>=0) && (qofs < s->sh_offset + s->sh_size)) {
					*section = i;
					return true;
				}
				s++;
			}
			break;
		}
	}*/
	MACHO_COMMAND_U **pp = command_headers->cmds;
	for (UINT i=0; i < command_headers->count; i++) {
		if ((*pp)->cmd.cmd == LC_SEGMENT) {
			MACHO_SEGMENT_COMMAND *s = (MACHO_SEGMENT_COMMAND*)*pp;
			if ((macho_valid_section(s, machoclass)) && (ofs >= s->fileoff) && (ofs<s->fileoff+s->filesize)) {
				*section = i;
				return true;
			}
		}
		pp++;
	}
	return false;
}

/*bool macho_ofs_to_addr_and_section(macho_command_headers *command_headers, UINT machoclass, dword ofs, ELFAddress *addr, int *section)
{
	return false;
}*/
