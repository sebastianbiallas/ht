/*
 *	HT Editor
 *	htanaly.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#include "analy.h"
#include "analy_names.h"
#include "global.h"
#include "htanaly.h"
#include "htapp.h"
#include "htdialog.h"
#include "htdisasm.h"
#include "hthist.h"
#include "htidle.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htmenu.h"
#include "htsearch.h"
#include "htstring.h"
#include "httag.h"
#include "httree.h"
#include "language.h"
#include "textedit.h"
#include "textfile.h"
#include "tools.h"
#include "syntax.h"
#include "out.h"
#include "out_ht.h"
#include "store.h"

extern "C" {
#include "evalx.h"
}

#include <stdlib.h>
#include <string.h>

/* FIXME: test */
#include "srt.h"
#include "out_html.h"


/*
 *	analyser_information
 */
void	analyser_information::init(bounds *b, ht_aviewer *a)
{
	analy = a;
	assert(a);
	ht_statictext::init(b, 0, align_left);
	register_idle_object(this);
	idle();
}

void analyser_information::done()
{
	unregister_idle_object(this);
	ht_statictext::done();
}

char *analyser_information::gettext()
{
	sprintf(buf,
		"Analyser statistics:\n"
		"====================\n\n"          
		"Type: %s\nFile: %s\n"
		"Using disassembler: %s\n\n"
		"Known addresses: %d\n"
		"Known symbols: %d\n",
		atype, aname,
		adis,
		addrs, labels);
	return buf;
}

bool analyser_information::idle()
{
	if (analy && analy->analy) {
		addrs = analy->analy->get_addr_count();
		labels = analy->analy->get_label_count();
		atype = analy->analy->get_type();
		aname = analy->analy->get_name();
		if (analy->analy->disasm) {
			adis = analy->analy->disasm->get_name();
		} else {
			adis = "?";
		}
		dirtyview();
	}
	return false;
}

/*
 *	symbolbox
 */

void symbolbox::init(bounds *b, analyser *Analy)
{
	analy = Analy;
	ht_listbox::init(b);
	str = (char *)smalloc(1024); // Buffer overflow...
	symbols = analy->get_label_count();
}

void symbolbox::done()
{
	free(str);
	ht_listbox::done();
}

int  symbolbox::calc_count()
{
	return analy->get_label_count();
}

int  symbolbox::cursor_adjust()
{
	return 18;
}

int  symbolbox::estimate_entry_pos(void *entry)
{
	return 0;
}

void *symbolbox::getfirst()
{
	return analy->enum_labels(NULL);
}

void *symbolbox::getlast()
{
	return analy->enum_labels_back(NULL);
}

void *symbolbox::getnext(void *entry)
{
	if (!entry) return NULL;
	return analy->enum_labels(((tlabel *)entry)->name);
}

void *symbolbox::getnth(int n)
{
	// slow!!!!!!!!!!
	// n==1 =^= 1th entry
	tlabel *e = NULL;
	for (int i=0; i<n; i++) {
		e = analy->enum_labels(((tlabel *)e)->name);
		if (!e) break;
	}
	return e;
}

void *symbolbox::getprev(void *entry)
{
	if (!entry) return NULL;
	return analy->enum_labels_back(((tlabel *)entry)->name);
}

char *symbolbox::getstr(int col, void *entry)
{
	if (!entry) return NULL;
	tlabel *l = ((tlabel *)entry);
	switch (col) {
		case 0:
			sprintf(str, "%8x", l->addr->addr);
			break;
		case 1:
			strcpy(str, label_type_short(l->type));
			break;
		case 2:
			strcpy(str, l->name);
			break;
	}
	return str;
}

bool symbolbox::idle()
{
	if ((idle_count % 500)==0) {
		update();
		redraw();
		symbols = analy->get_label_count();
		idle_count = 1;
		return 1;
	}
	idle_count++;
	return 0;
}

int	symbolbox::num_cols()
{
	return 3;
}

void *symbolbox::quickfind(char *s)
{
	tlabel *tmp = analy->find_label(s);
	if (tmp) return tmp;
	tmp = analy->enum_labels(s);
	if (tmp) {
		int slen = strlen(s);
		int tlen = strlen(tmp->name);
		if (slen > tlen) return NULL;
		if (strncmp(tmp->name, s, slen)==0) return tmp;
		return NULL;
	} else {
		return NULL;
	}
}


char	*symbolbox::quickfind_completition(char *s)
{
	if (analy->find_label(s)) {
		return ht_strdup(s);
	}
	tlabel *tmp = analy->enum_labels(s);
	if (!tmp) {
		return ht_strdup(s);
	}
	tlabel *tmp2 = analy->enum_labels(tmp->name);
	if (!tmp2) {
		return ht_strdup(tmp->name);
	}
	int slen = strlen(s);
	if (!ht_strncmp(tmp->name, tmp2->name, slen)==0) {
		return ht_strdup(tmp->name);
	}
	char *res = (char *)smalloc(1024); // buffer bla
	strcpy(res, tmp->name);
	while (tmp2 && (ht_strncmp(tmp2->name, s, slen)==0)) {
//		fprintf(stdout, "while(%s, %s, %d)\n", tmp2->name, s, slen);
		int a = strccomm(res, tmp2->name);
		res[a] = 0;
		tmp2 = analy->enum_labels(tmp2->name);
	}
	return res;
}


/*
 *	CLASS call_chain
 */
void	call_chain::init(bounds *b, analyser *Analy, ADDR a, char *desc)
{
	ht_treeview::init(b, desc);
	VIEW_DEBUG_NAME("call_chain");
	analy = Analy;
	root = create_node(a);
}

void call_chain_done(call_chain_node *n)
{
	while (n) {
		call_chain_done(n->child);
		call_chain_node *temp = n->next;
		free(n);
		n = temp;
	}
}

void	call_chain::done()
{
	call_chain_done(root);
	ht_treeview::done();
}

void	call_chain::adjust(void *node, bool expand)
{
	((call_chain_node*)node)->expanded = expand;
}

call_chain_node *call_chain::create_node(ADDR A)
{
	call_chain_node *n = (call_chain_node *)smalloc(sizeof(call_chain_node));
	n->next = NULL;
	n->prev = NULL;
	n->child = NULL;
	n->examined = false;
	n->xa = A;
	n->faddr = analy->find_addr_func(A);
	assert(n->faddr);
	n->fa = n->faddr->addr;
	n->expanded = false;
	return n;
}

