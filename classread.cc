/*
 *	HT Editor
 *	classread.cc
 *
 *	Copyright (C) 2001 Stanley Gambarin <stanleyg76@yahoo.com>
 *	Copyright (C) 2002 Sebastian Biallas (sb@biallas.net)
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

#include <stdlib.h>
#include <string.h>

#include "analy.h"
#include "class.h"
#include "strtools.h"
#include "snprintf.h"
#include "stream.h"


static u1 inp[4];
static u4 offset;
#define cls_read(a, b, c, d) (((b) != ((d)->read((a), (b)*(c)))) \
				? (0) : (offset+=(b), (b)))
#define READ1() \
  (((inp[0]=inp[1]=inp[2]=inp[3]=0),  \
   (1 == cls_read (inp, 1, 1, htio))) \
   ? (u1)(inp[0]) : 0)
#define READ2() \
  (((inp[0]=inp[1]=inp[2]=inp[3]=0),  \
   (2 == cls_read (inp, 2, 1, htio))) \
   ? ((((u2)inp[0])<<8)|inp[1]) : 0)
#define READ4() \
  (((inp[0]=inp[1]=inp[2]=inp[3]=0),  \
   (4 == cls_read (inp, 4, 1, htio))) \
   ? (((((((u4)inp[0]<<8)|inp[1])<<8)|inp[2])<<8)|inp[3]) : 0)
#define READN(inb, n) cls_read (inb, n, 1, htio)
#define SKIPN(n) {u1 b; for (u4 i=0; i<n; i++) {cls_read(&b, 1, 1, htio);}}

ClassMethod::ClassMethod(char *n, char *d, ClassAddress s, uint l, int f,
			int e_len, exception_info *e)
{
	name = n;
	start = s;
	type = d;
	length = l;
	flags = f;
	exctbl_len = e_len;
	exctbl = e;
}

int ClassMethod::compareTo(const Object *obj) const
{
	ClassMethod *cm = (ClassMethod*)obj;
	if ((start + length - 1) < cm->start) return -1;
	if (start > (cm->start + cm->length - 1)) return 1;
	return 0;
}

ClassField::ClassField(char *n, char *d, int f)
{
	name = n;
	type = d;
	flags = f;
}

/* extract name from a utf8 constant pool entry */
static char *get_string(Stream *htio, classfile *clazz, uint index)
{
	return (index < clazz->cpool_count) ? clazz->cpool[index]->value.string : (char*)"?";
}

/* extract name from a utf8 constant pool class entry */
static char *get_class_name(Stream *htio, classfile *clazz, uint index)
{
	return (index < clazz->cpool_count) ? get_string(htio, clazz, clazz->cpool[index]->value.llval[0]): (char*)"?";
}

/* extract name from a utf8 constant pool class entry */
static void get_name_and_type(Stream *htio, classfile *clazz, uint index, char *name, char *type)
{
	strcpy(name, (index < clazz->cpool_count) ? get_string(htio, clazz, clazz->cpool[index]->value.llval[0]) : "?");
	strcpy(type, (index < clazz->cpool_count) ? get_string(htio, clazz, clazz->cpool[index]->value.llval[1]) : "?");
}

/* read and return constant pool entry */
static cp_info *read_cpool_entry(Stream *htio, classfile *clazz)
{
	cp_info *cp;
	u2 idx;

	cp         = ht_malloc(sizeof (*cp));
	cp->offset = offset;
	cp->tag    = READ1();
	switch (cp->tag) {
	case CONSTANT_Utf8:
		idx = READ2();
		cp->value.string = ht_malloc(idx+1);
		cls_read(cp->value.string, idx, 1, htio);
		cp->value.string[idx] = 0;
		break;
	case CONSTANT_Integer:
	case CONSTANT_Float:
		cp->value.ival = READ4();
		break;
	case CONSTANT_Long:
	case CONSTANT_Double:
		cp->value.llval[0] = READ4();
		cp->value.llval[1] = READ4();
		break;
	case CONSTANT_Class:
	case CONSTANT_String:
		cp->value.llval[0] = READ2();
		break;
	case CONSTANT_Fieldref:
	case CONSTANT_Methodref:
	case CONSTANT_InterfaceMethodref:
	case CONSTANT_NameAndType:
		cp->value.llval[0] = READ2();
		cp->value.llval[1] = READ2();
		break;
	default:
		return NULL;
	}
	return (cp);
}

