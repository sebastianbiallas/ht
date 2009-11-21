/* 
 *	HT Editor
 *	htmacho.cc
 *
 *	Copyright (C) 2003 Stefan Weyergraf
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
#include "endianess.h"
#include "stream.h"
#include "tools.h"

#include "machostruc.h"

#include <stdlib.h>

static format_viewer_if *htmacho_ifs[] = {
	&htmachoheader_if,
	&htmachoimage_if,
	0
};

static ht_view *htmacho_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte magic[4];
	file->seek(0);
	file->read(&magic, sizeof magic);
	if (memcmp(magic, "\xfe\xed\xfa\xce", 4) == 0) {
		ht_macho *g = new ht_macho();
		g->init(b, file, htmacho_ifs, format_group, 0, big_endian, false);
		return g;
	} else if (memcmp(magic, "\xfe\xed\xfa\xcf", 4) == 0) {
		ht_macho *g = new ht_macho();
		g->init(b, file, htmacho_ifs, format_group, 0, big_endian, true);
		return g;
	} else if (memcmp(magic, "\xce\xfa\xed\xfe", 4) == 0) {
		ht_macho *g = new ht_macho();
		g->init(b, file, htmacho_ifs, format_group, 0, little_endian, false);
		return g;
	} else if (memcmp(magic, "\xcf\xfa\xed\xfe", 4) == 0) {
		ht_macho *g = new ht_macho();
		g->init(b, file, htmacho_ifs, format_group, 0, little_endian, true);
		return g;
	}
	return NULL;
}

format_viewer_if htmacho_if = {
	htmacho_init,
	0
};

/*
 *	CLASS ht_macho
 */