void call_chain::examine_node(call_chain_node *n)
{
	n->examined = true;
	if (has_children(n)) {
          ht_tree *x_tree = n->faddr->xrefs;
          assert(x_tree);
          addr_xref *x;
          ht_addr *a = (ht_addr*)x_tree->enum_next((ht_data**)&x, NULL);
          assert(a);
		call_chain_node *nn = n->child = create_node(a->addr);
          while ((a = (ht_addr*)x_tree->enum_next((ht_data**)&x, a))) {
			nn->next = create_node(a->addr);
			nn = nn->next;
          }
	}
}

void	*call_chain::get_child(void *node, int i)
{
	call_chain_node *p;
	if (node) {
		if (!((call_chain_node *)node)->examined) examine_node((call_chain_node*)node);
		p = ((call_chain_node *)node)->child;
	} else {
		p = root;
	}
	while (p && (--i)) p = p->next;
	return p;
}

void	*call_chain::get_next_node(void *node)
{
	return ((call_chain_node*)node)->next;
}

void	*call_chain::get_prev_node(void *node)
{
	return ((call_chain_node*)node)->prev;
}

void	*call_chain::get_root()
{
	return root;
}

char	*call_chain::get_text(void *node)
{
	static char stupid[1024]; // FIXME: static var + buffer bla
	call_chain_node *n=(call_chain_node*)node;
	sprintf(stupid, "%s+%x (0x%x)", n->faddr->label?n->faddr->label->name:"unknown", n->xa-n->faddr->addr, n->xa);
	return stupid;
}

bool	call_chain::has_children(void *node)
{
	return ((call_chain_node*)node)->faddr->xrefs != NULL;
}

bool	call_chain::is_expanded(void *node)
{
	return ((call_chain_node*)node)->expanded;
}

void	call_chain::select_node(void *node)
{
}


/////////////////////////////////////////////////////////////////////////////

/*
 *
 */
void analy_infoline::init(bounds *b, ht_aviewer *A, char *Format)
{
	ht_statictext::init(b, 0, align_left);
	VIEW_DEBUG_NAME("analy_infoline");
	analy = A;
	displayformat = ht_strdup(Format);
	s = (char *)smalloc(1024); // FIXME: buffer bla
}

void analy_infoline::done()
{
	free(displayformat);
	free(s);
	ht_statictext::done();
}

char *analy_infoline::gettext()
{
	if (valid()) {
		char *sec = analy->analy->get_addr_section_name(addr);
		if (fofs!=INVALID_FILE_OFS) {
			taddr *a = analy->analy->find_addr_func(addr);
			char *func = (a) ? ((a->label) ? a->label->name : NULL): NULL;
			
//			sprintf(s, displayformat, (sec) ? sec : "", fofs,  analy->analy->getdisasmstr(addr), (func)?func:"");
			char *d = displayformat;
			char *ss = s;
			while (*d) {
				if (*d=='%') {
					d++;
					switch (*d) {
						case ANALY_STATUS_ARG_SECTION:
							if (sec) ss+=sprintf(ss, "%s", sec);
							break;
						case ANALY_STATUS_ARG_FILEOFFSET:
							ss+=sprintf(ss, "%08x", fofs);
							break;
						case ANALY_STATUS_ARG_RAW_UNASM:
							ss+=sprintf(ss, "%s", analy->analy->get_disasm_str(addr));
							break;
						case ANALY_STATUS_ARG_FUNCTION:
							if (func) ss+=sprintf(ss, "%s+%x", func, addr-a->addr);
							break;
						case ANALY_STATUS_ARG_OFFSET:
							ss+=sprintf(ss, "%08x", addr);
							break;
						case '%':
							*ss++ = '%';
							break;
						default:
							ss += sprintf(ss, " error in format ");
					}
				} else {
					*ss++ = *d;
				}
				d++;
			}
			*ss = 0;
		} else {
			if (!sec) {
				strcpy(s, "[not in file]");
			} else {
				sprintf(s, "<%s> [not in file]", sec);
			}
		}
		return s;
	} else {
		return "<no analyser>";
	}
}

void analy_infoline::update(ADDR cursor_addr, ADDR ecursor_addr)
{
	if (valid()) {
		fofs = ecursor_addr;
		addr = cursor_addr;
	} else {
		fofs = INVALID_FILE_OFS;
		addr = INVALID_ADDR;
	}
	dirtyview();
	redraw();
}

bool analy_infoline::valid()
{
	return ((analy)&&(analy->analy));
}

/*
 *
 */
void ht_aviewer::init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, analyser *Analy)
{
	analy=Analy;
	if (Analy) {
		analy->set_display_mode(ANALY_TRANSLATE_SYMBOLS+ANALY_COLLAPSE_XREFS, 0);
	}
	analy_sub = NULL;
	ht_uformat_viewer::init(b, desc, caps, file, format_group);
	search_caps |= SEARCHMODE_BIN | SEARCHMODE_EVALSTR | SEARCHMODE_EXPR;
	infoline = NULL;
	idle_count = 0;
	last_active = true;
	pause = false;
	register_idle_object(this);
	one_load_hack = false;
}

void ht_aviewer::done()
{
	unregister_idle_object(this);
	ht_uformat_viewer::done();
	if (analy) {
		analy->done(); // FIXME: this musnt be here
		delete analy;
	}
}

void ht_aviewer::attach_infoline(analy_infoline *V)
{
	infoline = V;
}

bool ht_aviewer::address_to_offset(fmt_vaddress addr, FILEOFS *ofs)
{
	if (analy) {
		FILEOFS o=analy->file_addr(addr);
		if (o!=INVALID_FILE_OFS) {
			*ofs=o;
			return true;
		}
	}
	return false;
}

bool ht_aviewer::address_to_string(char *result, fmt_vaddress vaddr)
{
	if (!analy) return false;
	ADDR a = (ADDR)vaddr;
	taddr *addr = analy->find_addr(a);
	if (addr && addr->label) {
		strcpy(result, addr->label->name);
		return true;
	}
	addr = analy->find_addr_func(a);
	if (addr && addr->label) {
		sprintf(result, "%s+0%xh", addr->label->name, a-addr->addr);
		return true;
	}
	sprintf(result, "0%xh", a);
	return true;
}

