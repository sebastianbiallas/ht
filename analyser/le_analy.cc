/*
 *	HT Editor
 *	le_analy.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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
#include "htanaly.h"		// FIXME: for ht_aviewer, to call gotoAddress(entrypoint)
#include "le_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "endianess.h"
#include "htiobox.h"
#include "htle.h"
#include "strtools.h"
#include "nestruct.h"
#include "snprintf.h"
#include "x86asm.h"

/*
 *
 */
void	LEAnalyser::init(ht_le_shared_data *LE_shared, File *File)
{
	le_shared = LE_shared;
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
void	LEAnalyser::load(ObjectStream &f)
{
	/*
	ht_pe_shared_data 	*pe_shared;
	ht_stream 		*file;
	area				*validarea;
	*/
	GET_OBJECT(f, validarea);
	Analyser::load(f);
}

/*
 *
 */
void	LEAnalyser::done()
{
	validarea->done();
	delete validarea;
	Analyser::done();
}

/*
 *
 */
void LEAnalyser::beginAnalysis()
{
//	char	buffer[1024];

	/*
	 *	entrypoint
	 */

	LEAddress a;
	Address *control = NULL;
	Address *v86control = NULL;
	Address *pmcontrol = NULL;
	if (le_shared->is_vxd) {
		LEAddress addr;
		int temp;

		addr = le_shared->vxd_desc.v86_ctrl_ofs;
		if (LE_addr_to_segment(le_shared, addr, &temp)) {
			a = LE_MAKE_ADDR(le_shared, LE_ADDR_SEG(le_shared, addr),
				LE_ADDR_OFS(le_shared, addr));
			v86control = createAddressFlat32(a);
			le_shared->best_entrypoint = a;
		}

		addr = le_shared->vxd_desc.pm_ctrl_ofs;
		if (LE_addr_to_segment(le_shared, addr, &temp)) {
			a = LE_MAKE_ADDR(le_shared, LE_ADDR_SEG(le_shared, addr),
				LE_ADDR_OFS(le_shared, addr));
			pmcontrol = createAddressFlat32(a);
			le_shared->best_entrypoint = a;
		}
		
		addr = le_shared->vxd_desc.ctrl_ofs;
		if (LE_addr_to_segment(le_shared, addr, &temp)) {
			a = LE_MAKE_ADDR(le_shared, LE_ADDR_SEG(le_shared, addr),
				LE_ADDR_OFS(le_shared, addr));
			control = createAddressFlat32(a);
			le_shared->best_entrypoint = a;
		}
	}

	Address *entry = NULL;
	if (le_shared->hdr.startobj != 0) {
		a = LE_MAKE_ADDR(le_shared, le_shared->hdr.startobj-1, le_shared->hdr.eip);
		le_shared->best_entrypoint = a;
		entry = createAddressFlat32(a);
	}

	if (v86control) pushAddress(v86control, v86control);

	if (pmcontrol) pushAddress(pmcontrol, pmcontrol);

	if (control) pushAddress(control, control);

	if (entry) pushAddress(entry, entry);

	/*
	 * give all sections a descriptive comment:
	 */

	LE_OBJECT *s = le_shared->objmap.header;
	char blub[100];
	for (uint i = 0; i < le_shared->objmap.count; i++) {
		LEAddress la = LE_get_seg_addr(le_shared, i);
		Address *secaddr = createAddressFlat32(la);

//		uint psize = LE_get_seg_psize(le_shared, i);
		uint vsize = LE_get_seg_vsize(le_shared, i);

		sprintf(blub, ";  section %d <%s> USE%d", i+1, getSegmentNameByAddress(secaddr), (le_shared->objmap.header[i].flags & LE_OBJECT_FLAG_USE32) ? 32 : 16);
		addComment(secaddr, 0, "");
		addComment(secaddr, 0, ";******************************************************************");
		addComment(secaddr, 0, blub);
		sprintf(blub, ";  virtual address  %08x  virtual size   %08x", LE_get_seg_addr(le_shared, i), vsize);
		addComment(secaddr, 0, blub);
/*		sprintf(blub, ";  file offset      %08x  file size      %08x", psize);
		addComment(secaddr, 0, blub);*/
		addComment(secaddr, 0, ";******************************************************************");

		// mark end of sections
		sprintf(blub, ";  end of section <%s>", getSegmentNameByAddress(secaddr));
		Address *secend_addr = secaddr->clone();
		secend_addr->add(vsize);
		newLocation(secend_addr)->flags |= AF_FUNCTION_END;
		addComment(secend_addr, 0, "");
		addComment(secend_addr, 0, ";******************************************************************");
		addComment(secend_addr, 0, blub);
		addComment(secend_addr, 0, ";******************************************************************");

		validarea->add(secaddr, secend_addr);
		Address *seciniaddr = secaddr->clone();
		seciniaddr->add(vsize-1);
		if (validAddress(secaddr, scinitialized) && validAddress(seciniaddr, scinitialized)) {
			initialized->add(secaddr, seciniaddr);
		}
		delete secaddr;
		delete secend_addr;
		delete seciniaddr;
		s++;
	}

	// entrypoints
/*
	int entrypoint_count = le_shared->entrypoints->count();
	int *entropy = random_permutation(entrypoint_count);
	for (int i=0; i<entrypoint_count; i++) {
		ht_ne_entrypoint *f = (ht_ne_entrypoint*)le_shared->entrypoints->get(*(entropy+i));
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
*/
	// imports
/*
	if (le_shared->imports) {
		ht_tree *t = le_shared->imports;
		Object *v;
		ne_import_rec *imp = NULL;
		FileOfs h = le_shared->hdr_ofs + le_shared->hdr.imptab;
		while ((imp = (ne_import_rec*)t->enum_next(&v, imp))) {
			char *name = NULL;
			char *mod = (imp->module-1 < le_shared->modnames_count) ? le_shared->modnames[imp->module-1] : (char*)"invalid!";
			if (imp->byname) {
				file->seek(h+imp->name_ofs);
				name = getstrp(file);
			}
			char *label = import_func_name(mod, name, imp->byname ? 0 : imp->ord);
			if (name) free(name);
			Address *addr = createAddress1616(le_shared->fake_segment+1, imp->addr);
			addComment(addr, 0, "");
			assignSymbol(addr, label, label_func);
			data->setIntAddressType(addr, dst_ibyte, 1);
			free(label);
			delete addr;
		}
	}
*/

/*	virtual Object *enum_next(ht_data **value, Object *prevkey);
	int import_count = le_shared->imports.funcs->count();
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

	if (le_shared->is_vxd) {
		if (v86control) {
			addComment(v86control, 0, "");
			addComment(v86control, 0, ";****************************");
			addComment(v86control, 0, ";  VxD V86-control procedure");
			addComment(v86control, 0, ";****************************");
			assignSymbol(v86control, "VxD_v86_control", label_func);
		}
		if (pmcontrol) {
			addComment(pmcontrol, 0, "");
			addComment(pmcontrol, 0, ";****************************");
			addComment(pmcontrol, 0, ";  VxD PM-control procedure");
			addComment(pmcontrol, 0, ";****************************");
			assignSymbol(pmcontrol, "VxD_pm_control", label_func);
		}
		if (control) {
			addComment(control, 0, "");
			addComment(control, 0, ";****************************");
			addComment(control, 0, ";  VxD control procedure");
			addComment(control, 0, ";****************************");
			assignSymbol(control, "VxD_control", label_func);
		}
	}

	if (entry) {
		addComment(entry, 0, "");
		addComment(entry, 0, ";****************************");
		addComment(entry, 0, ";  program entry point");
		addComment(entry, 0, ";****************************");
		if (validCodeAddress(entry)) {
			assignSymbol(entry, "entrypoint", label_func);
		} else {
			assignSymbol(entry, "entrypoint", label_data);
		}
	}

	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	delete entry;

	if (le_shared->best_entrypoint != LE_ADDR_INVALID) {
		Address *tmpaddr = createAddressFlat32(le_shared->best_entrypoint);
		((ht_aviewer*)le_shared->v_image)->gotoAddress(tmpaddr, NULL);
		delete tmpaddr;
	}

	Analyser::beginAnalysis();
}

/*
 *
 */
ObjectID	LEAnalyser::getObjectID() const
{
	return ATOM_LE_ANALYSER;
}

/*
 *
 */
FileOfs LEAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FileOfs ofs;
		LEAddress na;
		if (!convertAddressToLEAddress(Addr, &na)) return INVALID_FILE_OFS;
		if (!LE_addr_to_ofs(le_shared, na, &ofs)) {
			return INVALID_FILE_OFS;
		}
		return ofs;
/*          uint m;
		FileOfs oo;
		if (!le_shared->linear_file->map_ofs(ofs, &oo, &m)) {
			le_shared->linear_file->map_ofs(ofs, &oo, &m);
			return INVALID_FILE_OFS;
		}
		return oo;*/
	} else {
		return INVALID_FILE_OFS;
	}
}

