/* 
 *	HT Editor
 *	analy.cc
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "analy.h"
#include "analy_names.h"
#include "analy_register.h"
#include "codeanaly.h"
#include "dataanaly.h"
#include "global.h"
#include "htatom.h"
#include "htctrl.h"
#include "htdebug.h"
#include "htstring.h"
#include "httag.h"
#include "language.h"
#include "tools.h"

//#undef DPRINTF
//#define DPRINTF(msg...) printf(##msg)


/*
 *
 */
 
addr_xref::addr_xref() {}

addr_xref::addr_xref(xref_type_t Type)
{
	type = Type;
}

int addr_xref::load(ht_object_stream *f)
{
	type = (xref_type_t) f->get_int_hex(1, "type");
	return f->get_error();
}

OBJECT_ID addr_xref::object_id()
{
	return ATOM_ADDR_XREF;
}

void addr_xref::store(ht_object_stream *f)
{
	f->put_int_hex((UINT)type, 1, "type");
}

ht_addr::ht_addr() {}

ht_addr::ht_addr(ADDR Addr)
{
	addr = Addr;
}

int ht_addr::load(ht_object_stream *f)
{
     GET_INT_HEX(f, addr);
	return f->get_error();
}

OBJECT_ID ht_addr::object_id()
{
	return ATOM_HT_ADDR;
}

void ht_addr::store(ht_object_stream *f)
{
     PUT_INT_HEX(f, addr);
}

/*
 *
 */
addrqueueitem::addrqueueitem() {}

addrqueueitem::addrqueueitem(ADDR Addr, ADDR Func)
{
	addr = Addr;
	func = Func;
}

OBJECT_ID	addrqueueitem::object_id()
{
	return ATOM_ADDRQUEUEMEMBER;
}

int	addrqueueitem::load(ht_object_stream *f)
{
	GET_INT_HEX(f, addr);
	GET_INT_HEX(f, func);
	return f->get_error();
}

void	addrqueueitem::store(ht_object_stream *f)
{
	PUT_INT_HEX(f, addr);
	PUT_INT_HEX(f, func);
}

/*
 *
 */
void comment_list::init()
{
	ht_clist::init();
}

void comment_list::append_pre_comment(char *s)
{
	append(new ht_data_string(s));
}

void comment_list::append_post_comment(char *s)
{
	append(new ht_data_string(s));
}

void comment_list::append_pre_comment(int special)
{
	append(new ht_data_uint(special));
}

void comment_list::append_post_comment(int special)
{
	append(new ht_data_uint(special+0x10000000));
}

char *comment_list::get_name(UINT i)
{
	ht_data *d = get(i);
	return d ? ((d->object_id()==ATOM_HT_DATA_UINT) ? comment_lookup(((ht_data_uint*)d)->value): ((ht_data_string*)d)->value) : NULL;
}

/*
 *
 */
void	analyser::init()
{
	active = false;
	dirty = false;
	addr_queue = new ht_queue();
	addr_queue->init();
	addrs = NULL;
	labels = NULL;
	label_count = addr_count = 0;
	set_addr_tree_optimize_threshold(1000);
	set_label_tree_optimize_threshold(1000);
	cur_addr_ops = cur_label_ops = 1;
	cur_func = NULL;
	ops_parsed = 0;
	next_explored = first_explored = last_explored = INVALID_ADDR;
	explored = new area();
	explored->init();
	initialized = new area();
	initialized->init();
	init_code_analyser();
	init_data_analyser();
	disasm = NULL;
	analy_disasm = NULL;
	maxopcodelength = 1;
	init_unasm();
	mode = ANALY_TRANSLATE_SYMBOLS;
}

static void loadaddrs(ht_object_stream *st, taddr *&addr, int level, int &left)
{
	if (left<=0) {
		addr = NULL;
		return;
	}
	if (st->get_error()) {
		addr = NULL;
		return;
	}

	addr = (taddr *) smalloc0(sizeof(taddr));

	bool sdf=!((level<= 1) || (left == 1));
	if (sdf) loadaddrs(st, addr->left, level / 2, left);
	if (st->get_error()) {
		free(addr);
		addr = NULL;
		return;
	}
/*	printf("%d, %d\n", level, left);
	if (left==18286) {
		int asdf=234;
	}*/
	
	addr->addr = st->get_int_hex(sizeof(ADDR), "addr");
	// xrefs
     
     st->get_object((object *)addr->xrefs, "xrefs");
/*XFIX:
	txref **xref = &addr->xreflist;
	int count = st->get_int_dec(4, "count_of_xrefs");
	if (st->get_error()) {
		free(addr);
		addr = NULL;
		return;
	}
	while (count--) {
		if (st->get_error()) {
			*xref = NULL;
			addr = NULL;
			return;
		}
		*xref = (txref *)smalloc(sizeof(txref));
		(*xref)->addr = st->get_int_hex(sizeof(ADDR), "xref_addr");
		(*xref)->type = (txreftype)st->get_int_hex(sizeof(txreftype), "xref_type");
		xref = &(*xref)->next;
	}
	*xref = NULL;
*/
	// comments
	st->get_object((object *)addr->comments, "comments");
	if (st->get_error()) {
		free(addr);
		addr = NULL;
		return;
	}

	analyser_get_addrtype(st, &addr->type);


	// must be resolved later
	addr->thisfunc = (taddr *)st->get_int_hex(4, "func");

	addr->flags = st->get_int_hex(4, "flags");
	
	left--;

	if (st->get_error()) {
		free(addr);
		addr = NULL;
		return;
	}
	if (sdf) loadaddrs(st, addr->right, level / 2 -1, left);
}

