/*
 *	HT Editor
 *	htformat.cc
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

#include "htformat.h"
#include "htsearch.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <exception>

#include "blockop.h"
#include "cmds.h"
#include "htapp.h"		// for popup_view_list(..)
#include "atom.h"
#include "htclipboard.h"
#include "htctrl.h"
#include "endianess.h"
#include "hteval.h"
#include "hthist.h"
#include "htiobox.h"
#include "keyb.h"
#include "htpal.h"
#include "httag.h"
#include "textedit.h"
#include "textfile.h"
#include "htprocess.h"
#include "snprintf.h"
#include "tools.h"

extern "C" {
#include "evalx.h"
#include "regex.h"
}

/*cmd_rec ht_format_viewer_cmds[] = {
	{cmd_file_truncate, true, true, NULL}
};*/

void clear_line_id(LINE_ID *l)
{
	l->id1 = 0;
	l->id2 = 0;
	l->id3 = 0;
	l->id4 = 0;
	l->id5 = 0;
}

bool compeq_line_id(const LINE_ID &a, const LINE_ID &b)
{
	return (a.id1 == b.id1 && a.id2 == b.id2
	     && a.id3 == b.id3 && a.id4 == b.id4 && a.id5 == b.id5);
}

void load_line_id(ObjectStream &st, LINE_ID &line)
{
	GET_INT32X(st, line.id1);
	GET_INT32X(st, line.id2);
	GET_INT32X(st, line.id3);
	GET_INT32X(st, line.id4);
	GET_INT32X(st, line.id5);
}

void store_line_id(ObjectStream &st, const LINE_ID &line)
{
	PUT_INT32X(st, line.id1);
	PUT_INT32X(st, line.id2);
	PUT_INT32X(st, line.id3);
	PUT_INT32X(st, line.id4);
	PUT_INT32X(st, line.id5);
}


/*
 *	CLASS ht_data_tagstring
 */

class ht_data_tagstring: public Object {
public:
	char *value;

	ht_data_tagstring(const char *tagstr = NULL) :
		value(tag_strdup(tagstr))
	{
	}

	~ht_data_tagstring()
	{
		free(value);
	}
};


/*
 *	CLASS ht_search_request
 */

ht_search_request::ht_search_request(uint _search_class, uint _type, uint _flags)
{
	search_class = _search_class;
	type = _type;
	flags = _flags;
}

/*
 *	CLASS ht_format_viewer_entry
 */

class ht_format_viewer_entry: public Object {
public:
	ht_view *instance;
	format_viewer_if *interface;

	ht_format_viewer_entry(ht_view *i, format_viewer_if *aIf):
		instance(i),
		interface(aIf)
	{
	}
		
};

/*
 *	CLASS ht_format_group
 */

void ht_format_group::init(Bounds *b, int options, const char *desc, File *f, bool own_f, bool editable_f, format_viewer_if **i, ht_format_group *format_group)
{
	ht_format_viewer::init(b, desc, 0, f, format_group);
	VIEW_DEBUG_NAME("ht_format_group");

	xgroup = new ht_xgroup();
	xgroup->init(b, options, desc);
	xgroup->group = group;

	format_views = new Array(true);	// a list of ht_format_viewer_entrys

	own_file = own_f;
	editable_file = editable_f;
	if (i) init_ifs(i);
}

void ht_format_group::done()
{
	done_ifs();

	delete format_views;

	xgroup->done();
	delete xgroup;

	ht_format_viewer::done();

	if (own_file) delete file;
}

int ht_format_group::childcount() const
{
	return xgroup->childcount();
}

bool ht_format_group::done_if(format_viewer_if *i, ht_view *v)
{
	remove(v);
	if (i->done) {
		i->done(v); 
	} else {
		v->done();
		delete v;
	}
	return true;
}

void ht_format_group::done_ifs()
{
	foreach(ht_format_viewer_entry, e, *format_views, {
		done_if(e->interface, e->instance);
	})
}

bool ht_format_group::edit()
{
	return file->getAccessMode() & IOAM_WRITE;
}

bool ht_format_group::focus(ht_view *view)
{
	bool r = ht_format_viewer::focus(view);
	if (!r) r = xgroup->focus(view);
	return r;
}

const char *ht_format_group::func(uint i, bool execute)
{
	return ht_format_viewer::func(i, execute);
}

void ht_format_group::getbounds(Bounds *b)
{
	xgroup->getbounds(b);
}

void *ht_format_group::get_shared_data()
{
	return shared_data;
}

ht_view *ht_format_group::getfirstchild()
{
	return xgroup->getfirstchild();
}

ht_view *ht_format_group::getselected()
{
	return xgroup->getselected();
}

int ht_format_group::get_pindicator_str(char *buf, int max_len)
{
	ht_view *c = xgroup->current;
	if (c && (c->options & VO_FORMAT_VIEW)) {
		return ((ht_format_viewer*)c)->get_pindicator_str(buf, max_len);
	} else {
		if (max_len > 0) *buf = 0;
		return 0;
	}
}

bool ht_format_group::get_hscrollbar_pos(int *pstart, int *psize)
{
	ht_view *c = xgroup->current;
	if (c && (c->options & VO_FORMAT_VIEW)) {
		return ((ht_format_viewer*)c)->get_hscrollbar_pos(pstart, psize);
	}
	return false;
}

bool ht_format_group::get_vscrollbar_pos(int *pstart, int *psize)
{
	ht_view *c = xgroup->current;
	if (c && (c->options & VO_FORMAT_VIEW)) {
		return ((ht_format_viewer*)c)->get_vscrollbar_pos(pstart, psize);
	}
	return false;
}

void ht_format_group::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_keypressed: {
		int i = 0;
		switch (msg->data1.integer) {
		case K_F12: i++;
		case K_F11: i++;
		case K_F10: i++;
		case K_F9: i++;
		case K_F8: i++;
		case K_F7: i++;
		case K_F6: i++;
		case K_F5: i++;
		case K_F4: i++;
		case K_F3: i++;
		case K_F2: i++;
		case K_F1: {
			i++;
			htmsg m;
			m.msg = msg_funcquery;
			m.type = mt_empty;
			m.data1.integer = i;
			sendmsg(&m);
			if (m.msg == msg_retval) {
				sendmsg(msg_funcexec, i);
				clearmsg(msg);
				return;
			}
			break;
		}
		}
		break;
	}
	}
	ht_format_viewer::handlemsg(msg);
	xgroup->handlemsg(msg);
	switch (msg->msg) {
	case msg_funcexec:
		if (func(msg->data1.integer, 1)) {
			clearmsg(msg);
			return;
		}
		break;
	case msg_funcquery: {
		const char *s=func(msg->data1.integer, 0);
		if (s) {
			msg->msg=msg_retval;
			msg->data1.cstr=s;
		}
		break;
	}
	}
}

bool ht_format_group::init_if(format_viewer_if *i)
{
	Bounds b;
	getbounds(&b);
	b.x = 0;
	b.y = 0;
	
	if (i->init) {
		try {
			ht_view *v = i->init(&b, file, this);
			if (v) {
				v->sendmsg(msg_complete_init, 0);
				insert(v);
				format_views->insert(new ht_format_viewer_entry(v, i));
				return true;
			}
		} catch (const Exception &x) {
			errorbox("unhandled exception: %y", &x);
		} catch (const std::exception &x) {
			errorbox("unhandled exception: %s", x.what());
		} catch (...) {
			errorbox("unhandled exception: unknown");
		}
	}
	return false;
}

void ht_format_group::init_ifs(format_viewer_if **ifs)
{
	format_viewer_if **i = ifs;
	while (*i) {
		init_if(*i);
		i++;
	}
	ifs = i;
}

void ht_format_group::insert(ht_view *view)
{
	xgroup->insert(view);
}

void ht_format_group::move(int rx, int ry)
{
	ht_format_viewer::move(rx, ry);
	xgroup->move(rx, ry);
}

void ht_format_group::receivefocus()
{
	xgroup->receivefocus();
}

void ht_format_group::redraw()
{
	xgroup->redraw();
}

void ht_format_group::releasefocus()
{
	xgroup->releasefocus();
}

void ht_format_group::remove(ht_view *view)
{
	xgroup->remove(view);
}

void ht_format_group::resize(int rw, int rh)
{
	ht_format_viewer::resize(rw, rh);
	xgroup->resize(rw, rh);
}

void ht_format_group::setgroup(ht_group *_group)
{
	xgroup->setgroup(_group);
}

bool ht_format_group::func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	ht_format_viewer *v = dynamic_cast<ht_format_viewer *>(xgroup->current);
	if (v) {
		return v->func_handler(result, name, params);
	} else {
		return false;
	}
}

bool ht_format_group::symbol_handler(eval_scalar *result, char *name)
{
	ht_format_viewer *v = dynamic_cast<ht_format_viewer *>(xgroup->current);
	if (v) {
		return v->symbol_handler(result, name);
	} else {
		return false;
	}
}

/*
 *	CLASS ht_viewer
 */

void ht_viewer::init(Bounds *b, const char *desc, uint c)
{
	ht_view::init(b, VO_OWNBUFFER | VO_BROWSABLE | VO_SELECTABLE | VO_MOVE | VO_RESIZE, desc);
	caps = c;
	
	growmode = MK_GM(GMH_FIT, GMV_FIT);
}

const char *ht_viewer::func(uint i, bool execute)
{
	return NULL;
}

void ht_viewer::handlemsg(htmsg *msg)
{
	int i=0;
	switch (msg->msg) {
	case msg_keypressed: {
		switch (msg->data1.integer) {
		case K_F12: i++;
		case K_F11: i++;
		case K_F10: i++;
		case K_F9: i++;
		case K_F8: i++;
		case K_F7: i++;
		case K_F6: i++;
		case K_F5: i++;
		case K_F4: i++;
		case K_F3: i++;
		case K_F2: i++;
		case K_F1: {
			i++;
			htmsg m;
			m.msg = msg_funcquery;
			m.type = mt_empty;
			m.data1.integer = i;
			sendmsg(&m);
			if (m.msg == msg_retval) {
				sendmsg(msg_funcexec, i);
				clearmsg(msg);
				return;
			}
			break;
		}
		}
		break;
	}
	case msg_funcexec:
		if (func(msg->data1.integer, 1)) {
			clearmsg(msg);
			return;
		}
		break;
	case msg_funcquery: {
		const char *s = func(msg->data1.integer, 0);
		if (s) {
			msg->msg = msg_retval;
			msg->data1.cstr = s;
		}
		break;
	}
	}
	ht_view::handlemsg(msg);
}

/*
 *	CLASS ht_format_viewer
 */

void ht_format_viewer::init(Bounds *b, const char *desc, uint caps, File *f, ht_format_group *fg)
{
	ht_viewer::init(b, desc, caps);
	options |= VO_FORMAT_VIEW;
	VIEW_DEBUG_NAME("ht_format_viewer");
	file = f;
	format_group = fg;

	last_search_request = NULL;
/*	vs_history = new ht_stack();
	vs_history->init();*/
}

void ht_format_viewer::done()
{
	delete last_search_request;

	ht_viewer::done();
}

bool ht_format_viewer::pos_to_offset(viewer_pos pos, FileOfs *ofs)
{
	return false;
}

void ht_format_viewer::clear_viewer_pos(viewer_pos *p)
{
}

bool ht_format_viewer::compeq_viewer_pos(viewer_pos *a, viewer_pos *b)
{
	return false;
}

bool ht_format_viewer::continue_search()
{
	if (last_search_request) {
		ht_search_result *r = NULL;
		if (last_search_physical) {
			FileOfs o, no;
			if (get_current_offset(&o)) {
				try {
					if (last_search_request->search_class == SC_PHYSICAL
					 && next_logical_offset(o, &no)) {
						r = psearch(last_search_request, no, last_search_end_ofs);
					}
				} catch (const Exception &e) {
					errorbox("error: %y", &e);
				}
			}
		} else {
			viewer_pos a, na;
			if (get_current_pos(&a)) {
				try {
					if (last_search_request->search_class == SC_VISUAL
					 && next_logical_pos(a, &na)) {
						r = vsearch(last_search_request, na, last_search_end_pos);
					}
				} catch (const Exception &e) {
					errorbox("error: %y", &e);
				}
			}
		}
		
		if (r) return show_search_result(r);
	}
	return false;
}

bool ht_format_viewer::func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	return false;
}

bool ht_format_viewer::symbol_handler(eval_scalar *result, char *name)
{
	return false;
}

bool ht_format_viewer::get_current_offset(FileOfs *ofs)
{
	return false;
}

bool ht_format_viewer::get_current_pos(viewer_pos *pos)
{
	return false;
}

bool ht_format_viewer::get_current_real_offset(FileOfs *ofs)
{
	return get_current_offset(ofs);
}

File *ht_format_viewer::get_file()
{
	return file;
}

int ht_format_viewer::get_pindicator_str(char *buf, int max_len)
{
	if (max_len > 0) *buf = 0;
	return 0;
}

bool ht_format_viewer::get_hscrollbar_pos(int *pstart, int *psize)
{
	return false;
}

bool ht_format_viewer::get_vscrollbar_pos(int *pstart, int *psize)
{
	return false;
}

bool ht_format_viewer::goto_offset(FileOfs ofs, bool save_vstate)
{
	return false;
}

bool ht_format_viewer::goto_pos(viewer_pos pos, bool save_vstate)
{
	return false;
}

static bool format_viewer_func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	ht_format_viewer *viewer = (ht_format_viewer*)eval_get_context();
	return viewer->func_handler(result, name, params);
}

static bool format_viewer_symbol_handler(eval_scalar *result, char *name)
{
	ht_format_viewer *viewer = (ht_format_viewer*)eval_get_context();
	return viewer->symbol_handler(result, name);
}

void ht_format_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case cmd_popup_dialog_eval:
                eval_dialog(format_viewer_func_handler, format_viewer_symbol_handler, this);
                clearmsg(msg);
                return;
	case msg_goto_offset:
		if (goto_offset((FileOfs)msg->data1.q, false)) {
			clearmsg(msg);
			return;
		}
		break;
	case msg_vstate_restore:
		vstate_restore((Object*)msg->data1.ptr);
		clearmsg(msg);
		return;
	case cmd_file_truncate: {
		File *f = (File*)msg->data1.ptr;
		FileOfs o = (FileOfs)msg->data2.q;
		if (file == f) {
			ht_format_loc loc;
			loc_enum_start();
			while (loc_enum_next(&loc)) {
				if (o < loc.start+loc.length) {
					if (confirmbox("truncating at %08qx will destroy format '%s', continue ? \n(format ranges from %08qx to %08qx)", o, loc.name, loc.start, loc.start+loc.length) != button_yes) {
						clearmsg(msg);
						return;
					}
					break;
				}
			}
		}
		break;
	}
	case cmd_edit_mode_i: {
		if (file/* && (file==msg->data1.ptr)*/) {
			try {
				file->setAccessModex(IOAM_READ | IOAM_WRITE);
				htmsg m;
				m.msg = cmd_edit_mode;
				m.type = mt_broadcast;
				sendmsg(&m);
			} catch (const IOException &e) {
				String fn;
				errorbox("can't open file %y in write mode! (%y)", &file->getFilename(fn), &e);
			}
		}
		clearmsg(msg);
		return;
	}
	case cmd_view_mode_i:
		if (file /*&& (file==msg->data1.ptr)*/) {
			FileOfs size = file->getSize();
			file->cntl(FCNTL_MODS_INVD);
			try {
				file->setAccessModex(IOAM_READ);
				htmsg m;
				m.msg = cmd_view_mode;
				m.type = mt_broadcast;
				sendmsg(&m);
			} catch (const IOException &e) {
				String fn;
				errorbox("can't (re)open file %y in read mode! (%y)", &file->getFilename(fn), &e);
			}
			if (size != file->getSize()) {
				htmsg m;
				m.msg = msg_filesize_changed;
				m.type = mt_broadcast;
				sendmsg(&m);
			}
		}
		clearmsg(msg);
		return;
	}
	ht_viewer::handlemsg(msg);
}

void ht_format_viewer::loc_enum_start()
{
}

bool ht_format_viewer::loc_enum_next(ht_format_loc *loc)
{
	return false;
}

bool ht_format_viewer::next_logical_pos(viewer_pos pos, viewer_pos *npos)
{
	return false;
}

bool ht_format_viewer::next_logical_offset(FileOfs ofs, FileOfs *nofs)
{
	return false;
}

bool ht_format_viewer::offset_to_pos(FileOfs ofs, viewer_pos *pos)
{
	return false;
}

bool ht_format_viewer::vstate_save()
{
	Object *vs = vstate_create();
	if (vs) {
		htmsg m;
		m.msg = msg_vstate_save;
		m.type = mt_empty;
		m.data1.ptr = vs;
		m.data2.ptr = this;
		app->sendmsg(&m);
		return true;
	}
	return false;
}

uint ht_format_viewer::pread(FileOfs ofs, void *buf, uint size)
{
	try {
		file->seek(ofs);
		return file->read(buf, size);
	} catch (const IOException &e) {
		return 0;
	}
}

ht_search_result *ht_format_viewer::psearch(ht_search_request *search, FileOfs start, FileOfs end)
{
	return 0;
}

