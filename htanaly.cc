/*
 *	HT Editor
 *	htanaly.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#include <cstdlib>
#include <cstring>
#include <memory>

#include "analy.h"
#include "analy_names.h"
#include "log.h"
#include "htanaly.h"
#include "htctrl.h" // FIXME: globalerror
#include "htdialog.h"
#include "htdisasm.h"
#include "hthist.h"
#include "htidle.h"
#include "htiobox.h"
#include "keyb.h"
#include "htmenu.h"
#include "htsearch.h"
#include "strtools.h"
#include "httag.h"
#include "httree.h"
#include "language.h"
#include "textedit.h"
#include "textfile.h"
#include "tools.h"
#include "snprintf.h"
#include "syntax.h"
#include "out.h"
#include "out_ht.h"
#include "out_txt.h"
#include "store.h"

extern "C" {
#include "evalx.h"
}

/* FIXME: test */
//#include "srt.h"
#include "out_html.h"
#include "out_sym.h"

/*
 *   AnalyserInformation
 */
void	AnalyserInformation::init(Bounds *b, ht_aviewer *a)
{
	analy = a;
	assert(a);
	ht_statictext::init(b, 0, align_left);
	register_idle_object(this);
	idle();
}

void AnalyserInformation::done()
{
	unregister_idle_object(this);
	ht_statictext::done();
}

int AnalyserInformation::gettext(char *buf, int maxlen)
{
	return ht_snprintf(buf, maxlen,
		"Analyser statistics:\n"
		"====================\n\n"          
		"Type: %s\nFile: %y\n"
		"Using disassembler: %s\n\n"
		"Known locations: %d\n"
		"Known symbols: %d\n\n",
		atype, &aname,
		adis,
		addrs, labels);
}

bool AnalyserInformation::idle()
{
	if (analy && analy->analy) {
		addrs = analy->analy->getLocationCount();
		labels = analy->analy->getSymbolCount();
		atype = analy->analy->getType();
		analy->analy->getName(aname);
		if (analy->analy->disasm) {
			adis = analy->analy->disasm->getName();
		} else {
			adis = "?";
		}
		dirtyview();
	}
	return false;
}

/*
 *   SymbolBox
 */

void SymbolBox::init(Bounds *b, Analyser *Analy)
{
	analy = Analy;
	ht_listbox::init(b);
	str = ht_malloc(1024);
	symbols = analy->getSymbolCount();
	idle_count = 1;
}

void SymbolBox::done()
{
	free(str);
	ht_listbox::done();
}

int  SymbolBox::calcCount()
{
	return analy->getSymbolCount();
}

int  SymbolBox::cursorAdjust()
{
	Symbol *l = ((Symbol *)getFirst());
	if (!l) return 0;
	return l->location->addr->stringSize()+10;
}

int  SymbolBox::estimateEntryPos(void *entry)
{
	return 0;
}

void *SymbolBox::getFirst()
{
	return analy->enumSymbols(NULL);
}

void *SymbolBox::getLast()
{
	return analy->enumSymbolsReverse(NULL);
}

void *SymbolBox::getNext(void *entry)
{
	if (!entry) return NULL;
	return analy->enumSymbols((Symbol *)entry);
}

void *SymbolBox::getPrev(void *entry)
{
	if (!entry) return NULL;
	return analy->enumSymbolsReverse((Symbol *)entry);
}

const char *SymbolBox::getStr(int col, void *entry)
{
	if (!entry) return NULL;
	Symbol *l = ((Symbol *)entry);
	switch (col) {
	case 0:
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_WHITESPACE;
		ht_snprintf(str, 1024, "%y", l->location->addr);
		break;
	case 1:
		ht_snprintf(str, 1024, "%s", label_type_short(l->type));
		break;
	case 2:
		ht_snprintf(str, 1024, "%s", l->name);
		break;
	}
	return str;
}

bool SymbolBox::idle()
{
	if ((idle_count % 500)==0) {
		update();
		redraw();
		symbols = analy->getSymbolCount();
		idle_count = 1;
		return 1;
	}
	idle_count++;
	return 0;
}

int SymbolBox::numColumns()
{
	return 3;
}

void *SymbolBox::quickfind(const char *s)
{
	Symbol *tmp = analy->getSymbolByName(s);
	if (tmp) return tmp;
	tmp = analy->enumSymbolsByName(s);
	if (tmp) {
		int slen = strlen(s);
		int tlen = strlen(tmp->name);
		if (slen > tlen) return NULL;
		if (ht_strncmp(tmp->name, s, slen)==0) return tmp;
		return NULL;
	} else {
		return NULL;
	}
}


char *SymbolBox::quickfindCompletition(const char *s)
{
	if (analy->getSymbolByName(s)) {
		return ht_strdup(s);
	}
	Symbol *tmp = analy->enumSymbolsByName(s);
	if (!tmp) {
		return ht_strdup(s);
	}
	Symbol *tmp2 = analy->enumSymbols(tmp);
	if (!tmp2) {
		return ht_strdup(tmp->name);
	}
	int slen = strlen(s);
	if (!ht_strncmp(tmp->name, tmp2->name, slen)==0) {
		return ht_strdup(tmp->name);
	}
	char *res = ht_malloc(1024); // buffer bla
	strcpy(res, tmp->name);
	while (tmp2 && (ht_strncmp(tmp2->name, s, slen)==0)) {
//		fprintf(stdout, "while(%s, %s, %d)\n", tmp2->name, s, slen);
		int a = ht_strccomm(res, tmp2->name);
		res[a] = 0;
		tmp2 = analy->enumSymbols(tmp2);
	}
	return res;
}


/*
 *	CLASS CallChain
 */
void	CallChain::init(Bounds *b, Analyser *Analy, Address *a, char *desc)
{
	ht_treeview::init(b, desc);
	VIEW_DEBUG_NAME("CallChain");
	analy = Analy;
	root = createNode(a);
}

void CallChain_done(CallChainNode *n)
{
	while (n) {
		CallChain_done(n->child);
		CallChainNode *temp = n->next;
		free(n);
		n = temp;
	}
}

void	CallChain::done()
{
	CallChain_done(root);
	ht_treeview::done();
}

void	CallChain::adjust(void *node, bool expand)
{
	((CallChainNode*)node)->expanded = expand;
}

CallChainNode *CallChain::createNode(Address *a)
{
	CallChainNode *n = ht_malloc(sizeof(CallChainNode));
	n->next = NULL;
	n->prev = NULL;
	n->child = NULL;
	n->examined = false;
	n->xa = a->clone();
	n->faddr = analy->getFunctionByAddress(a);
	assert(n->faddr);
	n->fa = n->faddr->addr;
	n->expanded = false;
	return n;
}

void CallChain::examineNode(CallChainNode *n)
{
	n->examined = true;
	if (has_children(n)) {
		Container *x_tree = n->faddr->xrefs;
		assert(x_tree);
		ObjHandle oh = x_tree->findFirst();
		AddrXRef *x = (AddrXRef *)x_tree->get(oh);
		assert(x);
		CallChainNode *nn = n->child = createNode(x->addr);
		while ((oh = x_tree->findNext(oh)) != invObjHandle) {
			x = (AddrXRef *)x_tree->get(oh);
			nn->next = createNode(x->addr);
			nn = nn->next;
		}
	}
}

void	*CallChain::get_child(void *node, int i)
{
	CallChainNode *p;
	if (node) {
		if (!((CallChainNode *)node)->examined) examineNode((CallChainNode*)node);
		p = ((CallChainNode *)node)->child;
	} else {
		p = root;
	}
	while (p && (--i)) p = p->next;
	return p;
}

void	*CallChain::get_next_node(void *node)
{
	return ((CallChainNode*)node)->next;
}