static void loadlabels(analyser *analy, ht_object_stream *st, tlabel *&label, int level, int &left)
{
	if (left<=0) {
		label = NULL;
		return;
	}
	if (st->get_error()) {
		free(label);
		label = NULL;
		return;
	}

	label = (tlabel *) smalloc0(sizeof(tlabel));

	bool sdf=!((level<= 1) || (left == 1));
	if (sdf) loadlabels(analy, st, label->left, level / 2, left);

	ADDR a = st->get_int_hex(sizeof a, "addr");
	(label->addr = analy->new_addr(a))->label = label;
//     label->addr->label = label;

//	fstrget(st, label->name);
//	          st->put_string(label->name, "name");
	label->name = st->get_string("name");

	label->type = (labeltype)st->get_int_hex(1, "type");

	left--;

	if (st->get_error()) {
		free(label);
		label = NULL;
		return;
	}
	if (sdf) loadlabels(analy, st, label->right, level / 2 -1, left);
}

static void resolveaddrs(analyser *a, taddr *addr)
{
	if (addr) {
		resolveaddrs(a, addr->left);
		if ((ADDR)addr->thisfunc != INVALID_ADDR) {
			addr->thisfunc = a->find_addr((ADDR)addr->thisfunc);
		} else {
			addr->thisfunc = NULL;
		}
		resolveaddrs(a, addr->right);
	}
}

/*
 *
 */
int analyser::load(ht_object_stream *st)
{
	GET_OBJECT(st, addr_queue);
	GET_INT_DEC(st, ops_parsed);
	GET_BOOL(st, active);
	if (active) {
		some_analyser_active++;
	}

	GET_INT_HEX(st, next_explored);
	GET_INT_HEX(st, first_explored);
	GET_INT_HEX(st, last_explored);

	GET_INT_HEX(st, mode);

	GET_OBJECT(st, explored);
	GET_OBJECT(st, initialized);

	GET_INT_DEC(st, addr_count);
	loadaddrs(st, addrs, addr_count, addr_count);

	if (st->get_error()) return st->get_error();

	resolveaddrs(this, addrs);
	
	GET_INT_DEC(st, label_count);
	loadlabels(this, st, labels, label_count, label_count);

	if (st->get_error()) return st->get_error();

	GET_OBJECT(st, analy_disasm);
	GET_OBJECT(st, disasm);     
	if (analy_disasm) {
		analy_disasm->analy = this;
		analy_disasm->disasm = disasm;
	}
	GET_OBJECT(st, code);
	GET_OBJECT(st, data);
	if (data) {
		data->analy = this;
	}
	GET_INT_DEC(st, addrthreshold);
	GET_INT_DEC(st, labelthreshold);

	GET_INT_DEC(st, maxopcodelength);

	if (st->get_error()) return st->get_error();

	ADDR curfuncaddr = st->get_int_hex(sizeof curfuncaddr, "cur_func");
	if (curfuncaddr==INVALID_ADDR) {
		cur_func = NULL;
	} else {
		cur_func = new_addr(curfuncaddr);
	}

	cur_addr_ops = 0;
	cur_label_ops = 0;
	dirty = false;
	return st->get_error();
}

/*
 *
 */
void	analyser::done()
{
	set_active(false);
	free_addrs(addrs);
	free_labels(labels);
	if (addr_queue) {
		addr_queue->destroy();
		delete addr_queue;
	}
	if (explored) {
		explored->done();
		delete explored;
	}
	if (initialized) {
		initialized->done();
		delete initialized;
	}
	if (code) {
		code->done();
		delete code;
	}
	if (data) {
		data->done();
		delete data;
	}
	if (disasm) {
		disasm->done();
		delete disasm;
	}
	if (analy_disasm) {
		analy_disasm->done();
		delete analy_disasm;
	}
}

/*
 *	addaddrlabel will never overwrite an existing label (like addlabel)
 */
bool analyser::add_addr_label(ADDR Addr, char *Prefix, labeltype type, taddr *infunc=NULL)
{
	if (!valid_addr(Addr, scvalid)) return false;


	char *prefix = label_prefix(Prefix);

	char	label[1024];

	sprintf(label, "%s"HEX8FORMAT, prefix, Addr);

	if (add_label(Addr, label, type, infunc)) {
		return true;
	} else {
		return false;
	}
}

/*
 *
 */
void	analyser::add_comment(ADDR Addr, int line, char *c)
{
	// line 0 meens append (at the moment assume append every time ;-))

//	if (!validaddr(Addr, scvalid)) return;

	taddr *a = new_addr(Addr);

	comment_list *com = a->comments;

	if (!com) {
		com = new comment_list();
		com->init();
		a->comments = com;
	}
	com->append_pre_comment(c);

	DPRINTF("#(%08lx) comment `%s'\n", Addr, c);
}

/*
 * addlabel: create label if there isnt one
 *           fail if label exist on another address
 *
 */
bool analyser::add_label(ADDR Addr, char *label, labeltype type, taddr *infunc=NULL)
{
	if (!valid_addr(Addr, scvalid)) return false;

	taddr *a = new_addr(Addr);

	if (!a->label) {

		tlabel *l = new_label(label, a, type, infunc);

		if (l->addr->addr != Addr) {
			// this label already exists at a different address
			return false;
		}

		a->label = l;
		if (a->type.type == dt_unknown || a->type.type == dt_unknown_data) {
	     	if (type == label_func && !valid_code_addr(Addr)) {
	          	type = label_unknown;
	          }
			switch (type) {
				case label_unknown:
					break;
				case label_func:
					data->set_code_addr_type(a, dst_function);
					break;
				case label_loc:
					data->set_code_addr_type(a, dst_location);
					break;
				case label_data:
					data->set_addr_type(a, dt_unknown_data, 0, 0);
					break;
			}
		}
		return true;

	} else {

		// adress already has a label
		return false;
	}
}