void ht_format_viewer::pselect_add(FileOfs start, FileOfs end)
{
}

void ht_format_viewer::pselect_get(FileOfs *start, FileOfs *end)
{
}

void ht_format_viewer::pselect_set(FileOfs start, FileOfs end)
{
}

uint ht_format_viewer::pwrite(FileOfs ofs, void *buf, uint size)
{
	sendmsg(msg_file_changed);
	file->seek(ofs);
	return file->write(buf, size);
}

bool ht_format_viewer::qword_to_offset(uint64 q, FileOfs *ofs)
{
	return false;
}

bool ht_format_viewer::qword_to_pos(uint64 q, viewer_pos *pos)
{
	return false;
}

bool ht_format_viewer::show_search_result(ht_search_result *r)
{
	switch (r->search_class) {
	case SC_PHYSICAL: {
		ht_physical_search_result *s = (ht_physical_search_result*)r;
		if (!goto_offset(s->offset, this)) return false;
		pselect_set(s->offset, s->offset + s->size);
		return true;
	}
	case SC_VISUAL: {
		ht_visual_search_result *s = (ht_visual_search_result*)r;
		return goto_pos(s->pos, this);
	}
	}
	return false;
}

bool ht_format_viewer::string_to_qword(const char *string, uint64 *q)
{
	eval_scalar r;
	if (eval(&r, string, format_viewer_func_handler, format_viewer_symbol_handler, this)) {
		eval_int i;
		scalar_context_int(&r, &i);
		scalar_destroy(&r);
		*q = i.value;
		return true;
	} else {
		const char *s;
		int p;
		get_eval_error(&s, &p);
		ht_snprintf(globalerror, GLOBAL_ERROR_SIZE, "%s at pos %d", s, p);
	}
	return false;
}

bool ht_format_viewer::string_to_pos(const char *string, viewer_pos *pos)
{
	uint64 q;
	if (!string_to_qword(string, &q)) return false;
	return qword_to_pos(q, pos);
}

bool ht_format_viewer::string_to_offset(char *string, FileOfs *ofs)
{
	uint64 q;
	if (!string_to_qword(string, &q)) return false;
	return qword_to_offset(q, ofs);
}

Object *ht_format_viewer::vstate_create()
{
	return NULL;
}

void ht_format_viewer::vstate_restore(Object *view_state)
{
}

uint ht_format_viewer::vread(viewer_pos pos, void *buf, uint size)
{
	FileOfs o;
	if (pos_to_offset(pos, &o)) {
		return pread(o, buf, size);
	}
	return 0;
}

ht_search_result *ht_format_viewer::vsearch(ht_search_request *search, viewer_pos start, viewer_pos end)
{
	return NULL;
}

void ht_format_viewer::vselect_add(viewer_pos start, viewer_pos end)
{
	FileOfs so, eo;
	if (pos_to_offset(start, &so) && pos_to_offset(end, &eo)) {
		return pselect_add(so, eo);
	}
}

void ht_format_viewer::vselect_get(viewer_pos *start, viewer_pos *end)
{
	HT_ERROR("NYI!");
}

void ht_format_viewer::vselect_set(viewer_pos start, viewer_pos end)
{
	FileOfs so, eo;
	if (pos_to_offset(start, &so) && pos_to_offset(end, &eo)) {
		return pselect_set(so, eo);
	}
}

uint ht_format_viewer::vwrite(viewer_pos pos, void *buf, uint size)
{
	FileOfs o;
	if (pos_to_offset(pos, &o)) {
		return pwrite(o, buf, size);
	}
	return 0;
}

void uformat_viewer_pos::load(ObjectStream &s)
{
	sub = (ht_sub*)(intptr_t(GETX_INT32D(s, "sub_idx")));
	load_line_id(s, line_id);
	PUT_INT32D(s, tag_group);
	PUT_INT32D(s, tag_idx);
}

void uformat_viewer_pos::store(ObjectStream &s) const
{
	int sub_idx = sub->uformat_viewer->sub_to_idx(sub);
	PUT_INT32D(s, sub_idx);
	store_line_id(s, line_id);
	PUT_INT32D(s, tag_group);
	PUT_INT32D(s, tag_idx);
}

/*
 *	CLASS ht_uformat_view
 */

#define UFORMAT_VIEWER_VSTATE MAGIC32("VST\x01")

class ht_uformat_viewer_vstate: public Object {
public:
	bool edit;
	bool resolve; // resolve uformat_viewer_pos

	/* top line position */
	uformat_viewer_pos top;
	/* cursor line and tag position */
	uformat_viewer_pos cursor;
	int cursor_state;
	int cursor_ypos;
	/* selection*/
	FileOfs sel_start;
	FileOfs sel_end;

	ht_uformat_viewer_vstate() { resolve = false; };
	ht_uformat_viewer_vstate(BuildCtorArg&a): Object(a) {};
	
	virtual	void load(ObjectStream &s)
	{
		resolve = true;
		GET_BOOL(s, edit);
		top.load(s);
		cursor.load(s);
		GET_INT32X(s, cursor_state);
		GET_INT32D(s, cursor_ypos);
		GET_INT64D(s, sel_start);
		GET_INT64D(s, sel_end);
	}
	
	virtual	void store(ObjectStream &s) const
	{
		PUT_BOOL(s, edit);
		top.store(s);
		cursor.store(s);
		PUT_INT32X(s, cursor_state);
		PUT_INT32D(s, cursor_ypos);
		PUT_INT64D(s, sel_start);
		PUT_INT64D(s, sel_end);
	}

	virtual	ObjectID getObjectID() const
	{
		return UFORMAT_VIEWER_VSTATE;
	}
};

void ht_uformat_viewer::init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group)
{
	tagpal.data = NULL;
	tagpal.size = 0;
	ht_format_viewer::init(b, desc, caps, file, format_group);
	VIEW_DEBUG_NAME("ht_uformat_view");
	first_sub = NULL;
	last_sub = NULL;
	clear_viewer_pos(&top);
	clear_viewer_pos(&cursor);
	xscroll = 0;
	cursor_ypos = 0;
	cursor_visual_length = 0;
	cursor_visual_xpos = 0;
	cursor_select = 0;
	cursor_select_start = -1ULL;
	sel_start = 0;
	sel_end = 0;
	isdirty_cursor_line = 0;

	search_caps = SEARCHMODE_VREGEX;

	uf_initialized = false;
}

void ht_uformat_viewer::done()
{
	edit_end();
	clear_subs();
	free(tagpal.data);
	ht_format_viewer::done();
}

int ht_uformat_viewer::address_input(const char *title, char *result, int limit, uint32 histid)
{
	Bounds b;
	app->getbounds(&b);
	b.x = (b.w - 60) / 2,
	b.y = (b.h - 8) / 2;
	b.w = 60;
	b.h = 8;

	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, title, FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);

	ht_strinputfield *input;
	const char *label = "~Address";

	Bounds  b2;
	b2.x = 3 + strlen(label);
	b2.y = 1;
	b2.w = b.w - 3 - b2.x;
	b2.h = 1;

	List *hist = NULL;
	if (histid) hist = (List*)getAtomValue(histid);
	input = new ht_strinputfield();
	input->init(&b2, limit, hist);
	ht_inputfield_data d;
	d.text = (byte*)result;
	d.textlen = strlen((char*)d.text);
	input->databuf_set(&d, sizeof d);
	dialog->insert(input);

	if (label) {
		b2.x = 1;
		b2.y = 1;
		b2.w = 3 + strlen(label) - b2.x;
		b2.h = 1;

		ht_label *lab = new ht_label();
		lab->init(&b2, label, input);
		dialog->insert(lab);
	}

	b2.x = b.w - 45;
	b2.y = b.h - 5;
	b2.w = 10;
	b2.h = 2;

	ht_button *bok = new ht_button();
	bok->init(&b2, "O~k", button_ok);
	dialog->insert(bok);

	b2.x += 12;

	ht_button *bcancel = new ht_button();
	bcancel->init(&b2, "~Cancel", button_cancel);
	dialog->insert(bcancel);

	b2.x += 12;
	b2.w = 14;

	ht_button *bhelp = new ht_button();
	bhelp->init(&b2, "~Functions", 100);
	dialog->insert(bhelp);
	
	int r;
	bool run = true;
	int retval = button_cancel;
	while (run && (r = dialog->run(0)) != button_cancel) {
		switch (r) {
		case 100: {
			dialog_eval_help(format_viewer_func_handler, format_viewer_symbol_handler, this);
			break;
		}
		case button_ok: {
			int dsize = input->datasize();
			ht_inputfield_data *data = ht_malloc(dsize);
			ViewDataBuf vdb(input, data, dsize);
			bin2str(result, data->text, data->textlen);
			free(data);
			if (hist) insert_history_entry(hist, result, 0);
			run = false;
			retval = button_ok;
			break;
		}
		}
	}

	dialog->done();
	delete dialog;
	return retval;
}

void ht_uformat_viewer::adjust_cursor_group()
{
	cursorline_get();
	int g = tag_count_groups(cursor_line);
	if (cursor.tag_group >= g) cursor.tag_group=0;
}

void ht_uformat_viewer::adjust_cursor_idx()
{
	cursorline_get();
	int c = tag_count_selectable_tags_in_group(cursor_line, cursor.tag_group);
	if (cursor.tag_idx > c-1) cursor.tag_idx = c-1;
}

int ht_uformat_viewer::center_view(viewer_pos p)
{
	top = p.u;
	int r = prev_line(&top, size.h/2);
	cursorline_dirty();
	return r;
}

void ht_uformat_viewer::check_cursor_visibility()
{
	if (cursor_state != cursor_state_disabled) {
		if (cursor_ypos < 0 || cursor_ypos >= size.h) {
			cursor_state = cursor_state_invisible;
		} else {
			cursor_state = cursor_state_visible;
		}
	}
}

void ht_uformat_viewer::complete_init()
{
	if (uf_initialized) return;
	cursor_state = cursor_state_disabled;
	if (!first_sub) {
		uf_initialized = true;
		return;
	}
	/*  initialize top_*  */
	clear_viewer_pos(&top);
	top.sub = first_sub;
	top.sub->first_line_id(&top.line_id);

	cursor_tag_micropos = 0;

	uformat_viewer_pos p;
	clear_viewer_pos(&p);
	p.sub = first_sub;
	p.sub->first_line_id(&p.line_id);
	char line[1024];
	cursor_ypos--;
	do {
		cursor_ypos++;
		if (!p.sub->getline(line, sizeof line, p.line_id)) break;
		if (tag_count_selectable_tags(line)) {
			if (cursor_ypos < size.h) {
				cursor_state = cursor_state_visible;
			} else {
				cursor_state = cursor_state_invisible;
				cursor_ypos = -1;
			}
			break;
		}
	} while (next_line(&p, 1) && cursor_ypos < size.h);
	p.tag_idx = 0;
	p.tag_group = 0;
	if (cursor_state == cursor_state_disabled) {
		p.sub = first_sub;
		p.sub->first_line_id(&p.line_id);
		if (p.sub->getline(line, sizeof line, p.line_id)) {
			cursor_ypos = -1;
			cursor_state = cursor_state_invisible;
		}
	}
	cursor = p;
	/* get cursorline */
	cursorline_dirty();
	cursorline_get();
	/* initialize visual */
	update_visual_info();
	/* initialize misc */
	update_misc_info();

	uf_initialized = true;
}

int ht_uformat_viewer::cursor_left()
{
	if (cursor.tag_idx) {
		cursor.tag_idx--;
		update_visual_info();
		update_misc_info();
		return 1;
	} else {
		if (cursor_up(1)) {
			cursor_end();
			return 1;
		}
	}
	return 0;
}

int ht_uformat_viewer::cursor_right()
{
	cursorline_get();
	if (cursor.tag_idx < tag_count_selectable_tags_in_group(cursor_line, cursor.tag_group)-1) {
		cursor.tag_idx++;
		update_visual_info();
		update_misc_info();
		return 1;
	} else {
		if (cursor_down(1)) {
			cursor_home();
			return 1;
		}
	}
	return 0;
}

int ht_uformat_viewer::cursor_up(int n)
{
	switch (cursor_state) {
		case cursor_state_invisible:
		case cursor_state_visible: {
			if (n == 1 && cursor_state == cursor_state_visible) {
				int r = 0;
				uformat_viewer_pos c;
				clear_viewer_pos(&c);
				c = cursor;
				char c_line[1024];
				int c_ypos = cursor_ypos;
				int c_tag_idx = cursor.tag_idx;
				int c_tag_group = cursor.tag_group;
				int d_tag_group = cursor.tag_group;

				while (prev_line(&c, 1) && c_ypos >= 0) {
					c_ypos--;
					c.sub->getline(c_line, sizeof c_line, c.line_id);
					int g = tag_count_groups(c_line);
					if (d_tag_group < g) c_tag_group = d_tag_group;
					int s;
					if (c_tag_group >= g) {
						c_tag_group = g-1;
						s = tag_count_selectable_tags_in_group(c_line, c_tag_group);
						if (s) {
							c_tag_idx = s-1;
							r = 1;
							break;
						}
					} else {
						s = tag_count_selectable_tags_in_group(c_line, c_tag_group);
						if (s) {
							if (c_tag_idx >= s) c_tag_idx = s-1;
							r = 1;
							break;
						}
					}
				}
				if (r) {
					cursor = c;
					memcpy(cursor_line, c_line, sizeof cursor_line);
					cursorline_dirty();
					cursor_ypos = c_ypos;
					cursor.tag_idx = c_tag_idx;
					cursor.tag_group = c_tag_group;
					if (cursor_ypos <= -1) scroll_up(-cursor_ypos);
					update_misc_info();
					update_visual_info();
					if (edit()) update_micropos();
				} else {
					scroll_up(n);
				}
				return r;
			} else {
				int r=0;
				char c_line[1024];
				int c_tag_idx = cursor.tag_idx;
				int c_tag_group = cursor.tag_group;
				int d_tag_group = cursor.tag_group;
				uformat_viewer_pos c;
				int c_ypos;
				if (cursor_state == cursor_state_invisible) {
					c = top;
					c_ypos = 0;
				} else {
					c = cursor;
					c_ypos = cursor_ypos;
				}
				int nc = prev_line(&c, n);
				c_ypos -= nc;

				while (nc--) {
					c.sub->getline(c_line, sizeof c_line, c.line_id);
					int g = tag_count_groups(c_line);
					if (d_tag_group < g) c_tag_group=d_tag_group;
					int s;
					if (c_tag_group >= g) {
						c_tag_group = g-1;
						s = tag_count_selectable_tags_in_group(c_line, c_tag_group);
						if (s) {
							c_tag_idx = s-1;
							r = 1;
							break;
						}
					}
					s = tag_count_selectable_tags_in_group(c_line, c_tag_group);
					if (s) {
						if (c_tag_idx >= s) c_tag_idx = s-1;
						r=1;
						break;
					}
					if (!next_line(&c, 1)) break;
					c_ypos++;
				}
				if (r) {
					cursor = c;
					memcpy(cursor_line, c_line, sizeof cursor_line);
					cursorline_dirty();
					cursor_ypos = c_ypos;
					cursor.tag_idx = c_tag_idx;
					cursor.tag_group = c_tag_group;
					if (-cursor_ypos+n-nc-1 > 0) scroll_up(-cursor_ypos+n-nc-1);
					update_misc_info();
					update_visual_info();
					if (edit()) update_micropos();
				} else {
					if (cursor_state == cursor_state_invisible) cursor_ypos = -0x80000000;
					scroll_up(n);
				}
				// FIXME: wrong value
				return 1;
			}
		}
		case cursor_state_disabled:
//			scroll_up(n);
//			return n;
			break;
	}
	return 0;
}

