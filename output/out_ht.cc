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

void	analyser_ht_output::init(analyser *Analy)
{
	analyser_output::init(Analy);
	analy_pal.data=NULL;
	analy_pal.size=0;
	reloadpalette();
}

void analyser_ht_output::done()
{
	if (analy_pal.data) free(analy_pal.data);
	analyser_output::done();
}

vcp analyser_ht_output::getcolor_analy(UINT pal_index)
{
	return getcolorv(&analy_pal, pal_index);
}

void analyser_ht_output::reloadpalette()
{
	if (analy_pal.data) {
		free(analy_pal.data);
		analy_pal.data = NULL;
	}	    
	load_pal(palclasskey_analyser, palkey_analyser_default, &analy_pal);
}

void analyser_ht_output::begin_addr()
{
	analyser_output::begin_addr();
}

void analyser_ht_output::begin_line()
{
	analyser_output::begin_line();
	if (line==0) {
		char temp[9];
		sprintf(temp, HEX8FORMAT8, addr);
		work_buffer = (byte *)tag_make_sel((TAGSTRING *)work_buffer, temp);
	} else {
		work_buffer = (byte *)tag_make_sel((TAGSTRING *)work_buffer, "   ...  ");
	}
     
	if (analy->explored->contains(addr)) {
		write(" ! ");
	} else {
		write("   ");
	}

	work_buffer_edit_bytes_insert = work_buffer;
}

void	analyser_ht_output::emit_edit_bytes(ADDR Addr, int count)
{

}

int	analyser_ht_output::element_len(char *s)
{
	return tag_strlen(s);
}

void	analyser_ht_output::end_addr()
{
	analyser_output::end_addr();
}

void	analyser_ht_output::end_line()
{
	if ((analy->mode & ANALY_EDIT_BYTES) && bytes_line) {
		if (analy->valid_addr(addr, scinitialized)) {

			FILEADDR a=analy->file_addr(addr);
			assert(a != INVALID_FILE_OFS);

			char workbuf2[1024];
			char *work_buffer2 = workbuf2;
               
			for (int i=0; i < bytes_line; i++) {
				work_buffer2 = tag_make_edit_byte(work_buffer2, a+i);
			}
			for (int i=0; i <= analy->maxopcodelength*2-bytes_line*2; i++) {
				*(work_buffer2++)=' ';
			}

			/* ugly */
			*work_buffer = 0;
			memmove(work_buffer_edit_bytes_insert+(work_buffer2-workbuf2), work_buffer_edit_bytes_insert, tag_strlen((char*)work_buffer_edit_bytes_insert));
			memcpy(work_buffer_edit_bytes_insert, workbuf2, (work_buffer2-workbuf2));
			work_buffer += (work_buffer2-workbuf2);
		}
	}
	analyser_output::end_line();
}

char *analyser_ht_output::external_link(char *s, int type1, int type2, void *special)
{
	*(tag_make_ref(tmpbuffer, type1, type2, s)) = 0;
	return tmpbuffer;
}


char *analyser_ht_output::link(char *s, ADDR Addr)
{
	*(tag_make_ref(tmpbuffer, Addr, 0, s)) = 0;
	return tmpbuffer;
}

void analyser_ht_output::put_element(int element_type, char *element)
{
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