/* read and return an attribute read */
attrib_info *attribute_read(Stream *htio, classfile *clazz)
{
	attrib_info *a;
	u4 len;
	char *aname;

	a = ht_malloc(sizeof (*a));
	if (!a) {
		return NULL;
	}
	a->offset    = offset;
	a->name      = READ2();
	a->len = len = READ4();

	aname = get_string(htio, clazz, a->name);
	if (!strcmp (aname, "ConstantValue")) {
		a->tag = ATTRIB_ConstantValue;
	} else if (!strcmp(aname, "Code")) {
		a->tag = ATTRIB_Code;
		a->code.max_stack = READ2();
		a->code.max_locals = READ2();
		a->code.len = READ4();
		a->code.start = offset;
		len -= 2+2+4+a->code.len;
		SKIPN(a->code.len);
		a->code.exctbl_len = READ2();
		if (a->code.exctbl_len) {
			a->code.exctbl = ht_malloc(a->code.exctbl_len * sizeof(exception_info));
			for (uint i=0; i < a->code.exctbl_len; i++) {
				a->code.exctbl[i].start_pc = READ2();
				a->code.exctbl[i].end_pc = READ2();
				a->code.exctbl[i].handler_pc = READ2();
				a->code.exctbl[i].catch_type = READ2();
			}
		}
		len -= 2 + 8*a->code.exctbl_len;
	} else if (!strcmp(aname, "Signature")) {
		a->tag = ATTRIB_Signature;
		a->signature = READ2();
		len -= 2;
	} else if (!strcmp(aname, "Exceptions")) {
		a->tag = ATTRIB_Exceptions;
	} else if (!strcmp(aname, "InnerClasses")) {
		a->tag = ATTRIB_InnerClasses;
	} else if (!strcmp(aname, "Synthetic")) {
		a->tag = ATTRIB_Synthetic;
	} else if (!strcmp(aname, "SourceFile")) {
		a->tag = ATTRIB_SourceFile;
	} else if (!strcmp(aname, "LineNumberTable")) {
		a->tag = ATTRIB_LineNumberTable;
	} else if (!strcmp(aname, "LocalVariableTable")) {
		a->tag = ATTRIB_LocalVariableTable;
	} else if (!strcmp(aname, "Deprecated")) {
		a->tag = ATTRIB_Deprecated;
	}
	SKIPN(len);
	return (a);
}

/* read and return method info */
static mf_info *read_fieldmethod(Stream *htio, ht_class_shared_data *shared)
{
	mf_info *m;
	u2 idx;
	classfile *clazz = shared->file;

	m = (mf_info *)calloc(1, sizeof (*m));
	if (!m) {
		return NULL;
	}
	m->offset = offset;
	m->flags = READ2();
	m->name = get_string(htio, clazz, READ2());
	m->desc = get_string(htio, clazz, READ2());
	m->attribs_count = idx = READ2();
	if (idx) {
		m->attribs = ht_malloc(idx * sizeof (*(m->attribs)));
		if (!m->attribs) {
			return NULL;
		}
		for (int i=0; i < (int)idx; i++) {
			m->attribs[i] = attribute_read(htio, clazz);
		}
	}
	return m;
}

