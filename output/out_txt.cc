/* 
 *	HT Editor
 *	out_txt.cc
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

#include "analy.h"
#include "htdebug.h"
#include "htinfo.h"
#include "snprintf.h"
#include "out_txt.h"

#include <string.h>
#include <stdlib.h>

// FIXMEMEMEMEME
#include "x86dis.h"

void	AnalyserTxtOutput::init(Analyser *analy, Stream *s)
{
	AnalyserOutput::init(analy);
	stream = s;
	dis_style = DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE+X86DIS_STYLE_OPTIMIZE_ADDR;
}

void	AnalyserTxtOutput::beginAddr()
{
	AnalyserOutput::beginAddr();
}

void	AnalyserTxtOutput::beginLine()
{
	AnalyserOutput::beginLine();
	char temp[20];
	if (line==0) {
		addr->stringify(temp, sizeof temp, ADDRESS_STRING_FORMAT_LEADING_WHITESPACE);
		char temp2[20];
		last = addr->stringify(temp2, sizeof temp2, ADDRESS_STRING_FORMAT_COMPACT);
	} else {
		int s = addr->stringSize();
		memset(temp, ' ', s);
		memset(&temp[s-last], '.', last);
		temp[s] = 0;
	}
	write(temp);
	
	if (analy->explored->contains(addr)) {
		write(" ! ");
	} else {
		write("   ");
	}
}

Stream *AnalyserTxtOutput::getGenerateStream()
{
	return stream;
}

int	AnalyserTxtOutput::elementLength(const char *s)
{
	return strlen(s);
}

void	AnalyserTxtOutput::endAddr()
{
	AnalyserOutput::endAddr();
}

void	AnalyserTxtOutput::endLine()
{
	write("\n");
	AnalyserOutput::endLine();
}

void AnalyserTxtOutput::putElement(int element_type, const char *element)
{
	switch (element_type) {
	case ELEMENT_TYPE_HIGHLIGHT_DATA_CODE: {
		write("  ");
		while (*element) {
			if (*element == '\\') {
				element++;
				if (*element == '@') {
					element++;
					if (!*element) break;
					element++;
				} else if (*element == 0) {
					break;
				} else {
					*work_buffer++ = '\\';
					*work_buffer++ = *element++;
				}
				continue;
			}
			*work_buffer++ = *element++;
		}
		break;
	}
	case ELEMENT_TYPE_INDENT_XREF:
		write("  ");
		break;
	default:
		write(element);
		break;
	}
}

char *AnalyserTxtOutput::link(char *s, Address *Addr)
{
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS | ADDRESS_STRING_FORMAT_ADD_H;
	ht_snprintf(tmpbuf, sizeof tmpbuf, "%s<%y>", s, Addr);
	return tmpbuf;
}

char *AnalyserTxtOutput::externalLink(char *s, uint32 type1, uint32 type2, uint32 type3, uint32 type4, void *special)
{
	strcpy(tmpbuf, "test");
	return tmpbuf;
}

void AnalyserTxtOutput::footer()
{
}

void	AnalyserTxtOutput::header()
{
	String name;
	ht_snprintf(tmpbuf, sizeof tmpbuf, "Analysis of %y\ngenerated by " ht_name " version " ht_version " (" ht_url ")\n\n", &analy->getName(name));
	stream->write(tmpbuf, strlen(tmpbuf));
}