char *ht_aviewer::func(UINT i, bool execute)
{
	switch (i) {
		case 8: {
			if (execute) {
				if (!analy) return 0;
				fmt_vaddress current_address;
				get_current_address(&current_address);
				taddr *Addr = analy->find_addr_label((ADDR)current_address);
				
				bounds b;
				b.w = 60;
				b.h = 15;
				center_bounds(&b);
				ht_dialog *dialog = new ht_dialog();
				dialog->init(&b, "symbols", FS_KILLER | FS_TITLE | FS_MOVE);
				/* pull down */
				BOUNDS_ASSIGN(b, 30, 0, 20, 1);
/*				ht_listpopup *lp = new ht_listpopup();
				lp->init(&b);
				lp->insertstring("show all");
				lp->insertstring("only functions");
				lp->insertstring("only labels");
				lp->insertstring("only offsets (code)");
				lp->insertstring("only data");
				dialog->insert(lp);*/
				/* text */
				BOUNDS_ASSIGN(b, 1, 0, 30, 1);
				ht_statictext *text = new ht_statictext();
				text->init(&b, " Address   Type   Name", 0);
				dialog->insert(text);
				/* list */
				BOUNDS_ASSIGN(b, 1, 1, 56, 12);
				symbolbox *sym=new symbolbox();
				sym->init(&b, analy);
				if (Addr && Addr->label) {                    
					sym->goto_item(sym->quickfind(Addr->label->name));
				}
				dialog->insert(sym);
				register_idle_object(sym);
				int r = dialog->run(0);
				unregister_idle_object(sym);
				if (r == button_ok) {
					// goto selected symbol
					ht_listbox_data d;
					sym->databuf_get(&d);
					fmt_vaddress vaddr;
					char address_of_symbol[1024];
					sprintf(address_of_symbol, "address_of('%s')", ((tlabel *)(sym->getbyid(d.cursor_id)))->name);
					if (string_to_address(address_of_symbol, &vaddr)) {
						goto_address(vaddr, this);
					}
				}
				dialog->done();
				delete dialog;
			}
			return "symbols";
		}
		default:
			return ht_uformat_viewer::func(i, execute);
	}
	return 0;
}

static int aviewer_func_address_of(scalar_t *result, str_t *str)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	char buffer[1024];
	bin2str(buffer, str->value, MIN((UINT)str->len, sizeof buffer));
	tlabel *l;
	if ((l = aviewer->analy->find_label(buffer))) {
		scalar_create_int_c(result, l->addr->addr);
		return 1;
	} else {
		set_eval_error("invalid label '%s'", buffer);
		return 0;
	}
}

/*
static int aviewer_func_rva(scalar_t *result, int_t *i)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	if (pe_rva_to_ofs(aviewer->
bool pe_rva_to_ofs(pe_section_headers *section_headers, ADDR rva, FILEOFS *ofs)
	scalar_create_int_c(result, );
}
*/

static int aviewer_func_fileofs(scalar_t *result, int_t *i)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	fmt_vaddress a;
	if (aviewer->offset_to_address(i->value, &a)) {
		scalar_create_int_c(result, (int)a);
		return 1;
	} else {
		set_eval_error("invalid file offset or no corresponding address for '0%xh'", i->value);
		return 0;
	}
}

static int aviewer_func_handler(scalar_t *result, char *name, scalarlist_t *params)
{
	evalfunc_t myfuncs[] = {
		{"address_of", (void*)&aviewer_func_address_of, {SCALAR_STR}},
		{"fileofs", (void*)&aviewer_func_fileofs, {SCALAR_INT}},
//		{"rva", (void*)&aviewer_func_rva, {SCALAR_INT}},
		{NULL}
	};
	return std_eval_func_handler(result, name, params, myfuncs);
}

static int aviewer_symbol_handler(scalar_t *result, char *name)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	unsigned int v, w;
	if (*name == '@') {
		name++;
		if (bnstr(&name, &v, 10)) {
			if (*name) return 0;
			if (!aviewer->offset_to_address(v, &w)) {
				set_eval_error("invalid offset: %08x", v);
				return 0;
			}				
			scalar_create_int_c(result, w);
			return 1;
		}
		// invalid number after @
		return 0;
	} else {
		if (strcmp(name, "$")==0) {
			aviewer->get_current_address(&v);
			scalar_create_int_c(result, v);
			return 1;
		}
		tlabel *l = aviewer->analy->find_label(name);
		if (l) {
			v=l->addr->addr;
			scalar_create_int_c(result, v);
			return 1;
/*		} else if (**s=='.') {
			// RVA
			if (bnstr(s, v, 16)) {
				return 1;
			}*/
		}
	}
	return 0;
}

static int ht_aviewer_symbol_to_addr(void *Aviewer, char **s, dword *v)
{
	ht_aviewer *aviewer = (ht_aviewer*)Aviewer;
	if (**s == '@') {
		(*s)++;
		if (bnstr(s, v, 10)) {
			if (!aviewer->offset_to_address(*v, v)) {
				return 0;
			}
			return 1;
		}
		// invalid number after @
		return 0;
	} else {
		char *k=ht_strdup(*s);
		char *t=k;
		while (!strchr("+-*/ \t[]", *t) && *t) t++;
		char temp=*t;
		*t=0;
		if ((*k == '$') && (k[1] == 0)) {
			*t=temp;
			*s+= t-k;
			aviewer->get_current_address(v);
			free(k);
			return 1;
		}
		tlabel *l = aviewer->analy->find_label(k);
		*t=temp;
		if (l) {
			// Label
			*s+=t-k;
			*v=l->addr->addr;
			free(k);
			return 1;
/*		} else if (**s=='.') {
			// RVA
			if (bnstr(s, v, 16)) {
				return 1;
			}*/
		}
		free(k);
	}
	return 0;
}

static void setdatastr(ht_view *v, char *str)
{
	ht_inputfield_data id;
	id.textlen = strlen(str);
	id.text = (byte*)str;
	v->databuf_set(&id);
}

static void getdatastr(ht_inputfield_data *id, char *result)
{
	memcpy(result, id->text, id->textlen);
	result[id->textlen]=0;
}

struct output_dialog_data {
	ht_inputfield_data id1;
	ht_listpopup_data lp;
	ht_inputfield_data id2;
	ht_inputfield_data id3;
};

