/*
 *	HT Editor
 *	analy.cc
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "analy.h"
#include "analy_names.h"
#include "analy_register.h"
#include "code_analy.h"
#include "data_analy.h"
#include "io/types.h"
#include "atom.h"
#include "except.h"
#include "htctrl.h"
#include "htdebug.h"
#include "strtools.h"
#include "language.h"
#include "snprintf.h"
#include "tools.h"

#define DPRINTF(msg...)
//#undef DPRINTF
//#define DPRINTF(msg...) {global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;char buf[1024]; ht_snprintf(buf, sizeof buf, ##msg); fprintf(stdout, buf);}

int global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT;

int Address::compareDelinear(Address *obj)
{
	return compareTo(obj);
}

bool Address::isValid()
{
	return true;
}

int Address::toString(char *s, int maxlen) const
{
	return stringify(s, maxlen, global_analyser_address_string_format);
}

bool InvalidAddress::add(int offset)
{
	return false;
}

int InvalidAddress::byteSize()
{
	return 0;
}

int InvalidAddress::compareTo(const Object *obj) const
{
	return 0;
}

void InvalidAddress::putIntoCPUAddress(CPU_ADDR *ca) const
{
}

bool InvalidAddress::difference(int &result, Address *to)
{
	return false;
}

InvalidAddress *InvalidAddress::clone() const
{
	return new InvalidAddress(*this);
}

void InvalidAddress::getFromArray(const byte *array)
{
}

void InvalidAddress::getFromCPUAddress(CPU_ADDR *ca)
{
}

bool InvalidAddress::getFromUInt64(uint64 u)
{
	return false;
}

bool InvalidAddress::isValid()
{
	return false;
}

int InvalidAddress::parseString(const char *s, int length, Analyser *a)
{
	return 0;
}

ObjectID InvalidAddress::getObjectID() const
{
	return ATOM_ADDRESS_INVALID;
}

void InvalidAddress::putIntoArray(byte *array) const
{
}

bool InvalidAddress::putIntoUInt64(uint64 &u) const
{
	return false;
}

int InvalidAddress::stringify(char *s, int max_length, int format) const
{
	return ht_snprintf(s, max_length, "*INVALID");
}

int InvalidAddress::stringSize() const
{
	return 8;
}

/*
 *
 */
bool AddressFlat32::add(int offset)
{
	// check for overflow
	if ((int)offset < 0) {
		if (addr+offset > addr) return false;
	} else {
		if (addr+offset < addr) return false;
	}
	addr+=offset;
	return true;
}

int AddressFlat32::byteSize()
{
	return 4;
}

int AddressFlat32::compareTo(const Object *obj) const
{
	assert(getObjectID() == obj->getObjectID());
	if (addr > ((AddressFlat32 *)obj)->addr) return 1;
	if (addr < ((AddressFlat32 *)obj)->addr) return -1;
	return 0;
}

int AddressFlat32::compareDelinear(Address *to)
{
	assert(getObjectID() == to->getObjectID());
	uint32 da = delinearize(addr);
	uint32 db = delinearize(((AddressFlat32 *)to)->addr);
	if (da > db) return 1;
	if (da < db) return -1;
	return 0;
}

bool AddressFlat32::difference(int &result, Address *to)
{
	if (getObjectID() == to->getObjectID()) {
		result = addr - ((AddressFlat32 *)to)->addr;
		return true;
	} else {
		return false;
	}
}

AddressFlat32 *AddressFlat32::clone() const
{
	return new AddressFlat32(*this);
}

void AddressFlat32::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(uint32*)array);
}

void AddressFlat32::getFromCPUAddress(CPU_ADDR *ca)
{
	addr = ca->addr32.offset;
}

bool AddressFlat32::getFromUInt64(uint64 u)
{
	if (u <= 0xffffffff) {
		addr = u;
		return true;
	} else {
		return false;
	}
}

void AddressFlat32::load(ObjectStream &st)
{
	GET_INT32X(st, addr);
}

ObjectID AddressFlat32::getObjectID() const
{
	return ATOM_ADDRESS_FLAT_32;
}

int AddressFlat32::parseString(const char *s, int length, Analyser *a)
{
	return false;
}

void AddressFlat32::putIntoArray(byte *array) const
{
	UNALIGNED_MOVE(*(uint32*)array, addr);
}

void AddressFlat32::putIntoCPUAddress(CPU_ADDR *ca) const
{
	ca->addr32.offset = addr;
}

bool AddressFlat32::putIntoUInt64(uint64 &u) const
{
	u = addr;
	return true;
}

void AddressFlat32::store(ObjectStream &st) const
{
	PUT_INT32X(st, addr);
}

