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

class OutLine: public ht_data {
public:
	int		textlen;
	byte		*text;
	int		bytes;                    // bytes of line in file
			OutLine(byte *Text, int Textlen, int Bytes);
			~OutLine();
};


class OutAddr: public ht_data {
public:
	Address	*addr;
	UINT		time;
	ht_clist  *lines;
	int		size;                     // size in memory
	int		bytes;                    // bytes of address in file
	
			OutAddr(Address *Addr, UINT Time);
			~OutAddr();
	void      appendLine(OutLine *l);
	void		clear();
	OutLine  *getLine(int i);
	void		updateTime(UINT Time);
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

#define OUTPUT_GENERATE_ERR_OK 0
#define OUTPUT_GENERATE_ERR_INVAL 1
#define OUTPUT_GENERATE_ERR_ANALYSER_NOT_FINISHED 2
#define OUTPUT_GENERATE_ERR_STREAM 3

/*
 *
 */
class AnalyserOutput: public Object {
public:
		Analyser	*analy;
		Address   *addr;
		int		line;
		Location  *cur_addr;
		OutAddr	*cur_out_addr;
		int		bytes_line;               // bytes of current line in file
		int		bytes_addr;               // bytes of current addr in file
		
		ht_dtree	*out_addrs;

		byte		*work_buffer_start;
		byte		*work_buffer;

		byte		*temp_buffer;
		
		UINT		current_time;
		int		size;
		
		int		dis_style;
		
				void	init(Analyser *analy);
		virtual	void done();
		virtual	void	beginAddr();
		virtual	void	beginLine();
		virtual	int	elementLength(char *s);
		virtual	void	endAddr();
		virtual	void	endLine();
		virtual	char *externalLink(char *s, int type1, int type2, int type3, int type4, void *special);
		virtual	void footer();
				void generateAddr(Address *Addr, OutAddr *oa);
				int	generateFile(Address *from, Address *to);
		virtual	ht_stream *getGenerateStream();
				void	generatePage(Address *from, int lines);
				OutAddr *getAddr(Address *Addr);
				OutLine *getLine(Address *Addr, int line);
				bool getLineString(char *buf, int maxlen, Address *Addr, int line);
				bool	getLineByteLength(int *len, Address *Addr, int line);
				int	getLineCount(Address *Addr);
				int	getAddrByteLength(Address *Addr);
		virtual	void	header();
				void	invalidateCache();
		virtual	char *link(char *s, Address *Addr);
				int	nextLine(Address **Addr, int *line, int n, Address *max);
				int	prevLine(Address **Addr, int *line, int n, Address *min);
		virtual	void putElement(int element_type, char *element);
				void	reset();
				void	write(char *s);
				void write(char *s, int n);
};

#endif
