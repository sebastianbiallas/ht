/*
 *	HT Editor
 *	htpeil.cc
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

#include "formats.h"
#include "htapp.h"
#include "atom.h"
#include "htcoff.h"
#include "htctrl.h"
#include "endianess.h"
#include "hthex.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpe.h"
#include "htpehead.h"
#include "htpeil.h"
#include "httag.h"
#include "strtools.h"
#include "snprintf.h"

#include "pestruct.h"
#include "ilopc.h"
#include "ilstruct.h"

#include <string.h>

static ht_mask_ptable il_directory[] = {
	{"size",           STATICTAG_EDIT_DWORD_LE("00000000")},
	{"major version",  STATICTAG_EDIT_WORD_LE("00000004")},
	{"minor version",  STATICTAG_EDIT_WORD_LE("00000006")},
	{"metadata rva",   STATICTAG_EDIT_DWORD_LE("00000008")},
	{"metadata size",  STATICTAG_EDIT_DWORD_LE("0000000c")},
	{"attributes",     STATICTAG_EDIT_DWORD_LE("00000010")},
	{"entrypoint token",STATICTAG_EDIT_DWORD_LE("00000014")},
	{"resources rva",   STATICTAG_EDIT_DWORD_LE("00000018")},
	{"resources size",  STATICTAG_EDIT_DWORD_LE("0000001c")},
	{"strong name sig",  STATICTAG_EDIT_DWORD_LE("00000020")},
	{"strong name sig",  STATICTAG_EDIT_DWORD_LE("00000024")},
	{"code manager rva",   STATICTAG_EDIT_DWORD_LE("00000028")},
	{"code manager size",  STATICTAG_EDIT_DWORD_LE("0000002c")},
	{"vtable fixups rva",   STATICTAG_EDIT_DWORD_LE("00000030")},
	{"vtable fixups size",   STATICTAG_EDIT_DWORD_LE("00000034")},
	{"export address table",  STATICTAG_EDIT_DWORD_LE("00000038")},
	{"export address table",  STATICTAG_EDIT_DWORD_LE("0000003c")},
	{"managed native header", STATICTAG_EDIT_DWORD_LE("00000040")},
	{0, 0}
};

static ht_mask_ptable metadata_section[] = {
	{"magic",                 STATICTAG_EDIT_DWORD_LE("00000000")},
	{"major_version",         STATICTAG_EDIT_WORD_LE("00000004")},
	{"minor_version",         STATICTAG_EDIT_WORD_LE("00000006")},
	{"unknown",               STATICTAG_EDIT_DWORD_LE("00000008")},
	{"version_string_length", STATICTAG_EDIT_DWORD_LE("00000010")},
	{0, 0}
};

static ht_view *htpeil_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return NULL;

	uint32 sec_rva, sec_size;
	FileOfs sec_ofs;
	sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IL].address;
	sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_IL].size;
	if (!sec_rva || !sec_size) return NULL;

	PE_IL_DIRECTORY dir;
	bool pe_bigendian = false;
	ht_pe_il_viewer *v;

	if (!pe_rva_to_ofs(&pe_shared->sections, sec_rva, &sec_ofs)) goto read_error;
		
	file->seek(sec_ofs);
	if (file->read(&dir, sizeof dir) != sizeof dir) goto read_error;

	createHostStruct(&dir, PE_IL_DIRECTORY_struct, little_endian);
	
	if (sec_size != dir.size) goto read_error;
	
	pe_shared->il = new ht_pe_il();
	pe_shared->il->dir = dir;

	v = new ht_pe_il_viewer();
	v->init(b, DESC_PE_IL, VC_EDIT | VC_SEARCH, file, group);
	pe_shared->v_il = v;
	ht_mask_sub *s;
//	ht_collapsable_sub *cs;
	
	s = new ht_mask_sub();
	s->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* COM+ directory at offset %08qx", sec_ofs);
	s->add_mask(info);
	v->insertsub(s);

	/* FIXME: */
	
	s=new ht_mask_sub();
	s->init(file, 1);
	s->add_staticmask_ptable(il_directory, sec_ofs, pe_bigendian);
	v->insertsub(s);
	
	FileOfs metadata_ofs;
	if (pe_rva_to_ofs(&pe_shared->sections, dir.metadata_section_rva, &metadata_ofs)) {
		/* read metadata section*/
		IL_METADATA_SECTION metadata;
		file->seek(metadata_ofs);
		if (file->read(&metadata, sizeof metadata) == sizeof metadata) {
			createHostStruct(&metadata, IL_METADATA_SECTION_struct, little_endian);
			pe_shared->il->metadata = metadata;
			uint32 add = 2;
			if (metadata.minor_version == 1) {
				// FIXME: align metadata.version_string_length
				uint32 version_string_length;
				file->read(&version_string_length, 4); // dummy
				file->read(&version_string_length, 4);
				version_string_length = createHostInt(&version_string_length, 4, little_endian);
				add += version_string_length + 8;
			}
			FileOfs ofs = metadata_ofs + sizeof metadata + add;
			file->seek(ofs);
			uint16 count;
			file->read(&count, 2);
			count = createHostInt(&count, 2, little_endian);
			pe_shared->il->entries = new Array(true);
			for (uint i=0; i < count; i++) {
				IL_METADATA_SECTION_ENTRY sec_entry;
				ht_il_metadata_entry *entry;
				// FIXME: error handling
				file->read(&sec_entry, sizeof sec_entry);
				createHostStruct(&sec_entry, IL_METADATA_SECTION_ENTRY_struct, little_endian);
				String name("?");
				file->readStringz(name);
				int nlen = name.length() + 1;
				uint32 dummy;
				if (nlen % 4) {
					// align properly
					file->read(&dummy, 4 - nlen % 4);
				}
				entry = new ht_il_metadata_entry(name.contentChar(), metadata_ofs+sec_entry.offset, sec_entry.size);
//				fprintf(stderr, "%s %x %x\n", name.contentChar(), metadata_ofs+sec_entry.offset, sec_entry.size);
				pe_shared->il->entries->insert(entry);
			}
			for (uint i=0; i<count; i++) {
				ht_il_metadata_entry *entry = (ht_il_metadata_entry *)(*pe_shared->il->entries)[i];
				if (strcmp(entry->name, "#~") == 0) {
					// token index
					char dummy[8];
					uint64 types;
					file->seek(entry->offset);
					file->read(&dummy, 8);
					file->read(&types, 8);
//					file->read();
				} else if (strcmp(entry->name, "#US")==0) {
					// stringpool
					pe_shared->il->string_pool = ht_malloc(entry->size);
					file->seek(entry->offset);
					pe_shared->il->string_pool_size = file->read(pe_shared->il->string_pool, entry->size);
				}
			}
		}		
	}
	
	return v;