int AddressFlat32::stringify(char *s, int max_length, int format) const
{
	const char *formats[] = {
		"%s%x%s",
		"%s%8x%s",
		"%s%08x%s",
		"",
		"%s%X%s",
		"%s%8X%s",
		"%s%08X%s",
		"",
	};
	return ht_snprintf(s, max_length, formats[format&7], (format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", addr, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"");
}

int AddressFlat32::stringSize() const
{
	return 8;
}

/*
 *
 */
bool AddressFlat64::add(int offset)
{
	// check for overflow
	// FIXME: XXXXXXXXXXXXX
	addr += sint64(offset);
	return true;
}

int AddressFlat64::byteSize()
{
	return 8;
}

int AddressFlat64::compareTo(const Object *obj) const
{
	assert(getObjectID() == obj->getObjectID());
	if (addr > ((AddressFlat64 *)obj)->addr) return 1;
	if (addr < ((AddressFlat64 *)obj)->addr) return -1;
	return 0;
}

int AddressFlat64::compareDelinear(Address *to)
{
	assert(getObjectID() == to->getObjectID());
	uint64 da1, da2;
	da1 = delinearize64(addr);
	da2 = delinearize64(((AddressFlat64 *)to)->addr);
	if (da1 > da2) return 1;
	if (da1 < da2) return -1;
	return 0;
}

bool AddressFlat64::difference(int &result, Address *to)
{
	if (getObjectID() == to->getObjectID()) {
		uint64 res = addr - ((AddressFlat64 *)to)->addr;
		uint32 rh = res >> 32;
		if (!rh || rh == 0xffffffff) {
			result = res;
			return true;
		}
	}
	return false;
}

AddressFlat64 *AddressFlat64::clone() const
{
	return new AddressFlat64(*this);
}

void AddressFlat64::getFromArray(const byte *array)
{
	UNALIGNED_MOVE(addr, *(uint64*)array);
}

void AddressFlat64::getFromCPUAddress(CPU_ADDR *ca)
{
	addr = ca->flat64.addr;
}

bool AddressFlat64::getFromUInt64(uint64 u)
{
	addr = u;
	return true;
}

void AddressFlat64::load(ObjectStream &st)
{
	GET_INT64X(st, addr);
}

ObjectID AddressFlat64::getObjectID() const
{
	return ATOM_ADDRESS_FLAT_64;
}

int AddressFlat64::parseString(const char *s, int length, Analyser *a)
{
	return false;
}

void AddressFlat64::putIntoArray(byte *array) const
{
	UNALIGNED_MOVE(*(uint64*)array, addr);
}

void AddressFlat64::putIntoCPUAddress(CPU_ADDR *ca) const
{
	ca->flat64.addr = addr;
}

bool AddressFlat64::putIntoUInt64(uint64 &u) const
{
	u = addr;
	return true;
}

void AddressFlat64::store(ObjectStream &st) const
{
	PUT_INT64X(st, addr);
}

int AddressFlat64::stringify(char *s, int max_length, int format) const
{
	const char *formats[] = {
		"%s%qx%s",
		"%s%16qx%s",
		"%s%016qx%s",
		"",
		"%s%qx%s",
		"%s%16qx%s",
		"%s%016qx%s",
		"",
	};
	return ht_snprintf(s, max_length, formats[format&7], (format & ADDRESS_STRING_FORMAT_ADD_0X) ? "0x":"", addr, (format & ADDRESS_STRING_FORMAT_ADD_H) ? "h":"");
}

int AddressFlat64::stringSize() const
{
	return 16;
}


/*
 *
 */
 
AddrXRef::AddrXRef(Address *a, xref_enum_t Type)
{
	addr = a->clone();
	type = Type;
}

AddrXRef::~AddrXRef()
{
	delete addr;
}

void AddrXRef::load(ObjectStream &f)
{
	GET_OBJECT(f, addr);
	type = (xref_enum_t)GETX_INTX(f, 1, "type");
}

ObjectID AddrXRef::getObjectID() const
{
	return ATOM_ADDR_XREF;
}

void AddrXRef::store(ObjectStream &f) const
{
	PUT_OBJECT(f, addr);
	PUT_INTX(f, type, 1);
}

int AddrXRef::compareTo(const Object *o) const
{
	Address *a = ((AddrXRef*)o)->addr;
	return addr->compareTo(a);
}

/*
 *
 */
AddressQueueItem::AddressQueueItem(Address *aAddr, Address *aFunc)
{
	addr = aAddr->clone();
	func = aFunc->clone();
}

AddressQueueItem::~AddressQueueItem()
{
	delete addr;
	delete func;
}

ObjectID AddressQueueItem::getObjectID() const
{
	return ATOM_ADDR_QUEUE_ITEM;
}

void AddressQueueItem::load(ObjectStream &f)
{
	GET_OBJECT(f, addr);
	GET_OBJECT(f, func);
}

void	AddressQueueItem::store(ObjectStream &f) const
{
	PUT_OBJECT(f, addr);
	PUT_OBJECT(f, func);
}

/*
 *
 */
CommentList::CommentList()
	: Array(true)
{
}

void CommentList::appendPreComment(const char *s)
{
	insert(new String(s));
}

void CommentList::appendPostComment(const char *s)
{
	insert(new String(s));
}

void CommentList::appendPreComment(int special)
{
	insert(new UInt(special));
}

void CommentList::appendPostComment(int special)
{
	insert(new UInt(special+0x10000000));
}

const char *CommentList::getName(uint i)
{
	Object *d = get(findByIdx(i));
//	return d ? ((d->getObjectID()==OBJID_UINT) ? comment_lookup(((UInt*)d)->value): 
	return d ? ((String*)d)->contentChar() : NULL;
}

/*
 *
 */
void	Analyser::init()
{
	active = false;
	dirty = false;
	next_address_is_invalid = false;
	addr = new InvalidAddress();
	invalid_addr = new InvalidAddress();
	addr_queue = new Queue(true);
	locations = NULL;
	symbols = NULL;
	symbol_count = location_count = 0;
	cur_addr_ops = cur_label_ops = 1;
	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);
	cur_func = NULL;
	ops_parsed = 0;
	next_explored = new InvalidAddress();
	first_explored = new InvalidAddress();
	last_explored = new InvalidAddress();
	explored = new Area();
	explored->init();
	initialized = new Area();
	initialized->init();
	initCodeAnalyser();
	initDataAnalyser();
	disasm = NULL;
	analy_disasm = NULL;
	max_opcode_length = 1;
	initUnasm();
	mode = ANALY_TRANSLATE_SYMBOLS;
}

static void loadlocations(ObjectStream &st, Location *&loc, int l, int r)
{
	if (l > r) {
		loc = NULL;
		return;
	}
	int m = (l+r)/2;
	loc = new Location;

	loadlocations(st, loc->left, l, m-1);

	loc->addr = GETX_OBJECT(st , "addr");

	// xrefs
	loc->xrefs = GETX_OBJECT(st, "xrefs");
	
	// comments
	loc->comments = GETX_OBJECT(st, "comments");
	
	analyser_get_addrtype(st, &loc->type);

	// must be resolved later (thisfunc is of type Location not Address)
	Address *a = GETX_OBJECT(st, "func");
	loc->thisfunc = (Location *)a;
	loc->flags = GETX_INT(st, 1, "flags");
	loc->label = NULL;

	loadlocations(st, loc->right, m+1, r);
}

static void loadsymbols(Analyser *analy, ObjectStream &st, Symbol *&symbol, int l, int r)
{
	if (l > r) {
		symbol = NULL;
		return;
	}
	int m = (l+r)/2;
	symbol = new Symbol;

	loadsymbols(analy, st, symbol->left, l, m-1);

	Address *a = GETX_OBJECT(st, "addr");

	(symbol->location = analy->newLocation(a))->label = symbol;
	delete a;
	
	symbol->name = st.getString("name");

	symbol->type = (labeltype)GETX_INT(st, 1, "type");

	loadsymbols(analy, st, symbol->right, m+1, r);
}

static void resolveaddrs(Analyser *a, Location *loc)
{
	if (loc) {
		resolveaddrs(a, loc->left);
		Address *tmp = (Address*)loc->thisfunc;
		if (tmp->isValid()) {
			loc->thisfunc = a->getLocationByAddress(tmp);
		} else {
			loc->thisfunc = NULL;
		}
		delete tmp;
		resolveaddrs(a, loc->right);
	}
}

/*
 *
 */
void Analyser::load(ObjectStream &st)
{
	cur_addr_ops = 0;
	cur_label_ops = 0;
	GET_OBJECT(st, addr);
	invalid_addr = new InvalidAddress();
	GET_OBJECT(st, addr_queue);
	GET_INT32D(st, ops_parsed);
	GET_BOOL(st, active);
	if (active) {
		some_analyser_active++;
	}
	next_address_is_invalid = false;
	GET_OBJECT(st, next_explored);
	GET_OBJECT(st, first_explored);
	GET_OBJECT(st, last_explored);

	GET_INT32X(st, mode);

	GET_OBJECT(st, explored);
	GET_OBJECT(st, initialized);

	cur_addr_ops = cur_label_ops = 1;
	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);

	GET_INT32D(st, location_count);
	loadlocations(st, locations, 0, location_count-1);

	resolveaddrs(this, locations);

	GET_INT32D(st, symbol_count);
	loadsymbols(this, st, symbols, 0, symbol_count-1);

	GET_OBJECT(st, analy_disasm);
	GET_OBJECT(st, disasm);     
	if (analy_disasm) {
		analy_disasm->analy = this;
		analy_disasm->disasm = disasm;
	}
	GET_OBJECT(st, code);
	GET_OBJECT(st, data);
	if (data) {
		data->analy = this;
	}
	GET_INT32D(st, location_threshold);
	GET_INT32D(st, symbol_threshold);

	GET_INT32D(st, max_opcode_length);

	Address *curfuncaddr = GETX_OBJECT(st, "cur_func");
	if (curfuncaddr) {
		cur_func = newLocation(curfuncaddr);
		delete curfuncaddr;
	} else {
		cur_func = NULL;
	}
	
	dirty = false;
}

