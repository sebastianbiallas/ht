/*
 *	HT Editor
 *	htpeimg.cc
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

#include "log.h"
#include "formats.h"
#include "htnewexe.h"
#include "htpal.h"
#include "htpeimg.h"
#include "htstring.h"
#include "pe_analy.h"
#include "pestruct.h"
#include "snprintf.h"

ht_view *htpeimage_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_pe_shared_data *pe_shared=(ht_pe_shared_data *)group->get_shared_data();

	if (pe_shared->opt_magic!=COFF_OPTMAGIC_PE32 && pe_shared->opt_magic!=COFF_OPTMAGIC_PE64) return 0;

     bool pe32 = (pe_shared->opt_magic==COFF_OPTMAGIC_PE32);

     LOG("%s: PE: loading image (starting analyser)...", file->get_filename());
	PEAnalyser *p = new PEAnalyser();
	p->init(pe_shared, file);

	bounds c=*b;
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
	for (UINT i=0; i<pe_shared->sections.section_count; i++) {
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
		low = p->createAddress64(to_qword(l) + pe_shared->pe64.header_nt.image_base);
		high = p->createAddress64(to_qword(h) + pe_shared->pe64.header_nt.image_base);
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
     	tmpaddr = p->createAddress32(pe_shared->pe32.header.entrypoint_address+pe_shared->pe32.header_nt.image_base);
     } else {
     	tmpaddr = p->createAddress64(to_qword(pe_shared->pe64.header.entrypoint_address)+pe_shared->pe64.header_nt.image_base);
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

/*
 *	CLASS ht_pe_aviewer
 */
void ht_pe_aviewer::init(bounds *b, char *desc, int caps, ht_streamfile *File, ht_format_group *format_group, Analyser *Analy, ht_pe_shared_data *PE_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	pe_shared = PE_shared;
	file = File;
}

void ht_pe_aviewer::setAnalyser(Analyser *a)
{
	((PEAnalyser *)a)->pe_shared = pe_shared;
	((PEAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}

