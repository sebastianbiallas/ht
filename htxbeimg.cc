/*
 *	HT Editor
 *	htxbeimg.cc
 *
 *	Copyright (C) 2003 Stefan Esser
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
#include "htxbeimg.h"
#include "strtools.h"
#include "xbe_analy.h"
#include "xbestruct.h"
#include "snprintf.h"

static ht_view *htxbeimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_xbe_shared_data *xbe_shared=(ht_xbe_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: XBE: loading image (starting analyser)...", &file->getFilename(fn));
	XBEAnalyser *p = new XBEAnalyser();
	p->init(xbe_shared, file);

	Bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_XBE_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_xbe_aviewer *v=new ht_xbe_aviewer();
	v->init(&c, DESC_XBE_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, xbe_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	/* search for lowest/highest */
	RVA l=(RVA)-1, h=0;
	XBE_SECTION_HEADER *s=xbe_shared->sections.sections;
	for (uint i=0; i<xbe_shared->sections.number_of_sections; i++) {
		if (s->virtual_address < l) l = s->virtual_address;
		if ((s->virtual_address + s->virtual_size > h) && s->virtual_size) h = s->virtual_address + s->virtual_size - 1;
		s++;
	}
	/**/
	Address *low;
	Address *high;
	l+=xbe_shared->header.base_address;
	h+=xbe_shared->header.base_address;
	low = p->createAddress32(l);
	high = p->createAddress32(h);
	
	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, v, p, low, high);
	
	delete low;
	delete high;
	
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);

	Address *tmpaddr;
	tmpaddr = p->createAddress32(xbe_shared->header.entry_point);

	v->gotoAddress(tmpaddr, NULL);
	delete tmpaddr;
	
	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	xbe_shared->v_image=v;
	return g;
}

format_viewer_if htxbeimage_if = {
	htxbeimage_init,
	0
};

static int xbe_viewer_func_rva(eval_scalar *result, eval_int *i)
{
	ht_xbe_aviewer *aviewer = (ht_xbe_aviewer*)eval_get_context();
	RVA rva = i->value;
	viewer_pos p;
	FileOfs ofs;
	if (xbe_rva_to_ofs(&aviewer->xbe_shared->sections, rva, &ofs)
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

static int xbe_viewer_func_section_int(eval_scalar *result, eval_int *q)
{
	ht_xbe_aviewer *aviewer = (ht_xbe_aviewer*)eval_get_context();
	sint64 i = q->value-1;
	if (i >= 0 &&
	(i < aviewer->xbe_shared->sections.number_of_sections)) {
		viewer_pos p;
		FileOfs ofs;
		if (xbe_rva_to_ofs(&aviewer->xbe_shared->sections,
					    aviewer->xbe_shared->sections.sections[i].virtual_address,
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

static int xbe_viewer_func_section_str(eval_scalar *result, eval_str *str)
{
	ht_xbe_aviewer *aviewer = (ht_xbe_aviewer*)eval_get_context();
	int section;
	if (xbe_section_name_to_section(&aviewer->xbe_shared->sections, str->value, &section)) {
		eval_scalar i;
		scalar_create_int_c(&i, section+1);
		return xbe_viewer_func_section_int(result, &i.scalar.integer);
	}
	return 0;
}

static int xbe_viewer_func_section(eval_scalar *result, eval_scalar *q)
{
	if (q->type == SCALAR_STR)
		return xbe_viewer_func_section_str(result, &q->scalar.str);
	else {
		eval_int i;
		scalar_context_int(q, &i);
		return xbe_viewer_func_section_int(result, &i);
	}
}

/*
 *	ht_xbe_aviewer
 */
void ht_xbe_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_xbe_shared_data *XBE_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	xbe_shared = XBE_shared;
}

bool ht_xbe_aviewer::func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	eval_func myfuncs[] = {
		{"rva", (void*)&xbe_viewer_func_rva, {SCALAR_INT},
			"returns address of rva"},
		{"section", (void*)&xbe_viewer_func_section, {SCALAR_ANY},
			"returns address of section named param1 if param1 is a string\n"
			"returns address of section with index param1 otherwise"},
		{NULL}
	};
	if (std_eval_func_handler(result, name, params, myfuncs)) return true;
	return ht_aviewer::func_handler(result, name, params);
}

void ht_xbe_aviewer::setAnalyser(Analyser *a)
{
	((XBEAnalyser *)a)->xbe_shared = xbe_shared;
	((XBEAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
