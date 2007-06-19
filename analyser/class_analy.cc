/*
 *	HT Editor
 *	class_analy.cc
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

#include "analy.h"
#include "analy_alpha.h"
#include "analy_names.h"
#include "analy_register.h"
#include "analy_java.h"
#include "class.h"
#include "class_analy.h"

#include "htctrl.h"
#include "htdebug.h"
#include "htiobox.h"
#include "strtools.h"
#include "snprintf.h"
#include "pestruct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
void	ClassAnalyser::init(ht_class_shared_data *Class_shared, File *File)
{
	class_shared = Class_shared;
	file = File;

	Analyser::init();

	initialized->done();
	delete initialized;
	initialized = class_shared->initialized->clone();
	/////////////

	setLocationTreeOptimizeThreshold(100);
	setSymbolTreeOptimizeThreshold(100);
}

/*
 *
 */
void ClassAnalyser::beginAnalysis()
{
	char buffer[1024];
	
	String b;
	*java_demangle_flags(buffer, class_shared->flags) = 0;
	b.assignFormat("; %s%s %s", buffer, (class_shared->flags & jACC_INTERFACE)?"interface":"class", class_shared->classinfo.thisclass);
	if (class_shared->classinfo.superclass) {
		String b2;
		b2.assignFormat(" extends %s", class_shared->classinfo.superclass);
		b += b2;
	}
	if (class_shared->classinfo.interfaces) {
		b += " implements";
		int count = class_shared->classinfo.interfaces->count();
		for (int i=0; i < count; i++) {
			String b2;
			b2.assignFormat("%y%c", (*class_shared->classinfo.interfaces)[i], (i+1<count)?',':' ');
			b += b2;
		}
	}
	b += " {";

	Address *a = createAddress32(0);
	addComment(a, 0, "");
	addComment(a, 0, ";********************************************************");
	addComment(a, 0, b.contentChar());
	addComment(a, 0, ";********************************************************");
	if (class_shared->fields && class_shared->fields->count()) {
		addComment(a, 0, "");
 		addComment(a, 0, ";  Fields: ");
		addComment(a, 0, "; =========");
		foreach (ClassField, cf, *class_shared->fields, {
			char buffer2[1024];
			java_demangle_field(buffer2, cf->name, cf->type, cf->flags);
			ht_snprintf(buffer, 1024, ";  %s", buffer2);
			addComment(a, 0, buffer);
		});
		addComment(a, 0, "");
	}
	delete a;
	if (class_shared->methods) {
		foreach (ClassMethod, cm, *class_shared->methods, {
			Address *a = createAddress32(cm->start);
			char buffer2[1024];
			java_demangle(buffer2, class_shared->classinfo.thisclass, cm->name, cm->type, cm->flags);
			ht_snprintf(buffer, 1024, "; %s", buffer2);
			addComment(a, 0, "");
			addComment(a, 0, ";----------------------------------------------");
			addComment(a, 0, buffer);
			addComment(a, 0, ";----------------------------------------------");
			addAddressSymbol(a, cm->name, label_func);
			pushAddress(a, a);
			if (cm->length) {
				Address *b = createAddress32(cm->start + cm->length - 1);
				initialized->add(a, b);
				delete b;
			}
			delete a;
			for (int i=0; i < cm->exctbl_len; i++) {
				exception_info *ei = cm->exctbl + i;
				if (ei->catch_type) {
					token_translate(buffer2, sizeof buffer2, ei->catch_type, class_shared);
				} else {
					ht_strlcpy(buffer2, "...", sizeof buffer2);
				}
				ht_snprintf(buffer, sizeof buffer, "catch (%s)", buffer2);
				Address *b = createAddress32(cm->start + ei->start_pc);
				ht_snprintf(buffer2, sizeof buffer2, "[%d] try { // %s", i, buffer);
				addComment(b, 0, buffer2);
				delete b;
				b = createAddress32(cm->start + ei->end_pc);
				ht_snprintf(buffer2, sizeof buffer2, "[%d] } // %s", i, buffer);
				addComment(b, 0, buffer2);
				delete b;
				b = createAddress32(cm->start + ei->handler_pc);
				ht_snprintf(buffer2, sizeof buffer2, "[%d] %s:", i, buffer);
				addComment(b, 0, buffer2);
				pushAddress(b, b);
				delete b;
			}
		});
 	}
	setLocationTreeOptimizeThreshold(1000);
	setSymbolTreeOptimizeThreshold(1000);

	Analyser::beginAnalysis();
}

/*
 *
 */
ObjectID ClassAnalyser::getObjectID() const
{
	return ATOM_CLASS_ANALYSER;
}

/*
 *
 */
uint ClassAnalyser::bufPtr(Address *Addr, byte *buf, int size)
{
	FileOfs ofs = addressToFileofs(Addr);
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
	return new AddressFlat32(uint32(addr));
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
FileOfs ClassAnalyser::addressToFileofs(Address *Addr)
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
const char *ClassAnalyser::getSegmentNameByAddress(Address *Addr)
{
	static char sectionname[1];
	Location *loc = getFunctionByAddress(Addr);
	if (loc && loc->label && loc->label->name) {
		return loc->label->name;
	} else {
		sectionname[0] = 0;
		return sectionname;
	}
}

/*
 *
 */
String &ClassAnalyser::getName(String &res)
{
	return file->getDesc(res);
}

/*
 *
 */
const char *ClassAnalyser::getType()
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


int class_token_func(char *result, int maxlen, uint32 token, void *context)
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
	return (Address *)class_shared->valid->findNext(Addr);
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

void	ClassAnalyser::reinit(ht_class_shared_data *cl_shared, File *f)
{
	class_shared = cl_shared;
	file = f;
	if (disasm->getObjectID() == ATOM_DISASM_JAVA) {
		((javadis *)disasm)->initialize(class_token_func, class_shared);
	}
}

/*
 *
 */
Address *ClassAnalyser::fileofsToAddress(FileOfs fileaddr)
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
	if (!Addr->isValid() || !class_shared->valid->contains(Addr)) return false;
	switch (action) {
	case scinitialized:
	case sccode:
		return class_shared->initialized->contains(Addr);
	default:
		return true;
	}
}