/*
 *
 */
void	Analyser::done()
{
	setActive(false);
	freeLocations(locations);
	freeSymbols(symbols);
	delete addr_queue;
	if (explored) {
		explored->done();
		delete explored;
	}
	if (initialized) {
		initialized->done();
		delete initialized;
	}
	if (code) {
		code->done();
		delete code;
	}
	if (data) {
		data->done();
		delete data;
	}
	if (disasm) {
		disasm->done();
		delete disasm;
	}
	if (analy_disasm) {
		analy_disasm->done();
		delete analy_disasm;
	}
	delete addr;
	delete invalid_addr;
	delete next_explored;
	delete first_explored;
	delete last_explored;
}

/*
 *	addAddressSymbol will never overwrite an existing label (like addlabel)
 */
bool Analyser::addAddressSymbol(Address *address, const char *prefix, labeltype type, Location *infunc)
{
	if (!validAddress(address, scvalid)) return false;

	char symbol[1024];
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT;
	ht_snprintf(symbol, sizeof symbol, "%s_%y", prefix, address);

	if (addSymbol(address, symbol, type, infunc)) {
		return true;
	} else {
		return false;
	}
}

/*
 *
 */
void	Analyser::addComment(Address *Addr, int line, const char *c)
{
	// line 0 meens append (at the moment assume append every time ;-))

//	if (!validaddr(Addr, scvalid)) return;

	Location *a = newLocation(Addr);

	CommentList *com = a->comments;

	if (!com) {
		com = new CommentList();
		com->init();
		a->comments = com;
	}
	com->appendPreComment(c);

	DPRINTF("#(%y) comment `%s'\n", Addr, c);
}

/*
 * addlabel: create label if there isnt one
 *           fail if label exist on another address
 *
 */
bool Analyser::addSymbol(Address *Addr, const char *label, labeltype type, Location *infunc)
{
	if (!validAddress(Addr, scvalid)) return false;

	Location *a = newLocation(Addr);

	if (!a->label) {

		Symbol *l = newSymbol(label, a, type, infunc);

		if (l->location->addr->compareTo(Addr) != 0) {
			// this label already exists at a different address
			return false;
		}

		a->label = l;
		if (a->type.type == dt_unknown || a->type.type == dt_unknown_data) {
			if (type == label_func && !validCodeAddress(Addr)) {
				type = label_unknown;
			}
			switch (type) {
				case label_unknown:
					break;
				case label_func:
					data->setCodeAddressType(a, dst_function);
					break;
				case label_loc:
					data->setCodeAddressType(a, dst_location);
					break;
				case label_data:
					data->setAddressType(a, dt_unknown_data, 0, 0);
					break;
			}
		}
		return true;

	} else {

		// adress already has a label
		return false;
	}
}

/*
 *
 */
bool Analyser::addXRef(Address *from, Address *to, xref_enum_t action)
{
	if (!validAddress(from, scvalid) || !validAddress(to, scvalid)) return false;

	Location *a = newLocation(from);
	Container *x = a->xrefs;

	if (x) {
		AddrXRef tmp(to);
		if (x->find(&tmp)) return false;
	} else {
		x = new AVLTree(true);
		a->xrefs = x;
	}
	x->insert(new AddrXRef(to, action));
	
	DPRINTF("xref %y->%y\n", from, to);
	return true;
}

/*
 *
 */
void	Analyser::assignComment(Address *Addr, int line, const char *c)
{
	/* not really implemented */
	addComment(Addr, line, c);
}

/*
 *
 */
bool Analyser::assignSymbol(Address *Addr, const char *label, labeltype type, Location *infunc)
{
	if (!validAddress(Addr, scvalid)) return false;

	Location *a = newLocation(Addr);

	Symbol *l = newSymbol(label, a, type, infunc);
	if (l->location->addr->compareTo(Addr) != 0) {
		// label already exists at a different address
		return false;
	}

	if (l->location->type.type == dt_unknown) {
		if (type == label_func && !validCodeAddress(Addr)) {
			type = label_unknown;
		}
		switch (type) {
			case label_unknown:
				break;
			case label_func:
				data->setCodeAddressType(a, dst_function);
				break;
			case label_loc:
				data->setCodeAddressType(a, dst_location);
				break;
			case label_data:
				data->setAddressType(a, dt_unknown_data, 0, 0);
				break;
		}
	}
	
	if (a->label) {
		// overwrite
		if (a->label != l) {
			// label has to be renamed
			disableSymbol(a->label);
			a->label = l;
		}
	} else {
		a->label = l;
	}
	return true;
}

/*
 *
 */
void Analyser::assignXRef(Address *from, Address *to, xref_enum_t action)
{
	if (!validAddress(from, scvalid) || !validAddress(to, scvalid)) return;

	Location	*a = newLocation(from);
	Container	*x = a->xrefs;

	if (x) {
		AddrXRef *xref;
		AddrXRef tmp(to);
		if ((xref = (AddrXRef*)x->get(x->find(&tmp)))) {
			// update xref
			xref->type = action;
			DPRINTF("xref %y->%y updated\n", from, to);
			return;
		}
	} else {
		x = new AVLTree(true);
		a->xrefs = x;
	}
	x->insert(new AddrXRef(to, action));
	
	DPRINTF("xref %y->%y\n", from, to);
}

/*
 *
 */
