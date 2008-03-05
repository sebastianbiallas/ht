/* 
 *	HT Editor
 *	out_ht.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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

#ifndef OUT_HT_H
#define OUT_HT_H

#include "analy.h"
#include "io/types.h"
#include "htobj.h"
#include "out.h"

/*
 *	analyser palette
 */

#define palidx_analyser_default					0
#define palidx_analyser_label						1
#define palidx_analyser_comment					2
#define palidx_analyser_string					3
#define palidx_analyser_number					4
#define palidx_analyser_symbol					5

/*
 *	CLASS analyser_ht_output
 */

class AnalyserHTOutput: public AnalyserOutput {
	palette	analy_pal;
	char tmpbuffer[1024];
	byte *work_buffer_edit_bytes_insert;
	int last;

	vcp getcolor_analy(uint pal_index);
public:

			void init(Analyser *analy);
		virtual	void done();
		virtual	void beginAddr();
		virtual	void beginLine();
		virtual	void changeConfig();
		virtual	int  elementLength(const char *s);
		virtual	void endAddr();
		virtual	void endLine();
		virtual	void putElement(int element_type, const char *element);
		virtual	char *link(char *s, Address *Addr);
		virtual	char *externalLink(char *s, uint32 type1, uint32 type2, uint32 type3, uint32 type4, void *special);
private:
			void reloadPalette();
};

#endif
