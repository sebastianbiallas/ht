/* 
 *	HT Editor
 *	htpeexp.cc
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "htapp.h"
#include "htctrl.h"
#include "htdata.h"
#include "htdebug.h"
#include "htendian.h"
#include "htiobox.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpe.h"
#include "htpeexp.h"
#include "htstring.h"
#include "httag.h"
#include "stream.h"
#include "tools.h"
#include "formats.h"

#include <stdlib.h>
#include <string.h>

ht_view *htpeexports_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32) return 0;

	dword sec_rva, sec_size;
	sec_rva = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].address;
	sec_size = pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].size;
	if (!sec_rva || !sec_size) return 0;

	int h0=new_timer();
	start_timer(h0);

	ht_group *g;
	dword *efunct=NULL, *enamet=NULL;
	word *eordt=NULL;
	bounds c;
	ht_statictext *head;
	char *filename = file->get_filename();

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, "pe/exports-g");

	c.y++;
	c.h--;
	ht_pe_export_viewer *v=new ht_pe_export_viewer();
	v->init(&c, DESC_PE_EXPORTS, VC_SEARCH, file, group);
	ht_pe_export_mask *m=0;

	c.y--;
	c.h=1;
	
	ht_mem_file *efile;
	ht_streamfile *origfile = file;
	char *esectionbuf;
	char eline[256];
	FILEOFS ename_ofs;
	char *ename;
	bool *lord;

/* get export directory offset */
	/* 1. get export directory rva */
	FILEOFS eofs;
	dword erva=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].address;
	dword esize=pe_shared->pe32.header_nt.directory[PE_DIRECTORY_ENTRY_EXPORT].size;
	/* 2. transform it into an offset */
	if (!pe_rva_to_ofs(&pe_shared->sections, erva, &eofs)) goto pe_read_error;
	LOG("%s: PE: reading export directory at offset %08x, rva %08x, size %08x...", filename, eofs, erva, esize);

/* make a memfile out of this section */
	efile=new ht_mem_file();
	efile->init(eofs, esize);
	esectionbuf=(char*)malloc(esize);
	file->seek(eofs);
	file->read(esectionbuf, esize);
	efile->write(esectionbuf, esize);
	file=efile;
	delete esectionbuf;
/* read export directory header */
	PE_EXPORT_DIRECTORY edir;
	file->seek(eofs);
	file->read(&edir, sizeof edir);
	create_host_struct(&edir, PE_EXPORT_DIRECTORY_struct, little_endian);
/* get export filename */
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.name_address, &ename_ofs)) goto pe_read_error;
	file->seek(ename_ofs);
	ename=fgetstrz(file);
/* read in function entrypoint table */
	FILEOFS efunct_ofs;
	efunct=(dword*)malloc(edir.function_count*sizeof *efunct);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.function_table_address, &efunct_ofs)) goto pe_read_error;
	file->seek(efunct_ofs);
	file->read(efunct, edir.function_count*sizeof *efunct);
	for (UINT i=0; i<edir.function_count;i++) {
		efunct[i] = create_host_int(efunct+i, sizeof *efunct, little_endian);
	}
/* read in name address table */
	FILEOFS enamet_ofs;
	enamet=(dword*)malloc(edir.name_count*sizeof *enamet);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.name_table_address, &enamet_ofs)) goto pe_read_error;
	file->seek(enamet_ofs);
	file->read(enamet, edir.name_count*sizeof *enamet);
	for (UINT i=0; i<edir.name_count; i++) {
		enamet[i] = create_host_int(enamet+i, sizeof *enamet, little_endian);
	}
/* read in ordinal table */
	FILEOFS eordt_ofs;
	eordt=(word*)malloc(edir.name_count*sizeof *eordt);
	if (!pe_rva_to_ofs(&pe_shared->sections, edir.ordinal_table_address, &eordt_ofs)) goto pe_read_error;
	file->seek(eordt_ofs);
	file->read(eordt, edir.name_count*sizeof *eordt);
	for (UINT i=0; i<edir.name_count; i++) {
		eordt[i] = create_host_int(eordt+i, sizeof *eordt, little_endian);
	}

	lord=(bool*)malloc(sizeof *lord*edir.function_count);
	memset(lord, 0, sizeof *lord*edir.function_count);
	for (UINT i=0; i < edir.name_count; i++) {
		if (eordt[i] < edir.function_count) {
			lord[eordt[i]]=true;
		}
	}

