/*
 *	HT Editor
 *	htstruct.h
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

#ifndef __HTSTRUCT_H__
#define __HTSTRUCT_H__

#include "formats.h"
#include "htdialog.h"
#include "htformat.h"

#if 0

#define DESC_STRUCTURE "structure"

extern format_viewer_if htstructure_if;

/*
 *	CLASS ht_format_loc_loc
 */
 
class ht_format_loc_loc: public ht_format_loc {
public:
	ht_sorted_list *sublocs;
	
	ht_format_loc_loc();
	~ht_format_loc_loc();
};

/*
 *	CLASS ht_structure_status
 */
 
struct ht_structure_status_data {
	char *name;
	FILEOFS cursor_ofs;
};

class ht_structure_status: public ht_statictext {
protected:
	char *name;
	FILEOFS cursor_ofs;
	char status[256];
public:
			void	init(bounds *b, char *text, int align, bool breaklines=1);
/* overwritten */
	virtual	char *gettext();
	virtual	void setdata(void *data);
};
 
/*
 *	CLASS ht_structure_viewer
 */
 
class ht_structure_viewer: public ht_format_viewer {
	ht_format_loc_loc *loclocs;
	ht_streamfile *file;
	bool init_completed;
	UINT granularity;

	ht_format_loc_loc *cursor_loc;
	bool cursor_loc_unassigned;	/* else multiple */
	FILEOFS cursor_ofs;
	UINT scroll;

	ht_structure_status *status;

	ht_clist *path;

/* new */
			void build_loc_string(char *buf, ht_list *clocs);
			UINT calc_y_extent();
			void complete_init();
			void collect_views(ht_format_viewer *v);
			UINT collect_locs(ht_format_viewer *v, ht_format_loc_loc *f, ht_format_loc *loc);
			UINT count_starts(ht_list *s, FILEOFS offset);
			UINT count_ends(ht_list *s, FILEOFS offset);
			void cursor_up(UINT n);
			void cursor_down(UINT n);
			void cursor_left(UINT n);
			void cursor_right(UINT n);
			ht_list *find_locs(ht_list *s, FILEOFS offset);
			void limit_cursor();
			void ref();
			void scroll_up(UINT n);
			void scroll_down(UINT n);
			void set_granularity(UINT granularity);
			void update_cursor_loc();
			void update_status();
public:
			void init(bounds *b, char *desc, UINT caps, ht_streamfile *file, ht_structure_status *status);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	void handlemsg(htmsg *msg);
	virtual	bool goto_offset(FILEOFS ofs, ht_view *source_object=0);
};

#endif /* 0 */

#endif /* __HTSTRUCT_H__ */

