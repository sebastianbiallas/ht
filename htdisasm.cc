/*
 *	HT Editor
 *	htdisasm.cc
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

#include <cstring>

#include "cmds.h"
#include "htctrl.h"
#include "htdisasm.h"
#include "hthist.h"
#include "htiobox.h"
#include "htmenu.h"
#include "httag.h"
#include "snprintf.h"
#include "x86asm.h"
#include "x86dis.h"
#include "ppcdis.h"

extern "C" {
#include "evalx.h"
#include "regex.h"
}

ht_view *htdisasm_init(Bounds *b, File *file, ht_format_group *group)
{
	int t1632;
#if 1
	Assembler *assembler=new x86asm(X86_OPSIZE32, X86_ADDRSIZE32);
	x86dis *disassembler=new x86dis(X86_OPSIZE32, X86_ADDRSIZE32);
#else
	Assembler *assembler = NULL;
	Disassembler *disassembler = new PPCDisassembler(PPC_MODE_32);
#endif
	t1632 = 0;

	ht_disasm_viewer *v=new ht_disasm_viewer();
	v->init(b, DESC_DISASM, VC_EDIT | VC_GOTO | VC_SEARCH, file, group, assembler, disassembler, t1632);

	ht_disasm_sub *d=new ht_disasm_sub();
	d->init(file, 0, file->getSize(),
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
 
static int opcode_compare(const char *a, const char *b)
{
	int al = strlen(a);
	int bl = strlen(b);
	if (al > bl) return 1; else if (al < bl) return -1; else return strcmp(a, b);
}

void dialog_assemble(ht_format_viewer *f, viewer_pos vaddr, CPU_ADDR cpuaddr, Assembler *a, Disassembler *disasm, const char *default_str, int want_length)
{
	char instr[257] = "";
	if (default_str) strcpy(instr, default_str);
	asm_insn *insn = a->alloc_insn();
	asm_code *ac = NULL;
	while (inputbox(a->get_name(), "~instruction:", instr, 255, HISTATOM_ASSEMBLER)) {
		if ((a->translate_str(insn, instr)) && (ac = a->encode(insn, 0, cpuaddr))) {
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
			Bounds b;
			b.w = 60;
			b.h = 15;
			center_bounds(&b);
			ht_dialog *dialog = new ht_dialog();
			dialog->init(&b, "choose opcode", FS_KILLER | FS_TITLE | FS_MOVE);
			b.assign(1, 0, 56, 1);
			ht_listbox_title *text = new ht_listbox_title();
			text->init(&b);
			text->setText(2, "opcode", "disassembly");
			dialog->insert(text);
			b.assign(1, 1, 56, 12);
			ht_text_listbox *list=new ht_text_listbox();
			list->init(&b, 2, 0);
			list->attachTitle(text);
			asm_code *ac2 = ac;
			uint aci = 0;
			int best = 0;
			while (ac2) {
				char s[1024], *tmp = s;
				for (int i=0; i < ac2->size; i++) {
					tmp += sprintf(tmp, "%02x ", ac2->data[i]);
				}
				if (best == 0 && want_length == ac2->size) {
					best = aci+1;
				}
				const char *tmp2;
				if (disasm) {
					dis_insn *o = disasm->decode((byte *)ac2->data, ac2->size, cpuaddr);
					tmp2 = disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);
				} else {
					tmp2 = "<no disassembler>";
				}
				list->insert_str(aci, s, tmp2);
				ac2 = ac2->next;
				aci++;
			}
			ht_text_listbox_sort_order so;
			so.col = 0;
			so.compare_func = opcode_compare;
			list->update();
			if (best) {
				list->gotoItemByPosition(best-1);
			}
			list->sort(1, &so);
			if (!best) {
				list->gotoItemByPosition(0);
			}
			dialog->insert(list);
			int r = dialog->run(0);
			ok = r;
			if (r == button_ok) {
				ht_listbox_data d;
				ViewDataBuf vdb(list, &d, sizeof d);
				ht_text_listbox_item *i = (ht_text_listbox_item *)d.data->cursor_ptr;
				asm_code *ac3 = ac;
				int ac3i = 0;
				while (ac3) {
					if (ac3i == i->id) {
						chosen_ac = ac3;
						break;
					}
					ac3 = ac3->next;
					ac3i++;
				}
			}
			dialog->done();
			delete dialog;
		}
		if (ok) {
			baseview->sendmsg(cmd_edit_mode_i, f->get_file(), NULL);
			if (f->get_file() && (f->get_file()->getAccessMode() & IOAM_WRITE)) {
				f->vwrite(vaddr, chosen_ac->data, chosen_ac->size);
			}
		}
	}
	free(insn);
}

/*
 *	CLASS ht_disasm_viewer
 */

