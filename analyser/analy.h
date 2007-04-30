/* 
 *	HT Editor
 *	analy.h
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

#ifndef analy_h
#define analy_h

#include "asm.h"
#include "data.h"
#include "code_analy.h"
#include "data_analy.h"
#include "stddata.h"

extern int num_ops_parsed;

class Analyser;

#define ADDRESS_STRING_FORMAT_COMPACT            0
#define ADDRESS_STRING_FORMAT_LEADING_WHITESPACE 1
#define ADDRESS_STRING_FORMAT_LEADING_ZEROS      2
#define ADDRESS_STRING_FORMAT_RESERVED           3

#define ADDRESS_STRING_FORMAT_HEX_CAPS		    4
#define ADDRESS_STRING_FORMAT_ADD_0X		    8
#define ADDRESS_STRING_FORMAT_ADD_H		   16

class Address: public Object {
public:
				Address() {};
				Address(BuildCtorArg&a): Object(a) {};
	virtual	bool		add(int offset) = 0;
	virtual	int		byteSize() = 0;
	virtual Address	*	clone() const = 0;
	virtual	int		compareDelinear(Address *to);
	virtual	bool 		difference(int &result, Address *to) = 0;
	virtual	void 		getFromArray(const byte *array) = 0;
	virtual	void 		getFromCPUAddress(CPU_ADDR *ca) = 0;
	virtual	bool		getFromUInt64(uint64 u) = 0;
	virtual	bool 		isValid();
	virtual	int		parseString(const char *s, int length, Analyser *a) = 0;
	virtual	void		putIntoArray(byte *array) const = 0;
	virtual	void		putIntoCPUAddress(CPU_ADDR *ca) const = 0;
	virtual	bool		putIntoUInt64(uint64 &u) const = 0;
	virtual	int		stringify(char *s, int max_length, int format) const = 0;
	virtual	int		stringSize() const = 0;
	virtual	int		toString(char *buf, int buflen) const;
};

class InvalidAddress: public Address {
public:
				InvalidAddress() {};
				InvalidAddress(BuildCtorArg&a): Address(a) {};
	virtual	bool		add(int offset);
	virtual	int		byteSize();
	virtual	int		compareTo(const Object *obj) const;
	virtual	bool		difference(int &result, Address *to);
	virtual	InvalidAddress *clone() const;
	virtual	void 		getFromArray(const byte *array);
	virtual	void		getFromCPUAddress(CPU_ADDR *ca);
	virtual	bool		getFromUInt64(uint64 u);
	virtual	bool		isValid();
	virtual	ObjectID	getObjectID() const;
	virtual	int		parseString(const char *s, int length, Analyser *a);
	virtual	void 		putIntoArray(byte *array) const;
	virtual	void 		putIntoCPUAddress(CPU_ADDR *ca) const;
	virtual	bool		putIntoUInt64(uint64 &u) const;
	virtual	int		stringify(char *s, int max_length, int format) const;
	virtual	int		stringSize() const;
};

/*
 *	This address type will be used by most analysers, so we define it here.
 */
class AddressFlat32: public Address {
public:
	uint32 addr;
				AddressFlat32(BuildCtorArg&a): Address(a) {};
				AddressFlat32(uint32 a=0): addr(a) {};
	virtual	bool		add(int offset);
	virtual	int		byteSize();
	virtual	AddressFlat32 *	clone() const;
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		compareDelinear(Address *to);
	virtual	void		getFromArray(const byte *array);
	virtual	void		getFromCPUAddress(CPU_ADDR *ca);
	virtual	bool		getFromUInt64(uint64 u);
	virtual	bool		difference(int &result, Address *to);
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	int		parseString(const char *s, int length, Analyser *a);
	virtual	void		putIntoArray(byte *array) const;
	virtual	void		putIntoCPUAddress(CPU_ADDR *ca) const;
	virtual	bool		putIntoUInt64(uint64 &u) const;
	virtual	void		store(ObjectStream &s) const;
	virtual	int		stringify(char *s, int max_length, int format) const;
	virtual	int		stringSize() const;
};

class AddressFlat64: public Address {
public:
	uint64 addr;
				AddressFlat64(BuildCtorArg&a): Address(a) {};
				AddressFlat64(uint64 a=0): addr(a) {};
	virtual	bool		add(int offset);
	virtual	int		byteSize();
	virtual	int		compareTo(const Object *obj) const;
	virtual	int		compareDelinear(Address *to);
	virtual	void		getFromArray(const byte *array);
	virtual	void		getFromCPUAddress(CPU_ADDR *ca);
	virtual	bool		getFromUInt64(uint64 u);
	virtual	bool		difference(int &result, Address *to);
	virtual	AddressFlat64 *	clone() const;
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	int		parseString(const char *s, int length, Analyser *a);
	virtual	void		putIntoArray(byte *array) const;
	virtual	void		putIntoCPUAddress(CPU_ADDR *ca) const;
	virtual	bool		putIntoUInt64(uint64 &u) const;
	virtual	void		store(ObjectStream &s) const;
	virtual	int		stringify(char *s, int max_length, int format) const;
	virtual	int		stringSize() const;
};