void ht_aviewer::generate_output_dialog()
{
	if (!analy) return;
	if (analy->active) {
		infobox("Please wait until analyser has finished before generating output file!");
		if (analy->active) return;
	}

	bounds b;
	b.w=50;
	b.h=15;
	center_bounds(&b);
	ht_dialog *dialog;
	NEW_OBJECT(dialog, ht_dialog, &b, "generate analyser output", FS_KILLER | FS_TITLE | FS_MOVE);
	ht_view *v1, *v2;
	BOUNDS_ASSIGN(b, 2, 2, 25, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	BOUNDS_ASSIGN(b, 2, 1, 25, 1);
	NEW_OBJECT(v2, ht_label, &b, "output ~filename:", v1);
	dialog->insert(v2);
	BOUNDS_ASSIGN(b, 29, 2, 15, 1);
	NEW_OBJECT(v1, ht_listpopup, &b);
	((ht_listpopup*)v1)->insertstring("HTML");
	((ht_listpopup*)v1)->insertstring("plain text");
	dialog->insert(v1);
	BOUNDS_ASSIGN(b, 29, 1, 15, 1);
	NEW_OBJECT(v2, ht_label, &b, "~output format:", v1);
	dialog->insert(v2);
	BOUNDS_ASSIGN(b, 2, 5, 35, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	BOUNDS_ASSIGN(b, 2, 4, 35, 1);
	NEW_OBJECT(v2, ht_label, &b, "~start address:", v1);
	fmt_vaddress cur;
	if (get_current_address(&cur)) {
		char str[1024]; // FIXME: buffer bla
		address_to_string(str, cur);
		setdatastr(v1, str);
	}
	dialog->insert(v2);
	BOUNDS_ASSIGN(b, 2, 8, 35, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	BOUNDS_ASSIGN(b, 2, 7, 35, 1);
	NEW_OBJECT(v2, ht_label, &b, "~end address (or #numberoflines):", v1);
	dialog->insert(v2);
	setdatastr(v1, "#1000");
	BOUNDS_ASSIGN(b, 13, 11, 9, 2);
	NEW_OBJECT(v1, ht_button, &b, "O~k", button_ok);
	dialog->insert(v1);
	BOUNDS_ASSIGN(b, 27, 11, 9, 2);
	NEW_OBJECT(v1, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(v1);
	while (dialog->run(0)==button_ok) {
		char filename[260];
		char start_str[1024], end_str[1024];
		fmt_vaddress start, end;
		bool by_lines;
		output_dialog_data odd;
		dialog->databuf_get(&odd);
		getdatastr(&odd.id1, filename);
		getdatastr(&odd.id2, start_str);
		getdatastr(&odd.id3, end_str);
		if (!string_to_address(start_str, &start)) {
			errorbox(globalerror);
			continue;
		}
		if ((by_lines = end_str[0]=='#')) {
			char *pend = &end_str[1];
			bnstr(&pend, (dword*)&end, 10);
		} else {
			if (!string_to_address(end_str, &end)) {
				errorbox(globalerror);
				continue;
			}
		}
		ht_file *s = new ht_file();
		s->init(filename, FAM_CREATE+FAM_WRITE);
		if (s->get_error()) {
			infobox("couldnt create file '%s'.", filename);
			continue;
		} else {
//			generate_html_output(analy, s, (ADDR)start, (ADDR)end, end, by_lines);
		}
		s->done();
		delete s;
		break;
	}
	dialog->done();
	delete dialog;
}

bool ht_aviewer::can_create_address(ADDR addr, bool error_msg)
{
	taddr *ctx = analy->find_addr_context(addr);
	if (ctx && ctx->addr != addr) {
		if (error_msg) errorbox("Can't create new symbol: Address %08x belongs to %08x (%s)", addr, ctx->addr, ctx->label ? ctx->label->name : "unnamed");
		return false;
	}
	return true;
}

void ht_aviewer::data_string_dialog()
{
	if (!analy) return;
	fmt_vaddress current_address;
	get_current_address(&current_address);
	if (!can_create_address(current_address, true)) {
		return;
	}
/*
	bounds b;
	b.w=50;
	b.h=15;
	center_bounds(&b);
	ht_dialog *dialog;
	NEW_OBJECT(dialog, ht_dialog, &b, "interprete data as string", FS_KILLER | FS_TITLE | FS_MOVE);
	
	while (dialog->run(0)==button_ok) {
	}
	dialog->done();
	delete dialog;*/
	
	if (analy->valid_addr(current_address, scinitialized)) {
		byte buffer[1024];
		taddr *a = analy->enum_addrs(current_address);
		UINT bz = analy->bufptr(current_address, buffer, MIN(sizeof(buffer), a?(a->addr-current_address):sizeof(buffer)));
		if (bz > 2) {
			analy_string *str = string_test(buffer, bz);
			if (str) {
				char string1[256], string2[31];
				str->render_string(string2, 30);
				sprintf(string1, "%s_%s_", str->name(), string2);
				make_valid_name(string2, string1);
				if (analy->add_addr_label(current_address, string2, label_data)) {
					analy->add_comment(current_address, 0, "");
				}
				analy->data->set_array_addr_type(current_address, dst_string, str->length());
				str->done();
				delete str;
			}
		}
	}


}

void ht_aviewer::get_pindicator_str(char *buf)
{
	fmt_vaddress a;
	if (analy && get_current_address(&a)) {
		FILEOFS o;
		if (get_current_offset(&o)) {
			sprintf(buf, " %x/@%08x%s ", a, o, (analy->is_dirty())?" dirty":"");
		} else {
			sprintf(buf, " %x%s ", a, (analy->is_dirty())?" dirty":"");
		}
	} else {
		strcpy(buf, "?");
	}
}

bool ht_aviewer::get_current_offset(FILEOFS *ofs)
{
	if (ht_uformat_viewer::get_current_offset(ofs)) {
		return true;
	} else {
		fmt_vaddress f;
		if (!get_current_address(&f)) return false;
		*ofs = analy->file_addr((ADDR)f);
		return *ofs != INVALID_FILE_OFS;
	}
}

bool ht_aviewer::get_hscrollbar_pos(int *pstart, int *psize)
{
	if (analy_sub) {
		int s=analy_sub->highestaddress-analy_sub->lowestaddress;
		if (s) {
			int z=MIN(size.h*16, (int)(top_id1-analy_sub->lowestaddress));
			return scrollbar_pos(top_id1-analy_sub->lowestaddress, z, s, pstart, psize);
		}
	}
	return false;
}

void ht_aviewer::handlemsg(htmsg *msg)
{
	char str[1024];
	switch (msg->msg) {
		case msg_contextmenuquery: {
			ht_static_context_menu *m=new ht_static_context_menu();
			m->init("~Analyser");
			m->insert_entry("~Information...", 0, cmd_analyser_info, 0, 1);
			m->insert_entry("~Save state", 0, cmd_analyser_save, 0, 1);
			m->insert_separator();
			m->insert_entry("~Assemble...", "Ctrl-a", cmd_analyser_call_assembler, 0, 1);
			m->insert_separator();
			m->insert_entry("~Function start", "Ctrl-f", cmd_analyser_this_function, 0, 1);
			m->insert_entry("Last ~label", "Ctrl-l", cmd_analyser_previous_label, 0, 1);
			m->insert_entry("Follow ptr", "f", cmd_analyser_follow, 0, 1);
			m->insert_separator();
			m->insert_entry("~Continue at address", "c", cmd_analyser_continue, 0, 1);
			m->insert_entry("~Pause / resume", "", cmd_analyser_pause_resume, 0, 1);
			m->insert_separator();
			m->insert_entry("~Name address (label)...", "n", cmd_analyser_name_addr, 0, 1);
			m->insert_entry("Edit co~mments", "#", cmd_analyser_comments, 0, 1);
			m->insert_entry("Show ~xrefs", "x", cmd_analyser_xrefs, 0, 1);
			m->insert_entry("Delete address", "Del", cmd_analyser_del_addr_bindings, 0, 1);
			m->insert_entry("Callcha~in", "Ctrl-t", cmd_analyser_call_chain, 0, 1);
			m->insert_separator();
			m->insert_entry("Data string", "s", cmd_analyser_data_string, 0, 1);
			m->insert_separator();
			m->insert_entry("Symbol reg trace (exp!)", "Alt-q", cmd_analyser_srt, K_Alt_Q, 1);
//			m->insert_separator();
	
			msg->msg = msg_retval;
			msg->data1.ptr = m;
			return;
		}
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_Control_A:
					sendmsg(cmd_analyser_call_assembler);
					clearmsg(msg);
					return;
				case K_Control_D:
					sendmsg(cmd_analyser_del_addr_bindings);
					clearmsg(msg);
					return;
				case K_Control_F:
					sendmsg(cmd_analyser_this_function);
					clearmsg(msg);
					return;
				case K_Control_L:
					sendmsg(cmd_analyser_previous_label);
					clearmsg(msg);
					return;
				case K_Control_O: {
					sendmsg(cmd_analyser_generate_output);
					clearmsg(msg);
					return;
				}
				case K_Control_T:
					sendmsg(cmd_analyser_call_chain);
					clearmsg(msg);
					return;
				case 'c':
					if (!edit()) {
						sendmsg(cmd_analyser_continue);
						clearmsg(msg);
						return;
					}
					break;
				case '#':
					sendmsg(cmd_analyser_comments);
					clearmsg(msg);
					return;
				case 'F':
					if (!edit()) {
						sendmsg(cmd_analyser_follow_ex);
						clearmsg(msg);
						return;
					}
					break;
				case 'f':
					if (!edit()) {
						sendmsg(cmd_analyser_follow);
						clearmsg(msg);
						return;
					}
					break;
				case 'n':
					sendmsg(cmd_analyser_name_addr);
					clearmsg(msg);
					return;
				case 's':
					sendmsg(cmd_analyser_data_string);
					clearmsg(msg);
					return;
				case 'x':
					sendmsg(cmd_analyser_xrefs);
					clearmsg(msg);
					return;
				case K_Delete:
					sendmsg(cmd_analyser_del_addr_bindings);
					clearmsg(msg);
					return;
			}
			break;
		case cmd_analyser_call_assembler: {
			if (!analy) break;
			assembler *a = analy->create_assembler();
			if (!a) {
				// FIXME: select assembler for list
				infobox("no assembler available.");
				clearmsg(msg);
				return;
			}
			fmt_vaddress current_address;
			get_current_address(&current_address);

			a->set_imm_eval_proc((int(*)(void *context, char **s, dword *v))ht_aviewer_symbol_to_addr, (void*)this);

			dialog_assemble(this, current_address, analy->map_addr(current_address), a, analy->disasm, analy->get_disasm_str_formatted(current_address));
			
			a->done();
			delete a;
			clearmsg(msg);
			return;
		}
		case cmd_analyser_this_function: {
			if (!analy) break;
			taddr *a = analy->find_addr_func(cursor_id1);
			if (a) {
				goto_address(a->addr, this);
			} else {
				errorbox("Address %08lx doesn't belong to a function.", cursor_id1);
			}
			clearmsg(msg);
			return;
		}
		case cmd_analyser_previous_label: {
			if (!analy) break;
			taddr *a = analy->find_addr_label(cursor_id1);
			if (a) {
				goto_address(a->addr, this);
			} else {
				errorbox("Address %08lx doesn't belong to a label.", cursor_id1);
			}
			clearmsg(msg);
			return;
		}
		case cmd_analyser_continue:          
			if (!analy) break;
			analy->continue_analysis_at(cursor_id1);
			analy->make_dirty();
			analy_sub->output->invalidate_cache();
			clearmsg(msg);
			return;
		case cmd_analyser_comments: {
			if (!analy) break;
			fmt_vaddress current_address;
			if (get_current_address(&current_address)) {
				show_comments((ADDR)current_address);
			}
			clearmsg(msg);
			return;
		}
		case cmd_analyser_name_addr: {
			if (!analy) break;
			fmt_vaddress current_address;
			get_current_address(&current_address);
			dword addr = (dword)current_address;
			if (!can_create_address(addr, true)) {
				clearmsg(msg);
				return;
			}
			if (analy->valid_addr(addr, scvalid)) {
				char n[255];
				tlabel *l = analy->get_addr_label(addr);
				if (l) strcpy(n, l->name); else n[0] = 0;
				sprintf(str, "name for address %08x", addr);
				while (inputbox(str, "~label name:", n, 255, HISTATOM_NAME_ADDR)) {
					if (n[0]) {
						if (valid_name(n)) {
							char *n2 = ht_strdup(n);
							if (!analy->assign_label(addr, n2, (l)?l->type:label_loc)) {
								l = analy->find_label(n);
								errorbox("Label '%s' already exists at address %08x!", n, l->addr->addr);
								free(n2);
							} else {
								analy->make_dirty();
								analy_sub->output->invalidate_cache();
								break;
							}
						} else {
							if (confirmbox("'%s' is an invalid label name.\nMake valid?", n)==button_yes) {
								char n2[255];
								make_valid_name(n2, n);
								strcpy(n, n2);
							}
						}
					} else {
						if (l) {
							// delete label if applicable
							if (confirmbox("Really delete label '%s' at address %08x?", l->name, addr)==button_yes) {
								analy->delete_label(addr);
								analy->make_dirty();
								analy_sub->output->invalidate_cache();
							}
						}
						break;
					}
				}
			} else {
				errorbox("Address %08lx is invalid!", addr);
			}
			clearmsg(msg);
			return;
		}
		case cmd_analyser_xrefs:
			if (!analy) break;
			show_xrefs(cursor_id1);
			clearmsg(msg);
			return;
		case cmd_analyser_follow: {
			if (!analy) break;
			ADDR p;
			analy->bufptr(cursor_id1, (byte*)&p, sizeof(p));
			if (analy->valid_addr(p, scvalid)) {
				goto_address(p, this);
			} else {
				errorbox("Follow: address %08lx is invalid!", p);
			}
			clearmsg(msg);
			return;
		}
		case cmd_analyser_follow_ex:
			if (!analy) break;
			clearmsg(msg);
			return;
		case cmd_analyser_pause_resume:
			if (!analy) break;
			pause = !pause;
			clearmsg(msg);
			return;
		case cmd_analyser_del_addr_bindings: {
			if (!analy) break;
			fmt_vaddress current_address;
			get_current_address(&current_address);
			if (confirmbox("Forget about address' %08x bindings?\n(name, comments, xrefs, etc.)", current_address)==button_yes) {
				analy->delete_addr((dword)current_address);
				analy->make_dirty();
				analy_sub->output->invalidate_cache();
			}
			clearmsg(msg);
			return;
		}
		case cmd_analyser_generate_output:
			if (!analy) break;
			generate_output_dialog();
			clearmsg(msg);
			return;
		case cmd_analyser_data_string:
			if (!analy) break;
			data_string_dialog();
			analy->make_dirty();
			analy_sub->output->invalidate_cache();
			dirtyview();
			clearmsg(msg);
			return;
		case cmd_analyser_info:
			if (!analy) break;
			show_info(0);
			dirtyview();
			clearmsg(msg);
			return;
		case cmd_edit_mode:
		case cmd_view_mode:
			if (analy) {
				if (edit()) {
					analy->set_display_mode(ANALY_EDIT_BYTES, 0);
				} else {
					analy->set_display_mode(0, ANALY_EDIT_BYTES);
				}
			}
			analy_sub->output->invalidate_cache();
			break;
		case msg_file_changed: {
			analy_sub->output->invalidate_cache();
			break;
		}
		case cmd_analyser_call_chain: {
			if (!analy) break;
			fmt_vaddress current_address;
			get_current_address(&current_address);
			show_call_chain((ADDR) current_address);
			return;
		}
		/* FIXME: srt-experimental */
		case cmd_analyser_srt: {
			fmt_vaddress a;
			if (get_current_address(&a)) {
				test_srt(analy, (ADDR)a);
			}
			clearmsg(msg);
			return;
		}
		case msg_get_analyser: {
			msg->msg=msg_retval;
			msg->data1.ptr=analy;
			return;
		}
		case msg_set_analyser: {
			analyser *a=(analyser*)msg->data1.ptr;
			if (analy) {
				analy->done();
				delete analy;
			}
			a->set_display_mode(0, ANALY_EDIT_BYTES);
			set_analyser(a);               
			clearmsg(msg);
			one_load_hack = true;
			return;
		}
		case msg_postinit: {
			if (analy && !one_load_hack) analy->begin_analysis();
			return;
		}
	}
	ht_uformat_viewer::handlemsg(msg);
	switch (msg->msg) {
		case msg_draw:
			if (infoline) {
				FILEADDR a;
				fmt_vaddress b;
				infoline->update((get_current_address(&b)) ? b : INVALID_ADDR, (get_current_offset(&a)) ? a : INVALID_FILE_OFS);
			}
			break;
	}
}

bool ht_aviewer::idle()
{
	if (!analy) return false;
	last_active=analy->active;
	if (!pause) analy->continue_analysis();
	if (last_active && !analy->active) {
		LOG("%s: analyser finished after %d ops.", analy->get_name(), analy->ops_parsed);
		dirtyview();
		app->sendmsg(msg_draw, 0);
	}
	idle_count++;
	if (idle_count % 501 == 0) {
			analy_sub->output->invalidate_cache();
			dirtyview();
			app->sendmsg(msg_draw, 0);
	}
	if (analy->active) {
		if (idle_count%50==0) {
			analy_sub->output->invalidate_cache();
			dirtyview();
			app->sendmsg(msg_draw, 0);
		}
	}
	return last_active && !pause;
}

bool ht_aviewer::offset_to_address(FILEOFS ofs, fmt_vaddress *addr)
{
	if (!analy) return false;
	*addr = analy->vaddr(ofs);
	return (*addr != INVALID_ADDR);
}

int ht_aviewer::ref_sel(ID id_low, ID id_high)
{
	switch (id_high) {
		case 0:
			return goto_address(id_low, this);
		case 1:
			show_xrefs((ADDR) id_low);
			return 0;
	}
	return 0;
}

void ht_aviewer::reloadpalette()
{
	ht_uformat_viewer::reloadpalette();
	if (analy_sub) analy_sub->output->reloadpalette();
}

void ht_aviewer::search_for_xrefs(ADDR Addr)
{
	char str[100];
	sprintf(str, "%xh", Addr);
	ht_regex_search_request *q=new ht_regex_search_request(SC_VISUAL, SF_REGEX_CASEINSENSITIVE, str);
	ht_visual_search_result *r=(ht_visual_search_result *)vsearch(q, 0, 0xffffffff);
	while (r) {
		analy->add_xref(Addr, r->address, xrefoffset);
		analy->make_dirty();
		analy_sub->output->invalidate_cache();
		delete r;
		fmt_vaddress na;
		next_logical_address(r->address, &na);
		r = (ht_visual_search_result *)vsearch(q, na, 0xffffffff);
	}
	delete q;
}

void ht_aviewer::show_call_chain(ADDR Addr)
{
	taddr *a = analy->find_addr_func(Addr);
	if (!a) return;

	bounds b;
	b.w = 60;
	b.h = 16;
	center_bounds(&b);
	char str[100];
	sprintf(str, "call chain of address %08x", Addr);
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE);
	BOUNDS_ASSIGN(b, 1, 0, 56, 10);
	ht_statictext *text = new ht_statictext();
	if (a->label) {
		sprintf(str, "function %s %s", a->label->name, "is referenced by ..");
	} else {
		sprintf(str, "address %08x %s", a->addr, "is referenced by ..");
	}
	text->init(&b, str, 0);
	dialog->insert(text);
	BOUNDS_ASSIGN(b, 1, 1, 56, 10);
	call_chain *cc;
	NEW_OBJECT(cc, call_chain, &b, analy, Addr, NULL);
	cc->adjust(cc->get_root(), true);
	dialog->insert(cc);
	BOUNDS_ASSIGN(b, 15, 12, 9, 2);
	ht_button *bt;
	NEW_OBJECT(bt, ht_button, &b, "O~k", button_ok);
	dialog->insert(bt);
	BOUNDS_ASSIGN(b, 35, 12, 9, 2);
	NEW_OBJECT(bt, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(bt);
	int r = dialog->run(0);
	if (r == button_ok) {
		ht_treeview_data tvd;
		dialog->databuf_get(&tvd);
//          errorbox("%x", ((call_chain_node*)tvd.selected)->xa);
		goto_address(((call_chain_node*)tvd.selected)->xa, this);
	}
	dialog->done();
	delete dialog;
}

void ht_aviewer::show_comments(ADDR Addr)
{
	if (!analy) return;
	if (!can_create_address(Addr, true)) {
		return;
	}
	comment_list *comment = analy->get_comments(Addr);

	// prepare mem_file
	ht_mem_file *mem_file = new ht_mem_file();
	mem_file->init();
	if (comment) {
		int c1 = comment->count();
		for (int i=0; i < c1; i++) {
			char *c = comment->get_name(i);
			int len = strlen(c);
			if (len) mem_file->write(c, len);
			if (i+1<c1) mem_file->write((void*)"\n", 1);
		}
	}
	
/*     ht_c_syntax_lexer *lexer = new ht_c_syntax_lexer();
	lexer->init();*/
	
	// prepare textfile
	ht_ltextfile *text_file = new ht_ltextfile();
	text_file->init(mem_file, true, NULL);

	// create dialog
	bounds b;
	b.w = 60;
	b.h = 16;
	center_bounds(&b);
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, "edit comments", FS_KILLER | FS_TITLE | FS_MOVE);

	BOUNDS_ASSIGN(b, 1, 1, 55, 10);
	ht_text_editor *text_editor = new ht_text_editor();
	text_editor->init(&b, false, text_file, NULL, TEXTEDITOPT_UNDO);
	dialog->insert(text_editor);

	ht_button *b1;
	BOUNDS_ASSIGN(b, 18, 12, 9, 2);
	NEW_OBJECT(b1, ht_button, &b, "O~k", button_ok);
	dialog->insert(b1);
	BOUNDS_ASSIGN(b, 32, 12, 9, 2);
	NEW_OBJECT(b1, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(b1);

	if (dialog->run(0)==button_ok) {
		taddr *a=analy->find_addr(Addr);
		if (a) analy->free_comments(a);
		UINT c=text_file->linecount();
		char buf[1024];
		bool empty=false;
		if (c==1) {
			UINT l = 0;
			text_file->getline(0, 0, buf, 1024, &l, NULL);
			empty=(l==0);
		}
		if (!empty) {
			for (UINT i=0; i<c; i++) {
				UINT l;
				if (text_file->getline(i, 0, buf, 1024, &l, NULL)) {
					buf[l]=0;
					analy->add_comment(Addr, 0, buf);
				}
			}
		}
		analy->make_dirty();
		analy_sub->output->invalidate_cache();
	}

	dialog->done();
	delete dialog;
	text_file->done();
	delete text_file;
}

void ht_aviewer::show_info(ADDR Addr)
{
	bounds c, b;
	app->getbounds(&c);
	b.w=c.w*5/6;
	b.h=c.h*5/6;
	center_bounds(&b);
	char str[100];
	sprintf(str, "Analyser information");
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE);
	BOUNDS_ASSIGN(b, 1, 0, b.w-4, 10);
	analyser_information *text = new analyser_information();
	text->init(&b, this);
	dialog->insert(text);
	
	dialog->run(0);
	
	dialog->done();
	delete dialog;
}

void ht_aviewer::show_xrefs(ADDR Addr)
{
	ht_tree *x_tree = analy->get_xrefs(Addr);
	if (x_tree) {
		bounds c, b;
		app->getbounds(&c);
		b.w=c.w*5/6;
		UINT bh=b.h=c.h*5/6;
		center_bounds(&b);
		char str[100];
		sprintf(str, "xrefs of address %08x", Addr);
		ht_dialog *dialog = new ht_dialog();
		dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE);
		BOUNDS_ASSIGN(b, 1, 0, b.w-4, 1);
		ht_statictext *text = new ht_statictext();
		text->init(&b, " xref to   type     from function", 0);
		dialog->insert(text);
		b.y = 1;          
		b.h = bh-5;
		ht_text_listbox *list;
		NEW_OBJECT(list, ht_text_listbox, &b, 3, 2);
		char str2[1024];
          addr_xref *x;
          ht_addr *xa = (ht_addr*)x_tree->enum_next((ht_data**)&x, NULL);
          while (xa) {
			sprintf(str, "%8x", xa->addr);
			taddr *a = analy->find_addr_func(xa->addr);
			char *func = (a) ? ((a->label) ? a->label->name : NULL): NULL;
			if (func) sprintf(str2, "%s+%x", func, xa->addr-a->addr); else sprintf(str2, "?");
			list->insert_str(xa->addr, str, xref_type(x->type), str2);
               xa = (ht_addr*)x_tree->enum_next((ht_data**)&x, xa);
          }          
		list->update();
		dialog->insert(list);
		int r = dialog->run(0);
		if (r == button_ok) {
			ht_listbox_data data;
			list->databuf_get(&data);
			goto_address(data.cursor_id, this);
		}
		dialog->done();
		delete dialog;
	} else {
		if (confirmbox("No xrefs for address %08lx!\nSearch for xrefs?", Addr)==button_yes) {
			search_for_xrefs(Addr);
		}
	}
}

bool ht_aviewer::string_to_address(char *string, fmt_vaddress *vaddr)
{
	if (!analy) return false;
	scalar_t r;
	if (eval(&r, string, aviewer_func_handler, aviewer_symbol_handler, this)) {
		int_t i;
		scalar_context_int(&r, &i);
		ADDR a=i.value;
		if (analy->valid_addr(a, scvalid)) {
			*vaddr=a;
			return true;
		} else {
			sprintf(globalerror, "address %08x is invalid", a);
		}
		scalar_destroy(&r);
	} else {
		char *s;
		int p;
		get_eval_error(&s, &p);
		sprintf(globalerror, "%s at pos %d", s, p);
	}
	return false;
}

/*
 *	CLASS ht_analy_sub
 */

void ht_analy_sub::init(ht_streamfile *file, analyser *A, ADDR Lowestaddress, ADDR Highestaddress)
{
	ht_sub::init(file);
	analy = A;
	output = new analyser_ht_output();
	((analyser_ht_output*)output)->init(analy);
	lowestaddress=Lowestaddress;
	highestaddress=Highestaddress-1;
}

void ht_analy_sub::done()
{
	output->done();
	delete output;
	ht_sub::done();
}

bool ht_analy_sub::closest_line_id(ID *id1, ID *id2)
{
	if (!prev_line_id(id1, id2, 1)) {
		if (!next_line_id(id1, id2, 1)) {
			first_line_id(id1, id2);
		}
	}
	return true;
}

bool	ht_analy_sub::convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2)
{
	if (!analy) return false;
	ADDR a=addr; /* FIXME: */
	if (analy->valid_addr(a, scvalid)) {
		*id1=addr;
		*id2=0;
		return true;
	}
	return false;
}

