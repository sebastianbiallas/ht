/*
 *	HT Editor
 *	class.h
 *
 *	Copyright (C) 2001 Stanley Gambarin <stanleyg76@yahoo.com>
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

#ifndef _CLASS_H
#define _CLASS_H

#include "htformat.h"

typedef unsigned char  u1;
typedef unsigned int   u2;
typedef unsigned long  u4;

/* flags */
static const u2 jACC_PUBLIC                 = 0x0001;
static const u2 jACC_PRIVATE                = 0x0002;
static const u2 jACC_PROTECTED              = 0x0004;
static const u2 jACC_STATIC                 = 0x0008;
static const u2 jACC_FINAL                  = 0x0010;
static const u2 jACC_SUPER                  = 0x0020;
static const u2 jACC_SYNCHRONIZED           = 0x0020;
static const u2 jACC_VOLATILE               = 0x0040;
static const u2 jACC_TRANSIENT              = 0x0080;
static const u2 jACC_NATIVE                 = 0x0100;
static const u2 jACC_INTERFACE              = 0x0200;
static const u2 jACC_ABSTRACT               = 0x0400;
static const u2 jACC_STRICT                 = 0x0800;
static const u2 jACC_JNM                    = 0x4000;

/* constant pool tags */
static const u1 CONSTANT_Utf8               =  1;
static const u1 CONSTANT_Integer            =  3;
static const u1 CONSTANT_Float              =  4;
static const u1 CONSTANT_Long               =  5;
static const u1 CONSTANT_Double             =  6;
static const u1 CONSTANT_Class              =  7;
static const u1 CONSTANT_String             =  8;
static const u1 CONSTANT_Fieldref           =  9;
static const u1 CONSTANT_Methodref          = 10;
static const u1 CONSTANT_InterfaceMethodref = 11;
static const u1 CONSTANT_NameAndType        = 12;

typedef struct _cp_info {
  u4 offset;
  u1 tag;
  union {
    char   *string;
    double dval;
    float  fval;
    long   lval;
    int    ival;
    long   llval[2];
  } value;
} cp_info;

static const u2 ATTRIB_ConstantValue      =  1;
static const u2 ATTRIB_Code               =  2;
static const u2 ATTRIB_Exceptions         =  3;
static const u2 ATTRIB_InnerClasses       =  4;
static const u2 ATTRIB_Synthetic          =  5;
static const u2 ATTRIB_SourceFile         =  6;
static const u2 ATTRIB_LineNumberTable    =  7;
static const u2 ATTRIB_LocalVariableTable =  8;
static const u2 ATTRIB_Deprecated         =  9;

typedef struct _attrib_info {
	u4 offset;
	u2 tag;
	u2 name;
	u4 len;
	union {
          struct {
			u2 max_stack;
			u2 max_locals;
               u4 len;
               u4 start;
          } code;
	};
} attrib_info;

/* mf_info */
typedef struct _mf_info {
  u4 offset;
  char *name;
  char *desc;
  u2 attribs_count;
  attrib_info **attribs;
} mf_info;

/* classfile */
typedef struct _classfile {
  u4 offset;
  u4 magic;
  u2 minor_version;
  u2 major_version;
  u2 cpool_count;
  cp_info **cpool;
  u4 coffset;
  u2 access_flags;
  u2 this_class;
  u2 super_class;
  u2 interfaces_count;
  u2 *interfaces;
  u4 foffset;
  u2 fields_count;
  mf_info **fields;
  u4 moffset;
  u2 methods_count;
  mf_info **methods;
  u4 aoffset;
  u2 attribs_count;
  attrib_info **attribs;
} classfile;

struct ht_class_shared_data {
	ht_stree	*methods;
     ht_mem_file *image;
	classfile	*file;
     struct {
     	char *thisclass;
          char *superclass;
          ht_list *interfaces;
     } classinfo;
};

extern ht_class_shared_data *class_read(ht_streamfile *);
extern void class_unread (ht_class_shared_data *);
extern attrib_info *attribute_read (ht_stream *, classfile *);

class cview : public ht_format_group {
public:
  void init(bounds *, ht_streamfile *, format_viewer_if **,
		  ht_format_group *, FILEOFS);
  virtual void done();
};

#define ClassAddress dword

class ClassMethod: public ht_data {
public:
	char *name;
     ClassAddress start;
     FILEOFS filestart;
     UINT length;
				ClassMethod(char *name, ClassAddress start, FILEOFS filestart, UINT length);
	virtual		~ClassMethod();
	virtual int	compareTo(Object *o);
};

int compare_keys_ClassMethodPosition(ht_data *key_a, ht_data *key_b);

#define DESC_JAVA			"java - class file"
#define DESC_JAVA_HEADERS	"java/headers"
#define DESC_JAVA_IMAGE		"java/image"

extern format_viewer_if htcls_if;

#endif /* _CLASS_H */