void	*CallChain::get_prev_node(void *node)
{
	return ((CallChainNode*)node)->prev;
}

void	*CallChain::get_root()
{
	return root;
}

char *CallChain::get_text(void *node)
{
	static char stupid[1024]; // FIXME: static var
	CallChainNode *n = (CallChainNode*)node;
	int d = 0;
	n->xa->difference(d, n->faddr->addr);
	char sign = '+';
	if (d < 0) {
		d = -d;
		sign = '-';          
	}
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
	ht_snprintf(stupid, sizeof stupid, "%s%c%x (%y)", 
		n->faddr->label ? n->faddr->label->name : "unknown", 
		sign, 
		d, 
		n->xa
	);
	return stupid;
}

bool	CallChain::has_children(void *node)
{
	return ((CallChainNode*)node)->faddr->xrefs != NULL;
}

bool	CallChain::is_expanded(void *node)
{
	return ((CallChainNode*)node)->expanded;
}

void	CallChain::select_node(void *node)
{
}


/////////////////////////////////////////////////////////////////////////////

/*
 *
 */
void AnalyInfoline::init(Bounds *b, ht_aviewer *A, const char *Format)
{
	ht_statictext::init(b, 0, align_left);
	VIEW_DEBUG_NAME("AnalyInfoline");
	analy = A;
	displayformat = ht_strdup(Format);
	addr = new InvalidAddress();
	fofs = INVALID_FILE_OFS;
}

void AnalyInfoline::done()
{
	free(displayformat);
	delete addr;
	ht_statictext::done();
}

int AnalyInfoline::gettext(char *buf, int maxlen)
{
	if (maxlen <= 0) return 0;
	if (valid()) {
		const char *sec = analy->analy->getSegmentNameByAddress(addr);
		if (fofs != INVALID_FILE_OFS) {
			Location *a = analy->analy->getFunctionByAddress(addr);
			const char *func = analy->analy->getSymbolNameByLocation(a);

			char *d = displayformat;
			char *ss = buf, *s = buf;
			while (*d) {
				if (*d == '%') {
					d++;
					switch (*d) {
					case ANALY_STATUS_ARG_SECTION:
						if (sec) ss += ht_snprintf(ss, maxlen-(ss-s), "%s", sec);
						break;
					case ANALY_STATUS_ARG_FILEOFFSET:
						ss += ht_snprintf(ss, maxlen-(ss-s), "%08qx", fofs);
						break;
					case ANALY_STATUS_ARG_RAW_UNASM: {
						int length;
						ss += ht_snprintf(ss, maxlen-(ss-s), "%s", analy->analy->getDisasmStr(addr, length));
						break;
					}
					case ANALY_STATUS_ARG_FUNCTION: {
						if (func) {
							int d = 0;
							addr->difference(d, a->addr);
							char sign = '+';
							if (d < 0) {
								d =- d;
								sign = '-';
							}
							ss += ht_snprintf(ss, maxlen-(ss-s), "%s%c%x", func, sign, d);
						}
						break;
					}
					case ANALY_STATUS_ARG_OFFSET:
						global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
						ss += ht_snprintf(ss, maxlen-(ss-s), "%y", addr);
						break;
					case '%':
						if (maxlen-(ss-s) > 1) {
							*ss++ = '%';
						}
						break;
					default:
						ss += ht_snprintf(ss, maxlen-(ss-s), " error in format ");
					}
				} else {
					if (maxlen - (ss-s) > 1) {
						// leave space for trailing zero
						*ss++ = *d;
					}
				}
				d++;
			}
			*ss = 0;
			return ss - s;
		} else {
			if (!sec) {
				return ht_strlcpy(buf, "[not in file]", maxlen);
			} else {
				return ht_snprintf(buf, maxlen, "<%s> [not in file]", sec);
			}
		}
	} else {
		return ht_strlcpy(buf, "<no analyser>", maxlen);
	}
}

void AnalyInfoline::update(Address *cursor_addr, FileOfs ecursor_addr)
{
	delete addr;
	if (valid()) {
		fofs = ecursor_addr;
		addr = cursor_addr->clone();
	} else {
		fofs = INVALID_FILE_OFS;
		addr = new InvalidAddress();
	}
	dirtyview();
	redraw();
}

bool AnalyInfoline::valid()
{
	return analy && analy->analy;
}

/*
 *
 */
void ht_aviewer::init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Analyser *Analy)
{
	analy = Analy;
	if (Analy) {
		analy->setDisplayMode(ANALY_SHOW_ADDRESS | ANALY_SHOW_COMMENTS
			| ANALY_SHOW_LABELS | ANALY_SHOW_XREFS
			| ANALY_TRANSLATE_SYMBOLS | ANALY_COLLAPSE_XREFS, 
			ANALY_EDIT_BYTES);
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

void ht_aviewer::attachInfoline(AnalyInfoline *V)
{
	infoline = V;
}

bool ht_aviewer::pos_to_offset(viewer_pos p, FileOfs *ofs)
{
	if (analy) {
		Address *addr;
		if (!convertViewerPosToAddress(p, &addr)) return false;
		FileOfs o=analy->addressToFileofs(addr);
		delete addr;
		if (o != INVALID_FILE_OFS) {
			*ofs=o;
			return true;
		}
	}
	return false;
}

bool ht_aviewer::pos_to_string(viewer_pos p, char *result, int maxlen)
{
	if (!analy) return false;
	Address *a;
	if (!convertViewerPosToAddress(p, &a)) return false;
	Location *addr = analy->getLocationByAddress(a);
	if (addr && addr->label) {
		ht_strlcpy(result, addr->label->name, maxlen);
		return true;
	}
	addr = analy->getFunctionByAddress(a);
	if (addr && addr->label) {
		int d = 0;
		a->difference(d, addr->addr);
		ht_snprintf(result, maxlen, "%s+0%xh", addr->label->name, d);
		return true;
	}
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS | ADDRESS_STRING_FORMAT_ADD_H;
	ht_snprintf(result, maxlen, "%y", a);
	return true;
}

bool ht_aviewer::convertViewerPosToAddress(const viewer_pos &p, Address **a)
{
	*a = analy->createAddress();
	(*a)->getFromUInt64((uint64(p.u.line_id.id1) << 32) + p.u.line_id.id2);
	return true;
}

bool ht_aviewer::convertAddressToViewerPos(Address *a, viewer_pos *p)
{
	if (a && a->isValid()) {
		clear_viewer_pos(p);
		p->u.sub = analy_sub;
		uint64 u;
		a->putIntoUInt64(u);
		p->u.line_id.id1 = u >> 32;
		p->u.line_id.id2 = u;
		p->u.line_id.id3 = 0;
		return true;
	} else {
		return false;
	}
}

const char *ht_aviewer::func(uint i, bool execute)
{
	switch (i) {
	case 8:
		if (execute) {
			sendmsg(cmd_analyser_symbols);
		}
		return "symbols";
	default:
		return ht_uformat_viewer::func(i, execute);
	}
	return NULL;
}

static int aviewer_func_addr(eval_scalar *result, eval_str *str)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	
	Address *addr = aviewer->analy->createAddress();
	int l = addr->parseString(str->value, str->len, aviewer->analy);
	if (l) {
		uint64 q;
		if (addr->putIntoUInt64(q)) {
			scalar_create_int_q(result, q);
			return 1;
		}
	} else {
		char buffer[1024];
		bin2str(buffer, str->value, MIN((uint)str->len, sizeof buffer));
		set_eval_error("invalid address '%s'", buffer);
	}
	return 0;
}

static int aviewer_func_address_of(eval_scalar *result, eval_str *str)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	char buffer[1024];
	bin2str(buffer, str->value, MIN((uint)str->len, sizeof buffer));
	Symbol *l;
	if ((l = aviewer->analy->getSymbolByName(buffer))) {
		uint64 q;
		if (l->location->addr->putIntoUInt64(q)) {
			scalar_create_int_q(result, q);
			return 1;
		}
	} else {
		set_eval_error("invalid label '%s'", buffer);
	}
	return 0;
}

