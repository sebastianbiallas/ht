/*
 *	HT Editor
 *	xbe_analy.cc
 *
 *	Copyright (C) 2003 Stefan Esser
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
#include "analy_ia64.h"
#include "analy_il.h"
#include "analy_names.h"
#include "analy_register.h"
#include "analy_ppc.h"
#include "analy_x86.h"
#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htxbeimp.h"
#include "htxbe.h"
#include "strtools.h"
#include "ilopc.h"
#include "xbe_analy.h"
#include "xbestruct.h"
#include "snprintf.h"
#include "x86asm.h"

/*
 *
 */
void	XBEAnalyser::init(ht_xbe_shared_data *XBE_shared, File *File)
{
	xbe_shared = XBE_shared;
	file = File;

	validarea = new Area();
	validarea->init();

	Analyser::init();
}


/*
 *
 */
void	XBEAnalyser::load(ObjectStream &f)
{
	GET_OBJECT(f, validarea);
	Analyser::load(f);
}

/*
 *
 */
void	XBEAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

/*
 *
 */
void XBEAnalyser::beginAnalysis()
{
	char	buffer[1024];

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);

	/*
	 *	entrypoint
	 */
	Address *entry;

	entry = createAddress32(xbe_shared->header.entry_point);
	pushAddress(entry, entry);
	
	/*
	 * give all sections a descriptive comment:
	 */

	XBE_SECTION_HEADER *s=xbe_shared->sections.sections;
	char blub[100];
	for (uint i=0; i<xbe_shared->sections.number_of_sections; i++) {
		Address *secaddr;

		secaddr = createAddress32(s->virtual_address);

		ht_snprintf(blub, sizeof blub, ";  TEST section %d <%s>", i+1, getSegmentNameByAddress(secaddr));
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, blub);
		ht_snprintf(blub, sizeof blub, ";  virtual address  %08x  virtual size   %08x", s->virtual_address, s->virtual_size);
		addComment(secaddr, 0, blub);
		ht_snprintf(blub, sizeof blub, ";  file offset      %08x  file size      %08x", s->raw_address, s->raw_size);
		addComment(secaddr, 0, blub);
		addComment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		ht_snprintf(blub, sizeof blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
		Address *secend_addr = secaddr->clone();
		secend_addr->add(MAX(s->virtual_size, s->raw_size));
		newLocation(secend_addr)->flags |= AF_FUNCTION_END;
		addComment(secend_addr, 0, "");
		addComment(secend_addr, 0, ";******************************************************************");
		addComment(secend_addr, 0, blub);
		addComment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		Address *seciniaddr = secaddr->clone();
		seciniaddr->add(MIN(s->virtual_size, s->raw_size));
		if (validAddress(secaddr, scinitialized) && validAddress(seciniaddr, scinitialized)) {
			initialized->add(secaddr, seciniaddr);
		}
		s++;
		delete secaddr;
		delete secend_addr;
		delete seciniaddr;
	}

	int import_count=xbe_shared->imports.funcs->count();
	int *entropy = random_permutation(import_count);
	for (int i = 0; i < import_count; i++) {
		ht_xbe_import_function *f=(ht_xbe_import_function *) (*xbe_shared->imports.funcs)[entropy[i]];
//		ht_pe_import_library *d=(ht_pe_import_library *)(*pe_shared->imports.libs)[f->libidx];
		char *label;
		label = import_func_name("NTOSKRNL.EXE", (f->byname) ? f->name.name : NULL, f->ordinal);
		Address *faddr;

		faddr = createAddress32(f->address+xbe_shared->header.base_address);

		addComment(faddr, 0, "");
		if (!assignSymbol(faddr, label, label_func)) {
			// multiple import of a function (duplicate labelname)
			// -> mangle name a bit more
			addComment(faddr, 0, "; duplicate import");               
			ht_snprintf(buffer, sizeof buffer, "%s_%x", label, f->address);
			assignSymbol(faddr, buffer, label_func);
		}
		data->setIntAddressType(faddr, dst_idword, 4);
		free(label);
		delete faddr;
	}
	if (entropy) free(entropy);


	addComment(entry, 0, "");
	addComment(entry, 0, ";****************************");
	addComment(entry, 0, ";  program entry point");
	addComment(entry, 0, ";****************************");
	assignSymbol(entry, "entrypoint", label_func);

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	delete entry;

	Address *tls;
	tls = createAddress32(xbe_shared->header.tls_address+xbe_shared->header.base_address+0);
	assignSymbol(tls, "tls.data_start_address", label_data);
	free(tls);
	tls = createAddress32(xbe_shared->header.tls_address+xbe_shared->header.base_address+4);
	assignSymbol(tls, "tls.data_end_address", label_data);
	free(tls);
	tls = createAddress32(xbe_shared->header.tls_address+xbe_shared->header.base_address+8);
	assignSymbol(tls, "tls.index_address", label_data);
	free(tls);
	tls = createAddress32(xbe_shared->header.tls_address+xbe_shared->header.base_address+12);
	assignSymbol(tls, "tls.callback_address", label_data);
	free(tls);
	tls = createAddress32(xbe_shared->header.tls_address+xbe_shared->header.base_address+16);
	assignSymbol(tls, "tls.size_of_zero_fill", label_data);
	free(tls);
	tls = createAddress32(xbe_shared->header.tls_address+xbe_shared->header.base_address+20);
	assignSymbol(tls, "tls.characteristics", label_data);
	free(tls);

	Analyser::beginAnalysis();
}

