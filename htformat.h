/*
 *	HT Editor
 *	htformat.h
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

#ifndef __HTFORMAT_H__
#define __HTFORMAT_H__

class ht_format_group;

#include "evalx.h"
#include "data.h"
#include "htobj.h"
#include "strtools.h"
#include "formats.h"

class ht_sub;

struct LINE_ID {
	ID id1;
	ID id2;
	ID id3;
	ID id4;
	ID id5;
};

struct uformat_viewer_pos {
	/* which line ? */
	ht_sub *sub;
	LINE_ID line_id;
	/* which tag ? */
	int tag_group;
	int tag_idx;

	void load(ObjectStream &s);
	void store(ObjectStream &s) const;
};

union viewer_pos {
	uformat_viewer_pos u;
};

// search classes
#define SC_PHYSICAL			0    // search in underlying binary data
#define SC_VISUAL			1    // search in displayed text

/*
 *	CLASS ht_search_request
 */
 
class ht_search_request: public Object {
public:
	uint search_class;
	uint type;
	uint flags;
	
		ht_search_request(uint search_class, uint type, uint flags);
	virtual ht_search_request *clone() const = 0;
};

/*
 *	CLASS ht_search_result
 */
 
class ht_search_result: public Object {
public:
	uint search_class;

	ht_search_result(uint asearch_class): search_class(asearch_class) {}
};

/*
 *	CLASS ht_physical_search_result
 */
 
class ht_physical_search_result: public ht_search_result {
public:
	FileOfs offset;
	uint size;
	
	ht_physical_search_result(): ht_search_result(SC_PHYSICAL) {}
};

/*
 *	CLASS ht_visual_search_result
 */
 
class ht_visual_search_result: public ht_search_result {
public:
	viewer_pos pos;
	uint xpos;
	uint length;
	
	ht_visual_search_result(): ht_search_result(SC_VISUAL) {}
};

/*
 *	formats
 */

class ht_format_loc: public Object {
public:
	const char *name;
	FileOfs start;
	FileOfs length;
};

/*
 *	CLASS ht_viewer
 */

/* caps */
#define VC_NULL		0x0000
#define VC_EDIT		0x0001
#define VC_GOTO		0x0002
#define VC_SEARCH		0x0004
#define VC_REPLACE		0x0008
#define VC_RESIZE		0x0010

class ht_viewer: public ht_view {
protected:
	uint caps;

/* new */
	virtual	const char *func(uint i, bool execute);
public:
		void init(Bounds *b, const char *desc, uint caps);
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_format_viewer
 */

class ht_format_viewer: public ht_viewer {
protected:
	File *file;
// last search (request)
	ht_search_request *last_search_request;
	bool last_search_physical;
	union {
		FileOfs last_search_end_ofs;
		viewer_pos last_search_end_pos;
	};

/* new */
	virtual	bool compeq_viewer_pos(viewer_pos *a, viewer_pos *b);

	virtual	void vstate_restore(Object *view_state);
	virtual	Object *vstate_create();
	
	virtual	bool next_logical_pos(viewer_pos pos, viewer_pos *npos);
	virtual	bool next_logical_offset(FileOfs ofs, FileOfs *nofs);
public:
	ht_format_group *format_group;