static int aviewer_func_fileofs(eval_scalar *result, eval_int *i)
{
	ht_aviewer *aviewer = (ht_aviewer*)eval_get_context();
	viewer_pos p;
	if (aviewer->offset_to_pos(i->value, &p)) {
		Address *a;
		uint64 q;
		aviewer->convertViewerPosToAddress(p, &a);
		std::auto_ptr<Address> blub(a);
		if (a->putIntoUInt64(q)) {
			scalar_create_int_q(result, q);
			return 1;
		}
	} else {
		set_eval_error("invalid file offset or no corresponding address for '0%xh'", i->value);
	}
	return 0;
}

/*
 *	for assembler
 */
static int ht_aviewer_symbol_to_addr(void *Aviewer, const char *s, uint64 &v)
{
	// FIXNEW
	ht_aviewer *aviewer = (ht_aviewer*)Aviewer;
	Address *a;
	if (*s == '@') {
		s++;
		if (str2int(s, v, 10)) {
			viewer_pos vp;
			if (!aviewer->offset_to_pos(v, &vp)) {
				set_eval_error("invalid offset: %08qx", v);
				return false;
			}
			aviewer->convertViewerPosToAddress(vp, &a);
			std::auto_ptr<Address> blub(a);
			if (a->putIntoUInt64(v)) {
				return true;
			}
		}
		// invalid number after @
		return false;
	} else if (strcmp(s, "&") ==0) {
		if (aviewer->getCurrentAddress(&a)) {
			a->putIntoUInt64(v);
			delete a;
			return true;
		} else {
			return false;
		}
	} else {
		Symbol *l = aviewer->analy->getSymbolByName(s);
		if (l) {
			// Label
			a = l->location->addr;
			if (a->putIntoUInt64(v)) {
				return true;
			}
		}
	}
	return false;
}

