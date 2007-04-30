/*
 *	HT Editor
 *	htxeximg.cc
 *
 *	Copyright (C) 2006 Sebastian Biallas (sb@biallas.net)
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
#include "htxeximg.h"
#include "strtools.h"
#include "xex_analy.h"
#include "xexstruct.h"
#include "snprintf.h"

#define DESC_XEX_IMAGE "xex/image"

static ht_view *htxeximage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_xex_shared_data *xex_shared=(ht_xex_shared_data *)group->get_shared_data();

	String fn;
	LOG("%y: XEX: loading image (starting analyser)...", &file->getFilename(fn));
	
	file = xex_shared->image;

	XEXAnalyser *p = new XEXAnalyser();
	p->init(xex_shared, file);

	Bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_XEX_IMAGE"-g");
	AnalyInfoline *head;

	c.y+=2;
	c.h-=2;
	ht_xex_aviewer *v=new ht_xex_aviewer();
	v->init(&c, DESC_XEX_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, xex_shared);

	c.y-=2;
	c.h=2;
	head=new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	Address *low = p->createAddress32(xex_shared->image_base);
	Address *high = p->createAddress32(xex_shared->image_base+xex_shared->image_size-1);
	
	p->validarea->add(low, high);
	
	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, v, p, low, high);
	
	delete low;
	delete high;
	
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);

	Address *tmpaddr;
	tmpaddr = p->createAddress32(xex_shared->entrypoint);
	v->gotoAddress(tmpaddr, NULL);
	delete tmpaddr;
	
	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	xex_shared->v_image=v;
	return g;
}

format_viewer_if htxeximage_if = {
	htxeximage_init,
	0
};

#if 0
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
#endif

/*
 *	CLASS ht_pe_aviewer
 */
void ht_xex_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_xex_shared_data *XEX_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	xex_shared = XEX_shared;
}

#if 0
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
#endif

void ht_xex_aviewer::setAnalyser(Analyser *a)
{
	((XEXAnalyser*)a)->xex_shared = xex_shared;
	((XEXAnalyser*)a)->file = xex_shared->image;
	
	analy = a;
	analy_sub->setAnalyser(a);
}