/* exports by ordinal */
	for (UINT i=0; i<edir.function_count; i++) {
		if (lord[i]) continue;

		dword f=efunct[i]+pe_shared->pe32.header_nt.image_base;

		ht_pe_export_function *efd=new ht_pe_export_function(f, i+edir.ordinal_base);
		pe_shared->exports.funcs->insert(efd);
	}
	free(lord);
/* exports by name */
	for (UINT i=0; i < edir.name_count; i++) {
		dword o=eordt[i];
		dword f=efunct[o]+pe_shared->pe32.header_nt.image_base;
		FILEOFS en;
		if (!pe_rva_to_ofs(&pe_shared->sections, *(enamet+i), &en)) goto pe_read_error;
		file->seek(en);
		char *s=fgetstrz(file);

		ht_pe_export_function *efd=new ht_pe_export_function(f, o+edir.ordinal_base, s);
		pe_shared->exports.funcs->insert(efd);
		delete s;
	}

	sprintf(eline, "Ord  VA       Name      %s exports %d functions", ename, edir.function_count);
	head=new ht_statictext();
	head->init(&c, eline, align_left);

	delete ename;

	sprintf(eline, "* PE export directory at offset %08x", eofs);

	m=new ht_pe_export_mask();
	m->init(file, &pe_shared->exports, eline);

	v->insertsub(m);

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	stop_timer(h0);
//	LOG("%s: PE: %d ticks (%d msec) to read exports", filename, get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

	efile->done();
	delete efile;

	if (efunct) free(efunct);
	if (enamet) free(enamet);
	if (eordt) free(eordt);
	pe_shared->v_exports=v;
	return g;
pe_read_error:
	delete_timer(h0);
	errorbox("%s: PE export directory seems to be corrupted.", origfile->get_filename());
	if (efunct) free(efunct);
	if (enamet) free(enamet);
	if (eordt) free(eordt);
	g->done();
	delete g;
	if (m) {
		m->done();
		delete m;
	}		
	v->done();
	delete v;
	return 0;
}

format_viewer_if htpeexports_if = {
	htpeexports_init,
	0
};

/*
 *	CLASS ht_pe_export_viewer
 */

void ht_pe_export_viewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *group)
{
	ht_uformat_viewer::init(b, desc, caps, file, group);
	VIEW_DEBUG_NAME("ht_pe_export_viewer");
}

void ht_pe_export_viewer::done()
{
	ht_uformat_viewer::done();
}

char *ht_pe_export_viewer::func(UINT i, bool execute)
{
	switch (i) {
		case 2:
			if (execute) {
				((ht_pe_export_mask*)cursor_sub)->sortbyord();
				dirtyview();
			}
			return "sortord";
		case 3:
			if (execute) {
				((ht_pe_export_mask*)cursor_sub)->sortbyva();
				dirtyview();
			}
			return "sortva";
		case 4:
			if (execute) {
				((ht_pe_export_mask*)cursor_sub)->sortbyname();
				dirtyview();
			}
			return "sortname";
		case 5:
			if (execute) {
				((ht_pe_export_mask*)cursor_sub)->unsort();
				dirtyview();
			}
			return "unsort";
	}
	return ht_uformat_viewer::func(i, execute);
}

int ht_pe_export_viewer::ref_sel(ID id_low, ID id_high)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)format_group->get_shared_data();

	if (pe_shared->v_image->goto_address(id_low, this)) {
		app->focus(pe_shared->v_image);
	} else errorbox("can't follow: %s %08x is not valid !", "export address", id_low);
	return 1;
}

/*
 *	CLASS ht_pe_export_mask
 */

void ht_pe_export_mask::init(ht_streamfile *file, ht_pe_export *_exp, char *_firstline)
{
	ht_sub::init(file);
	firstline=ht_strdup(_firstline);
	exp=_exp;
	sort_path=0;
	sort_va=0;
	sort_name=0;
}

void ht_pe_export_mask::done()
{
	delete firstline;
	if (sort_va) delete sort_va;
	if (sort_name) delete sort_name;
	ht_sub::done();
}

bool ht_pe_export_mask::convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2)
{
	*id2 = 0;
	*id1 = addr;
	return true;
}

bool ht_pe_export_mask::convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr)
{
	*addr = id1;
	return true;
}

void ht_pe_export_mask::first_line_id(ID *id1, ID *id2)
{
	*id1=0;
	*id2=0;
}

