/*
 *	HT Editor
 *	htformat.h
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

#ifndef __HTFORMAT_H__
#define __HTFORMAT_H__

class ht_format_group;

#include "htdata.h"
#include "htobj.h"
#include "htstring.h"
#include "formats.h"

typedef dword fmt_vaddress;

/*
 *	CLASS ht_search_request
 */
 
class ht_search_request: public ht_data {
public:
	UINT search_class;
	UINT type;
	UINT flags;
	
			ht_search_request(UINT search_class, UINT type, UINT flags);
};

/*
 *	CLASS ht_search_result
 */
 
class ht_search_result: public ht_data {
public:
	UINT search_class;

			ht_search_result(UINT search_class);
};

/*
 *	CLASS ht_physical_search_result
 */
 
class ht_physical_search_result: public ht_search_result {
public:
	FILEOFS offset;
	UINT size;
	
			ht_physical_search_result();
};

/*
 *	CLASS ht_visual_search_result
 */
 
class ht_visual_search_result: public ht_search_result {
public:
	fmt_vaddress address;
	UINT xpos;
	UINT length;
	
			ht_visual_search_result();
};

/*
 *	formats
 */

class ht_format_loc: public ht_data {
public:
	char *name;
	FILEOFS start;
	UINT length;
};

/*
 *	vstate
 */

class vstate: public ht_data {
public:
	ht_view *focused;		/* view to focus */
	ht_view *view;			/* view whose state has changed */
	void *data;			/* view's data */
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
	UINT caps;

/* new */
	virtual	char *func(UINT i, bool execute);
public:
			void init(bounds *b, char *desc, UINT caps);
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
	ht_stack *vs_history;
// last search (request)
	ht_search_request *last_search_request;
	bool last_search_physical;
	union {
		FILEOFS last_search_end_ofs;
		fmt_vaddress last_search_end_addr;
	};

/* new */
	virtual	void view_state_load(void *view_state);
	virtual	void *view_state_create();
	virtual	void view_state_destroy(void *view_state);
	
	virtual	bool next_logical_address(fmt_vaddress addr, fmt_vaddress *naddr);
	virtual	bool next_logical_offset(FILEOFS ofs, FILEOFS *nofs);
public:
	ht_format_group *format_group;
	
