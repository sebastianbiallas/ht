/* 
 *	HT Editor
 *	elf_analy.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include "analy.h"
#include "analy_alpha.h"
#include "analy_names.h"
#include "analy_register.h"
#include "analy_x86.h"
#include "codeanaly.h"
#include "global.h"
#include "elf_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htelf.h"
#include "htstring.h"
#include "pestruct.h"
#include "x86asm.h"

extern "C" {
#include "demangle.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
 
void	elf_analyser::init(ht_elf_shared_data *Elf_shared, ht_streamfile *File)
{
	elf_shared = Elf_shared;
	file = File;

	ADDR entry=elf_shared->header32.e_entry;

	validarea = new area();
	validarea->init();

	analyser::init();

	/////////////

	set_addr_tree_optimize_threshold(100);
	set_label_tree_optimize_threshold(100);

	/*
	 *	entrypoint
	 */
	push_addr(entry, entry);
	
	/*
	 *	give all sections a descriptive comment:
	 */

	ELF_SECTION_HEADER32 *s=elf_shared->sheaders.sheaders32;
	char blub[100];
	for (UINT i=0; i<elf_shared->sheaders.count; i++) {
		ADDR secaddr = s->sh_addr;
		if (valid_addr(secaddr, scvalid)) {
			sprintf(blub, ";  section %d <%s>", i+1, get_addr_section_name(secaddr));
			add_comment(secaddr, 0, "");
			add_comment(secaddr, 0, ";******************************************************************");
			add_comment(secaddr, 0, blub);
			sprintf(blub, ";  virtual address  %08x  virtual size   %08x", s->sh_addr, s->sh_size);
			add_comment(secaddr, 0, blub);
			sprintf(blub, ";  file offset      %08x  file size      %08x", s->sh_offset, s->sh_size);
			add_comment(secaddr, 0, blub);
			add_comment(secaddr, 0, ";******************************************************************");

			// mark end of sections
			sprintf(blub, ";  end of section <%s>", get_addr_section_name(secaddr));
			ADDR secend_addr = secaddr + s->sh_size;
			new_addr(secend_addr)->flags |= AF_FUNCTION_END;
			add_comment(secend_addr, 0, "");
			add_comment(secend_addr, 0, ";******************************************************************");
			add_comment(secend_addr, 0, blub);
			add_comment(secend_addr, 0, ";******************************************************************");

			validarea->add(secaddr, secend_addr);
		}

		s++;
	}

/* symbols */
	for (UINT i=1; i<elf_shared->sheaders.count; i++) {
		if ((elf_shared->sheaders.sheaders32[i].sh_type==ELF_SHT_SYMTAB) || (elf_shared->sheaders.sheaders32[i].sh_type==ELF_SHT_DYNSYM)) {
			init_insert_symbols(i);
		}
	}

	assign_label(entry, "entrypoint", label_func);
	add_comment(entry, 0, "");
	add_comment(entry, 0, ";****************************");
	switch (elf_shared->header32.e_type) {
		case ELF_ET_DYN:
			add_comment(entry, 0, ";  dynamic executable entry point");
			break;
		case ELF_ET_EXEC:
			add_comment(entry, 0, ";  executable entry point");
			break;
		default:
			add_comment(entry, 0, ";  entry point");
	}
	add_comment(entry, 0, ";****************************");

	set_addr_tree_optimize_threshold(1000);
	set_label_tree_optimize_threshold(1000);
}

/*
 *
 */
void	elf_analyser::init_insert_symbols(int shidx)
{
	char elf_buffer[1024];

	FILEOFS h=elf_shared->sheaders.sheaders32[shidx].sh_offset;
	FILEOFS sto=elf_shared->sheaders.sheaders32[elf_shared->sheaders.sheaders32[shidx].sh_link].sh_offset;
	UINT symnum=elf_shared->sheaders.sheaders32[shidx].sh_size / sizeof (ELF_SYMBOL32);

	int *entropy = random_permutation(symnum);	
	for (UINT i=0; i<symnum; i++) {
		ELF_SYMBOL32 sym;
		if (entropy[i] == 0) continue;
		file->seek(h+entropy[i]*sizeof (ELF_SYMBOL32));
		file->read(&sym, sizeof sym);
		create_host_struct(&sym, ELF_SYMBOL32_struct, elf_shared->byte_order);

		file->seek(sto+sym.st_name);
		char *name = fgetstrz(file);
		if (!name) continue;

		switch (sym.st_shndx) {
			case ELF_SHN_UNDEF:
				break;
			case ELF_SHN_ABS:
				break;
			case ELF_SHN_COMMON:
				break;
			default: {
				// sym.st_shndx
				break;
			}
		}

		char *bind;
		switch (ELF32_ST_BIND(sym.st_info)) {
			case ELF_STB_LOCAL:
				bind="local";
				break;
			case ELF_STB_GLOBAL:
				bind="global";
				break;
			case ELF_STB_WEAK:
				bind="weak";
				break;
			default:
				bind="?";
				break;
		}

		switch (ELF32_ST_TYPE(sym.st_info)) {
			case ELF_STT_NOTYPE:
			case ELF_STT_FUNC: {
				char *label = name;
				if (!find_label(label)) {
					ADDR address = sym.st_value;

					char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);

					make_valid_name(label, label);

					sprintf(elf_buffer, "; function %s (%s)", (demangled) ? demangled : label, bind);

					if (demangled) free(demangled);

					add_comment(address, 0, "");
					add_comment(address, 0, ";********************************************************");
					add_comment(address, 0, elf_buffer);
					add_comment(address, 0, ";********************************************************");
					push_addr(address, address);
					assign_label(address, label, label_func);
				}
				break;
			}
			case ELF_STT_OBJECT: {
				char *label = name;
				if (!find_label(label)) {
					ADDR address = sym.st_value;
		
					char *demangled = cplus_demangle(label, DMGL_PARAMS | DMGL_ANSI);
					
					make_valid_name(label, label);
					
					sprintf(elf_buffer, "; data object %s, size %d (%s)", (demangled) ? demangled : label, sym.st_size, bind);

					if (demangled) free(demangled);

					add_comment(address, 0, "");
					add_comment(address, 0, ";********************************************************");
					add_comment(address, 0, elf_buffer);
					add_comment(address, 0, ";********************************************************");
					assign_label(address, label, label_data);
				}
				break;
			}
			case ELF_STT_SECTION:
			case ELF_STT_FILE:
				break;
		}
		free(name);
	}
	if (entropy) free(entropy);
}

