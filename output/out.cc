/* 
 *	HT Editor
 *	out.cc
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

#include <string.h>

#include "analy.h"
#include "analy_names.h"
#include "global.h"
#include "htdebug.h"
#include "htstring.h"
#include "tools.h"
#include "out.h"

// FIXME: grrrrrrrrrrr
#include "x86dis.h"

/*
 *	make address a little bit more random
 */
ADDR ADDR_delinearize(ADDR a)
{
	return a*0x8088405+1;	/* there's magic in here... */
	// 1664525
}


/*
 *
 */
out_line::out_line(byte *Text, int Textlen, int Bytes)
{
	textlen = Textlen;
	text = (byte*)malloc(Textlen);
	memcpy(text, Text, textlen);
	bytes = Bytes;
}

out_line::~out_line()
{
	free(text);
}


/*
 *
 */
out_addr::out_addr(ADDR Addr, UINT Time)
{
	addr = Addr;
	update_time(Time);
	lines = new ht_clist();
	lines->init();
	size = 0;
	bytes = 0;
	
}

out_addr::~out_addr()
{
	lines->destroy();
	delete lines;
}

void out_addr::append_line(out_line *l)
{
	lines->insert(l);
	bytes += l->bytes;
	size += l->textlen;
}

void out_addr::clear()
{
	lines->empty();
	bytes = 0;
	size = 0;
}

out_line *out_addr::get_line(int i)
{
	return (out_line*)lines->get(i);
}

void out_addr::update_time(UINT Time)
{
	time = Time;
}

/*
 *
 */
void	analyser_output::init(analyser *Analy)
{
	analy = Analy;
	addr = INVALID_ADDR;
	cur_addr = NULL;
	cur_out_addr = NULL;
	out_addrs = new ht_dtree();
	out_addrs->init(compare_keys_uint);
	bytes_addr = 0;
	bytes_line = 0;
	size = 0;
	current_time = 0;
	work_buffer_start = (byte*)malloc(WORKBUF_LEN);
	work_buffer = work_buffer_start;
	temp_buffer = (byte*)malloc(WORKBUF_LEN);
}

void analyser_output::done()
{
	out_addrs->destroy();
	delete out_addrs;
	free(work_buffer_start);
	free(temp_buffer);
}

void	analyser_output::begin_addr()
{
	bytes_addr = 0;
	line = 0;
}

void	analyser_output::begin_line()
{
	work_buffer = work_buffer_start;
	bytes_line = 0;
}

void	analyser_output::emit_edit_bytes(ADDR Addr, int count)
{
}

int	analyser_output::element_len(char *s)
{
	return strlen(s);
}

void	analyser_output::end_addr()
{
}

void	analyser_output::end_line()
{
	cur_out_addr->append_line(new out_line(work_buffer_start, (work_buffer-work_buffer_start), bytes_line));
	bytes_addr += bytes_line;
	line++;
}

void analyser_output::footer()
{
	// STUB
}

char *analyser_output_addr_sym_func(CPU_ADDR Addr, int *symstrlen, void *context)
{
	analyser_output *analy_output = (analyser_output *) context;
	char *buf;
	taddr *a = analy_output->analy->find_addr(Addr.addr32.offset);
	
	if (a && a->label) {
		buf = analy_output->link(a->label->name, Addr.addr32.offset);
		if (symstrlen) *symstrlen = analy_output->element_len(buf);
		return buf;
	} else {
		if (analy_output->analy->valid_addr(Addr.addr32.offset, scvalid)) {
			char m[9];
			sprintf(m, "%xh", Addr.addr32.offset);
			buf = analy_output->link(m, Addr.addr32.offset);
			if (symstrlen) *symstrlen = analy_output->element_len(buf);
			return buf;
		}
	}
	return NULL;
}

