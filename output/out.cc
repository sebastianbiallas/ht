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

#include "analy.h"
#include "analy_names.h"
#include "global.h"
#include "htdebug.h"
#include "htstring.h"
#include "snprintf.h"
#include "tools.h"
#include "out.h"

// FIXME: grrrrrrrrrrr
#include "x86dis.h"

#include <string.h>

#undef DPRINTF
//#define DPRINTF(msg...) printf(##msg)
//#define DPRINTF2(msg...) printf(##msg)
#define DPRINTF(msg...)
#define DPRINTF2(msg...)

int compare_keys_addresses_delinear(ht_data *key_a, ht_data *key_b)
{
	return ((Address*)key_a)->compareDelinear((Address*)key_b);
}

/*
 *
 */
OutLine::OutLine(byte *Text, int Textlen, int Bytes)
{
	textlen = Textlen;
	text = (byte*)malloc(Textlen);
	memcpy(text, Text, textlen);
	bytes = Bytes;
}

OutLine::~OutLine()
{
	free(text);
}


/*
 *
 */
OutAddr::OutAddr(Address *Addr, UINT Time)
{
	addr = (Address *)Addr->duplicate();
	updateTime(Time);
	lines = new ht_clist();
	lines->init();
	size = 0;
	bytes = 0;
	
}

OutAddr::~OutAddr()
{
	delete addr;
	lines->destroy();
	delete lines;
}

void OutAddr::appendLine(OutLine *l)
{
	lines->insert(l);
	bytes += l->bytes;
	size += l->textlen;
}

void OutAddr::clear()
{
	lines->empty();
	bytes = 0;
	size = 0;
}

OutLine *OutAddr::getLine(int i)
{
	return (OutLine*)lines->get(i);
}

void OutAddr::updateTime(UINT Time)
{
	time = Time;
}

/*
 *
 */
void	AnalyserOutput::init(Analyser *Analy)
{
	analy = Analy;
	addr = new InvalidAddress();
	cur_addr = NULL;
	cur_out_addr = NULL;
	out_addrs = new ht_dtree();
	out_addrs->init(compare_keys_addresses_delinear);
	bytes_addr = 0;
	bytes_line = 0;
	size = 0;
	current_time = 0;
	work_buffer_start = (byte*)malloc(WORKBUF_LEN);
	work_buffer = work_buffer_start;
	temp_buffer = (byte*)malloc(WORKBUF_LEN);
	dis_style = DIS_STYLE_HIGHLIGHT+DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE+X86DIS_STYLE_OPTIMIZE_ADDR;
	changeConfig();
}

void AnalyserOutput::done()
{
	out_addrs->destroy();
	delete out_addrs;
	delete addr;
	free(work_buffer_start);
	free(temp_buffer);
}

void	AnalyserOutput::beginAddr()
{
	bytes_addr = 0;
	line = 0;
}

void	AnalyserOutput::beginLine()
{
	work_buffer = work_buffer_start;
	bytes_line = 0;
}

void	AnalyserOutput::changeConfig()
{
}

int	AnalyserOutput::elementLength(char *s)
{
	return strlen(s);
}

void	AnalyserOutput::endAddr()
{
}

void	AnalyserOutput::endLine()
{
	cur_out_addr->appendLine(new OutLine(work_buffer_start, (work_buffer-work_buffer_start), bytes_line));
	bytes_addr += bytes_line;
	line++;
}

void AnalyserOutput::footer()
{
	// STUB
}

