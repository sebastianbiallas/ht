/* 
 *	HT Editor
 *	out_txt.cc
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

#ifndef OUT_TXT_H
#define OUT_TXT_H

#include "analy.h"
#include "common.h"
#include "global.h"
#include "stream.h"
#include "out.h"

class AnalyserTxtOutput: public AnalyserOutput {
		ht_stream *stream;
		char tmpbuf[1024];
		int last;
public:
				void	init(Analyser *analy, ht_stream *stream);
		virtual	void done();
		virtual	void	beginAddr();
		virtual	void	beginLine();
		virtual	ht_stream *getGenerateStream();
		virtual	int	elementLength(const char *s);
		virtual	void	endAddr();
		virtual	void	endLine();
		virtual	void footer();
		virtual	void	header();
		virtual	void putElement(int element_type, const char *element);
		virtual	char *link(char *s, Address *Addr);
		virtual	char *externalLink(char *s, int type1, int type2, int type3, int type4, void *special);
};

#endif
