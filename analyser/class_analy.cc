/*
 *	HT Editor
 *	class_analy.cc
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
#include "analy_java.h"
#include "global.h"
#include "class.h"
#include "class_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "htstring.h"
#include "snprintf.h"
#include "pestruct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
void	ClassAnalyser::init(ht_class_shared_data *Class_shared, ht_streamfile *File)
{
	class_shared = Class_shared;
     file = File;

	Analyser::init();

     validarea = class_shared->valid;
     initialized = class_shared->initialized;
	/////////////

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);
}


/*
 *
 */
int	ClassAnalyser::load(ht_object_stream *f)
{
	return Analyser::load(f);
}

/*
 *
 */
void	ClassAnalyser::done()
{
	Analyser::done();
}

/*
 *
 */
void ClassAnalyser::beginAnalysis()
{
	char buffer[1024];
     
     char *b = &buffer[ht_snprintf(buffer, 1024, "; public class %s", class_shared->classinfo.thisclass)];
     if (class_shared->classinfo.superclass) {
          b += ht_snprintf(b, 1024, " extends %s", class_shared->classinfo.superclass);
     }
     if (class_shared->classinfo.interfaces) {
          b += ht_snprintf(b, 1024, " implements");
          int count = class_shared->classinfo.interfaces->count();
          for (int i=0; i<count; i++) {
	          b += ht_snprintf(b, 1024, " %y%c", class_shared->classinfo.interfaces->get(i), (i+1<count)?',':' ');
          }
     }
     b += ht_snprintf(b, 1024, " {");

	Address *a = createAddress32(0);
     addComment(a, 0, "");
	addComment(a, 0, ";********************************************************");
	addComment(a, 0, buffer);
	addComment(a, 0, ";********************************************************");
     delete a;
     if (class_shared->methods) {
     	ClassMethod *cm = NULL;
          ht_data *value;
          while ((cm = (ClassMethod*)class_shared->methods->enum_next(&value, cm))) {
               Address *a = createAddress32(cm->start);
               char buffer2[1024];
               java_demangle(buffer2, class_shared->classinfo.thisclass, cm->name, cm->type);
               ht_snprintf(buffer, 1024, "; %s", buffer2);
               addComment(a, 0, "");
			addComment(a, 0, ";----------------------------------------------");
               addComment(a, 0, buffer);
			addComment(a, 0, ";----------------------------------------------");
			addAddressSymbol(a, cm->name, label_func);
			delete a;
          }
     }
	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);

	Analyser::beginAnalysis();
}

/*
 *
 */
OBJECT_ID	ClassAnalyser::object_id()
{
	return ATOM_CLASS_ANALYSER;
}

/*
 *
 */
UINT ClassAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FILEOFS ofs = addressToFileofs(Addr);
	assert(ofs != INVALID_FILE_OFS);
	file->seek(ofs);
	return file->read(buf, size);
}

/*
 *
 */
Address *ClassAnalyser::createAddress()
{
	return new AddressFlat32(0);
}

/*
 *
 */
Address *ClassAnalyser::createAddress32(ClassAddress addr)
{
	return new AddressFlat32((dword)addr);
}

/*
 *
 */
Assembler *ClassAnalyser::createAssembler()
{
	return NULL;
}

/*
 *
 */
FILEOFS ClassAnalyser::addressToFileofs(Address *Addr)
{
	if (validAddress(Addr, scinitialized)) {
		return ((AddressFlat32*)Addr)->addr;
	} else {
		return INVALID_FILE_OFS;
	}
}

/*
 *
 */
char *ClassAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char sectionname[9];
	strcpy(sectionname, "test");
	return sectionname;
}

/*
 *
 */
char	*ClassAnalyser::getName()
{
	return file->get_desc();
}

/*
 *
 */
char *ClassAnalyser::getType()
{
	return "Java-Class/Analyser";
}

/*
 *
 */
void ClassAnalyser::initCodeAnalyser()
{
	Analyser::initCodeAnalyser();
}


int class_token_func(char *result, int maxlen, dword token, void *context)
{
	return token_translate(result, maxlen, token, (ht_class_shared_data *)context);
}

/*
 *
 */
void ClassAnalyser::initUnasm()
{
	DPRINTF("class_analy: ");
	analy_disasm = new AnalyJavaDisassembler();
	((AnalyJavaDisassembler*)analy_disasm)->init(this, class_token_func, class_shared);
}

/*
 *
 */
void ClassAnalyser::log(const char *msg)
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
Address *ClassAnalyser::nextValid(Address *Addr)
{
	return (Address *)validarea->findNext(Addr);
}

/*
 *
 */
void ClassAnalyser::store(ht_object_stream *st)
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
int	ClassAnalyser::queryConfig(int mode)
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
Address *ClassAnalyser::fileofsToAddress(FILEOFS fileaddr)
{
	Address *a = createAddress32(fileaddr);
     if (validAddress(a, scvalid)) {
          return a;
     } else {
     	delete a;
          return NULL;
     }
}

/*
 *
 */
bool ClassAnalyser::validAddress(Address *Addr, tsectype action)
{
/*	pe_section_headers *sections=&pe_shared->sections;
	int sec;
	Addr-=pe_shared->pe32.header_nt.image_base;
	if (!pe_rva_to_section(sections, Addr, &sec)) return false;
	COFF_SECTION_HEADER *s=sections->sections+sec;
	switch (action) {
		case scvalid:
			return true;
		case scread:
			return s->characteristics & COFF_SCN_MEM_READ;
		case scwrite:
			return s->characteristics & COFF_SCN_MEM_WRITE;
		case screadwrite:
			return s->characteristics & COFF_SCN_MEM_WRITE;
		case sccode:
			// FIXME: EXECUTE vs. CNT_CODE ?
			if (!pe_rva_is_physical(sections, Addr)) return false;
			return (s->characteristics & (COFF_SCN_MEM_EXECUTE | COFF_SCN_CNT_CODE));
		case scinitialized:
			if (!pe_rva_is_physical(sections, Addr)) return false;
			return !(s->characteristics & COFF_SCN_CNT_UNINITIALIZED_DATA);
	}*/
	if (!Addr->isValid()) return false;
	switch (action) {
		case scinitialized:
			return class_shared->initialized->contains(Addr);
		default:
			return class_shared->valid->contains(Addr);
     }
}


