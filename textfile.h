/* 
 *	HT Editor
 *	textfile.h
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

#ifndef __TEXTFILE_H__
#define __TEXTFILE_H__

#include "data.h"
#include "stream.h"
#include "syntax.h"

/*
 *	CLASS ht_textfile
 */
 
class ht_textfile: public ht_layer_streamfile {
public:
/* new */
	virtual	bool convert_ofs2line(FileOfs o, uint *line, uint *pofs)=0;
	virtual	bool convert_line2ofs(uint line, uint pofs, FileOfs *o)=0;
	virtual	void delete_lines(uint line, uint count)=0;
	virtual	void delete_chars(uint line, uint ofs, uint count)=0;
	virtual	bool get_char(uint line, char *ch, uint pos)=0;
	virtual	bool getline(uint line, uint pofs, void *buf, uint buflen, uint *retlen, lexer_state *state)=0;
	virtual	uint getlinelength(uint line)=0;
	virtual	void insert_lines(uint before, uint count, void **line_ends = NULL, int *lineendlens = NULL)=0;
	virtual	void insert_chars(uint line, uint ofs, void *chars, uint len)=0;
	virtual	bool has_line(uint line)=0;
	virtual	uint linecount()=0;
	virtual	void set_layered_assume(File *streamfile, bool changes_applied)=0;
	virtual	void set_lexer(ht_syntax_lexer *lexer)=0;
};

/*
 *	CLASS ht_layer_textfile
 */
 
class ht_layer_textfile: public ht_textfile {
public:
			void init(ht_textfile *textfile, bool own_textfile);
/* overwritten */
	virtual	bool convert_ofs2line(FileOfs o, uint *line, uint *pofs);
	virtual	bool convert_line2ofs(uint line, uint pofs, FileOfs *o);
	virtual	void delete_lines(uint line, uint count);
	virtual	void delete_chars(uint line, uint ofs, uint count);
	virtual	bool get_char(uint line, char *ch, uint pos);
	virtual	bool getline(uint line, uint pofs, void *buf, uint buflen, uint *retlen, lexer_state *state);
	virtual	uint getlinelength(uint line);
	virtual	void insert_lines(uint before, uint count, void **line_ends, int *lineendlens);
	virtual	void insert_chars(uint line, uint ofs, void *chars, uint len);
	virtual	bool has_line(uint line);
	virtual	uint linecount();
	virtual	void set_layered_assume(File *streamfile, bool changes_applied);
	virtual	void set_lexer(ht_syntax_lexer *lexer);
};

/*
 *	CLASS ht_ltextfile_line
 */

class ht_ltextfile_line: public Object {
public:
	virtual ~ht_ltextfile_line();

	lexer_state instate;
	struct {
		FileOfs ofs;
		uint len;
	} on_disk;
	bool is_in_memory;
	struct {
		char *data;
		uint len;
	} in_memory;
	FileOfs nofs;
	byte lineendlen;
	byte lineend[2];
};

/*
 *	CLASS ht_ltextfile
 */
 
class ht_ltextfile: public ht_textfile {
protected:
	FileOfs ofs;
	ht_clist *lines;
	ht_clist *orig_lines;
	ht_syntax_lexer *lexer;
	uint first_parse_dirty_line;
	uint first_nofs_dirty_line;
	bool dirty;

			void cache_invd();
			void cache_flush();
			void dirty_nofs(uint line);
			void dirty_parse(uint line);
			uint find_linelen_forwd(byte *buf, uint maxbuflen, FileOfs ofs, int *le_len);
	virtual	ht_ltextfile_line *fetch_line(uint line);
			ht_ltextfile_line *fetch_line_nofs_ok(uint line);
			ht_ltextfile_line *fetch_line_into_memory(uint line);
			uint getlinelength_i(ht_ltextfile_line *e);
			bool is_dirty_nofs(uint line);
			bool is_dirty_parse(uint line);
			byte *match_lineend_forwd(byte *buf, uint buflen, int *le_len);
			lexer_state next_instate(uint line);
			FileOfs next_nofs(ht_ltextfile_line *l);
			void split_line(uint a, uint pos, void *line_end, int line_end_len);
			void update_nofs(uint line);
			void update_parse(uint line);
			void reread();
public:
			void	init(File *streamfile, bool own_streamfile, ht_syntax_lexer *lexer);
	virtual	void done();
/* overwritten (streamfile) */
	virtual	void	copy_to(ht_stream *stream);
	virtual	int	extend(uint newsize);
	virtual	uint	get_size();
	virtual	void	pstat(pstat_t *s);
	virtual	uint	read(void *buf, uint size);
	virtual	void set_layered(File *streamfile);
	virtual	int	seek(FileOfs offset);
	virtual	FileOfs tell();
	virtual	int	truncate(uint newsize);
	virtual	int	vcntl(uint cmd, va_list vargs);
	virtual	uint	write(const void *buf, uint size);
/* overwritten (textfile) */
	virtual	bool convert_ofs2line(FileOfs o, uint *line, uint *pofs);
	virtual	bool convert_line2ofs(uint line, uint pofs, FileOfs *o);
	virtual	void delete_lines(uint line, uint count);
	virtual	void delete_chars(uint line, uint ofs, uint count);
	virtual	bool get_char(uint line, char *ch, uint pos);
	virtual	bool getline(uint line, uint pofs, void *buf, uint buflen, uint *retlen, lexer_state *state);
	virtual	uint getlinelength(uint line);
	virtual	void insert_lines(uint before, uint count, void **line_ends, int *lineendlens);
	virtual	void insert_chars(uint line, uint ofs, void *chars, uint len);
	virtual	bool has_line(uint line);
	virtual	uint linecount();
	virtual	void set_layered_assume(File *streamfile, bool changes_applied);
	virtual	void set_lexer(ht_syntax_lexer *lexer);
};

#endif /* __TEXTFILE_H__ */