static void setdatastr(ht_view *v, char *str)
{
	ht_inputfield_data id;
	id.textlen = strlen(str);
	id.text = (byte*)str;
	v->databuf_set(&id, sizeof id);
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

void ht_aviewer::generateOutputDialog()
{
	if (!analy) return;
	if (analy->active) {
		infobox("Please wait until analyser has finished before generating output file!");
		if (analy->active) return;
	}

	Bounds b;
	b.w=50;
	b.h=15;
	center_bounds(&b);
	ht_dialog *dialog;
	NEW_OBJECT(dialog, ht_dialog, &b, "generate analyser output", FS_KILLER | FS_TITLE | FS_MOVE);
	ht_view *v1, *v2;
	b.assign(2, 2, 25, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	b.assign(2, 1, 25, 1);
	NEW_OBJECT(v2, ht_label, &b, "output ~filename:", v1);
	dialog->insert(v2);

	String filename, basename, basename2, suffix;
	file->getFilename(filename);
	filename.rightSplit('/', suffix, basename2);
	basename2.rightSplit('.', basename, suffix);
	basename += ".out";
	setdatastr(v1, basename.contentChar());

	b.assign(29, 2, 15, 1);
	NEW_OBJECT(v1, ht_listpopup, &b);
	((ht_listpopup*)v1)->insertstring("HTML");
	((ht_listpopup*)v1)->insertstring("plain text");
	dialog->insert(v1);
	b.assign(29, 1, 15, 1);
	NEW_OBJECT(v2, ht_label, &b, "~output format:", v1);
	dialog->insert(v2);
	b.assign(2, 5, 35, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	b.assign(2, 4, 35, 1);
	NEW_OBJECT(v2, ht_label, &b, "~start address:", v1);
	viewer_pos cur;
	if (get_current_pos(&cur)) {
		char str[1024];
		pos_to_string(cur, str, sizeof str);
		setdatastr(v1, str);
	}
	dialog->insert(v2);
	b.assign(2, 8, 35, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	b.assign(2, 7, 35, 1);
	NEW_OBJECT(v2, ht_label, &b, "~end address:", v1);
	dialog->insert(v2);
//	setdatastr(v1, "#1000");
	if (get_current_pos(&cur)) {
		char str[1024];
		pos_to_string(cur, str, sizeof str);
		strcat(str, "+20");
		setdatastr(v1, str);
	}
	b.assign(13, 11, 9, 2);
	NEW_OBJECT(v1, ht_button, &b, "O~k", button_ok);
	dialog->insert(v1);
	b.assign(27, 11, 9, 2);
	NEW_OBJECT(v1, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(v1);
	while (dialog->run(false) == button_ok) {
		char filename[260];
		char start_str[1024], end_str[1024];
		viewer_pos start, end;
		output_dialog_data odd;
		ViewDataBuf vdb(dialog, &odd, sizeof odd);
		getdatastr(&odd.id1, filename);
		getdatastr(&odd.id2, start_str);
		getdatastr(&odd.id3, end_str);
		if (start_str[0] == 0) {
			convertAddressToViewerPos(analy_sub->lowestaddress, &start);
		} else {
			if (!string_to_pos(start_str, &start)) {
				errorbox(globalerror);
				continue;
			}
		}
		if (end_str[0] == 0) {
			convertAddressToViewerPos(analy_sub->highestaddress, &end);
		} else {
			if (!string_to_pos(end_str, &end)) {
				errorbox(globalerror);
				continue;
			}
		}
		Address *start_addr, *end_addr;
		if (!convertViewerPosToAddress(start, &start_addr) || !convertViewerPosToAddress(end, &end_addr)) {
			errorbox("invalid address");
			continue;
		}
		
		try {
			String name(filename);
			LocalFile s(name, IOAM_WRITE, FOM_CREATE);
			AnalyserOutput *out = NULL;
			switch (odd.lp.cursor_pos) {
			case 0:
				out = new AnalyserHTMLOutput();
				((AnalyserHTMLOutput*)out)->init(analy, &s);
				break;
			case 1:
				out = new AnalyserTxtOutput();
				((AnalyserTxtOutput*)out)->init(analy, &s);
				break;
			}
			out->generateFile(start_addr, end_addr);
			out->done();
			delete out;
		} catch (const IOException &e) {
			infobox("couldnt create file '%y': %y.", &filename, &e);
			continue;
		}
		break;
	}
	dialog->done();
	delete dialog;
}

bool ht_aviewer::canCreateAddress(Address *addr, bool error_msg)
{
	Location *ctx = analy->getLocationContextByAddress(addr);
	if (ctx && ctx->addr->compareTo(addr) != 0) {
		if (error_msg) {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
			errorbox("Can't create new symbol: Address %y belongs to %y (%s)", addr, ctx->addr, ctx->label ? ctx->label->name : "unnamed");
		}
		return false;
	}
	return true;
}

void ht_aviewer::dataIntDialog(taddr_int_subtype subtype, int length)
{
	if (!analy) return;
	Address *current_address;
	if (!getCurrentAddress(&current_address)) return;
	if (!canCreateAddress(current_address, true)) {
		delete current_address;
		return;
	}
	
	if (analy->validAddress(current_address, scinitialized)) {
		analy->data->setIntAddressType(current_address, subtype, length);
	}
	delete current_address;
}

void ht_aviewer::dataStringDialog()
{
	if (!analy) return;
	Address *current_address;
	if (!getCurrentAddress(&current_address)) return;
	if (!canCreateAddress(current_address, true)) {
		delete current_address;
		return;
	}
/*
	Bounds b;
	b.w=50;
	b.h=15;
	center_bounds(&b);
	ht_dialog *dialog;
	NEW_OBJECT(dialog, ht_dialog, &b, "interprete data as string", FS_KILLER | FS_TITLE | FS_MOVE);
	
	while (dialog->run(false)==button_ok) {
	}
	dialog->done();
	delete dialog;*/
	
	if (analy->validAddress(current_address, scinitialized)) {
		byte buffer[1024];
		Location *a = analy->enumLocations(current_address);
		int d = sizeof buffer;
		if (a) a->addr->difference(d, current_address);
		uint bz = analy->bufPtr(current_address, buffer, MIN(sizeof buffer, (uint)d));
		if (bz > 2) {
			analy_string *str = string_test(buffer, bz);
			if (str) {
				char string1[128], string2[128];
				str->render_string(string2, sizeof string2);
				ht_snprintf(string1, sizeof string1, "%s_%s", str->name(), string2);
				make_valid_name(string2, string1);
				if (analy->addAddressSymbol(current_address, string2, label_data)) {
					analy->addComment(current_address, 0, "");
				}
				analy->data->setArrayAddressType(current_address, dst_string, str->length());
				str->done();
				delete str;
			}
		}
	}
	delete current_address;
}

struct export_dialog_data {
	ht_inputfield_data id1;
	ht_listpopup_data lp;
};

void ht_aviewer::exportFileDialog()
{
	if (!analy) return;
	if (analy->active) {
		infobox("Please wait until analyser has finished before exporting file!");
		if (analy->active) return;
	}

	Bounds b;
	b.w=50;
	b.h=12;
	center_bounds(&b);
	ht_dialog *dialog;
	NEW_OBJECT(dialog, ht_dialog, &b, "export analyser information", FS_KILLER | FS_TITLE | FS_MOVE);
	ht_view *v1, *v2;
	b.assign(2, 2, 35, 1);
	NEW_OBJECT(v1, ht_strinputfield, &b, 260);
	dialog->insert(v1);
	b.assign(2, 1, 35, 1);
	NEW_OBJECT(v2, ht_label, &b, "output ~filename:", v1);
	dialog->insert(v2);

	String filename, basename, suffix;
	file->getFilename(filename);
	filename.rightSplit('.', basename, suffix);
	basename += ".exp";
	setdatastr(v1, basename.contentChar());

	b.assign(2, 5, 25, 1);
	NEW_OBJECT(v1, ht_listpopup, &b);
	((ht_listpopup*)v1)->insertstring(".sym symbol file");
	dialog->insert(v1);
	b.assign(2, 4, 25, 1);
	NEW_OBJECT(v2, ht_label, &b, "~export format:", v1);
	dialog->insert(v2);
	
	b.assign(13, 8, 9, 2);
	NEW_OBJECT(v1, ht_button, &b, "O~k", button_ok);
	dialog->insert(v1);
	b.assign(27, 8, 9, 2);
	NEW_OBJECT(v1, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(v1);
	while (dialog->run(false) == button_ok) {
		char filename[260];
		export_dialog_data edd;
		ViewDataBuf vdb(dialog, &edd, sizeof edd);
		getdatastr(&edd.id1, filename);
		
		String name(filename);
		LocalFile s(name, IOAM_WRITE, FOM_CREATE);
		try {
			switch (edd.lp.cursor_pos) {
			case 0:
				export_to_sym(analy, &s);
				break;
			}
		} catch (const IOException &) {
			infobox("couldnt create file '%s'.", filename);
			continue;
		}
		break;
	}
	dialog->done();
	delete dialog;
}

bool ht_aviewer::getCurrentAddress(Address **a)
{
	viewer_pos vp;
	if (!get_current_pos(&vp)) return false;
	return convertViewerPosToAddress(vp, a);
}

bool ht_aviewer::get_current_offset(FileOfs *ofs)
{
	if (ht_uformat_viewer::get_current_offset(ofs)) {
		return true;
	} else {
		Address *a;
		if (!getCurrentAddress(&a)) return false;
		*ofs = analy->addressToFileofs(a);
		delete a;
		return *ofs != INVALID_FILE_OFS;
	}
}

bool ht_aviewer::get_hscrollbar_pos(int *pstart, int *psize)
{
	if (analy_sub) {
		// FIXNEW
/*		if (analy_sub->highestaddress->difference(s, analy_sub->lowestaddress) && s) {
			int z=MIN(size.h*16, (int)(top.line_id.id1-analy_sub->lowestaddress));
			return scrollbar_pos(top.line_id.id1-analy_sub->lowestaddress, z, s, pstart, psize);
		}*/
	}
	return false;
}

void ht_aviewer::getminbounds(int *width, int *height)
{
	*width = 25;
	*height = 4;
}

int ht_aviewer::get_pindicator_str(char *buf, int max_len)
{
	Address *addr;
	if (analy && getCurrentAddress(&addr)) {
		std::auto_ptr<Address> blub(addr);
		FileOfs o;
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT;
		if (get_current_offset(&o)) {
			return ht_snprintf(buf, max_len, " %y/@%08qx%s ", addr, o, (analy->isDirty())?" dirty":"");
		} else {
			return ht_snprintf(buf, max_len, " %y%s ", addr, (analy->isDirty())?" dirty":"");
		}
	} else {
		return ht_snprintf(buf, max_len, " ? ");
	}
}

bool ht_aviewer::gotoAddress(Address *a, ht_view *source_object)
{
	viewer_pos p;
	if (analy) {
		if (!analy->validAddress(a, scvalid)) return false;
	}
	// FIXME: insert a->compare(hi, low address) here
	if (convertAddressToViewerPos(a, &p)) {
		return goto_pos(p, source_object);
	}
	return false;
}

void ht_aviewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_contextmenuquery: {
		ht_static_context_menu *m = new ht_static_context_menu();
		m->init("~Analyser");
		m->insert_entry("~Assemble...", "Ctrl-A", cmd_analyser_call_assembler, K_Control_A, 1);
		m->insert_separator();
		ht_static_context_menu *sub=new ht_static_context_menu();
		sub->init("~Control");
		sub->insert_entry("~Information...", NULL, cmd_analyser_info, 0, 1);
		sub->insert_separator();
		sub->insert_entry("~Save state", NULL, cmd_analyser_save, 0, 1);
		sub->insert_entry("~Export to file...", "Ctrl-O", cmd_analyser_generate_output, K_Control_O, 1);
		sub->insert_separator();
		sub->insert_entry("~Continue at address", "c", cmd_analyser_continue, 0, 1);
		sub->insert_entry("~Pause / resume", "p", cmd_analyser_pause_resume, 0, 1);
		m->insert_submenu(sub);
		sub=new ht_static_context_menu();
		sub->init("~Jump");
		sub->insert_entry("~Symbol list", "F8", cmd_analyser_symbols, 0, 1);
		sub->insert_entry("~Function start", "Ctrl-F", cmd_analyser_this_function, K_Control_F, 1);
		sub->insert_entry("Prev ~label", "Ctrl-L", cmd_analyser_previous_label, K_Control_L, 1);
		sub->insert_entry("Follow ~ptr", "f", cmd_analyser_follow, 0, 1);
		m->insert_submenu(sub);
		sub=new ht_static_context_menu();
		sub->init("~Location");
		sub->insert_entry("~Name address (label)...", "n", cmd_analyser_name_addr, 0, 1);
		sub->insert_entry("Edit co~mments", "#", cmd_analyser_comments, 0, 1);
		sub->insert_entry("Show ~xrefs", "x", cmd_analyser_xrefs, 0, 1);
		sub->insert_entry("~Delete location", "Del", cmd_analyser_del_addr_bindings, 0, 1);
		sub->insert_entry("~Callchain", "Ctrl-T", cmd_analyser_call_chain, K_Control_T, 1);
		m->insert_submenu(sub);
		sub=new ht_static_context_menu();
		sub->init("~Data");
		sub->insert_entry("Data ~string", "s", cmd_analyser_data_string, 0, 1);
		sub->insert_entry("Data ~word 32", "i", cmd_analyser_data_int, 0, 1);
		sub->insert_entry("Data ~halfword 16", "h", cmd_analyser_data_half, 0, 1);
		sub->insert_entry("Data ~byte 8", "b", cmd_analyser_data_byte, 0, 1);
		m->insert_submenu(sub);
//		m->insert_separator();
//		m->insert_entry("Symbol reg trace (exp!)", "Alt-Q", cmd_analyser_srt, K_Meta_Q, 1);

		msg->msg = msg_retval;
		msg->data1.ptr = m;
		return;
	}
	case msg_keypressed:
		switch (msg->data1.integer) {
		case K_Control_L:
			sendmsg(cmd_analyser_previous_label);
			clearmsg(msg);
			return;
		case K_Control_F:
			sendmsg(cmd_analyser_this_function);
			clearmsg(msg);
			return;
		case 'c':
			if (cursor_tag_class == tag_class_sel) {
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
			if (cursor_tag_class == tag_class_sel) {
				sendmsg(cmd_analyser_follow_ex);
				clearmsg(msg);
				return;
			}
			break;
		case 'f':
			if (cursor_tag_class == tag_class_sel) {
				sendmsg(cmd_analyser_follow);
				clearmsg(msg);
				return;
			}
			break;
		case 'n':
			sendmsg(cmd_analyser_name_addr);
			clearmsg(msg);
			return;
		case 'p':
			sendmsg(cmd_analyser_pause_resume);
			clearmsg(msg);
			return;
		case 'i':
			if (cursor_tag_class == tag_class_sel) {
				sendmsg(cmd_analyser_data_int);
				clearmsg(msg);
			}
			return;
		case 'h':
			if (cursor_tag_class == tag_class_sel) {
				sendmsg(cmd_analyser_data_half);
				clearmsg(msg);
				return;
			}
			break;
		case 'b':
			if (cursor_tag_class == tag_class_sel) {
				sendmsg(cmd_analyser_data_byte);
				clearmsg(msg);
				return;
			}
			break;
		case 's':
			sendmsg(cmd_analyser_data_string);
			clearmsg(msg);
			return;
		case 'x':
			sendmsg(cmd_analyser_xrefs);
			clearmsg(msg);
			return;
		case K_Control_D:
		case K_Delete:
			sendmsg(cmd_analyser_del_addr_bindings);
			clearmsg(msg);
			return;
		}
		break;
	case cmd_analyser_call_assembler: {
		if (!analy) break;
		Assembler *a = analy->createAssembler();
		if (!a) {
			// FIXME: select assembler for list
			infobox("no assembler available.");
			clearmsg(msg);
			return;
		}
		viewer_pos current_pos;
		Address *current_address;
		if (get_current_pos(&current_pos) && getCurrentAddress(&current_address)) {
			a->set_imm_eval_proc(ht_aviewer_symbol_to_addr, (void*)this);
			int want_length;
			analy->getDisasmStr(current_address, want_length);
			dialog_assemble(this, current_pos, analy->mapAddr(current_address), a, analy->disasm, analy->getDisasmStrFormatted(current_address), want_length);
			delete current_address;
		}			
		a->done();
		delete a;
		clearmsg(msg);
		return;
	}
	case cmd_analyser_this_function: {
		if (!analy) break;
		Address *c;
		if (!getCurrentAddress(&c)) break;
		Location *a = analy->getFunctionByAddress(c);
		if (a) {
			gotoAddress(a->addr, this);
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
			errorbox("Address %y doesn't belong to a function.", c);
		}
		delete c;
		clearmsg(msg);
		return;
	}
	case cmd_analyser_previous_label: {
		if (!analy) break;
		Address *c;
		if (!getCurrentAddress(&c)) break;
		Location *a = analy->getPreviousSymbolByAddress(c);
		if (a) {
			gotoAddress(a->addr, this);
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
			errorbox("Address %y doesn't belong to a symbol.", c);
		}
		delete c;
		clearmsg(msg);
		return;
	}
	case cmd_analyser_continue: {
		if (!analy) break;
		Address *a;
		if (!getCurrentAddress(&a)) break;
		analy->continueAnalysisAt(a);
		delete a;
		analy->makeDirty();
		analy_sub->output->invalidateCache();
		clearmsg(msg);
		return;
	}
	case cmd_analyser_comments: {
		if (!analy) break;
		Address *current_address;
		if (getCurrentAddress(&current_address)) {
			showComments(current_address);
			delete current_address;
		}
		clearmsg(msg);
		return;
	}
	case cmd_analyser_name_addr: {
		if (!analy) break;
		Address *addr = NULL;
		if (!getCurrentAddress(&addr) || !canCreateAddress(addr, true)) {
			delete addr;
			clearmsg(msg);
			return;
		}
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
		if (analy->validAddress(addr, scvalid)) {
			char n[256];
			char str[1024];
			Symbol *l = analy->getSymbolByAddress(addr);
			if (l) ht_strlcpy(n, l->name, sizeof n); else n[0] = 0;
			ht_snprintf(str, sizeof str, "name for address %y", addr);
			while (inputbox(str, "~label name:", n, 255, HISTATOM_NAME_ADDR)) {
				if (n[0]) {
					if (valid_name(n)) {
						char *n2 = ht_strdup(n);
						if (!analy->assignSymbol(addr, n2, (l)?l->type:label_unknown)) {
							l = analy->getSymbolByName(n);
							global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
							errorbox("Label '%s' already exists at address %y!", n, l->location->addr);
							free(n2);
						} else {
							analy->makeDirty();
							analy_sub->output->invalidateCache();
							break;
						}
					} else {
						global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
						if (confirmbox("'%s' is an invalid label name.\nMake valid?", n)==button_yes) {
							make_valid_name(n, n);
						}
					}
				} else {
					if (l) {
						// delete label if applicable
						global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
						if (confirmbox("Really delete label '%s' at address %y?", l->name, addr)==button_yes) {
							analy->deleteSymbol(addr);
							analy->makeDirty();
							analy_sub->output->invalidateCache();
						}
					}
					break;
				}
			}
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
			errorbox("Address %y is invalid!", addr);
		}
		delete addr;
		clearmsg(msg);
		return;
	}
	case cmd_analyser_xrefs: {
		if (!analy) break;
		Address *a;
		if (!getCurrentAddress(&a)) break;
		showXRefs(a);
		delete a;
		clearmsg(msg);
		return;
	}
	case cmd_analyser_follow: {
		if (!analy) break;
		Address *c;
		if (!getCurrentAddress(&c)) break;
		std::auto_ptr<Address> blub(c);
		if (!analy->validAddress(c, scinitialized)) break;
		Address *b = analy->createAddress();
		std::auto_ptr<Address> blub2(b);
		uint bz = b->byteSize();
		if (!bz) break;
		byte buf[bz];
		if (analy->bufPtr(c, buf, bz) != bz) break;
		b->getFromArray(buf);
		if (analy->validAddress(b, scvalid)) {
			gotoAddress(b, this);
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
			errorbox("Follow: address %y is invalid!", b);
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
		Address *addr;
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
		if (getCurrentAddress(&addr)) {
			if (confirmbox("Forget about address' %y bindings?\n(name, comments, xrefs, etc.)", addr)==button_yes) {
				analy->deleteLocation(addr);
				analy->makeDirty();
				analy_sub->output->invalidateCache();
			}
			delete addr;
		}
		clearmsg(msg);
		return;
	}
	case cmd_analyser_generate_output:
		if (!analy) break;
		generateOutputDialog();
		clearmsg(msg);
		return;
	case cmd_analyser_export_file:
		if (!analy) break;
		exportFileDialog();
		clearmsg(msg);
		return;
	case cmd_analyser_data_int:
		if (!analy) break;
		dataIntDialog(dst_idword, 4);
		analy->makeDirty();
		analy_sub->output->invalidateCache();
		dirtyview();
		clearmsg(msg);
		return;
	case cmd_analyser_data_half:
		if (!analy) break;
		dataIntDialog(dst_iword, 2);
		analy->makeDirty();
		analy_sub->output->invalidateCache();
		dirtyview();
		clearmsg(msg);
		return;
	case cmd_analyser_data_byte:
		if (!analy) break;
		dataIntDialog(dst_ibyte, 1);
		analy->makeDirty();
		analy_sub->output->invalidateCache();
		dirtyview();
		clearmsg(msg);
		return;
	case cmd_analyser_data_string:
		if (!analy) break;
		dataStringDialog();
		analy->makeDirty();
		analy_sub->output->invalidateCache();
		dirtyview();
		clearmsg(msg);
		return;
	case cmd_analyser_info:
		if (!analy) break;
		Address *current_address;
		if (!getCurrentAddress(&current_address)) {
			showInfo(0);
		} else {
			showInfo(current_address);
			delete current_address;
		}
		dirtyview();
		clearmsg(msg);               
		return;
	case cmd_analyser_symbols: {
		Address *current_address;
		if (!getCurrentAddress(&current_address)) return;
		showSymbols(current_address);
		delete current_address;
		clearmsg(msg);
		return;
	}
	case cmd_edit_mode:
	case cmd_view_mode:
		if (analy) {
			if (edit()) {
				analy->setDisplayMode(ANALY_EDIT_BYTES, 0);
			} else {
				analy->setDisplayMode(0, ANALY_EDIT_BYTES);
			}
		}
		analy_sub->output->invalidateCache();
		break;
	case msg_file_changed: {
		analy_sub->output->invalidateCache();
		break;
	}
	case cmd_analyser_call_chain: {
		if (!analy) break;
		Address *addr;
		if (getCurrentAddress(&addr)) {
			showCallChain(addr);
			delete addr;
		}
		return;
	}
	/* FIXME: srt-experimental */
/*	case cmd_analyser_srt: {
		Address *current_addr;
		if (getCurrentAddress(&current_addr)) {
			test_srt(analy, current_addr);
			delete current_addr;
		}
		clearmsg(msg);
		return;
	}*/
	case msg_get_analyser: {
		msg->msg = msg_retval;
		msg->data1.ptr = analy;
		return;
	}
	case msg_set_analyser: {
		Analyser *a = (Analyser*)msg->data1.ptr;
		if (analy) {
			analy->done();
			delete analy;
		}
		a->setDisplayMode(0, ANALY_EDIT_BYTES);
		setAnalyser(a);
		analy_sub->output->invalidateCache();
		clearmsg(msg);
		one_load_hack = true;
		return;
	}
	case msg_postinit:
		if (analy && !one_load_hack) {
			analy->beginAnalysis();
			analy_sub->output->invalidateCache();
		}
		return;
	}

	ht_uformat_viewer::handlemsg(msg);
	
	switch (msg->msg) {
	case msg_draw:
		if (infoline) {
			FileOfs a;
			Address *addr;
			if (!getCurrentAddress(&addr)) {
				addr = new InvalidAddress();
			}
			infoline->update(addr, (get_current_real_offset(&a)) ? a : INVALID_FILE_OFS);
			delete addr;
		}
		break;
	}
}

bool ht_aviewer::idle()
{
	if (!analy) return false;
	last_active = analy->active;
	if (!pause) analy->continueAnalysis();
	if (last_active && !analy->active) {
		String name;
		LOG("%y: analyser finished after %d ops.", &analy->getName(name), analy->ops_parsed);
		dirtyview();
		app->sendmsg(msg_draw, 0);
	}
	idle_count++;
/*	if (idle_count % 565 == 0) {
		analy_sub->output->invalidateCache();
		dirtyview();
		app->sendmsg(msg_draw, 0);
	}*/
	if (analy->active) {
		if (idle_count%53==0) {
			analy_sub->output->invalidateCache();
			dirtyview();
			app->sendmsg(msg_draw, 0);
		}
	}
	return last_active && !pause;
}

bool ht_aviewer::offset_to_pos(FileOfs ofs, viewer_pos *p)
{
	if (!analy) return false;
	Address *a = analy->fileofsToAddress(ofs);
	bool res = convertAddressToViewerPos(a, p);
	delete a;
	return res;
}

bool ht_aviewer::ref_sel(LINE_ID *id)
{
	if (!id->id1 && !id->id2 && !id->id3 && !id->id4) return 0;
	switch (id->id4) {
	case 0:
		if (analy) {
			Address *a = analy->createAddress();
			a->getFromUInt64((uint64(id->id1) << 32) + id->id2);
			bool res = gotoAddress(a, this);
			delete a;
			return res;
		} else {
			return false;
		}
	case 1:
		if (analy) {
			Address *a = analy->createAddress();
			a->getFromUInt64((uint64(id->id1) << 32) + id->id2);
			showXRefs(a);
			delete a;
		}
		return false;
	}
	return false;
}

void ht_aviewer::reloadpalette()
{
	ht_uformat_viewer::reloadpalette();
	if (analy_sub) analy_sub->output->changeConfig();
}

void ht_aviewer::searchForXRefs(Address *Addr)
{
	// FIXME: viewer_pos && FIXNEW
	char str[100];
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT;
	ht_snprintf(str, sizeof str, "%y", Addr);
	ht_regex_search_request *q = new ht_regex_search_request(SC_VISUAL, SF_REGEX_CASEINSENSITIVE, str);
	viewer_pos vp_start, vp_end;
	convertAddressToViewerPos(analy_sub->lowestaddress, &vp_start);
	convertAddressToViewerPos(analy_sub->highestaddress, &vp_end);
	int oldmode = analy->getDisplayMode();
	analy->setDisplayMode(0, -1);
	analy_sub->output->invalidateCache();
	ht_visual_search_result *r = (ht_visual_search_result *)vsearch(q, vp_start, vp_end);
	while (r) {
		Address *to;
		convertViewerPosToAddress(r->pos, &to);
		analy->addXRef(Addr, to, xrefoffset);
		viewer_pos na;
		next_logical_pos(r->pos, &na);
		delete to;
		delete r;
		r = (ht_visual_search_result *)vsearch(q, na, vp_end);
	}
	analy->setDisplayMode(oldmode, -1);
	analy->makeDirty();
	analy_sub->output->invalidateCache();
	delete q;
}

void ht_aviewer::showCallChain(Address *Addr)
{
	Location *a = analy->getFunctionByAddress(Addr);
	if (!a) return;
	Bounds b;
	b.w = 60;
	b.h = 16;
	center_bounds(&b);
	char str[256];
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
	ht_snprintf(str, sizeof str, "call chain of address %y", Addr);
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE);
	b.assign(1, 0, 56, 10);
	ht_statictext *text = new ht_statictext();
	if (a->label) {
		ht_snprintf(str, sizeof str, "function %s %s", a->label->name, "is referenced by ..");
	} else {
		ht_snprintf(str, sizeof str, "address %y %s", a->addr, "is referenced by ..");
	}
	text->init(&b, str, align_left);
	dialog->insert(text);
	b.assign(1, 1, 56, 10);
	CallChain *cc;
	NEW_OBJECT(cc, CallChain, &b, analy, Addr, NULL);
	cc->adjust(cc->get_root(), true);
	dialog->insert(cc);
	b.assign(15, 12, 9, 2);
	ht_button *bt;
	NEW_OBJECT(bt, ht_button, &b, "O~k", button_ok);
	dialog->insert(bt);
	b.assign(35, 12, 9, 2);
	NEW_OBJECT(bt, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(bt);
	int r = dialog->run(false);
	if (r == button_ok) {
		ht_treeview_data tvd;
		ViewDataBuf vdb(dialog, &tvd, sizeof tvd);
		gotoAddress(((CallChainNode*)tvd.selected)->xa, this);
	}
	dialog->done();
	delete dialog;
}

void ht_aviewer::showComments(Address *Addr)
{
	if (!analy) return;
	if (!canCreateAddress(Addr, true)) {
		return;
	}
	CommentList *comment = analy->getComments(Addr);

	// prepare mem_file
	MemoryFile *mem_file = new MemoryFile();
	if (comment) {
		int c1 = comment->count();
		for (int i=0; i < c1; i++) {
			const char *c = comment->getName(i);
			int len = strlen(c);
			if (len) mem_file->write(c, len);
			if (i+1<c1) mem_file->write((void*)"\n", 1);
		}
	}
	
/*     ht_c_syntax_lexer *lexer = new ht_c_syntax_lexer();
	lexer->init();*/
	
	// prepare textfile
	ht_ltextfile text_file(mem_file, true, NULL);

	// create dialog
	Bounds b;
	b.w = 60;
	b.h = 16;
	center_bounds(&b);
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, "edit comments", FS_KILLER | FS_TITLE | FS_MOVE);

	b.assign(1, 1, 55, 10);
	ht_text_editor *text_editor = new ht_text_editor();
	text_editor->init(&b, false, &text_file, NULL, TEXTEDITOPT_UNDO);
	dialog->insert(text_editor);

	/* FIXME: scrollbar
	BOUNDS_ASSIGN(b, 56, 1, 1, 10);
	*/
	
	ht_button *b1;
	b.assign(18, 12, 9, 2);
	NEW_OBJECT(b1, ht_button, &b, "O~k", button_ok);
	dialog->insert(b1);
	b.assign(32, 12, 9, 2);
	NEW_OBJECT(b1, ht_button, &b, "~Cancel", button_cancel);
	dialog->insert(b1);

	if (dialog->run(false) == button_ok) {
		Location *a = analy->getLocationByAddress(Addr);
		if (a) analy->freeComments(a);
		uint c = text_file.linecount();
		char buf[1024];
		bool empty=false;
		if (c == 1) {
			uint l = 0;
			text_file.getline(0, 0, buf, 1024, &l, NULL);
			empty=(l==0);
		}
		if (!empty) {
			for (uint i=0; i<c; i++) {
				uint l;
				if (text_file.getline(i, 0, buf, 1024, &l, NULL)) {
					buf[l]=0;
					analy->addComment(Addr, 0, buf);
				}
			}
		}
		analy->makeDirty();
		analy_sub->output->invalidateCache();
	}

	dialog->done();
	delete dialog;
}

void ht_aviewer::showInfo(Address *Addr)
{
	Bounds c, b;
	app->getbounds(&c);
	b.w=c.w*5/6;
	b.h=c.h*5/6;
	center_bounds(&b);
	char str[100];
	strcpy(str, "Analyser information");
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE);
	b.assign(1, 0, b.w-4, 10);
	AnalyserInformation *text = new AnalyserInformation();
	text->init(&b, this);
	dialog->insert(text);
	
	dialog->run(false);
	
	dialog->done();
	delete dialog;
}

void ht_aviewer::showSymbols(Address *addr)
{
	if (!analy) return;
	
	Location *loc = analy->getPreviousSymbolByAddress(addr);
				
	Bounds b;
	b.w = 60;
	b.h = 15;
	center_bounds(&b);
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, "symbols", FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
	/* pull down */
	b.assign(30, 0, 20, 1);
/*				ht_listpopup *lp = new ht_listpopup();
				lp->init(&b);
				lp->insertstring("show all");
				lp->insertstring("only functions");
				lp->insertstring("only labels");
				lp->insertstring("only offsets (code)");
				lp->insertstring("only data");
				dialog->insert(lp);*/
	/* text */
	b.assign(1, 0, 56, 1);
	ht_listbox_title *text = new ht_listbox_title();
	text->init(&b);
	text->setText(3, "Address", "Type", "Name");
	/* list */
	b.assign(1, 1, 56, 12);
	SymbolBox *sym = new SymbolBox();
	sym->init(&b, analy);
	if (loc && loc->label) {
		sym->gotoItemByEntry(sym->quickfind(loc->label->name));
	}
	dialog->insert(sym);
	dialog->insert(text);
	sym->attachTitle(text);
	register_idle_object(sym);
	int r = dialog->run(false);
	unregister_idle_object(sym);
	if (r == button_ok) {
		// goto selected symbol
		ht_listbox_data d;
		ViewDataBuf vdb(sym, &d, sizeof d);
		if (d.data->cursor_ptr) gotoAddress(((Symbol *)d.data->cursor_ptr)->location->addr, this);
	}
	dialog->done();
	delete dialog;
}

void ht_aviewer::showXRefs(Address *Addr)
{
	if (!analy->getXRefs(Addr)) {
		if (confirmbox("No xrefs for address %y!\nSearch for xrefs?", Addr) == button_yes) {
			searchForXRefs(Addr);
		}
	}

	Bounds c, b;
	app->getbounds(&c);
	b.w = c.w*5/6;
	b.h = c.h*5/6;
	center_bounds(&b);
	int result;
	do {
		Container *x_tree = analy->getXRefs(Addr);
		uint bw = b.w;
		uint bh = b.h;
		char str[256];
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
		ht_snprintf(str, sizeof str, "xrefs of address %y", Addr);
		ht_dialog *dialog = new ht_dialog();
		dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
		b.assign(1, 0, bw-4, 1);
		ht_listbox_title *text = new ht_listbox_title();
		text->init(&b);
		text->setText(3, "xref to", "type", "from function");
		b.y = 1;          
		b.h = bh-6;
		ht_text_listbox *list;
		NEW_OBJECT(list, ht_text_listbox, &b, 3, 2);
		b.assign(2, bh-4, 26, 2);
		ht_button *search_for_xrefs;
		NEW_OBJECT(search_for_xrefs, ht_button, &b, "~Search for more XRefs", 666);
		search_for_xrefs->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		b.assign(29, bh-4, 11, 2);
		ht_button *delete_xref;
		NEW_OBJECT(delete_xref, ht_button, &b, "~Delete", 667);
		delete_xref->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		// FIXME: disable button when possible
		b.assign(41, bh-4, 10, 2);
		ht_button *new_xref;
		NEW_OBJECT(new_xref, ht_button, &b, "~Add", 668);
		new_xref->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
		char str2[1024];
		int xcount=0;
		if (x_tree) foreach(AddrXRef, x, *x_tree, {
			xcount++;
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_WHITESPACE;
			ht_snprintf(str, sizeof str, "%y", x->addr);
			Location *a = analy->getFunctionByAddress(x->addr);
			const char *func = analy->getSymbolNameByLocation(a);
			if (func) {
				int d=0;
				x->addr->difference(d, a->addr);
				char sign = '+';
				if (d < 0) {
					d = -d;
					sign = '-';
				}
				ht_snprintf(str2, sizeof str2, "%s%c%x", func, sign, d);
			} else {
				strcpy(str2, "?");
			}
			list->insert_str_extra(xcount, x->addr, str, xref_type(x->type), str2);
		});
		list->attachTitle(text);
		list->update();
		dialog->insert(text);
		dialog->insert(list);
		dialog->insert(search_for_xrefs);
		dialog->insert(delete_xref);
		dialog->insert(new_xref);
		result = dialog->run(false);
		ht_listbox_data data;
		ViewDataBuf vdb(list, &data, sizeof data);
		switch (result) {
		case 666:
			searchForXRefs(Addr);
			break;
		case 667:
			if (xcount) {
				analy->deleteXRef(Addr, (Address*)list->getExtra(data.data->cursor_ptr));
				analy->makeDirty();
				analy_sub->output->invalidateCache();
			}
			break;
		case 668: {
			char result[256];
			ht_snprintf(str, sizeof str, "add xref from %y", Addr);
			result[0] = 0;
			while (inputbox(str, "to ~address: ", result, 255, HISTATOM_GOTO)) {
				viewer_pos res_pos;
				if (!string_to_pos(result, &res_pos)) {
					errorbox(globalerror);
					continue;
				}
				Address *a;
				if (!convertViewerPosToAddress(res_pos, &a)) {
					errorbox("invalid address");
					delete a;
					continue;
				}
				if (!analy->addXRef(Addr, a, xrefoffset)) {
					// FIXME: some error msg
				}
				delete a;
				analy->makeDirty();
				analy_sub->output->invalidateCache();
				break;
			}
			break;
		}
		case button_ok:
			if (xcount) gotoAddress((Address*)list->getExtra(data.data->cursor_ptr), this);
			break;
		}
		dialog->getbounds(&b);
		dialog->done();
		delete dialog;
	} while (result >= 666 && result <= 668);
}

bool ht_aviewer::func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	eval_func myfuncs[] = {
		{"addressOf", (void*)&aviewer_func_address_of, {SCALAR_STR}, "return address of symbol"},
		{"fileofs", (void*)&aviewer_func_fileofs, {SCALAR_INT}, "convert file offset to address"},
//		{"addr", (void*)&aviewer_func_addr, {SCALAR_STR}}, "",
		{NULL}
	};
	return std_eval_func_handler(result, name, params, myfuncs);
}

bool ht_aviewer::symbol_handler(eval_scalar *result, char *name)
{
	uint64 v;
	viewer_pos vp;
	Address *w;
	if (*name == '@') {
		name++;
		if (parseIntStr(name, v, 10)) {
			if (*name) return false;
			if (!offset_to_pos(v, &vp)) {
				set_eval_error("invalid offset: %08qx", v);
				return false;
			}
			convertViewerPosToAddress(vp, &w);
			uint64 b = 0;
			w->putIntoUInt64(b);
			delete w;
			scalar_create_int_q(result, b);
			return true;
		}
		// invalid number after @
	} else {
		if (strcmp(name, "$")==0) {
			if (getCurrentAddress(&w)) {
				uint64 b = 0;
				w->putIntoUInt64(b);
				scalar_create_int_q(result, b);
				delete w;
				return true;
			} else {
				return false;
			}
		}
		Symbol *l = analy->getSymbolByName(name);
		if (l) {
			w=l->location->addr;
			uint64 b;
			w->putIntoUInt64(b);
			scalar_create_int_q(result, b);
			return true;
		}
	}
	return ht_uformat_viewer::symbol_handler(result, name);
}
	
bool ht_aviewer::qword_to_pos(uint64 q, viewer_pos *pos)
{
	if (!analy) return false;
	Address *a=analy->createAddress();
	a->getFromUInt64(q);
	if (analy->validAddress(a, scvalid)) {
		bool res = convertAddressToViewerPos(a, pos);
		delete a;
		return res;
	} else {
		global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;
		ht_snprintf(globalerror, GLOBAL_ERROR_SIZE, "address %y is invalid", a);
	}
	delete a;
	return false;
}

/*
 *	CLASS ht_analy_sub
 */

void ht_analy_sub::init(File *file, ht_aviewer *A, Analyser *analyser, Address *Lowestaddress, Address *Highestaddress)
{
	ht_sub::init(file);
	aviewer = A;
	analy = analyser;
	output = new AnalyserHTOutput();
	((AnalyserHTOutput*)output)->init(analy);
	lowestaddress = Lowestaddress->clone();
	highestaddress = Highestaddress->clone();
}

void ht_analy_sub::done()
{
	delete lowestaddress;
	delete highestaddress;
	output->done();
	delete output;
	ht_sub::done();
}

bool ht_analy_sub::closest_line_id(LINE_ID *line_id)
{
	if (!prev_line_id(line_id, 1)) {
		if (!next_line_id(line_id, 1)) {
			first_line_id(line_id);
		}
	}
	return true;
}

bool ht_analy_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	viewer_pos a;
	if (!uformat_viewer->offset_to_pos(offset, &a)) return false;
	*line_id = a.u.line_id;
	return true;
}

void	ht_analy_sub::first_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	uint64 u;
	lowestaddress->putIntoUInt64(u);
	line_id->id1 = u >> 32;
	line_id->id2 = u;
}

bool ht_analy_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	if (!analy) return false;
	Address *a = analy->createAddress();
	a->getFromUInt64((uint64(line_id.id1) << 32) + line_id.id2);
	bool res = output->getLineString(line, maxlen, a, (int)line_id.id3);
	delete a;
	return res;
}

