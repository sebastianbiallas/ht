/* 
 *	HT Editor
 *	out_ht.cc
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
#include "htatom.h"
#include "htctrl.h"
#include "htdebug.h"
#include "httag.h"
#include "htpal.h"
#include "out_ht.h"

#include <string.h>
#include <stdlib.h>

#define palclasskey_analyser						"analyser"
#define palkey_analyser_default					"default"

void	AnalyserHTOutput::init(Analyser *Analy)
{
	analy_pal.data=NULL;
	analy_pal.size=0;
	AnalyserOutput::init(Analy);
}

void AnalyserHTOutput::done()
{
	if (analy_pal.data) free(analy_pal.data);
	AnalyserOutput::done();
}

vcp AnalyserHTOutput::getcolor_analy(UINT pal_index)
{
	return getcolorv(&analy_pal, pal_index);
}

void AnalyserHTOutput::reloadPalette()
{
	if (analy_pal.data) {
	    free(analy_pal.data);
	    analy_pal.data = NULL;
	}	    
	load_pal(palclasskey_analyser, palkey_analyser_default, &analy_pal);
}

void	AnalyserHTOutput::beginAddr()
{
	AnalyserOutput::beginAddr();
}

void	AnalyserHTOutput::beginLine()
{
	AnalyserOutput::beginLine();
	if (analy->mode & ANALY_SHOW_ADDRESS) {
		char temp[20];
		if (line==0) {
#if 0
			char temp2[20];
			addr->stringify(temp2, sizeof temp2, ADDRESS_STRING_FORMAT_COMPACT);
			int s = strlen(temp2);
			int s2 = addr->stringSize();
			memset(temp, ' ', s2);
			memcpy(&temp[(s2-s)/2], temp2, s);
			temp[s2]=0;
#endif
			// FIXME:sda lksdfj
			addr->stringify(temp, sizeof temp, ADDRESS_STRING_FORMAT_LEADING_WHITESPACE);
			char temp2[20];
			last = addr->stringify(temp2, sizeof temp2, ADDRESS_STRING_FORMAT_COMPACT);
		} else {
			int s = addr->stringSize();
			memset(temp, ' ', s);
			memset(&temp[s-last], '.', last);
//			memcpy(&temp[(s-3)/2], (void*)"...", 3);
			temp[s] = 0;
		}
		// FIXME: buffer bla
		work_buffer = (byte *)tag_make_sel((TAGSTRING *)work_buffer, temp);
	}
	
	if (analy->explored->contains(addr)) {
		write(" ! ");
	} else {
		write("   ");
	}

	work_buffer_edit_bytes_insert = work_buffer;
}

void	AnalyserHTOutput::changeConfig()
{
	reloadPalette();
}

int	AnalyserHTOutput::elementLength(const char *s)
{
	return tag_strlen(s);
}

void	AnalyserHTOutput::endAddr()
{
	AnalyserOutput::endAddr();
}

void	AnalyserHTOutput::endLine()
{
	if ((analy->mode & ANALY_EDIT_BYTES) && bytes_line) {
		if (analy->validAddress(addr, scinitialized)) {

			// FIXME: bufferbla ? and ugly
			FILEOFS a=analy->addressToFileofs(addr);
			assert(a != INVALID_FILE_OFS);

			char workbuf2[1024];
			char *work_buffer2 = workbuf2;
			
			for (int i=0; i < bytes_line; i++) {
				work_buffer2 = tag_make_edit_byte(work_buffer2, a+i);
			}
			for (int i=0; i <= analy->max_opcode_length*2-bytes_line*2; i++) {
				*(work_buffer2++)=' ';
			}

			/* ugly */
			*work_buffer = 0;
			memmove(work_buffer_edit_bytes_insert+(work_buffer2-workbuf2), work_buffer_edit_bytes_insert, tag_strlen((char*)work_buffer_edit_bytes_insert));
			memcpy(work_buffer_edit_bytes_insert, workbuf2, (work_buffer2-workbuf2));
			work_buffer += (work_buffer2-workbuf2);
		}
	}
	AnalyserOutput::endLine();
}

char *AnalyserHTOutput::externalLink(char *s, int type1, int type2, int type3, int type4, void *special)
{
	*(tag_make_ref(tmpbuffer, type1, type2, type3, type4, s)) = 0;
	return tmpbuffer;
}


char *AnalyserHTOutput::link(char *s, Address *Addr)
{
	// FIXNEW
	if (Addr->byteSize()==4) {
		dword d;
		Addr->putIntoArray((byte*)&d);
		*(tag_make_ref(tmpbuffer, d, 0, 0, 0, s)) = 0;
	} else {
		qword d;
		Addr->putIntoArray((byte*)&d);
		*(tag_make_ref(tmpbuffer, d.hi, d.lo, 0, 0, s)) = 0;
	}
	return tmpbuffer;
}

void AnalyserHTOutput::putElement(int element_type, const char *element)
{
	// bufferbla's
	switch (element_type) {
		case ELEMENT_TYPE_PRE_COMMENT:
			work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_comment));
			write(element);
			break;
		case ELEMENT_TYPE_COMMENT:
			work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_comment));
			write(element);
			break;
		case ELEMENT_TYPE_POST_COMMENT:
			work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_comment));
			write(element);
			break;
		case ELEMENT_TYPE_LABEL:
			work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_label));
			write(element);
			break;
		case ELEMENT_TYPE_DATA_CODE:
			write(element);
			break;
		case ELEMENT_TYPE_HIGHLIGHT_DATA_CODE:
			work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_default));
			write("  ");
			while (*element) {
				if (*element == '\e') {
					int len = tag_get_len(element);
					while (len--) *(work_buffer++) = *(element++);
					continue;
				}
				if (*element == '\\') {
					element++;
					if (*element == '@') {
						element++;
						switch (*element) {
							case '#': // comment
								work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_comment));
								break;
							case 'c': // symbol
								work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_symbol));
								break;
							case 'd': // default
								work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_default));
								break;
							case 'n': // number
								work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_number));
								break;
							case 's': // string
								work_buffer = (byte *)tag_make_color((TAGSTRING *)work_buffer, getcolor_analy(palidx_analyser_string));
								break;
						}
						element++;
					} else {
						*work_buffer++ = '\\';
						*work_buffer++ = *(element++);
					}
					continue;
				}
				*(work_buffer++) = *(element++);
			}
			break;
		case ELEMENT_TYPE_INDENT_XREF:
			write("  ");
			break;
	}
}