bool	ht_analy_sub::convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr)
{
	*addr=id1;
	return 1;
}

bool ht_analy_sub::convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2)
{
	fmt_vaddress a;
	if (!uformat_viewer->offset_to_address(offset, &a)) return false;
	return convert_addr_to_id(a, id1, id2);
}

void	ht_analy_sub::first_line_id(ID *id1, ID *id2)
{
	*id1=lowestaddress;
	*id2=0;
}

bool ht_analy_sub::getline(char *line, ID id1, ID id2)
{
#if 1
	if (!analy) return false;
	return output->get_line_str(line, id1, id2);
/*     char bbuf[1024];
	tag_striptags(bbuf, line);
	fprintf(stdout, "gfl(%x, %d) -> %s\n", id1, id2, bbuf);
	fflush(stdout);*/
#else
	int len;
	analy->get_formatted_line(id1, id2+1, line, &len);
/*	if (id1==0x703b1415) {
		int a=1;
	}*/
	char bbuf[1024];
	tag_striptags(bbuf, line);
	fprintf(stdout, "gfl(%x, %d) -> %s\n", id1, id2, bbuf);
	return true;
#endif
}

void	ht_analy_sub::last_line_id(ID *id1, ID *id2)
{
	*id1=highestaddress;
	*id2=0;
}