int ht_uformat_viewer::cursor_down(int n)
{
	switch (cursor_state) {
		case cursor_state_invisible:
		case cursor_state_visible: {
			if (n == 1 && cursor_state == cursor_state_visible) {
				int r=0;
				uformat_viewer_pos c;
				c = cursor;
				char c_line[1024];
				int c_ypos=cursor_ypos;
				int c_tag_idx=cursor.tag_idx;
				int c_tag_group=cursor.tag_group;
				int d_tag_group=cursor.tag_group;
				int nls;	/* controls scrolling beyond end */

				while ((nls = next_line(&c, 1)) && c_ypos <= size.h-1) {
					c_ypos++;
					c.sub->getline(c_line, sizeof c_line, c.line_id);
					int g = tag_count_groups(c_line);
					if (d_tag_group < g) c_tag_group = d_tag_group;
					int s;
					if (c_tag_group >= g) {
						c_tag_group = g-1;
						s=tag_count_selectable_tags_in_group(c_line, c_tag_group);
						if (s) {
							c_tag_idx = s-1;
							r = 1;
							break;
						}
					}
					s = tag_count_selectable_tags_in_group(c_line, c_tag_group);
					if (s) {
						if (c_tag_idx >= s) c_tag_idx = s-1;
						r = 1;
						break;
					}
				}
				if (r) {
					cursor = c;
					memcpy(cursor_line, c_line, sizeof cursor_line);
					cursorline_dirty();
					cursor_ypos = c_ypos;
					cursor.tag_idx = c_tag_idx;
					cursor.tag_group = c_tag_group;
					if (cursor_ypos >= size.h) scroll_down(cursor_ypos-size.h+1);
					update_misc_info();
					update_visual_info();
					if (edit()) update_micropos();
				} else if (nls) scroll_down(1);
				return r;
			} else {
				int r=0;
				char c_line[1024];
				int c_tag_idx=cursor.tag_idx;
				int c_tag_group=cursor.tag_group;
				int d_tag_group=cursor.tag_group;
				uformat_viewer_pos c;
				int c_ypos;
				if (cursor_state==cursor_state_invisible) {
					c = top;
					c_ypos = next_line(&c, size.h-1);
				} else {
					c = cursor;
					c_ypos = cursor_ypos;
				}

				int nc=next_line(&c, n);
				int onc=c_ypos+nc-size.h+1;
				c_ypos+=nc;

				while (nc--) {
					c.sub->getline(c_line, sizeof c_line, c.line_id);
					int g=tag_count_groups(c_line);
					if (d_tag_group<g) c_tag_group=d_tag_group;
					int s;
					if (c_tag_group>=g) {
						c_tag_group=g-1;
						s=tag_count_selectable_tags_in_group(c_line, c_tag_group);
						if (s) {
							c_tag_idx=s-1;
							r=1;
							break;
						}
					} else {
						s=tag_count_selectable_tags_in_group(c_line, c_tag_group);
						if (s) {
							if (c_tag_idx>=s) c_tag_idx=s-1;
							r=1;
							break;
						}
					}
					if (nc) if (!prev_line(&c, 1)) break;
					c_ypos--;
				}
				if (r) {
					cursor = c;
					memcpy(cursor_line, c_line, sizeof cursor_line);
					cursorline_dirty();
					cursor_ypos=c_ypos;
					cursor.tag_idx=c_tag_idx;
					cursor.tag_group=c_tag_group;
					if (cursor_ypos-size.h+1>0) scroll_down(cursor_ypos-size.h+1);
					update_misc_info();
					update_visual_info();
					if (edit()) update_micropos();
				} else if (onc>0) {
					if (cursor_state==cursor_state_invisible) cursor_ypos=-0x80000000;
					scroll_down(onc);
				}
				// FIXME: wrong value
				return 1;
			}
		}
		case cursor_state_disabled:
//			scroll_down(n);
//			return n;
			break;
	}
	return 0;
}

int ht_uformat_viewer::cursor_home()
{
	cursor.tag_idx=0;
	if (edit()) cursor_tag_micropos=0;
	update_visual_info();
	update_misc_info();
	return 1;
}

int ht_uformat_viewer::cursor_end()
{
	cursorline_get();
	int c = tag_count_selectable_tags_in_group(cursor_line, cursor.tag_group);
	cursor.tag_idx = c-1;
	if (edit()) {
		char *e = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
		if (e) cursor_tag_micropos = tag_get_microsize(e) - 1;
	}
	update_visual_info();
	update_misc_info();
	return 1;
}

void ht_uformat_viewer::cursor_tab()
{
	cursor.tag_group++;
	adjust_cursor_group();
}

void ht_uformat_viewer::cursorline_dirty()
{
	isdirty_cursor_line = true;
}

void ht_uformat_viewer::cursorline_get()
{
	if (isdirty_cursor_line) {
		if (cursor.sub) cursor.sub->getline(cursor_line, sizeof cursor_line, cursor.line_id);
		isdirty_cursor_line = false;
	}
}

int ht_uformat_viewer::cursormicroedit_forward()
{
	cursorline_get();
	uformat_viewer_pos p;
	p = top;
	p.tag_group = cursor.tag_group;
	uint cursor_tag_bitidx = 0;

	cursorline_get();
	char *e=tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	if (e) {
		if (((byte*)e)[1] == HT_TAG_EDIT_BIT) {
			cursor_tag_bitidx = ((ht_tag_edit_bit*)e)->bitidx;
		}
	}

	bool cursor_found = false;
	char c_line[1024];
	for (int y=0; y < size.h+16; y++) {
		if (!p.sub->getline(c_line, sizeof c_line, p.line_id)) break;
		int c=tag_count_selectable_tags_in_group(c_line, p.tag_group);
		while (p.tag_idx<c) {
			char *t=tag_get_selectable_tag(c_line, p.tag_idx, p.tag_group);
			if (t && (tag_get_class(t) == tag_class_edit)) {
				if (cursor_found) {
					cursor_tag_micropos=0;
					set_cursor(p);
					cursorline_dirty();
					update_misc_info();
					update_visual_info();
					return 1;
				} else if (tag_get_offset(t) == cursor_tag_offset) {
					byte *tb = (byte*)t;
					if ( ( tb[1] == HT_TAG_EDIT_BIT &&
					 ((ht_tag_edit_bit*)t)->bitidx == cursor_tag_bitidx ) 
					|| tb[1] != HT_TAG_EDIT_BIT) {
						cursor_found = true;
						char *t = tag_get_selectable_tag(c_line, p.tag_idx, p.tag_group);
						int s = tag_get_microsize(t);
						if (cursor_tag_micropos+1 < s) {
							cursor_tag_micropos++;
							set_cursor(p);
							cursorline_dirty();
							update_misc_info();
							update_visual_info();
							return 1;
						}
					}
				}
			}
			p.tag_idx++;
		}
		p.tag_idx = 0;
		if (!next_line(&p, 1)) break;
	}
	if (cursor_right()) cursor_tag_micropos = 0;
	return 0;
}

int ht_uformat_viewer::cursormicro_forward()
{
	cursorline_get();
	char *t = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	int s = tag_get_microsize(t);
	if (cursor_tag_micropos+1 >= s) {
		if (cursor_right()) cursor_tag_micropos=0; else return 0;
	} else {
		cursor_tag_micropos++;
	}
	return 1;
}

int ht_uformat_viewer::cursormicro_backward()
{
	if (cursor_tag_micropos>0) {
		cursor_tag_micropos--;
	} else {
		if (cursor_left()) {
			cursorline_get();
			char *t=tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
			cursor_tag_micropos=tag_get_microsize(t)-1;
		} else {
			return 0;
		}
	}
	return 1;
}

void ht_uformat_viewer::draw()
{
	int sdown_count=0;
	if (!uf_initialized) HT_ERROR("complete_init() not called!");
restart:
	clear(getcolor(palidx_generic_body));
	if (!first_sub) return;
	uformat_viewer_pos p = top;
	uformat_viewer_pos sp;
	clear_viewer_pos(&sp);

	char line[1024];
	bool cursor_in_line;
	int cursor_found=-1;
	if (focused) hidecursor();
	if (!p.sub->getline(line, sizeof line, p.line_id)) {
		if (p.sub->closest_line_id(&p.line_id)) {
			top = p;
		} else return;
		if (!p.sub->getline(line, sizeof line, p.line_id)) return;
	}
	for (int y=0; y < size.h; y++) {
		if (y == cursor_ypos) sp = p;
		if (!p.sub->getline(line, sizeof line, p.line_id)) break;
/**/
/*		char test[128];
		tag_striptags(test+sprintf(test, "%08x,%08x: ", id1, id2), line);
		fprintf(stdout, "%s\n", test);
		fflush(stdout);*/
/**/
// FIXME: revision
		if (cursor_state == cursor_state_visible && p.sub == cursor.sub
		 && compeq_line_id(p.line_id, cursor.line_id)) {
			cursor_found = y;
			cursorline_dirty();
			update_misc_info();
			int c = tag_count_selectable_tags_in_group(line, cursor.tag_group);
			if (cursor.tag_idx >= c) cursor.tag_idx = c-1;
			if (cursor_tag_class == tag_class_edit && edit()) {
				char *t = tag_get_selectable_tag(line, cursor.tag_idx, cursor.tag_group);
				if (t) {
					int x = cursor_visual_xpos + tag_get_micropos(t, cursor_tag_micropos) - xscroll;
					if (focused && x >= 0) setcursor(x, y);
				}
			}
			cursor_in_line = true;
		} else cursor_in_line = false;
		print_tagstring(0, y, size.w, xscroll, line, cursor_in_line);
		if (xscroll > 0) buf->printChar(0, y, VCP(VC_GREEN, VC_TRANSPARENT), '<');
		if (!next_line(&p, 1)) break;
	}
	if (cursor_state == cursor_state_visible && cursor_found == -1) {
#if 1
		if (!sp.sub) sp = top;
		cursor = sp;
#else
		top = cursor;
#endif
		cursorline_dirty();
		update_misc_info();
		int c = tag_count_selectable_tags_in_group(line, cursor.tag_group);
		if (cursor.tag_idx > c-1) cursor.tag_idx=c-1;

		if (sdown_count++ > 0) goto restart;
	}
	if (cursor_found != -1) cursor_ypos = cursor_found;
//	buf_printf(1, 1, VCP(VC_GREEN, VC_BLACK), "%s", focused ? "focused" : "unfocused");
//	if (cursor_select) printf(20, 1, VCP(VC_GREEN, VC_BLACK), "start=%08x, l=%08x", cursor_select_start, cursor_select_cursor_length);
//	buf_printf(20, 6, 7, "csize.x=%2d, csize.y=%2d, csize.w=%2d, csize.h=%2d", csize.x, csize.y, csize.w, csize.h);
//	buf_printf(2, 3, 7, "size.x=%2d, size.y=%2d, size.w=%2d, size.h=%2d", size.x, size.y, size.w, size.h);
//	buf_printf(2, 4, 7, "vsize.x=%2d, vsize.y=%2d, vsize.w=%2d, vsize.h=%2d", vsize.x, vsize.y, vsize.w, vsize.h);
//	buf_printf(2, 5, 7, "bsize.x=%2d, bsize.y=%2d, bsize.w=%2d, bsize.h=%2d", buf->size.x, buf->size.y, buf->size.w, buf->size.h);
//	buf_printf(20, 7, 7, "size.x=%2d, size.y=%2d, size.w=%2d, size.h=%2d", group->group->size.x, group->group->size.y, group->group->size.w, group->group->size.h);
//	buf_printf(20, 8, 7, "vsize.x=%2d, vsize.y=%2d, vsize.w=%2d, vsize.h=%2d", group->group->vsize.x, group->group->vsize.y, group->group->vsize.w, group->group->vsize.h);
//	buf_printf(0, 7, 7, "vx=%2d, vl=%2d, ypos=%2d", cursor_visual_xpos, cursor_visual_length, cursor_ypos);
//	buf->printf(0, 2, 7, CP_DEVICE, "cursor_micropos=%d", cursor_tag_micropos);
//	buf_printf(0, 2, 7, "%x, %x, %x, %x, %x, c_tagidx", cursor.line_id.id1, cursor.line_id.id2, cursor.line_id.id3, cursor.line_id.id4, cursor.line_id.id5, cursor.tag_idx);
//	buf_printf(0, 3, 7, "(%c)", "vid"[cursor_state]);
//	buf_printf(0, 6, 7, "cursor.tag_idx=%d", cursor.tag_idx);
/*	if (cursor_tag_class==tag_class_edit) {
		buf_printf(30, 7, 7, "class=edit, addr=%08x", cursor_tag_offset);
	} else {
		buf_printf(30, 7, 7, "class=sel, id_low=%08x, id_high=%08x", 0,0);
	}*/
}

bool ht_uformat_viewer::edit()
{
	return (file && (file->getAccessMode() & IOAM_WRITE));
}

bool ht_uformat_viewer::edit_end()
{
	if (!edit()) {
		hidecursor();
		set_cursor(cursor);
#if 0
		uformat_viewer_pos p = cursor;
		if (find_first_tag(&p, size.h)) {
			set_cursor(p);
		} else {
//			if (cursor_state!=cursor_state_disabled) {
/* is that all we have to do ??? testing needed */
//				cursor_state=cursor_state_invisible;
//				assert(0);	/* FIXME: not yet implemented */
//			}
		}
#endif
		cursorline_dirty();
		adjust_cursor_idx();
		update_visual_info();
		update_misc_info();
		dirtyview();
		return true;
	}
	return false;
}

