/* 
 *	HT Editor
 *	htleimg.cc
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

#include "htdebug.h"
#include "htdisasm.h"
#include "htnewexe.h"
#include "htle.h"
#include "htleimg.h"
#include "htstring.h"
#include "formats.h"
#include "vxd.h"
#include "x86dis.h"

#include "lestruct.h"

#include <string.h>

class x86dis_le: public x86dis {
protected:
	virtual void str_op(char *opstr, int *opstrlen, x86dis_insn *insn, x86_insn_op *op, bool explicit_params);
public:
	x86dis_le(int opsize, int addrsize);
	virtual ~x86dis_le();

	virtual dis_insn *decode(byte *code, byte maxlen, CPU_ADDR addr);
};

/**/

ht_view *htleimage_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
	ht_le_shared_data *le_shared=(ht_le_shared_data *)group->get_shared_data();

	ht_le_page_file *mfile = new ht_le_page_file();
	mfile->init(file, 1, &le_shared->pagemap, le_shared->pagemap.count,
		le_shared->hdr.pagesize);

	ht_uformat_viewer *v = new ht_uformat_viewer();
	v->init(b, "le/image", VC_EDIT | VC_GOTO | VC_SEARCH, mfile, group);

	for (int i=0; i<le_shared->objmap.count; i++) {
		char t[64];
		char n[5];
		bool use32=le_shared->objmap.header[i].flags & IMAGE_LE_OBJECT_FLAG_USE32;
		memmove(n, le_shared->objmap.header[i].name, 4);
		n[4]=0;
		sprintf(t, "object %d USE%d: %s (psize=%08x, vsize=%08x)", i+1, use32 ? 32 : 16, n, le_shared->objmap.psize[i], le_shared->objmap.vsize[i]);

		disassembler *disasm;

		if (use32) {
			disasm = new x86dis_le(X86_OPSIZE32, X86_ADDRSIZE32);
		} else {
			disasm = new x86dis_le(X86_OPSIZE16, X86_ADDRSIZE16);
		}

		ht_disasm_sub *d = new ht_disasm_sub();
		d->init(mfile, (le_shared->objmap.header[i].page_map_index-1)*
			le_shared->hdr.pagesize, le_shared->objmap.vsize[i], disasm,
			true, X86DIS_STYLE_OPTIMIZE_ADDR);
		ht_collapsable_sub *cs = new ht_collapsable_sub();
		cs->init(file, d, 1, t, 1);
		v->insertsub(cs);
	}

	return v;
}

format_viewer_if htleimage_if = {
	htleimage_init,
	0
};

/*
 *	CLASS ht_le_page_file
 */

void ht_le_page_file::init(ht_streamfile *file, bool own_file, ht_le_pagemap *pm, dword pms, dword ps)
{
	ht_layer_streamfile::init(file, own_file);
	pagemap = pm;
	pagemapsize = pms;
	page_size = ps;
	ofs = 0;
}

bool ht_le_page_file::isdirty(FILEOFS offset, UINT range)
{
	FILEOFS mofs;
	UINT msize;
	while (range) {
		dword s=range;
		if (!map_ofs(offset, &mofs, &msize)) break;
		if (s>msize) s=msize;
		bool isdirty;
		streamfile->cntl(FCNTL_MODS_IS_DIRTY, mofs, s, &isdirty);
		if (isdirty) return 1;
		range-=s;
		ofs+=s;
	}
	return 0;
}

int ht_le_page_file::map_ofs(dword qofs, FILEOFS *offset, dword *maxsize)
{
	dword i=qofs/page_size, j=qofs % page_size;
	if (i<pagemapsize) {
		if (j<pagemap->vsize[i]) {
			*offset=pagemap->offset[i]+j;
			*maxsize=pagemap->vsize[i]-j;
			return 1;
		}
	}
	return 0;
}

UINT ht_le_page_file::read(void *buf, UINT size)
{
	FILEOFS mofs;
	UINT msize;
	int c=0;
	while (size) {
		dword s=size;
		if (!map_ofs(ofs, &mofs, &msize)) break;
		if (s>msize) s=msize;
		streamfile->seek(mofs);
		s=streamfile->read(buf, s);
		if (!s) break;
		((byte*)buf)+=s;
		size-=s;
		c+=s;
		ofs+=s;
	}
	return c;
}

int ht_le_page_file::seek(FILEOFS offset)
{
	ofs=offset;
	return 0;
}

FILEOFS ht_le_page_file::tell()
{
	return ofs;
}

UINT ht_le_page_file::write(void *buf, UINT size)
{
	dword mofs, msize;
	int c=0;
	while (size) {
		dword s=size;
		if (!map_ofs(ofs, &mofs, &msize)) break;
		if (s>msize) s=msize;
		streamfile->seek(mofs);
		((byte*)buf)+=streamfile->write(buf, s);
		size-=s;
		c+=s;
		ofs+=s;
	}
	return c;
}

/*
 *	CLASS x86dis_le
 */

x86dis_le::x86dis_le(int opsize, int addrsize)
: x86dis(opsize, addrsize)
{
}

x86dis_le::~x86dis_le()
{
}

dis_insn *x86dis_le::decode(byte *code, byte maxlen, CPU_ADDR addr)
{
	if ((maxlen>=6) && (code[0]==0xcd) && (code[1]==0x20)) {
		insn.name="VxDCall";
		insn.size=6;
		vxd_t *v=find_vxd(vxds, *(word*)(code+4));

		if (v) {
			insn.op[1].type=X86_OPTYPE_USER;
			insn.op[1].user[0]=*(word*)(code+4);
			insn.op[1].user[1]=(int)v->name;
		
			char *vs=find_vxd_service(v->services, *(word*)(code+2) & 0x7fff);

			if (vs) {
				insn.op[0].type=X86_OPTYPE_USER;
				insn.op[0].user[0]=*(word*)(code+2);
				insn.op[0].user[1]=(int)vs;
			} else {
				insn.op[0].type=X86_OPTYPE_IMM;
				insn.op[0].size=2;
				insn.op[0].imm=*(word*)(code+2);
			}
		} else {
			insn.op[0].type=X86_OPTYPE_IMM;
			insn.op[0].size=2;
			insn.op[0].imm=*(word*)(code+2);
		
			insn.op[1].type=X86_OPTYPE_IMM;
			insn.op[1].size=2;
			insn.op[1].imm=*(word*)(code+4);
		}
		insn.op[2].type=X86_OPTYPE_EMPTY;
		insn.lockprefix=X86_PREFIX_NO;
		insn.repprefix=X86_PREFIX_NO;
		insn.segprefix=X86_PREFIX_NO;
		return &insn;
	}
	return x86dis::decode(code, maxlen, addr);
}

void x86dis_le::str_op(char *opstr, int *opstrlen, x86dis_insn *insn, x86_insn_op *op, bool explicit_params)
{
	if (op->type==X86_OPTYPE_USER) {
		*opstrlen=0;
		strcpy(opstr, (char*)op->user[1]);
	} else {
		x86dis::str_op(opstr, opstrlen, insn, op, explicit_params);
	}
}