void	Analyser::beginAnalysis()
{
	if (queryConfig(Q_DO_ANALYSIS)) {
		DPRINTF("################################\nAnalysis started.\n");
		if (analy_disasm && disasm) {
			ops_parsed = 0;
			if (gotoAddress(invalid_addr, invalid_addr)) setActive(true);
		} else {
			DPRINTF("Analysis can't be started. No disassembler available.");
		}
	}
}

/*
 *
 */
bool	Analyser::continueAnalysis()
{
	byte			buf[32];
	OPCODE		*instr;
	int			len;
	branch_enum_t	branch;

	assert((uint)max_opcode_length <= sizeof buf);

	if (!active) return	false;
	do {
	
		int diff;
//		char tbuf[100];
//		char tbuf2[100];
//		addr->stringify(tbuf, 100, 0);
/*		if (strcmp(tbuf, "200970ec") == 0) {
			int as=0;
		}*/
//		printf("*** %s ***\n", tbuf);
		if ((addr->difference(diff, next_explored) && (diff >= 0)) || !validCodeAddress(addr)) {
			if (!gotoAddress(addr, invalid_addr)) {
				finish();
				return false;
			}               
		}

		int bz = bufPtr(addr, buf, max_opcode_length);

		instr = disasm->decode(buf, bz, mapAddr(addr));
		instr = disasm->duplicateInsn(instr);
		
		ops_parsed++;
		num_ops_parsed++;

		len = disasm->getSize(instr);
		last_explored->add(len);

		do {
			DPRINTF("opcode @%y [func: %y]: %s\n", addr, (cur_func) ? cur_func->addr : invalid_addr, disasm->str(instr, 0));
			if (disasm->validInsn(instr)) {
				branch = analy_disasm->isBranch(instr);
				if (branch != br_nobranch) {
					doBranch(branch, instr, len);
				} else {
					analy_disasm->examineOpcode(instr);
				}
			} else {
				DPRINTF("invalid opcode @%y\n", addr);
//				log("invalid opcode at address %y\n", addr);
				if (!addr->add(len)) {
					delete addr;
					addr = new InvalidAddress();
				} else {
					newLocation(addr)->flags |= AF_FUNCTION_END;
					next_address_is_invalid = true;
				}
			}
		} while (disasm->selectNext(instr));
		if (next_address_is_invalid || !addr->add(len)) {
			gotoAddress(invalid_addr, invalid_addr);
			next_address_is_invalid = false;
		}
		free(instr);
	} while ((ops_parsed % MAX_OPS_PER_CONTINUE) !=0);
	return true;
}

/*
 *
 */
void Analyser::continueAnalysisAt(Address *Addr)
{
	if (!validAddress(Addr, sccode)) return;
	if (queryConfig(Q_DO_ANALYSIS)) {
		DPRINTF("continueing analysis at %y\n", Addr);
		if (active || disasm) {
			data->setCodeAddressType(Addr, dst_function);
			pushAddress(Addr, Addr);               
		}
		if (!active) {
			if (disasm) {
				Analyser::beginAnalysis();
			} else {
				DPRINTF("couldn't start analysis: no disasm available\n");
			}
		}
	}
}

/*
 *	should return a new instance of an apropriate assembler
 */
Assembler *Analyser::createAssembler()
{
	return NULL;
}

/*
 *
 */
void	Analyser::dataAccess(Address *Addr, taccess access)
{
	if (!validAddress(Addr, scvalid)) {
		char	msg[100];
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
		ht_snprintf(msg, sizeof msg, "access of invalid addr %y at addr %y", Addr, addr);
		log(msg);
		return;
	}

	DPRINTF("dataaccess of %y\n", Addr);

	// string test
	byte buffer[1024];

	if (validAddress(Addr, scinitialized)) {
		uint bz = bufPtr(Addr, buffer, sizeof buffer);
		if (bz > 2) {
			analy_string *str = string_test(buffer, bz);
			if (str) {
				char string1[128], string2[128];
				str->render_string(string2, sizeof string2);
				ht_snprintf(string1, sizeof string1, "%s_%s", str->name(), string2);
				make_valid_name(string2, string1);
				if (addAddressSymbol(Addr, string2, label_data)) {
					addComment(Addr, 0, "");
				}
				data->setArrayAddressType(Addr, dst_string, str->length());
				str->done();
				delete str;
				return;
			}
		}
	}

	if (validCodeAddress(Addr) && access.type == acoffset) {
		if (addAddressSymbol(Addr, LPRFX_OFS, label_func)) {
			addComment(Addr, 0, "");
		}
		Location *a = getLocationByAddress(Addr);
		assert(a);
		// test if Addr points to code
		if ((a->type.type == dt_unknown) || (a->type.type == dt_code)) {
			// test if Addr points to valid code (test not yet complete)
			byte buf[16];
			int bz = bufPtr(Addr, buf, sizeof(buf));
			OPCODE *instr = disasm->decode(buf, MIN(bz, max_opcode_length), mapAddr(Addr));
			if (disasm->validInsn(instr)) {
				data->setCodeAddressType(Addr, dst_cunknown);
				pushAddress(Addr, Addr);
			}
		}
	} else {
		if (access.type != acoffset) {
			switch (access.size) {
				case 1: data->setIntAddressType(Addr, dst_ibyte, 1); break;
				case 2: data->setIntAddressType(Addr, dst_iword, 2); break;
				case 4: data->setIntAddressType(Addr, dst_idword, 4); break;
			}
		}
		if (validAddress(Addr, scinitialized)) {
			addAddressSymbol(Addr, LPRFX_DTA, label_data);
		} else {
			addAddressSymbol(Addr, LPRFX_DTU, label_data);
		}
	}
}

/*
 *	disables address, frees misc
 */
void	Analyser::deleteLocation(Address *Addr)
{
	Location *a = getLocationByAddress(Addr);
	if (a) {
		disableSymbol(a->label);
		a->label = NULL;
		a->flags |= AF_DELETED;
		location_count--;
	}
}

/*
 *	disables label of an address and unassigns address' label
 */
void Analyser::deleteSymbol(Address *Addr)
{
	Location *a = getLocationByAddress(Addr);
	if (a) {
		disableSymbol(a->label);
		a->label = NULL;
		symbol_count--;
	}
}

/*
 *
 */
bool Analyser::deleteXRef(Address *from, Address *to)
{
	if (!validAddress(from, scvalid) || !validAddress(to, scvalid)) return false;

	Location *a = getLocationByAddress(from);
	if (!a) return false;
	Container *x = a->xrefs;
	if (!x) return false;

	DPRINTF("deleted xref %y->%y\n", from, to);
	
	AddrXRef tmp(to);
	return x->delObj(&tmp);
}

/*
 *	an disabled label will be overwritten as soon as possible,
 *   and never be returned nor saved.
 *	performed this way to preserve the labeltrees structure
 *
 */
