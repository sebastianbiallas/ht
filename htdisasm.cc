/* 
 *	HT Editor
 *	htdisasm.cc
 *
 *	Copyright (C) 2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <string.h>

#include "cmds.h"
#include "htapp.h"
#include "htdisasm.h"
#include "hthist.h"
#include "htiobox.h"
#include "htmenu.h"
#include "httag.h"
#include "x86asm.h"
#include "x86dis.h"

ht_view *htdisasm_init(bounds *b, ht_streamfile *file, ht_format_group *group)
{
#if 1
	x86asm *assembler=new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
	x86dis *disassembler=new x86dis(X86_OPSIZE32, X86_ADDRSIZE32);
#else
	x86asm *assembler=new x86asm(X86_OPSIZE16, X86_ADDRSIZE16);
	x86dis *disassembler=new x86dis(X86_OPSIZE16, X86_ADDRSIZE16);
#endif

	ht_disasm_viewer *v=new ht_disasm_viewer();
	v->init(b, DESC_DISASM, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, assembler, disassembler);

	ht_disasm_sub *d=new ht_disasm_sub();
	d->init(file, 0, file->get_size(),
		disassembler, false, X86DIS_STYLE_OPTIMIZE_ADDR);

	v->insertsub(d);
	return v;
}

format_viewer_if htdisasm_if = {
	htdisasm_init,
	0
};

/*
 *	dialog_assemble
 */
 
static int opcode_compare(char *a, char *b)
{
	int al = strlen(a);
	int bl = strlen(b);
	if (al > bl) return 1; else if (al < bl) return -1; else return 0;
}

void dialog_assemble(ht_format_viewer *f, fmt_vaddress vaddr, CPU_ADDR cpuaddr, assembler *a, disassembler *disasm, char *default_str)
{
	char instr[257] = "";
	strcpy(instr, default_str);
	asm_insn *insn = a->alloc_insn();
	asm_code *ac = NULL;
	while (inputbox(a->get_name(), "~instruction:", instr, 255, HISTATOM_ASSEMBLER)) {
		if ((a->prepare_str(insn, instr)) && (ac = a->encode(insn, 0, cpuaddr))) {
			break;
		} else {
			errorbox("%s: %s", a->get_name(), a->get_error_msg());
		}
	}
	if (ac) {
		bool ok=true;
		asm_code *chosen_ac = ac;
		if (ac->next) {
			// choose from list if ambigous
			bounds b;
			b.w = 60;
			b.h = 15;
			center_bounds(&b);
			ht_dialog *dialog = new ht_dialog();
			dialog->init(&b, "choose opcode", FS_KILLER | FS_TITLE | FS_MOVE);
			BOUNDS_ASSIGN(b, 1, 0, 56, 1);
			ht_statictext *text = new ht_statictext();
			text->init(&b, " opcode (& disassembly)", 0);
			dialog->insert(text);
			BOUNDS_ASSIGN(b, 1, 1, 56, 12);
			ht_text_listbox *list=new ht_text_listbox();
			list->init(&b, 2, 0);
			asm_code *ac2 = ac;
			UINT aci=0;
			while (ac2) {
				char s[1024]="", *tmp = s;
				for (UINT i=0; i<ac2->size; i++) {
					tmp += sprintf(tmp, "%02x ", ac2->data[i]);
				}
				if (disasm) {
					dis_insn *o=disasm->decode((byte *)ac2->data, ac2->size, cpuaddr);
					tmp = disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);
				} else {
					tmp = "<no disassembler>";
				}
				list->insert_str(aci, s, tmp);
				ac2 = ac2->next;
				aci++;
			}
			ht_text_listbox_sort_order so;
			so.col = 0;
			so.compare_func = opcode_compare;
			list->update();
			list->sort(1, &so);
			list->goto_item_by_position(0);
			dialog->insert(list);
			int r = dialog->run(0);
			ok = r;
			if (r == button_ok) {
				ht_listbox_data d;
				list->databuf_get(&d);
				ht_text_listbox_item *i=(ht_text_listbox_item *)list->getbyid(d.cursor_id);
				asm_code *ac3 = ac;
				int ac3i=0;
				while (ac3) {
					if (ac3i==i->id) {
						chosen_ac=ac3;
						break;
					}
					ac3=ac3->next;
					ac3i++;
				}
			}
			dialog->done();
			delete dialog;
		}
		if (ok) {
			baseview->sendmsg(cmd_edit_mode_i, f->get_file(), NULL);
			if (f->get_file() && (f->get_file()->get_access_mode() & FAM_WRITE)) {
				f->vwrite(vaddr, chosen_ac->data, chosen_ac->size);
			}
		}
	}
	free(insn);
}
/*
 *	CLASS ht_disasm_viewer
 */

