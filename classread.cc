/*
 *	java .class file parsing
 */

/*
 *	HT Editor
 *	classread.cc
 *
 *	Copyright (C) 2001 Stanley Gambarin <stanleyg76@yahoo.com>
 *	Copyright (C) 2002 Sebastian Biallas (sb@web-productions.de)
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

#include "stream.h"
#include "class.h"

#include <stdlib.h>
#include <string.h>

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
 

ClassMethodPosition::ClassMethodPosition(ClassAddress s, FILEOFS f, UINT l)
{
	start = s;
	filestart = f;
	length = l;
}

int ClassMethodPosition::compareTo(ClassMethodPosition *b)
{
	if ((start+length-1) < b->start) return -1;
	if (start > (b->start+b->length-1)) return 1;
	return 0;
}


int compare_keys_ClassMethodPosition(ht_data *key_a, ht_data *key_b)
{
	return ((ClassMethodPosition*)key_a)->compareTo((ClassMethodPosition*)key_b);
}

/* extract name from a utf8 constant pool entry */
static char *
get_string (ht_stream *htio, classfile *clazz, int index)
{
  return clazz->cpool[index]->value.string;
}

/* read and return constant pool entry */
static cp_info *
read_cpool_entry (ht_stream *htio, classfile *clazz)
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
attrib_info *
attribute_read (ht_stream *htio, classfile *clazz)
{
  attrib_info *a;
  u4 len;
  char *aname;

  a = (attrib_info *)malloc (sizeof (*a));
  if (!a) {
    return NULL;
  }
  a->offset    = offset;
  a->name      = READ2();
  a->len = len = READ4();

  aname = get_string (htio, clazz, a->name); 
  if (!strcmp (aname, "ConstantValue")) {
    a->tag = ATTRIB_ConstantValue;
  } else if (!strcmp (aname, "Code")) {
    a->tag = ATTRIB_Code;
  } else if (!strcmp (aname, "Exceptions")) {
    a->tag = ATTRIB_Exceptions;
  } else if (!strcmp (aname, "InnerClasses")) {
    a->tag = ATTRIB_InnerClasses;
  } else if (!strcmp (aname, "Synthetic")) {
    a->tag = ATTRIB_Synthetic;
  } else if (!strcmp (aname, "SourceFile")) {
    a->tag = ATTRIB_SourceFile;
  } else if (!strcmp (aname, "LineNumberTable")) {
    a->tag = ATTRIB_LineNumberTable;
  } else if (!strcmp (aname, "LocalVariableTable")) {
    a->tag = ATTRIB_LocalVariableTable;
  } else if (!strcmp (aname, "Deprecated")) {
    a->tag = ATTRIB_Deprecated;
  }
  SKIPN(len);
  return (a);
}

/* read and return method info */
static mf_info *
read_fieldmethod (ht_stream *htio, ht_class_shared_data *shared)
{
  mf_info *m;
  u2 idx;
  int i;
  classfile *clazz = shared->file;
  
  m = (mf_info *)calloc (1, sizeof (*m));
  if (!m) {
    return NULL;
  }
  m->offset   = offset;
  idx         = READ2();
  idx         = READ2();
  m->name     = get_string (htio, clazz, idx);
  idx         = READ2();
  m->desc     = get_string (htio, clazz, idx);
  idx         = READ2();
  m->attribs_count = idx;
  if (idx) {
    m->attribs = (attrib_info **)malloc (idx * sizeof (*(m->attribs)));
    if (!m->attribs) {
	 return NULL;
    }
    for (i=0; i<(int)idx; i++) {
	 m->attribs[i] = attribute_read (htio, clazz);
    }
  }
  return m;
}