bool ht_uformat_viewer::edit_input(byte b)
{
	cursorline_get();
	char *t = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	switch (t[1]) {
		case HT_TAG_EDIT_BYTE:
		case HT_TAG_EDIT_WORD_LE:
		case HT_TAG_EDIT_DWORD_LE:
		case HT_TAG_EDIT_QWORD_LE:
		case HT_TAG_EDIT_WORD_BE:
		case HT_TAG_EDIT_DWORD_BE:
		case HT_TAG_EDIT_QWORD_BE: {
			int nibval = edit_input_c2h(b);
			if (nibval == -1) break;
			
			int size = 0;
			bool bigendian = true;
			switch (t[1]) {
				case HT_TAG_EDIT_BYTE: size = 1; break;
				case HT_TAG_EDIT_WORD_LE: size = 2; bigendian=false; break;
				case HT_TAG_EDIT_DWORD_LE: size = 4; bigendian=false; break;
				case HT_TAG_EDIT_QWORD_LE: size = 8; bigendian=false; break;
				case HT_TAG_EDIT_WORD_BE: size = 2; bigendian=true; break;
				case HT_TAG_EDIT_DWORD_BE: size = 4; bigendian=true; break;
				case HT_TAG_EDIT_QWORD_BE: size = 8; bigendian=true; break;
			}
			
			uint shift = 4 - (cursor_tag_micropos&1)*4;
			uint m = ~(0xf << shift);
			uint o = nibval << shift;

			int b;
			if (bigendian) {
				b = cursor_tag_micropos/2;
			} else {
				b = size - cursor_tag_micropos/2 - 1;
			}
			
			byte buf;
			pread(cursor_tag_offset + b, &buf, 1);
			buf &= m;
			buf |= o;
			pwrite(cursor_tag_offset + b, &buf, 1);
			cursormicroedit_forward();
			return true;
		}
		case HT_TAG_EDIT_CHAR: {
			if (b >= 32 && b < 128) {
				pwrite(cursor_tag_offset, &b, 1);
				cursormicroedit_forward();
				return true;
			}
			break;
		}
		case HT_TAG_EDIT_BIT: {
			if (b == '1' || b == '0' || b == ' ') {
				byte d;
				cursorline_get();
				char *t = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
				int shift = ((ht_tag_edit_bit*)t)->bitidx;
				uint32 mask = 1 << (shift%8);
				int op = shift/8;
				pread(cursor_tag_offset+op, &d, 1);
				switch (b) {
				case '0':
					d &= ~mask;
					break;
				case '1':
					d |= mask;
					break;
				case K_Space:
					d ^= mask;
					break;
				}
				pwrite(cursor_tag_offset+op, &d, 1);
				cursormicroedit_forward();
				return true;
			}
			break;
		}
		case HT_TAG_EDIT_TIME_BE:
		case HT_TAG_EDIT_TIME_LE: {
			uint32 d;
			int h = edit_input_c2d(b);
					
			byte buf[4];
			if (pread(cursor_tag_offset, &buf, 4)==4 && h!=-1) {
				if (t[1] == HT_TAG_EDIT_TIME_LE) {
					d=(buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
				} else {
					d=(buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
				}
				time_t tt = d;
				tm *ti = gmtime(&tt);
				tm q = *ti;
				int k;
				bool worked = false;
				#define DEC_MASK(value, mask) ((value) - (value) / (mask) % 10 * (mask))
				switch (cursor_tag_micropos) {
				case 0:
					k = q.tm_hour % 10 + h * 10;
					if (k < 24) {
						q.tm_hour = k;
						worked = true;
					}
					break;
				case 1:
					k = q.tm_hour - q.tm_hour % 10 + h;
					if (k < 24) {
						q.tm_hour = k;
						worked = true;
					}
					break;
				case 2:
					k = q.tm_min % 10 + h * 10;
					if (k < 60) {
						q.tm_min = k;
						worked = true;
					}
					break;
				case 3:
					k = q.tm_min - q.tm_min % 10 + h;
					if (k < 60) {
						q.tm_min = k;
						worked = true;
					}
					break;
				case 4:
					k = q.tm_sec % 10 + h * 10;
					if (k < 60) {
						q.tm_sec = k;
						worked = true;
					}
					break;
				case 5:
					k = q.tm_sec - q.tm_sec % 10 + h;
					if (k < 60) {
						q.tm_sec = k;
						worked = true;
					}
					break;
				case 6:
					k = (q.tm_mday % 10) + h * 10;
					if (k <= 31) {
						q.tm_mday = k;
						worked = true;
					}
					break;
				case 7:
					k = q.tm_mday - q.tm_mday % 10 + h;
					if (k <= 31) {
						q.tm_mday = k;
						worked = true;
					}
					break;
				case 8:
					k = (q.tm_mon+1) % 10 + h * 10;
					if (k <= 12) {
						q.tm_mon = k-1;
						worked = true;
					}
					break;
				case 9:
					k = q.tm_mon - q.tm_mon % 10 + h;
					if (k <= 12) {
						q.tm_mon = k-1;
						worked = true;
					}
					break;
				case 10:
					k = DEC_MASK(q.tm_year, 1000) + h * 1000;
					if ((k+1900 >= 1970 ) && (k+1900 < 2106)) {
						q.tm_year = k;
						worked = true;
					}
					break;
				case 11:
					k = DEC_MASK(q.tm_year, 100) + h * 100;
					if ((k+1900 >= 1970 ) && (k+1900 < 2106)) {
						q.tm_year = k;
						worked = true;
					}
					break;
				case 12:
					k = DEC_MASK(q.tm_year, 10) + h * 10;
					if ((k+1900 >= 1970 ) && (k+1900 < 2106)) {
						q.tm_year = k;
						worked = true;
					}
					break;
				case 13:
					k = DEC_MASK(q.tm_year, 1) + h * 1;
					if ((k+1900 >= 1970 ) && (k+1900 < 2106)) {
						q.tm_year = k;
						worked = true;
					}
					break;
				}
				/* FIXME: big bad hack... */
				if (sizeof(uint32) == sizeof(time_t))
				if (worked) {
					/* FIXME: !!! */
					uint32 tz;
					time_t l = time(NULL);
					time_t g = l;
					struct tm *gtm = gmtime(&g);
					g = mktime(gtm);
					tz = (uint32)difftime(g, l);
					time_t tt = mktime(&q);
					d = tt;
					d -= tz;
					if (t[1] == HT_TAG_EDIT_TIME_LE) {
						buf[0] = d>>0;
						buf[1] = d>>8;
						buf[2] = d>>16;
						buf[3] = d>>24;
					} else {
						buf[3] = d>>0;
						buf[2] = d>>8;
						buf[1] = d>>16;
						buf[0] = d>>24;
					}
					pwrite(cursor_tag_offset, &buf, 4);
					cursormicroedit_forward();
					return true;
				}
			}
		}
	}
	return false;
}

int ht_uformat_viewer::edit_input_c2h(byte b)
{
	int h = -1;
	if (b >= '0' && b <= '9') {
		h = b-'0';
	} else if (b >= 'a' && b <= 'f') {
		h = b-'a'+10;
	} else if (b >= 'A' && b <= 'F') {
		h = b-'A'+10;
	}
	return h;
}

int ht_uformat_viewer::edit_input_c2d(byte b)
{
	int h = -1;
	if (b >= '0' && b <= '9') {
		h = b-'0';
	}
	return h;
}

void ht_uformat_viewer::edit_input_correctpos()
{
	uformat_viewer_pos p = top;

	/* try finding edited tag by its offset */
	char c_line[1024];
	int g=cursor.tag_group;
	for (int y=0; y < size.h+10; y++) {
		if (!p.sub->getline(c_line, sizeof c_line, p.line_id)) break;
		int c = tag_count_selectable_tags_in_group(c_line, cursor.tag_group);
		for (int i=0; i < c; i++) {
			char *t = tag_get_selectable_tag(c_line, i, cursor.tag_group);
			if (tag_get_class(t) == tag_class_edit && tag_get_offset(t) == cursor_tag_offset) {
				set_cursor(p);
				cursorline_dirty();
				cursor.tag_idx = i;
				cursor.tag_group = g;
				update_misc_info();
				update_visual_info();
				return;
			}
		}
		if (!next_line(&p, 1)) break;
	}

	/* try finding edited tag by cursor_ypos */
	p = top;
	next_line(&p, cursor_ypos);
	int ci = cursor.tag_idx;
	set_cursor(p);
	cursorline_dirty();
	cursorline_get();
	int x = tag_count_selectable_tags(cursor_line);
	if (ci >= x) ci = x-1;
	cursor.tag_idx = ci;
	update_misc_info();
	update_visual_info();
}

bool ht_uformat_viewer::edit_start()
{
	if (edit() && cursor.sub) {
		cursor_tag_micropos=0;
		cursorline_dirty();
		adjust_cursor_idx();
		update_visual_info();
		update_misc_info();
		dirtyview();
		return true;
	}
	return false;
}

bool ht_uformat_viewer::edit_update()
{
	if (edit()) {
		file->cntl(FCNTL_MODS_FLUSH);
		dirtyview();
		return true;
	}
	return false;
}

bool ht_uformat_viewer::find_first_tag(uformat_viewer_pos *p, int limit)
{
	char line[1024];
	int i=0;
	uformat_viewer_pos q = *p;
	if (!q.sub) return false;
	do {
		q.sub->getline(line, sizeof line, q.line_id);
		if (tag_get_selectable_tag(line, 0, 0)) {
			*p = q;
			p->tag_idx = 0;
			p->tag_group = 0;
			return true;
		}
	} while ((next_line(&q, 1)) && (i++<limit));
	return false;
}

bool ht_uformat_viewer::find_first_edit_tag_with_offset(uformat_viewer_pos *p, int limit, FileOfs offset)
{
	char line[1024];
	int i=0;
	uformat_viewer_pos q = *p;
	if (!q.sub) return false;
	do {
		q.sub->getline(line, sizeof line, q.line_id);
		int c = tag_count_selectable_tags(line);
		char *t = line;
		for (int j=0; j<c; j++) {
			t = tag_get_selectable_tag(t, 0, -1);
			if (tag_get_class(t) == tag_class_edit && tag_get_offset(t) == offset) {
				*p = q;
				p->tag_idx = j;
				p->tag_group = 0;
				/* FIXME: what about groups ??? */
				return true;
			}
			t += tag_get_len(t);
		}
	} while (next_line(&q, 1) && i++ < limit);
	return false;
}

void ht_uformat_viewer::focus_cursor()
{
// why ?
/*	if (cursor_state==cursor_state_invisible) {
		center_view(cursor_sub, cursor_id1, cursor_id2);
		cursor_state=cursor_state_visible;
	}*/
	update_visual_info();
	if (cursor_visual_xpos-xscroll <= 1) {
		if (cursor_visual_xpos>=1) xscroll = cursor_visual_xpos-1; else
			xscroll = 0;
	} else if (cursor_visual_xpos+cursor_visual_length-xscroll > size.w-1) {
		xscroll = cursor_visual_xpos+cursor_visual_length-size.w+1;
	}
}

const char *ht_uformat_viewer::func(uint i, bool execute)
{
	switch (i) {
		case 2:
			if (caps & VC_EDIT) {
				if (edit()) {
					if (execute) app->sendmsg(cmd_file_save);
//					if (execute) edit_update();
					return "save";
				} else {
					return "~save";
				}
			}
			break;
/*		case 3:
			if (caps & VC_REPLACE) {
				if (edit()) {
					if (execute) sendmsg(cmd_file_replace);
					return "replace";
				} else {
					return "~replace";
				}
			}
			break;*/
		case 4:
			if (caps & VC_EDIT) {
				if (edit()) {
					if (execute) {
						FileOfs start = 0;
						FileOfs size = file->getSize();
						bool isdirty = false;						
						file->cntl(FCNTL_MODS_IS_DIRTY, start, size, &isdirty);
						char q[1024];
						String fn;
						if (!file->getFilename(fn).isEmpty()) {
							ht_snprintf(q, sizeof q,
								"file %y has been modified, apply changes?",
								&fn);
						} else {
							ht_snprintf(q, sizeof q,
								"untitled file has been modified, apply changes?");
						}
						if (isdirty && confirmbox(q) == button_yes) {
							file->cntl(FCNTL_MODS_FLUSH);
						} else {
							file->cntl(FCNTL_MODS_INVD);
						}
						if (size != file->getSize()) {
							htmsg m;
							m.msg = msg_filesize_changed;
							m.type = mt_broadcast;
							sendmsg(&m);
						}
						baseview->sendmsg(cmd_view_mode_i, file, NULL);
					}
					return "view";
				} else {
					if (execute) baseview->sendmsg(cmd_edit_mode_i, file, NULL);
					return "edit";
				}
			}
			break;
		case 5:
			if (caps & VC_GOTO) {
				if (execute) sendmsg(cmd_file_goto);
				return "goto";
			}
			break;
		case 7:
			if (caps & VC_SEARCH) {
				if (execute) sendmsg(cmd_file_search);
				return "search";
			}
			break;
		case 8:
			if (caps & VC_RESIZE) {
				if (edit()) {
					if (execute) sendmsg(cmd_file_resize);
					return "resize";
				} else {
					return "~resize";
				}
			}
			break;
		case 9: {
			if (execute) {
				FileOfs o;
				if (get_current_real_offset(&o)) {
					char title[128];
					ht_snprintf(title, sizeof title, "view offset %08qx in...", o);
					ht_view *v = ((ht_app*)app)->popup_view_list(title);
					if (v) {
						htmsg m;
						m.msg = msg_goto_offset;
						m.type = mt_empty;
						m.data1.q = o;
						v->sendmsg(&m);
						if (m.msg == msg_empty) {
							vstate_save();
							app->focus(v);
						} else {
							errorbox("offset %08qx is not supported/invalid in '%s'", o, v->desc);
						}
					}
				}
			}
			return "viewin...";
		}
	}
	return ht_format_viewer::func(i, execute);
}

vcp ht_uformat_viewer::getcolor_tag(uint pal_index)
{
	return getcolorv(&tagpal, pal_index);
}

bool ht_uformat_viewer::get_current_offset(FileOfs *offset)
{
	if (cursor_state != cursor_state_disabled && cursor_tag_class == tag_class_edit) {
		*offset = cursor_tag_offset;
		return true;
	}
	return false;
}

bool ht_uformat_viewer::get_current_pos(viewer_pos *pos)
{
	clear_viewer_pos(pos);
	pos->u = cursor;
	return true;
}

bool ht_uformat_viewer::get_current_tag(char **tag)
{
	cursorline_get();
	char *e = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	if (e) {
		*tag = e;
		return true;
	}
	return false;
}

bool ht_uformat_viewer::get_current_tag_size(uint32 *size)
{
	if (cursor_tag_class == tag_class_edit) {
		cursorline_get();
		char *e = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
		if (e) {
			*size = tag_get_size(e);
			return true;
		}
	}
	return false;
}

void ht_uformat_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_get_pindicator:
			msg->data1.integer = get_pindicator_str((char*)msg->data2.ptr, msg->data1.integer);
			clearmsg(msg);
			return;
		case msg_get_scrollinfo:
			switch (msg->data1.integer) {
				case gsi_hscrollbar: {
					gsi_scrollbar_t *p = (gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_hscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					clearmsg(msg);
					return;
				}
				case gsi_vscrollbar: {
					gsi_scrollbar_t *p = (gsi_scrollbar_t*)msg->data2.ptr;
					if (!get_vscrollbar_pos(&p->pstart, &p->psize)) {
						p->pstart = 0;
						p->psize = 100;
					}
					clearmsg(msg);
					return;
				}
			}
			break;
		case msg_complete_init:
			complete_init();
			clearmsg(msg);
			return;
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_Meta_S:
					if (cursor_select) {
					    select_mode_off();
					} else {
					    select_mode_on();
					}
					clearmsg(msg);
					dirtyview();
					return;
				case K_Up:
					select_mode_pre();
					cursor_up(1);
					select_mode_post(0);
					clearmsg(msg);
					dirtyview();
					return;
				case K_Down:
					select_mode_pre();
					cursor_down(1);
					select_mode_post(0);
					clearmsg(msg);
					dirtyview();
					return;
				case K_PageUp:
					select_mode_pre();
					cursor_up(size.h);
					select_mode_post(0);
					clearmsg(msg);
					dirtyview();
					return;
				case K_PageDown:
					select_mode_pre();
					cursor_down(size.h);
					select_mode_post(0);
					clearmsg(msg);
					dirtyview();
					return;
				case K_Left:
					if (cursor_state!=cursor_state_disabled) {
						select_mode_pre();
						if (edit()) cursormicro_backward(); else cursor_left();
						select_mode_post(0);
						focus_cursor();
						clearmsg(msg);
						dirtyview();
						return;
					}
					break;
				case K_Right:
					if (cursor_state!=cursor_state_disabled) {
						int r;
						select_mode_pre();
						if (edit()) r=cursormicro_forward(); else r=cursor_right();
						select_mode_post(!r);
						focus_cursor();
						clearmsg(msg);
						dirtyview();
						return;
					}
					break;
				case K_Return:
					switch (cursor_tag_class) {
						case tag_class_sel:
							ref();
							break;
						case tag_class_edit:
							if (edit()) {
								edit_input(msg->data1.integer);
							}
							break;
					}
					dirtyview();
					clearmsg(msg);
					return;
				case K_Backspace: {
					FileOfs f;
					if (edit() 
					 && cursor_tag_class == tag_class_edit
					 && get_current_offset(&f)) {
						file->cntl(FCNTL_MODS_CLEAR_DIRTY_RANGE, f, 1ULL);
						cursor_left();
						focus_cursor();
						dirtyview();
						clearmsg(msg);
					}                         
					return;
				}
				case K_Tab: {
					int c = cursor.tag_group;
					cursor_tab();
					if (cursor.tag_group != c) {
						focus_cursor();
						update_visual_info();
						dirtyview();
						clearmsg(msg);
						return;
					}
					break;
				}
				case K_Home:
					if (cursor_state == cursor_state_visible) {
						select_mode_pre();
						cursor_home();
						select_mode_post(0);
						focus_cursor();
						clearmsg(msg);
						dirtyview();
						return;
					}
					break;
				case K_End:
					if (cursor_state == cursor_state_visible) {
						select_mode_pre();
						cursor_end();
						select_mode_post(0);
						focus_cursor();
						clearmsg(msg);
						dirtyview();
						return;
					}
					break;
				case K_Control_Left:
					xscroll -= 2;
					if (xscroll < 0) xscroll = 0;
					dirtyview();
					clearmsg(msg);
					return;
				case K_Control_Right:
					xscroll += 2;
					dirtyview();
					clearmsg(msg);
					return;
				case K_Control_PageUp: {
					top.sub = first_sub;
					if (top.sub) {
						vstate_save();
						select_mode_pre();
						top.sub->first_line_id(&top.line_id);

						uformat_viewer_pos p = top;
						p.tag_group = cursor.tag_group;
						if (find_first_tag(&p, size.h)) {
							set_cursor(p);
						} else {
							cursor_ypos = 0x7fffffff;
							update_misc_info();
							update_visual_info();
							check_cursor_visibility();
						}
						select_mode_post(0);
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Control_PageDown: {
					top.sub = last_sub;
					if (top.sub) {
						vstate_save();
						select_mode_pre();
						top.sub->last_line_id(&top.line_id);

						uformat_viewer_pos p = top;
						p.tag_group = cursor.tag_group;
						if (find_first_tag(&p, 1)) {
							set_cursor(p);
						} else {
							cursor_ypos = 0x7fffffff;
							update_misc_info();
							update_visual_info();
							check_cursor_visibility();
						}
						
						cursor_up(size.h);
						cursor_down(size.h);
						
						select_mode_post(0);
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_Up: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class == tag_class_edit)
						s = cursor_tag_offset;
					else
						s = (sel_end > sel_start) ? sel_end : -1ULL;
					e = get_current_tag_size(&ts) ? s+ts : -1ULL;
					cursor_up(1);
					if (s != -1ULL) {
						if (cursor_tag_class == tag_class_edit) {
							pselect_add(s, cursor_tag_offset);
						} else if (cursor_tag_class == tag_class_sel 
						       && e != -1ULL) {
							pselect_add(s, e);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_Down: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class == tag_class_edit)
						s = cursor_tag_offset;
					else
						s = (sel_end > sel_start) ? sel_end : -1ULL;
					e = get_current_tag_size(&ts) ? s+ts : -1ULL;
					cursor_down(1);
					if (s != -1ULL) {
						if (cursor_tag_class == tag_class_edit) {
							pselect_add(s, cursor_tag_offset);
						} else if (cursor_tag_class == tag_class_sel
						        && e != -1ULL) {
							pselect_add(s, e);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_Left: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class==tag_class_edit)
						s = cursor_tag_offset;
					else
						s = sel_end > sel_start ? sel_end : -1ULL;
					e = get_current_tag_size(&ts) ? s+ts : -1ULL;
					int r;
					if (edit()) r = cursormicro_backward(); else r = cursor_left();
					if (s != -1ULL && r) {
						if (cursor_tag_class == tag_class_edit) {
							pselect_add(s, cursor_tag_offset);
						} else if (cursor_tag_class == tag_class_sel && e != -1ULL) {
							pselect_add(s, e);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_Right: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class == tag_class_edit)
						s = cursor_tag_offset;
					else
						s = (sel_end > sel_start) ? sel_end : -1ULL;
					e=get_current_tag_size(&ts) ? s+ts : -1ULL;
					int r;
					if (edit()) r = cursormicro_forward(); else r = cursor_right();
					if (s != -1ULL) {
						if (r) {
							if (cursor_tag_class == tag_class_edit) {
								pselect_add(s, cursor_tag_offset);
							} else if (cursor_tag_class == tag_class_sel && e != -1ULL) {
								pselect_add(s, e);
							}
						} else {
							if (cursor_tag_class == tag_class_edit && sel_end != e) {
								pselect_add(s, e);
							}
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_PageUp: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class == tag_class_edit)
						s = cursor_tag_offset;
					else
						s = (sel_end > sel_start) ? sel_end : -1ULL;
					e = get_current_tag_size(&ts) ? s+ts : -1ULL;
					cursor_up(size.h);
					if (s != -1ULL) {
						if (cursor_tag_class == tag_class_edit) {
							pselect_add(s, cursor_tag_offset);
						} else if (cursor_tag_class == tag_class_sel && e != -1ULL) {
							pselect_add(s, e);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_PageDown: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class==tag_class_edit)
						s=cursor_tag_offset;
					else
						s=(sel_end>sel_start) ? sel_end : -1ULL;
					e=get_current_tag_size(&ts) ? s+ts : -1ULL;
					cursor_down(size.h);
					if (s != -1ULL) {
						if (cursor_tag_class==tag_class_edit) {
							pselect_add(s, cursor_tag_offset);
						} else if (cursor_tag_class == tag_class_sel && e != -1ULL) {
							pselect_add(s, e);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_Home: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class==tag_class_edit)
						s=cursor_tag_offset;
					else
						s=(sel_end>sel_start) ? sel_end : -1ULL;
					e=get_current_tag_size(&ts) ? s+ts : -1ULL;
					int r=cursor_home();
					if (s != -1ULL && r) {
						if (cursor_tag_class == tag_class_edit) {
							pselect_add(cursor_tag_offset, s);
						} else if (cursor_tag_class == tag_class_sel && e != -1ULL) {
							pselect_add(e, s);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Shift_End: {
					focus_cursor();
					FileOfs s, e;
					uint32 ts;
					if (cursor_tag_class==tag_class_edit)
						s=cursor_tag_offset;
					else
						s=(sel_end>sel_start) ? sel_end : -1ULL;
					e=get_current_tag_size(&ts) ? s+ts : -1ULL;
					int r=cursor_end();
					if (s != -1ULL && r){
						if (cursor_tag_class == tag_class_edit) {
							pselect_add(s, cursor_tag_offset);
						} else if (cursor_tag_class == tag_class_sel && e != -1ULL) {
							pselect_add(s, e);
						}
					}
					clearmsg(msg);
					dirtyview();
					return;
				}
				case K_Control_F:
					if (caps & VC_SEARCH) {
						sendmsg(cmd_file_search);
						dirtyview();
						clearmsg(msg);
						return;
					}
					break;
				case K_Control_L:
				case K_Shift_F7:
					if (caps & VC_SEARCH) {
						if (!continue_search()) {
							if (last_search_request) {
								infobox("no further matches");
							} else {
								infobox("you must 'search' first!");
							}
						}
						dirtyview();
						clearmsg(msg);
						return;
					}
					break;
				case K_Meta_C:
				case K_Control_Insert:
					sendmsg(cmd_edit_copy);
					dirtyview();
					clearmsg(msg);
					return;
				case K_Meta_V:
				case K_Shift_Insert:
					sendmsg(cmd_edit_paste);
					dirtyview();
					clearmsg(msg);
					return;
				case K_Escape:
					if (caps & VC_EDIT) {
						baseview->sendmsg(cmd_view_mode_i, file, NULL);
						return;
					}						
					break;
				default: {
					if (((uint32)msg->data1.integer<=255) && (edit())) {
						if (cursor_tag_class==tag_class_edit) {
							focus_cursor();
							if (edit_input(msg->data1.integer)) {
								dirtyview();
								clearmsg(msg);
								return;
							}
						}
					}
				}
			}
			break;
		case cmd_edit_copy:
			if (sel_end > sel_start) {
				char dsc[1024];
				String fn;
				ht_snprintf(dsc, sizeof dsc, "%y::%s", &file->getFilename(fn), desc);
				clipboard_copy(dsc, file, sel_start, sel_end-sel_start);
			}
			clearmsg(msg);
			return;
		case cmd_edit_paste:
			FileOfs ofs;
			if (get_current_offset(&ofs)) {
				baseview->sendmsg(cmd_edit_mode_i, file, NULL);
				if (file->getAccessMode() & IOAM_WRITE) {
					FileOfs s = clipboard_paste(file, ofs);
					if (s) {
						pselect_set(ofs, ofs+s);
						sendmsg(cmd_edit_mode_i);
					}
				}
			}
			dirtyview();
			clearmsg(msg);
			return;
		case cmd_edit_mode:
			edit_start();
			dirtyview();
			return;
		case cmd_view_mode:
			edit_end();
			dirtyview();
			return;
		case cmd_file_goto: {
			char addrstr[1024];
			addrstr[0] = 0;
			while (address_input("goto", addrstr, sizeof addrstr, HISTATOM_GOTO) != button_cancel) {
				if (addrstr[0]) {
					viewer_pos pos;
					globalerror[0] = 0;
					if (string_to_pos(addrstr, &pos) && goto_pos(pos, this)) {
						focus_cursor();
						break;
					}
					if (globalerror[0]) {
						infobox("error: %s\nin '%s'", globalerror, addrstr);
					} else {
						infobox("invalid address: '%s'", addrstr);
					}
				}
			}
			clearmsg(msg);
			dirtyview();
			return;
		}
		case cmd_file_search: {
			viewer_pos start_pos, end_pos;
			get_current_pos(&start_pos);
			clear_viewer_pos(&end_pos);
			end_pos.u.sub = last_sub;
			end_pos.u.tag_idx = -1;
			last_sub->last_line_id(&end_pos.u.line_id);
			ht_search_request *request = search_dialog(this, search_caps, &start_pos, &end_pos);
			ht_search_result *result = NULL;
			if (request) {
				switch (request->search_class) {
					case SC_PHYSICAL: {
						try {
							FileOfs start, end;
							if (pos_to_offset(start_pos, &start)) {
								if (!pos_to_offset(end_pos, &end)) {
									end = FileOfs(-1);
								}
								result = psearch(request, start, end);
							}
						} catch (const Exception &e) {
							errorbox("error: %y", &e);
						}
						break;
					}
					case SC_VISUAL: {
						try {
							result = vsearch(request, start_pos, end_pos);
						} catch (const Exception &e) {
							errorbox("error: %y", &e);
						}
						break;
					}
				}
				delete request;
				if (result) {
					// FIXME: !!!!!!!!
					if (!show_search_result(result)) infobox("couldn't display result (internal error)");
					delete result;
				} else infobox("not found");
			}
			clearmsg(msg);
			dirtyview();
			return;
		}
		case cmd_file_replace: {
			sendmsg(cmd_edit_mode_i);
			if (edit()) {
				bool cancel;
				FileOfs s = get_file()->getSize();
				uint repls = replace_dialog(this, search_caps, &cancel);
				if (repls) {
					if (s != get_file()->getSize()) {
						sendmsg(msg_filesize_changed);
					}
					infobox("%d replacement(s) made", repls);
				}
			}
			clearmsg(msg);
			dirtyview();
			return;
		}
		case msg_filesize_changed: {
			htmsg m;
			m.msg = msg_filesize_changed;
			m.type = mt_broadcast;
			sendsubmsg(&m);
			break;
		}
		case cmd_file_save: {
			if (edit()) edit_update();
			clearmsg(msg);
			return;
		}
		case cmd_file_resize: {
			FileOfs o = 0;
			if (get_current_offset(&o)) {
				uint32 s;
				if (get_current_tag_size(&s)) {
					o += s;
				}
			}

			if (!o) o = clipboard_getsize();

			FileOfs s = file->getSize();

			char buf[32];
			ht_snprintf(buf, sizeof buf, "%qd", o);
			if (inputbox("resize", "new file size", buf, sizeof buf, 0)==button_ok) {
				eval_scalar r;
				if (eval(&r, buf, NULL, NULL, NULL)) {
					eval_int i;
					scalar_context_int(&r, &i);
					scalar_destroy(&r);
					o = i.value;
					if (o < s) {
						/* truncate */
						htmsg m;

						m.msg = cmd_file_truncate;
						m.type = mt_broadcast;
						m.data1.ptr = file;
						m.data2.q = o;
						baseview->sendmsg(&m);

						m.msg = msg_filesize_changed;
						m.type = mt_broadcast;
						sendsubmsg(&m);
						sendmsg(&m);
					} else if (o>s) {
						/* extend */
						htmsg m;
			
						m.msg = cmd_file_extend;
						m.type = mt_broadcast;
						m.data1.ptr = file;
						m.data2.q = o;
						baseview->sendmsg(&m);

						m.msg = msg_filesize_changed;
						m.type = mt_broadcast;
						sendsubmsg(&m);
						sendmsg(&m);
					}
				} else {
					const char *errmsg="?";
					int errpos=0;
					get_eval_error(&errmsg, &errpos);
					errorbox("eval error: %s at %d\n in '%s'", errmsg, errpos, buf);
				}
				update_misc_info();
			}

			clearmsg(msg);
			dirtyview();
			return;
		}
		case cmd_file_blockop: {
			if (sel_end>sel_start) {
				blockop_dialog(this, sel_start, sel_end);
			} else {
				FileOfs o = 0;
				get_current_offset(&o);
				blockop_dialog(this, o, file->getSize());
			}
			clearmsg(msg);
			dirtyview();
			return;
		}
	}
	ht_format_viewer::handlemsg(msg);
}

void ht_uformat_viewer::insertsub(ht_sub *sub)
{
	if (last_sub) last_sub->next = sub;
	sub->uformat_viewer = this;
	sub->prev = last_sub;
	last_sub = sub;
	if (!first_sub) first_sub = sub;
}

bool ht_uformat_viewer::goto_offset(FileOfs offset, bool save_vstate)
{
	uformat_viewer_pos p;
	p.sub = first_sub;
	p.tag_group = cursor.tag_group;
	while (p.sub) {
		if (p.sub->convert_ofs_to_id(offset, &p.line_id)) {
			if (save_vstate) vstate_save();
			switch (cursor_state) {
			case cursor_state_visible: {
				select_mode_pre();
				/* FIXME: magic 42 */
				if (!find_first_edit_tag_with_offset(&p, 42, offset)
					&& !find_first_tag(&p, size.h)) break;
				bool r = set_cursor(p);
				select_mode_post(false);
				return r;
			}
			case cursor_state_invisible:
				select_mode_pre();
				p.tag_group = -1;
				p.tag_idx = -1;
				bool r = set_cursor(p);
				select_mode_post(false);
				return r;
			}
			break;
		}
		p.sub = p.sub->next;
	}
	return false;
}

bool ht_uformat_viewer::goto_pos(viewer_pos pos, bool save_vstate)
{
	if (save_vstate) vstate_save();
	select_mode_pre();
	set_cursor(pos.u);
	select_mode_post(false);
	return true;
}

bool ht_uformat_viewer::next_logical_pos(viewer_pos pos, viewer_pos *npos)
{
	if (next_line(&pos.u, 1) == 1) {
		*npos = pos;
		return true;
	}
	return false;
}

bool ht_uformat_viewer::next_logical_offset(FileOfs ofs, FileOfs *nofs)
{
	*nofs = ofs + 1;
	return true;
}

int ht_uformat_viewer::next_line(uformat_viewer_pos *p, int n)
{
	int n0 = n;
	while (n) {
		if (n -= p->sub->next_line_id(&p->line_id, n)) {
			if (!p->sub->next) return n0 - n;
			p->sub = p->sub->next;
			p->sub->first_line_id(&p->line_id);
			p->tag_idx = -1;
			n--;
		}
	}
	return n0 - n;
}

int ht_uformat_viewer::prev_line(uformat_viewer_pos *p, int n)
{
	int n0 = n;
	while (n){
		if (n -= p->sub->prev_line_id(&p->line_id, n)) {
			if (!p->sub->prev) return n0 - n;
			p->sub = p->sub->prev;
			p->sub->last_line_id(&p->line_id);
			p->tag_idx = -1;
			n--;
		}
	}
	return n0 - n;
}

vcp ht_uformat_viewer::get_tag_color_edit(FileOfs tag_offset, uint size, bool atcursoroffset, bool iscursor)
{
	vcp tag_color = getcolor_tag(palidx_tags_edit_tag);
	bool isdirty = false;
	FileOfs fsize = size;
	file->cntl(FCNTL_MODS_IS_DIRTY, tag_offset, fsize, &isdirty);
	if (isdirty) tag_color = mixColors(tag_color, getcolor_tag(palidx_tags_edit_tag_modified));
	if (tag_offset >= sel_start && tag_offset < sel_end) tag_color = mixColors(tag_color, getcolor_tag(palidx_tags_edit_tag_selected));
	if (iscursor) {
		int coloridx;
		if (atcursoroffset) {
			if (edit()) {
				coloridx = palidx_tags_edit_tag_cursor_edit;
			} else {
				coloridx = palidx_tags_edit_tag_cursor_select;
			}
		} else {
			coloridx = palidx_tags_edit_tag_cursor_unfocused;
		}
		tag_color = mixColors(tag_color, getcolor_tag(coloridx));
	}
	return tag_color;
}

void ht_uformat_viewer::render_tagstring_desc(const char **string, int *length, vcp *tc, char *tag, uint size, bool bigendian, bool is_cursor)
{
	ID id;
	vcp tag_color = getcolor_tag(palidx_tags_sel_tag);
	if (is_cursor) tag_color = getcolor_tag(palidx_tags_sel_tag_cursor_focused);
	*string = "?";
	*length = 1;
	*tc = tag_color;
	if (tag_get_desc_id(tag, &id)) {
		int_hash *tbl;
		if ((tbl = (int_hash*)getAtomValue(id))) {
			const char *str;
			uint64 q = 0;
			FileOfs tag_offset = tag_get_offset(tag);
			byte buf[8];

			if (pread(tag_offset, buf, size) == size) {
				switch (size) {
				case 1:
					q = buf[0];
					break;
				case 2:
					if (bigendian) {
						q = (buf[0]<<8) | buf[1];
					} else {
						q = (buf[1]<<8) | buf[0];
					}
					break;
				case 4:
					if (bigendian) {
						q = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
					} else {
						q = (buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
					}
					break;
				case 8:
					if (bigendian) {
						q = ((uint64)buf[0] << 56) | ((uint64)buf[1] << 48) | ((uint64)buf[2] << 40) | ((uint64)buf[3] << 32)
						  | ((uint64)buf[4] << 24) | ((uint64)buf[5] << 16) | ((uint64)buf[6] << 8) | (uint64)buf[7];
					} else {
						q = ((uint64)buf[7] << 56) | ((uint64)buf[6] << 48) | ((uint64)buf[5] << 40) | ((uint64)buf[4] << 32)
						  | ((uint64)buf[3] << 24) | ((uint64)buf[2] << 16) | ((uint64)buf[1] << 8) | (uint64)buf[0];
					}
					break;
				}
				/* FIXME: uint64 ? */
				if ((str = matchhash(q, tbl))) {
					*string = str;
					*length = strlen(*string);
				}
			} else {
				*string = "?";
				*length = strlen(*string);
			}
		}
	}
}

void ht_uformat_viewer::reloadpalette()
{
	ht_format_viewer::reloadpalette();
	free(tagpal.data);
	tagpal.data = NULL;
	load_pal(palclasskey_tags, palkey_tags_default, &tagpal);
}

uint ht_uformat_viewer::render_tagstring(char *chars, vcp *colors, uint maxlen, char *tagstring, bool cursor_in_line)
{
	char *n = tagstring;
	uint c = 0;
	int i = 0, g = 0;
	bool is_cursor;
	vcp color_normal = getcolor(palidx_generic_body);
	do {
		int l = 0;
		while (n[l] && n[l] != '\e') l++;
		c += render_tagstring_single(chars, colors, maxlen, c, n, l, color_normal);
		
		n += l;
		is_cursor = cursor_in_line && i == cursor.tag_idx;
		if (*n == '\e') {
			FileOfs tag_offset;
			vcp tag_color;
			char str[64];
			switch (n[1]) {
			case HT_TAG_EDIT_BYTE: {
				byte d;
					
				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 1, focused && (g==cursor.tag_group), is_cursor);

				if (pread(tag_offset, &d, 1) == 1) {
					ht_snprintf(str, sizeof str, "%02x", d);
				} else {
					strcpy(str, "??");
				}
					
				c += render_tagstring_single(chars, colors, maxlen, c, str, 2, tag_color);
		
				n += HT_TAG_EDIT_BYTE_LEN;
				break;
			}
			case HT_TAG_EDIT_WORD_LE: {
				uint16 d;

				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 2, (g==cursor.tag_group), is_cursor);
					
				byte buf[2];
				if (pread(tag_offset, &buf, 2) == 2) {
					/* little endian */
					d = (buf[1] << 8) | buf[0];
					ht_snprintf(str, sizeof str, "%04x", d);
				} else {
					strcpy(str, "????");
				}
					
				c += render_tagstring_single(chars, colors, maxlen, c, str, 4, tag_color);
				n += HT_TAG_EDIT_WORD_LE_LEN;
				break;
			}
			case HT_TAG_EDIT_DWORD_LE: {
				uint32 d;
					
				tag_offset=tag_get_offset(n);
				tag_color=get_tag_color_edit(tag_offset, 4, (g==cursor.tag_group), is_cursor);

				byte buf[4];
				if (pread(tag_offset, &buf, 4) == 4) {
					/* little endian */
					d = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
					ht_snprintf(str, sizeof str, "%08x", d);
				} else {
					strcpy(str, "????????");
				}

				c += render_tagstring_single(chars, colors, maxlen, c, str, 8, tag_color);
				n += HT_TAG_EDIT_DWORD_LE_LEN;
				break;
			}
			case HT_TAG_EDIT_QWORD_LE: {
				uint64 q;

				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 8, (g==cursor.tag_group), is_cursor);

				byte buf[8];
				if (pread(tag_offset, &buf, 8)==8) {
					/* little endian */
					q = ((uint64)buf[7] << 56) | ((uint64)buf[6] << 48) | ((uint64)buf[5] << 40) | ((uint64)buf[4] << 32)
					  | ((uint64)buf[3] << 24) | ((uint64)buf[2] << 16) | ((uint64)buf[1] << 8) | (uint64)buf[0];
					ht_snprintf(str, sizeof str, "%016qx", q);
				} else {
					strcpy(str, "????????????????");
				}

				c += render_tagstring_single(chars, colors, maxlen, c, str, 16, tag_color);
				n += HT_TAG_EDIT_QWORD_LE_LEN;
				break;
			}
			case HT_TAG_EDIT_WORD_BE: {
				uint16 d;
					
				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 2, (g==cursor.tag_group), is_cursor);
					
				byte buf[2];
				if (pread(tag_offset, &buf, 2)==2) {
					/* big endian */
					d = (buf[0] << 8) | buf[1];
					ht_snprintf(str, sizeof str, "%04x", d);
				} else {
					strcpy(str, "????");
				}
					
				c += render_tagstring_single(chars, colors, maxlen, c, str, 4, tag_color);
				n += HT_TAG_EDIT_WORD_BE_LEN;
				break;
			}
			case HT_TAG_EDIT_DWORD_BE: {
				uint32 d;
					
				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 4, (g==cursor.tag_group), is_cursor);
					
				byte buf[4];
				if (pread(tag_offset, &buf, 4) == 4) {
					/* big endian */
					d = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
					ht_snprintf(str, sizeof str, "%08x", d);
				} else {
					strcpy(str, "????????");
				}
				c += render_tagstring_single(chars, colors, maxlen, c, str, 8, tag_color);
				n += HT_TAG_EDIT_DWORD_BE_LEN;
				break;
			}
			case HT_TAG_EDIT_QWORD_BE: {
				uint64 q;
					
				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 8, (g==cursor.tag_group), is_cursor);
					
				byte buf[8];
				if (pread(tag_offset, &buf, 8) == 8) {
					/* big endian */
					q = ((uint64)buf[0] << 56) | ((uint64)buf[1] << 48) | ((uint64)buf[2] << 40) | ((uint64)buf[3] << 32)
					  | ((uint64)buf[4] << 24) | ((uint64)buf[5] << 16) | ((uint64)buf[6] << 8) | (uint64)buf[7];
					ht_snprintf(str, sizeof str, "%016qx", q);
				} else {
					strcpy(str, "????????????????");
				}
				c += render_tagstring_single(chars, colors, maxlen, c, str, 16, tag_color);
				n += HT_TAG_EDIT_QWORD_BE_LEN;
				break;
			}
			case HT_TAG_EDIT_TIME_LE:
			case HT_TAG_EDIT_TIME_BE: {
				uint32 d;
					
				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 4, (g==cursor.tag_group), is_cursor);

				byte buf[4];
				if (pread(tag_offset, &buf, 4) == 4) {
					if (n[1] == HT_TAG_EDIT_TIME_LE) {
						d = (buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
					} else {
						d = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
					}
					time_t tt = d;
					tm *t = gmtime(&tt);
					sprintf(str, "%02d:%02d:%02d %02d.%02d.%04d +1900", t->tm_hour, t->tm_min, t->tm_sec, t->tm_mday, t->tm_mon+1, t->tm_year);
				} else {
					strcpy(str, "?");
				}
					
				c += render_tagstring_single(chars, colors, maxlen, c, str, strlen(str), tag_color);
				n += HT_TAG_EDIT_TIME_LE_LEN;
				break;
			}
			case HT_TAG_EDIT_CHAR: {
				char d;
					
				tag_offset = tag_get_offset(n);
				tag_color = get_tag_color_edit(tag_offset, 1, (g==cursor.tag_group), is_cursor);
					
				if (pread(tag_offset, &d, 1) != 1) {
					d = '?';
				}
					
				c += render_tagstring_single(chars, colors, maxlen, c, &d, 1, tag_color);
				n += HT_TAG_EDIT_CHAR_LEN;
				break;
			}
			case HT_TAG_EDIT_BIT: {
				int shift = ((ht_tag_edit_bit*)n)->bitidx;
				int op = shift/8;
				byte d;

				tag_offset=tag_get_offset(n);
				tag_color=getcolor_tag(palidx_tags_edit_tag);
				bool isdirty = false;
				FileOfs o = IS_DIRTY_SINGLEBIT | (shift & 7);
				file->cntl(FCNTL_MODS_IS_DIRTY, tag_offset+op, o, &isdirty);
				if (isdirty) tag_color = mixColors(tag_color, getcolor_tag(palidx_tags_edit_tag_modified));
				if (tag_offset >= sel_start && tag_offset < sel_end) tag_color = mixColors(tag_color, getcolor_tag(palidx_tags_edit_tag_selected));
				if (is_cursor) tag_color = mixColors(tag_color, getcolor_tag(edit() ? palidx_tags_edit_tag_cursor_edit : palidx_tags_edit_tag_cursor_select));

				if (pread(tag_offset+op, &d, 1)==1) {
					str[0]=(d& (1 << (shift%8))) ? '1' : '0';
					str[1]=0;
				} else {
					strcpy(str, "?");
				}
					
				c += render_tagstring_single(chars, colors, maxlen, c, str, 1, tag_color);
				n += HT_TAG_EDIT_BIT_LEN;
				break;
			}
			case HT_TAG_EDIT_SELVIS: {
				tag_offset=tag_get_offset(n);
				tag_color=getcolor_tag(palidx_tags_edit_tag);

				if (tag_offset >= sel_start && tag_offset < sel_end) tag_color = mixColors(tag_color, getcolor_tag(palidx_tags_edit_tag_selected));

				c += render_tagstring_single(chars, colors, maxlen, c, &((ht_tag_edit_selvis*)n)->ch, 1, tag_color);
				n += HT_TAG_EDIT_SELVIS_LEN;
				continue;
			}
			case HT_TAG_SEL: {
				int tag_textlen = tag_get_seltextlen(n);
				if (is_cursor) {
					tag_color = getcolor_tag(
						(focused && (g == cursor.tag_group)) ?
						palidx_tags_sel_tag_cursor_focused :
						palidx_tags_sel_tag_cursor_unfocused
					);
				} else {
					tag_color = getcolor_tag(palidx_tags_sel_tag);
				}
				n += HT_TAG_SEL_LEN(0);
				c += render_tagstring_single(chars, colors, maxlen, c, n, tag_textlen, tag_color);
				n += tag_textlen;
				break;
			}
			case HT_TAG_DESC_BYTE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 1, true, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_BYTE_LEN;
				break;
			}
			case HT_TAG_DESC_WORD_LE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 2, false, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_WORD_LE_LEN;
				break;
			}
			case HT_TAG_DESC_DWORD_LE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 4, false, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_DWORD_LE_LEN;
				break;
			}
			case HT_TAG_DESC_QWORD_LE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 8, false, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_QWORD_LE_LEN;
				break;
			}
			case HT_TAG_DESC_WORD_BE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 2, true, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_WORD_BE_LEN;
				break;
			}
			case HT_TAG_DESC_DWORD_BE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 4, true, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_DWORD_BE_LEN;
				break;
			}
			case HT_TAG_DESC_QWORD_BE: {
				const char *str;
				int l;
				render_tagstring_desc(&str, &l, &tag_color, n, 8, true, is_cursor);
				c += render_tagstring_single(chars, colors, maxlen, c, str, l, tag_color);
				n += HT_TAG_DESC_DWORD_BE_LEN;
				break;
			}
			case HT_TAG_FLAGS:
				n += HT_TAG_FLAGS_LEN;
				tag_color = getcolor_tag(palidx_tags_sel_tag);
				if (is_cursor) tag_color = getcolor_tag(palidx_tags_sel_tag_cursor_focused);
				c += render_tagstring_single(chars, colors, maxlen, c, "details", 7, tag_color);
				break;
			case HT_TAG_GROUP:
				n += HT_TAG_GROUP_LEN;
				g++;
				i = 0;
				continue;
			case HT_TAG_COLOR:
				color_normal = tag_get_color(n);
				if (color_normal == -1) {
					color_normal = getcolor(palidx_generic_body);
				}
				n += HT_TAG_COLOR_LEN;
				continue;
			default: {
				assert(0);
			}
			}
		}
		i++;
	} while (*n);
	return c;
}

