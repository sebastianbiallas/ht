/* 
 *	HT Editor
 *	infoview.h
 *
 *	Copyright (C) 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

/*
 *	CLASS info_node
 */

class info_node: public ht_data {
public:
	FILEOFS start;
	UINT len;
	ht_tree *xrefs;
	
	info_node(FILEOFS start);
	~info_node();
};

/*
 *	CLASS ht_info_lexer
 */

class ht_info_lexer: public ht_syntax_lexer {
public:
	ht_tree *xrefs;
	UINT cx, cy;
	ht_view *pal_from;
	
			void init(ht_view *pal_from);
/* overwritten */
	virtual	vcp getcolor_syntax(UINT pal_index);
	virtual	lexer_state getinitstate();
	virtual	lexer_token geterrortoken();
	virtual	char *getname();
	virtual	lexer_token gettoken(void *buf, UINT buflen, text_pos p, bool start_of_line, lexer_state *ret_state, UINT *ret_len);
	virtual	vcp gettoken_color(lexer_token t);
/* new */
			void set_xrefs(ht_tree *xrefs);
			void set_cursor(UINT cx, UINT cy);
};

/*
 *	CLASS ht_info_textfile
 */
 
class ht_info_textfile: public ht_ltextfile {
protected:
	UINT start, end;

	virtual ht_ltextfile_line *fetch_line(UINT line);
public:
			void	init(ht_streamfile *streamfile, bool own_streamfile, ht_syntax_lexer *lexer);
	virtual	void done();
/* overwritten */
	virtual	UINT linecount();
/* new */	
			void set_node(UINT ofs, UINT len);
};

/*
 *	CLASS ht_info_viewer
 */

class ht_info_viewer: public ht_text_viewer {
protected:
	char *cwd;
	char *file;
	char *node;
	ht_tree *xrefs;
	ht_list *history;

			int find_node(char *infofile, char *node);
			ht_tree *get_xrefs();
			bool igotonode(char *file, char *node, bool add2hist);
			UINT readfile(char *fn, char **text);
public:
			void init(bounds *b);
	virtual	void done();
/* overwritten */
	virtual	void draw();
	virtual	char *defaultpalette();
	virtual void get_pindicator_str(char *buf);
	virtual	void handlemsg(htmsg *msg);
/* new */	
	virtual	bool gotonode(char *file, char *node);
};

#endif /* __TEXTEDIT_H__ */
