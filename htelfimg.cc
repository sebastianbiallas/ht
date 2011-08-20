/*
 *	HT Editor
 *	htelfimg.cc
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
#include "htelfimg.h"
#include "htpal.h"
#include "strtools.h"
#include "formats.h"
#include "tools.h"

#include "elfstruc.h"
#include "elf_analy.h"

static ht_view *htelfimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_elf_shared_data *elf_shared=(ht_elf_shared_data *)group->get_shared_data();

//	if (elf_shared->ident.e_ident[ELF_EI_CLASS]!=ELFCLASS32) return 0;

	String fn;
	LOG("%y: ELF: loading image (starting analyser)...", &file->getFilename(fn));
	ElfAnalyser *p = new ElfAnalyser();
	p->init(elf_shared, file);

	Bounds c = *b;
	ht_group *g = new ht_group();
	g->init(&c, VO_RESIZE, DESC_ELF_IMAGE"-g");
	AnalyInfoline *head;

	c.y += 2;
	c.h -= 2;
	ht_elf_aviewer *v = new ht_elf_aviewer();
	v->init(&c, DESC_ELF_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, p, elf_shared);

	c.y -= 2;
	c.h = 2;
	head = new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

	/* find lowest/highest address */
	Address *low = NULL;
	Address *high = NULL;
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
	case ELFCLASS32: {
		ELFAddress l, h;
		l.a32 = (uint32)-1;
		h.a32 = 0;
		ELF_SECTION_HEADER32 *s = elf_shared->sheaders.sheaders32;
		for (uint i=0; i < elf_shared->sheaders.count; i++) {
			if (elf_valid_section((elf_section_header*)s, elf_shared->ident.e_ident[ELF_EI_CLASS])) {
				if (s->sh_addr < l.a32) l.a32=s->sh_addr;
				if ((s->sh_addr + s->sh_size > h.a32) && s->sh_size) h.a32=s->sh_addr + s->sh_size - 1;
			}
			s++;
		}
		low = p->createAddress32(l.a32);
		high = p->createAddress32(h.a32);
		break;
	}
	case ELFCLASS64: {
		ELFAddress l, h;
		l.a64 = (uint64)-1;
		h.a64 = 0;
		ELF_SECTION_HEADER64 *s = elf_shared->sheaders.sheaders64;
		for (uint i=0; i < elf_shared->sheaders.count; i++) {
			if (elf_valid_section((elf_section_header*)s, elf_shared->ident.e_ident[ELF_EI_CLASS])) {
				if (s->sh_addr < l.a64) l.a64 = s->sh_addr;
				if ((s->sh_addr + s->sh_size > h.a64)
				&& s->sh_size != 0) {
					h.a64 = s->sh_addr + s->sh_size - 1;
				}
			}
			s++;
		}
		low = p->createAddress64(l.a64);
		high = p->createAddress64(h.a64);
		break;
	}
	}

	ht_analy_sub *analy = new ht_analy_sub();

	if (low->compareTo(high) < 0) {
		analy->init(file, v, p, low, high);
		v->analy_sub = analy;
		v->insertsub(analy);
	} else {
		delete analy;
		v->done();
		delete v;
		head->done();
		delete head;
		g->done();
		delete g;
		delete high;
		delete low;
		return NULL;
	}
	
	delete high;
	delete low;

	v->sendmsg(msg_complete_init, 0);

	Address *tmpaddr = NULL;
	switch (elf_shared->ident.e_ident[ELF_EI_CLASS]) {
	case ELFCLASS32:
		tmpaddr = p->createAddress32(elf_shared->header32.e_entry);
		break;
	case ELFCLASS64:
		tmpaddr = p->createAddress64(elf_shared->header64.e_entry);
		break;
	}

	v->gotoAddress(tmpaddr, NULL);
	delete tmpaddr;

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

/*
 *	CLASS ht_elf_aviewer
 */
void ht_elf_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_elf_shared_data *ELF_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	elf_shared = ELF_shared;
}

void ht_elf_aviewer::setAnalyser(Analyser *a)
{
	((ElfAnalyser *)a)->elf_shared = elf_shared;
	((ElfAnalyser *)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
