/*
 *	HT Editor
 *	classview.cc
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

#include <stdlib.h>

#include "class.h"
#include "classimg.h"
#include "atom.h"
#include "httag.h"
#include "snprintf.h"
#include "stream.h"

#define ATOM_CLS_ACCESS     0xcafebab0
#define ATOM_CLS_ACCESS_STR  "cafebab0"
#define ATOM_CLS_CPOOL      0xcafebab1
#define ATOM_CLS_CPOOL_STR   "cafebab1"

ht_tag_flags_s access_flags[] = 
{
	{ -1,  "access flags"},
	{8+0,  "[0x0001] public"},
	{8+1,  "[0x0002] private"},
	{8+2,  "[0x0004] protected"},
	{8+3,  "[0x0008] static"},
	{8+4,  "[0x0010] final"},
	{8+5,  "[0x0020] synchronized"},
	{8+6,  "[0x0040] volatile"},
	{8+7,  "[0x0080] transient"},
	{0,    "[0x0100] native"},
	{1,    "[0x0200] interface"},
	{2,    "[0x0400] abstract"},
	{3,    "[0x0800] strict"},
	{0, 0}
};
int_hash cpool_tags [] = 
{
	{ 1, "utf8"},
	{ 2, "unknown type"},
	{ 3, "integer"},
	{ 4, "float"},
	{ 5, "long"},
	{ 6, "double"},
	{ 7, "class"},
	{ 8, "string"},
	{ 9, "fieldref"},
	{10, "methodref"},
	{11, "interfacemethodref"},
	{12, "nameandtype"},
	{0, 0}
};
ht_mask_ptable cls_class1_hdr[] = 
{
	{"magic",                      STATICTAG_EDIT_DWORD_BE("00000000")},
	{"minor version",              STATICTAG_EDIT_WORD_BE ("00000004")},
	{"major version",              STATICTAG_EDIT_WORD_BE ("00000006")},
	{"constant pool count",        STATICTAG_EDIT_WORD_BE ("00000008")},
	{0, 0}
};
ht_mask_ptable cls_class2_hdr[] = 
{
	{"access flags",               STATICTAG_EDIT_WORD_BE ("00000000")
                                       " "STATICTAG_FLAGS("00000000", ATOM_CLS_ACCESS_STR)},
	{"this class",                 STATICTAG_EDIT_WORD_BE ("00000002")},
	{"super class",                STATICTAG_EDIT_WORD_BE ("00000004")},
	{"interfaces count",           STATICTAG_EDIT_WORD_BE ("00000006")},
	{0, 0}
};
ht_mask_ptable cpool_class[] = 
{
	{"name index",                 STATICTAG_EDIT_WORD_BE("00000001")},
	{0, 0}
};
ht_mask_ptable cpool_fmi[] =
{
	{"class index",                STATICTAG_EDIT_WORD_BE("00000001")},
	{"name and type index",        STATICTAG_EDIT_WORD_BE("00000003")},
	{0, 0}
};
ht_mask_ptable cpool_str[] =
{
	{"string index",               STATICTAG_EDIT_WORD_BE("00000001")},
	{0, 0}
};
ht_mask_ptable cpool_if[] = 
{
	{"bytes",                      STATICTAG_EDIT_DWORD_BE("00000001")},
	{0, 0}
};
ht_mask_ptable cpool_ld[] = 
{
	{"high bytes",                 STATICTAG_EDIT_DWORD_BE("00000001")},
	{"low bytes",                  STATICTAG_EDIT_DWORD_BE("00000005")},
	{0, 0}
};
ht_mask_ptable cpool_nat[] = 
{
	{"name index",                 STATICTAG_EDIT_WORD_BE("00000001")},
	{"descriptor index",           STATICTAG_EDIT_WORD_BE("00000003")},
	{0, 0}
};
ht_mask_ptable cpool_utf8[] = 
{
	{"length",                     STATICTAG_EDIT_WORD_BE("00000001")},
	{0, 0}
};
ht_mask_ptable cpool_hdr[] = 
{ 
	{"tag",                        STATICTAG_EDIT_BYTE("00000000")
" "STATICTAG_DESC_BYTE("00000000", ATOM_CLS_CPOOL_STR)},
	{0, 0}
};
ht_mask_ptable iface_hdr[] = 
{
	{"class name index",           STATICTAG_EDIT_WORD_BE("00000000")},
	{0, 0}
};
ht_mask_ptable field_hdr[] = 
{
	{"fields count",               STATICTAG_EDIT_WORD_BE("00000000")},
	{0, 0}
};
ht_mask_ptable method_hdr[] = 
{
	{"methods count",              STATICTAG_EDIT_WORD_BE("00000000")},
	{0, 0}
};
ht_mask_ptable mf_hdr[] = 
{
	{"access flags",               STATICTAG_EDIT_WORD_BE ("00000000")
" "STATICTAG_FLAGS("00000000", ATOM_CLS_ACCESS_STR)},
	{"name index",                 STATICTAG_EDIT_WORD_BE ("00000002")},
	{"descriptor index",           STATICTAG_EDIT_WORD_BE ("00000004")},
	{"attributes count",           STATICTAG_EDIT_WORD_BE ("00000006")},
	{0, 0}
};
ht_mask_ptable atr_hdr[] = 
{
	{"attributes count",           STATICTAG_EDIT_WORD_BE("00000000")},
	{0, 0}
};
ht_mask_ptable aexpt_hdr[] = 
{
	{"exception table length",     STATICTAG_EDIT_WORD_BE("00000000")},
	{0, 0}
};
ht_mask_ptable aexpt_info[] = 
{
	{"start pc",                   STATICTAG_EDIT_WORD_BE("00000000")},
	{"end pc",                     STATICTAG_EDIT_WORD_BE("00000002")},
	{"handler pc",                 STATICTAG_EDIT_WORD_BE("00000004")},
	{"catch type",                 STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
}; 
ht_mask_ptable ainn_info[] = 
{
	{"inner class info index",     STATICTAG_EDIT_WORD_BE("00000000")},
	{"outer class info index",     STATICTAG_EDIT_WORD_BE("00000002")},
	{"inner name index",           STATICTAG_EDIT_WORD_BE("00000004")},
	{"inner class access flags",   STATICTAG_EDIT_WORD_BE("00000006")
                                       " "STATICTAG_FLAGS("00000006", ATOM_CLS_ACCESS_STR)},
	{0, 0}
};
ht_mask_ptable aline_info[] = 
{
	{"start pc",                   STATICTAG_EDIT_WORD_BE("00000000")},
	{"line number",                STATICTAG_EDIT_WORD_BE("00000002")},
	{0, 0}
}; 
ht_mask_ptable aloc_info[] = 
{
	{"start pc",                   STATICTAG_EDIT_WORD_BE("00000000")},
	{"length",                     STATICTAG_EDIT_WORD_BE("00000002")},
	{"name index",                 STATICTAG_EDIT_WORD_BE("00000004")},
	{"descriptor index",           STATICTAG_EDIT_WORD_BE("00000006")},
	{"index",                      STATICTAG_EDIT_WORD_BE("00000008")},
	{0, 0}
};
ht_mask_ptable axpt_info[] = 
{
	{"exception",                  STATICTAG_EDIT_WORD_BE("00000000")},
	{0, 0}
};
ht_mask_ptable attrib_hdr[] = 
{
	{"attribute name index",       STATICTAG_EDIT_WORD_BE("00000000")},
	{"attribute length",           STATICTAG_EDIT_DWORD_BE("00000002")},
	{0, 0}
};
ht_mask_ptable aconst_hdr[] = 
{
	{"constantvalue index",        STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};
ht_mask_ptable acode_hdr[] = 
{
	{"max stack",                  STATICTAG_EDIT_WORD_BE("00000006")},
	{"max locals",                 STATICTAG_EDIT_WORD_BE("00000008")},
	{"code length",                STATICTAG_EDIT_DWORD_BE("0000000a")},
	{0, 0}
};
ht_mask_ptable axpt_hdr[] = 
{
	{"number of exceptions",       STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};
ht_mask_ptable ainn_hdr[] = 
{
	{"number of classes",          STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};
ht_mask_ptable asrc_hdr[] = 
{
	{"sourcefile index",           STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};
ht_mask_ptable alin_hdr[] = 
{
	{"line_number table length",   STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};
ht_mask_ptable aloc_hdr[] = 
{
	{"local variable table length",STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};
ht_mask_ptable asignature_hdr[] = 
{
	{"signature",                  STATICTAG_EDIT_WORD_BE("00000006")},
	{0, 0}
};

static void attrib_view(ht_group_sub *g, File *f,
		  unsigned *idx, classfile *c, attrib_info *a)
{
	ht_group_sub *g2, *g3;
	ht_mask_sub *s, *s2;
	ht_collapsable_sub *cs, *cs2;
	attrib_info *atr;
	u4 code_len = 0;
	u2 tabl_len = 0;
	u1 inp[4];
	unsigned i, j;
	char info[128];

	s = new ht_mask_sub();
	s->init(f, (*idx)++);
	s->add_staticmask_ptable(attrib_hdr, a->offset, true);
	g->insertsub(s);
	switch (a->tag) {
	case ATTRIB_SourceFile:
		s->add_staticmask_ptable(asrc_hdr, a->offset, true);
		break;
	case ATTRIB_Code:
		s->add_staticmask_ptable(acode_hdr, a->offset, true);
		j = a->offset + 10;
		f->seek(j);
		f->read(inp, 4);
		j += 4;
		code_len = (((((((u4)inp[0]<<8)|inp[1])<<8)|inp[2])<<8)|inp[3]);
		j += code_len;

		s2 = new ht_mask_sub();
		s2->init(f, (*idx)++);
		s2->add_staticmask_ptable(aexpt_hdr, j, true);
		g->insertsub(s2);
		f->seek(j);
		f->read(inp, 2);
		j += 2;
		tabl_len = (((u2)inp[0]<<8)|inp[1]);
		g2 = new ht_group_sub();
		g2->init(f);
		if (!tabl_len) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_mask("<none>");
			g2->insertsub(s2);
		}
		for (i=0; i < tabl_len; i++) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_staticmask_ptable(aexpt_info, j+i*8, true);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "exception table entry [%08x]:", i);
			cs->init(f, s2, 1, info, 1);
			g2->insertsub(cs);
		}
		j += tabl_len * 8;
		cs2 = new ht_collapsable_sub();
		cs2->init(f, g2, 1, "exception table", 1);
		g->insertsub(cs2);

		s2 = new ht_mask_sub();
		s2->init(f, (*idx)++);
		s2->add_staticmask_ptable(atr_hdr, j, true);
		g->insertsub(s2);
		f->seek(j);
		f->read(inp, 2);
		j += 2;
		tabl_len = (((u2)inp[0]<<8)|inp[1]);
		g2 = new ht_group_sub();
		g2->init(f);
		if (!tabl_len) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_mask("<none>");
			g2->insertsub(s2);
		}
		for (i=0; i<tabl_len; i++) {
			g3 = new ht_group_sub();
			g3->init(f);
			f->seek(j);
			atr = attribute_read(f, c);
			atr->offset = j;
			j += atr->len + 6;
			attrib_view(g3, f, idx, c, atr);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "attribute entry [%08x]: %s", i,
				c->cpool[atr->name]->value.string); 
			free(atr);
			cs->init(f, g3, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(f, g2, 1, "attributes", 1);
		g->insertsub(cs2);
		break;
	case ATTRIB_Signature:
		s->add_staticmask_ptable(asignature_hdr, a->offset, true);
		break;
	case ATTRIB_ConstantValue:
		s->add_staticmask_ptable(aconst_hdr, a->offset, true);
		break;
	case ATTRIB_LineNumberTable:
		s->add_staticmask_ptable(alin_hdr, a->offset, true);
		f->seek(a->offset+6);
		f->read(inp, 2);
		j = a->offset + 6 + 2;
		tabl_len = (((u2)inp[0]<<8)|inp[1]);
		g2 = new ht_group_sub();
		g2->init(f);
		if (!tabl_len) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_mask("<none>");
			g2->insertsub(s2);
		}
		for (i=0; i<tabl_len; i++) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_staticmask_ptable(aline_info, j+i*4, true);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "line number table entry [%08x]:", i);
			cs->init(f, s2, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(f, g2, 1, "line number table", 1);
		g->insertsub(cs2);
		break;
	case ATTRIB_InnerClasses:
		s->add_staticmask_ptable(ainn_hdr, a->offset, true);
		f->seek(a->offset+6);
		f->read(inp, 2);
		j = a->offset + 6 + 2;
		tabl_len = (((u2)inp[0]<<8)|inp[1]);
		g2 = new ht_group_sub();
		g2->init(f);
		if (!tabl_len) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_mask("<none>");
			g2->insertsub(s2);
		}
		for (i=0; i<tabl_len; i++) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_staticmask_ptable(ainn_info, j+i*8, true);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "classes entry [%08x]:", i);
			cs->init(f, s2, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(f, g2, 1, "classes", 1);
		g->insertsub(cs2);
		break;
	case ATTRIB_Exceptions:
		s->add_staticmask_ptable(axpt_hdr, a->offset, true);
		f->seek(a->offset+6);
		f->read(inp, 2);
		j = a->offset + 6 + 2;
		tabl_len = (((u2)inp[0]<<8)|inp[1]);
		g2 = new ht_group_sub();
		g2->init(f);
		if (!tabl_len) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_mask("<none>");
			g2->insertsub(s2);
		}
		for (i=0; i<tabl_len; i++) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_staticmask_ptable(ainn_info, j+i*2, true);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "exception index table entry [%08x]:", i);
			cs->init(f, s2, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(f, g2, 1, "exception index table", 1);
		g->insertsub(cs2);
		break;
	case ATTRIB_LocalVariableTable:
		s->add_staticmask_ptable(aloc_hdr, a->offset, true);
		f->seek(a->offset+6);
		f->read(inp, 2);
		j = a->offset + 6 + 2;
		tabl_len = (((u2)inp[0]<<8)|inp[1]);
		g2 = new ht_group_sub();
		g2->init(f);
		if (!tabl_len) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_mask("<none>");
			g2->insertsub(s2);
		}
		for (i=0; i<tabl_len; i++) {
			s2 = new ht_mask_sub();
			s2->init(f, (*idx)++);
			s2->add_staticmask_ptable(aloc_info, j+i*10, true);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "local variable table entry [%08x]:", i);
			cs->init(f, s2, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(f, g2, 1, "local variable table", 1);
		g->insertsub(cs2);
		break;
	case ATTRIB_Synthetic:
	case ATTRIB_Deprecated:
	default:
		break;
	}
}

static void mf_view(ht_group_sub *g, File *f,
	   unsigned *idx, classfile *c, mf_info *mf)
{
	ht_group_sub *g2, *g3;
	ht_mask_sub *s;
	ht_collapsable_sub *cs, *cs2;
	char info[128];

	s = new ht_mask_sub();
	s->init(f, (*idx)++);
	s->add_staticmask_ptable(mf_hdr, mf->offset, true);
	g->insertsub(s);

	g2 = new ht_group_sub();
	g2->init(f);
	if (!mf->attribs_count) {
		s = new ht_mask_sub();
		s->init(f, (*idx)++);
		s->add_mask("<none>");
		g2->insertsub(s);
	}
	for (uint i=0; i<mf->attribs_count; i++) {
		g3 = new ht_group_sub();
		g3->init(f);
		attrib_view(g3, f, idx, c, mf->attribs[i]);
		cs = new ht_collapsable_sub();
		ht_snprintf(info, sizeof info, "attribute entry [%08x]: %s", i, c->cpool[mf->attribs[i]->name]->value.string);
		cs->init(f, g3, 1, info, 1);
		g2->insertsub(cs);
	}
	cs2 = new ht_collapsable_sub();
	cs2->init(f, g2, 1, "attributes", 1);
	g->insertsub(cs2);
}

static ht_view *class_view(Bounds *b, File *file, ht_format_group *group)
{
	ht_mask_sub *s;
	ht_collapsable_sub *cs, *cs2;
	ht_group_sub *g, *g2, *g3;
	classfile *clazz;
	char info[128];
	unsigned i, j, idx = 0;

	clazz = ((ht_class_shared_data *)group->get_shared_data())->file;
	if (clazz) {
		ht_uformat_viewer *v = new ht_uformat_viewer();
		v->init(b, DESC_JAVA_HEADERS, VC_EDIT, file, group);
		registerAtom(ATOM_CLS_ACCESS, access_flags);
		registerAtom(ATOM_CLS_CPOOL,  cpool_tags);

		g = new ht_group_sub();
		g->init(file);
  
		s = new ht_mask_sub();
		s->init(file, idx++);
		s->add_staticmask_ptable(cls_class1_hdr, clazz->offset, true);
		g->insertsub(s);

		g2 = new ht_group_sub();
		g2->init(file);
		for (i=1; i < clazz->cpool_count; i++) {
			s = new ht_mask_sub();
			s->init(file, idx++);
			s->add_staticmask_ptable(cpool_hdr, clazz->cpool[i]->offset, true);
			switch (clazz->cpool[i]->tag) {
			case CONSTANT_Utf8:
				s->add_staticmask_ptable(cpool_utf8, clazz->cpool[i]->offset, true);
				break;
			case CONSTANT_Integer:
			case CONSTANT_Float:
				s->add_staticmask_ptable(cpool_if, clazz->cpool[i]->offset, true);
				break;
			case CONSTANT_Long:
			case CONSTANT_Double:
				s->add_staticmask_ptable(cpool_ld, clazz->cpool[i]->offset, true);
				break;
			case CONSTANT_Class:
				s->add_staticmask_ptable(cpool_class, clazz->cpool[i]->offset, true);
				break;
			case CONSTANT_String:
				s->add_staticmask_ptable(cpool_str, clazz->cpool[i]->offset, true);
				break;
			case CONSTANT_Fieldref:
			case CONSTANT_Methodref:
			case CONSTANT_InterfaceMethodref:
				s->add_staticmask_ptable(cpool_fmi, clazz->cpool[i]->offset, true);
				break;
			case CONSTANT_NameAndType:
				s->add_staticmask_ptable(cpool_nat, clazz->cpool[i]->offset, true);
				break;
			}
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "constant pool entry [%08x]: %s", i,
					(clazz->cpool[i]->tag == CONSTANT_Utf8) 
					? clazz->cpool[i]->value.string : "");
			cs->init(file, s, 1, info, 1);
			g2->insertsub(cs);
			if (clazz->cpool[i]->tag == CONSTANT_Long
			 || clazz->cpool[i]->tag == CONSTANT_Double) {
				i++;
			}
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(file, g2, 1, "constant pool", 1);
		g->insertsub(cs2);

		s = new ht_mask_sub();
		s->init(file, idx++);
		s->add_staticmask_ptable(cls_class2_hdr, clazz->coffset, true);
		g->insertsub(s);

		g2 = new ht_group_sub();
		g2->init(file);
		if (!clazz->interfaces_count) {
			s = new ht_mask_sub();
			s->init(file, idx++);
			s->add_mask("<none>");
			g2->insertsub(s);
		}
		for (i=0; i < clazz->interfaces_count; i++) {
			s = new ht_mask_sub();
			s->init(file, idx++);
			s->add_staticmask_ptable(iface_hdr, clazz->coffset+8+i*2, true);
			cs = new ht_collapsable_sub();
			j = clazz->cpool[clazz->interfaces[i]]->value.llval[0];
			ht_snprintf(info, sizeof info, "interface entry [%08x]: %s", i,
				clazz->cpool[j]->value.string);
			cs->init(file, s, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(file, g2, 1, "interfaces", 1);
		g->insertsub(cs2);

		s = new ht_mask_sub();
		s->init(file, idx++);
		s->add_staticmask_ptable(field_hdr, clazz->foffset, true);
		g->insertsub(s);
		g2 = new ht_group_sub();
		g2->init(file);
		if (!clazz->fields_count) {
			s = new ht_mask_sub();
			s->init(file, idx++);
			s->add_mask("<none>");
			g2->insertsub(s);
		}
		for (i=0; i < clazz->fields_count; i++) {
			g3 = new ht_group_sub();
			g3->init(file);
			mf_view(g3, file, &idx, clazz, clazz->fields[i]);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "field entry [%08x]: %s", i, clazz->fields[i]->name);
				cs->init(file, g3, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(file, g2, 1, "fields", 1);
		g->insertsub(cs2);

		s = new ht_mask_sub();
		s->init(file, idx++);
		s->add_staticmask_ptable(method_hdr, clazz->moffset, true);
		g->insertsub(s);
		g2 = new ht_group_sub();
		g2->init(file);
		if (!clazz->methods_count) {
			s = new ht_mask_sub();
			s->init(file, idx++);
			s->add_mask("<none>");
			g2->insertsub(s);
		}
		for (i=0; i<clazz->methods_count; i++) {
			g3 = new ht_group_sub();
			g3->init(file);
			mf_view(g3, file, &idx, clazz, clazz->methods[i]);
			cs = new ht_collapsable_sub();
			ht_snprintf(info, sizeof info, "method entry [%08x]: %s", i, clazz->methods[i]->name);
			cs->init(file, g3, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(file, g2, 1, "methods", 1);
		g->insertsub(cs2);

		s = new ht_mask_sub();
		s->init(file, idx++);
		s->add_staticmask_ptable(atr_hdr, clazz->aoffset, true);
		g->insertsub(s);
		g2 = new ht_group_sub();
		g2->init(file);
		if (!clazz->attribs_count) {
			s = new ht_mask_sub();
			s->init(file, idx++);
			s->add_mask("<none>");
			g2->insertsub(s);
		}
		for (i = 0; i < clazz->attribs_count; i++) {
			g3 = new ht_group_sub();
			g3->init(file);
			attrib_view(g3, file, &idx, clazz, clazz->attribs[i]);
			cs = new ht_collapsable_sub();
			j = clazz->attribs[i]->name;
			ht_snprintf(info, sizeof info, "attribute entry [%08x]: %s", i,
			    clazz->cpool[j]->value.string);
			cs->init(file, g3, 1, info, 1);
			g2->insertsub(cs);
		}
		cs2 = new ht_collapsable_sub();
		cs2->init(file, g2, 1, "attributes", 1);
		g->insertsub(cs2);

		v->insertsub(g);
		return v;
	} else {
		return NULL;
	}
}

void cview::init(Bounds *b, File *f, format_viewer_if **ifs,
		  ht_format_group *g, FileOfs header_ofs, void *shared)
{
	ht_format_group::init(b, VO_SELECTABLE | VO_BROWSABLE | VO_RESIZE, DESC_JAVA, f, false, true, 0, g);

	shared_data = shared;
	ht_format_group::init_ifs(ifs);
}

void cview::done()
{
	ht_format_group::done();
	ht_class_shared_data *clazz = (ht_class_shared_data *)shared_data;
	if (clazz) {
		class_unread(clazz);
	}
}

static format_viewer_if htcls_cview = {
	&class_view,
	0
};

static format_viewer_if *htcls_ifs[] = {
	&htcls_cview,
	&htclassimage_if,
	0
};

static ht_view *class_init(Bounds *b, File *file, ht_format_group *group)
{
	u1 magic[4];

	file->seek(0);
	file->read(magic, 4);
	if (magic[0] == 0xca && magic[1] == 0xfe
	 && magic[2] == 0xba && magic[3] == 0xbe) {
		file->seek(0);
		void *shared_data = (void*)class_read(file);
		if (!shared_data) return NULL;
		cview *c = new cview();
		c->init(b, file, htcls_ifs, group, 0, shared_data);
		return c;
	}
	return NULL;
}

format_viewer_if htcls_if = {
	&class_init,
	0
};