int	ht_analy_sub::next_line_id(ID *id1, ID *id2, int n)
{
	if (!analy) return false;
	return output->next_line((ADDR*)id1, (int*)id2, n, highestaddress);
/*	int len;
	int res=0;
	while (n--) {
		if (analy->get_formatted_line_length(*id1, *id2+2, &len)) {
			(*id2)++;
		} else {
			analy->get_formatted_line_length(*id1, *id2+1, &len);
			if (*id1>=highestaddress) return res;
			*id1 += len;
			*id2 = 0;
		}
		res++;
	}
	return res;*/
}

int	ht_analy_sub::prev_line_id(ID *id1, ID *id2, int n)
{
	if (!analy) return false;
#if 1
	return output->prev_line((ADDR*)id1, (int*)id2, n, lowestaddress);
#else
/* FIXME: interesting solution! */
	ADDR addrbuf[1024];
	int	linebuf[1024];
	int	i=0;
	int len;

/*
 *	24==1*2^3*3
 *   so it "supports" opcodes of length 1, 2, 3, 4, 6, 8 and 12
 *   (and of course 24)
 */
	ADDR	l=n*24;

/*
 *	120==24*5
 *   so 120 additionally "supports" opcodes of length 5, 10 and 15
 *   (and some more)
 */
	if (l<120) l=120;

	/* look backwards */
//     fprintf(stdout, "============= up ==============================\n");
//     fprintf(stdout, "%d x from %lx line %ld\n", n, *id1, *id2);
	ADDR tmpaddr = (*id1) - l;
	ADDR tmpaddr2;
	dword tmpline;
	first_line_id(&tmpaddr2, &tmpline);
	if ((l > (*id1)) || (tmpaddr < tmpaddr2)) tmpaddr = tmpaddr2;
	ADDR	nextaddr = tmpaddr;
	while (1) {
		if (tmpaddr >= (*id1)) {
			if ((tmpline >= (*id2)) || (tmpaddr > *id1)) break;
		}
		if (analy->get_formatted_line_length(tmpaddr, tmpline+1, &len)) {
			nextaddr += len;
			tmpline++;
		} else {
			tmpaddr = nextaddr;
			tmpline = 0;
			continue;
		}
//          fprintf(stdout, "%2d. %lx line %ld\n", i, tmpaddr, tmpline-1);
		addrbuf[i&(1023)] = tmpaddr;
		linebuf[i&(1023)] = tmpline-1;
		i++;
	}

	if (!i) return 0;

	if (i >= n) {
//     	fprintf(stdout, "result1: %lx line %d (%d)\n", addrbuf[i-n], linebuf[i-n], n);
		(*id1) = addrbuf[(i-n)&(1023)];
		(*id2) = linebuf[(i-n)&(1023)];
		return n;
	} else {
//     	fprintf(stdout, "result2: %lx line %d (%d)\n", addrbuf[0], linebuf[0], i);
		(*id1) = addrbuf[0];
		(*id2) = linebuf[0];
		return i;
	}
#endif
}

ht_search_result *ht_analy_sub::search(ht_search_request *search, FILEOFS start, FILEOFS end)
{
	ADDR st = 0;
	ht_search_result *r = NULL;
	while (!r) {
		area_s *s = analy->initialized->get_area(analy->initialized->find_next(st));
		if (!s) break;
		st = s->end;
		FILEOFS fofs;
		FILEOFS fend;
		dword fsize;
		if (!uformat_viewer->address_to_offset(s->start, &fofs)) assert(0);
		if (!uformat_viewer->address_to_offset(s->end-1, &fend)) assert(0);
		fsize = fend-fofs;
		if ((search->search_class==SC_PHYSICAL) && (search->type==ST_EXPR)) {
			r = linear_expr_search(search, start, end, this, uformat_viewer, fofs, fsize);
		} else if ((search->search_class==SC_PHYSICAL) && (search->type==ST_FXBIN)) {
			r = linear_bin_search(search, start, end, file, fofs, fsize);
		}
	}
	return r;
}

void	ht_analy_sub::set_analyser(analyser *Analy)
{
	analy = Analy;
	output->analy = Analy;
	output->invalidate_cache();
}