/* read and return classfile */
ht_class_shared_data *class_read(File *htio)
{
	ht_class_shared_data *shared;
	classfile *clazz;
	u2 count;
	u2 cpcount, index;

	clazz = ht_malloc(sizeof (*clazz));
	if (!clazz) {
		return NULL;
	}
	shared = ht_malloc(sizeof (ht_class_shared_data));
	shared->file = clazz;
	shared->methods = new AVLTree(true);
	shared->fields = new Array(true);
	shared->valid = new Area();
	shared->valid->init();
	shared->initialized = new Area();
	shared->initialized->init();
	clazz->offset = offset = 0;
	clazz->magic  = READ4();
	if (clazz->magic != 0xCAFEBABE) {
		return NULL;
	}
	clazz->minor_version = READ2();
	clazz->major_version = READ2();
	count = clazz->cpool_count = READ2();
	clazz->cpool = ht_malloc((count+1) * sizeof (*(clazz->cpool)));
	if (!clazz->cpool) {
		return NULL;
	}
	cpcount = clazz->cpool_count;
	for (int i=1; i<(int)count; i++) {
		clazz->cpool[i] = read_cpool_entry(htio, clazz);
		if ((clazz->cpool[i]->tag == CONSTANT_Long) ||
			(clazz->cpool[i]->tag == CONSTANT_Double)) {
			i++;
		}
	}
	clazz->coffset = offset;
	clazz->access_flags = READ2();
	shared->flags = clazz->access_flags;
	index = READ2();
	clazz->this_class = index;
	index = READ2();
	clazz->super_class = index;
	count = clazz->interfaces_count = READ2();
	shared->classinfo.interfaces = NULL;
	shared->classinfo.thisclass = get_class_name(htio, clazz, clazz->this_class);
	if (strcmp(shared->classinfo.thisclass, "?") == 0) return NULL;
	shared->classinfo.superclass = get_class_name(htio, clazz, clazz->super_class);
	if (strcmp(shared->classinfo.superclass, "?") == 0) return NULL;
	if (count) {
		clazz->interfaces = ht_malloc(count * sizeof (*(clazz->interfaces)));
		if (!clazz->interfaces) return NULL;
		shared->classinfo.interfaces = new Array(true);
		for (int i=0; i < (int)count; i++) {
			index = READ2();
			clazz->interfaces[i] = index;
			shared->classinfo.interfaces->insert(new String(get_class_name(htio, clazz, index)));
		}
	} else {
		clazz->interfaces = 0;
	}
	clazz->foffset = offset;
	count = clazz->fields_count = READ2();
	if (count) {
		clazz->fields = ht_malloc(count * sizeof (*(clazz->fields)));
		if (!clazz->fields) {
			return NULL;
		}
		for (int i=0; i < (int)count; i++) {
			mf_info *m = read_fieldmethod(htio, shared);
			clazz->fields[i] = m;
			ClassField *cf = new ClassField(m->name, m->desc, m->flags);
			int acount = m->attribs_count;
			for (int j=0; j < acount; j++) {
				attrib_info *ai = m->attribs[j];
				if (ai->tag == ATTRIB_Signature) {
					cf->addsig(get_string(htio, shared->file, ai->signature));
				}
			}
			shared->fields->insert(cf);
		}
	} else {
		clazz->fields = 0;
	}
	clazz->moffset = offset;
	count = clazz->methods_count = READ2();
	if (count) {
		clazz->methods = ht_malloc(count * sizeof (*(clazz->methods)));
		if (!clazz->methods) {
			return NULL;
		}
		for (int i=0; i < (int)count; i++) {
			mf_info *m = read_fieldmethod(htio, shared);
			clazz->methods[i] = m;
			int acount = m->attribs_count;
			bool ok = false;
			ClassMethod *cm = NULL;
			for (int j=0; j < acount; j++) {
				attrib_info *ai = m->attribs[j];
				if (ai->tag == ATTRIB_Code) {
					cm = new ClassMethod(m->name, m->desc, 
						ai->code.start, ai->code.len, m->flags, 
						ai->code.exctbl_len, ai->code.exctbl);
					shared->methods->insert(cm);
					AddressFlat32 a1(ai->code.start);
					AddressFlat32 a2(ai->code.start+ai->code.len);
					shared->initialized->add(&a1, &a2);
					shared->valid->add(&a1, &a2);
					ok = true;
				} else if (ai->tag == ATTRIB_Signature && cm) {
					cm->addsig(get_string(htio, shared->file, ai->signature));
				}
			}
			if (!ok) {
				// fake abstract method
				ClassMethod *cm = new ClassMethod(m->name, m->desc, offset, 1, m->flags, 0, NULL);
				shared->methods->insert(cm);
				Address *a1 = new AddressFlat32(offset);
				Address *a2 = new AddressFlat32(offset+1);
				shared->valid->add(a1, a2);
				delete a1;
				delete a2;
			}
		}
	} else {
		clazz->methods = 0;
	}
	clazz->aoffset = offset;
	count = clazz->attribs_count = READ2();
	if (count) {
		clazz->attribs = ht_malloc(count*sizeof (*(clazz->attribs)));
		if (!clazz->attribs) {
			return NULL;
		}
		for (int i=0; i<(int)count; i++) {
			clazz->attribs[i] = attribute_read (htio, clazz);
			if (!clazz->attribs[i]) {
				return NULL;
			}
		}
	} else {
		clazz->attribs = 0;
	}
	return shared;
}