#define ANALY_SEGMENT_CAP_WRITE 1
#define ANALY_SEGMENT_CAP_INITIALIZED 2
// other caps can be defined locally

class Segment: public Object {
	Address *start, *end;
	char *name;
	int caps;
	
					Segment(const char *n, Address *s, Address *e, int c, int address_size);
	virtual	bool			containsAddress(Address *addr) = 0;
	virtual	String &		getName(String &res);
	virtual	int			getAddressSize();
	virtual	int			getCapability(int cap);
};

/*
 *	these are the different possibilities of a branch
 *	to support further processors other types can be added
 */
enum branch_enum_t {
			br_nobranch,					// straight exec. flow
			br_jump,
			br_return,
			br_call,
			br_jXX
};

/*
 *   internal opcodes are interchanged in this format
 */
#define OPCODE dis_insn

/*
 *
 */
class AnalyDisassembler: public Object {
public:
	Analyser		*analy;
	Disassembler		*disasm;
					AnalyDisassembler();
					AnalyDisassembler(BuildCtorArg &a): Object(a) {};

		void			init(Analyser *A);

	virtual	Address *		branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine) = 0;
	virtual	void			examineOpcode(OPCODE *opcode) = 0;
	virtual	void			initDisasm();
	virtual	branch_enum_t		isBranch(OPCODE *opcode) = 0;
};

/***************************************************************************/

enum xref_enum_t {
	xrefread,
	xrefwrite,
	xrefoffset,
	xrefjump,
	xrefcall,
	xrefijump,
	xreficall
};

class AddrXRef: public Object {
public:
	Address		*addr;
	xref_enum_t	type;
				AddrXRef(Address *a, xref_enum_t aType = xrefread);
				AddrXRef(BuildCtorArg&a): Object(a) {};
	virtual			~AddrXRef();
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
	virtual int		compareTo(const Object *) const;
};

class CommentList: public Array {
public:
				CommentList();
	void			appendPreComment(const char *s);
	void			appendPreComment(int special);
	void			appendPostComment(const char *s);
	void			appendPostComment(int special);
	const char *		getName(uint i);
};

struct Symbol;

struct Location {
	// the address
	Address		*addr;
	// this is a tree structure (key is addr)
	Location	*left, *right;
	// attached label
	Symbol		*label;
	// attached xrefs
	Container	*xrefs;
	// attached comments
	CommentList	*comments;
	// for data types
	taddr_type	type;
	// the function the address belongs to (if applicable)
	Location	*thisfunc;
	// some flags
	int		flags;
};

/*
 * taddr.flags:
 */
#define AF_DELETED 1
#define AF_FUNCTION_SET 2
#define AF_FUNCTION_END 4

enum tsectype {
	scvalid,
	scread,
	scwrite,
	screadwrite,
	sccode,
	scinitialized
};

enum taccesstype {
	acread,
	acwrite,
	acoffset
};

struct taccess	{
	bool		indexed;
	int		size;
	taccesstype 	type;
};

enum labeltype {
	label_unknown = 0,
	label_func,
	label_loc,
	label_data
};

struct Symbol {
	labeltype	type;
	Location *	location;
	char *		name;
	Symbol		*left, *right;
};

class AddressQueueItem: public Object {
public:
	Address	*addr;
	Address	*func;
				AddressQueueItem(BuildCtorArg&a): Object(a) {};
				AddressQueueItem(Address *Addr, Address *Func);
				~AddressQueueItem();
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		store(ObjectStream &s) const;
};

class CodeAnalyser;
class DataAnalyser;

class Analyser: public Object	{
public:
	Address *		addr;
	Address *		invalid_addr;
	Queue *			addr_queue;
	int			ops_parsed;							// for continuing
	bool			active;
	Address			*next_explored, *first_explored, *last_explored;
	bool			next_address_is_invalid;
	Area *			explored;
	Area *			initialized;
	Location *		locations;
	CodeAnalyser *		code;
	DataAnalyser *		data;
	AnalyDisassembler *	analy_disasm;
	Disassembler *		disasm;
	Symbol *		symbols;
	int			location_threshold, symbol_threshold;
	int			cur_addr_ops, cur_label_ops;                 // for threshold
	int			max_opcode_length;
	Location		*cur_func;
	mutable bool		dirty;

	int			symbol_count;
	int			location_count;

				Analyser() {};
				Analyser(BuildCtorArg&a): Object(a) {};

		void		init();
	virtual	void		load(ObjectStream &s);
	virtual	void		done();

