/* 
 *	HT Editor
 *	htmzimg.cc
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

#include "htdisasm.h"
#include "htmz.h"
#include "htmzimg.h"
#include "htpal.h"
#include "strtools.h"
#include "htformat.h"
#include "formats.h"
#include "snprintf.h"
#include "x86asm.h"
#include "x86dis.h"

static ht_view *htmzimage_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_mz_shared_data *mz_shared=(ht_mz_shared_data *)group->get_shared_data();

	x86asm *assembler=new x86asm(X86_OPSIZE16, X86_ADDRSIZE16);
	x86dis *disassembler=new x86dis(X86_OPSIZE16, X86_ADDRSIZE16);

	FileOfs o = mz_shared->header.header_size*16;
	ht_disasm_viewer *v=new ht_disasm_viewer();
	v->init(b, DESC_MZ_IMAGE, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, assembler, disassembler, 1);
	ht_mask_sub *m=new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* MZ image at offset 0x%08qx", o);
	m->add_mask(info);
	v->insertsub(m);

	ht_disasm_sub *d=new ht_disasm_sub();
	d->init(file, o, (mz_shared->header.sizep-1)*512+
		mz_shared->header.sizelp,
		disassembler, false, X86DIS_STYLE_OPTIMIZE_ADDR);

	v->insertsub(d);

	v->sendmsg(msg_complete_init, 0);

	viewer_pos p;
	v->clear_viewer_pos(&p);
	p.u.sub = d;
	p.u.tag_idx = 0;

	if (d->convert_ofs_to_id(o+mz_shared->header.cs*16+
	mz_shared->header.ip, &p.u.line_id)) {
		v->goto_pos(p, false);
	}

	return v;
}

format_viewer_if htmzimage_if = {
	htmzimage_init,
	0
};