void analyser_output::generate_addr(ADDR Addr, out_addr *oa)
{
#define LABELINDENT 32
#define MAX_XREF_COLS 3
#define MAX_XREF_LINES 7
	addr = Addr;
/*     fprintf(stdout, "generate(%x)\n", Addr);
	fflush(stdout);*/
	if (addr==0x703b1415) {


	}
	cur_addr = analy->find_addr(addr);
	cur_out_addr = oa;
	cur_out_addr->clear();
	
	begin_addr();

	if (cur_addr) {
		int xref_count = cur_addr->xrefs ? cur_addr->xrefs->count() : 0;
		// comments
		comment_list *c = cur_addr->comments;
		if (c) {
			int c1 = c->count();
			for (int i=0; i < c1; i++) {
				begin_line();
				put_element(ELEMENT_TYPE_PRE_COMMENT, c->get_name(i));
				end_line();
			}
		}
		// label && xrefs
		bool collapsed_xrefs = false;
		if (analy->mode & ANALY_COLLAPSE_XREFS) {
			// count xrefs
			if (xref_count>=(MAX_XREF_COLS*MAX_XREF_LINES)) {
				collapsed_xrefs = true;
			}
		}               
		if ((cur_addr->label) || (cur_addr->xrefs)) {
			begin_line();
			// label
			int labellength = (cur_addr->label) ? strlen(cur_addr->label->name) : 0;
			if (labellength) {
				put_element(ELEMENT_TYPE_LABEL, cur_addr->label->name);
				write(":");
				labellength++;
			}
			// xrefs
			if (labellength >= LABELINDENT && cur_addr->xrefs) {
				end_line();
				begin_line();
				labellength = 0;
			}
			if (collapsed_xrefs) {
				for (int j=labellength; j<LABELINDENT; j++) write(" ");
				char b[32];
				sprintf(b, "<< show xrefs (%d) >>", xref_count);
				char *t = external_link(b, Addr, 1, NULL);
				write(t);
			} else {
				ht_tree *x_tree = cur_addr->xrefs;
                    if (x_tree) {
					int i=0;
			          addr_xref *x;
			          ht_addr *xa = (ht_addr*)x_tree->enum_next((ht_data**)&x, NULL);
			          while (xa) {
						if ((i % (MAX_XREF_COLS+1))!=0) {
							char buf3[9];
							sprintf(buf3, " %c", xref_type_short(x->type));
							write(buf3);
							sprintf(buf3, HEX8FORMAT, xa->addr);
							write(link(buf3, xa->addr));
					          xa = (ht_addr*)x_tree->enum_next((ht_data**)&x, xa);
						} else {
							if (i!=0) {
								end_line();
								begin_line();
							}
							for (int j=labellength; j<LABELINDENT; j++) write(" ");
							labellength = 0;
							put_element(ELEMENT_TYPE_COMMENT, ";xref");
						}
						i++;
					}
                    }
			}
			end_line();
		}
	}
	
	begin_line();

	bool is_valid_code_addr = analy->valid_code_addr(addr);
	
	if ((cur_addr && ((cur_addr->type.type == dt_code) || ((cur_addr->type.type == dt_unknown) && (is_valid_code_addr))))
	|| (!cur_addr && is_valid_code_addr)) {
		// code
		taddr *next_addr = analy->enum_addrs(addr);
		int op_len;
		
		// max. length of current opcode
		if (next_addr) {
			op_len = MIN((dword)analy->maxopcodelength, (next_addr->addr - addr));
		} else {
			op_len = analy->maxopcodelength;
		}

		byte buf[16];
		int buffer_size = analy->bufptr(addr, buf, sizeof(buf));
		if (analy->disasm && buffer_size) {
			OPCODE *o=analy->disasm->decode(buf, MIN(buffer_size, op_len), analy->map_addr(Addr));
			/* inits for addr-sym transformations */
			addr_sym_func_context = this;
			addr_sym_func = &analyser_output_addr_sym_func;
			char *x=analy->disasm->str(o, DIS_STYLE_HIGHLIGHT+DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE+X86DIS_STYLE_OPTIMIZE_ADDR);
			put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, x);
			/* deinits for addr-sym transformations */
			addr_sym_func_context = NULL;
			addr_sym_func = NULL;
			bytes_line += analy->disasm->getsize(o);
		} else {
			if (buffer_size >= 1) {
				char buff[20];
				sprintf(buff, "db      \\@n%02xh", buf[0]);
				put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buff);
			} else {
				put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
			}
			bytes_line++;
		}
		
	} else {
		// data
		if (analy->valid_addr(addr, scvalid)) {
			if (cur_addr && (cur_addr->type.type != dt_unknown) && (cur_addr->type.type != dt_unknown_data)) {
				switch (cur_addr->type.type) {
					case dt_int: {
						bytes_line += cur_addr->type.length;
						assert(cur_addr->type.length);
						if (analy->valid_addr(addr, scinitialized)) {
							char buf[20];
							switch (cur_addr->type.int_subtype) {
								case dst_iword: {
									word c;
									analy->bufptr(addr, (byte *)&c, 2);
									sprintf(buf, "dw      \\@n%04xh", c);
									put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									break;
								}
								case dst_idword: {
									dword c;
									analy->bufptr(addr, (byte *)&c, 4);
									sprintf(buf, "dd      \\@n%08xh", c);
									put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									break;
								}
								case dst_ibyte:
								default: {
									byte c;
									if (analy->bufptr(addr, (byte *)&c, 1)==1) {
										sprintf(buf, "db      \\@n%02xh ", c);
										put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
										sprintf(buf, "; '%c'", (c<32)?32:c);
										put_element(ELEMENT_TYPE_COMMENT, buf);
									} else {
										put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
									}
								}
							}
						} else {
							// not initialized
							switch (cur_addr->type.int_subtype) {
								case dst_iword:
									put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dw      ????");
									break;
								case dst_idword:
									put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dd      ????????");
									break;
								case dst_ibyte:
								default:
									put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
							}
						}
						break;
					}
					case dt_array: {
						if (analy->valid_addr(addr, scinitialized)) {
							switch (cur_addr->type.array_subtype) {
								case dst_string: {
									char buf[1024];
									byte bufread[1024];
									char *b;
									int r = analy->bufptr(addr, bufread, MIN(cur_addr->type.length, 1024));
									strcpy(buf, "db      \\@s\"");
									b = buf + 12 + escape_special(buf+12, 100, bufread, r, "\"", false);
									*b = '\"'; b++; *b = 0;
									put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									bytes_line += cur_addr->type.length;
									break;                                             
								}
								default: {assert(0);}
							}
						} else {
							put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ?");
						}
						break;
					}                         
					default: {
						assert(0);
					}
				}
			} else {
				// not a known address
				bytes_line++;
				byte c;
				if (analy->valid_addr(addr, scinitialized) && (analy->bufptr(addr, &c, 1)==1)) {
					char buf[20];
					sprintf(buf, "db      \\@n%02xh ", c);
					put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
					sprintf(buf, "; '%c'", (c<32)?32:c);
					put_element(ELEMENT_TYPE_COMMENT, buf);
				} else {
					put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
				}
			}
		} else {
			// invalid addr
/*			ADDR next = next_valid(Addr);
			if (next != INVALID_ADDR) {
				bytes_line += next - Addr;
				sprintf(tempbuf, "  db      ?? * %d", next - Addr);
			} else {*/
				bytes_line += 1;
				put_element(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
/*			}*/
		}
	}
	
	end_line();
	end_addr();
}

