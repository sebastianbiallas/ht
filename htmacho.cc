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

	FILEOFS ofs;
	/* read header */
	file->seek(header_ofs);
	file->read(&macho_shared->header, sizeof macho_shared->header);
	create_host_struct(&macho_shared->header, MACHO_HEADER_struct, big_endian);

	/* read commands */
	uint nsections = 0;
	ofs = header_ofs+sizeof macho_shared->header;
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
				// already count sections (needed for reading sections, see below)
				nsections += macho_shared->cmds.cmds[i]->segment.nsects;
				break;
			case LC_SYMTAB:
				create_host_struct(macho_shared->cmds.cmds[i], MACHO_SYMTAB_COMMAND_struct, big_endian);
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

	/* read sections */
	ofs = header_ofs+sizeof macho_shared->header;
	macho_shared->sections.count = nsections;
	macho_shared->sections.sections = (MACHO_SECTION*)malloc(sizeof (MACHO_SECTION) * macho_shared->sections.count);
	uint sec = 0;
	for (uint i=0; i<macho_shared->cmds.count; i++) {
		if (macho_shared->cmds.cmds[i]->cmd.cmd == LC_SEGMENT) {
			FILEOFS sofs = ofs+sizeof (MACHO_SEGMENT_COMMAND);
			file->seek(sofs);
			for (uint j=0; j<macho_shared->cmds.cmds[i]->segment.nsects; j++) {
				file->read(&macho_shared->sections.sections[sec], sizeof (MACHO_SECTION));
				create_host_struct(&macho_shared->sections.sections[sec], MACHO_SECTION_struct, big_endian);
				sec++;
			}
		}
		ofs += macho_shared->cmds.cmds[i]->cmd.cmdsize;
	}

	/* init ifs */
	ht_format_group::init_ifs(ifs);
}

void ht_macho::done()
{
	ht_format_group::done();
}

/*
 *	address conversion routines
 */

bool macho_phys_and_mem_section(MACHO_SECTION *s, UINT machoclass)
{
	return true;
}

bool macho_valid_section(MACHO_SECTION *s, UINT machoclass)
{
	return true;
}

bool macho_addr_to_ofs(macho_sections *sections, UINT machoclass, MACHOAddress addr, dword *ofs)
{
	MACHO_SECTION *s = sections->sections;
	for (UINT i=0; i < sections->count; i++) {
		if (macho_phys_and_mem_section(s, machoclass) &&
		(addr >= s->vmaddr) && (addr < s->vmaddr+s->vmsize)) {
			*ofs = addr - s->vmaddr + s->fileoff;
			return true;
		}
		s++;
	}
	return false;
}


bool macho_addr_to_section(macho_sections *sections, UINT machoclass, MACHOAddress addr, int *section)
{
	MACHO_SECTION *s = sections->sections;
	for (UINT i=0; i < sections->count; i++) {
		if ((macho_valid_section(s, machoclass)) && (addr >= s->vmaddr) && (addr < s->vmaddr + s->vmsize)) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}

bool macho_addr_is_valid(macho_sections *sections, UINT machoclass, MACHOAddress addr)
{
	MACHO_SECTION *s = sections->sections;
	for (UINT i=0; i < sections->count; i++) {
		if ((macho_valid_section(s, machoclass)) && (addr >= s->vmaddr) && (addr < s->vmaddr + s->vmsize)) {
			return true;
		}
		s++;
	}
	return false;
}

/*bool macho_addr_is_physical(macho_sections *sections, UINT machoclass, ELFAddress addr)
{
	return false;
}*/

/*
 *	offset conversion routines
 */

bool macho_ofs_to_addr(macho_sections *sections, UINT machoclass, dword ofs, MACHOAddress *addr)
{
	MACHO_SECTION *s = sections->sections;
	for (UINT i=0; i < sections->count; i++) {
		if ((macho_phys_and_mem_section(s, machoclass)) && (ofs>=s->fileoff) && (ofs<s->fileoff+s->vmsize)) {
			*addr = ofs - s->fileoff + s->vmaddr;
			return true;
		}
		s++;
	}
	return false;
}

bool macho_ofs_to_section(macho_sections *sections, UINT machoclass, dword ofs, dword *section)
{
	MACHO_SECTION *s = sections->sections;
	for (UINT i=0; i < sections->count; i++) {
		if ((macho_valid_section(s, machoclass)) && (ofs >= s->fileoff) && (ofs<s->fileoff+s->vmsize)) {
			*section = i;
			return true;
		}
		s++;
	}
	return false;
}

/*bool macho_ofs_to_addr_and_section(macho_sections *sections, UINT machoclass, dword ofs, ELFAddress *addr, int *section)
{
	return false;
}*/