/*
 *
 */
ObjectID	XBEAnalyser::getObjectID() const
{
	return ATOM_XBE_ANALYSER;
}

/*
 *
 */
uint XBEAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
/*	if (ofs == INVALID_FILE_OFS) {
		int as=0;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool XBEAnalyser::convertAddressToRVA(Address *addr, RVA *r)
{
	ObjectID oid = addr->getObjectID();
	if (oid==ATOM_ADDRESS_FLAT_32) {
		*r = ((AddressFlat32*)addr)->addr - xbe_shared->header.base_address;
		return true;
	} else if (oid == ATOM_ADDRESS_X86_FLAT_32) {
		*r = ((AddressX86Flat32*)addr)->addr - xbe_shared->header.base_address;
		return true;
	}
//	*r = ((AddressFlat32*)addr)->addr - xbe_shared->header.base_address;
//	*r = ((AddressX86Flat32*)addr)->addr - xbe_shared->header.base_address;
	return false;
}

/*
 *
 */
Address *XBEAnalyser::createAddress32(uint32 addr)
{
	return new AddressX86Flat32(addr);
//	return new AddressFlat32(addr);
}

/*
 *
 */
Address *XBEAnalyser::createAddress64(uint64 addr)
{
	return NULL;
}

Address *XBEAnalyser::createAddress()
{
	return new AddressX86Flat32();
//	return new AddressFlat32();
}

/*
 *
 */
Assembler *XBEAnalyser::createAssembler()
{
	Assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
	a->init();
	return a;
}

/*
 *
 */