		void init(Bounds *b, const char *desc, uint caps, File *file, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
/* new */
	virtual	bool pos_to_offset(viewer_pos pos, FileOfs *ofs);
	virtual	bool offset_to_pos(FileOfs ofs, viewer_pos *pos);
	/* position indicator string */
	virtual	int get_pindicator_str(char *buf, int max_len);
	/* scrollbar pos */
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	/* physical file location(s) */
	virtual	void loc_enum_start();
	virtual	bool loc_enum_next(ht_format_loc *loc);

	/* physical address (offset) functions */
	virtual	bool get_current_offset(FileOfs *ofs);
	virtual	bool goto_offset(FileOfs ofs, bool save_vstate);
	virtual	uint pread(FileOfs ofs, void *buf, uint size);
	virtual	ht_search_result *psearch(ht_search_request *search, FileOfs start, FileOfs end);
	virtual	void pselect_add(FileOfs start, FileOfs end);
	virtual	void pselect_get(FileOfs *start, FileOfs *end);
	virtual	void pselect_set(FileOfs start, FileOfs end);
	virtual	uint pwrite(FileOfs ofs, void *buf, uint size);
	virtual	bool string_to_offset(char *string, FileOfs *ofs);
	virtual	bool qword_to_offset(uint64 q, FileOfs *ofs);

	virtual	bool get_current_real_offset(FileOfs *ofs);

	/* visual address (viewer pos) functions */
	virtual	bool get_current_pos(viewer_pos *pos);
	virtual	bool goto_pos(viewer_pos pos, bool save_vstate);
	virtual	uint vread(viewer_pos pos, void *buf, uint size);
	virtual	ht_search_result *vsearch(ht_search_request *search, viewer_pos start, viewer_pos end);
	virtual	void vselect_add(viewer_pos start, viewer_pos end);
	virtual	void vselect_get(viewer_pos *start, viewer_pos *end);
	virtual	void vselect_set(viewer_pos start, viewer_pos end);
	virtual	uint vwrite(viewer_pos pos, void *buf, uint size);
	virtual	bool string_to_pos(const char *string, viewer_pos *pos);
	virtual	bool qword_to_pos(uint64 q, viewer_pos *pos);

	/* string evaluation */
	virtual	bool func_handler(eval_scalar *result, char *name, eval_scalarlist *params);
	virtual	bool symbol_handler(eval_scalar *result, char *name);

	/* search related */
		bool continue_search();
	virtual	bool show_search_result(ht_search_result *result);
	/* misc */
		void clear_viewer_pos(viewer_pos *p);
		File *get_file();
		bool string_to_qword(const char *string, uint64 *q);
		bool vstate_save();
};

/*
 *	CLASS ht_format_group
 */

class ht_format_group: public ht_format_viewer {
protected:
	Container *format_views;
	void *shared_data;
	bool editable_file;
	bool own_file;
	ht_xgroup *xgroup;

/* new */
		void init_ifs(format_viewer_if **ifs);
		void done_ifs();
			
		bool init_if(format_viewer_if *i);
		bool done_if(format_viewer_if *i, ht_view *v);
			
		bool edit();
public:
		void init(Bounds *b, int options, const char *desc, File *file, bool own_file, bool editable_file, format_viewer_if **ifs, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	int childcount() const;
	virtual	bool focus(ht_view *view);
	virtual	const char *func(uint i, bool execute);
		void getbounds(Bounds *b);
	virtual ht_view *getfirstchild();
	virtual	ht_view *getselected();
	virtual	int get_pindicator_str(char *buf, int max_len);
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	virtual	void handlemsg(htmsg *msg);
	virtual	void move(int x, int y);
	virtual	void receivefocus();
	virtual	void redraw();
	virtual	void releasefocus();
	virtual	void resize(int rw, int rh);
	virtual	void setgroup(ht_group *group);
	virtual	bool func_handler(eval_scalar *result, char *name, eval_scalarlist *params);
	virtual	bool symbol_handler(eval_scalar *result, char *name);
/* new */
	virtual	void insert(ht_view *view);
		void remove(ht_view *view);
		void *get_shared_data();
};

/*
 *	CLASS ht_uformat_viewer
 */

#define cursor_state_visible		0
#define cursor_state_invisible	1
#define cursor_state_disabled		2

class ht_uformat_viewer: public ht_format_viewer {
protected:
	ht_sub *first_sub, *last_sub;
/* top line position */
	uformat_viewer_pos top;
/* cursor line and state and tag position */
	uformat_viewer_pos cursor;
	int cursor_ypos;
	int cursor_state;
	bool cursor_select;
	FileOfs cursor_select_start;
	uint32 cursor_select_cursor_length;
	int cursor_tag_micropos;
/* selection*/
	FileOfs sel_start;
	FileOfs sel_end;
/* visual info */
	int cursor_visual_xpos;
	int cursor_visual_length;
/* misc info */
	int cursor_tag_class;
//	union {
		FileOfs cursor_tag_offset;
		struct {
			LINE_ID id;
		} cursor_tag_id;
//	};
	char cursor_line[1024];
/* tag palettes */
	palette tagpal;
/**/
	int xscroll;