/*
 *
 */
void	analyser::add_xref(ADDR from, ADDR to, xref_type_t action)
{
	if ((!valid_addr(from, scvalid)) || (!valid_addr(to, scvalid))) return;

	taddr	*a =	new_addr(from);
	ht_tree	*x = a->xrefs;

	if (x) {
          addr_xref *xref;
          ht_addr tmp_to(to);
		if ((xref = (addr_xref*)x->get(&tmp_to))) {
			// update xref
			xref->type = action;
			DPRINTF("xref %08lx->%08lx updated\n", from, to);
			return;
		}
	} else {
     	x = new ht_dtree();
          ((ht_dtree *)x)->init(compare_keys_uint);
	}
     x->insert(new ht_addr(to), new addr_xref(action));
     a->xrefs = x;
     
	DPRINTF("xref %08lx->%08lx\n", from, to);
}

/*
 *
 */
void	analyser::assign_comment(ADDR Addr, int line, char *c)
{
	/* not really implemented */
	add_comment(Addr, line, c);
}

/*
 *
 */
bool analyser::assign_label(ADDR Addr, char *label, labeltype type, taddr *infunc=NULL)
{
	if (!valid_addr(Addr, scvalid)) return false;

	taddr *a = new_addr(Addr);

	tlabel *l = new_label(label, a, type, infunc);
	if (l->addr->addr != Addr) {
		// label already exists at a different address
		return false;
	}

	if (l->addr->type.type == dt_unknown) {
     	if (type == label_func && !valid_code_addr(Addr)) {
          	type = label_unknown;
          }
		switch (type) {
			case label_unknown:
				break;
			case label_func:
				data->set_code_addr_type(a, dst_function);
				break;
			case label_loc:
				data->set_code_addr_type(a, dst_location);
				break;
			case label_data:
				data->set_addr_type(a, dt_unknown_data, 0, 0);
				break;
		}
	}
	
	if (a->label) {
		// overwrite
		if (a->label != l) {
			// label has to be renamed
			disable_label(a->label);
			a->label = l;
		}
	} else {
		a->label = l;
	}
	return true;
}

/*
 *
 */
void	analyser::begin_analysis()
{
	if (query_config(Q_DO_ANALYSIS)) {
		DPRINTF("################################\nAnalysis started at %08lx:\n", Addr);
		if (analy_disasm && disasm) {
			ops_parsed = 0;
			if (goto_addr(INVALID_ADDR, INVALID_ADDR)) set_active(true);
		} else {
			DPRINTF("Analysis can't be started. No disassembler available.");
		}
	}
}

/*
 *
 */
bool	analyser::continue_analysis()
{
	byte			buf[16];
	OPCODE		*instr;
	int			len;
	tbranchtype	branch;

	if (!active) return	false;
	do {
		if (!savety_change_addr(addr)) {
			finish();
			return false;
		}

		int bz = bufptr(addr, buf, sizeof(buf));

		instr = disasm->decode(buf, MIN(bz, maxopcodelength), map_addr(addr));
		DPRINTF("opcode @%08lx [func: %08lx]: %s\n", addr, (cur_func) ? cur_func->addr : INVALID_ADDR, disasm->str(instr, 0));

		ops_parsed++;
		num_ops_parsed++;

		len = disasm->getsize(instr);
		last_explored += len;

		if (disasm->valid_insn(instr)) {
			branch = analy_disasm->is_branch(instr);
			if (branch != brnobranch) do_branch(branch, instr, len); else {
				analy_disasm->examine_opcode(instr);
				addr	+= len;
			}
		} else {
			DPRINTF("invalid opcode @%08lx\n", addr);
//             log("invalid opcode at address %08lx\n", addr);
			new_addr(addr+len)->flags |= AF_FUNCTION_END;
			goto_addr(INVALID_ADDR, INVALID_ADDR);
		}
	} while ((ops_parsed % MAX_OPS_PER_CONTINUE) !=0);
	notify_progress(addr);
	return true;
}

/*
 *
 */
void analyser::continue_analysis_at(ADDR Addr)
{
	if (!valid_code_addr(Addr)) return;
	if (query_config(Q_DO_ANALYSIS)) {
		DPRINTF("continueing analysis at %08lx\n", Addr);
		if (active) {
			push_addr(Addr, Addr);
		} else {
			if (disasm) {
				push_addr(Addr, Addr);
				analyser::begin_analysis();
			} else {
				DPRINTF("couldn't start analysis: no disasm available\n");
			}
		}
	}
}

/*
 *
 */