void ht_disasm_viewer::init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Assembler *a, Disassembler *d, int t)
{
	ht_uformat_viewer::init(b, desc, caps, file, format_group);
	assem = a;
	disasm = d;
	op1632 = t;
}

void ht_disasm_viewer::done()
{
	ht_uformat_viewer::done();
	delete assem;
	delete disasm;
}

int ht_disasm_viewer::get_pindicator_str(char *buf, int max_len)
{
	FileOfs o;
	if (get_current_offset(&o)) {
		return ht_snprintf(buf, max_len, " %s 0x%08qx/%qu ", edit() ? "edit" : "view", o, o);
	} else {
		return ht_snprintf(buf, max_len, " ? ");
	}
}
	
bool ht_disasm_viewer::get_vscrollbar_pos(int *pstart, int *psize)
{
	FileOfs s=file->getSize();
	if (s) {
		int z = MIN(size.h*16, s-(int)top.line_id.id1);
		return scrollbar_pos(top.line_id.id1, z, s, pstart, psize);
	}
	return false;
}

void ht_disasm_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_contextmenuquery: {
			ht_static_context_menu *m=new ht_static_context_menu();
			m->init("~Local-Disasm");
			m->insert_entry("~Assemble", "Ctrl+A", cmd_disasm_call_assembler, K_Control_A, 1);
			// FIXME: wrong implementation
			m->insert_entry("~Toggle 16/32", NULL, cmd_disasm_toggle1632, 0, 1);

			msg->msg = msg_retval;
			msg->data1.ptr = m;
			return;
		}
		case msg_get_scrollinfo: {
			switch (msg->data1.integer) {
				case gsi_hscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_hscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					clearmsg(msg);
					return;
				}
				case gsi_vscrollbar: {
					gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_vscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					clearmsg(msg);
					return;
				}
			}
			break;
		}
		case msg_filesize_changed: {
			htmsg m;
			m.msg=msg_filesize_changed;
			m.type=mt_broadcast;
			sendsubmsg(&m);
			
			// FIXME: hack
			uf_initialized=false;
			complete_init();
			
			dirtyview();
			return;
		}
		case cmd_disasm_call_assembler: {
			viewer_pos current_pos;
			get_current_pos(&current_pos);

			CPU_ADDR cpuaddr;
			cpuaddr.addr32.seg = 0;
			cpuaddr.addr32.offset = current_pos.u.line_id.id1;

			assem->set_imm_eval_proc(NULL, NULL);

			byte data[32];
			int datalen = vread(current_pos, data, sizeof data);
			dis_insn *o = disasm->decode(data, datalen, cpuaddr);
			const char *curinsn = disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);
			int want_length = disasm->getSize(o);

			dialog_assemble(this, current_pos, cpuaddr, assem, disasm, curinsn, want_length);

			clearmsg(msg);
			return;
		}
		case cmd_disasm_toggle1632: {
			// FIXME: very beautiful...
			op1632 ^= 1;
			if (op1632) {
				((x86asm *)assem)->opsize = X86_OPSIZE16;
				((x86asm *)assem)->addrsize = X86_ADDRSIZE16;
				((x86dis *)disasm)->opsize = X86_OPSIZE16;
				((x86dis *)disasm)->addrsize = X86_ADDRSIZE16;
			} else {
				((x86asm *)assem)->opsize = X86_OPSIZE32;
				((x86asm *)assem)->addrsize = X86_ADDRSIZE32;
				((x86dis *)disasm)->opsize = X86_OPSIZE32;
				((x86dis *)disasm)->addrsize = X86_ADDRSIZE32;
			}
			dirtyview();
			clearmsg(msg);
			return;
		}
	}
	ht_uformat_viewer::handlemsg(msg);
}

ht_disasm_sub *ht_disasm_viewer::get_disasm_sub()
{
	return (ht_disasm_sub*)cursor.sub;
}

bool ht_disasm_viewer::offset_to_pos(FileOfs ofs, viewer_pos *p)
{
	p->u.sub = get_disasm_sub();
	p->u.line_id.id1 = ofs;
	p->u.line_id.id2 = 0;
	p->u.tag_idx = 0;
	return true;
}

bool ht_disasm_viewer::pos_to_offset(viewer_pos p, FileOfs *ofs)
{
	*ofs = p.u.line_id.id1;
	return true;
}

bool ht_disasm_viewer::ref_sel(LINE_ID *id)
{
	return goto_offset(id->id1, true);
}

bool ht_disasm_viewer::qword_to_pos(uint64 q, viewer_pos *p)
{
	ht_linear_sub *s = get_disasm_sub();
	FileOfs ofs = q;
	clear_viewer_pos(p);
	p->u.sub = s;
	p->u.tag_idx = 0;
	return s->convert_ofs_to_id(ofs, &p->u.line_id);
}

