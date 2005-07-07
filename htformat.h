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
#include "htdata.h"
#include "htobj.h"
#include "htstring.h"
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
};

union viewer_pos {
	uformat_viewer_pos u;
};

/*
 *	CLASS ht_search_request
 */
 
class ht_search_request: public ht_data {
public:
	uint search_class;
	uint type;
	uint flags;
	
			ht_search_request(UINT search_class, uint type, uint flags);
};

/*
 *	CLASS ht_search_result
 */
 
class ht_search_result: public ht_data {
public:
	uint search_class;

			ht_search_result(UINT search_class);
};

/*
 *	CLASS ht_physical_search_result
 */
 
class ht_physical_search_result: public ht_search_result {
public:
	FILEOFS offset;
	uint size;
	
			ht_physical_search_result();
};

/*
 *	CLASS ht_visual_search_result
 */
 
class ht_visual_search_result: public ht_search_result {
public:
	viewer_pos pos;
	uint xpos;
	uint length;
	
			ht_visual_search_result();
};

/*
 *	formats
 */

class ht_format_loc: public ht_data {
public:
	char *name;
	FILEOFS start;
	uint length;
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
	virtual	char *func(UINT i, bool execute);
public:
			void init(bounds *b, const char *desc, uint caps);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
};

/*
 *	CLASS ht_format_viewer
 */

class ht_format_viewer: public ht_viewer {
protected:
	ht_streamfile *file;
// last search (request)
	ht_search_request *last_search_request;
	bool last_search_physical;
	union {
		FILEOFS last_search_end_ofs;
		viewer_pos last_search_end_pos;
	};

/* new */
	virtual	bool compeq_viewer_pos(viewer_pos *a, viewer_pos *b);

	virtual	void vstate_restore(ht_data *view_state);
	virtual	ht_data *vstate_create();
	
	virtual	bool next_logical_pos(viewer_pos pos, viewer_pos *npos);
	virtual	bool next_logical_offset(FILEOFS ofs, FILEOFS *nofs);
public:
	ht_format_group *format_group;

