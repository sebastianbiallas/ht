/*
 *	HT Editor
 *	htelfimg.cc
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
#include "htelfimg.h"
#include "htpal.h"
#include "htstring.h"
#include "formats.h"

#include "elfstruc.h"

#include "htanaly.h"
#include "elf_analy.h"

ht_view *htelfimage_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();

	if (elf_shared->ident.e_ident[ELF_EI_CLASS]!=ELFCLASS32) return 0;

	LOG("%s: ELF: loading image (starting analyser)...", file->get_filename());
	elf_analyser *p = new elf_analyser();
	p->init(elf_shared, file);
	p->begin_analysis();

	bounds c=*b;
	ht_group *g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_ELF_IMAGE"-g");
	analy_infoline *head;

	c.y+=2;
	c.h-=2;
	ht_elf_aviewer *v=new ht_elf_aviewer();
	v->init(&c, DESC_ELF_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, elf_shared);

	c.y-=2;
	c.h=2;
	head=new analy_infoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attach_infoline(head);

/* lowest/highest finden */
	ADDR l=(ADDR)-1, h=0;
	ELF_SECTION_HEADER32 *s=elf_shared->sheaders.sheaders32;
	for (UINT i=0; i<elf_shared->sheaders.count; i++) {
		if (elf_valid_section((elf_section_header*)s, elf_shared->ident.e_ident[ELF_EI_CLASS])) {
			if (s->sh_addr<l) l=s->sh_addr;
			if (s->sh_addr+s->sh_size>h) h=s->sh_addr+s->sh_size;
		}
		s++;
	}
/**/

	ht_analy_sub *analy=new ht_analy_sub();
	analy->init(file, p, l, h);
	v->analy_sub = analy;
	v->insertsub(analy);

	v->sendmsg(msg_complete_init, 0);
	
	fmt_vaddress vaddr;
	if (v->string_to_address("entrypoint", &vaddr)) {
		v->goto_address(vaddr);
	}

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	elf_shared->v_image=v;
	return g;
}

format_viewer_if htelfimage_if = {
	htelfimage_init,
	0
};