void	Analyser::disableSymbol(Symbol *label)
{
	if (label) {
		label->location = NULL;
	}
}

/*
 *
 */
void	Analyser::doBranch(branch_enum_t branch, OPCODE *opcode, int len)
{
	Address *branch_addr = analy_disasm->branchAddr(opcode, branch, true);

	if (branch != br_return) {
		if (!validCodeAddress(branch_addr)) {
			char	msg[100];
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
			ht_snprintf(msg, sizeof msg, "branch to invalid addr %y from addr %y", branch_addr, addr);
			log(msg);
		}
	}
/*	if (branch != brcall) {
		taddr *a = new_addr(addr+len);
		if (!a->comments) {
			add_comment(a->addr, 0, "");
		}
	}*/
	Address *next_addr = addr->clone();
	if (!next_addr->add(len)) {
		delete next_addr;
		next_addr = new InvalidAddress();
	}
	switch (branch) {
		case	br_jump:
			if (next_addr->isValid())
				newLocation(next_addr)->flags |= AF_FUNCTION_END;

			addXRef(branch_addr, addr, xrefjump);
			if (addAddressSymbol(branch_addr, LPRFX_LOC, label_loc, cur_func)) {
				addComment(branch_addr, 0, "");
			}
//			gotoAddress(branch_addr, invalid_addr);
			pushAddress(branch_addr, invalid_addr);
			next_address_is_invalid = true;
			break;
		case	br_return:
			if (next_addr->isValid())
				newLocation(next_addr)->flags |= AF_FUNCTION_END;

			next_address_is_invalid = true;
			break;
		case	br_call: {
			addXRef(branch_addr, addr, xrefcall);
			bool special_func = false;
			const char *lprfx = LPRFX_SUB;
			if (!getSymbolByAddress(branch_addr) && validCodeAddress(branch_addr)) {
				// should be in code_analy
				byte buf[16];
				int bz = bufPtr(branch_addr, buf, sizeof(buf));
				OPCODE *instr = disasm->decode(buf, MIN(bz, max_opcode_length), mapAddr(branch_addr));
				branch_enum_t bt = analy_disasm->isBranch(instr);
				
				if (bt == br_return) {
					lprfx = LPRFX_STUB;
				} else if (bt == br_jump) {
					char buf[1024], label[1024];
					Address *wrap_addr = analy_disasm->branchAddr(instr, bt, false);
					if (validAddress(wrap_addr, scvalid)) {
						Symbol *l = getSymbolByAddress(wrap_addr);
						addComment(branch_addr, 0, "");
						addComment(branch_addr, 0, ";----------------------------------------------");
						global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT;
						if (l && l->name) {
							ht_snprintf(buf, sizeof buf, "%s %s", ";  W R A P P E R for", l->name);
							ht_snprintf(label, sizeof label, "%s_%s_%y", LPRFX_WRAP, l->name, branch_addr);
						} else {
							ht_snprintf(buf, sizeof buf, "%s %s %y", ";  W R A P P E R for", "address", wrap_addr);
							ht_snprintf(label, sizeof label, "%s_%y_%y", LPRFX_WRAP, wrap_addr, branch_addr);
						}
						addComment(branch_addr, 0, buf);
						addComment(branch_addr, 0, ";----------------------------------------------");
						addSymbol(branch_addr, label, label_func);
						special_func = true;
					}
					delete wrap_addr;
				}
			}
			if (!special_func && addAddressSymbol(branch_addr, lprfx, label_func)) {
				addComment(branch_addr, 0, "");
				addComment(branch_addr, 0, ";-----------------------");
				addComment(branch_addr, 0, ";  S U B R O U T I N E");
				addComment(branch_addr, 0, ";-----------------------");
				if (branch_addr->compareTo(cur_func->addr) == 0) {
					addComment(branch_addr, 0, ";  (recursive)");
				}
			}
//			pushAddress(next_addr, cur_func->addr);
//			gotoAddress(branch_addr, branch_addr);
			pushAddress(branch_addr, branch_addr);
			break;
		}
		case	br_jXX:
			addXRef(branch_addr, addr, xrefjump);
			if (addAddressSymbol(branch_addr, LPRFX_LOC, label_loc, cur_func)) {
				addComment(branch_addr, 0, "");
			}
//			pushAddress(next_addr, cur_func->addr);
//			gotoAddress(branch_addr, invalid_addr);
			pushAddress(branch_addr, invalid_addr);
			break;
		default:{}
			// stupid but neccessary
	}
	delete next_addr;
	delete branch_addr;
}

/*
 *
 */
void	Analyser::engageCodeanalyser()
{
	if (queryConfig(Q_ENGAGE_CODE_ANALYSER)) {
		DPRINTF("starting code analyser.\n");
	}
}

static void analyserenum_addrs(Location *locs, Address *at, Location *&loc)
{
	if ((at->compareTo(locs->addr) < 0) || !at->isValid()) {
		loc = locs;
		if (locs->left) analyserenum_addrs(locs->left, at, loc);
	} else /*if (at >= locs->addr)*/ {
		if (locs->right) analyserenum_addrs(locs->right, at, loc);
	}
}

/*
 *
 */
Location *Analyser::enumLocations(Address *Addr)
{
	Location *result = NULL;
	if (locations) analyserenum_addrs(locations, Addr, result);
	while (result && (result->flags & AF_DELETED)) {
		Address *a = result->addr;
		result = NULL;
		analyserenum_addrs(locations, a, result);
	}
	return result;
}

static void analyserenum_addrs_back(Location *locs, Address *at, Location *&loc)
{
	if (at->compareTo(locs->addr) <= 0) {
		if (locs->left) analyserenum_addrs_back(locs->left, at, loc);
	} else /*if (at > locs->addr)*/ {
		loc = locs;
		if (locs->right) analyserenum_addrs_back(locs->right, at, loc);
	}
}

/*
 *
 */
Location *Analyser::enumLocationsReverse(Address *Addr)
{
	Location *result = NULL;
	if (locations) analyserenum_addrs_back(locations, Addr, result);
	while ((result) && (result->flags & AF_DELETED)) {
		Address *a = result->addr;
		result = NULL;
		analyserenum_addrs_back(locations, a, result);
	}
	return result;
}

static void analyserenum_labels(Symbol *labels, const char *at, Symbol *&label)
{
	int i = strcmp(at, labels->name);
	if (i < 0) {
		label = labels;
		if (labels->left) analyserenum_labels(labels->left, at, label);
	} else /*if (i >= 0)*/ {
		if (labels->right) analyserenum_labels(labels->right, at, label);
	}
}

/*
 *   returns the (alphanumerically) next label to "at"
 *	or the first if "at" is NULL
 *	returns NULL if there is no preceding label
 *
 */