void class_unread(ht_class_shared_data *shared)
{
	u1 tag;
	classfile *clazz = shared->file;

	if (!clazz) return;
	for (uint i = 1; i < clazz->cpool_count; i++) {
		tag = clazz->cpool[i]->tag;
		if (tag == CONSTANT_Utf8) {
			free(clazz->cpool[i]->value.string);
		}
		free(clazz->cpool[i]);
		if (tag == CONSTANT_Long || tag == CONSTANT_Double) {
			i++;
		}
	}
	if (clazz->cpool_count) free(clazz->cpool);
	free(clazz->interfaces);
	for (uint i=0; i < clazz->fields_count; i++) {
		for (uint j=0; j < clazz->fields[i]->attribs_count; j++) {
			free(clazz->fields[i]->attribs[j]);
		}
		if (clazz->fields[i]->attribs_count) free(clazz->fields[i]->attribs);
		free(clazz->fields[i]);
	}
	if (clazz->fields_count) free(clazz->fields);
	for (uint i=0; i < clazz->methods_count; i++) {
		for (uint j=0; j<clazz->methods[i]->attribs_count; j++) {
			free (clazz->methods[i]->attribs[j]);
		}
		if (clazz->methods[i]->attribs_count) free(clazz->methods[i]->attribs);
		free(clazz->methods[i]);
	}
	if (clazz->methods_count) free(clazz->methods);
	for (uint i=0; i < clazz->attribs_count; i++) {
		free(clazz->attribs[i]);
	}
	if (clazz->attribs_count) {
		free(clazz->attribs);
	}
	if (shared->valid) {
		shared->valid->done();
		delete shared->valid;
	}
/*     if (shared->initialized) {
		shared->initialized->done();
		delete shared->initialized;
	}*/
	delete shared->methods;
	free(shared->file);
	free(shared);
}

int java_demangle_type(char *result, const char **type);

int java_demangle_generic(char *result, const char **type)
{
#if 0
 (Ljava/util/List<+Ljava/lang/Float;>;)V
 (Ljava/util/List<*>;)V
 (Ljava/util/Map<Ljava/lang/Integer;+Ljava/lang/Integer;>;)V
 <S:Ljava/lang/Object;>(Ljava/util/Map<Ljava/lang/Integer;+TS;>;)V
 <S:Ljava/lang/Object;B:Ljava/lang/Object;>(Ljava/util/Map<TS;TB;>;)V
 <S:Ljava/lang/Object;B:TS;>(Ljava/util/Map<TS;TB;>;)V
 <T:Ljava/lang/Object;:Ljava/util/List;>(Ljava/util/List<TT;>;)V
#endif
	char *old = result;
	*result++ = '<';
	(*type)++;

	goto first;
	do {
		*result++ = ',';
		*result++ = ' ';
	first:
		switch (**type) {
		case 0:
			*result = 0;
			return result-old;
		case '*':
			(*type)++;
			*result++ = '?'; 
			break;
		case '+':
		case '-':
			result += sprintf(result, "? %s ", **type=='+' ? "extends": "super");
			(*type)++;
			// fall through
		default:
			result += java_demangle_type(result, type);
		}
	} while (**type != '>');
	(*type)++;
	*result++ = '>';
	*result = 0;
	return result - old;
}