/*
 *
 */
int	elf_analyser::load(ht_object_stream *f)
{
	GET_OBJECT(f, validarea);
	return analyser::load(f);
}

/*
 *
 */
void	elf_analyser::done()
{
	validarea->done();
	delete validarea;
	analyser::done();
}

OBJECT_ID	elf_analyser::object_id()
{
	return ATOM_ELF_ANALYSER;
}

/*
 *
 */
UINT elf_analyser::bufptr(ADDR Addr, byte *buf, int size)
{
	FILEADDR ofs = file_addr(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);;
}

/*
 *
 */
assembler *elf_analyser::create_assembler()
{
	switch (elf_shared->header32.e_machine) {
		case ELF_EM_386:
			assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
			a->init();
			return a;
	}
	return NULL;
}

/*
 *
 */
FILEADDR elf_analyser::file_addr(ADDR Addr)
{
	if (valid_addr(Addr, scinitialized)) {
		dword ofs;
		if (!elf_addr_to_ofs(&elf_shared->sheaders, elf_shared->ident.e_ident[ELF_EI_CLASS], Addr, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
char elf_sectionname[33];

char *elf_analyser::get_addr_section_name(ADDR Addr)
{
	elf_section_headers *sections=&elf_shared->sheaders;
	int i;
	if (!elf_addr_to_section(sections, elf_shared->ident.e_ident[ELF_EI_CLASS], Addr, &i)) return NULL;
	strncpy(elf_sectionname, elf_shared->shnames[i], 32);
	elf_sectionname[32]=0;
	return elf_sectionname;
}

/*
 *
 */
char	*elf_analyser::get_name()
{
	return file->get_desc();
}

/*
 *
 */
char *elf_analyser::get_type()
{
	return "ELF/Analyser";
}

/*
 *
 */
void elf_analyser::init_code_analyser()
{
	analyser::init_code_analyser();
	code->loaddefs("analyser/sign.def");
}

/*
 *
 */
void elf_analyser::init_unasm()
{
	DPRINTF("elf_analy: ");
	switch (elf_shared->header32.e_machine) {
		case ELF_EM_386: // Intel
			DPRINTF("initing analy_x86_disassembler\n");
			analy_disasm = new analy_x86_disassembler();
			((analy_x86_disassembler*)analy_disasm)->init(this);
			break;
		default:
			DPRINTF("no apropriate disassembler for machine %04x\n", elf_shared->header32.e_machine);
			warnbox("No disassembler for unknown machine type %04x!", elf_shared->header32.e_machine);
	}
}

/*
 *
 */
void elf_analyser::log(char *msg)
{
	/*
	 *	log() does to much traffic so dont log
	 *   perhaps we reactivate this later
	 *
	 */
/*	LOG(msg);*/
}

/*
 *
 */
ADDR elf_analyser::next_valid(ADDR Addr)
{
	return INVALID_ADDR; //validarea->findnext(Addr);
	// FIXME (hack while validarea isnt active):
//   taddr *a = enum_addrs(Addr);
//   return (a)?a->addr:INVALID_ADDR;
}

/*
 *
 */
void elf_analyser::store(ht_object_stream *f)
{
	PUT_OBJECT(f, validarea);
	analyser::store(f);
}

/*
 *
 */
int	elf_analyser::query_config(int mode)
{
	switch (mode) {
		case Q_DO_ANALYSIS:
		case Q_ENGAGE_CODE_ANALYSER:
		case Q_ENGAGE_DATA_ANALYSER:
			return true;
		default:
			return 0;
	}
}

/*
 *
 */
ADDR elf_analyser::vaddr(FILEADDR fileaddr)
{
	ADDR a;
	if (elf_ofs_to_addr(&elf_shared->sheaders, elf_shared->ident.e_ident[ELF_EI_CLASS], fileaddr, &a)) {
		return a;
	} else {
		return INVALID_ADDR;
	}
}

/*
 *
 */
bool elf_analyser::valid_addr(ADDR Addr, tsectype action)
{
	elf_section_headers *sections=&elf_shared->sheaders;
	int sec;
	if (!elf_addr_to_section(sections, elf_shared->ident.e_ident[ELF_EI_CLASS], Addr, &sec)) return false;
	ELF_SECTION_HEADER32 *s=sections->sheaders32+sec;
	switch (action) {
		case scvalid:
			return true;
		case scread:
			return true;
		case scwrite:
		case screadwrite:
			return s->sh_flags & ELF_SHF_WRITE;
		case sccode:
			return (s->sh_flags & ELF_SHF_EXECINSTR) && (s->sh_type==ELF_SHT_PROGBITS);
		case scinitialized:
			return s->sh_type==ELF_SHT_PROGBITS;
	}
	return false;
}


