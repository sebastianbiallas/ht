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
#include "htstring.h"
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

ClassMethod::ClassMethod(char *n, char *d, ClassAddress s, UINT l, int f)
{
     name = ht_strdup(n);
     start = s;
     type = d;
	length = l;
	flags = f;
}

ClassMethod::~ClassMethod()
{
	free(name);
}

int ClassMethod::compareTo(Object *o)
{
	if ((start+length-1) < ((ClassMethod*)o)->start) return -1;
	if (start > (((ClassMethod*)o)->start+((ClassMethod*)o)->length-1)) return 1;
	return 0;
}


/* extract name from a utf8 constant pool entry */
static char *get_string(ht_stream *htio, classfile *clazz, UINT index)
{
	return (index < clazz->cpool_count) ? clazz->cpool[index]->value.string : (char*)"?";
}

/* extract name from a utf8 constant pool class entry */
static char *get_class_name(ht_stream *htio, classfile *clazz, UINT index)
{
	return (index < clazz->cpool_count) ? get_string(htio, clazz, clazz->cpool[index]->value.llval[0]): (char*)"?";
}

/* extract name from a utf8 constant pool class entry */
static void get_name_and_type(ht_stream *htio, classfile *clazz, UINT index, char *name, char *type)
{
	strcpy(name, (index < clazz->cpool_count) ? get_string(htio, clazz, clazz->cpool[index]->value.llval[0]) : "?");
	strcpy(type, (index < clazz->cpool_count) ? get_string(htio, clazz, clazz->cpool[index]->value.llval[1]) : "?");
}