bool ht_disasm_viewer::symbol_handler(eval_scalar *result, char *name)
{
	if (strcmp(name, "$") == 0) {
		FileOfs ofs;
		if (!pos_to_offset(*(viewer_pos*)&cursor, &ofs)) return 0;
		scalar_create_int_q(result, ofs);
		return true;
	}
	return ht_uformat_viewer::symbol_handler(result, name);
}

const char *ht_disasm_viewer::func(uint i, bool execute)
{
	switch (i) {
		// FIXME: wrong implementation
		case 8:
			if (execute) sendmsg(cmd_disasm_toggle1632);
			return op1632 ? (char*)"use32" : (char*)"use16";
	}
	return ht_uformat_viewer::func(i, execute);
}

/*
 *	CLASS ht_disasm_sub
 */

void ht_disasm_sub::init(File *f, FileOfs ofs, int size, Disassembler *u, bool own_u, int ds)
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

bool ht_disasm_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	if ((offset >= fofs) && (offset < fofs+fsize)) {
		line_id->id1=offset;
		line_id->id2=0;
		return true;
	}
	return false;
}

bool ht_disasm_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset)
{
	*offset = line_id.id1;
	return true;
}

static char *diasm_addr_sym_func(CPU_ADDR Addr, int *symstrlen, void *context)
{
	ht_disasm_sub *sub = (ht_disasm_sub *) context;
	static char buf[120];
	LINE_ID line_id;
	sub->first_line_id(&line_id);
	if (Addr.addr32.offset >= line_id.id1) {
		sub->last_line_id(&line_id);
		if (Addr.addr32.offset <= line_id.id1) {
			char buf2[60];
			ht_snprintf(buf2, sizeof buf2, "0x%x", Addr.addr32.offset);
			char *b = tag_make_ref(buf, sizeof buf-1, Addr.addr32.offset, 0, 0, 0, buf2);
			*b = 0;
			if (symstrlen) *symstrlen = b-buf;
			return buf;
		}
	}
	return NULL;
}

bool ht_disasm_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	if (line_id.id2) return false;
	uint64 ofs = line_id.id1;
	byte buf[16];
	int c = MIN(16, sint64(fofs+fsize-ofs));
	if (c <= 0) return false;
	file->seek(ofs);
	c = file->read(buf, c);
	CPU_ADDR caddr;
	caddr.addr32.seg = 0;
	caddr.addr32.offset = ofs;
	const char *s;
	char *l = line;
	if (c) {
		dis_insn *insn = disasm->decode(buf, c, caddr);
		addr_sym_func_context = this;
		addr_sym_func = &diasm_addr_sym_func;
		s = disasm->str(insn, display_style);
		addr_sym_func = NULL;
		c = disasm->getSize(insn);
	} else {
		s = "db ?";
		c = 0;
	}
	l += ht_snprintf(l, maxlen, "%08qx ", ofs);
	for (int i=0; i<15; i++) {
		if (i<c) {
			l=tag_make_edit_byte(l, maxlen, ofs+i);
		} else {
			*l++=' ';
			*l++=' ';
		}
	}
	*l++=' ';
	tag_strcpy(l, maxlen, s);
	return true;
}

void ht_disasm_sub::first_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = fofs;
}

void ht_disasm_sub::last_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = fofs+fsize-1;
}

int ht_disasm_sub::prev_line_id(LINE_ID *line_id, int n)
{
	if (line_id->id2) return 0;
	uint32 *ofs=&line_id->id1;
	int min_length;
	int max_length;
	int min_look_ahead;
	int avg_look_ahead;
	int addr_align;
	disasm->getOpcodeMetrics(min_length, max_length, min_look_ahead, avg_look_ahead, addr_align);

	unsigned char buf[avg_look_ahead*50], *bufp=buf;
	int offsets[avg_look_ahead*50];
	int *of=offsets;
	int r=n<6 ? 6*avg_look_ahead : n*avg_look_ahead;
	if (r > (int)sizeof buf) r = sizeof buf;
	uint32 o=*ofs-r;
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
			s=disasm->getSize(insn);
/*			if (s!=4) {
				insn=disasm->decode(bufp, d, caddr);
			}
			assert(s==4);*/
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

int ht_disasm_sub::next_line_id(LINE_ID *line_id, int n)
{
	if (line_id->id2) return 0;
	uint32 *ofs = &line_id->id1;
	unsigned char buf[15];
	int c=0, s;
	uint z;
	CPU_ADDR caddr;
	caddr.addr32.seg = 0;
	caddr.addr32.offset = 0;
	while (n--) {
		z=MIN(15, (uint)(fofs+fsize-*ofs));
		file->seek(*ofs);
		z=file->read(buf, z);
		if (z) {
			dis_insn *insn=disasm->decode(buf, z, caddr);
			s=disasm->getSize(insn);
		} else {
			s=1;
		}
		if (*ofs+s>fofs+fsize-1) return c;
		*ofs+=s;
		c++;
	}
	return c;
}