uint ht_uformat_viewer::render_tagstring_single(char *chars, vcp *colors, uint maxlen, uint offset, const char *text, uint len, vcp color)
{
	if (offset >= maxlen) {
		return 0;
	}
	maxlen -= offset;
	if (chars) chars += offset;
	if (colors) colors += offset;
	uint l = MIN(len, maxlen);
	uint r = 0;
	while (l--) {
		if (chars) *chars++ = *text++;
		if (colors) *colors++ = color;
		r++;
	}
	return r;
}

#define MAX_PRINT_TAGSTRING_LINELENGTH 256

void ht_uformat_viewer::print_tagstring(int x, int y, int maxlen, int xscroll, char *tagstring, bool cursor_in_line)
{
	char text[MAX_PRINT_TAGSTRING_LINELENGTH];
	char *t = text+xscroll;
	vcp color[MAX_PRINT_TAGSTRING_LINELENGTH];
	vcp *c = color+xscroll;
	int l = render_tagstring(text, color,
		(maxlen+xscroll+1 > MAX_PRINT_TAGSTRING_LINELENGTH) ? MAX_PRINT_TAGSTRING_LINELENGTH
		: maxlen+xscroll+1, tagstring, cursor_in_line);

	if (l > xscroll) {
		l -= xscroll;
		while (l--) {
			if (x >= size.w) {
				buf->printChar(x-1, y, VCP(VC_GREEN, VC_TRANSPARENT), '>');
				break;
			}
			buf->printChar(x, y, *c, (unsigned char)*t);
			t++;
			c++;
			x++;
		}
	}
}

