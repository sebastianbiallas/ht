/*
 *	HT Editor
 *	htpeimg.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#include "log.h"
#include "formats.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpeimg.h"
#include "strtools.h"
#include "pe_analy.h"
#include "pestruct.h"
#include "snprintf.h"

static ht_view *htpeimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32 && pe_shared->opt_magic!=COFF_OPTMAGIC_PE64) return 0;

	bool pe32 = (pe_shared->opt_magic==COFF_OPTMAGIC_PE32);

	String fn;
	LOG("%y: PE: loading image (starting analyser)...", &file->getFilename(fn));
	PEAnalyser *p = new PEAnalyser();
	p->init(pe_shared, file);

	Bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_PE_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_pe_aviewer *v=new ht_pe_aviewer();
	v->init(&c, DESC_PE_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, pe_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	/* search for lowest/highest */
	RVA l=(RVA)-1, h=0;
	COFF_SECTION_HEADER *s=pe_shared->sections.sections;
	for (uint i=0; i<pe_shared->sections.section_count; i++) {
		if (s->data_address < l) l = s->data_address;
		if ((s->data_address + s->data_size > h) && s->data_size) h = s->data_address + s->data_size - 1;
		s++;
	}
/**/
	Address *low;
	Address *high;
	if (pe32) {
		l += pe_shared->pe32.header_nt.image_base;
		h += pe_shared->pe32.header_nt.image_base;
		low = p->createAddress32(l);
		high = p->createAddress32(h);
	} else {
		low = p->createAddress64(l + pe_shared->pe64.header_nt.image_base);
		high = p->createAddress64(h + pe_shared->pe64.header_nt.image_base);
	}
	
	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, v, p, low, high);
	
	delete low;
	delete high;
	
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);

	Address *tmpaddr;
	if (pe32) {
		tmpaddr = p->createAddress32(pe_shared->pe32.header.entrypoint_address + pe_shared->pe32.header_nt.image_base);
	} else {
		tmpaddr = p->createAddress64(pe_shared->pe64.header.entrypoint_address + pe_shared->pe64.header_nt.image_base);
	}
	v->gotoAddress(tmpaddr, NULL);
	delete tmpaddr;
	
	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	pe_shared->v_image=v;
	return g;
}

format_viewer_if htpeimage_if = {
	htpeimage_init,
	0
};

static int pe_viewer_func_rva(eval_scalar *result, eval_int *i)
{
	ht_pe_aviewer *aviewer = (ht_pe_aviewer*)eval_get_context();
	RVA rva = i->value;
	viewer_pos p;
	FileOfs ofs;
	if (pe_rva_to_ofs(&aviewer->pe_shared->sections, rva, &ofs)
	&& aviewer->offset_to_pos(ofs, &p)) {
		Address *a;
		int b;
		aviewer->convertViewerPosToAddress(p, &a);
		a->putIntoArray((byte*)&b);
		delete a;
		scalar_create_int_c(result, b);
		return 1;
	} else {
		set_eval_error("invalid file offset or no corresponding RVA for '0%xh'", rva);
	}
	return 0;
}

static int pe_viewer_func_section_int(eval_scalar *result, eval_int *q)
{
	ht_pe_aviewer *aviewer = (ht_pe_aviewer*)eval_get_context();
	uint64 i = q->value - 1;
	if (i >= 0 && i < aviewer->pe_shared->sections.section_count) {
		viewer_pos p;
		FileOfs ofs;
		if (pe_rva_to_ofs(&aviewer->pe_shared->sections,
					    aviewer->pe_shared->sections.sections[i].data_address,
					   &ofs)
		 && aviewer->offset_to_pos(ofs, &p)) {
			Address *a;
			uint64 b;
			aviewer->convertViewerPosToAddress(p, &a);
			a->putIntoUInt64(b);
			delete a;
			scalar_create_int_q(result, b);
			return 1;
		} else {
//			set_eval_error("invalid file offset or no corresponding RVA for '0%xh'", rva);
		}     
	} else {
		set_eval_error("no section number %qd", q->value);
	}
	return 0;
}

static int pe_viewer_func_section_str(eval_scalar *result, eval_str *str)
{
	ht_pe_aviewer *aviewer = (ht_pe_aviewer*)eval_get_context();
	int section;
	char str2[COFF_SIZEOF_SHORT_NAME+1];
	memset(str2, 0, COFF_SIZEOF_SHORT_NAME+1);
	memcpy(str2, str->value, MIN(str->len, COFF_SIZEOF_SHORT_NAME));
	if (pe_section_name_to_section(&aviewer->pe_shared->sections, str2, &section)) {
		eval_scalar i;
		scalar_create_int_c(&i, section+1);
		return pe_viewer_func_section_int(result, &i.scalar.integer);
	}
	return 0;
}

static int pe_viewer_func_section(eval_scalar *result, eval_scalar *q)
{
	if (q->type == SCALAR_STR)
		return pe_viewer_func_section_str(result, &q->scalar.str);
	else {
		eval_int i;
		scalar_context_int(q, &i);
		return pe_viewer_func_section_int(result, &i);
	}
}

/*
 *	CLASS ht_pe_aviewer
 */
void ht_pe_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_pe_shared_data *PE_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	pe_shared = PE_shared;
}

bool ht_pe_aviewer::func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	eval_func myfuncs[] = {
		{"rva", (void*)&pe_viewer_func_rva, {SCALAR_INT},
			"returns address of rva"},
		{"section", (void*)&pe_viewer_func_section, {SCALAR_ANY},
			"returns address of section named param1 if param1 is a string\n"
			"returns address of section with index param1 otherwise"},
		{NULL}
	};
	if (std_eval_func_handler(result, name, params, myfuncs)) return true;
	return ht_aviewer::func_handler(result, name, params);
}

void ht_pe_aviewer::setAnalyser(Analyser *a)
{
	((PEAnalyser *)a)->reinit(pe_shared, file);
	
	analy = a;
	analy_sub->setAnalyser(a);
}