FileOfs LEAnalyser::addressToRealFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		FileOfs ofs;
		LEAddress na;
		if (!convertAddressToLEAddress(Addr, &na)) return INVALID_FILE_OFS;
		if (!LE_addr_to_ofs(le_shared, na, &ofs)) {
			return INVALID_FILE_OFS;
		}
		FileOfs m;
		FileOfs oo;
		if (!le_shared->linear_file->map_ofs(ofs, &oo, &m)) {
			return INVALID_FILE_OFS;
		}
		return oo;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
uint LEAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
/*     if (ofs == INVALID_FILE_OFS) {
		ht_printf("%y", Addr);
		int as=1;
	}*/
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

bool LEAnalyser::convertAddressToLEAddress(Address *addr, LEAddress *r)
{
	if (addr->getObjectID()==ATOM_ADDRESS_X86_FLAT_32) {
//		*r = LE_MAKE_ADDR(le_shared, ((AddressX86Flat32*)addr)->seg, ((AddressX86_1632*)addr)->addr);
		*r = ((AddressX86Flat32*)addr)->addr;
		return true;
	} else {
		return false;
	}
}

Address *LEAnalyser::createAddress()
{
	return new AddressX86Flat32(0);
}

Address *LEAnalyser::createAddressFlat32(uint32 ofs)
{
	return new AddressX86Flat32(ofs);
}