			void init(bounds *b, char *desc, UINT caps, ht_streamfile *file, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
/* new */
	virtual	bool address_to_offset(fmt_vaddress addr, FILEOFS *ofs);
	virtual	bool offset_to_address(FILEOFS ofs, fmt_vaddress *addr);
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
	virtual	bool goto_offset(FILEOFS ofs, ht_view *source_object);
	virtual	UINT pread(FILEOFS ofs, void *buf, UINT size);
	virtual	ht_search_result *psearch(ht_search_request *search, FILEOFS start, FILEOFS end);
	virtual	void pselect_add(FILEOFS start, FILEOFS end);
	virtual	void pselect_get(FILEOFS *start, FILEOFS *end);
	virtual	void pselect_set(FILEOFS start, FILEOFS end);
	virtual	UINT pwrite(FILEOFS ofs, void *buf, UINT size);
	virtual	bool string_to_offset(char *string, FILEOFS *ofs);

	/* virtual address functions */
	virtual	bool get_current_address(fmt_vaddress *addr);
	virtual	bool goto_address(fmt_vaddress addr, ht_view *source_object);
	virtual	UINT vread(fmt_vaddress addr, void *buf, UINT size);
	virtual	ht_search_result *vsearch(ht_search_request *search, fmt_vaddress start, fmt_vaddress end);
	virtual	void vselect_add(fmt_vaddress start, fmt_vaddress end);
	virtual	void vselect_get(fmt_vaddress *start, fmt_vaddress *end);
	virtual	void vselect_set(fmt_vaddress start, fmt_vaddress end);
	virtual	UINT vwrite(fmt_vaddress addr, void *buf, UINT size);
	virtual	bool string_to_address(char *string, fmt_vaddress *addr);
	
	/* search related */
			bool continue_search();
	virtual	bool show_search_result(ht_search_result *result);
	/* view state histories */
			bool push_vs_history(ht_view *focused);
			bool pop_vs_history();
	/* misc */
			ht_streamfile *get_file();
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
			void init(bounds *b, int options, char *desc, ht_streamfile *file, bool own_file, bool editable_file, format_viewer_if **ifs, ht_format_group *format_group);
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

class ht_sub;

class ht_uformat_viewer: public ht_format_viewer {
protected:
	ht_sub *first_sub, *last_sub;
/* top line position */
	ht_sub *top_sub;
	ID top_id1, top_id2;
/* cursor line and state and tag position */
	int cursor_ypos;
	int cursor_state;
	bool cursor_select;
	FILEOFS cursor_select_start;
	dword cursor_select_cursor_length;
	ht_sub *cursor_sub;
	ID cursor_id1, cursor_id2;
	int cursor_tag_idx;
	int cursor_tag_group;
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
			ID low, high;
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
	virtual	bool next_logical_address(fmt_vaddress addr, fmt_vaddress *naddr);
	virtual	bool next_logical_offset(FILEOFS ofs, FILEOFS *nofs);
	virtual	void view_state_load(void *view_state);
	virtual	void *view_state_create();
/* new */
	void adjust_cursor_group();
	void adjust_cursor_idx();
	int center_view(ht_sub *sub, ID id1, ID id2);
	void clear_subs();
	void check_cursor_visibility();
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
	int find_first_tag(ht_sub **sub, ID *id1, ID *id2, int limit);
	int find_first_edit_tag_with_offset(ht_sub **sub, ID *id1, ID *id2, int *tag_idx, int limit, FILEOFS offset);
	void focus_cursor();
	vcp getcolor_tag(UINT pal_index);
	int get_current_tag(char **tag);
	int get_current_tag_size(dword *size);
	vcp get_tag_color_edit(FILEOFS tag_offset, UINT size, bool atcursoroffset, bool iscursor);
	int next_line(ht_sub **sub, ID *id1, ID *id2, int n);
	int prev_line(ht_sub **sub, ID *id1, ID *id2, int n);
	void print_tagstring(int x, int y, int maxlen, int xscroll, char *tagstring, bool cursor_in_line);
	virtual	int	ref();
	int ref_desc(dword id, FILEOFS offset, UINT size, bool bigendian);
	int ref_flags(dword id, FILEOFS offset);
	virtual int ref_sel(ID id_low, dword id_high);
	virtual void reloadpalette();
	UINT render_tagstring(char *chars, vcp *colors, UINT maxlen, char *tagstring, bool cursor_in_line);
	void render_tagstring_desc(char **string, int *length, vcp *tag_color, char *tag, UINT size, bool bigendian, bool is_cursor);
	UINT render_tagstring_single(char *chars, vcp *colors, UINT maxlen, UINT offset, char *text, UINT len, vcp color);
	void scroll_up(int n);
	void scroll_down(int n);
	void select_mode_off();
	void select_mode_on();
	void select_mode_pre();
	void select_mode_post(bool lastpos);
	int set_cursor(ht_sub *sub, ID id1, ID id2);
	int set_cursor_ex(ht_sub *sub, ID id1, ID id2, int tag_idx, int tag_group);
	void update_micropos();
	void update_misc_info();
	void update_visual_info();
	void update_ypos();
public:
	UINT search_caps;
	
			void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	bool get_current_address(fmt_vaddress *address);
	virtual	bool get_current_offset(FILEOFS *offset);
	virtual	bool goto_offset(FILEOFS offset, ht_view *source_object=0);
	virtual	bool goto_address(fmt_vaddress addr, ht_view *source_object=0);
	virtual	void handlemsg(htmsg *msg);
	virtual	ht_search_result *psearch(ht_search_request *search, FILEOFS start, FILEOFS end);
	virtual	void pselect_add(FILEOFS start, FILEOFS end);
	virtual	void pselect_get(FILEOFS *start, FILEOFS *end);
	virtual	void pselect_set(FILEOFS start, FILEOFS end);
	virtual	UINT pwrite(FILEOFS ofs, void *buf, UINT size);
	virtual	bool string_to_address(char *string, fmt_vaddress *vaddr);
	virtual	bool string_to_offset(char *string, FILEOFS *ofs);
	virtual	ht_search_result *vsearch(ht_search_request *search, fmt_vaddress start, fmt_vaddress end);
/* new */
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

class ht_sub: public object {
protected:
	ht_streamfile *file;
public:
	ht_uformat_viewer *uformat_viewer;
	ht_sub *prev, *next;

			void init(ht_streamfile *file);
	virtual	void done();
/* new */
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	bool convert_id_to_ofs(ID id1, ID id2, FILEOFS *offset);
	virtual	bool closest_line_id(ID *id1, ID *id2);
	virtual	void first_line_id(ID *id1, ID *id2);
	virtual	bool getline(char *line, ID id1, ID id2);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(ID *id1, ID *id2);
	virtual	int next_line_id(ID *id1, ID *id2, int n);
	virtual	int prev_line_id(ID *id1, ID *id2, int n);
	virtual	bool ref(ID id1, ID id2);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
};

/*
 *	CLASS ht_linear_sub
 */

class ht_linear_sub: public ht_sub {
protected:
	FILEOFS fofs;
	dword fsize;
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
	dword vaddrinc;
	dword balign;
	UINT uid;
public:
			void init(ht_streamfile *file, FILEOFS ofs, dword size, UINT uid, dword vaddrinc=0);
	virtual 	void done();
/* overwritten */
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	bool convert_id_to_ofs(ID id1, ID id2, FILEOFS *ofs);
	virtual	void first_line_id(ID *id1, ID *id2);
	virtual	bool getline(char *line, ID id1, ID id2);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(ID *id1, ID *id2);
	virtual	int next_line_id(ID *id1, ID *id2, int n);
	virtual	int prev_line_id(ID *id1, ID *id2, int n);
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
	fmt_vaddress baseaddr;
	ht_string_list *masks;
	char temp[512];	/* FIXME: possible buffer overflow */
	UINT uid;
public:
			void init(ht_streamfile *file, UINT uid, fmt_vaddress baseaddr=0);
	virtual 	void done();
/* overwritten */
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual 	bool getline(char *line, ID id1, ID id2);
	virtual 	void first_line_id(ID *id1, ID *id2);
	virtual 	void last_line_id(ID *id1, ID *id2);
	virtual 	int next_line_id(ID *id1, ID *id2, int n);
	virtual 	int prev_line_id(ID *id1, ID *id2, int n);
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
/* new */
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	bool convert_id_to_ofs(ID id1, ID id2, FILEOFS *offset);
	virtual	bool closest_line_id(ID *id1, ID *id2);
	virtual	void first_line_id(ID *id1, ID *id2);
	virtual	bool getline(char *line, ID id1, ID id2);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(ID *id1, ID *id2);
	virtual	int next_line_id(ID *id1, ID *id2, int n);
	virtual	int prev_line_id(ID *id1, ID *id2, int n);
	virtual	bool ref(ID id1, ID id2);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
};

/*
 *	CLASS ht_collapsable_sub
 */

class ht_collapsable_sub: public ht_layer_sub {
protected:
	char *nodestring;
	bool collapsed;
	ID fid1, fid2;
	ID myfid1, myfid2;
	fmt_vaddress myfaddr;
public:
			void init(ht_streamfile *file, ht_sub *sub, bool own_sub, char *nodename, bool collapsed);
	virtual	void done();
/* new */
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	bool convert_id_to_ofs(ID id1, ID id2, FILEOFS *offset);
	virtual	void first_line_id(ID *id1, ID *id2);
	virtual	bool getline(char *line, ID id1, ID id2);
	virtual	void last_line_id(ID *id1, ID *id2);
	virtual	int next_line_id(ID *id1, ID *id2, int n);
	virtual	int prev_line_id(ID *id1, ID *id2, int n);
	virtual	bool ref(ID id1, ID id2);
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
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	bool convert_id_to_ofs(ID id1, ID id2, FILEOFS *offset);
	virtual	void first_line_id(ID *id1, ID *id2);
	virtual	bool getline(char *line, ID id1, ID id2);
	virtual	void handlemsg(htmsg *msg);
	virtual	void last_line_id(ID *id1, ID *id2);
	virtual	int next_line_id(ID *id1, ID *id2, int n);
	virtual	int prev_line_id(ID *id1, ID *id2, int n);
	virtual	bool ref(ID id1, ID id2);
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

ht_search_result *linear_expr_search(ht_search_request *search, FILEOFS start, FILEOFS end, ht_sub *sub, ht_uformat_viewer *ufv, FILEOFS fofs, dword fsize);
ht_search_result *linear_bin_search(ht_search_request *search, FILEOFS start, FILEOFS end, ht_streamfile *file, FILEOFS fofs, dword fsize);

#endif /* __HTFORMAT_H__ */