#define STRIP_PATH
int java_demangle_template(char *result, const char **type)
{
	char *old = result;
	*result++ = '<';
	(*type)++;

	goto first;
	do {
		*result++ = ',';
		*result++ = ' ';
	first:
		if (*type == 0) {
			*result = 0;
			return result - old;
		}
		do {
			*result++ = **type;
			(*type)++;
		} while (**type != ':' && **type != 0);
		result += sprintf(result, " extends ");
		goto first2;
		do {
			*result++ = ' ';
			*result++ = '&';
			*result++ = ' ';
		first2:
			if (*type == 0) {
				*result = 0;
				return result - old;
			}
			(*type)++;
			result += java_demangle_type(result, type);
		} while (**type == ':');
	} while (**type != '>');
	*result++ = '>';
	(*type)++;
	return result - old;
}

int java_demangle_type(char *result, const char **type)
{
	switch (*(*type)++) {
	case 0:
		*result = 0;
		(*type)--;
		return 0;
	case '[': {
		char temp[300];
		java_demangle_type(temp, type);
		return sprintf(result, "%s[]", temp);
	}
	case 'B':
		return sprintf(result, "byte");
	case 'C':
		return sprintf(result, "char");
	case 'D':
		return sprintf(result, "double");
	case 'F':
		return sprintf(result, "float");
	case 'I':
		return sprintf(result, "int");
	case 'J':
		return sprintf(result, "long");
	case 'T':
	case 'L': {
		char *oldresult = result;
		while (**type != ';' && **type != '<' && **type != 0) {
			*result = **type;
#ifdef STRIP_PATH
			if (*result == '/') result = oldresult; else
#endif
			result++;
			(*type)++;
		}
		if (**type == '<') {
			result += java_demangle_generic(result, type);
		}
		(*type)++;
		*result = 0;
		return result-oldresult;
	}
	case 'S':
		return sprintf(result, "short");
	case 'V':
		return sprintf(result, "void");
	case 'Z':
		return sprintf(result, "boolean");
	default:
		return sprintf(result, "%c", *(*type-1));
	}
}

char *java_demangle_flags(char *result, int flags)
{
	if (flags & jACC_PUBLIC) result += sprintf(result, "public ");
	if (flags & jACC_PRIVATE) result += sprintf(result, "private ");
	if (flags & jACC_PROTECTED) result += sprintf(result, "protected ");
	if (flags & jACC_STATIC) result += sprintf(result, "static ");
	if (flags & jACC_FINAL) result += sprintf(result, "final ");
	if (flags & jACC_SUPER) result += sprintf(result, "super ");
	if (flags & jACC_SYNCHRONIZED) result += sprintf(result, "synchronized ");
	if (flags & jACC_VOLATILE) result += sprintf(result, "volatile ");
	if (flags & jACC_TRANSIENT) result += sprintf(result, "transient ");
	if (flags & jACC_NATIVE) result += sprintf(result, "native ");
	if (flags & jACC_INTERFACE) result += sprintf(result, "interface ");
	if (flags & jACC_ABSTRACT) result += sprintf(result, "abstract ");
	if (flags & jACC_STRICT) result += sprintf(result, "strict ");
	return result;
}

static const char *java_strip_path(const char *name)
{
#ifdef STRIP_PATH
	const char *nname = strrchr(name, '/');
	return nname?(nname+1):name;
#else
	return name;
#endif
}