int	analyser_output::generate_file(ADDR from, ADDR to)
{
	return 0;
}

void	analyser_output::generate_page(ADDR from, int lines)
{

}

out_addr *analyser_output::get_addr(ADDR Addr)
{
	if (Addr != addr) {
		ht_data_uint *a = new ht_data_uint(ADDR_delinearize(Addr));
		out_addr *oa = (out_addr*)out_addrs->get(a);
		if (!oa) {
			if (out_addrs->count() > 1024) reset();
			oa = new out_addr(Addr, current_time);
			generate_addr(Addr, oa);
			out_addrs->insert(a, oa);
		} else {
			delete a;
			if (oa->time != current_time) {
				generate_addr(Addr, oa);
				oa->update_time(current_time);
			}
		}
		addr = Addr;
		cur_out_addr = oa;
	} else if (cur_out_addr) {
		if (cur_out_addr->time != current_time) {
			generate_addr(Addr, cur_out_addr);
			cur_out_addr->update_time(current_time);
		}
	}
	return cur_out_addr;
}

out_line *analyser_output::get_line(ADDR Addr, int line)
{
	out_addr *a = get_addr(Addr);
	return a ? a->get_line(line) : NULL;
}

bool analyser_output::get_line_str(char *buf, ADDR Addr, int line)
{
	out_line *ol = get_line(Addr, line);
	if (ol) {
		memcpy(buf, ol->text, ol->textlen);
		buf[ol->textlen] = 0;
		return true;
	} else {
		return false;
	}
}

