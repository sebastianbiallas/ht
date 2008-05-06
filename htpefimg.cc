/*
 *	HT Editor
 *	htpefimg.cc
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
#include "htpal.h"
#include "htpefimg.h"
#include "strtools.h"
#include "pef_analy.h"
#include "pefstruc.h"
#include "snprintf.h"

ht_view *htpefimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_pef_shared_data *pef_shared=(ht_pef_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: PEF: loading image (starting analyser)...", &file->getFilename(fn));
	PEFAnalyser *p = new PEFAnalyser();
	p->init(pef_shared, file);

	Bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_PEF_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_pef_aviewer *v=new ht_pef_aviewer();
	v->init(&c, DESC_PEF_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, pef_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	/* search for lowest/highest */
	uint32 l=0xffffffff, h=0;
	PEF_SECTION_HEADER *s=pef_shared->sheaders.sheaders;
	for (uint i=0; i<pef_shared->sheaders.count; i++) {
		if (s->defaultAddress < l) l = s->defaultAddress;
		if ((s->defaultAddress + s->totalSize > h) &&
		s->totalSize && pef_phys_and_mem_section(s))
			h = s->defaultAddress + s->totalSize - 1;
		s++;
	}
	/**/
	Address *low;
	Address *high;

	low = p->createAddress32(l);
	high = p->createAddress32(h);

	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, v, p, low, high);
	
	delete low;
	delete high;
	
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);

//	Address *tmpaddr;
//	tmpaddr = p->createAddress32(pef_shared->pe32.header.entrypoint_address+pef_shared->pe32.header_nt.image_base);
//	v->gotoAddress(tmpaddr, NULL);
//	delete tmpaddr;
	
	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

//	pef_shared->v_image=v;
	return g;
}

format_viewer_if htpefimage_if = {
	htpefimage_init,
	0
};

/*static int pe_viewer_func_rva(eval_scalar *result, eval_int *i)
{
	ht_pe_aviewer *aviewer = (ht_pe_aviewer*)eval_get_context();
	RVA rva = QWORD_GET_INT(i->value);
	viewer_pos p;
	FileOfs ofs;
	if (pe_rva_to_ofs(&aviewer->pef_shared->sections, rva, &ofs)
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
	uint i = QWORD_GET_INT(q->value)-1;
	if (!QWORD_GET_HI(q->value) && (i >= 0) &&
	(i < aviewer->pef_shared->sections.section_count)) {
		viewer_pos p;
		FileOfs ofs;
		if (pe_rva_to_ofs(&aviewer->pef_shared->sections,
					    aviewer->pef_shared->sections.sections[i].data_address,
					   &ofs)
		 && aviewer->offset_to_pos(ofs, &p)) {
			Address *a;
			int b;
			aviewer->convertViewerPosToAddress(p, &a);
			a->putIntoArray((byte*)&b);
			delete a;
			scalar_create_int_c(result, b);
			return 1;
		} else {
//			set_eval_error("invalid file offset or no corresponding RVA for '0%xh'", rva);
		}     
	} else {
		set_eval_error("no section number %qd", &q->value);
	}
	return 0;
}

static int pe_viewer_func_section_str(eval_scalar *result, eval_str *str)
{
	ht_pe_aviewer *aviewer = (ht_pe_aviewer*)eval_get_context();
	int section;
	char str2[COFF_SIZEOF_SHORT_NAME+1];
	memset(str2, 0, COFF_SIZEOF_SHORT_NAME+1);
	memmove(str2, str->value, MIN(str->len, COFF_SIZEOF_SHORT_NAME));
	if (pe_section_name_to_section(&aviewer->pef_shared->sections, str2, &section)) {
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
}*/

/*
 *	CLASS ht_pef_aviewer
 */
void ht_pef_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_pef_shared_data *PEF_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	pef_shared = PEF_shared;
}

void ht_pef_aviewer::setAnalyser(Analyser *a)
{
	((PEFAnalyser *)a)->pef_shared = pef_shared;
	((PEFAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