void java_demangle(char *result, const char *classname, const char *name, const char *type, int flags)
{
	result = java_demangle_flags(result, flags);
	name = java_strip_path(name);
	classname = java_strip_path(classname);
	strcpy(result, name);
	const char *ret = strchr(type, ')');
	if (!ret) return;
	if (*type == '<') {
		result += java_demangle_template(result, &type);
		*result++ = ' ';
		*result = 0;
	}
	if (*type != '(') return;
	ret++;
	result += java_demangle_type(result, &ret);
	if (strcmp(name, "<init>")==0) {
		name = classname;
	}
	result += sprintf(result, " %s::%s(", classname, name);
	type++;
	while (*type != ')') {
		result += java_demangle_type(result, &type);
		if (*type != ')') {
			result += sprintf(result, ", ");
		}
	}
	result += sprintf(result, ")");
}

void java_demangle_field(char *result, const char *name, const char *type, int flags)
{
	result = java_demangle_flags(result, flags);
	result += java_demangle_type(result, &type);
	*result++ = ' ';
	strcpy(result, name);
}

int token_translate(char *buf, int maxlen, uint32 token, ht_class_shared_data *shared)
{
	classfile *clazz = shared->file;
	char tag[20];
	char data[1024];
	char classname[1024];
	char name[1024];
	char type[1024];
	strcpy(tag, "?");
	strcpy(data, "?");
	strcpy(classname, "?");
	strcpy(name, "?");
	strcpy(type, "?");
	if (token < clazz->cpool_count)
	switch (clazz->cpool[token]->tag) {
	case CONSTANT_Class: {
		strcpy(tag, "Class");
		const char *cl = get_class_name(NULL, clazz, token);
		if (cl[0] == '[') {
			java_demangle_type(data, &cl);
		} else {
			strcpy(data, java_strip_path(cl));
		}
		break;
	}
	case CONSTANT_Double:
		strcpy(tag, "Double");
		sprintf(data, "double (%f)", clazz->cpool[token]->value.dval);
		break;
	case CONSTANT_Float:
		strcpy(tag, "Float");
		sprintf(data, "float (%f)", clazz->cpool[token]->value.fval);
		break;
	case CONSTANT_Integer:
		strcpy(tag, "Int");
		ht_snprintf(data, sizeof data, "int (%d)", clazz->cpool[token]->value.ival);
		break;
	case CONSTANT_Long: {
		strcpy(tag, "Long");
		uint64 v = (uint64(clazz->cpool[token]->value.llval[0]) << 32)
		                 | clazz->cpool[token]->value.llval[1];
		ht_snprintf(data, sizeof data, "long (%qd)", v);
		break;
	}
	case CONSTANT_String: {
		strcpy(tag, "String");
		char *d = data;
		*d++ = '\"';
		// FIXME: add "..." on too long strings
		d += escape_special_str(d, 256, get_string(NULL, clazz, clazz->cpool[token]->value.llval[0]), "\"", false);
		*d++ = '\"';
		*d = 0;
		break;
	}
	case CONSTANT_Fieldref: {
		strcpy(tag, "Field");
		get_name_and_type(NULL, clazz, clazz->cpool[token]->value.llval[1], name, type);
		char dtype[1024];
		const char *ttype=type;
		java_demangle_type(dtype, &ttype);
		ht_snprintf(data, sizeof data, "%s %s", dtype, name);
		break;
	}
	case CONSTANT_Methodref:
		strcpy(tag, "Method");
		strcpy(classname, get_class_name(NULL, clazz, clazz->cpool[token]->value.llval[0]));
		get_name_and_type(NULL, clazz, clazz->cpool[token]->value.llval[1], name, type);
		java_demangle(data, classname, name, type, 0);
		break;
	case CONSTANT_InterfaceMethodref:
		strcpy(tag, "InterfaceMethod");
		strcpy(classname, get_class_name(NULL, clazz, clazz->cpool[token]->value.llval[0]));
		get_name_and_type(NULL, clazz, clazz->cpool[token]->value.llval[1], name, type);
		java_demangle(data, classname, name, type, 0);
		break;
	}
//	return ht_snprintf(buf, maxlen, "<%s %s>", tag, data);
	return ht_snprintf(buf, maxlen, "<%s>", data);
}