void ht_uformat_viewer::select_mode_off()
{
	if (cursor_select) {
		cursor_select = 0;
	}
}

void ht_uformat_viewer::select_mode_on()
{
	if (!cursor_select) {
		cursor_select = 1;
	}
}

void ht_uformat_viewer::select_mode_pre()
{
	if (cursor_select) {
		if (cursor_tag_class == tag_class_edit) {
			cursor_select_start = cursor_tag_offset;
		} else {
			cursor_select_start = (sel_end > sel_start) ? sel_end : -1ULL;
		}
		if (!get_current_tag_size(&cursor_select_cursor_length)) {
			cursor_select_cursor_length = -1;
		}
	}		
}	
	
void ht_uformat_viewer::select_mode_post(bool lastpos)
{
	if (cursor_select) {
		if (cursor_select_start != -1ULL) {
			if (cursor_tag_class == tag_class_edit) {
				if (lastpos && cursor_select_cursor_length != uint32(-1)) {
					FileOfs s, e;
					pselect_get(&s, &e);
					if (e != cursor_tag_offset+cursor_select_cursor_length) {
						pselect_add(cursor_select_start, cursor_tag_offset+cursor_select_cursor_length);
					}
				} else {
					pselect_add(cursor_select_start, cursor_tag_offset);
				}
			} else if (cursor_tag_class == tag_class_sel && cursor_select_cursor_length != uint32(-1)) {
				pselect_add(cursor_select_start, cursor_select_start+cursor_select_cursor_length);
			}
		}
	}
}

uint ht_uformat_viewer::pwrite(FileOfs ofs, void *buf, uint size)
{
	cursorline_dirty();
	return ht_format_viewer::pwrite(ofs, buf, size);
}

bool ht_uformat_viewer::ref()
{
	cursorline_get();
	char *e = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	if (!e) return false;
	if (tag_get_class(e) == tag_class_sel) {
		if (!cursor.sub->ref(&cursor_tag_id.id)) {
			switch (e[1]) {
			case HT_TAG_SEL:
				return ref_sel(&cursor_tag_id.id);
			case HT_TAG_FLAGS:
				return ref_flags(((ht_tag_flags*)e)->id, ((ht_tag_flags*)e)->offset);
			case HT_TAG_DESC_BYTE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 1, true);
			case HT_TAG_DESC_WORD_LE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 2, false);
			case HT_TAG_DESC_DWORD_LE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 4, false);
			case HT_TAG_DESC_QWORD_LE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 8, false);
			case HT_TAG_DESC_WORD_BE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 2, true);
			case HT_TAG_DESC_DWORD_BE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 4, true);
			case HT_TAG_DESC_QWORD_BE:
				return ref_desc(((ht_tag_desc_byte*)e)->id, ((ht_tag_desc_byte*)e)->offset, 8, true);
			}
		}
	}
	return false;
}