void	analyser::data_access(ADDR Addr, taccess access)
{
	if (!valid_addr(Addr, scvalid)) {
		char	msg[100];
		sprintf(msg, "access of invalid addr %08x at addr %08x", Addr, addr);
		log(msg);
		return;
	}

	DPRINTF("dataaccess of %08x\n", Addr);

	// string test
	byte buffer[1024];

	if (valid_addr(Addr, scinitialized)) {
		UINT bz = bufptr(Addr, buffer, sizeof(buffer));
		if (bz > 2) {
			analy_string *str = string_test(buffer, bz);
			if (str) {
				char string1[256], string2[31];
				str->render_string(string2, 30);
				sprintf(string1, "%s_%s_", str->name(), string2);
				make_valid_name(string2, string1);
				if (add_addr_label(Addr, string2, label_data)) {
					add_comment(Addr, 0, "");
				}
				data->set_array_addr_type(Addr, dst_string, str->length());
				str->done();
				delete str;
				return;
			}
		}
	}

	if (valid_code_addr(Addr) && access.type == acoffset) {
		if (add_addr_label(Addr, LPRFX_OFS, label_func)) {
			add_comment(Addr, 0, "");
		}
		taddr *a = find_addr(Addr);
		assert(a);
		// test if Addr points to code
		if ((a->type.type == dt_unknown) || (a->type.type == dt_code)) {
			// test if Addr points to valid code (test not yet complete)
			byte buf[16];
			int bz = bufptr(Addr, buf, sizeof(buf));
			OPCODE *instr = disasm->decode(buf, MIN(bz, maxopcodelength), map_addr(Addr));
			if (disasm->valid_insn(instr)) {
				data->set_code_addr_type(Addr, dst_cunknown);
				push_addr(Addr, Addr);
			}
		}
	} else {
		if (access.type != acoffset) {
			switch (access.size) {
				case 1: data->set_int_addr_type(Addr, dst_ibyte, 1); break;
				case 2: data->set_int_addr_type(Addr, dst_iword, 2); break;
				case 4: data->set_int_addr_type(Addr, dst_idword, 4); break;
			}
		}
		if (valid_addr(Addr, scinitialized)) {
			add_addr_label(Addr, LPRFX_DTA, label_data);
		} else {
			add_addr_label(Addr, LPRFX_DTU, label_data);
		}
	}
}

/*
 *	disables address, frees misc
 */
void	analyser::delete_addr(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (a) {
		disable_label(a->label);
		a->label = NULL;
		a->flags |= AF_DELETED;
		addr_count--;
	}
}

/*
 *	disables label of an address and unassigns address' label
 */
void analyser::delete_label(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (a) {
		disable_label(a->label);
		a->label = NULL;
		label_count--;
	}
}

/*
 *	an disabled label will be overwritten as soon as possible,
 *   and never be returned nor saved.
 *	performed this way to preserve the labeltrees structure
 *
 */
void	analyser::disable_label(tlabel *label)
{
	if (label) {
		label->addr = NULL;
	}
}

/*
 *
 */
void	analyser::do_branch(tbranchtype branch, OPCODE *opcode, int len)
{
	ADDR	branch_addr = analy_disasm->branch_addr(opcode, branch, true);

	if (branch != brreturn) {
		if (!valid_code_addr(branch_addr)) {
			char	msg[100];
			sprintf(msg, "branch to invalid addr %08x from addr %08x", branch_addr, addr);
			log(msg);
		}
	}
/*     if (branch != brcall) {
		taddr *a = new_addr(addr+len);
		if (!a->comments) {
			add_comment(a->addr, 0, "");
		}
	}*/
	switch (branch) {
		case	brjump:
			new_addr(addr+len)->flags |= AF_FUNCTION_END;

			add_xref(branch_addr, addr, xrefjump);
			if (add_addr_label(branch_addr, LPRFX_LOC, label_loc, cur_func)) {
				add_comment(branch_addr, 0, "");
			}
			goto_addr(branch_addr, INVALID_ADDR);
			break;
		case	brreturn:
			new_addr(addr+len)->flags |= AF_FUNCTION_END;

			goto_addr(INVALID_ADDR, INVALID_ADDR);
			break;
		case	brcall: {
			add_xref(branch_addr, addr, xrefcall);
			bool special_func = false;
			char *lprfx = LPRFX_SUB;
			if (!explored->contains(branch_addr) && valid_code_addr(branch_addr)) {
				// should be in code_analy
				byte buf[16];
				int bz = bufptr(branch_addr, buf, sizeof(buf));
				OPCODE *instr = disasm->decode(buf, MIN(bz, maxopcodelength), map_addr(addr));
				tbranchtype bt = analy_disasm->is_branch(instr);
				
				if (bt == brreturn) {
					lprfx = LPRFX_STUB;
				} else if (bt == brjump) {
					char buf[1024], label[1024];
					ADDR wrap_addr = analy_disasm->branch_addr(instr, bt, false);
					if (valid_addr(wrap_addr, scvalid)) {
						tlabel *l = get_addr_label(wrap_addr);
						add_comment(branch_addr, 0, "");
						add_comment(branch_addr, 0, ";-----------------------");
						if (l && l->name) {
							sprintf(buf, "%s %s", ";  W R A P P E R for", l->name);
							sprintf(label, "%s%s_%x", label_prefix(LPRFX_WRAP), l->name, branch_addr);
						} else {
							sprintf(buf, "%s %s %x", ";  W R A P P E R for", "address", wrap_addr);
							sprintf(label, "%s%x_%x", label_prefix(LPRFX_WRAP), wrap_addr, branch_addr);
						}
						add_comment(branch_addr, 0, buf);
						add_comment(branch_addr, 0, ";-----------------------");
						add_label(branch_addr, label, label_func);
						special_func = true;
					}
				}
			}
			if (!special_func && add_addr_label(branch_addr, lprfx, label_func)) {
				add_comment(branch_addr, 0, "");
				add_comment(branch_addr, 0, ";-----------------------");
				add_comment(branch_addr, 0, ";  S U B R O U T I N E");
				add_comment(branch_addr, 0, ";-----------------------");
				if (branch_addr==cur_func->addr) {
					add_comment(branch_addr, 0, ";  (recursive)");
				}
			}
			push_addr(addr+len, cur_func->addr);
			goto_addr(branch_addr, branch_addr);
			break;
		}
		case	brjXX:
			add_xref(branch_addr, addr, xrefjump);
			if (add_addr_label(branch_addr, LPRFX_LOC, label_loc, cur_func)) {
				add_comment(branch_addr, 0, "");
			}
			push_addr(addr+len, cur_func->addr);
			goto_addr(branch_addr, INVALID_ADDR);
			break;
		default:{}
			// stupid but neccessary
	}
}

