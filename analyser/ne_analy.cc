/*
 *	HT Editor
 *	ne_analy.cc
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
#include "ne_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htendian.h"
#include "htiobox.h"
#include "htne.h"
#include "htneent.h"
#include "htstring.h"
#include "nestruct.h"
#include "x86asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
void	ne_analyser::init(ht_ne_shared_data *NE_shared, ht_streamfile *File)
{
	ne_shared = NE_shared;
	file = File;

	validarea = new area();
	validarea->init();

	analyser::init();

	/////////////

	set_addr_tree_optimize_threshold(100);
	set_label_tree_optimize_threshold(100);
}


/*
 *
 */
int	ne_analyser::load(ht_object_stream *f)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	GET_OBJECT(f, validarea);
	return analyser::load(f);
}

/*
 *
 */
void	ne_analyser::done()
{
	validarea->done();
	delete validarea;
	analyser::done();
}

/*
 *
 */
void ne_analyser::begin_analysis()
{
	char	buffer[1024];

	/*
	 *	entrypoint
	 */
	 
	ADDR entry = ne_shared->hdr.csip;
	
	if (ne_shared->hdr.flags & NE_FLAGS_BOUND) {
		// bound as family app (needs loader)
		struct bla {
		    ADDR a;
		    char *b;
		    char *c;
		};
		bla blabla[] = {
			{ NE_MAKE_ADDR(0x0001, 0x0004), "loader_ptr_1", "loader_entry_1"},
			{ NE_MAKE_ADDR(0x0001, 0x0008), "loader_ptr_2", "loader_entry_2"},
			{ NE_MAKE_ADDR(0x0001, 0x0018), "loader_ptr_3", "loader_entry_3"},
			{ 0, 0, 0 },
		};
		bla *blla = blabla;
		while (blla->b) {
			add_comment(blla->a, 0, "");
			assign_label(blla->a, blla->b, label_data);
			data->set_int_addr_type(blla->a, dst_idword, 4);
			ADDR addr;
			byte buf[4];
			if (valid_addr(blla->a, scinitialized) && bufptr(blla->a, buf, 4)==4) {
				sprintf(buffer, "; pointer to %s", blla->c);
				add_comment(blla->a, 0, buffer);
				addr = create_host_int(buf, 4, little_endian);
				add_comment(addr, 0, "");
				assign_label(addr, blla->c, label_func);
				push_addr(addr, addr);
			}
			blla++;
		}
	} else {
		push_addr(entry, entry);
	}
	
	/*
	 * give all sections a descriptive comment:
	 */

	NE_SEGMENT *s = ne_shared->segments.segments;
	char blub[100];
	for (UINT i = 0; i < ne_shared->segments.segment_count; i++) {
		ADDR secaddr = NE_get_seg_addr(ne_shared, i);

		UINT epsize = MIN(NE_get_seg_vsize(ne_shared, i), NE_get_seg_psize(ne_shared, i));
		UINT evsize = MAX(NE_get_seg_vsize(ne_shared, i), NE_get_seg_psize(ne_shared, i));

		sprintf(blub, ";  section %d <%s>", i+1, get_addr_section_name(secaddr));
		add_comment(secaddr, 0, "");
		add_comment(secaddr, 0, ";******************************************************************");
		add_comment(secaddr, 0, blub);
		sprintf(blub, ";  virtual address  %08x  virtual size   %08x", NE_get_seg_addr(ne_shared, i), evsize);
		add_comment(secaddr, 0, blub);
		sprintf(blub, ";  file offset      %08x  file size      %08x", NE_get_seg_ofs(ne_shared, i), epsize);
		add_comment(secaddr, 0, blub);
		add_comment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		sprintf(blub, ";  end of section <%s>", get_addr_section_name(secaddr));
		ADDR secend_addr = secaddr + evsize;
		new_addr(secend_addr)->flags |= AF_FUNCTION_END;
		add_comment(secend_addr, 0, "");
		add_comment(secend_addr, 0, ";******************************************************************");
		add_comment(secend_addr, 0, blub);
		add_comment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		if (valid_addr(secaddr, scinitialized) && valid_addr(secaddr + epsize, scinitialized)) {
			initialized->add(secaddr, secaddr + epsize);
		}
		s++;
	}

	// entrypoints

	int entrypoint_count = ne_shared->entrypoints->count();
	int *entropy = random_permutation(entrypoint_count);
	for (int i=0; i<entrypoint_count; i++) {
		ht_ne_entrypoint *f = (ht_ne_entrypoint*)ne_shared->entrypoints->get(*(entropy+i));
		if (f) {
			ADDR address = NE_MAKE_ADDR(f->seg, f->offset);
			if (valid_addr(address, scvalid)) {
				char *label;
				if (f->name) {
					sprintf(buffer, "; exported function %s, ordinal %04x", f->name, f->ordinal);
				} else {
					sprintf(buffer, "; unnamed exported function, ordinal %04x", f->ordinal);
				}
				label = export_func_name(f->name, f->ordinal);
				add_comment(address, 0, "");
				add_comment(address, 0, ";********************************************************");
				add_comment(address, 0, buffer);
				add_comment(address, 0, ";********************************************************");
				push_addr(address, address);
				assign_label(address, label, label_func);
				free(label);
			}
		}
	}
	if (entropy) free(entropy);

/*	int import_count=pe_shared->imports.funcs->count();
	entropy = random_permutation(import_count);
	for (int i=0; i<import_count; i++) {
		ht_pe_import_function *f=(ht_pe_import_function *)pe_shared->imports.funcs->get(*(entropy+i));
		ht_pe_import_library *d=(ht_pe_import_library *)pe_shared->imports.libs->get(f->libidx);
		char *label;
		label = import_func_name(d->name, (f->byname) ? f->name.name : NULL, f->ordinal);
		add_comment(f->address, 0, "");
		assign_label(f->address, label, label_func);
		data->set_int_addr_type(f->address, dst_idword, 4);
		free(label);
	}
	if (entropy) free(entropy);

	int dimport_count=pe_shared->dimports.funcs->count();
	entropy = random_permutation(dimport_count);
	for (int i=0; i<dimport_count; i++) {
		// FIXME: delay imports need work (push addr)
		ht_pe_import_function *f=(ht_pe_import_function *)pe_shared->dimports.funcs->get(*(entropy+i));
		ht_pe_import_library *d=(ht_pe_import_library *)pe_shared->dimports.libs->get(f->libidx);
		char *label;
		label = import_func_name(d->name, f->byname ? f->name.name : NULL, f->ordinal);
		add_comment(f->address, 0, "");
		assign_label(f->address, label, label_func);
//		data->setintaddrtype(f->address, dstidword, 4);
		free(label);
	}
	if (entropy) free(entropy);*/

	add_comment(entry, 0, "");
	add_comment(entry, 0, ";****************************");
	if (ne_shared->hdr.flags & NE_FLAGS_NOTAPROCESS) {
		add_comment(entry, 0, ";  library entry point");
	} else {
		add_comment(entry, 0, ";  program entry point");
	}
	add_comment(entry, 0, ";****************************");
	if (valid_code_addr(entry)) {
		assign_label(entry, "entrypoint", label_func);
	} else {
		assign_label(entry, "entrypoint", label_data);
	}

	set_addr_tree_optimize_threshold(1000);
	set_label_tree_optimize_threshold(1000);

	analyser::begin_analysis();
}