bool ht_uformat_viewer::ref_desc(ID id, FileOfs offset, uint size, bool bigendian)
{
	Endianess end = bigendian ? big_endian : little_endian;
	int_hash *desc=(int_hash*)getAtomValue(id);
	if (desc) {
		Bounds b;
		b.w = 60;
		b.h = 14;
		b.x = (screen->w - b.w)/2;
		b.y = (screen->h - b.h)/2;
		ht_dialog *g=new ht_dialog();
		g->init(&b, "desc", FS_KILLER | FS_MOVE);

		b.x = 0;
		b.y = 0;
		b.w -= 2;
		b.h -= 2;
		ht_itext_listbox *l = new ht_itext_listbox();
		l->init(&b, 2, 1);

		byte buf[4];
		int curpos = 0;
		int i = 0;
		int d = 0;

		if (pread(offset, buf, size) != size) return false;
		
		switch (size) {
			case 1: d = buf[0]; break;
			case 2:
				if (bigendian) {
					d=(buf[0]<<8) | buf[1];
				} else {
					d=(buf[1]<<8) | buf[0];
				}
				break;
			case 4:
				if (bigendian) {
					d=(buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
				} else {
					d=(buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
				}
				break;
		}

		int_hash *dsc = desc;
		while (dsc->desc) {
			char buf[32];
			switch (size) {
			case 1:
				sprintf(buf, "0x%02x", dsc->value);
				break;
			case 2:
				sprintf(buf, "0x%04x", dsc->value);
				break;
			case 4:
				sprintf(buf, "0x%08x", dsc->value);
				break;
			}
			if (dsc->value == d) {
				curpos = i;
			}
			l->insert_str(i, buf, dsc->desc);
			dsc++;
			i++;
		}

		l->update();
		l->gotoItemByPosition(curpos);

		g->insert(l);
		g->setpalette(palkey_generic_window_default);

		if (g->run(false) == button_ok) {
			ht_listbox_data da;
			ViewDataBuf vdb(l, &da, sizeof da);
			int i = l->getID(da.data->cursor_ptr);
			if (desc[i].value != d) {
				baseview->sendmsg(cmd_edit_mode_i, file, NULL);
				if (edit()) {
					byte buf[4];
					uint v = desc[i].value;
					createForeignInt(buf, v, size, end);
					pwrite(offset, buf, size);
					dirtyview();
				}					
			}
		}

		g->done();
		delete g;
		return true;
	}
	return false;
}

bool ht_uformat_viewer::ref_flags(ID id, FileOfs offset)
{
	ht_tag_flags_s *flags = (ht_tag_flags_s*)getAtomValue(id);
	if (flags) {
		Bounds b;
		b.w = 60;
		b.h = 14;
		b.x = (screen->w-b.w)/2;
		b.y = (screen->h-b.h)/2;
		ht_dialog *d = new ht_dialog();
		d->init(&b, (flags->bitidx==-1) ? flags->desc : 0, FS_KILLER | FS_TITLE | FS_MOVE);

		b.x = 0;
		b.y = 0;
		b.w -= 2;
		b.h -= 2;
		ht_uformat_viewer *u = new ht_uformat_viewer();
		u->init(&b, 0, VC_EDIT, file, 0);

		ht_mask_sub *m = new ht_mask_sub();
		m->init(file, 0);
		char *t, x[256];

		int width = 0;
		ht_tag_flags_s *fl = flags;
		if (fl->bitidx == -1) fl++;
		do {
			int l = strlen(fl->desc);
			if (l > width) width = l;
			fl++;
		} while (fl->desc);

		width = MAX(width, 25);
		width++;

		fl = flags;
		if (fl->bitidx == -1) fl++;
		do {
			t = x;
			int l = strlen(fl->desc);
			memcpy(t, fl->desc, l);
			t += l;
			while (t < x+width) *(t++) = ' ';
			t = tag_make_edit_bit(t, sizeof x, offset, fl->bitidx);
			*t = 0;
			m->add_mask(x);
			fl++;
		} while (fl->desc);


		u->insertsub(m);
		u->sendmsg(msg_complete_init, 0);
		d->insert(u);

		d->setpalette(palkey_generic_window_default);

		uint pmode = file->getAccessMode() & IOAM_WRITE;

		if (d->run(false) == button_ok) u->edit_update();

		if (pmode != (file->getAccessMode() & IOAM_WRITE)) {
			if (pmode) {
				baseview->sendmsg(cmd_view_mode_i, file, NULL);
			} else {
				baseview->sendmsg(cmd_edit_mode_i, file, NULL);
			}
		}

		d->done();
		delete d;
		return true;
	}
	return false;
}

bool ht_uformat_viewer::ref_sel(LINE_ID *id)
{
	return false;
}

ht_search_result *ht_uformat_viewer::psearch(ht_search_request *request, FileOfs start, FileOfs end)
{
	if (request != last_search_request) {
		delete last_search_request;
		last_search_request = request->clone();
	}
	last_search_physical = true;
	last_search_end_ofs = end;
	
	ht_sub *sub=first_sub;
	while (sub) {
		ht_search_result *r = sub->search(request, start, end);
		if (r) return r;
		sub = sub->next;
	}
	return NULL;
}

ht_search_result *ht_uformat_viewer::vsearch(ht_search_request *request, viewer_pos start, viewer_pos end)
{
	if (request != last_search_request) {
		delete last_search_request;
		last_search_request = request->clone();
	}
	last_search_physical = false;
	last_search_end_pos = end;
	
	if (request->search_class == SC_VISUAL && request->type == ST_REGEX) {
		if (!cursor.sub) return 0;
		ht_regex_search_request *s=(ht_regex_search_request*)request;
		/* build progress indicator */
		Bounds b;
		get_std_progress_indicator_metrics(&b);
		ht_progress_indicator *progress_indicator=new ht_progress_indicator();
		progress_indicator->init(&b, "ESC to cancel");
		uint lines=0;

		uformat_viewer_pos p = start.u;
		while (p.sub) {
			do {
				char line[1024];
				if (!p.sub->getline(line, sizeof line, p.line_id)) assert(0);
				char rdrd[256];
				int c = render_tagstring(rdrd, 0, sizeof rdrd-1, line, 0);
				rdrd[c] = 0;
				regmatch_t pos;
				if (!regexec(&s->rx, rdrd, 1, &pos, 0)) {
					ht_visual_search_result *r = new ht_visual_search_result();
					r->pos.u = p;
					r->xpos = pos.rm_so;
					r->length = pos.rm_eo-pos.rm_so;
					progress_indicator->done();
					delete progress_indicator;

					return r;
				}
				lines++;
				if (lines % 500==0) {
					if (keyb_keypressed()) {
						if (keyb_getkey() == K_Escape) goto leave;
					}

					char text[256];
/*					if (highest_va>lowest_va) {
						int p=100*((double)(va-lowest_va))/(highest_va-lowest_va);
						sprintf(text, "searching for '%s'... %d%% complete (%d lines)", s->rx_str, p, lines);
					} else*/ {
						ht_snprintf(text, sizeof text, "searching for '%s'... %d lines", s->rx_str, lines);
					}
					progress_indicator->settext(text);
					progress_indicator->sendmsg(msg_draw, 0);
					screen->show();
				}
			} while (p.sub->next_line_id(&p.line_id, 1));
			p.sub = p.sub->next;
			if (p.sub) p.sub->first_line_id(&p.line_id);
		}
leave:
		progress_indicator->done();
		delete progress_indicator;
	}
	return NULL;
}

void ht_uformat_viewer::pselect_add(FileOfs start, FileOfs end)
{
	bool downward = (start<end);
	if (end<start) {
		FileOfs temp=start;
		start=end;
		end=temp;
	}
	if ((end==sel_start) && !downward) {
		sel_start=start;
	} else if ((start==sel_end) && downward) {
		sel_end=end;
	} else if ((end==sel_end) && !downward) {
		sel_end=start;
	} else if ((start==sel_start) && downward) {
		sel_start=end;
	} else {
		sel_start=start;
		sel_end=end;
	}
	if (sel_start>sel_end) {
		FileOfs temp=sel_start;
		sel_start=sel_end;
		sel_end=temp;
	}
}

void ht_uformat_viewer::pselect_get(FileOfs *start, FileOfs *end)
{
	*start=sel_start;
	*end=sel_end;
}

void ht_uformat_viewer::pselect_set(FileOfs start, FileOfs end)
{
	sel_start=start;
	sel_end=end;
}

void ht_uformat_viewer::clear_subs()
{
	ht_sub *s = first_sub, *t;
	while (s) {
		t = s->next;
		s->done();
		delete s;
		s = t;
	}

	uf_initialized = false;
	cursor_ypos = 0;

	clear_viewer_pos(&top);
	clear_viewer_pos(&cursor);
	first_sub = NULL;
	last_sub = NULL;
}

void ht_uformat_viewer::clear_viewer_pos(viewer_pos *p)
{
	clear_viewer_pos(&p->u);
}

void ht_uformat_viewer::clear_viewer_pos(uformat_viewer_pos *p)
{
	p->sub = NULL;
	clear_line_id(&p->line_id);
	p->tag_idx = 0;
	p->tag_group = 0;
}

bool ht_uformat_viewer::compeq_viewer_pos(viewer_pos *a, viewer_pos *b)
{
	return compeq_viewer_pos(&a->u, &b->u);
}
	
bool ht_uformat_viewer::compeq_viewer_pos(uformat_viewer_pos *a, uformat_viewer_pos *b)
{
	return (a->sub == b->sub
		   && compeq_line_id(a->line_id, b->line_id)
		   && a->tag_idx == b->tag_idx
		   && a->tag_group == b->tag_group);
}

void ht_uformat_viewer::sendsubmsg(int msg)
{
	htmsg m;
	m.msg = msg;
	sendsubmsg(&m);
}

void ht_uformat_viewer::sendsubmsg(htmsg *msg)
{
	if (msg->type == mt_broadcast) {
		ht_sub *s = first_sub;
		while (s) {
			s->handlemsg(msg);
			s = s->next;
		}
	} else {
		cursor.sub->handlemsg(msg);
	}
}

bool ht_uformat_viewer::set_cursor(uformat_viewer_pos p)
{
	cursorline_dirty();
	uformat_viewer_pos t = top;
	int ty = 0;
//     bool hasnext = true;
/* test if cursor is already on screen */
//	if (cursor_state == cursor_state_visible) {
		do {
//			if (compeq_viewer_pos(&t, &p)) {
			if ((t.sub == p.sub) && compeq_line_id(t.line_id, p.line_id)) {
				cursor = p;
				if (p.tag_group != -1) cursor.tag_group = p.tag_group;
				adjust_cursor_group();
				if (p.tag_idx !=-1) cursor.tag_idx = p.tag_idx;
				adjust_cursor_idx();
				cursor_ypos = ty;
				update_misc_info();
				update_visual_info();
				check_cursor_visibility();
				cursorline_dirty();
				dirtyview();
				return true;
			}
		} while ((/*hasnext = */next_line(&t, 1)) && (ty++ < size.h-1));
//	}
/**/

	char line[1024];
	char *e;
	p.sub->getline(line, sizeof line, p.line_id);
	e = tag_get_selectable_tag(line, 0, 0);
	if (!e) return 0;
	cursor = p;
	if (p.tag_group != -1) cursor.tag_group = p.tag_group;
	adjust_cursor_group();
	if (p.tag_idx != -1) cursor.tag_idx = p.tag_idx;
	adjust_cursor_idx();
	cursor_ypos=0;
//	cursor_ypos=center_view(sub, id1, id2);
	top = p;
	update_misc_info();
	update_visual_info();
	check_cursor_visibility();
	cursorline_dirty();
	dirtyview();
	return true;
}

void ht_uformat_viewer::scroll_up(int n)
{
	cursor_ypos += prev_line(&top, n);
	cursorline_dirty();
	check_cursor_visibility();
}

void ht_uformat_viewer::scroll_down(int n)
{
	cursor_ypos -= next_line(&top, n);
	cursorline_dirty();
	check_cursor_visibility();
}

int ht_uformat_viewer::sub_to_idx(const ht_sub *sub) const
{
	int sub_idx = 0;
	ht_sub *s = first_sub;
	while (s != sub) {
		sub_idx++;
		s = s->next;
		assert(s);
	}
	return sub_idx;
}

ht_sub *ht_uformat_viewer::idx_to_sub(int idx) const
{
	ht_sub *s = first_sub;
	while (idx--) {
		if (!s->next) return s;
		s = s->next;
	}
	return s;
}

bool ht_uformat_viewer::qword_to_offset(uint64 q, FileOfs *ofs)
{
	*ofs = q;
	return true;
}

void ht_uformat_viewer::update_micropos()
{
	cursorline_get();
	char *e=tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	if (e) {
		int s=tag_get_microsize(e);
		if (cursor_tag_micropos>=s) cursor_tag_micropos=s-1;
	}
}

void ht_uformat_viewer::update_misc_info()
{
	cursorline_get();
	char *e = tag_get_selectable_tag(cursor_line, cursor.tag_idx, cursor.tag_group);
	if (e) {
		cursor_tag_class = tag_get_class(e);
		switch (cursor_tag_class) {
		case tag_class_edit:
			cursor_tag_offset = tag_get_offset(e);
			break;
		case tag_class_sel:
			clear_line_id(&cursor_tag_id.id);
			tag_get_id(e, &cursor_tag_id.id.id1, &cursor_tag_id.id.id2, &cursor_tag_id.id.id3, &cursor_tag_id.id.id4);
			break;
		}
	}
}

void ht_uformat_viewer::update_visual_info()
{
	cursorline_get();
	const char *s, *t = cursor_line;
	int v = 0, vl = 0;
	int i = 0, g = 0;
	while ((s=tag_findnext(t))) {
		int cl = tag_get_class(s);
		if (s[1] == HT_TAG_GROUP) {
			i = 0;
			g++;
		}
		v += s-t;
		vl = tag_get_vlen(s);
		if (i == cursor.tag_idx && g == cursor.tag_group && cl != tag_class_no) break;
		v += vl;
		t = s+tag_get_len(s);
		if (cl != tag_class_no) i++;
	}

	if (cursor_tag_micropos > vl-1) cursor_tag_micropos = vl ? vl-1 : 0;
	cursor_visual_xpos = v;
	cursor_visual_length = vl;
}

void ht_uformat_viewer::update_ypos()
{
	uformat_viewer_pos p = top;
	int y = 0;
	while (next_line(&p, 1) && y < size.h) {
		if (compeq_viewer_pos(&p, &cursor)) {
			cursor_ypos = y;
			break;
		}
		y++;
	}
}

void ht_uformat_viewer::vstate_restore(Object *data)
{
	ht_uformat_viewer_vstate *vs = (ht_uformat_viewer_vstate*)data;
	if (vs->resolve) {
		vs->top.sub = idx_to_sub(intptr_t(vs->top.sub));
		vs->cursor.sub = idx_to_sub(intptr_t(vs->cursor.sub));
		vs->resolve = false;
	}
	top = vs->top;
	cursor = vs->cursor;
	cursor_state = vs->cursor_state;
	cursor_ypos = vs->cursor_ypos;
	sel_start = vs->sel_start;
	sel_end = vs->sel_end;
	cursorline_dirty();
	update_misc_info();
	update_visual_info();
}

Object *ht_uformat_viewer::vstate_create()
{
	ht_uformat_viewer_vstate *vs = new ht_uformat_viewer_vstate();
	vs->top = top;
	vs->cursor = cursor;
	vs->cursor_state = cursor_state;
	vs->cursor_ypos = cursor_ypos;
	vs->sel_start = sel_start;
	vs->sel_end = sel_end;
	return vs;
}

/*
 *	CLASS ht_sub
 */

void ht_sub::init(File *f)
{
	Object::init();
	uformat_viewer=NULL;
	prev=NULL;
	next=NULL;
	file=f;
}

bool ht_sub::closest_line_id(LINE_ID *line_id)
{
	first_line_id(line_id);
	return true;
}

bool ht_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	return false;
}

bool ht_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset)
{
	return false;
}

bool ht_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	return false;
}

void ht_sub::handlemsg(htmsg *msg)
{
}

void ht_sub::first_line_id(LINE_ID *line_id)
{
}

void ht_sub::last_line_id(LINE_ID *line_id)
{
}

int ht_sub::prev_line_id(LINE_ID *line_id, int n)
{
	return 0;
}

int ht_sub::next_line_id(LINE_ID *line_id, int n)
{
	return 0;
}

bool ht_sub::ref(LINE_ID *id)
{
	return false;
}

ht_search_result *ht_sub::search(ht_search_request *search, FileOfs start, FileOfs end)
{
	return NULL;
}

/*
 *	CLASS ht_linear_sub
 */

void ht_linear_sub::init(File *f, FileOfs ofs, FileOfs size)
{
	ht_sub::init(f);
	fofs = ofs;
	fsize = size;
}

void ht_linear_sub::handlemsg(htmsg *msg)
{
	if (msg->msg == msg_filesize_changed) {
		FileOfs s = file->getSize();
		if (fofs > s) {
			fsize = 0;
		} else if (fofs+fsize > s) {
			fsize = s-fofs;
		}
		return;
	}
}

static int ht_linear_func_readbyte(eval_scalar *result, eval_int *offset)
{
	struct context_t {
		ht_linear_sub *sub;
		ht_format_viewer *fv;
		int i, o;
	};
	ht_format_viewer *f=((context_t*)eval_get_context())->fv;
	byte b;
	if (f->pread(offset->value, &b, 1) != 1) {
		set_eval_error("i/o error (requested %d, read %d from ofs 0x%08qx)", 1, 0, offset->value);
		return 0;
	}
	scalar_create_int_c(result, b);
	return 1;
}

static int ht_linear_func_readstring(eval_scalar *result, eval_int *offset, eval_int *len)
{
	struct context_t {
		ht_linear_sub *sub;
		ht_format_viewer *fv;
		int i, o;
	};
	ht_format_viewer *f = ((context_t*)eval_get_context())->fv;

	uint l = len->value;
	void *buf = malloc(l);	/* FIXME: may be too slow... */

	if (buf) {
		eval_str s;
		uint c = f->pread(offset->value, buf, l);
		if (c != l) {
			free(buf);
			set_eval_error("i/o error (requested %d, read %d from ofs 0x%08qx)", l, c, offset->value);
			return 0;
		}
		s.value = (char*)buf;
		s.len = l;
		scalar_create_str(result, &s);
		free(buf);
		return 1;
	}
	set_eval_error("out of memory");
	return 0;
}

static int ht_linear_func_entropy(eval_scalar *result, eval_str *buf)
{
	scalar_create_int_c(result, calc_entropy2((byte *)buf->value, buf->len));
	return 1;
}

static int ht_linear_func_entropy2(eval_scalar *result, eval_str *buf)
{
	scalar_create_float_c(result, calc_entropy((byte *)buf->value, buf->len));
	return 1;
}

struct search_expr_eval_context_t {
	ht_sub *sub;
	ht_format_viewer *fv;
	FileOfs i, o;
};

static bool ht_linear_sub_func_handler(eval_scalar *result, char *name, eval_scalarlist *params)
{
	eval_func myfuncs[] = {
		{"entropy", (void*)&ht_linear_func_entropy, {SCALAR_STR}},
		{"entropy2", (void*)&ht_linear_func_entropy2, {SCALAR_STR}},
		{"readbyte", (void*)&ht_linear_func_readbyte, {SCALAR_INT}},
		{"readstring", (void*)&ht_linear_func_readstring, {SCALAR_INT, SCALAR_INT}},
		{NULL}
	};
	return std_eval_func_handler(result, name, params, myfuncs);
}

static bool ht_linear_sub_symbol_handler(eval_scalar *result, char *name)
{
	search_expr_eval_context_t *context =
		(search_expr_eval_context_t*)eval_get_context();
	if (strcmp(name, "i")==0) {
		scalar_create_int_q(result, context->i);
		return true;
	} else if (strcmp(name, "o")==0) {
		scalar_create_int_q(result, context->o);
		return true;
	} else return false;
}

class ht_expr_search_pcontext: public Object {
public:
	/* in */
	ht_search_request *request;
	ht_sub *sub;
	ht_format_viewer *fv;
	FileOfs start;
	FileOfs end;
	int i;
	FileOfs o;
	/* out */
	ht_search_result **result;
};

bool process_search_expr(Object *ctx, ht_text *progress_indicator)
{
#define PROCESS_EXPR_SEARCH_BYTES_PER_CALL	256
	ht_expr_search_pcontext *c = (ht_expr_search_pcontext*)ctx;
	ht_expr_search_request *s = (ht_expr_search_request*)c->request;

	search_expr_eval_context_t context;
	context.sub = c->sub;
	context.fv = c->fv;
	int w = PROCESS_EXPR_SEARCH_BYTES_PER_CALL;
	while (c->o < c->end) {
		eval_scalar r;
		context.i = c->i;
		context.o = c->o;
		if (eval(&r, s->expr, ht_linear_sub_func_handler, ht_linear_sub_symbol_handler, &context)) {
			eval_int i;
			scalar_context_int(&r, &i);
			if (i.value != 0) {
				ht_physical_search_result *r = new ht_physical_search_result();
				r->offset = c->o;
				r->size = 1;
				*c->result = r;
				return false;
			}
		} else {
			const char *str;
			int pos;
			get_eval_error(&str, &pos);
			throw MsgfException("eval error at pos %d: %s", pos, str);
		}
		c->i++;
		c->o++;
		
		if (!--w) {
			char text[64];
			if (c->end > c->start) {
				ht_snprintf(text, 100, "%qd %% done", (c->o - c->start) * 100 / (c->end - c->start));
			} else {
				strcpy(text, "? % done");
			}
			progress_indicator->settext(text);
	
			return true;
		}
	}
	return false;
}

