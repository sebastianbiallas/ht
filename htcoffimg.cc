/*
 *	HT Editor
 *	htcoffimg.cc
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
#include "htnewexe.h"
#include "htpal.h"
#include "htcoffimg.h"
#include "htstring.h"
#include "formats.h"

#include "coff_s.h"

#include "htanaly.h"
#include "coff_analy.h"

ht_view *htcoffimage_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_coff_shared_data *coff_shared=(ht_coff_shared_data *)group->get_shared_data();

	if (coff_shared->opt_magic!=COFF_OPTMAGIC_COFF32) return 0;

	LOG("%s: COFF: loading image (starting analyser)...", file->get_filename());
	coff_analyser *p = new coff_analyser();
	p->init(coff_shared, file);

	bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_COFF_IMAGE"-g");
	analy_infoline *head;

	c.y+=2;
	c.h-=2;
	ht_coff_aviewer *v=new ht_coff_aviewer();
	v->init(&c, DESC_COFF_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, coff_shared);

	c.y-=2;
	c.h=2;
	head=new analy_infoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attach_infoline(head);

/* search for lowest/highest */
	ADDR l=(ADDR)-1, h=0;
	COFF_SECTION_HEADER *s=coff_shared->sections.sections;
	for (UINT i=0; i<coff_shared->sections.section_count; i++) {
		if (s->data_address<l) l=s->data_address;
		if (s->data_address+s->data_size>h) h=s->data_address+s->data_size;
		s++;
	}
/**/
/*	l+=coff_shared->pe32.header_nt.image_base;
	h+=coff_shared->pe32.header_nt.image_base;*/

	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, p, l, h);
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);
	
	v->goto_address(coff_shared->coff32header.entrypoint_address);

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	coff_shared->v_image=v;
	return g;
}

format_viewer_if htcoffimage_if = {
	htcoffimage_init,
	0
};

