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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analy.h"
#include "analy_alpha.h"
#include "analy_names.h"
#include "analy_register.h"
#include "analy_x86.h"
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
#include "snprintf.h"
#include "x86asm.h"

/*
 *
 */
void	NEAnalyser::init(ht_ne_shared_data *NE_shared, ht_streamfile *File)
{
	ne_shared = NE_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();

	/////////////

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);
}


/*
 *
 */
int	NEAnalyser::load(ht_object_stream *f)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	GET_OBJECT(f, validarea);
	return Analyser::load(f);
}

/*
 *
 */
void	NEAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

/*
 *
 */
void NEAnalyser::beginAnalysis()
{
	char	buffer[1024];

	/*
	 *	entrypoint
	 */
	 
	Address *entry = createAddress1616(NE_ADDR_SEG(ne_shared->hdr.csip), NE_ADDR_OFS(ne_shared->hdr.csip));
	
	if (ne_shared->hdr.flags & NE_FLAGS_SELFLOAD) {
		// bound as family app (needs loader)
		struct bla {
		    NEAddress a;
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
			Address *bllaa = createAddress1616(NE_ADDR_SEG(blla->a), NE_ADDR_OFS(blla->a));
			addComment(bllaa, 0, "");
			assignSymbol(bllaa, blla->b, label_data);
			data->setIntAddressType(bllaa, dst_idword, 4);
			NEAddress addr;
			byte buf[4];
			if (validAddress(bllaa, scinitialized) && bufPtr(bllaa, buf, 4)==4) {
				sprintf(buffer, "; pointer to %s", blla->c);
				addComment(bllaa, 0, buffer);
				addr = create_host_int(buf, 4, little_endian);
				Address *a = createAddress1616(NE_ADDR_SEG(addr), NE_ADDR_OFS(addr));
				addComment(a, 0, "");
				assignSymbol(a, blla->c, label_func);
				pushAddress(a, a);
				delete a;
			}
			blla++;
			delete bllaa;
		}
	} else {
		pushAddress(entry, entry);
	}
	
	/*
	 * give all sections a descriptive comment:
	 */

	NE_SEGMENT *s = ne_shared->segments.segments;
	char blub[100];
	for (UINT i = 0; i < ne_shared->segments.segment_count; i++) {
		Address *secaddr = createAddress1616(NE_ADDR_SEG(NE_get_seg_addr(ne_shared, i)), NE_ADDR_OFS(NE_get_seg_addr(ne_shared, i)));

		UINT epsize = MIN(NE_get_seg_vsize(ne_shared, i), NE_get_seg_psize(ne_shared, i));
		UINT evsize = MAX(NE_get_seg_vsize(ne_shared, i), NE_get_seg_psize(ne_shared, i));

		sprintf(blub, ";  section %d <%s>", i+1, getSegmentNameByAddress(secaddr));
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  virtual address  %08x  virtual size   %08x", NE_get_seg_addr(ne_shared, i), evsize);
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  file offset      %08x  file size      %08x", NE_get_seg_ofs(ne_shared, i), epsize);
		addComment(secaddr, 0, blub);
		addComment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		sprintf(blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
		Address *secend_addr = (Address *)secaddr->duplicate();
		secend_addr->add(evsize);
		newLocation(secend_addr)->flags |= AF_FUNCTION_END;
		addComment(secend_addr, 0, "");
		addComment(secend_addr, 0, ";******************************************************************");
		addComment(secend_addr, 0, blub);
		addComment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		Address *seciniaddr = (Address *)secaddr->duplicate();
		seciniaddr->add(epsize);
		if (validAddress(secaddr, scinitialized) && validAddress(seciniaddr, scinitialized)) {
			initialized->add(secaddr, seciniaddr);
		}
		delete secaddr;
		delete secend_addr;
		delete seciniaddr;
		s++;
	}

	// entrypoints

	int entrypoint_count = ne_shared->entrypoints->count();
	int *entropy = random_permutation(entrypoint_count);
	for (int i=0; i<entrypoint_count; i++) {
		ht_ne_entrypoint *f = (ht_ne_entrypoint*)ne_shared->entrypoints->get(*(entropy+i));
		if (f) {
			Address *address = createAddress1616(f->seg, f->offset);
			if (validAddress(address, scvalid)) {
				char *label;
				if (f->name) {
					sprintf(buffer, "; exported function %s, ordinal %04x", f->name, f->ordinal);
				} else {
					sprintf(buffer, "; unnamed exported function, ordinal %04x", f->ordinal);
				}
				label = export_func_name(f->name, f->ordinal);
				addComment(address, 0, "");
				addComment(address, 0, ";********************************************************");
				addComment(address, 0, buffer);
				addComment(address, 0, ";********************************************************");
				pushAddress(address, address);
				assignSymbol(address, label, label_func);
				free(label);
			}
			delete address;
		}
	}
	if (entropy) free(entropy);

	// imports

	if (ne_shared->imports) {
		ht_tree *t = ne_shared->imports;
		ht_data *v;
		ne_import_rec *imp = NULL;
		FILEOFS h = ne_shared->hdr_ofs + ne_shared->hdr.imptab;
		while ((imp = (ne_import_rec*)t->enum_next(&v, imp))) {
			char *name = NULL;
			char *mod = (imp->module-1 < ne_shared->modnames_count) ? ne_shared->modnames[imp->module-1] : (char*)"invalid!";
			if (imp->byname) {
				file->seek(h+imp->name_ofs);
				name = getstrp(file);
			}
			char *label = import_func_name(mod, name, imp->byname ? 0 : imp->ord);
			if (name) free(name);
			Address *addr = createAddress1616(ne_shared->fake_segment+1, imp->addr);
			addComment(addr, 0, "");
			assignSymbol(addr, label, label_func);
			data->setIntAddressType(addr, dst_ibyte, 1);
			free(label);
			delete addr;
		}
	}

/*	virtual ht_data *enum_next(ht_data **value, ht_data *prevkey);
	int import_count = ne_shared->imports.funcs->count();
	for (int i=0; i<import_count; i++) {
		ht_pe_import_function *f=(ht_pe_import_function *)pe_shared->imports.funcs->get(*(entropy+i));
		ht_pe_import_library *d=(ht_pe_import_library *)pe_shared->imports.libs->get(f->libidx);
		char *label;
		label = import_func_name(d->name, (f->byname) ? f->name.name : NULL, f->ordinal);
		addComment(f->address, 0, "");
		assignSymbol(f->address, label, label_func);
		data->set_int_addr_type(f->address, dst_idword, 4);
		free(label);
	}*/

	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	if (ne_shared->hdr.flags & NE_FLAGS_NOTAPROCESS) {
		addComment(entry, 0, ";  library entry point");
	} else {
		addComment(entry, 0, ";  program entry point");
	}
	addComment(entry, 0, ";****************************");
	if (validCodeAddress(entry)) {
		assignSymbol(entry, "entrypoint", label_func);
	} else {
		assignSymbol(entry, "entrypoint", label_data);
	}

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	delete entry;
	
	Analyser::beginAnalysis();
}

/*
 *
 */
OBJECT_ID	NEAnalyser::object_id()
{
	return ATOM_NE_ANALYSER;
}

/*
 *
 */
UINT NEAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FILEOFS ofs = addressToFileofs(Addr);
/*     if (ofs == INVALID_FILE_OFS) {
		int as=1;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool NEAnalyser::convertAddressToNEAddress(Address *addr, NEAddress *r)
{
	if (addr->object_id()==ATOM_ADDRESS_X86_1616) {
		*r = NE_MAKE_ADDR(((AddressX86_1616*)addr)->seg, ((AddressX86_1616*)addr)->addr);
		return true;
	} else {
		return false;
	}
}

Address *NEAnalyser::createAddress()
{
	return new AddressX86_1616(0, 0);
}

Address *NEAnalyser::createAddress1616(word seg, word ofs)
{
	return new AddressX86_1616(seg, ofs);
}

/*
 *
 */
Assembler *NEAnalyser::createAssembler()
{
/* FIXME: 16/32 */
	Assembler *a = new x86asm(X86_OPSIZE16, X86_ADDRSIZE16);
	a->init();
	return a;
}

/*
 *
 */
FILEOFS NEAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FILEOFS ofs;
		NEAddress na;
		if (!convertAddressToNEAddress(Addr, &na)) return INVALID_FILE_OFS;
		if (!NE_addr_to_ofs(ne_shared, na, &ofs)) {
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
char *NEAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char segmentname[16];
	int i;
	NEAddress na;
	if (!convertAddressToNEAddress(Addr, &na)) return NULL;
	if (!NE_addr_to_segment(ne_shared, na, &i)) return NULL;
	if (i == (int)ne_shared->fake_segment) {
		strcpy(segmentname, "faked names");
	} else {
		sprintf(segmentname, "seg%d", i+1);
	}
	return segmentname;
}

/*
 *
 */
char	*NEAnalyser::getName()
{
	return file->get_desc();
}

/*
 *
 */
char *NEAnalyser::getType()
{
	return "NE/Analyser";
}

/*
 *
 */
void NEAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void NEAnalyser::initUnasm()
{
	DPRINTF("ne_analy: ");
	DPRINTF("initing analy_x86_disassembler\n");
	analy_disasm = new AnalyX86Disassembler();
	((AnalyX86Disassembler*)analy_disasm)->init(this, ANALYX86DISASSEMBLER_FLAGS_16BIT | ANALYX86DISASSEMBLER_FLAGS_SEGMENTED);
}

/*
 *
 */
void NEAnalyser::log(const char *msg)
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
Address *NEAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void NEAnalyser::store(ht_object_stream *st)
{
	PUT_OBJECT(st, validarea);
	Analyser::store(st);
}

/*
 *
 */
int	NEAnalyser::queryConfig(int mode)
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
Address *NEAnalyser::fileofsToAddress(FILEOFS fileofs)
{
	NEAddress a;
	if (NE_ofs_to_addr(ne_shared, fileofs, &a)) {
		return createAddress1616(NE_ADDR_SEG(a), NE_ADDR_OFS(a));
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool NEAnalyser::validAddress(Address *Addr, tsectype action)
{
	ne_segment_headers *segments = &ne_shared->segments;
	int sec;
	NEAddress na;
	if (!convertAddressToNEAddress(Addr, &na)) return false;
	if (!NE_addr_to_segment(ne_shared, na, &sec)) return false;
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
			return (!(s->flags & NE_DATA)) && NE_addr_is_physical(ne_shared, na);
		case scinitialized:
			return NE_addr_is_physical(ne_shared, na);
	}
	return false;
}