void ht_macho::init(Bounds *b, File *f, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs, Endianess image_endianess, bool _64)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_MACHO, f, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_macho");

	String fn;
	file->getFilename(fn);
	LOG("%y: Mach-O: found header at %08qx", &fn, header_ofs);
	
	ht_macho_shared_data *macho_shared = ht_malloc(sizeof(ht_macho_shared_data));

	shared_data = macho_shared;
	macho_shared->image_endianess = image_endianess;
	macho_shared->_64 = _64;
	macho_shared->header_ofs = header_ofs;
	macho_shared->cmds.count = 0;
	macho_shared->cmds.cmds = NULL;

	FileOfs ofs;
	file->seek(header_ofs);
	if (_64) {
		file->read(&macho_shared->header64, sizeof macho_shared->header64);
		createHostStruct(&macho_shared->header64, MACHO_HEADER_64_struct, image_endianess);
		ofs = header_ofs + sizeof macho_shared->header64;
	} else {
		file->read(&macho_shared->header, sizeof macho_shared->header);
		createHostStruct(&macho_shared->header, MACHO_HEADER_struct, image_endianess);
		ofs = header_ofs + sizeof macho_shared->header;
	}

	/* read commands */
	uint nsections = 0;
	macho_shared->cmds.cmds = ht_malloc(sizeof (MACHO_COMMAND_U*) * macho_shared->header.ncmds);
	macho_shared->cmds.count = 0;
	for (uint i=0; i < macho_shared->header.ncmds; i++) {
		MACHO_COMMAND cmd;
		file->seek(ofs);
		file->read(&cmd, sizeof cmd);
		createHostStruct(&cmd, MACHO_COMMAND_struct, image_endianess);
		if (cmd.cmdsize > 1024*1024) break;
		macho_shared->cmds.cmds[i] = ht_malloc(cmd.cmdsize);
		file->seek(ofs);
		if (file->read(macho_shared->cmds.cmds[i], cmd.cmdsize) != cmd.cmdsize) {
			LOG_EX(LOG_ERROR, "%y: Mach-O: error processing command idx %d "
				"(read error %d bytes from %08qx)", &fn, i, cmd.cmdsize, ofs);
			free(macho_shared->cmds.cmds[i]);
			break;
		}
		switch (cmd.cmd) {
		case LC_SEGMENT:
			createHostStruct(macho_shared->cmds.cmds[i], MACHO_SEGMENT_COMMAND_struct, image_endianess);
			// already count sections (needed for reading sections, see below)
			nsections += macho_shared->cmds.cmds[i]->segment.nsects;
			break;
		case LC_SEGMENT_64:
			createHostStruct(macho_shared->cmds.cmds[i], MACHO_SEGMENT_64_COMMAND_struct, image_endianess);
			// already count sections (needed for reading sections, see below)
			nsections += macho_shared->cmds.cmds[i]->segment_64.nsects;
			break;
		case LC_SYMTAB:
			createHostStruct(macho_shared->cmds.cmds[i], MACHO_SYMTAB_COMMAND_struct, image_endianess);
			break;
		case LC_THREAD:
		case LC_UNIXTHREAD: {
			MACHO_THREAD_COMMAND *c = (MACHO_THREAD_COMMAND*)macho_shared->cmds.cmds[i];
			createHostStruct(macho_shared->cmds.cmds[i], MACHO_THREAD_COMMAND_struct, image_endianess);
			switch (macho_shared->header.cputype) {
			case MACHO_CPU_TYPE_ARM:
				switch (c->flavor) {
				case FLAVOR_ARM_THREAD_STATE:
					createHostStruct(&c->state, MACHO_ARM_THREAD_STATE_struct, image_endianess);
					break;
				}
				break;
			case MACHO_CPU_TYPE_I386:
				switch (c->flavor) {
				case -1:
					createHostStruct(&c->state, MACHO_I386_THREAD_STATE_struct, image_endianess);
					break;
				}
				break;
			case MACHO_CPU_TYPE_X86_64:
				switch (c->flavor) {
				case FLAVOR_X86_64_THREAD_STATE:
					createHostStruct(&c->state, MACHO_X86_64_THREAD_STATE_struct, image_endianess);
					break;
				}
				break;
			case MACHO_CPU_TYPE_POWERPC:
				switch (c->flavor) {
				case FLAVOR_PPC_THREAD_STATE:
					createHostStruct(&c->state, MACHO_PPC_THREAD_STATE_struct, image_endianess);
					break;
				}
				break;
			case MACHO_CPU_TYPE_POWERPC64:
				switch (c->flavor) {
				case FLAVOR_PPC_64_THREAD_STATE:
					createHostStruct(&c->state, MACHO_PPC_64_THREAD_STATE_struct, image_endianess);
					break;
				}
				break;
			}
			break;
		}
		default:
			createHostStruct(macho_shared->cmds.cmds[i], MACHO_COMMAND_struct, image_endianess);
		}
		ofs += cmd.cmdsize;
		macho_shared->cmds.count++;
	}

	/* read sections */
	if (_64) {
		ofs = header_ofs + sizeof macho_shared->header64;
	} else {
		ofs = header_ofs + sizeof macho_shared->header;
	}
	macho_shared->section_count = nsections;
	macho_shared->sections = ht_malloc(sizeof (MACHO_SECTION_U) * macho_shared->section_count);
	uint sec = 0;
	for (uint i=0; i < macho_shared->cmds.count; i++) {
		if (macho_shared->cmds.cmds[i]->cmd.cmd == LC_SEGMENT) {
			FileOfs sofs = ofs + sizeof (MACHO_SEGMENT_COMMAND);
			file->seek(sofs);
			for (uint j=0; j < macho_shared->cmds.cmds[i]->segment.nsects; j++) {
				macho_shared->sections[sec]._64 = false;
				file->read(&macho_shared->sections[sec].s, sizeof (MACHO_SECTION));
				createHostStruct(&macho_shared->sections[sec].s, MACHO_SECTION_struct, image_endianess);
				sec++;
			}
		} else if (macho_shared->cmds.cmds[i]->cmd.cmd == LC_SEGMENT_64) {
			FileOfs sofs = ofs + sizeof (MACHO_SEGMENT_64_COMMAND);
			file->seek(sofs);
			for (uint j=0; j < macho_shared->cmds.cmds[i]->segment_64.nsects; j++) {
				macho_shared->sections[sec]._64 = true;
				file->read(&macho_shared->sections[sec].s64, sizeof (MACHO_SECTION_64));
				createHostStruct(&macho_shared->sections[sec].s64, MACHO_SECTION_64_struct, image_endianess);
				sec++;
			}
		}
		ofs += macho_shared->cmds.cmds[i]->cmd.cmdsize;
	}

	/* init ifs */
	ht_format_group::init_ifs(ifs);
}

