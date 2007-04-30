/* 
 *	HT Editor
 *	infoview.h
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

#ifndef __INFOVIEW_H__
#define __INFOVIEW_H__

#include "textedit.h"
#include "textfile.h"

#define MAGIC_HT_HELP "$$HTHELP"

/*
 *	CLASS info_node
 */

class info_node: public Object {
public:
	FileOfs start;
	uint len;
	Container *xrefs;
	
	info_node(FileOfs start);
	~info_node();
};

/*
 *	CLASS ht_info_lexer
 */

class ht_info_lexer: public ht_syntax_lexer {
public:
	Container *xrefs;
	uint cx, cy;
	ht_view *pal_from;
	
		void init(ht_view *pal_from);
/* overwritten */
	virtual	vcp getcolor_syntax(uint pal_index);
	virtual	lexer_state getinitstate();
	virtual	lexer_token geterrortoken();
	virtual	const char *getname();
	virtual	lexer_token gettoken(void *buf, uint buflen, text_pos p, bool start_of_line, lexer_state *ret_state, uint *ret_len);
	virtual	vcp gettoken_color(lexer_token t);
/* new */
		void set_xrefs(Container *xrefs);
		void set_cursor(uint cx, uint cy);
};

/*
 *	CLASS ht_info_textfile
 */
 
class ht_info_textfile: public ht_ltextfile {
protected:
	uint start, end;

	virtual ht_ltextfile_line *fetch_line(uint line) const;
public:
		ht_info_textfile(File *streamfile, bool own_streamfile, ht_syntax_lexer *lexer);
/* overwritten */
	virtual	uint linecount() const;
/* new */	
			void set_node(uint ofs, uint len);
};

/*
 *	CLASS ht_info_viewer
 */

class ht_info_viewer: public ht_text_viewer {
protected:
	char *cwd;
	char *file;
	char *node;
	Container *xrefs;
	Container *history;

			int find_node(const char *infofile, const char *node);
			bool igotonode(const char *file, const char *node, bool add2hist);
			uint readfile(char *fn, char **text);
public:
			void init(Bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	const char *defaultpalette();
	virtual int get_pindicator_str(char *buf, int max_len);
	virtual	void handlemsg(htmsg *msg);
/* new */	
	virtual	bool gotonode(const char *file, const char *node);
};

#endif /* __INFOVIEW_H__ */
