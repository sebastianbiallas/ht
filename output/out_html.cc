/*
 *	HT Editor
 *	out_html.cc
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

#include "analy_names.h"
#include "htdebug.h"
#include "htinfo.h"
#include "out_html.h"
#include "tools.h"
#include "x86dis.h"
#include "string.h"
#include "snprintf.h"

static const char *header_str = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html>\n<head>\n";
static const char *footer_str = "</body>\n</html>\n";
static const char *stylesheet_str = "<style type=\"text/css\">\n"
"<!--\n"
"body {\n"
"\tfont: 10pt arial,helvetica,sans-serif;\n"
"\tcolor:#00ff00;\n"
"\tbackground:#000080;\n"
"}\n"
"A {\n"
"\tcolor:#ffffff;\n"
"\ttext-decoration: none;\n"
"}\n"
".c {\n"
"\tcolor:#c0c0c0;\n"
"}\n"
".l {\n"
"\tcolor:#ffff00;\n"
"}\n"
".sym {\n"
"\tcolor:#00ffff;\n"
"}\n"
".n {\n"
"\tcolor:#008080;\n"
"}\n"
".str {\n"
"\tcolor:#ff0000;\n"
"}\n"
"//-->\n"
"</style>\n";

void AnalyserHTMLOutput::init(Analyser *analy, Stream *s)
{
	AnalyserOutput::init(analy);
	stream = s;
	dis_style = DIS_STYLE_HIGHLIGHT+DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE+X86DIS_STYLE_OPTIMIZE_ADDR;
}

void AnalyserHTMLOutput::beginLine()
{
	AnalyserOutput::beginLine();
	char temp[20], temp3[20];
	if (line == 0) {
		addr->stringify(temp, sizeof temp, ADDRESS_STRING_FORMAT_LEADING_WHITESPACE);
		char temp2[20];
		last = addr->stringify(temp2, sizeof temp2, ADDRESS_STRING_FORMAT_COMPACT);
	} else {
		int s = addr->stringSize();
		memset(temp, ' ', s);
		memset(&temp[s-last], '.', last);
		temp[s] = 0;
	}

	write("<a name=\"");
	addr->stringify(temp3, sizeof temp, ADDRESS_STRING_FORMAT_LEADING_ZEROS | ADDRESS_STRING_FORMAT_ADD_H);
	write(temp3);
	write("\">");
	write(temp);
	write("</a>");

	if (analy->explored->contains(addr)) {
		write("<span class=\"n\"> ! </span>");
	} else {
		write("   ");
	}
}

void AnalyserHTMLOutput::endLine()
{
	write("\n");
	AnalyserOutput::endLine();
}

Stream *AnalyserHTMLOutput::getGenerateStream()
{
	return stream;
}

int AnalyserHTMLOutput::elementLength(const char *s)
{
	return strlen(s);
}

char *AnalyserHTMLOutput::link(char *s, Address *Addr)
{
	global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS | ADDRESS_STRING_FORMAT_ADD_H;
	ht_snprintf(tmpbuf, sizeof tmpbuf, "<a href=\"#%y\">%s</a>", Addr, s);
	return tmpbuf;
}

char *AnalyserHTMLOutput::externalLink(char *s, uint32 type1, uint32 type2, uint32 type3, uint32 type4, void *special)
{
	strcpy(tmpbuf, "test");
	return tmpbuf;
}

static void swrite(Stream *s, const char *str)
{
	s->writex(str, strlen(str));
}

void AnalyserHTMLOutput::footer()
{
	swrite(stream, footer_str);
}

void AnalyserHTMLOutput::putElement(int element_type, const char *element)
{
	switch (element_type) {
	case ELEMENT_TYPE_PRE_COMMENT:
	case ELEMENT_TYPE_COMMENT:
	case ELEMENT_TYPE_POST_COMMENT:
		write("<span class=\"c\">");
		write(element);
		write("</span>");
		break;
	case ELEMENT_TYPE_LABEL:
		write("<span class=\"l\">");
		write(element);
		write("</span>");
		break;
	case ELEMENT_TYPE_DATA_CODE:
		write(element);
		break;
	case ELEMENT_TYPE_HIGHLIGHT_DATA_CODE: {
		bool span = false;
		write("  ");
		while (*element) {
			if (*element == '\\') {
				element++;
				if (*element == '@') {
					element++;
					const char *cl = NULL;
					switch (*element) {
					case '#': // comment
						cl = "c";
						break;
					case 'c': // symbol
						cl = "sym";
						break;
					case 'd': // default
						break;
					case 'n': // number
						cl = "n";
						break;
					case 's': // string
						cl = "str";
						break;
					}
					if (span) write("</span>");
					span = false;
					if (cl) {
						write("<span class=\"");
						write(cl);
						write("\">");
						span = true;
					}
					element++;
				} else {
					*work_buffer++ = '\\';
					*work_buffer++ = *element++;
				}
				continue;
			}
			*work_buffer++ = *element++;
		}
		if (span) write("</span>");
		break;
	}
	case ELEMENT_TYPE_INDENT_XREF:
		write("  ");
		break;
	}
}

void AnalyserHTMLOutput::header()
{
	String name;
	analy->getName(name);
	swrite(stream, header_str);
	swrite(stream, "\t<title>Analysis of ");
	swrite(stream, name.contentChar());
	swrite(stream, "</title>\n");
	swrite(stream, stylesheet_str);
	swrite(stream, "</head>\n<body bgcolor=\"#ffffff\">\n\n");
	swrite(stream, "Analysis of <i>");
	swrite(stream, name.contentChar());
	swrite(stream, "</i><br>generated by <a href=\""ht_url"\">"ht_name" version "ht_version"</a>.\n<hr>\n<pre>\n");
}