/* read and return classfile */
ht_class_shared_data *
class_read (ht_stream *htio)
{
  ht_class_shared_data *shared;
  classfile *clazz;
  u2 count;
  u2 cpcount, index;
  int i;
  
  shared = (ht_class_shared_data *)malloc (sizeof (ht_class_shared_data));
  clazz = (classfile *)malloc (sizeof (*clazz));
  shared->file = clazz;
  shared->methods = new ht_dtree();
  shared->methods->init(compare_keys_ClassMethodPosition);
  if (!clazz) {
    return NULL;
  }
  clazz->offset = offset = 0;
  clazz->magic  = READ4();
  if (clazz->magic != 0xCAFEBABE) {
    return NULL;
  }
  clazz->minor_version = READ2();
  clazz->major_version = READ2();
  count = clazz->cpool_count = READ2();
  clazz->cpool = (cp_info **)malloc ((count+1) * sizeof (*(clazz->cpool)));
  if (!clazz->cpool) {
    return NULL;
  }
  cpcount = clazz->cpool_count;
  for (i=1; i<(int)count; i++) {
    clazz->cpool[i] = read_cpool_entry (htio, clazz);
    if ((clazz->cpool[i]->tag == CONSTANT_Long) ||
	   (clazz->cpool[i]->tag == CONSTANT_Double)) {
	 i++;
    }
  }
  clazz->coffset = offset;
  clazz->access_flags = READ2();
  index = READ2();
  clazz->this_class = index;
  index = READ2();
  clazz->super_class = index;
  count = clazz->interfaces_count = READ2();
  if (count) {
    clazz->interfaces = (u2 *)malloc (count * sizeof (*(clazz->interfaces)));
    if (!clazz->interfaces) {
	 return NULL;
    }
    for (i=0; i<(int)count; i++) {
	 index = READ2();
	 clazz->interfaces[i] = index;
    }
  } else {
    clazz->interfaces = 0;
  }
  clazz->foffset = offset;
  count = clazz->fields_count = READ2();
  if (count) {
    clazz->fields = (mf_info **)malloc (count * sizeof (*(clazz->fields)));
    if (!clazz->fields) {
	 return NULL;
    }
    for (i=0; i<(int)count; i++) {
	 clazz->fields[i] = read_fieldmethod (htio, shared);
    }
  } else {
    clazz->fields = 0;
  }
  clazz->moffset = offset;
  count = clazz->methods_count = READ2();
  if (count) {
    clazz->methods = (mf_info **)malloc (count * sizeof (*(clazz->methods)));
    if (!clazz->methods) {
	 return NULL;
    }
    for (i=0; i<(int)count; i++) {
	 clazz->methods[i] = read_fieldmethod (htio, shared);
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
    for (i=0; i<(int)count; i++) {
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

void
class_unread (ht_class_shared_data *shared)
{
  u1 tag;
  unsigned i, j;
  classfile *clazz = shared->file;

  if (!clazz) {
    return;
  }
  for (i=1; i<clazz->cpool_count; i++) {
    tag = clazz->cpool[i]->tag;
    free (clazz->cpool[i]);
    if ((tag == CONSTANT_Long) || (tag == CONSTANT_Double)) {
	 i++;
    }
  }
  if (clazz->cpool_count)
    free (clazz->cpool);
  if (clazz->interfaces) {
    free (clazz->interfaces);
  }
  for (i=0; i<clazz->fields_count; i++) {
    for (j=0; j<clazz->fields[i]->attribs_count; j++) {
	 free (clazz->fields[i]->attribs[j]);
    }
    if (clazz->fields[i]->attribs_count) 
	 free (clazz->fields[i]->attribs);
    free (clazz->fields[i]);
  }
  if (clazz->fields_count)
    free (clazz->fields);
  for (i=0; i<clazz->methods_count; i++) {
    for (j=0; j<clazz->methods[i]->attribs_count; j++) {
	 free (clazz->methods[i]->attribs[j]);
    }
    if (clazz->methods[i]->attribs_count) 
	 free (clazz->methods[i]->attribs);
    free (clazz->methods[i]);
  }
  if (clazz->methods_count)
    free (clazz->methods);
  for (i=0; i<clazz->attribs_count; i++) {
    free (clazz->attribs[i]);
  }
  if (clazz->attribs_count) {
    free (clazz->attribs);
  }
  free(shared);
}