/*
 *
 */
void	analyser::engage_codeanalyser()
{
	if (query_config(Q_ENGAGE_CODE_ANALYSER)) {
		DPRINTF("starting code analyser.\n");
	}
}

static void analyserenum_addrs(taddr *addrs, ADDR at, taddr *&addr)
{
	if ((at < addrs->addr) || (at == INVALID_ADDR)) {
		addr = addrs;
		if (addrs->left) analyserenum_addrs(addrs->left, at, addr);
	} else /*if (at >= addrs->addr)*/ {
		if (addrs->right) analyserenum_addrs(addrs->right, at, addr);
	}
}

/*
 *
 */
taddr *analyser::enum_addrs(ADDR Addr)
{
	taddr *result = NULL;
	if (addrs) analyserenum_addrs(addrs, Addr, result);
	while ((result) && (result->flags & AF_DELETED)) {
		ADDR a = result->addr;
		result = NULL;
		analyserenum_addrs(addrs, a, result);
	}
	return result;
}

static void analyserenum_addrs_back(taddr *addrs, ADDR at, taddr *&addr)
{
	if (at <= addrs->addr) {
		if (addrs->left) analyserenum_addrs_back(addrs->left, at, addr);
	} else /*if (at > addrs->addr)*/ {
		addr = addrs;
		if (addrs->right) analyserenum_addrs_back(addrs->right, at, addr);
	}
}

/*
 *
 */
taddr *analyser::enum_addrs_back(ADDR Addr)
{
	taddr *result = NULL;
	if (addrs) analyserenum_addrs_back(addrs, Addr, result);
	while ((result) && (result->flags & AF_DELETED)) {
		ADDR a = result->addr;
		result = NULL;
		analyserenum_addrs_back(addrs, a, result);
	}
	return result;
}

static void analyserenum_labels(tlabel *labels, char *at, tlabel *&label)
{
	int i = strcmp(at, labels->name);
	if (i < 0) {
		label = labels;
		if (labels->left) analyserenum_labels(labels->left, at, label);
	} else /*if (i >= 0)*/ {
		if (labels->right) analyserenum_labels(labels->right, at, label);
	}
}

/*
 *   returns the (alphanumerically) next label to "at"
 *	or the first if "at" is NULL
 *	returns NULL if there is no preceding label
 *
 */
/* e.g.:
		tlabel *label = enum_labels(NULL);
		while (label) {
			printf("%d %08x %s\n", label->type, label->addr->addr, label->name);
			label = enum_labels(label->name);
		}
 */
tlabel *analyser::enum_labels(char *at)
{
	tlabel *result = NULL;
	if (!at) at="";
	if (labels) analyserenum_labels(labels, at, result);

	// label with !addr mustnt be returned
	while ((result) && (!result->addr)) {
		char *name = result->name;
		result = NULL;
		analyserenum_labels(labels, name, result);
	}

	return result;
}

static void analyserenum_labels_back(tlabel *labels, char *at, tlabel *&label)
{
	int i = strcmp(at, labels->name);
	if (i <= 0) {
		if (labels->left) analyserenum_labels_back(labels->left, at, label);
	} else /*if (i >= 0)*/ {
		label = labels;
		if (labels->right) analyserenum_labels_back(labels->right, at, label);
	}
}

/*
 *
 */
tlabel *analyser::enum_labels_back(char *at)
{
	tlabel *result = NULL;
	// FIXME:
	if (!at) at="\xff\xff\xff";
	if (labels) analyserenum_labels_back(labels, at, result);

	// labels with !addr mustnt be returned
	while ((result) && (!result->addr)) {
		char *name = result->name;
		result = NULL;
		analyserenum_labels_back(labels, name, result);
	}

	return result;
}

/*
 *
 */
taddr_typetype analyser::examine_data(ADDR Addr)
{
	if ((valid_read_addr(Addr)) && (valid_addr(Addr, scinitialized))) {
		DPRINTF("examinating data @%08lx:\n", Addr);

	} else return dt_unknown;
	return dt_unknown;
}

/*
 *
 */
static taddr *analyserfindaddr(taddr *addrs, ADDR Addr)
{
	if (addrs) {
		if (Addr < addrs->addr) return analyserfindaddr(addrs->left, Addr);
		if (Addr > addrs->addr) return analyserfindaddr(addrs->right, Addr);
		return (addrs->flags & AF_DELETED) ? NULL : addrs;
	}
	return NULL;
}

/*
 *
 */
taddr *analyser::find_addr(ADDR Addr)
{
	return analyserfindaddr(addrs, Addr);
}

/*
 *   finds the address Addr belongs to (if address has size > 1)
 */
taddr *analyser::find_addr_context(ADDR Addr)
{
	taddr *res = enum_addrs_back(Addr);
	if (res && res->type.type != dt_unknown && (res->addr + res->type.length > Addr)) {
		return res;
	} else {
		return NULL;
	}
}

/*
 *	findaddrfunc finds the function the Addr belongs to (if possible/applicable).
 */