			void init(bounds *b, const char *desc, uint caps, ht_streamfile *file, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
/* new */
	virtual	bool pos_to_offset(viewer_pos pos, FILEOFS *ofs);
	virtual	bool offset_to_pos(FILEOFS ofs, viewer_pos *pos);
	/* position indicator string */
	virtual	void get_pindicator_str(char *buf);
	/* scrollbar pos */
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	/* physical file location(s) */
	virtual	void loc_enum_start();
	virtual	bool loc_enum_next(ht_format_loc *loc);

	/* physical address (offset) functions */
	virtual	bool get_current_offset(FILEOFS *ofs);
	virtual	bool goto_offset(FILEOFS ofs, bool save_vstate);
	virtual	uint pread(FILEOFS ofs, void *buf, uint size);
	virtual	ht_search_result *psearch(ht_search_request *search, FILEOFS start, FILEOFS end);
	virtual	void pselect_add(FILEOFS start, FILEOFS end);
	virtual	void pselect_get(FILEOFS *start, FILEOFS *end);
	virtual	void pselect_set(FILEOFS start, FILEOFS end);
	virtual	uint pwrite(FILEOFS ofs, void *buf, uint size);
	virtual	bool string_to_offset(char *string, FILEOFS *ofs);
	virtual	bool qword_to_offset(qword q, FILEOFS *ofs);

	virtual	bool get_current_real_offset(FILEOFS *ofs);

	/* visual address (viewer pos) functions */
	virtual	bool get_current_pos(viewer_pos *pos);
	virtual	bool goto_pos(viewer_pos pos, bool save_vstate);
	virtual	uint vread(viewer_pos pos, void *buf, uint size);
	virtual	ht_search_result *vsearch(ht_search_request *search, viewer_pos start, viewer_pos end);
	virtual	void vselect_add(viewer_pos start, viewer_pos end);
	virtual	void vselect_get(viewer_pos *start, viewer_pos *end);
	virtual	void vselect_set(viewer_pos start, viewer_pos end);
	virtual	uint vwrite(viewer_pos pos, void *buf, uint size);
	virtual	bool string_to_pos(char *string, viewer_pos *pos);
	virtual	bool qword_to_pos(qword q, viewer_pos *pos);

	/* string evaluation */
	virtual	int func_handler(eval_scalar *result, char *name, eval_scalarlist *params);
	virtual	int symbol_handler(eval_scalar *result, char *name);

	/* search related */
			bool continue_search();
	virtual	bool show_search_result(ht_search_result *result);
	/* misc */
			void clear_viewer_pos(viewer_pos *p);
			ht_streamfile *get_file();
			bool string_to_qword(char *string, qword *q);
			bool vstate_save();
};

/*
 *	CLASS ht_format_viewer_entry
 */

class ht_format_viewer_entry: public ht_data {
public:
	ht_view *instance;
	format_viewer_if *interface;
};

/*
 *	CLASS ht_format_group
 */

class ht_format_group: public ht_format_viewer {
protected:
	ht_clist *format_views;
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
			void init(bounds *b, int options, const char *desc, ht_streamfile *file, bool own_file, bool editable_file, format_viewer_if **ifs, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	int childcount();
	virtual	int focus(ht_view *view);
	virtual	char *func(UINT i, bool execute);
			void getbounds(bounds *b);
	virtual 	ht_view *getfirstchild();
	virtual	ht_view *getselected();
	virtual	void get_pindicator_str(char *buf);
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	bool get_vscrollbar_pos(int *pstart, int *psize);
	virtual	void handlemsg(htmsg *msg);
	virtual	void move(int x, int y);
	virtual	void receivefocus();
	virtual	void redraw();
	virtual	void releasefocus();
	virtual	void resize(int rw, int rh);
	virtual	void setgroup(ht_group *group);
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
	FILEOFS cursor_select_start;
	uint32 cursor_select_cursor_length;
	int cursor_tag_micropos;
/* selection*/
	FILEOFS sel_start;
	FILEOFS sel_end;
/* visual info */
	int cursor_visual_xpos;
	int cursor_visual_length;
/* misc info */
	int cursor_tag_class;
	union {
		FILEOFS cursor_tag_offset;
		struct {
			LINE_ID id;
		} cursor_tag_id;
	};
	char cursor_line[1024];	/* FIXME: possible buffer overflow ! */
/* tag palettes */
	palette tagpal;
/**/
	int xscroll;

	bool uf_initialized;

	bool isdirty_cursor_line;

/* overwritten */
	virtual	char *func(UINT i, bool execute);
	virtual	bool next_logical_pos(viewer_pos pos, viewer_pos *npos);
	virtual	bool next_logical_offset(FILEOFS ofs, FILEOFS *nofs);
	virtual	ht_data *vstate_create();
	virtual	void vstate_restore(ht_data *view_state);
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
	bool find_first_edit_tag_with_offset(uformat_viewer_pos *p, int limit, FILEOFS offset);
	void focus_cursor();
	vcp getcolor_tag(UINT pal_index);
	int get_current_tag(char **tag);
	int get_current_tag_size(uint32 *size);
	vcp get_tag_color_edit(FILEOFS tag_offset, uint size, bool atcursoroffset, bool iscursor);
	int next_line(uformat_viewer_pos *p, int n);
	int prev_line(uformat_viewer_pos *p, int n);
	void print_tagstring(int x, int y, int maxlen, int xscroll, char *tagstring, bool cursor_in_line);
	virtual int ref();
	int ref_desc(ID id, FILEOFS offset, uint size, bool bigendian);
	int ref_flags(ID id, FILEOFS offset);
	virtual int ref_sel(LINE_ID *id);
	virtual void reloadpalette();
	uint render_tagstring(char *chars, vcp *colors, uint maxlen, char *tagstring, bool cursor_in_line);
	void render_tagstring_desc(char **string, int *length, vcp *tag_color, char *tag, uint size, bool bigendian, bool is_cursor);
	uint render_tagstring_single(char *chars, vcp *colors, uint maxlen, uint offset, char *text, uint len, vcp color);
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
	
			void init(bounds *b, const char *desc, int caps, ht_streamfile *file, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	void clear_viewer_pos(viewer_pos *p);
	virtual	void draw();
	virtual	bool get_current_offset(FILEOFS *offset);
	virtual	bool get_current_pos(viewer_pos *pos);
	virtual	bool goto_offset(FILEOFS offset, bool save_vstate);
	virtual	bool goto_pos(viewer_pos pos, bool save_vstate);
	virtual	void handlemsg(htmsg *msg);
	virtual	ht_search_result *psearch(ht_search_request *search, FILEOFS start, FILEOFS end);
	virtual	void pselect_add(FILEOFS start, FILEOFS end);
	virtual	void pselect_get(FILEOFS *start, FILEOFS *end);
	virtual	void pselect_set(FILEOFS start, FILEOFS end);
	virtual	uint pwrite(FILEOFS ofs, void *buf, uint size);
	virtual	bool qword_to_offset(qword q, FILEOFS *ofs);
	virtual	ht_search_result *vsearch(ht_search_request *search, viewer_pos start, viewer_pos end);
/* new */
	virtual	bool compeq_viewer_pos(viewer_pos *a, viewer_pos *b);
			void complete_init();
	virtual	void insertsub(ht_sub *sub);
			void sendsubmsg(int msg);
			void sendsubmsg(htmsg *msg);
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
	ht_streamfile *file;
public:
	ht_uformat_viewer *uformat_viewer;
	ht_sub *prev, *next;

			void init(ht_streamfile *file);
	virtual	void done();
/* new */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FILEOFS *offset);
	virtual	bool closest_line_id(LINE_ID *line_id);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
};

/*
 *	CLASS ht_linear_sub
 */

class ht_linear_sub: public ht_sub {
protected:
	FILEOFS fofs;
	uint32 fsize;
public:
			void init(ht_streamfile *file, FILEOFS offset, int size);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
};

/*
 *	CLASS ht_hex_sub
 */

class ht_hex_sub: public ht_linear_sub {
protected:
	uint32 vaddrinc;
	uint32 balign;
	uint32 line_length;
	uint uid;
public:
			void init(ht_streamfile *file, FILEOFS ofs, uint32 size, uint line_length, uint uid, uint32 vaddrinc=0);
	virtual 	void done();
			int	get_line_length();
			void	set_line_length(int line_length);
/* overwritten */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FILEOFS *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
};

/*
 *	CLASS ht_mask_sub
 */

struct ht_mask_ptable {
	char *desc;
	char *fields;
};
 
class ht_mask_sub: public ht_sub {
protected:
	ht_string_list *masks;
	char temp[512];	/* FIXME: possible buffer overflow */
	uint uid;
public:
			void init(ht_streamfile *file, uint uid);
	virtual 	void done();
/* overwritten */
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
/* new */
	virtual 	void add_mask(char *tagstr);
	virtual 	void add_mask_table(char **tagstr);
	virtual 	void add_staticmask(char *statictag_str, FILEOFS reloc, bool std_bigendian);
	virtual 	void add_staticmask_table(char **statictag_table, FILEOFS reloc, bool std_bigendian);
	virtual 	void add_staticmask_ptable(ht_mask_ptable *statictag_ptable, FILEOFS reloc, bool std_bigendian);
};

/*
 *	CLASS ht_layer_sub
 */

class ht_layer_sub: public ht_sub {
protected:
	ht_sub *sub;
	bool own_sub;
public:
			void init(ht_streamfile *file, ht_sub *sub, bool own_sub);
	virtual	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FILEOFS *offset);
	virtual	bool closest_line_id(LINE_ID *line_id);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
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
			void init(ht_streamfile *file, ht_sub *sub, bool own_sub, char *nodename, bool collapsed);
	virtual	void done();
/* new */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FILEOFS *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
};

/*
 *	CLASS ht_group_sub
 */

class ht_group_sub: public ht_sub {
protected:
	ht_clist *subs;
public:
			void init(ht_streamfile *file);
	virtual	void done();
/* overwritten */
	virtual	bool convert_ofs_to_id(const FILEOFS offset, LINE_ID *line_id);
	virtual	bool convert_id_to_ofs(const LINE_ID line_id, FILEOFS *offset);
	virtual	void first_line_id(LINE_ID *line_id);
	virtual	bool getline(char *line, const LINE_ID line_id);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(LINE_ID *line_id);
	virtual	int next_line_id(LINE_ID *line_id, int n);
	virtual	int prev_line_id(LINE_ID *line_id, int n);
	virtual	bool ref(LINE_ID *id);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
/* new */
			void insertsub(ht_sub *sub);
};

/*
 *	CLASS ht_data_tagstring
 */

class ht_data_tagstring: public ht_data_string {
public:
			ht_data_tagstring(char *tagstr=0);
	virtual	~ht_data_tagstring();
};

ht_search_result *linear_expr_search(ht_search_request *search, FILEOFS start, FILEOFS end, ht_sub *sub, ht_uformat_viewer *ufv, FILEOFS fofs, uint32 fsize);
ht_search_result *linear_bin_search(ht_search_request *search, FILEOFS start, FILEOFS end, ht_streamfile *file, FILEOFS fofs, uint32 fsize);

void clear_line_id(LINE_ID *l);
bool compeq_line_id(const LINE_ID &a, const LINE_ID &b);

#endif /* __HTFORMAT_H__ */