/*
 *	address conversion routines
 */

bool macho_phys_and_mem_section(MACHO_SECTION_U *s)
{
	return true;
}

bool macho_valid_section(MACHO_SECTION_U *s)
{
	return true;
}

bool macho_addr_to_ofs(MACHO_SECTION_U *s, uint section_count, MACHOAddress addr, FileOfs *ofs)
{
	for (uint i=0; i < section_count; i++) {
		if (macho_phys_and_mem_section(s)) {
			if (s->_64) {
				if (addr >= s->s64.vmaddr 
				 && addr < s->s64.vmaddr + s->s64.vmsize) {
					*ofs = addr - s->s64.vmaddr + s->s64.fileoff;
					return true;
				}
			} else {
				if (addr >= s->s.vmaddr 
				 && addr < s->s.vmaddr + s->s.vmsize) {
					*ofs = addr - s->s.vmaddr + s->s.fileoff;
					return true;
				}
			}
		}
		s++;
	}
	return false;
}

bool macho_addr_to_section(MACHO_SECTION_U *s, uint section_count, MACHOAddress addr, int *section)
{
	for (uint i=0; i < section_count; i++) {
		if (s->_64) {
			if (macho_valid_section(s) && addr >= s->s64.vmaddr && addr < s->s64.vmaddr + s->s64.vmsize) {
				*section = i;
				return true;
			}
		} else {
			if (macho_valid_section(s) && addr >= s->s.vmaddr && addr < s->s.vmaddr + s->s.vmsize) {
				*section = i;
				return true;
			}
		}
		s++;
	}
	return false;
}

bool macho_addr_is_valid(MACHO_SECTION_U *s, uint section_count, MACHOAddress addr)
{
	for (uint i=0; i < section_count; i++) {
		if (s->_64) {
			if (macho_valid_section(s) && addr >= s->s64.vmaddr && addr < s->s64.vmaddr + s->s64.vmsize) {
				return true;
			}
		} else {
			if (macho_valid_section(s) && addr >= s->s.vmaddr && addr < s->s.vmaddr + s->s.vmsize) {
				return true;
			}
		}
		s++;
	}
	return false;
}

/*
 *	offset conversion routines
 */

bool macho_ofs_to_addr(MACHO_SECTION_U *s, uint section_count, FileOfs ofs, MACHOAddress *addr)
{
	for (uint i=0; i < section_count; i++) {
		if (s->_64) {
			if (macho_phys_and_mem_section(s) && ofs >= s->s64.fileoff && ofs < s->s64.fileoff+s->s64.vmsize) {
				*addr = ofs - s->s64.fileoff + s->s64.vmaddr;
				return true;
			}
		} else {
			if (macho_phys_and_mem_section(s) && ofs >= s->s.fileoff && ofs < s->s.fileoff+s->s.vmsize) {
				*addr = ofs - s->s.fileoff + s->s.vmaddr;
				return true;
			}
		}
		s++;
	}
	return false;
}

bool macho_ofs_to_section(MACHO_SECTION_U *s, uint section_count, FileOfs ofs, int *section)
{
	for (uint i=0; i < section_count; i++) {
		if (s->_64) {
			if (macho_valid_section(s) && ofs >= s->s64.fileoff && ofs < s->s64.fileoff+s->s64.vmsize) {
				*section = i;
				return true;
			}
		} else {
			if (macho_valid_section(s) && ofs >= s->s.fileoff && ofs < s->s.fileoff+s->s.vmsize) {
				*section = i;
				return true;
			}
		}
		s++;
	}
	return false;
}