		bool		addAddressSymbol(Address *Addr, const char *Prefix, labeltype type, Location *infunc=NULL);
		void	 	addComment(Address *Addr, int line, const char *c);
		bool		addSymbol(Address *Addr, const char *label, labeltype type, Location *infunc=NULL);
	virtual	FileOfs		addressToFileofs(Address *Addr) = 0;
		bool		addXRef(Address *from, Address *to, xref_enum_t action);
		void	 	assignComment(Address *Addr, int line, const char *c);
		bool		assignSymbol(Address *Addr, const char *label, labeltype type, Location *infunc=NULL);
		void		assignXRef(Address *from, Address *to, xref_enum_t action);
	virtual	void		beginAnalysis();
	virtual	uint		bufPtr(Address *Addr, byte *buf, int size) = 0;
		bool	  	continueAnalysis();
		void		continueAnalysisAt(Address *Addr);
	virtual	Address *	createAddress() = 0;
		void		dataAccess(Address *Addr, taccess access);
		void		deleteLocation(Address *Addr);
		void		deleteSymbol(Address *Addr);
		bool		deleteXRef(Address *from, Address *to);
		void		disableSymbol(Symbol *label);
		void		doBranch(branch_enum_t branch, OPCODE *opcode, int len);
		void		engageCodeanalyser();
		Location *	enumLocations(Address *Addr);
		Location *	enumLocationsReverse(Address *Addr);
		Symbol *	enumSymbolsByName(const char *at);
		Symbol *	enumSymbolsByNameReverse(const char *at);
		Symbol *	enumSymbols(Symbol *sym);
		Symbol *	enumSymbolsReverse(Symbol *sym);
	virtual	taddr_typetype	examineData(Address *Addr);
		void		finish();
		void		freeLocation(Location *loc);
		void		freeLocations(Location *locs);
		void		freeComments(Location *loc);
		void		freeSymbol(Symbol *sym);
		void		freeSymbols(Symbol *syms);
		Location *	getLocationByAddress(Address *Addr);
		Location *	getLocationContextByAddress(Address *Addr);
		int		getLocationCount() const;
		Location *	getFunctionByAddress(Address *Addr);
		Location *	getPreviousSymbolByAddress(Address *Addr);
	virtual	const char *	getSegmentNameByAddress(Address *Addr);
		Symbol *	getSymbolByAddress(Address *Addr);
		Symbol *	getSymbolByName(const char *label);
		const char *	getSymbolNameByLocation(Location *loc);
		int		getSymbolCount() const;
		bool		gotoAddress(Address *Addr, Address *func);
	virtual	void 		initCodeAnalyser();
	virtual	void		initDataAnalyser();
	virtual	void		initUnasm() = 0;
	virtual	void		log(const char *s);                // stub
	virtual	CPU_ADDR 	mapAddr(Address *Addr);      // stub
		Location *	newLocation(Address *Addr);
		Location *	newLocation(Location *&locs, Address *Addr);
		Symbol *	newSymbol(const char *label, Location *loc, labeltype type, Location *infunc);
		Symbol *	newSymbol(Symbol *&syms, const char *label, Location *loc, labeltype type);
	virtual	Address *	nextValid(Address *Addr) = 0;
		void		optimizeLocationTree();
		void		optimizeSymbolTree();
		bool		popAddress(Address **Addr, Address **func);
		void		pushAddress(Address *Addr, Address *func);
	virtual	int		queryConfig(int mode);				// stub
		void		setActive(bool mode);
		void		setLocationFunction(Location *a, Location *func);
		void		setLocationTreeOptimizeThreshold(int threshold);
		void		setDisasm(Disassembler *d);
		void		setSymbolTreeOptimizeThreshold(int threshold);
	virtual	void		store(ObjectStream &s) const;
	virtual	bool		validAddress(Address *addr, tsectype action) = 0;
		bool		validCodeAddress(Address *addr);
		bool		validReadAddress(Address *addr);
		bool		validWriteAddress(Address *addr);

//  interface only (there's no internal use)
		int	mode;

	virtual	Assembler *	createAssembler();
	virtual	Address *	fileofsToAddress(FileOfs fileofs);
		CommentList *	getComments(Address *Addr);
		const char *	getDisasmStr(Address *Addr, int &length);
		const char *	getDisasmStrFormatted(Address *Addr);
		int		getDisplayMode();
	virtual	String &	getName(String &res);
	virtual	const char *	getType();
		Container *	getXRefs(Address *Addr);
		bool		isDirty();
		void		makeDirty();
		void		setDisplayMode(int enable, int disable);
		void		toggleDisplayMode(int toggle);
};

/* display modes */
#define ANALY_SHOW_ADDRESS 1
#define ANALY_SHOW_COMMENTS 2
#define ANALY_SHOW_LABELS 4
#define ANALY_SHOW_XREFS 8
#define ANALY_SHOW_BYTES 16
#define ANALY_EDIT_BYTES 32
#define ANALY_TRANSLATE_SYMBOLS 64
#define ANALY_COLLAPSE_XREFS 128

/* queryConfig() constants */
#define Q_DO_ANALYSIS 1
#define Q_ENGAGE_CODE_ANALYSER 2
#define Q_ENGAGE_DATA_ANALYSER 3

/* interesting constants */
#define INVALID_FILE_OFS ((FileOfs)-1)

/* analyser system constants */
#define MAX_OPS_PER_CONTINUE 10

extern int global_analyser_address_string_format;
 
#endif
