/* 
 *	HT Editor
 *	out.h
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

#ifndef OUT_H
#define OUT_H

#include "analy.h"
#include "common.h"
#include "global.h"
#include "htdata.h"

class out_line: public ht_data {
public:
	int		textlen;
	byte		*text;
	int		bytes;                    // bytes of line in file
			out_line(byte *Text, int Textlen, int Bytes);
			~out_line();
};


class out_addr: public ht_data {
public:
	ADDR		addr;
	UINT		time;
	ht_clist  *lines;
	int		size;                     // size in memory
	int		bytes;                    // bytes of address in file
	
			out_addr(ADDR Addr, UINT Time);
			~out_addr();
	void      append_line(out_line *l);
	void		clear();
	out_line  *get_line(int i);
	void		update_time(UINT Time);
};

/*
 *	max. length of a generated line is 1024
 */
#define WORKBUF_LEN 1024

#define ELEMENT_TYPE_PRE_COMMENT 0
#define ELEMENT_TYPE_COMMENT 1
#define ELEMENT_TYPE_POST_COMMENT 2
#define ELEMENT_TYPE_LABEL 3
#define ELEMENT_TYPE_DATA_CODE 4
#define ELEMENT_TYPE_HIGHLIGHT_DATA_CODE 5
#define ELEMENT_TYPE_INDENT_XREF 6

#define EXTERNAL_LINK_SHOW_XREFS 0
#define EXTERNAL_LINK_SHOW_COMMENTS 1

/*
 *
 */
class analyser_output: public object {
public:
		analyser	*analy;
		ADDR      addr;
		int		line;
		taddr     *cur_addr;
		out_addr	*cur_out_addr;
		int		bytes_line;               // bytes of current line in file
		int		bytes_addr;               // bytes of current addr in file
		
		ht_dtree	*out_addrs;

		byte		*work_buffer_start;
		byte		*work_buffer;

		byte		*temp_buffer;
		
		UINT		current_time;
		int		size;
		
				void	init(analyser *Analy);
		virtual	void done();
		virtual	void	begin_addr();
		virtual	void	begin_line();
		virtual	int	element_len(char *s);
          virtual	void	emit_edit_bytes(ADDR Addr, int count);
		virtual	void	end_addr();
		virtual	void	end_line();
		virtual	char *external_link(char *s, int type1, int type2, void *special);
		virtual	void footer();
				void generate_addr(ADDR Addr, out_addr *oa);
				int	generate_file(ADDR from, ADDR to);
				void	generate_page(ADDR from, int lines);
				out_addr *get_addr(ADDR Addr);
				out_line *get_line(ADDR Addr, int line);
				bool get_line_str(char *buf, ADDR Addr, int line);
				bool	get_line_len(int *len, ADDR Addr, int line);
				int	get_line_count(ADDR Addr);
				int	get_addr_len(ADDR Addr);
		virtual	void	header();
				void	invalidate_cache();
		virtual	char *link(char *s, ADDR Addr);
				int	next_line(ADDR *Addr, int *line, int n, ADDR max);
				int	prev_line(ADDR *Addr, int *line, int n, ADDR min);
		virtual	void put_element(int element_type, char *element);
				void	reset();
				void	write(char *s);
				void write(char *s, int n);
};

/*
 *
 */
#define ANALY_OUTPUT_OK 0
#define ANALY_OUTPUT_ERR_GENERIC 1
#define ANALY_OUTPUT_ERR_STREAM 2
#define ANALY_OUTPUT_ERR_ANALY_NOT_FINISHED 3

#endif