	bool uf_initialized;

	bool isdirty_cursor_line;

/* overwritten */
	virtual	const char *func(uint i, bool execute);
	virtual	bool next_logical_pos(viewer_pos pos, viewer_pos *npos);
	virtual	bool next_logical_offset(FileOfs ofs, FileOfs *nofs);
	virtual	Object *vstate_create();
	virtual	void vstate_restore(Object *view_state);
/* new */
	int address_input(const char *title, char *buf, int buflen, uint32 histid);
	void adjust_cursor_group();
	void adjust_cursor_idx();
	int center_view(viewer_pos p);
	void clear_subs();
	bool compeq_viewer_pos(uformat_viewer_pos *a, uformat_viewer_pos *b);
	void check_cursor_visibility();
	void clear_viewer_pos(uformat_viewer_pos *p);
	int cursor_left();
	int cursor_right();
	int cursor_down(int n);
	int cursor_up(int n);
	int cursor_home();
	int cursor_end();
	void cursor_tab();
	void cursorline_dirty();
	void cursorline_get();
	virtual int cursormicro_forward();
	virtual int cursormicro_backward();
	virtual int cursormicroedit_forward();
	bool edit_end();
	bool edit_input(byte b);
	int edit_input_c2h(byte b);
	int edit_input_c2d(byte b);
	void edit_input_correctpos();
	bool edit_start();
	bool edit_update();
	bool edit();
	bool find_first_tag(uformat_viewer_pos *p, int limit);
	bool find_first_edit_tag_with_offset(uformat_viewer_pos *p, int limit, FileOfs offset);
	void focus_cursor();
	vcp getcolor_tag(uint pal_index);
	bool get_current_tag(char **tag);
	bool get_current_tag_size(uint32 *size);
	vcp get_tag_color_edit(FileOfs tag_offset, uint size, bool atcursoroffset, bool iscursor);
	int next_line(uformat_viewer_pos *p, int n);
	int prev_line(uformat_viewer_pos *p, int n);
	void print_tagstring(int x, int y, int maxlen, int xscroll, char *tagstring, bool cursor_in_line);
	virtual bool ref();
	bool ref_desc(ID id, FileOfs offset, uint size, bool bigendian);
	bool ref_flags(ID id, FileOfs offset);
	virtual bool ref_sel(LINE_ID *id);
	virtual void reloadpalette();
	uint render_tagstring(char *chars, vcp *colors, uint maxlen, char *tagstring, bool cursor_in_line);
	void render_tagstring_desc(const char **string, int *length, vcp *tag_color, char *tag, uint size, bool bigendian, bool is_cursor);
	uint render_tagstring_single(char *chars, vcp *colors, uint maxlen, uint offset, const char *text, uint len, vcp color);
	void scroll_up(int n);
	void scroll_down(int n);
	void select_mode_off();
	void select_mode_on();
	void select_mode_pre();
	void select_mode_post(bool lastpos);
	bool set_cursor(uformat_viewer_pos p);
	void update_micropos();
	void update_misc_info();
	void update_visual_info();
	void update_ypos();
public:
	uint search_caps;
	
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	void clear_viewer_pos(viewer_pos *p);
	virtual	void draw();
	virtual	bool get_current_offset(FileOfs *offset);
	virtual	bool get_current_pos(viewer_pos *pos);
	virtual	bool goto_offset(FileOfs offset, bool save_vstate);
	virtual	bool goto_pos(viewer_pos pos, bool save_vstate);
	virtual	void handlemsg(htmsg *msg);
	virtual	ht_search_result *psearch(ht_search_request *search, FileOfs start, FileOfs end);
	virtual	void pselect_add(FileOfs start, FileOfs end);
	virtual	void pselect_get(FileOfs *start, FileOfs *end);
	virtual	void pselect_set(FileOfs start, FileOfs end);
	virtual	uint pwrite(FileOfs ofs, void *buf, uint size);
	virtual	bool qword_to_offset(uint64 q, FileOfs *ofs);
	virtual	ht_search_result *vsearch(ht_search_request *search, viewer_pos start, viewer_pos end);
/* new */
	virtual	bool compeq_viewer_pos(viewer_pos *a, viewer_pos *b);
		void complete_init();
	virtual	void insertsub(ht_sub *sub);
		void sendsubmsg(int msg);
		void sendsubmsg(htmsg *msg);
		int sub_to_idx(const ht_sub *sub) const;
		ht_sub *idx_to_sub(int idx) const;
};

/*
 *	CLASS ht_sub
 */

/* search results */
#define SR_NOT_FOUND		0
#define SR_FOUND			1
#define SR_NOT_SUPPORTED		2

class ht_sub: public Object {
protected:
	File *file;
public:
	ht_uformat_viewer *uformat_viewer;
	ht_sub *prev, *next;