Symbol *Analyser::enumSymbolsByName(const char *at)
{
	Symbol *result = NULL;
	if (!at) at="";
	if (symbols) analyserenum_labels(symbols, at, result);

	// label with !addr mustnt be returned
	while ((result) && (!result->location)) {
		char *name = result->name;
		result = NULL;
		analyserenum_labels(symbols, name, result);
	}

	return result;
}

static void analyserenum_labels_back(Symbol *labels, const char *at, Symbol *&label)
{
	int i = strcmp(at, labels->name);
	if (i <= 0) {
		if (labels->left) analyserenum_labels_back(labels->left, at, label);
	} else /*if (i >= 0)*/ {
		label = labels;
		if (labels->right) analyserenum_labels_back(labels->right, at, label);
	}
}

/*
 *
 */
Symbol *Analyser::enumSymbolsByNameReverse(const char *at)
{
	Symbol *result = NULL;
	// FIXME:
	if (!at) at="\xff\xff\xff";
	if (symbols) analyserenum_labels_back(symbols, at, result);

	// labels with !addr mustnt be returned
	while ((result) && (!result->location)) {
		char *name = result->name;
		result = NULL;
		analyserenum_labels_back(symbols, name, result);
	}

	return result;
}

Symbol *Analyser::enumSymbols(Symbol *sym)
{
	if (sym) {
		return enumSymbolsByName(sym->name);
	} else {
		return enumSymbolsByName(NULL);
	}
}

Symbol *Analyser::enumSymbolsReverse(Symbol *sym)
{
	if (sym) {
		return enumSymbolsByNameReverse(sym->name);
	} else {
		return enumSymbolsByNameReverse(NULL);
	}
}


/*
 *
 */
taddr_typetype Analyser::examineData(Address *Addr)
{
	if ((validReadAddress(Addr)) && (validAddress(Addr, scinitialized))) {
		DPRINTF("examinating data @%y:\n", Addr);

	} else return dt_unknown;
	return dt_unknown;
}

/*
 *
 */
static Location *analyserfindaddr(Location *locs, Address *Addr)
{
	if (locs) {
		if (Addr->compareTo(locs->addr) < 0) return analyserfindaddr(locs->left, Addr);
		if (Addr->compareTo(locs->addr) > 0) return analyserfindaddr(locs->right, Addr);
		return (locs->flags & AF_DELETED) ? NULL : locs;
	}
	return NULL;
}

/*
 *
 */
Location *Analyser::getLocationByAddress(Address *Addr)
{
	return analyserfindaddr(locations, Addr);
}

/*
 *   finds the address Addr belongs to (if address has size > 1)
 */
Location *Analyser::getLocationContextByAddress(Address *Addr)
{
	Location *res = enumLocationsReverse(Addr);
	if (res && res->type.type != dt_unknown) {
		Address *resaddr = res->addr->clone();
		resaddr->add(res->type.length);
		if (resaddr->compareTo(Addr) > 0) {
			delete resaddr;
			return res;
		}
		delete resaddr;
	}
	return NULL;
}

/*
 *	finds the function the Addr belongs to (if possible/applicable).
 */
Location *Analyser::getFunctionByAddress(Address *Addr)
{
	Location *loc = getLocationByAddress(Addr);
	if (!loc) loc = enumLocationsReverse(Addr);
	while (loc && !(loc->flags & AF_FUNCTION_SET)) {
		if (loc->flags & AF_FUNCTION_END) return NULL;
		loc = enumLocationsReverse(loc->addr);
	}
	return loc ? loc->thisfunc : NULL;
}

/*
 *	searches back to the last location
 */
Location *Analyser::getPreviousSymbolByAddress(Address *Addr)
{
	Location *loc = getLocationByAddress(Addr);
	if (!loc) loc = enumLocationsReverse(Addr);
	while (loc && !loc->label) {
		if (loc->flags & AF_FUNCTION_END) return NULL;
		loc = enumLocationsReverse(loc->addr);
	}
	return loc ? loc : NULL;
}

/*
 *
 */
static Symbol *analyserfindlabel(Symbol *labels, const char *label)
{
	if (labels) {
		int i = strcmp(label, labels->name);
		if (i < 0) return analyserfindlabel(labels->left, label);
		if (i > 0) return analyserfindlabel(labels->right, label);
		return labels->location ? labels : NULL;
	}
	return NULL;
}

/*
 *
 */
Symbol *Analyser::getSymbolByName(const char *label)
{
	return analyserfindlabel(symbols, label);
}

const char *Analyser::getSymbolNameByLocation(Location *loc)
{
	return loc ? (loc->label ? loc->label->name : NULL): NULL;
}


/**
 *	converts |FileOfs fileofs| to |Address|
 */
Address *Analyser::fileofsToAddress(FileOfs fileaddr)
{
	// abstract / stub
	return new InvalidAddress();
}

/**
 *	called once every time the analyser has nothing more to do.
 */
void	Analyser::finish()
{
	DPRINTF("the analyser finished (for now).\n");
	cur_func = NULL;
	delete addr;
	addr = new InvalidAddress();
	setActive(false);
}

/*
 *
 */
void Analyser::freeLocation(Location *loc)
{
	if (loc) {
		// label will be freed separatly
		delete loc->xrefs;
		freeComments(loc);
		delete loc->addr;
		delete loc;
	}
}

/*
 *
 */
void	Analyser::freeLocations(Location *addrs)
{
	if (addrs) {
		freeLocations(addrs->left);
		freeLocations(addrs->right);
		freeLocation(addrs);
	}
}

/*
 *
 */
void Analyser::freeComments(Location *aAddr)
{
	delete aAddr->comments;
	aAddr->comments = NULL;
}

/*
 *
 */
void Analyser::freeSymbol(Symbol *label)
{
	if (label) {
		free(label->name);
		delete label;
	}
}


/*
 *
 */
void Analyser::freeSymbols(Symbol *labels)
{
	if (labels) {
		freeSymbols(labels->left);
		freeSymbols(labels->right);
		freeSymbol(labels);
	}
}

/*
 *
 */
CommentList *Analyser::getComments(Address *Addr)
{
	Location *a = getLocationByAddress(Addr);
	if (a) {
		return a->comments;
	}
	return NULL;
}

static char *analy_addr_sym_func2(CPU_ADDR Addr, int *symstrlen, void *analy)
{
	Address *a = ((Analyser*)analy)->createAddress();     
	a->getFromCPUAddress(&Addr);
	Location *loc = ((Analyser*)analy)->getLocationByAddress(a);
	delete a;
	if (loc && loc->label) {
		if (symstrlen) *symstrlen = strlen(loc->label->name);
		return loc->label->name;
	}
	return NULL;
}

/*
 *
 */