bool ht_pe_export_mask::getline(char *line, ID id1, ID id2)
{
	if (id1>=1) {
		char *l=line;
		UINT i=id1-1;
		if (sort_path) i=sort_path[i];
		ht_pe_export_function *f=(ht_pe_export_function*)exp->funcs->get(i);

		l+=sprintf(l, "%04x %08x ", f->ordinal, f->address);
		if (f->byname) {
			l=tag_make_ref(l, f->address, 0, f->name);
		} else {
			l=tag_make_ref(l, f->address, 0, "<ordinal only>");
		}
		*l=0;
	} else {
		strcpy(line, firstline);
	}
	return 1;
}

void ht_pe_export_mask::last_line_id(ID *id1, ID *id2)
{
	*id1=exp->funcs->count();
	*id2=0;
}

int ht_pe_export_mask::next_line_id(ID *id1, ID *id2, int n)
{
	UINT c=exp->funcs->count()+1;
	if (*id1+n>=c) {
		int r=c-*id1-1;
		*id1=c-1;
		return r;
	}
	*id1+=n;
	return n;
}

int ht_pe_export_mask::prev_line_id(ID *id1, ID *id2, int n)
{
	if ((UINT)n>*id1) {
		int r=*id1;
		*id1=0;
		return r;
	}
	*id1-=n;
	return n;
}

ht_pe_export *g_export;

int compare_export_ord(UINT *i1, UINT *i2)
{
	ht_pe_export_function *f1=(ht_pe_export_function *)g_export->funcs->get(*i1);
	ht_pe_export_function *f2=(ht_pe_export_function *)g_export->funcs->get(*i2);
	if (f1->ordinal>f2->ordinal) return 1; else
	if (f1->ordinal<f2->ordinal) return -1; else
		return 0;
}

void ht_pe_export_mask::sortbyord()
{
	if (!sort_va) {
		int c=exp->funcs->count();
		sort_ord=(UINT*)malloc(sizeof (UINT) * c);
		for (int i=0; i<c; i++) {
			sort_ord[i]=i;
		}
		g_export=exp;
		qsort(sort_ord, c, sizeof (UINT), (int (*)(const void *, const void*))compare_export_ord);
	}
	sort_path=sort_ord;
}

int compare_export_va(UINT *i1, UINT *i2)
{
	ht_pe_export_function *f1=(ht_pe_export_function *)g_export->funcs->get(*i1);
	ht_pe_export_function *f2=(ht_pe_export_function *)g_export->funcs->get(*i2);
	if (f1->address>f2->address) return 1; else
	if (f1->address<f2->address) return -1; else
		return 0;
}

void ht_pe_export_mask::sortbyva()
{
	if (!sort_va) {
		int c=exp->funcs->count();
		sort_va=(UINT*)malloc(sizeof (UINT) * c);
		for (int i=0; i<c; i++) {
			sort_va[i]=i;
		}
		g_export=exp;
		qsort(sort_va, c, sizeof (UINT), (int (*)(const void *, const void*))compare_export_va);
	}
	sort_path=sort_va;
}

int compare_export_name(UINT *i1, UINT *i2)
{
	ht_pe_export_function *f1=(ht_pe_export_function *)g_export->funcs->get(*i1);
	ht_pe_export_function *f2=(ht_pe_export_function *)g_export->funcs->get(*i2);
	if (f1->byname) {
		if (f2->byname) {
			return strcmp(f1->name, f2->name);
		} else {
			return 1;
		}
	} else {
		if (f2->byname) {
			return -1;
		} else {
			if (f1->ordinal>f2->ordinal) return 1; else
			if (f1->ordinal<f2->ordinal) return -1; else
				return 0;
		}
	}
}

void ht_pe_export_mask::sortbyname()
{
	if (!sort_name) {
		int c=exp->funcs->count();
		sort_name=(UINT*)malloc(sizeof (UINT) * c);
		for (int i=0; i<c; i++) {
			sort_name[i]=i;
		}
		g_export=exp;
		qsort(sort_name, c, sizeof (UINT), (int (*)(const void *, const void*))compare_export_name);
	}
	sort_path=sort_name;
}

void ht_pe_export_mask::unsort()
{
	sort_path=0;
}

/*
 *	class ht_pe_export_function
 */

ht_pe_export_function::ht_pe_export_function()
{
	name=0;
	ordinal=0;
	address=0;
}

ht_pe_export_function::ht_pe_export_function(int _address, int _ordinal)
{
	ordinal=_ordinal;
	address=_address;
	byname=0;
}

ht_pe_export_function::ht_pe_export_function(int _address, int _ordinal, char *_name)
{
	ordinal=_ordinal;
	name=ht_strdup(_name);
	address=_address;
	byname=1;
}

ht_pe_export_function::~ht_pe_export_function()
{
	if ((byname) && (name)) delete name;
}

