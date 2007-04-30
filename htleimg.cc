/*
 *	HT Editor
 *	htleimg.cc
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
#include "htnewexe.h"
#include "htpal.h"
#include "htleimg.h"
#include "htsearch.h"
#include "strtools.h"
#include "formats.h"
#include "tools.h"

#include "lestruct.h"

#include "htanaly.h"
#include "le_analy.h"

static ht_view *htleimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_le_shared_data *le_shared=(ht_le_shared_data *)group->get_shared_data();

	File *myfile = le_shared->reloc_file;

	String fn;
	LOG("%y: LE: loading image (starting analyser)...", &file->getFilename(fn));
	LEAnalyser *p = new LEAnalyser();
	p->init(le_shared, myfile);

	Bounds c = *b;
	ht_group *g = new ht_group();
	g->init(&c, VO_RESIZE, DESC_LE_IMAGE"-g");
	AnalyInfoline *head;

	c.y += 2;
	c.h -= 2;
	ht_le_aviewer *v = new ht_le_aviewer();
	v->init(&c, DESC_LE_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, myfile, group, p, le_shared);
	v->search_caps = SEARCHMODE_VREGEX;

	c.y -= 2;
	c.h = 2;
	head = new AnalyInfoline();
	head->init(&c, v, ANALY_STATUS_DEFAULT);

	v->attachInfoline(head);

/* search for lowest/highest */
	LEAddress l=(LEAddress)-1, h=0;
	LE_OBJECT *s = le_shared->objmap.header;
	for (uint i=0; i<le_shared->objmap.count; i++) {
		LEAddress base = LE_MAKE_ADDR(le_shared, i, 0);
		uint evsize = MAX(LE_get_seg_vsize(le_shared, i), LE_get_seg_psize(le_shared, i));
		if (base < l) l = base;
		if ((base + evsize > h) && (evsize)) h = base + evsize - 1;
		s++;
	}
/**/
	Address *low = p->createAddressFlat32(l);
	Address *high = p->createAddressFlat32(h);
	ht_analy_sub *analy = new ht_analy_sub();
	analy->init(myfile, v, p, low, high);
	v->analy_sub = analy;
	v->insertsub(analy);
	delete high;
	delete low;

	v->sendmsg(msg_complete_init, 0);

	g->insert(head);
	g->insert(v);

	g->setpalette(palkey_generic_window_default);

	le_shared->v_image = v;
	return g;
}

format_viewer_if htleimage_if = {
	htleimage_init,
	0
};

/*
 *	CLASS ht_le_aviewer
 */

void ht_le_aviewer::init(Bounds *b, const char *desc, int caps, File *File, ht_format_group *format_group, Analyser *Analy, ht_le_shared_data *LE_shared)
{
	ht_aviewer::init(b, desc, caps, File, format_group, Analy);
	le_shared = LE_shared;
	file = File;
}

const char *ht_le_aviewer::func(uint i, bool execute)
{
	switch (i) {
		case 3: {
			bool e = false;
			file->cntl(FCNTL_GET_RELOC, &e);
			if (execute) {
				file->cntl(FCNTL_SET_RELOC, !e);
				analy_sub->output->invalidateCache();
				dirtyview();
			}
			return e ? (char*)"unrelocate" : (char*)"relocate";
		}
	}
	return ht_aviewer::func(i, execute);
}

bool ht_le_aviewer::offset_to_pos(FileOfs ofs, viewer_pos *p)
{
	if (!analy) return false;
	Address *a = ((LEAnalyser*)analy)->realFileofsToAddress(ofs);
	bool res = convertAddressToViewerPos(a, p);
	delete a;
	return res;
}

bool ht_le_aviewer::pos_to_offset(viewer_pos p, FileOfs *ofs)
{
	if (analy) {
		Address *addr;
		if (!convertViewerPosToAddress(p, &addr)) return false;
		FileOfs o=((LEAnalyser*)analy)->addressToRealFileofs(addr);
		delete addr;
		if (o!=INVALID_FILE_OFS) {
			*ofs=o;
			return true;
		}
	}
	return false;
}

bool ht_le_aviewer::get_current_real_offset(FileOfs *ofs)
{
	FileOfs o;
	if (!get_current_offset(&o)) return false;
	FileOfs m;
	if (!le_shared->linear_file->map_ofs(o, ofs, &m)) return false;
	return true;
}

void ht_le_aviewer::setAnalyser(Analyser *a)
{
	((LEAnalyser*)a)->le_shared = le_shared;
	((LEAnalyser*)a)->file = file;
	analy = a;
	analy_sub->setAnalyser(a);
}