const char *Analyser::getDisasmStr(Address *Addr, int &length)
{
	if (validAddress(Addr, scinitialized)) {
		if (disasm) {
			addr_sym_func = NULL;
			byte buf[16];
			int bz = bufPtr(Addr, buf, sizeof(buf));
			OPCODE *o = disasm->decode(buf, MIN(bz, max_opcode_length), mapAddr(Addr));
			length = disasm->getSize(o);
			return disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);
		} else {
			return "<no disassembler!>";
		}
	} else {
		return "";
	}
}

/*
 *
 */
const char *Analyser::getDisasmStrFormatted(Address *Addr)
{
	if (disasm) {
		addr_sym_func_context = this;
		addr_sym_func = &analy_addr_sym_func2;

		byte buf[16];
		int bz = bufPtr(Addr, buf, sizeof(buf));
		OPCODE *o=disasm->decode(buf, MIN(bz, max_opcode_length), mapAddr(Addr));
		const char *res = disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);

		addr_sym_func_context = NULL;
		addr_sym_func = NULL;

		return res;
	} else {
		return "<no disassembler!>";
	}
}

/*
 *
 */
int Analyser::getDisplayMode()
{
	return mode;
}

static void analysergetlocationcount(Location *addr, int *c)
{
	if (addr) {
		analysergetlocationcount(addr->left, c);
		if (!(addr->flags & AF_DELETED)) (*c)++;
		analysergetlocationcount(addr->right, c);
	}
}

/*
 *
 */
int	Analyser::getLocationCount() const
{
	int c=0;
	analysergetlocationcount(locations, &c);
	return c;
}

/*
 *
 */
String & Analyser::getName(String &res)
{
	return res = "generic";
}

static void analysergetsymbolcount(Symbol *l, int *c)
{
	if (l) {
		analysergetsymbolcount(l->left, c);
		if (l->location) (*c)++;
		analysergetsymbolcount(l->right, c);
	}
}

/*
 *
 */
int	Analyser::getSymbolCount() const
{
	int c=0;
	analysergetsymbolcount(symbols, &c);
	return c;
}

/*
 *
 */
const char *Analyser::getSegmentNameByAddress(Address *Addr)
{
	return NULL;
}

/*
 *
 */
Symbol *Analyser::getSymbolByAddress(Address *Addr)
{
	Location *a = getLocationByAddress(Addr);

	return (a) ? a->label : NULL;
}

/*
 *
 */
const char *Analyser::getType()
{
	return "generic";
}

/*
 *
 */
Container *Analyser::getXRefs(Address *Addr)
{
	Location *a = getLocationByAddress(Addr);
	if (!a) return NULL;
	return a->xrefs;
}

/*
 *
 */
bool	Analyser::gotoAddress(Address *aAddr, Address *func)
{
	DPRINTF("goto(%y, %y)\n", aAddr, func);
	int diff;
	
	if (first_explored->difference(diff, last_explored) && diff < 0) {
		DPRINTF("explored->add(%y - %y)\n", first_explored, last_explored);
		explored->add(first_explored, last_explored);
	}

	delete first_explored;
	delete last_explored;
	delete next_explored;
	func = func->clone();
	
	if (!validCodeAddress(aAddr) || explored->contains(aAddr)) {
		DPRINTF("Address: %y Valid: %d Explored: %d\n", addr, validCodeAddress(addr), explored->contains(addr));
		do {
			delete addr;
			delete func;
			if (!popAddress(&addr, &func)) {
				first_explored = new InvalidAddress();
				last_explored = new InvalidAddress();
				next_explored = new InvalidAddress();
				addr = new InvalidAddress();
				return false;
			}

			DPRINTF("pop %y   (Valid: %d  Explored: %d)\n", addr, validCodeAddress(addr), explored->contains(addr));
		} while ((explored->contains(addr)) || (!validCodeAddress(addr)));          
	} else {
		if (addr != aAddr) {
			delete addr;
			addr = aAddr->clone();
		}
	}

	if (func->isValid()) {
		cur_func = newLocation(func);
		setLocationFunction(cur_func, cur_func);
	}
	delete func;
	next_explored = (Address *)explored->findNext(addr);
	if (!next_explored) {
		next_explored = new InvalidAddress();
	} else {
		next_explored = next_explored->clone();
	}
	first_explored = addr->clone();
	last_explored = addr->clone();
	return true;
}

/*
 *
 */
void Analyser::initCodeAnalyser()
{
	code = new CodeAnalyser();
	code->init(this);
}

/*
 *
 */
void Analyser::initDataAnalyser()
{
	data = new DataAnalyser();
	data->init(this);
}

/*
 *
 */
bool Analyser::isDirty()
{
	return dirty;
}

/*
 *
 */
void	Analyser::log(const char *s)
{
	// stub
}

/*
 *
 */
void Analyser::makeDirty()
{
	dirty = true;
}

/*
 *
 */
CPU_ADDR Analyser::mapAddr(Address *aAddr)
{
	/*
	 * 	this function should map the independent address Addr to a
	 * 	processor dependent
	 * 	e.g.    .23423 --> 00234324    (or something like this)
	 *   it is only used for relativ calls/jumps
	 */
	CPU_ADDR a;
	aAddr->putIntoCPUAddress(&a);
	return a;
}

Location *Analyser::newLocation(Location *&locs, Address *aAddr)
{
	if (locs) {
		if (aAddr->compareTo(locs->addr) < 0) return newLocation(locs->left, aAddr);
		if (aAddr->compareTo(locs->addr) > 0) return newLocation(locs->right, aAddr);
	} else {
		locs = new Location;
		memset(locs, 0, sizeof *locs);
		locs->addr = aAddr->clone();
		location_count++;
	}
	locs->flags &= ~AF_DELETED;
	return locs;
}

/*
 *
 */
Location *Analyser::newLocation(Address *Addr)
{
/*	if (!(cur_addr_ops--)) {
		optimizeLocationTree();
	}*/
	return newLocation(locations, Addr);
}

Symbol *Analyser::newSymbol(Symbol *&labels, const char *label, Location *loc, labeltype type)
{
	if (labels) {
		int i = strcmp(label, labels->name);
		if (i < 0) return newSymbol(labels->left, label, loc, type);
		if (i > 0) return newSymbol(labels->right, label, loc, type);
		if (!labels->location) {
			labels->location = loc;
			if (type != label_unknown) labels->type = type;
		}
	} else {
		labels = new Symbol;
		labels->name = ht_strdup(label);
		labels->location = loc;
		labels->type = type;
		labels->left = NULL;
		labels->right = NULL;
		symbol_count++;
	}
	return labels;
}

/*
 *
 */
