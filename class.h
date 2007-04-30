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
#include "stddata.h"

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

struct cp_info {
	u4 offset;
	u1 tag;
	union {
		char   *string;
		double dval;
		float  fval;
		sint32 ival;
		uint32 llval[2];
	} value;
};

static const u2 ATTRIB_ConstantValue      =  1;
static const u2 ATTRIB_Code               =  2;
static const u2 ATTRIB_Exceptions         =  3;
static const u2 ATTRIB_InnerClasses       =  4;
static const u2 ATTRIB_Synthetic          =  5;
static const u2 ATTRIB_SourceFile         =  6;
static const u2 ATTRIB_LineNumberTable    =  7;
static const u2 ATTRIB_LocalVariableTable =  8;
static const u2 ATTRIB_Deprecated         =  9;
static const u2 ATTRIB_Signature          =  10;

struct exception_info {
	u2 start_pc;
	u2 end_pc;
	u2 handler_pc;
	u2 catch_type;
};

struct attrib_info {
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
			u2 exctbl_len;
			exception_info *exctbl;
		} code;
		u2 signature;
	};
};

/* mf_info */
struct mf_info {
	u4 offset;
	u2 flags;
	char *name;
	char *desc;
	u2 attribs_count;
	attrib_info **attribs;
};

/* classfile */
struct classfile {
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
};

struct ht_class_shared_data {
	Container	*methods;
	Container	*fields;
	classfile	*file;
	Area		*valid;
	Area		*initialized;
	int		flags;
	struct {
		char *thisclass;
		char *superclass;
		Container *interfaces;
	} classinfo;
};

extern ht_class_shared_data *class_read(File *);
extern void class_unread(ht_class_shared_data *);
extern attrib_info *attribute_read(Stream *, classfile *);

int token_translate(char *buf, int maxlen, uint32 token, ht_class_shared_data *shared);
void java_demangle(char *result, const char *classname, const char *name, const char *type, int flags);
void java_demangle_field(char *result, const char *name, const char *type, int flags);
char *java_demangle_flags(char *result, int flags);

class cview: public ht_format_group {
public:
	void init(Bounds *, File *, format_viewer_if **, ht_format_group *, FileOfs, void *shared);
	virtual void done();
};

#define ClassAddress uint32

class ClassMethod: public Object {
public:
	const char *name;
	const char *type;
	ClassAddress start;
	uint length;
	int flags;
	int exctbl_len;
	exception_info *exctbl;

			ClassMethod(char *name, char *type, ClassAddress start, uint length, int flags,
				int exctbl_len, exception_info *exctbl);
	virtual int	compareTo(const Object *obj) const;
		void	addsig(const char *s) { type = s; } 
};

class ClassField: public Object {
public:
	const char *name;
	const char *type;
	int flags;

			ClassField(char *name, char *type, int flags);
		void	addsig(const char *s) { type = s; } 
};


#define DESC_JAVA		"java - class file"
#define DESC_JAVA_HEADERS	"java/headers"
#define DESC_JAVA_IMAGE		"java/image"

extern format_viewer_if htcls_if;

#endif /* _CLASS_H */