/*
 *
 */
Assembler *LEAnalyser::createAssembler()
{
/* FIXME: 16/32 */
	Assembler *a = new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
	a->init();
	return a;
}

/*
 *
 */
const char *LEAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char segmentname[16];
	int i;
	LEAddress na;
	if (!convertAddressToLEAddress(Addr, &na)) return NULL;
	if (!LE_addr_to_segment(le_shared, na, &i)) return NULL;
//	LEAddress temp;
/*	bool init = LE_addr_to_ofs(le_shared, na, &temp);
	if (!init)
		return NULL;*/
/*	if (i == (int)le_shared->fake_segment) {
		strcpy(segmentname, "faked names");
	} else {*/
		sprintf(segmentname, "seg%d", i+1);
//	}
	return segmentname;
}

/*
 *
 */
String &LEAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *LEAnalyser::getType()
{
	return "LE/Analyser";
}

/*
 *
 */
void LEAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}

/*
 *
 */
void LEAnalyser::initUnasm()
{
	DPRINTF("le_analy: ");
	DPRINTF("initing analy_x86_disassembler\n");
	analy_disasm = new AnalyX86Disassembler();
	((AnalyX86Disassembler*)analy_disasm)->init(this, le_shared->is_vxd ? ANALYX86DISASSEMBLER_FLAGS_VXD_X86DIS : 0);
}

/*
 *
 */
void LEAnalyser::log(const char *msg)
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
Address *LEAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void LEAnalyser::store(ObjectStream &st) const
{
	PUT_OBJECT(st, validarea);
	Analyser::store(st);
}

/*
 *
 */
int	LEAnalyser::queryConfig(int mode)
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
Address *LEAnalyser::fileofsToAddress(FileOfs fileofs)
{
	LEAddress a;
	if (LE_ofs_to_addr(le_shared, fileofs, &a)) {
		return createAddressFlat32(a);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */

/* FIXME: fileofs chaos */
Address *LEAnalyser::realFileofsToAddress(FileOfs fileofs)
{
	LEAddress a;
	uint lofs;
	if (le_shared->linear_file->unmap_ofs(fileofs, &lofs) &&
	LE_ofs_to_addr(le_shared, lofs, &a)) {
		return createAddressFlat32(a);
	} else {
		return new InvalidAddress();
	}
}

/*
 *
 */
bool LEAnalyser::validAddress(Address *Addr, tsectype action)
{
	ht_le_objmap *objects = &le_shared->objmap;
	int sec;
	LEAddress na;
	if (!convertAddressToLEAddress(Addr, &na)) return false;
	if (!LE_addr_to_segment(le_shared, na, &sec)) return false;
	FileOfs temp;
	bool init = LE_addr_to_ofs(le_shared, na, &temp);
	LE_OBJECT *s = objects->header + sec;

	switch (action) {
	case scvalid:
		return true;
	case scread:
		return (s->flags & LE_OBJECT_FLAG_READABLE);
	case scwrite:
		return (s->flags & LE_OBJECT_FLAG_WRITEABLE);
	case screadwrite:
		return (s->flags & LE_OBJECT_FLAG_READABLE) && (s->flags & LE_OBJECT_FLAG_WRITEABLE);
	case sccode:
		return (s->flags & LE_OBJECT_FLAG_EXECUTABLE) && init;
	case scinitialized:
		return init;
	}
	return false;
}