void	ht_analy_sub::last_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	uint64 u;
	highestaddress->putIntoUInt64(u);
	line_id->id1 = u >> 32;
	line_id->id2 = u;
}

int ht_analy_sub::next_line_id(LINE_ID *line_id, int n)
{
	if (!analy) return false;
	Address *a = analy->createAddress();
	a->getFromUInt64((uint64(line_id->id1) << 32) + line_id->id2);
	int line = line_id->id3;
	int res = output->nextLine(a, line, n, highestaddress);
	if (res) {
		line_id->id3 = line;
		uint64 u;
		a->putIntoUInt64(u);
		line_id->id1 = u >> 32;
		line_id->id2 = u;
	}
	delete a;
	return res;
}

int ht_analy_sub::prev_line_id(LINE_ID *line_id, int n)
{
	if (!analy) return false;
	Address *a = analy->createAddress();
	a->getFromUInt64((uint64(line_id->id1) << 32) + line_id->id2);
	int line = line_id->id3;
	int res = output->prevLine(a, line, n, lowestaddress);
	if (res) {
		line_id->id3 = line;
		uint64 u;
		a->putIntoUInt64(u);
		line_id->id1 = u >> 32;
		line_id->id2 = u;
	}
	delete a;
	return res;
}

ht_search_result *ht_analy_sub::search(ht_search_request *search, FileOfs start, FileOfs end)
{
	// FIXME: viewer pos     
	Address *st = NULL;
	ht_search_result *r = NULL;
	while (!r) {
		st = (Address *)analy->initialized->findNext(st);
		if (!st) break;
		area_s *s = analy->initialized->getArea(st);
		if (!s) break;
		st = (Address *)s->end;
		FileOfs fstart, fend;
		FileOfs fsize;
		viewer_pos vp_start, vp_end;
		aviewer->convertAddressToViewerPos((Address *)s->start, &vp_start);
		if (!aviewer->pos_to_offset(vp_start, &fstart)) assert(0);
		Address *send = (Address*)s->end->clone();
		send->add(-1);
		aviewer->convertAddressToViewerPos(send, &vp_end);
		delete send;
		if (!aviewer->pos_to_offset(vp_end, &fend)) assert(0);
		fsize = fend - fstart;
		if (search->search_class == SC_PHYSICAL && search->type == ST_EXPR) {
			r = linear_expr_search(search, start, end, this, uformat_viewer, fstart, fsize);
		} else if (search->search_class == SC_PHYSICAL && search->type == ST_FXBIN) {
			r = linear_bin_search(search, start, end, file, fstart, fsize);
		}
	}
	return r;
}

void	ht_analy_sub::setAnalyser(Analyser *Analy)
{
	analy = Analy;
	output->analy = Analy;
	output->invalidateCache();
}