read_error:
	return NULL;
}

format_viewer_if htpeil_if = {
	htpeil_init,
	0
};

/*
 *	CLASS ht_pe_header_viewer
 */

void ht_pe_il_viewer::init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *group)
{
	ht_uformat_viewer::init(b, desc, caps, file, group);
	VIEW_DEBUG_NAME("ht_pe_il_viewer");
}

void ht_pe_il_viewer::done()
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();
	if (pe_shared && pe_shared->il) {
		free(pe_shared->il->string_pool);
		delete pe_shared->il->entries;
		delete pe_shared->il;
	}
	ht_uformat_viewer::done();
}

ht_il_metadata_entry::ht_il_metadata_entry(const char *n, uint32 o, uint32 s)
{
	name = ht_strdup(n);
	offset = o;
	size = s;
}

ht_il_metadata_entry::~ht_il_metadata_entry()
{
	free(name);
}

/*
 *	NOTE: ILunpack returns result in host-endianess
 */
int ILunpackDword(uint32 &result, const byte *buf, int len)
{
	if (len) {
		result = *(buf++);
		len--;
		if (result < 0x80) return 1; // one byte form
		if (!len) return 0;
		if ((result & 0xc0) == 0x80) {
			// two byte form
			result = ((result & 0x3f) << 8) | buf[0];
			return 2;
		} else if ((result & 0xe0) == 0xc0) {
			// four byte form
			if (len < 2) return 0;
			result = ((result & 0x1F) << 24) |
				    (((uint32)buf[0]) << 16) |
				    (((uint32)buf[1]) << 8) |
					 (uint32)buf[2];
			return 4;
		} else if ((result & 0xf0) == 0xe0) {
			// five byte form
			if (len < 3) return 0;
			result = (((uint32)buf[0]) << 24) |
				    (((uint32)buf[1]) << 16) |
				    (((uint32)buf[2]) << 8) |
					 (uint32)buf[3];
			return 5;
		}
	}
	return 0;
}

int ILunpackToken(uint32 &result, const byte *buf, int len)
{
	int read = ILunpackDword(result, buf, len);
	if (!read) return 0;
	uint32 type;
	switch (result & 0x03) {
		case 0x00:
			type = IL_META_TOKEN_TYPE_DEF;
			break;
		case 0x01:
			type = IL_META_TOKEN_TYPE_REF;
			break;
		case 0x02:
			type = IL_META_TOKEN_TYPE_SPEC;
			break;
		default:
			type = IL_META_TOKEN_BASE_TYPE;
	}
	result = (result >> 2) | type;
	return read;
}