ht_search_result *linear_expr_search(ht_search_request *search, FileOfs start, FileOfs end, ht_sub *sub, ht_uformat_viewer *ufv, FileOfs fofs, FileOfs fsize)
{
	if (start < fofs) start = fofs;
	if (end > fofs + fsize) end = fofs+fsize;
	if (fsize) {
		ht_search_result *r = NULL;
		ht_expr_search_pcontext c;
		c.request = search;
		c.sub = sub;
		c.fv  =ufv;
		c.start = start;
		c.end = end;
		c.result = &r;
		c.i = 0;
		c.o = start;
		if (execute_process(process_search_expr, &c)) return r;
	}
	return NULL;
}

ht_search_result *ht_linear_sub::search(ht_search_request *search, FileOfs start, FileOfs end)
{
	ht_search_result *r = NULL;
	if (search->search_class == SC_PHYSICAL && search->type == ST_EXPR) {
		r = linear_expr_search(search, start, end, this, uformat_viewer, fofs, fsize);
	} else if (search->search_class == SC_PHYSICAL && search->type == ST_FXBIN) {
		r = linear_bin_search(search, start, end, file, fofs, fsize);
	}
	return r;
}

/*
 *	CLASS ht_hex_sub
 */

void ht_hex_sub::init(File *f, FileOfs ofs, FileOfs size, uint Line_length, uint u, int Disp)
{
	line_length = Line_length;
	ht_linear_sub::init(f, ofs, size);
	if (Disp == -1) {
		disp = line_length - ofs % line_length;
		if (disp == line_length) disp = 0;
	} else {
		disp = Disp;
	}
	uid = u;
}

int ht_hex_sub::get_line_length()
{
	return line_length;
}

void ht_hex_sub::set_line_length(uint Line_length)
{
	if (Line_length > 0 && Line_length > disp) {
		line_length = Line_length;
	}
}

int ht_hex_sub::get_disp()
{
	return disp;
}

void ht_hex_sub::set_disp(uint Disp)
{
	if (Disp >= 0 && Disp < line_length) {
		disp = Disp;
	}
}


bool ht_hex_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	if (offset >= fofs && offset < fofs+fsize) {
		clear_line_id(line_id);
		FileOfs o = offset - fofs;
		FileOfs begin;
		if (o < disp) {
			begin = 0;
		} else {
			begin = o - (o - disp)%line_length;
		}
		line_id->id1 = begin >> 32;
		line_id->id2 = begin;
		line_id->id3 = uid;
		return true;
	}
	return false;
}

bool ht_hex_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *ofs)
{
	*ofs = (uint64(line_id.id1) << 32) + line_id.id2 + fofs;
	return true;
}

bool ht_hex_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	if (line_id.id3 != uid || maxlen < 1) return false;
	FileOfs ofs = (uint64(line_id.id1) << 32) + line_id.id2;
	uint c = MIN(uint64(line_length), fsize - ofs);
	if (c <= 0) return false;
	
	char *l = line;
	char *l_end = line+maxlen-1;
	l += ht_snprintf(l, l_end - l, "%08qx ", ofs + fofs);

	uint start = 0;
	if (ofs < disp) {
		start = line_length - disp;
	}
	
	uint end = line_length;
	if (c < line_length - start) {
		end = start + c;
	}

	ofs -= start;
	ofs += fofs;

	for (uint i = 0; i < line_length; i++) {
		if (i >= start && i < end) {
			l = tag_make_edit_byte(l, l_end - l, ofs);
		} else {
			l += ht_snprintf(l, l_end - l, "  ");
		}
		if (i+1 > start && i+1 < end) {
			if (i%8 == 7) {
				l = tag_make_edit_selvis(l, l_end-l, ofs, '-');
			} else {
				l = tag_make_edit_selvis(l, l_end-l, ofs, ' ');
			}
		} else {
			l += ht_snprintf(l, l_end - l, " ");
		}
		ofs++;
	}
	ofs -= line_length;
	l = tag_make_group(l, l_end-l);
	l += ht_snprintf(l, l_end - l, "|");
	for (uint i=0; i < line_length; i++) {
		if (i >= start && i < end) {
			l = tag_make_edit_char(l, l_end-l, ofs);
		} else {
			l += ht_snprintf(l, l_end-l , " ");
		}
		ofs++;
	}
	l += ht_snprintf(l, l_end - l, "|");
	*l = 0;
	return true;
}

void ht_hex_sub::first_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = 0;
	line_id->id2 = 0;
	line_id->id3 = uid;
}

void ht_hex_sub::last_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	FileOfs ofs = fsize - 1;
	if (!fsize || ofs < disp) {
		ofs = 0;
	} else {
		ofs = ofs - (ofs-disp)%line_length;
	}
	line_id->id1 = ofs >> 32;
	line_id->id2 = ofs;
	line_id->id3 = uid;
}

int ht_hex_sub::prev_line_id(LINE_ID *line_id, int n)
{
	if (line_id->id3 != uid) return 0;
	int c = 0;
	FileOfs o = (uint64(line_id->id1) << 32) + line_id->id2;
	while (n--) {
		if (!o) break;
		if (line_length > o) {
			o = 0;
		} else {
			o -= line_length;
		}
		c++;
	}
	if (o < disp) {
		o = 0;
	} else {
		o = o - (o-disp)%line_length;
	}
	line_id->id1 = o >> 32;
	line_id->id2 = o;
	return c;
}

int ht_hex_sub::next_line_id(LINE_ID *line_id, int n)
{
	if (line_id->id3 != uid) return 0;
	int c = 0;
	FileOfs o = (uint64(line_id->id1) << 32) + line_id->id2;
	if (o < disp && disp < fsize) {
		o = disp;
		c++;
		n--;
	}
	while (n--) {
		if (o + line_length >= fsize) break;
		o += line_length;
		c++;
	}
	if (o < disp) {
		o = 0;
	} else {
		o = o - (o-disp)%line_length;
	}
	line_id->id1 = o >> 32;
	line_id->id2 = o;
	return c;
}

/*
 *	CLASS ht_mask
 */

void ht_mask_sub::init(File *f, uint u)
{
	ht_sub::init(f);
	uid = u;
}

void ht_mask_sub::first_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = 0;
	line_id->id2 = uid;
}

bool ht_mask_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	if (line_id.id2 != uid) return false;
	const char *s = ((ht_data_tagstring *)masks[line_id.id1])->value;
	if (s) {
		tag_strcpy(line, maxlen, s);
		return true;
	}
	return false;
}

void ht_mask_sub::last_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	line_id->id1 = masks.count()-1;
	line_id->id2 = uid;
}

int ht_mask_sub::next_line_id(LINE_ID *line_id, int n)
{
	int r=n;
	if (line_id->id2 != uid) return 0;
	int c = masks.count();
	ID i1 = line_id->id1;
	i1 += n;
	if ((int)i1 > c-1) {
		r -= i1-c+1;
		i1 = c-1;
	}
	if (r) line_id->id1 = i1;
	return r;
}

int ht_mask_sub::prev_line_id(LINE_ID *line_id, int n)
{
	int r;
	if (line_id->id2 != uid) return 0;
	ID i1 = line_id->id1;
	if (i1 < (uint32)n) {
		r = i1;
		i1 = 0;
	} else {
		r = n;
		i1 -= n;
	}
	if (r) line_id->id1 = i1;
	return r;
}

void ht_mask_sub::add_mask(const char *tagstr)
{
	masks += new ht_data_tagstring(tagstr);
}

void ht_mask_sub::add_mask_table(char **tagstr)
{
	while (*tagstr) add_mask(*tagstr++);
}

void ht_mask_sub::add_staticmask(const char *statictag_str, FileOfs reloc, bool std_bigendian)
{
	char tag_str[1024];
	statictag_to_tag(statictag_str, tag_str, sizeof tag_str, reloc, std_bigendian);
	masks += new ht_data_tagstring(tag_str);
}

void ht_mask_sub::add_staticmask_table(char **statictag_table, FileOfs reloc, bool std_bigendian)
{
	while (*statictag_table) add_staticmask(*(statictag_table++), reloc, std_bigendian);
}

#define ht_MASK_STD_INDENT	50

void ht_mask_sub::add_staticmask_ptable(ht_mask_ptable *statictag_ptable, FileOfs reloc, bool std_bigendian)
{
	char s[1024]; /* FIXME: possible buffer overflow */
	while (statictag_ptable->desc || statictag_ptable->fields) {
		s[0] = 0;
		if (statictag_ptable->desc) strcpy(s, statictag_ptable->desc);
		int n = strlen(s);
		while (n < ht_MASK_STD_INDENT) {
			s[n]=' ';
			n++;
		}
		s[n]=0;
		if (statictag_ptable->fields) strcat(s, statictag_ptable->fields);

		add_staticmask(s, reloc, std_bigendian);
		
		statictag_ptable++;
	}
}

/*
 *	CLASS ht_layer_sub
 */

void ht_layer_sub::init(File *file, ht_sub *s, bool own_s)
{
	ht_sub::init(file);
	sub = s;
	own_sub = own_s;
}

void ht_layer_sub::done()
{
	if (own_sub) {
		sub->done();
		delete sub;
	}
	ht_sub::done();
}

bool ht_layer_sub::closest_line_id(LINE_ID *line_id)
{
	return sub->closest_line_id(line_id);
}

bool ht_layer_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	return sub->convert_ofs_to_id(offset, line_id);
}

bool ht_layer_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset)
{
	return sub->convert_id_to_ofs(line_id, offset);
}

void ht_layer_sub::first_line_id(LINE_ID *line_id)
{
	return sub->first_line_id(line_id);
}

bool ht_layer_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	return sub->getline(line, maxlen, line_id);
}

void ht_layer_sub::handlemsg(htmsg *msg)
{
	sub->handlemsg(msg);
}

void ht_layer_sub::last_line_id(LINE_ID *line_id)
{
	return sub->last_line_id(line_id);
}

int ht_layer_sub::next_line_id(LINE_ID *line_id, int n)
{
	return sub->next_line_id(line_id, n);
}

int ht_layer_sub::prev_line_id(LINE_ID *line_id, int n)
{
	return sub->prev_line_id(line_id, n);
}

bool ht_layer_sub::ref(LINE_ID *id)
{
	return sub->ref(id);
}

ht_search_result *ht_layer_sub::search(ht_search_request *search, FileOfs start, FileOfs end)
{
	return sub->search(search, start, end);
}

/*
 *	CLASS ht_collapsable_sub
 */

static ID ht_collapsable_sub_globalfaddr = -1;

void ht_collapsable_sub::init(File *file, ht_sub *sub, bool own_sub, const char *ns, bool c)
{
	ht_layer_sub::init(file, sub, own_sub);
	nodestring = ht_strdup(ns);
	collapsed = c;
	ht_layer_sub::first_line_id(&fid);
	clear_line_id(&myfid);
	myfid.id1 = 0;
	myfid.id2 = ht_collapsable_sub_globalfaddr--;
}

void ht_collapsable_sub::done()
{
	free(nodestring);
	ht_layer_sub::done();
}

bool ht_collapsable_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
/*	if (offset==myfaddr) {
		first_line_id(id1, id2);
		return true;
	} else*/
	// FIXME: The Right Thing ?
	if (!collapsed) {
		return ht_layer_sub::convert_ofs_to_id(offset, line_id);
	}
	return false;
}

bool ht_collapsable_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset)
{
	return ht_layer_sub::convert_id_to_ofs(line_id, offset);
}

void ht_collapsable_sub::first_line_id(LINE_ID *line_id)
{
	clear_line_id(line_id);
	*line_id = myfid;
}

bool ht_collapsable_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	char *l = line;
	char *l_end = line+maxlen;
	if (compeq_line_id(line_id, myfid)) {
		line += ht_snprintf(line, l_end-l, "[%c] ", collapsed ? '+' : '-');
		line = tag_make_ref(line, l_end-l, myfid.id1, myfid.id2, 0, 0, nodestring);
		*line = 0;
		return true;
	} else if (collapsed) {
		return false;
	}
	if (ht_layer_sub::getline(line, maxlen, line_id)) {
		if (maxlen < 3) {
			return false;
		}
		int l = tag_strlen(line)+1;
		memmove(line+2, line, MIN(l, maxlen-2));
		line[0]=' ';
		line[1]=' ';
		return true;
	}
	return false;
}

void ht_collapsable_sub::last_line_id(LINE_ID *line_id)
{
	if (collapsed) return first_line_id(line_id); else
		return ht_layer_sub::last_line_id(line_id);
}

int ht_collapsable_sub::next_line_id(LINE_ID *line_id, int n)
{
	if (collapsed) return 0;
	int r=0;
	LINE_ID t;
	if (compeq_line_id(*line_id, myfid)) {
		ht_layer_sub::first_line_id(&t);
		n--;
		r++;
	} else {
		t = *line_id;
	}
	if (n) r+=ht_layer_sub::next_line_id(&t, n);
	if (r) {
		*line_id = t;
	}
	return r;
}

int ht_collapsable_sub::prev_line_id(LINE_ID *line_id, int n)
{
	if (collapsed) return 0;
	if (compeq_line_id(*line_id, myfid)) return 0;
	int r=ht_layer_sub::prev_line_id(line_id, n);
	if (compeq_line_id(*line_id, fid) && (r<n)) {
		*line_id = myfid;
		r++;
	}
	return r;
}

bool ht_collapsable_sub::ref(LINE_ID *id)
{
	if (compeq_line_id(*id, myfid)) {
		collapsed = !collapsed;
		return true;
	}
	if (!collapsed) return ht_layer_sub::ref(id);
	return false;
}

ht_search_result *ht_collapsable_sub::search(ht_search_request *search, FileOfs start, FileOfs end)
{
	if (collapsed) return NULL;
	return ht_layer_sub::search(search, start, end);
}

/*
 *	CLASS ht_group_sub
 */

void ht_group_sub::init(File *file)
{
	ht_sub::init(file);
	subs = new Array(true);
}

void ht_group_sub::done()
{
	delete subs;
	ht_sub::done();
}

bool ht_group_sub::convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id)
{
	return false;
}

bool ht_group_sub::convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset)
{
	return false;
}

void ht_group_sub::first_line_id(LINE_ID *line_id)
{
	ht_sub *s = (ht_sub*)(*subs)[0];
	if (s) s->first_line_id(line_id);
}

bool ht_group_sub::getline(char *line, int maxlen, const LINE_ID line_id)
{
	ht_sub *s;
	uint c = subs->count();
	for (uint i=0; i<c; i++) {
		s = (ht_sub*)(*subs)[i];
		if (s->getline(line, maxlen, line_id)) return true;
	}
	return false;
}

void ht_group_sub::handlemsg(htmsg *msg)
{
	ht_sub::handlemsg(msg);
}

void ht_group_sub::last_line_id(LINE_ID *line_id)
{
	ht_sub *s = (ht_sub*)(*subs)[subs->count()-1];
	if (s) s->last_line_id(line_id);
}

int ht_group_sub::next_line_id(LINE_ID *line_id, int n)
{
	ht_sub *s;
	uint c=subs->count();
	int on=n;
	for (uint i=0; i<c; i++) {
		s=(ht_sub*)(*subs)[i];
		LINE_ID t;
		s->last_line_id(&t);
		if (compeq_line_id(t, *line_id)) {
			s=(ht_sub*)(*subs)[i+1];
			if (s) {
				s->first_line_id(line_id);
				n--;
			}
		} else {
			n -= s->next_line_id(line_id, n);
		}
		if (!n) break;
	}
	return on - n;
}

int ht_group_sub::prev_line_id(LINE_ID *line_id, int n)
{
	ht_sub *s;
	uint c=subs->count();
	int on=n;
	for (uint i=0; i<c; i++) {
		s=(ht_sub*)(*subs)[i];
		LINE_ID t;
		s->first_line_id(&t);
		if (compeq_line_id(t, *line_id)) {
			s=(ht_sub*)(*subs)[i-1];
			if (s) {
				s->last_line_id(line_id);
				n--;
			}
		} else {
			n -= s->prev_line_id(line_id, n);
		}
		if (!n) break;
	}
	return on - n;
}

bool ht_group_sub::ref(LINE_ID *id)
{
	foreach(ht_sub, s, *subs, 
		if (s->ref(id)) return true;
	);
	return false;
}

ht_search_result *ht_group_sub::search(ht_search_request *search, FileOfs start, FileOfs end)
{
	return NULL;
}

void ht_group_sub::insertsub(ht_sub *sub)
{
	subs->insert(sub);
}




BUILDER(UFORMAT_VIEWER_VSTATE, ht_uformat_viewer_vstate, Object)

bool init_format()
{
	REGISTER(UFORMAT_VIEWER_VSTATE, ht_uformat_viewer_vstate)
	return true;
}

void done_format()
{
	UNREGISTER(UFORMAT_VIEWER_VSTATE, ht_uformat_viewer_vstate)
}