bool analyser_output::get_line_len(int *len, ADDR Addr, int line)
{
	out_line *ol = get_line(Addr, line);
	if (ol) {
		*len = ol->bytes;
		return true;
	} else {
		return false;
	}
}

int analyser_output::get_line_count(ADDR Addr)
{
    return get_addr(Addr)->lines->count();
}

int analyser_output::get_addr_len(ADDR Addr)
{
	return get_addr(Addr)->bytes;
}

void	analyser_output::header()
{
	// STUB
}

char *analyser_output::link(char *s, ADDR Addr)
{
	strcpy((char*)temp_buffer, s);
	return (char*)temp_buffer;
}

char *analyser_output::external_link(char *s, int type1, int type2, void *special)
{
	strcpy((char*)temp_buffer, s);
	return (char*)temp_buffer;
}

void	analyser_output::invalidate_cache()
{
	current_time++;
	if ((current_time % 50)==0) {
		// collect garbage
		reset();
	}
}

int	analyser_output::next_line(ADDR *Addr, int *line, int n, ADDR max)
{
	int res = 0;
	int len;
	while (n--) {
		if (get_line_len(&len, *Addr, *line+1)) {
			(*line)++;
		} else {
			get_line_len(&len, *Addr, *line);
			if (*Addr>=max) return res;
			*Addr += len;
			*line = 0;
		}
		res++;
	}
	return res;
}

int	analyser_output::prev_line(ADDR *Addr, int *line, int n, ADDR min)
{
//	fprintf(stdout, "prev_line(%x, %d, %d)\n", *Addr, *line, n);
	int res = 0;
	// trivial cases
	if (*Addr < min || (*Addr == min && *line==0)) return 0;
	while (n && *line) {
		n--;
		(*line)--;
		res++;
	}
	if (!n) return res;

	// complicated cases
	ADDR	l=n*24;
	if (l<120) l = 120;
	ADDR search_addr = *Addr - l;
	int search_line = 0;
	if (l > *Addr) search_addr = min;

	taddr *prev = analy->enum_addrs_back(*Addr);
	if (prev) {
		ADDR prevnext = prev->addr + get_addr_len(prev->addr);
		if (prevnext > *Addr) {
			// somebody jumped in the middle of an continuous address
			*Addr = prev->addr;
			*line = 0;
			res++;
			return prev_line(Addr, line, n-1, min)+res;
		}
		if (prevnext == *Addr) {
			*Addr = prev->addr;
			*line = get_line_count(prev->addr)-1;
			res++;
			return prev_line(Addr, line, n-1, min)+res;
		}
		
		if (prevnext + l >= *Addr) {
			search_addr = prevnext;
		}
	}

	if (search_addr < min) {
		search_addr = min;
		search_line = 0;
	}

	ADDR addrbuf[1024];
	int	linebuf[1024];
	int  i = 0;
	int  len;
	
	ADDR	next_addr = search_addr;
	while (1) {
		if (search_addr >= *Addr) {
			if (search_line >= *line || search_addr > *Addr) break;
		}
		if (get_line_len(&len, search_addr, search_line)) {
			next_addr += len;
			search_line++;
		} else {
			search_addr = next_addr;
			search_line = 0;
			continue;
		}
		addrbuf[i & 1023] = search_addr;
		linebuf[i & 1023] = search_line-1;
		i++;
	}

	if (!i) return res;

	if (i >= n) {
		*Addr = addrbuf[(i-n) & 1023];
		*line = linebuf[(i-n) & 1023];
		res += n;
		n = 0;
	} else {
		*Addr = addrbuf[0];
		*line = linebuf[0];
		res += i;
		n -= i;
	}


	if (n) return prev_line(Addr, line, n, min)+res;
	return res;
}

void analyser_output::put_element(int element_type, char *element)
{
	write(element);
}

void	analyser_output::reset()
{
	addr = INVALID_ADDR;
	cur_out_addr = NULL;
	out_addrs->empty();
}

void	analyser_output::write(char *s)
{
	int len = element_len(s);
	memcpy(work_buffer, s, len);
	work_buffer += len;
}

void analyser_output::write(char *s, int n)
{
	memcpy(work_buffer, s, n);
	work_buffer += n;
}