taddr *analyser::find_addr_func(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (!a) a = enum_addrs_back(Addr);
	while ((a) && (!(a->flags & AF_FUNCTION_SET))) {
		if (a->flags & AF_FUNCTION_END) return NULL;
		a = enum_addrs_back(a->addr);
	}
	return (a) ? a->thisfunc : NULL;
}

/*
 *	findaddrlabel searches back to the last label
 */
taddr *analyser::find_addr_label(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (!a) a = enum_addrs_back(Addr);
	while ((a) && (!a->label)) {
		if (a->flags & AF_FUNCTION_END) return NULL;
		a = enum_addrs_back(a->addr);
	}
	return (a) ? a : NULL;
}

/*
 *
 */
static tlabel *analyserfindlabel(tlabel *labels, char *label)
{
	if (labels) {
		int i=strcmp(label, labels->name);
		if (i < 0) return analyserfindlabel(labels->left, label);
		if (i > 0) return analyserfindlabel(labels->right, label);
		return labels->addr ? labels : NULL;
	}
	return NULL;
}

/*
 *
 */
tlabel *analyser::find_label(char *label)
{
	return analyserfindlabel(labels, label);
}

/*
 *	called once every time the analyser has nothing more to do.
 */
void	analyser::finish()
{
	DPRINTF("the analyser finished (for now).\n");
	cur_func = NULL;
	addr = INVALID_ADDR;
	set_active(false);
}

/*
 *
 */
void analyser::free_addr(taddr *Addr)
{
	if (Addr) {
		// label will be freed separatly
          if (Addr->xrefs) {
          	Addr->xrefs->destroy();
               delete Addr->xrefs;
          }
		free_comments(Addr);
		free(Addr);
	}
}

/*
 *
 */
void	analyser::free_addrs(taddr *addrs)
{
	if (addrs) {
		free_addrs(addrs->left);
		free_addrs(addrs->right);
		free_addr(addrs);
	}
}

/*
 *
 */
void analyser::free_comments(taddr *Addr)
{
	comment_list *c = Addr->comments;
	if (c) {
		c->destroy();
		delete c;
	}
	Addr->comments = NULL;
}

/*
 *
 */
void analyser::free_label(tlabel *label)
{
	if (label) {
		if (label->name) free(label->name);
		free(label);
	}
}


/*
 *
 */
void analyser::free_labels(tlabel *labels)
{
	if (labels) {
		free_labels(labels->left);
		free_labels(labels->right);
		free_label(labels);
	}
}

static void analysergetaddrcount(taddr *addr, int *c)
{
	if (addr) {
		analysergetaddrcount(addr->left, c);
		if (!(addr->flags & AF_DELETED)) (*c)++;
		analysergetaddrcount(addr->right, c);
	}
}

/*
 *
 */
int	analyser::get_addr_count()
{
	int c=0;
	analysergetaddrcount(addrs, &c);
	return c;
}


static void analysergetlabelcount(tlabel *l, int *c)
{
	if (l) {
		analysergetlabelcount(l->left, c);
		if (l->addr) (*c)++;
		analysergetlabelcount(l->right, c);
	}
}

/*
 *
 */
int	analyser::get_label_count()
{
	int c=0;
	analysergetlabelcount(labels, &c);
	return c;
}
/*
 *
 */
tlabel *analyser::get_addr_label(ADDR Addr)
{
	taddr	*a = find_addr(Addr);

	return (a) ? a->label : NULL;
}

char *analyser::get_addr_section_name(ADDR Addr)
{
	return NULL;
}

/*
 *
 */
bool	analyser::goto_addr(ADDR Addr, ADDR func)
{
	if (first_explored < last_explored) {
		DPRINTF("explored->add(%08lx - %08lx)\n", first_explored, last_explored);
		explored->add(first_explored, last_explored);
	}

	addr	= Addr;

	if (!valid_code_addr(addr) || explored->contains(addr)) {
		DPRINTF("Address: %08lx Valid: %d Explored: %d\n", addr, validcodeaddr(addr), explored->contains(addr));
		do {
			if (!pop_addr(&addr, &func)) {
				first_explored	= last_explored = INVALID_ADDR;
				addr	= INVALID_ADDR;
				return false;
			}

			DPRINTF("pop %08lx   (Valid: %d  Explored: %d)\n", addr, validcodeaddr(addr), explored->contains(addr));
		} while ((explored->contains(addr)) || (!valid_code_addr(addr)));
	}

	if (func!=INVALID_ADDR) {
		cur_func = new_addr(func);
		set_addr_func(cur_func, cur_func);
	}

	next_explored = explored->find_next(addr);
	first_explored	= last_explored = addr;
	return true;
}

/*
 *
 */
void analyser::init_code_analyser()
{
	code = new code_analyser();
	code->init(this);
}

/*
 *
 */
void analyser::init_data_analyser()
{
	data = new data_analyser();
	data->init(this);
}

/*
 *
 */
void	analyser::log(char *s)
{
	// stub
}

/*
 *
 */
CPU_ADDR analyser::map_addr(ADDR Addr)
{
	/*
	 * 	this function should map the independent address Addr to a
	 * 	processor dependent
	 * 	e.g.    .23423 --> 00234324    (or something like this)
	 *   it is only used for relativ calls/jumps
	 */
	CPU_ADDR a;
	a.addr32.seg = 0;
	a.addr32.offset = Addr;
	return a;
}

taddr *analyser::new_addr(taddr *&addrs, ADDR Addr)
{
	if (addrs) {
		if (Addr < addrs->addr) return new_addr(addrs->left, Addr);
		if (Addr > addrs->addr) return new_addr(addrs->right, Addr);
	} else {
		addrs = (taddr *) smalloc0(sizeof(taddr));
		addrs->addr = Addr;
		addr_count++;
	}
	addrs->flags &= ~AF_DELETED;
	return addrs;
}