Symbol *Analyser::newSymbol(const char *label, Location *loc, labeltype type, Location *infunc)
{
/*	if (!(cur_label_ops--)) {
		optimizeSymbolTree();
	}*/
	Symbol *result = newSymbol(symbols, label, loc, type);
	if ((result) && (result->location==loc) && (infunc)) setLocationFunction(loc, infunc);
	return result;
}

/*
 * optimizes(=balances) addr tree
 */
void Analyser::optimizeLocationTree()
{
	cur_addr_ops = location_threshold;
	// implement me!
}

/*
 * see optimize_addr_tree()
 */
void Analyser::optimizeSymbolTree()
{
	cur_label_ops = symbol_threshold;
	// implement me!
}

/*
 *
 */
bool Analyser::popAddress(Address **Addr, Address **func)
{
	if (!addr_queue->isEmpty()) {
		AddressQueueItem *aqi = (AddressQueueItem *) addr_queue->deQueue();
		*Addr = aqi->addr->clone();
		*func = aqi->func->clone();
		delete aqi;

		DPRINTF("addr %y (from sub %y) poped\n", *Addr, *func);
		return true;

	} else {
		DPRINTF("pop failed -> analyser obviously finished\n");
		return false;
	}
}

/*
 *
 */
void Analyser::pushAddress(Address *Addr, Address *func)
{
	if (validCodeAddress(Addr)) {
		if (!func->isValid()) {
			if (cur_func) {
				func = cur_func->addr;
			}
		}
		DPRINTF("addr %y (from func %y) pushed\n", Addr, func);
		AddressQueueItem *aqi = new AddressQueueItem(Addr, func);
		addr_queue->enQueue(aqi);
	}
}

/*
 *
 */
int	Analyser::queryConfig(int mode)
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

static void saveaddrs(ObjectStream &st, Location *addr)
{
	if (addr) {
		saveaddrs(st, addr->left);

		if (!(addr->flags & AF_DELETED)) {
			PUTX_OBJECT(st, addr->addr, "addr");
			PUTX_OBJECT(st, addr->xrefs, "xrefs");
			PUTX_OBJECT(st, addr->comments, "comments");
			
			analyser_put_addrtype(st, &addr->type);
			
			if (addr->thisfunc) {
				PUTX_OBJECT(st, addr->thisfunc->addr, "func");
			} else {
				Address *invalid_addr = new InvalidAddress();
				PUTX_OBJECT(st, invalid_addr, "func");
				delete invalid_addr;
			}
			PUTX_INT8X(st, addr->flags, "flags");
		}

		saveaddrs(st, addr->right);
	}
}

static void savelabels(ObjectStream &st, Symbol *label)
{
	if (label) {
		savelabels(st, label->left);

		if (label->location) {
			// label isn't deleted

			PUTX_OBJECT(st, label->location->addr, "addr");
			PUTX_STRING(st, label->name, "name");

			int a = (int)label->type;
			PUTX_INTX(st, a, 1, "type");
		}

		savelabels(st, label->right);
	}
}

/*
 *
 */
void Analyser::store(ObjectStream &st) const
{
	PUT_OBJECT(st, addr);
	
	PUT_OBJECT(st, addr_queue);
	PUT_INT32D(st, ops_parsed);
	PUT_BOOL(st, active);
	PUT_OBJECT(st, next_explored);
	PUT_OBJECT(st, first_explored);
	PUT_OBJECT(st, last_explored);
	PUT_INT32X(st, mode);
	PUT_OBJECT(st, explored);
	PUT_OBJECT(st, initialized);

	st.putComment("locations");
	PUTX_INT32D(st, getLocationCount(), "location_count");
	saveaddrs(st, locations);

	st.putComment("symbols");
	PUTX_INT32D(st, getSymbolCount(), "symbol_count");
	savelabels(st, symbols);

	PUT_OBJECT(st, analy_disasm);
	PUT_OBJECT(st, disasm);
	PUT_OBJECT(st, code);
	PUT_OBJECT(st, data);

	PUT_INT32D(st, location_threshold);
	PUT_INT32D(st, symbol_threshold);

	PUT_INT32D(st, max_opcode_length);
	
	if (cur_func) {
		PUTX_OBJECT(st, cur_func->addr, "cur_func");
	} else {
		PUTX_OBJECT(st, NULL, "cur_func");
	}
	
	dirty = false;
}

/*
 *
 */
void Analyser::setActive(bool mode)
{
	if (mode) {
		if (!active) {
			active = true;
			some_analyser_active++;
		}
	} else {
		if (active) {
			active = false;
			some_analyser_active--;
		}
	}
}

/*
 *
 */
void Analyser::setDisplayMode(int enable, int disable)
{
	mode &= ~disable;
	mode |= enable;
}

/*
 *
 */
void Analyser::setLocationFunction(Location *a, Location *func)
{
	if (a) {
		a->thisfunc = func;
		a->flags &= !AF_FUNCTION_END;
		a->flags |= AF_FUNCTION_SET;
	}
}

/*
 *
 */
void Analyser::setDisasm(Disassembler *d)
{
	disasm = d;
	if (disasm) {
		int t;
		disasm->getOpcodeMetrics(t, max_opcode_length, t, t, t);
	} else {
		max_opcode_length = 1;
	}
}

/*
 *	sets addr_threshold. after threshold addr_tree ops the tree will
 *	be optimized
 */
void Analyser::setLocationTreeOptimizeThreshold(int threshold)
{
	location_threshold = threshold;
	if (cur_addr_ops > location_threshold) cur_addr_ops = location_threshold;
}

/*
 *	see set_addr_tree_optimize_threshold
 */
void Analyser::setSymbolTreeOptimizeThreshold(int threshold)
{
	symbol_threshold = threshold;
	if (cur_label_ops > symbol_threshold) cur_label_ops = symbol_threshold;
}

/*
 *
 */
void Analyser::toggleDisplayMode(int toggle)
{
	mode ^= toggle;
}

/*
 *
 */
bool	Analyser::validCodeAddress(Address *Addr)
{
	Location *a = getLocationByAddress(Addr);
	if (a) {
		if ((a->type.type != dt_code) && (a->type.type != dt_unknown)) return false;
	}
	return validAddress(Addr, sccode);
}

/*
 *
 */
bool	Analyser::validReadAddress(Address *Addr)
{
	return validAddress(Addr, scread);
}

/*
 *
 */
bool	Analyser::validWriteAddress(Address *Addr)
{
	return validAddress(Addr, scwrite);
}

/****************************************************************************/

AnalyDisassembler::AnalyDisassembler()
{
	disasm = NULL;
}

/*
 *
 */
void AnalyDisassembler::init(Analyser *A)
{
	analy = A;
	initDisasm();
}

/*
 *
 */
void AnalyDisassembler::initDisasm()
{
	if (analy) {
		analy->setDisasm(disasm);
	}
}