char *analyser_output_addr_sym_func(CPU_ADDR Addr, int *symstrlen, void *context)
{
	AnalyserOutput *output = (AnalyserOutput *) context;
	char *buf;
	Address *addr = output->analy->createAddress();
/*     if (Addr.addr32.offset == 0xaea) {
		int as=0;
	}*/
	addr->getFromCPUAddress(&Addr);
	Location *loc = output->analy->getLocationByAddress(addr);
	
	if (loc && loc->label) {
		buf = output->link(loc->label->name, addr);
		if (symstrlen) *symstrlen = output->elementLength(buf);
		delete addr;
		return buf;
	} else {
		if (output->analy->validAddress(addr, scvalid)) {
			char m[90];
			addr->stringify(m, sizeof m, ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_H);
			buf = output->link(m, addr);
			if (symstrlen) *symstrlen = output->elementLength(buf);
			delete addr;
			return buf;
		}
	}
	delete addr;
	return NULL;
}

void AnalyserOutput::generateAddr(Address *Addr, OutAddr *oa)
{
#define LABELINDENT 32
#define MAX_XREF_COLS 3
#define MAX_XREF_LINES 7
/*               char tbuf[1024];
			Addr->stringify(tbuf, 1024, 0);
	printf("generate_addr(%s, ", tbuf);
			addr->stringify(tbuf, 1024, 0);
			printf("%s)\n", tbuf);*/
	cur_addr = analy->getLocationByAddress(addr);
	cur_out_addr = oa;
	cur_out_addr->clear();
	
	beginAddr();

	if (cur_addr) {
		int xref_count = cur_addr->xrefs ? cur_addr->xrefs->count() : 0;
		// comments
		CommentList *c = cur_addr->comments;
		if (c && (analy->mode & ANALY_SHOW_COMMENTS)) {
			int c1 = c->count();
			for (int i=0; i < c1; i++) {
				beginLine();
				putElement(ELEMENT_TYPE_PRE_COMMENT, c->getName(i));
				endLine();
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
		if ((cur_addr->label && (analy->mode & ANALY_SHOW_LABELS)) || (cur_addr->xrefs && (analy->mode & ANALY_SHOW_XREFS))) {
			beginLine();
			// label
			int labellength = (cur_addr->label) ? strlen(cur_addr->label->name) : 0;
			if (labellength && (analy->mode & ANALY_SHOW_LABELS)) {
				putElement(ELEMENT_TYPE_LABEL, cur_addr->label->name);
				write(":");
				labellength++;
			}
			// xrefs
			if (labellength >= LABELINDENT && cur_addr->xrefs) {
				endLine();
				beginLine();
				labellength = 0;
			}
			if (collapsed_xrefs) {
				for (int j=labellength; j<LABELINDENT; j++) write(" ");
				char b[32];
				sprintf(b, "<< show xrefs (%d) >>", xref_count);
				char *t;
				if (Addr->byteSize()==4) {
					dword d;
					Addr->putIntoArray((byte*)&d);
					t = externalLink(b, d, 0, 0, 1, NULL);
				} else {
					qword d;
					Addr->putIntoArray((byte*)&d);
					t = externalLink(b, d.hi, d.lo, 0, 1, NULL);
				}
				write(t);
			} else {
				ht_tree *x_tree = cur_addr->xrefs;
				if (x_tree) {
					int i=0;
					AddrXRef *x;
					Address *xa = (Address*)x_tree->enum_next((ht_data**)&x, NULL);
					while (xa) {
						if ((i % (MAX_XREF_COLS+1))!=0) {
							char buf3[90];
							sprintf(buf3, " %c", xref_type_short(x->type));
							write(buf3);
							xa->stringify(buf3, 1024, ADDRESS_STRING_FORMAT_COMPACT);
							write(link(buf3, xa));
							xa = (Address*)x_tree->enum_next((ht_data**)&x, xa);
						} else {
							if (i!=0) {
								endLine();
								beginLine();
							}
							for (int j=labellength; j<LABELINDENT; j++) write(" ");
							labellength = 0;
							putElement(ELEMENT_TYPE_COMMENT, ";xref");
						}
						i++;
					}
				}
			}
			endLine();
		}
	}
	
	beginLine();

	bool is_valid_code_addr = analy->validCodeAddress(addr);

/*     char bbuf[1232];
	addr->stringify(bbuf, 1024, 0);
	if (strcmp(bbuf, "30:00b2")==0) {
		int asf=0;
	}*/
	
	
	if ((cur_addr && ((cur_addr->type.type == dt_code) || ((cur_addr->type.type == dt_unknown) && (is_valid_code_addr))))
	|| (!cur_addr && is_valid_code_addr)) {
		// code
		Location *next_addr = analy->enumLocations(addr);
		int op_len;
		
		// max. length of current opcode
		if (next_addr) {
			int d=255;
			next_addr->addr->difference(d, addr);
			op_len = MIN((dword)analy->max_opcode_length, (UINT)d);
		} else {
			op_len = analy->max_opcode_length;
		}

		byte buf[16];
		int buffer_size = analy->bufPtr(addr, buf, sizeof(buf));
		if (analy->disasm && buffer_size) {
			OPCODE *o=analy->disasm->decode(buf, MIN(buffer_size, op_len), analy->mapAddr(Addr));
			/* inits for addr-sym transformations */
			addr_sym_func_context = this;
			if (analy->mode & ANALY_TRANSLATE_SYMBOLS) addr_sym_func = &analyser_output_addr_sym_func;

			bool s;
			do {
				char *x = analy->disasm->str(o, dis_style);
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, x);
				if ((s = analy->disasm->selectNext(o))) {
					endLine();
					beginLine();
				}
			} while (s);
			
			bytes_line += analy->disasm->getSize(o);
			/* deinits for addr-sym transformations */
			addr_sym_func_context = NULL;
			addr_sym_func = NULL;
		} else {
			if (buffer_size >= 1) {
				char buff[20];
				sprintf(buff, "db      \\@n%02xh", buf[0]);
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buff);
				sprintf(buff, " ; '%c'", (buf[0]<32)?' ':buf[0]);
				putElement(ELEMENT_TYPE_COMMENT, buff);

			} else {
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
			}
			bytes_line++;
		}
		
	} else {
		// data
		if (analy->validAddress(addr, scvalid)) {
			if (cur_addr && (cur_addr->type.type != dt_unknown) && (cur_addr->type.type != dt_unknown_data)) {
				switch (cur_addr->type.type) {
					case dt_int: {
						bytes_line += cur_addr->type.length;
						assert(cur_addr->type.length);
						if (analy->validAddress(addr, scinitialized)) {
							char buf[50];
							switch (cur_addr->type.int_subtype) {
								case dst_iword: {
									word c;
									analy->bufPtr(addr, (byte *)&c, 2);
									sprintf(buf, "dw      \\@n%04xh", c);
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									break;
								}
								case dst_idword: {
									dword c;
									analy->bufPtr(addr, (byte *)&c, 4);
									sprintf(buf, "dd      \\@n%08xh", c);
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									break;
								}
								case dst_iqword: {
									qword c;
									analy->bufPtr(addr, (byte *)&c, 8);
									ht_snprintf(buf, sizeof buf, "dq      \\@n%016qxh", &c);
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									break;
								}
								case dst_ibyte:
								default: {
									byte c;
									if (analy->bufPtr(addr, (byte *)&c, 1)==1) {
										sprintf(buf, "db      \\@n%02xh ", c);
										putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
										sprintf(buf, "; '%c'", (c<32)?32:c);
										putElement(ELEMENT_TYPE_COMMENT, buf);
									} else {
										putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
									}
								}
							}
						} else {
							// not initialized
							switch (cur_addr->type.int_subtype) {
								case dst_iword:
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dw      ????");
									break;
								case dst_idword:
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dd      ????????");
									break;
								case dst_ibyte:
								default:
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
							}
						}
						break;
					}
					case dt_array: {
						if (analy->validAddress(addr, scinitialized)) {
							switch (cur_addr->type.array_subtype) {
								case dst_string: {
									char buf[1024];
									byte bufread[1024];
									char *b;
									int r = analy->bufPtr(addr, bufread, MIN(cur_addr->type.length, 1024));
									strcpy(buf, "db      \\@s\"");
									b = buf + 12 + escape_special(buf+12, 100, bufread, r, "\"", false);
									*b = '\"'; b++; *b = 0;
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									bytes_line += cur_addr->type.length;
									break;                                             
								}
								default: {assert(0);}
							}
						} else {
							putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ?");
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
				if (analy->validAddress(addr, scinitialized) && (analy->bufPtr(addr, &c, 1)==1)) {
					char buf[20];
					sprintf(buf, "db      \\@n%02xh ", c);
					putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
					sprintf(buf, "; '%c'", (c<32)?32:c);
					putElement(ELEMENT_TYPE_COMMENT, buf);
				} else {
					putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
				}
			}
		} else {
			// invalid addr
			Address *next = analy->nextValid(Addr);
			int d;
			if (next && next->isValid() && next->difference(d, Addr)) {
				bytes_line += d;
				char buf[100];
				sprintf(buf, "db      ?? * %d", d);
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
			} else {
				bytes_line += 1;
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db      ??");
			}
		}
	}
	
	endLine();
	endAddr();
}

ht_stream *AnalyserOutput::getGenerateStream()
{
	return NULL;
}

int	AnalyserOutput::generateFile(Address *from, Address *to)
{
	if (analy->active) return OUTPUT_GENERATE_ERR_ANALYSER_NOT_FINISHED;
	if (!from->isValid() || !to->isValid()) return OUTPUT_GENERATE_ERR_INVAL;
	ht_stream *out = getGenerateStream();
	if (!out) return OUTPUT_GENERATE_ERR_INVAL;
	header();
	int line = 0;
	while (from->compareTo(to) <= 0) {
		char buffer[1024];
		if (getLineString(buffer, sizeof buffer, from, line)) {
			// write buffer
			// FIXME: remove strlen
			UINT wr = strlen(buffer);
			if (out->write(buffer, wr) != wr) return OUTPUT_GENERATE_ERR_STREAM;
			
			// update address
			int len;
			if (getLineByteLength(&len, from, line)) {
				from->add(len);
			}
			// update line
			line++;
		} else {
			line = 0;
		}
	}
	footer();
	return OUTPUT_GENERATE_ERR_OK;
}

void	AnalyserOutput::generatePage(Address *from, int lines)
{

}

OutAddr *AnalyserOutput::getAddr(Address *Addr)
{
			char tbuf[1024];
			Addr->stringify(tbuf, 1024, 0);
			DPRINTF("%s -- ",tbuf);
	if (!addr->isValid() || Addr->compareTo(addr) != 0) {
		assert(addr != Addr);
		DPRINTF("not cached1 --");
		delete addr;
		addr = (Address *)Addr->duplicate();
		OutAddr *oa = (OutAddr*)out_addrs->get(Addr);
		if (!oa) {
			DPRINTF("generate\n");
			if (out_addrs->count() > 1024) {
				reset();
				addr = (Address *)Addr->duplicate();
			}
			oa = new OutAddr(Addr, current_time);
			generateAddr(Addr, oa);
			out_addrs->insert((Address *)Addr->duplicate(), oa);
		} else {
			DPRINTF("but cached2 ");
			if (oa->time != current_time) {
				DPRINTF("(and generate)");
				generateAddr(Addr, oa);
				oa->updateTime(current_time);
			}
			DPRINTF("\n");
		}
		cur_out_addr = oa;
	} else {
		DPRINTF("Cached! ");
		if (cur_out_addr->time != current_time) {
			DPRINTF("(but generate)");
			generateAddr(Addr, cur_out_addr);
			cur_out_addr->updateTime(current_time);
		}
		DPRINTF("\n");
	}
	return cur_out_addr;
}

OutLine *AnalyserOutput::getLine(Address *Addr, int line)
{
	return getAddr(Addr)->getLine(line);
}

bool AnalyserOutput::getLineString(char *buf, int maxlen, Address *Addr, int line)
{
	OutLine *ol = getLine(Addr, line);
	if (ol) {
		if (maxlen>0) {
			int len = MIN(ol->textlen+1, maxlen);
			len--;
			memcpy(buf, ol->text, len);
			buf[len] = 0;
		}
		return true;
	} else {
		return false;
	}
}

bool AnalyserOutput::getLineByteLength(int *len, Address *Addr, int line)
{
	OutLine *ol = getLine(Addr, line);
	if (ol) {
		*len = ol->bytes;
		return true;
	} else {
		return false;
	}
}

int AnalyserOutput::getLineCount(Address *Addr)
{
    return getAddr(Addr)->lines->count();
}

int AnalyserOutput::getAddrByteLength(Address *Addr)
{
	return getAddr(Addr)->bytes;
}

void	AnalyserOutput::header()
{
	// STUB
}

char *AnalyserOutput::link(char *s, Address *Addr)
{
	strcpy((char*)temp_buffer, s);
	return (char*)temp_buffer;
}

char *AnalyserOutput::externalLink(char *s, int type1, int type2, int type3, int type4, void *special)
{
	strcpy((char*)temp_buffer, s);
	return (char*)temp_buffer;
}

void	AnalyserOutput::invalidateCache()
{
	DPRINTF("invalidateCache()\n");
	current_time++;
	if ((current_time % 50)==0) {
		// collect garbage
		reset();
	}
}

int	AnalyserOutput::nextLine(Address **Addr, int *line, int n, Address *max)
{
	int res = 0;
	int len;
	while (n--) {
		if (getLineByteLength(&len, *Addr, *line+1)) {
			(*line)++;
		} else {
			getLineByteLength(&len, *Addr, *line);
			if ((*Addr)->compareTo(max)>=0) return res;
			if (!(*Addr)->add(len)) return res;
			*line = 0;
		}
		res++;
	}
	return res;
}

int	AnalyserOutput::prevLine(Address **Addr, int *line, int n, Address *min)
{
//	fprintf(stdout, "prev_line(%x, %d, %d)\n", *Addr, *line, n);
// #define ADDRBUF(a) (a)->stringify(tbuf, 1024, 0);
#define ADDRBUF(a)
	char tbuf[1024];
	ADDRBUF(*Addr)
	DPRINTF2("prev_line(%s, %d, %d", tbuf, *line, n);
//	printf("prev_line(%s, %d, %d", tbuf, *line, n);
	ADDRBUF(min)
	DPRINTF2(", %s)\n", tbuf);
//	printf(", %s)\n", tbuf);
	int res = 0;
	// trivial cases
	int cmp = (*Addr)->compareTo(min);
	DPRINTF2("cmp=%d\n", cmp);
	if (cmp<0 || (cmp == 0 && *line==0)) {
		DPRINTF2("geht nicht\n");
		return 0;
	}
	while (n && *line) {
		DPRINTF2("simple\n");
		n--;
		(*line)--;
		res++;
	}
	DPRINTF2("test\n");
	if (!n) return res;
	DPRINTF2("test2\n");

	// complicated cases
	int min_length, max_length, min_look_ahead, avg_look_ahead, addr_align;
	if (analy->disasm) {
		analy->disasm->getOpcodeMetrics(min_length, max_length, min_look_ahead, avg_look_ahead, addr_align);
	} else {
		min_look_ahead=1;
		avg_look_ahead=1;
	}
	int l = n*avg_look_ahead;
	if (l<min_look_ahead) l = min_look_ahead;
	Address *search_addr = (Address*)(*Addr)->duplicate();
	if (!search_addr->add(-l) || search_addr->compareTo(min)<0) {
		DPRINTF2("nicht gut\n");
		delete search_addr;
		search_addr = (Address *)min->duplicate();
	}
	int search_line = 0;

	Location *prev = analy->enumLocationsReverse(*Addr);
	if (prev) {
		Address *prevnext = (Address *)prev->addr->duplicate();
		if (prevnext->add(getAddrByteLength(prev->addr))) {
				DPRINTF2("mid-test\n");
			if (prevnext->compareTo(*Addr) > 0) {
				// somebody jumped in the middle of an continuous address
				delete *Addr;
				delete search_addr;
				delete prevnext;
				*Addr = (Address *)prev->addr->duplicate();
				*line = 0;
				res++;
				DPRINTF2("mid\n");
				return prevLine(Addr, line, n-1, min)+res;
			}
			if (prevnext->compareTo(*Addr) == 0) {
				delete *Addr;
				delete search_addr;
				delete prevnext;
				*Addr = (Address *)prev->addr->duplicate();
				*line = getLineCount(prev->addr)-1;
				res++;
				DPRINTF2("mid2\n");
				return prevLine(Addr, line, n-1, min)+res;
			}
			Address *oldprevnext = (Address *)prevnext->duplicate();
			if (prevnext->add(l) && prevnext->compareTo(*Addr) >= 0) {
				delete search_addr;
				search_addr = oldprevnext;
				DPRINTF2("mid3\n");
			} else {
				delete oldprevnext;
			}
		}
		delete prevnext;
	}

	if (search_addr->compareTo(min) < 0) {
		DPRINTF2("search_addr < min\n");
		delete search_addr;
		search_addr = (Address *)min->duplicate();
		search_line = 0;
	}

	Address *addrbuf[1024];
	int	linebuf[1024];
	int  i = 0;
	int  len;
	
	Address *next_addr = (Address *)search_addr->duplicate();
	while (1) {
		ADDRBUF(search_addr);
		DPRINTF2("search_addr: (%s, %d) ", tbuf, search_line);
		ADDRBUF(next_addr);
		DPRINTF2("next_addr: %s \n", tbuf);
		if (search_addr->compareTo(*Addr) >= 0) {
			if (search_line >= *line || (search_addr->compareTo(*Addr) > 0)) break;
		}
		if (getLineByteLength(&len, search_addr, search_line)) {
			next_addr->add(len);
			search_line++;
		} else {
			delete search_addr;
			search_addr = (Address *)next_addr->duplicate();
			search_line = 0;
			continue;
		}
		assert(i<1024);
		addrbuf[i & 1023] = (Address *)search_addr->duplicate();
		linebuf[i & 1023] = search_line-1;
		i++;
	}

	delete next_addr;
	delete search_addr;
	
	if (!i) {
		DPRINTF2("no i!\n");
		return res;
	}
	delete *Addr;
	if (i >= n) {
		*Addr = (Address *)addrbuf[(i-n) & 1023]->duplicate();
		*line = linebuf[(i-n) & 1023];
		res += n;
		n = 0;
	} else {
		*Addr = (Address *)addrbuf[0]->duplicate();
		*line = linebuf[0];
		res += i;
		n -= i;
	}

	for (int j=0; j<i; j++) delete addrbuf[j];

	if (n) return prevLine(Addr, line, n, min)+res;
	return res;
}

void AnalyserOutput::putElement(int element_type, char *element)
{
	write(element);
}

void	AnalyserOutput::reset()
{
	delete addr;
	addr = new InvalidAddress;
	cur_out_addr = NULL;
	out_addrs->empty();
}

void	AnalyserOutput::write(char *s)
{
	int len = elementLength(s);
	len = MIN(len, WORKBUF_LEN-(work_buffer-work_buffer_start));
	memcpy(work_buffer, s, len);
	work_buffer += len;
}

void AnalyserOutput::write(char *s, int n)
{
	n = MIN(n, WORKBUF_LEN-(work_buffer-work_buffer_start));
	memcpy(work_buffer, s, n);
	work_buffer += n;
}