/*
 *
 */
taddr *analyser::new_addr(ADDR Addr)
{
	if (!(cur_addr_ops--)) {
		optimize_addr_tree();
	}
	return new_addr(addrs, Addr);
}

tlabel *analyser::new_label(tlabel *&labels, char *label, taddr *Addr, labeltype type)
{
	if (labels) {
		int i = strcmp(label, labels->name);
		if (i < 0) return new_label(labels->left, label, Addr, type);
		if (i > 0) return new_label(labels->right, label, Addr, type);
		if (!(labels->addr)) goto w;
	} else {
		labels = (tlabel *) smalloc0(sizeof(tlabel));
		labels->name = ht_strdup(label);
		label_count++;
	    w:
		labels->addr = Addr;
		labels->type = type;
	}
	return labels;
}

/*
 *
 */
tlabel *analyser::new_label(char *label, taddr *Addr, labeltype type, taddr *infunc)
{
	if (!(cur_label_ops--)) {
		optimize_label_tree();
	}
	tlabel *result = new_label(labels, label, Addr, type);
	if ((result) && (result->addr==Addr) && (infunc)) set_addr_func(Addr, infunc);
	return result;
}

/*
 *
 */
void	analyser::notify_progress(ADDR Addr)
{
	// stub
}

/*
 * optimizes(=balances) addr tree
 */
void analyser::optimize_addr_tree()
{
	cur_addr_ops = addrthreshold;
	// implement me!
}

/*
 * see optimize_addr_tree()
 */
void analyser::optimize_label_tree()
{
	cur_label_ops = labelthreshold;
	// implement me!
}

/*
 *
 */
bool analyser::pop_addr(ADDR *Addr, ADDR *func)
{
	if (addr_queue->count()) {

		addrqueueitem *aqi = (addrqueueitem *) addr_queue->dequeue();
		*Addr = aqi->addr;
		*func = aqi->func;
		delete aqi;

		DPRINTF("addr %08lx (from sub %08lx) poped\n", *Addr, *func);
		return true;

	} else {
		DPRINTF("pop failed -> analyser obviously finished\n");
		return false;
	}
}

/*
 *
 */
void analyser::push_addr(ADDR Addr, ADDR func)
{
	if (valid_code_addr(Addr)) {
		DPRINTF("addr %08lx (from func %08lx) pushed\n", Addr, func);
		addrqueueitem *aqi = new addrqueueitem(Addr, func);
		addr_queue->enqueue(aqi);
	}
}

static void saveaddrs(ht_object_stream *st, taddr *addr)
{
	if (addr) {
		saveaddrs(st, addr->left);

		if (!(addr->flags & AF_DELETED)) {
			st->put_int_hex(addr->addr, sizeof(ADDR), "addr");

			// xrefs
               st->put_object(addr->xrefs, "xrefs");
/* XFIX:
			txref	*xref = addr->xreflist;
			int		count = 0;
			while (xref) {
				count++;
				xref = xref->next;
			}
			st->put_int_dec(count, 4, "count_of_xrefs");
			xref = addr->xreflist;
			while (xref) {
				st->put_int_hex(xref->addr, sizeof(ADDR), "xref_addr");
				st->put_int_hex(xref->type, sizeof(txreftype), "xref_type");
				xref = xref->next;
			}
*/
			st->put_object(addr->comments, "comments");

			analyser_put_addrtype(st, &addr->type);
			
			if (addr->thisfunc) {
				st->put_int_hex(addr->thisfunc->addr, 4, "func");
			} else {
				st->put_int_hex(INVALID_ADDR, 4, "func");
			}

			st->put_int_hex(addr->flags, 4, "flags");
		}

		saveaddrs(st, addr->right);
	}
}

static void savelabels(ht_object_stream *st, tlabel *label)
{
	if (label) {
		savelabels(st, label->left);

		if (label->addr) {
			// label isn't deleted

			st->put_int_hex(label->addr->addr, sizeof(ADDR), "addr");

			st->put_string(label->name, "name");

			int a = (int)label->type;
			st->put_int_hex(a, 1, "type");
		}

		savelabels(st, label->right);
	}
}

/*
 *
 */
void analyser::store(ht_object_stream *st)
{
	PUT_OBJECT(st, addr_queue);
	PUT_INT_DEC(st, ops_parsed);
	PUT_BOOL(st, active);
	PUT_INT_HEX(st, next_explored);
	PUT_INT_HEX(st, first_explored);
	PUT_INT_HEX(st, last_explored);
	PUT_INT_HEX(st, mode);
	PUT_OBJECT(st, explored);
	PUT_OBJECT(st, initialized);

	st->put_info("addresses");
	st->put_int_dec(get_addr_count(), 4, "addr_count");
	saveaddrs(st, addrs);

	st->put_info("labels");
	st->put_int_dec(get_label_count(), 4, "label_count");
	savelabels(st, labels);

	PUT_OBJECT(st, analy_disasm);
	PUT_OBJECT(st, disasm);
	PUT_OBJECT(st, code);
	PUT_OBJECT(st, data);

	PUT_INT_DEC(st, addrthreshold);
	PUT_INT_DEC(st, labelthreshold);

	PUT_INT_DEC(st, maxopcodelength);
	if (cur_func) {
		st->put_int_hex(cur_func->addr, sizeof(ADDR), "cur_func");
	} else {
		st->put_int_hex(INVALID_ADDR, sizeof(ADDR), "cur_func");
	}
	dirty = false;
}