/* read and return constant pool entry */
static cp_info *read_cpool_entry (ht_stream *htio, classfile *clazz)
{
	cp_info *cp;
	u2 idx;

	cp         = (cp_info *)malloc (sizeof (*cp));
	cp->offset = offset;
	cp->tag    = READ1();
	switch (cp->tag) {
		case CONSTANT_Utf8:
			idx = READ2();
			cp->value.string = (char *)malloc (idx+1);
			cls_read (cp->value.string, idx, 1, htio);
			cp->value.string[idx] = 0;
			break;
		case CONSTANT_Integer:
		case CONSTANT_Float:
			cp->value.fval = READ4();
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
attrib_info *attribute_read(ht_stream *htio, classfile *clazz)
{
	attrib_info *a;
	u4 len;
	char *aname;

	a = (attrib_info *)malloc(sizeof (*a));
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
          len -= 2+2+4;
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
static mf_info *read_fieldmethod (ht_stream *htio, ht_class_shared_data *shared)
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
		m->attribs = (attrib_info **)malloc(idx * sizeof (*(m->attribs)));
		if (!m->attribs) {
			return NULL;
		}
		for (int i=0; i<(int)idx; i++) {
			m->attribs[i] = attribute_read(htio, clazz);
		}
	}
	return m;
}

/* read and return classfile */
ht_class_shared_data *class_read(ht_streamfile *htio)
{
	ht_class_shared_data *shared;
	classfile *clazz;
	u2 count;
	u2 cpcount, index;

	clazz = (classfile *)malloc(sizeof (*clazz));
	if (!clazz) {
		return NULL;
	}
	shared = (ht_class_shared_data *)malloc(sizeof (ht_class_shared_data));
	shared->file = clazz;
	shared->methods = new ht_stree();
	shared->methods->init(compare_keys_ht_data);
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
	clazz->cpool = (cp_info **)malloc((count+1) * sizeof (*(clazz->cpool)));
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
     shared->classinfo.superclass = get_class_name(htio, clazz, clazz->super_class);
	if (count) {
		clazz->interfaces = (u2 *)malloc(count * sizeof (*(clazz->interfaces)));
		if (!clazz->interfaces) {
			return NULL;
		}
          shared->classinfo.interfaces = new ht_clist();
          ((ht_clist*)shared->classinfo.interfaces)->init();
		for (int i=0; i<(int)count; i++) {
			index = READ2();
			clazz->interfaces[i] = index;
               shared->classinfo.interfaces->append(new ht_data_string(get_class_name(htio, clazz, index)));
		}
	} else {
		clazz->interfaces = 0;
	}
	clazz->foffset = offset;
	count = clazz->fields_count = READ2();
	if (count) {
		clazz->fields = (mf_info **)malloc(count * sizeof (*(clazz->fields)));
	if (!clazz->fields) {
		return NULL;
	}
	for (int i=0; i<(int)count; i++) {
			clazz->fields[i] = read_fieldmethod(htio, shared);
		}
	} else {
		clazz->fields = 0;
	}
	clazz->moffset = offset;
	count = clazz->methods_count = READ2();
	if (count) {
		clazz->methods = (mf_info **)malloc(count * sizeof (*(clazz->methods)));
		if (!clazz->methods) {
			return NULL;
		}
		for (int i=0; i<(int)count; i++) {
			clazz->methods[i] = read_fieldmethod(htio, shared);
               int acount = clazz->methods[i]->attribs_count;
               bool ok = false;
               for (int j=0; j < acount; j++) {
                    attrib_info *ai = clazz->methods[i]->attribs[j];
                    if (ai->tag == ATTRIB_Code) {
                    	ClassMethod *cm = new ClassMethod(clazz->methods[i]->name, clazz->methods[i]->desc, ai->code.start, ai->code.len, clazz->methods[i]->flags);
                         shared->methods->insert(cm, NULL);
	                    Address *a1 = new AddressFlat32(ai->code.start);
	                    Address *a2 = new AddressFlat32(ai->code.start+ai->code.len);
                         shared->initialized->add(a1, a2);
                         shared->valid->add(a1, a2);
	                    delete a1;
	                    delete a2;
                         ok = true;
                    }
               }
               if (!ok) {
               	// fake abstract method
				ClassMethod *cm = new ClassMethod(clazz->methods[i]->name, clazz->methods[i]->desc, offset, 1, clazz->methods[i]->flags);
				shared->methods->insert(cm, NULL);
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
		clazz->attribs = (attrib_info **)malloc (count*sizeof (*(clazz->attribs)));
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
	for (UINT i=1; i<clazz->cpool_count; i++) {
		tag = clazz->cpool[i]->tag;
		free (clazz->cpool[i]);
		if ((tag == CONSTANT_Long) || (tag == CONSTANT_Double)) {
			i++;
		}
	}
	if (clazz->cpool_count) free(clazz->cpool);
	if (clazz->interfaces) free(clazz->interfaces);
	for (UINT i=0; i<clazz->fields_count; i++) {
		for (UINT j=0; j<clazz->fields[i]->attribs_count; j++) {
			free(clazz->fields[i]->attribs[j]);
		}
		if (clazz->fields[i]->attribs_count) free(clazz->fields[i]->attribs);
		free(clazz->fields[i]);
	}
	if (clazz->fields_count) free(clazz->fields);
	for (UINT i=0; i<clazz->methods_count; i++) {
		for (UINT j=0; j<clazz->methods[i]->attribs_count; j++) {
			free (clazz->methods[i]->attribs[j]);
		}
		if (clazz->methods[i]->attribs_count) free(clazz->methods[i]->attribs);
		free(clazz->methods[i]);
	}
	if (clazz->methods_count) free(clazz->methods);
	for (UINT i=0; i<clazz->attribs_count; i++) {
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
	free(shared);
}

#define STRIP_PATH

int java_demangle_type(char *result, char **type)
{
	switch (*((*type)++)) {
     	case '[': {
          	char temp[200];
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
     	case 'L': {
               char *oldresult = result;
               while (**type != ';') {
               	*result = **type;
#ifdef STRIP_PATH
				if (*result == '/') result = oldresult; else
#endif
                    result++;
                    (*type)++;
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
//     if (flags & jACC_SUPER)
//     if (flags & jACC_SYNCHRONIZED)
//     if (flags & jACC_VOLATILE)
//     if (flags & jACC_TRANSIENT)
     if (flags & jACC_NATIVE) result += sprintf(result, "native ");
//     if (flags & jACC_INTERFACE)
     if (flags & jACC_ABSTRACT) result += sprintf(result, "abstract ");
//     if (flags & jACC_STRICT)
	return result;
}

static char *java_strip_path(char *name)
{
#ifdef STRIP_PATH
	char *nname = strrchr(name, '/');
     return nname?(nname+1):name;
#else
     return name;
#endif
}

void java_demangle(char *result, char *classname, char *name, char *type, int flags)
{
	result = java_demangle_flags(result, flags);
	name = java_strip_path(name);
	classname = java_strip_path(classname);
	strcpy(result, name);
     if (*type != '(') return;
     char *ret = strchr(type, ')');
     if (!ret) return;
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

int token_translate(char *buf, int maxlen, dword token, ht_class_shared_data *shared)
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
		case CONSTANT_Class:
          	strcpy(tag, "Class");
               strcpy(data, java_strip_path(get_class_name(NULL, clazz, token)));
			break;
		case CONSTANT_Double:
          	strcpy(tag, "Double");
               sprintf(data, "double");
			break;
		case CONSTANT_Float:
          	strcpy(tag, "Float");
               sprintf(data, "%f", (float)clazz->cpool[token]->value.llval[0]);
			break;
		case CONSTANT_Integer:
          	strcpy(tag, "Int");
               sprintf(data, "0x%lx", clazz->cpool[token]->value.llval[0]);
			break;
		case CONSTANT_Long:
          	strcpy(tag, "Long");
               sprintf(data, "long");
			break;
		case CONSTANT_String: {
          	strcpy(tag, "String");
               char *d = data;
               *(d++) = '\"';
               // FIXME: add "..." on too long strings
			d += escape_special_str(d, 256, get_string(NULL, clazz, clazz->cpool[token]->value.llval[0]), "\"", false);
               *(d++) = '\"';
               *d = 0;
			break;
          }
		case CONSTANT_Fieldref: {
          	strcpy(tag, "Field");
               get_name_and_type(NULL, clazz, clazz->cpool[token]->value.llval[1], name, type);
               char dtype[1024];
               char *ttype=type;
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
//     return ht_snprintf(buf, maxlen, "<%s %s>", tag, data);
	return ht_snprintf(buf, maxlen, "<%s>", data);
}