FileOfs XBEAnalyser::addressToFileofs(Address *Addr)
{
/*     char tbuf[1024];
	Addr->stringify(tbuf, 1024, 0);
	printf("ADDR=%s", tbuf);*/
	if (validAddress(Addr, scinitialized)) {
//     	printf(" v1\n");
		FileOfs ofs;
		RVA r;
		if (!convertAddressToRVA(Addr, &r)) return INVALID_FILE_OFS;
		if (!xbe_rva_to_ofs(&xbe_shared->sections, r, &ofs)) return INVALID_FILE_OFS;
		return ofs;
	} else {
//     	printf(" IV1\n");
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
const char *XBEAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char sectionname[9];
	xbe_section_headers *sections=&xbe_shared->sections;
	int i;
	RVA r;
	bool b;

//	Addr-=pe_shared->pe32.header_nt.image_base;
	if (!convertAddressToRVA(Addr, &r)) return NULL;
	
//	{ FILE *f;f=fopen("/tmp/rva","a+");if (f){fprintf(f,"rva: %08x\n",r);fclose(f);} }
	
	xbe_rva_to_section(sections, r, &i);
//	XBE_SECTION_HEADER *s=sections->sections+i;
	b = xbe_rva_is_valid(sections, r);
//	{ FILE *f;f=fopen("/tmp/rva","a+");if (f){fprintf(f,"rva: %08x %u\n",r,(uint)b);fclose(f);} }
	
	if (!b) return NULL;

	strcpy(sectionname, "<notyet>");
	return sectionname;
}

/*
 *
 */
String &XBEAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *XBEAnalyser::getType()
{
	return "XBE/Analyser";
}

/*
 *
 */
void XBEAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}
/*
static char *string_func(uint32 ofs, void *context)
{
	char str[1024];
	static char str2[1024];
	ht_pe_shared_data *pe = (ht_pe_shared_data*)context;
	if (ofs < pe->il->string_pool_size) {
		uint32 length;
		uint32 o = ILunpackDword(length, (byte*)&pe->il->string_pool[ofs], 10);
		wide_char_to_multi_byte(str, (byte*)&pe->il->string_pool[ofs+o], length/2+1);
		escape_special_str(str2, sizeof str2, str, "\"");
		return str2;
	} else {
		return NULL;
	}
}

static char *token_func(uint32 token, void *context)
{
	static char tokenstr[1024];
	switch (token & IL_META_TOKEN_MASK) {
		case IL_META_TOKEN_TYPE_REF:
		case IL_META_TOKEN_TYPE_DEF: {
			sprintf(tokenstr, "typedef");
			break;
		}
		case IL_META_TOKEN_FIELD_DEF: {
			sprintf(tokenstr, "fielddef");
			break;
		}
		case IL_META_TOKEN_METHOD_DEF: {
			sprintf(tokenstr, "methoddef");
			break;
		}
		case IL_META_TOKEN_MEMBER_REF: {
			sprintf(tokenstr, "memberref");
			break;
		}
		case IL_META_TOKEN_TYPE_SPEC: {
			sprintf(tokenstr, "typespec");
			break;
		}
		default:
			return NULL;
	}
	return tokenstr;
}
*/

/*
 *
 */
void XBEAnalyser::initUnasm()
{
	DPRINTF("xbe_analy: ");

	DPRINTF("initing analy_x86_disassembler\n");
	analy_disasm = new AnalyX86Disassembler();
	((AnalyX86Disassembler *)analy_disasm)->init(this, 0);
}

/*
 *
 */
void XBEAnalyser::log(const char *msg)
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
Address *XBEAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void XBEAnalyser::store(ObjectStream &st) const
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	PUT_OBJECT(st, validarea);
	Analyser::store(st);
}

/*
 *
 */
int	XBEAnalyser::queryConfig(int mode)
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
Address *XBEAnalyser::fileofsToAddress(FileOfs fileofs)
{
	RVA r;
	if (xbe_ofs_to_rva(&xbe_shared->sections, fileofs, &r)) {
		return createAddress32(r+xbe_shared->header.base_address);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool XBEAnalyser::validAddress(Address *Addr, tsectype action)
{
	xbe_section_headers *sections=&xbe_shared->sections;
	int sec;
	RVA r;
	if (!convertAddressToRVA(Addr, &r)) return false;
	if (!xbe_rva_to_section(sections, r, &sec)) return false;
	XBE_SECTION_HEADER *s=sections->sections+sec;
	switch (action) {
	case scvalid:
		return true;
	case scread:
		return true;
	case scwrite:
		return s->section_flags & XBE_SECTION_FLAGS_WRITABLE;
	case screadwrite:
		return s->section_flags & XBE_SECTION_FLAGS_WRITABLE;
	case sccode:
		if (!xbe_rva_is_physical(sections, r)) return false;
		return s->section_flags & XBE_SECTION_FLAGS_EXECUTABLE;
	case scinitialized:
		if (!xbe_rva_is_physical(sections, r)) return false;
		return true;
	}
	return false;
}