/*
 *
 */
int	analyser::query_config(int mode)
{
	// stub
	return 0;
}

/*
 *
 */
bool	analyser::savety_change_addr(ADDR Addr)
{
	addr	= Addr;
	return ((addr>=next_explored) || (!valid_code_addr(addr))) ? goto_addr(addr, INVALID_ADDR) : true;
}

/*
 *
 */
void analyser::set_active(bool mode)
{
	if (mode) {
		if (!active) {
			active = true;
			some_analyser_active++;
		}
	} else {
		if (active) {
			active = false;
			some_analyser_active--;
		}
	}
}


/*
 *
 */
void analyser::set_addr_func(taddr *a, taddr *func)
{
	if (a) {
		a->thisfunc = func;
		a->flags &= !AF_FUNCTION_END;
		a->flags |= AF_FUNCTION_SET;
	}
}

/*
 *
 */
void analyser::set_disasm(disassembler *Disasm)
{
	disasm = Disasm;
	if (disasm) {
		maxopcodelength = disasm->getmaxopcodelength();
	} else {
		maxopcodelength = 1;
	}
}

/*
 *	sets addr_threshold. after threshold addr_tree ops the tree will
 *   be optimized
 */
void analyser::set_addr_tree_optimize_threshold(int threshold)
{
	addrthreshold = threshold;
	if (cur_addr_ops > addrthreshold) cur_addr_ops = addrthreshold;
}

/*
 *	see set_addr_tree_optimize_threshold
 */
void analyser::set_label_tree_optimize_threshold(int threshold)
{
	labelthreshold = threshold;
	if (cur_label_ops > labelthreshold) cur_label_ops = labelthreshold;
}

/*
 *
 */
bool	analyser::valid_code_addr(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (a) {
		if ((a->type.type != dt_code) && (a->type.type != dt_unknown)) return false;
	}
	return valid_addr(Addr, sccode);
}

/*
 *
 */
bool	analyser::valid_read_addr(ADDR Addr)
{
	return valid_addr(Addr, scread);
}

/*
 *
 */
bool	analyser::valid_write_addr(ADDR Addr)
{
	return valid_addr(Addr, scwrite);
}

/*
 *
 *		interface only (there's no internal use)
 *
 */

static char *analy_addr_sym_func2(CPU_ADDR Addr, int *symstrlen, void *analy)
{
	/* should only be called/used/assigned if theanaly is set */
	taddr *a = ((analyser*)analy)->find_addr(Addr.addr32.offset);
	if ((a) && (a->label)) {
		if (symstrlen) *symstrlen = strlen(a->label->name);
		return a->label->name;
	}
	return NULL;
}

/*
 *	should return a new instance of an apropriate assembler
 *
 */
assembler *analyser::create_assembler()
{
	return NULL;
}

/*
 *
 */
comment_list *analyser::get_comments(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (a) {
		return a->comments;
	}
	return NULL;
}

/*
 *
 */
char	*analyser::get_disasm_str(ADDR Addr)
{
	if (valid_addr(Addr, scinitialized)) {
		if (disasm) {
			addr_sym_func = 0;
			byte buf[16];
			int bz = bufptr(Addr, buf, sizeof(buf));
			OPCODE *o=disasm->decode(buf, MIN(bz, maxopcodelength), map_addr(Addr));
			return disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);
		} else {
			return "<no disassembler!>";
		}
	} else {
		return "";
	}
}

/*
 *
 */
char	*analyser::get_disasm_str_formatted(ADDR Addr)
{
	if (disasm) {
		addr_sym_func_context = this;
		addr_sym_func = &analy_addr_sym_func2;

		byte buf[16];
		int bz = bufptr(Addr, buf, sizeof(buf));
		OPCODE *o=disasm->decode(buf, MIN(bz, maxopcodelength), map_addr(Addr));
		char *res = disasm->strf(o, DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE, DISASM_STRF_SMALL_FORMAT);

		addr_sym_func_context = NULL;
		addr_sym_func = NULL;

		return res;
	} else {
		return "<no disassembler!>";
	}
}

/*
 *
 */
char *analyser::get_name()
{
	return "generic";
}

/*
 *
 */
char *analyser::get_type()
{
	return "generic";
}

/*
 *
 */
ht_tree *analyser::get_xrefs(ADDR Addr)
{
	taddr *a = find_addr(Addr);
	if (!a) return NULL;
	return a->xrefs;
}

/*
 *
 */
bool analyser::is_dirty()
{
	return dirty;
}

/*
 *
 */
void analyser::make_dirty()
{
	dirty = true;
}


/*
 *
 */
void analyser::set_display_mode(int enable, int disable)
{
	mode |= enable;
	mode &= ~disable;
}

/*
 *
 */
void analyser::toggle_display_mode(int toggle)
{
	mode ^= toggle;
}

/*
 *	converts FILEADDR fileaddr to ADDR
 */
ADDR analyser::vaddr(FILEADDR fileaddr)
{
	// abstract / stub
	return INVALID_ADDR;
}

/****************************************************************************/

/*
 *
 */
void analy_disassembler::init(analyser *A)
{
	analy = A;
	disasm = NULL;
	init_disasm();
}

/*
 *
 */
int  analy_disassembler::load(ht_object_stream *st)
{
	return st->get_error();
}

/*
 *
 */
void analy_disassembler::done()
{
}

/*
 *
 */
void analy_disassembler::init_disasm()
{
	if (analy) {
		analy->set_disasm(disasm);
	}
}

/*
 *
 */
void analy_disassembler::store(ht_object_stream *f)
{
}