/*
 *
 */
OBJECT_ID	ne_analyser::object_id()
{
	return ATOM_NE_ANALYSER;
}

/*
 *
 */
UINT ne_analyser::bufptr(ADDR Addr, byte *buf, int size)
{
	FILEADDR ofs = file_addr(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

/*
 *
 */
assembler *ne_analyser::create_assembler()
{
/* FIXME: 16/32 */
	assembler *a = new x86asm(X86_OPSIZE16, X86_ADDRSIZE16);
	a->init();
	return a;
}

/*
 *
 */
FILEADDR ne_analyser::file_addr(ADDR Addr)
{
	if (valid_addr(Addr, scinitialized)) {
		FILEOFS ofs;
		if (!NE_addr_to_ofs(ne_shared, Addr, &ofs)) {
			return INVALID_FILE_OFS;
		}
		return ofs;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
char *ne_analyser::get_addr_section_name(ADDR Addr)
{
	static char segmentname[16];
	int i;
	if (!NE_addr_to_segment(ne_shared, Addr, &i)) return NULL;
	sprintf(segmentname, "seg%d", i+1);
	return segmentname;
}

/*
 *
 */
char	*ne_analyser::get_name()
{
	return file->get_desc();
}

/*
 *
 */
char *ne_analyser::get_type()
{
	return "NE/Analyser";
}

/*
 *
 */
void ne_analyser::init_code_analyser()
{
	analyser::init_code_analyser();
	code->loaddefs("analyser/sign.def");
}

/*
 *
 */
void ne_analyser::init_unasm()
{
	DPRINTF("ne_analy: ");
	DPRINTF("initing analy_x86_disassembler\n");
	analy_disasm = new analy_x86_disassembler();
	((analy_x86_disassembler*)analy_disasm)->init(this, true);
}

/*
 *
 */
void ne_analyser::log(char *msg)
{
	/*
	 *	log() creates to much traffic so dont log
	 *   perhaps we reactivate this later
	 *
	 */
/*	LOG(msg);*/
}

/*
 *
 */
ADDR ne_analyser::next_valid(ADDR Addr)
{
	return INVALID_ADDR; //validarea->findnext(Addr);
	// FIXME (hack while validarea isnt active):
//   taddr *a = enum_addrs(Addr);
//   return (a)?a->addr:INVALID_ADDR;
}

/*
 *
 */
void ne_analyser::store(ht_object_stream *st)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	PUT_OBJECT(st, validarea);
	analyser::store(st);
}

/*
 *
 */
int	ne_analyser::query_config(int mode)
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
ADDR ne_analyser::vaddr(FILEADDR fileaddr)
{
	ADDR a;
	if (NE_ofs_to_addr(ne_shared, fileaddr, &a)) {
		return a;
	} else {
		return INVALID_ADDR;
	}
}

/*
 *
 */
bool ne_analyser::valid_addr(ADDR Addr, tsectype action)
{
	ne_segment_headers *segments = &ne_shared->segments;
	int sec;
	if (!NE_addr_to_segment(ne_shared, Addr, &sec)) return false;
	NE_SEGMENT *s = segments->segments + sec;
	switch (action) {
		case scvalid:
			return true;
		case scread:
			return !(s->flags & NE_READONLY) || (s->flags & NE_DATA);
		case scwrite:
			return !(s->flags & NE_READONLY) && (s->flags & NE_DATA);
		case screadwrite:
			return !(s->flags & NE_READONLY) && (s->flags & NE_DATA);
		case sccode:
			return (!(s->flags & NE_DATA)) && NE_addr_is_physical(ne_shared, Addr);
		case scinitialized:
			return NE_addr_is_physical(ne_shared, Addr);
	}
	return false;
}