void ht_disasm_viewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, assembler *a, disassembler *d)
{
	ht_uformat_viewer::init(b, desc, caps, file, format_group);
	assem = a;
	disasm = d;
}

void ht_disasm_viewer::done()
{
	ht_uformat_viewer::done();
	if (assem) delete assem;
	if (disasm) delete disasm;
}

void ht_disasm_viewer::get_pindicator_str(char *buf)
{
	FILEOFS o;
	if (get_current_offset(&o)) {
		sprintf(buf, " %s %08x/%u ", edit() ? "edit" : "view", o, o);
	} else {
		strcpy(buf, "?");
	}
}
	
bool ht_disasm_viewer::get_hscrollbar_pos(int *pstart, int *psize)
{
	int s=file->get_size();
	if (s) {
		int z=MIN(size.h*16, s-(int)top_id1);
		return scrollbar_pos(top_id1, z, s, pstart, psize);
	}
	return false;
}

void ht_disasm_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_contextmenuquery: {
			ht_static_context_menu *m=new ht_static_context_menu();
			m->init("~Local (Disasm)");
			m->insert_entry("~Assemble", "Ctrl+A", cmd_disasm_call_assembler, K_Control_A, 1);

			msg->msg = msg_retval;
			msg->data1.ptr = m;
			return;
		}
		case msg_get_scrollinfo: {
			switch (msg->data1.integer) {
				case gsi_pindicator: {
					get_pindicator_str((char*)msg->data2.ptr);
					break;
				}
				case gsi_hscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_hscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					break;
				}
				case gsi_vscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_vscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					break;
				}
			}
			clearmsg(msg);
			return;
		}
		case msg_filesize_changed: {
			htmsg m;
			m.msg=msg_filesize_changed;
			m.type=mt_broadcast;
			sendsubmsg(&m);
			
			uf_initialized=false;
			complete_init();
			
			dirtyview();
			return;
		}
		case cmd_disasm_call_assembler: {
			fmt_vaddress current_address;
			get_current_address(&current_address);

			CPU_ADDR cpuaddr;
			cpuaddr.addr32.seg = 0;
			cpuaddr.addr32.offset = current_address;

			assem->set_imm_eval_proc(NULL, NULL);

			dialog_assemble(this, current_address, cpuaddr, assem, disasm, "");
			
			clearmsg(msg);
			return;
		}
	}
	ht_uformat_viewer::handlemsg(msg);
}

bool ht_disasm_viewer::address_to_offset(fmt_vaddress addr, FILEOFS *ofs)
{
	*ofs=addr;
	return 1;
}

bool ht_disasm_viewer::offset_to_address(FILEOFS ofs, fmt_vaddress *addr)
{
	*addr=ofs;
	return 1;
}

/*
 *	CLASS ht_disasm_sub
 */

void ht_disasm_sub::init(ht_streamfile *f, FILEOFS ofs, int size, disassembler *u, bool own_u, int ds)
{
	ht_linear_sub::init(f, ofs, size);
	disasm = u;
	own_disasm = own_u;
	display_style = ds;
}