		void init(File *file);
/* new */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	bool closest_line_id(LINE_ID *line_id);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FileOfs start, FileOfs end);
};

/*
 *	CLASS ht_linear_sub
 */

class ht_linear_sub: public ht_sub {
protected:
	FileOfs fofs;
	FileOfs fsize;
public:
		void init(File *file, FileOfs offset, FileOfs size);
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	ht_search_result *search(ht_search_request *search, FileOfs start, FileOfs end);
};

/*
 *	CLASS ht_hex_sub
 */

class ht_hex_sub: public ht_linear_sub {
protected:
	uint disp;
	uint line_length;
	uint uid;
public:
		void init(File *file, FileOfs ofs, FileOfs size, uint line_length, uint uid, int disp=-1);
		int  get_line_length();
		void set_line_length(uint line_length);
		int  get_disp();
		void set_disp(uint disp);
	/* overwritten */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
};

/*
 *	CLASS ht_mask_sub
 */

struct ht_mask_ptable {
	const char *desc;
	const char *fields;
};
 
class ht_mask_sub: public ht_sub {
protected:
	Array masks;
	uint uid;
public:
		ht_mask_sub(): masks(true) {}
 
		void init(File *file, uint uid);
/* overwritten */
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
/* new */
	virtual	void add_mask(const char *tagstr);
	virtual	void add_mask_table(char **tagstr);
	virtual	void add_staticmask(const char *statictag_str, FileOfs reloc, bool std_bigendian);
	virtual	void add_staticmask_table(char **statictag_table, FileOfs reloc, bool std_bigendian);
	virtual	void add_staticmask_ptable(ht_mask_ptable *statictag_ptable, FileOfs reloc, bool std_bigendian);
};

/*
 *	CLASS ht_layer_sub
 */

class ht_layer_sub: public ht_sub {
protected:
	ht_sub *sub;
	bool own_sub;
public:
		void init(File *file, ht_sub *sub, bool own_sub = true);
	virtual	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	bool closest_line_id(LINE_ID *line_id);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FileOfs start, FileOfs end);
};

/*
 *	CLASS ht_collapsable_sub
 */

class ht_collapsable_sub: public ht_layer_sub {
protected:
	char *nodestring;
	bool collapsed;
	LINE_ID fid;
	LINE_ID myfid;
public:
		void init(File *file, ht_sub *sub, bool own_sub, const char *nodename, bool collapsed);
	virtual	void done();
/* new */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FileOfs start, FileOfs end);
};

/*
 *	CLASS ht_group_sub
 */

class ht_group_sub: public ht_sub {
protected:
	Container *subs;
public:
		void init(File *file);
	virtual	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FileOfs *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FileOfs start, FileOfs end);
/* new */
		void insertsub(ht_sub *sub);
};

ht_search_result *linear_expr_search(ht_search_request *search, FileOfs start, FileOfs end, ht_sub *sub, ht_uformat_viewer *ufv, FileOfs fofs, FileOfs fsize);
ht_search_result *linear_bin_search(ht_search_request *search, FileOfs start, FileOfs end, File *file, FileOfs fofs, FileOfs fsize);

void clear_line_id(LINE_ID *l);
bool compeq_line_id(const LINE_ID &a, const LINE_ID &b);

bool init_format();
void done_format();

#endif /* __HTFORMAT_H__ */

