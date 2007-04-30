/*
 *	HT Editor
 *	out_html.h
 *
 *	Copyright (C) 1999, 2000, 2001 Sebastian Biallas (sb@biallas.net)
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

#ifndef OUT_HTML_H
#define OUT_HTML_H

#include "analy.h"
#include "io/types.h"
#include "stream.h"
#include "out.h"

class AnalyserHTMLOutput: public AnalyserOutput {
	Stream *stream;
	char tmpbuf[1024];
	int last;
public:
		void init(Analyser *analy, Stream *stream);
	virtual	void beginLine();
	virtual	void endLine();
	virtual	Stream *getGenerateStream();
	virtual	int  elementLength(const char *s);
	virtual	void footer();
	virtual	void header();
	virtual	void putElement(int element_type, const char *element);
	virtual	char *link(char *s, Address *Addr);
	virtual	char *externalLink(char *s, uint32 type1, uint32 type2, uint32 type3, uint32 type4, void *special);
};

#endif