void ht_disasm_sub::done()
{
	if (own_disasm) {
		delete disasm;
	}
	ht_linear_sub::done();
}

bool ht_disasm_sub::convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2)
{
	return convert_ofs_to_id(addr, id1, id2);
}

bool ht_disasm_sub::convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2)
{
	if (( offset >=fofs) && ( offset <fofs+fsize)) {
		*id1=offset;
		*id2=0;
		return 1;
	}
	return 0;
}

bool ht_disasm_sub::convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr)
{
	*addr=id1;
	return 1;
}

bool ht_disasm_sub::getline(char *line, ID id1, ID id2)
{
	if (id2) return 0;
	dword ofs=id1;
	unsigned char buf[15];
	int c=MIN(15, (int)(fofs+fsize-ofs));
	if (c<=0) return 0;
	file->seek(ofs);
	c=file->read(buf, c);
	CPU_ADDR caddr;
	caddr.addr32.seg = 0;
	caddr.addr32.offset = ofs;
	char *s;
	char *l=line;
	if (c) {
		dis_insn *insn=disasm->decode(buf, c, caddr);
		s=disasm->str(insn, display_style);
		c=disasm->getsize(insn);
	} else {
		s="db ?";
		c=0;
	}
	l=mkhexd(l, ofs);
	*l++=' ';
	for (int i=0; i<15; i++) {
		if (i<c) {
			l=tag_make_edit_byte(l, ofs+i);
		} else {
			*l++=' ';
			*l++=' ';
		}
	}
	*l++=' ';
	strcpy(l, s);
//	l+=sprintf(l, " %s", s);
	return 1;
}

void ht_disasm_sub::first_line_id(ID *id1, ID *id2)
{
	*id1=fofs;
	*id2=0;
}

void ht_disasm_sub::last_line_id(ID *id1, ID *id2)
{
	*id1=fofs+fsize;
	*id2=0;
	prev_line_id(id1, id2, 1);
}

int ht_disasm_sub::prev_line_id(ID *id1, ID *id2, int n)
{
	if (*id2) return 0;
	dword *ofs=id1;
	unsigned char buf[15*50], *bufp=buf;
	int offsets[15*50];
	int *of=offsets;
	int r=n<6 ? 6*15 : n*15;
	ADDR o=*ofs-r;
	int c=r, d;
	int s;
	if (*ofs<fofs+r) {
		c-=fofs-o;
		o=fofs;
	}
	if (o+c>fofs+fsize) {
		c=fofs+fsize-o;
	}
	file->seek(o);
	d=file->read(buf, c);

	CPU_ADDR caddr;
	caddr.addr32.seg = 0;
	caddr.addr32.offset = 0;
	do {
		if (d>0) {
			dis_insn *insn=disasm->decode(bufp, d, caddr);
			s=disasm->getsize(insn);
			d-=s;
		} else {
			s=1;
		}
		*(of++)=o;
		o+=s;
		bufp+=s;
	} while (o<=*ofs);
	if (of-n-1<offsets) {
		*ofs=*(offsets);
		return of-offsets-1;
	} else {
		*ofs=*(of-n-1);
		return n;
	}
}

int ht_disasm_sub::next_line_id(ID *id1, ID *id2, int n)
{
	if (*id2) return 0;
	dword *ofs=id1;
	unsigned char buf[15];
	int c=0, s;
	UINT z;
	CPU_ADDR caddr;
	caddr.addr32.seg = 0;
	caddr.addr32.offset = 0;
	while (n--) {
		z=MIN(15, (UINT)(fofs+fsize-*ofs));
		file->seek(*ofs);
		z=file->read(buf, z);
		if (z) {
			dis_insn *insn=disasm->decode(buf, z, caddr);
			s=disasm->getsize(insn);
		} else {
			s=1;
		}
		if (*ofs+s>fofs+fsize-1) return c;
		*ofs+=s;
		c++;
	}
	return c;
}


