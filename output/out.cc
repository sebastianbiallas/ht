/* 
 *	HT Editor
 *	out.cc
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
#include "analy_names.h"
#include "htdebug.h"
#include "strtools.h"
#include "snprintf.h"
#include "tools.h"
#include "out.h"

// FIXME: grrrrrrrrrrr
#include "x86dis.h"

#include <string.h>

#undef DPRINTF
#define DPRINTF(msg...)
#define DPRINTF2(msg...)
//#define DPRINTF(msg...) printf(##msg)
//#define DPRINTF2(msg...) printf(##msg)

int compare_keys_addresses_delinear(Object *key_a, Object *key_b)
{
	return ((Address*)key_a)->compareDelinear((Address*)key_b);
}

/*
 *
 */
OutLine::OutLine(byte *Text, int Textlen, int Bytes)
{
	textlen = Textlen;
	text = ht_malloc(Textlen);
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
OutAddr::OutAddr(Address *aAddr, uint aTime)
{
	addr = aAddr->clone();
	updateTime(aTime);
	lines = new Array(true);
	size = 0;
	bytes = 0;	
}

OutAddr::~OutAddr()
{
	delete addr;
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
	lines->delAll();
	bytes = 0;
	size = 0;
}

OutLine *OutAddr::getLine(int i)
{
	return (OutLine*) (*lines)[i];
}

int OutAddr::compareTo(const Object *o) const
{
//	uint oo = o->getObjectID();
	return addr->compareTo(((OutAddr*)o)->addr);
}

void OutAddr::updateTime(uint Time)
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
	out_addrs = new AVLTree(true);
	bytes_addr = 0;
	bytes_line = 0;
	size = 0;
	current_time = 0;
	work_buffer_start = ht_malloc(WORKBUF_LEN);
	work_buffer = work_buffer_start;
	work_buffer_end = work_buffer_start + WORKBUF_LEN - 1;
	*work_buffer_end = 0;
	temp_buffer = ht_malloc(WORKBUF_LEN);
	dis_style = DIS_STYLE_HIGHLIGHT+DIS_STYLE_HEX_NOZEROPAD+DIS_STYLE_HEX_ASMSTYLE+X86DIS_STYLE_OPTIMIZE_ADDR;
	changeConfig();
}

void AnalyserOutput::done()
{
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

int	AnalyserOutput::elementLength(const char *s)
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

static char *analyser_output_addr_sym_func(CPU_ADDR Addr, int *symstrlen, void *context)
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

#if 0	
	char tbuf[1024];
	Addr->stringify(tbuf, sizeof tbuf, 0);
	printf("generate_addr(%s, ", tbuf);
	char tbuf2[1024];
	addr->stringify(tbuf2, sizeof tbuf2, 0);
	printf("%s)\n", tbuf2);
#endif

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
			if (xref_count >= MAX_XREF_COLS * MAX_XREF_LINES) {
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
				uint64 u;
				Addr->putIntoUInt64(u);
				t = externalLink(b, u >> 32, u, 0, 1, NULL);
				write(t);
			} else {
				Container *xr = cur_addr->xrefs;
				if (xr) {
					int i=0;
					ObjHandle xh = xr->findFirst();
					while (xh != invObjHandle) {
						if ((i % (MAX_XREF_COLS+1))!=0) {
							AddrXRef *x = (AddrXRef *)xr->get(xh);
							char buf3[90];
							sprintf(buf3, " %c", xref_type_short(x->type));
							write(buf3);
							x->addr->stringify(buf3, sizeof buf3, ADDRESS_STRING_FORMAT_COMPACT);
							write(link(buf3, x->addr));
							xh = xr->findNext(xh);
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

	bool is_valid_ini_addr = analy->validAddress(addr, scinitialized);
	bool is_valid_code_addr = analy->validCodeAddress(addr);
	
	if (
		is_valid_ini_addr 
		&& (
			(
				cur_addr 
				&& (
					cur_addr->type.type == dt_code
					|| (
						cur_addr->type.type == dt_unknown 
						&& is_valid_code_addr
					)
				)
			)			
			|| (
				!cur_addr 
				&& is_valid_code_addr
			)
		)
	) {	
		// code
		Location *next_addr = analy->enumLocations(addr);
		int op_len;
		
		// max. length of current opcode
		if (next_addr) {
			int d = 255;
			next_addr->addr->difference(d, addr);
			op_len = MIN(uint32(analy->max_opcode_length), uint(d));
		} else {
			op_len = analy->max_opcode_length;
		}

		byte buf[16];
		int buffer_size = analy->bufPtr(addr, buf, sizeof buf);
		if (analy->disasm && buffer_size) {
			OPCODE *o = analy->disasm->decode(buf, MIN(buffer_size, op_len), analy->mapAddr(Addr));
			/* inits for addr-sym transformations */
			addr_sym_func_context = this;
			if (analy->mode & ANALY_TRANSLATE_SYMBOLS) addr_sym_func = &analyser_output_addr_sym_func;

			int complete_bytes_line = analy->disasm->getSize(o);
			bool s = true;
			do {
				if (s) {
					const char *x = analy->disasm->str(o, dis_style);
					putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, x);
				}
				if ((s = analy->disasm->selectNext(o))/* || complete_bytes_line*/) {
					endLine();
					beginLine();
				} else {
					if (analy->mode & ANALY_EDIT_BYTES) {
						want_bytes_line = MIN(complete_bytes_line, 16);
					} else {
						want_bytes_line = complete_bytes_line;
					}					
					bytes_line += want_bytes_line;
					complete_bytes_line -= want_bytes_line;
				}
			} while (s || complete_bytes_line);
			
			/* deinits for addr-sym transformations */
			addr_sym_func_context = NULL;
			addr_sym_func = NULL;
		} else {
			if (buffer_size >= 1) {
				char buff[20];
				sprintf(buff, "db          \\@n%02xh", buf[0]);
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buff);
				sprintf(buff, " ; '%c'", (buf[0]<32)?' ':buf[0]);
				putElement(ELEMENT_TYPE_COMMENT, buff);

			} else {
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db          ??");
			}
			bytes_line = want_bytes_line = 1;
		}
		
	} else {
		// data
		if (analy->validAddress(addr, scvalid)) {
			if (cur_addr && (cur_addr->type.type != dt_unknown)
			&& (cur_addr->type.type != dt_unknown_data)
			&& (cur_addr->type.type != dt_code)) {
				switch (cur_addr->type.type) {
					case dt_int: {
						bytes_line = want_bytes_line = cur_addr->type.length;
						assert(cur_addr->type.length);
						if (analy->validAddress(addr, scinitialized)) {
							char buf[50];
							switch (cur_addr->type.int_subtype) {
							case dst_iword: {
								uint16 c;
								analy->bufPtr(addr, (byte *)&c, 2);
								sprintf(buf, "dw          \\@n%04xh", c);
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
								break;
							}
							case dst_idword: {
								uint32 c;
								analy->bufPtr(addr, (byte *)&c, 4);
								sprintf(buf, "dd          \\@n%08xh", c);
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
								break;
							}
							case dst_iqword: {
								uint64 c;
								analy->bufPtr(addr, (byte *)&c, 8);
								ht_snprintf(buf, sizeof buf, "dq          \\@n%016qxh", c);
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
								break;
							}
							case dst_ibyte:
							default: {
								byte c;
								if (analy->bufPtr(addr, &c, 1)==1) {
									sprintf(buf, "db          \\@n%02xh ", c);
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
									sprintf(buf, "; '%c'", (c<32)?32:c);
									putElement(ELEMENT_TYPE_COMMENT, buf);
								} else {
									putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db          ??");
								}
							}
							}
						} else {
							// not initialized
							switch (cur_addr->type.int_subtype) {
							case dst_iword:
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dw          ????");
								break;
							case dst_idword:
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dd          ????????");
								break;
							case dst_iqword:
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "dq          ????????????????");
								break;
							case dst_ibyte:
							default:
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db          ??");
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
								int r = analy->bufPtr(addr, bufread, MIN(cur_addr->type.length, sizeof bufread));
								strcpy(buf, "db          \\@s\"");
								b = buf + 16 + escape_special(buf+16, 100, bufread, r, "\"", false);
								*b++ = '\"'; *b = 0;
								putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
								bytes_line = want_bytes_line = cur_addr->type.length;
								break;
							}
							default: {assert(0);}
							}
						} else {
							putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db          ?");
						}
						break;
					}                         
					default: {
						assert(0);
					}
				}
			} else {
				// not a known address
				bytes_line = want_bytes_line = 1;
				byte c;
				if (analy->validAddress(addr, scinitialized) && (analy->bufPtr(addr, &c, 1)==1)) {
					char buf[20];
					sprintf(buf, "db          \\@n%02xh ", c);
					putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
					sprintf(buf, "; '%c'", (c<32)?32:c);
					putElement(ELEMENT_TYPE_COMMENT, buf);
				} else {
					putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db          ??");
				}
			}
		} else {
			// invalid addr
			Address *next = analy->nextValid(Addr);
			int d;
			if (next && next->isValid() && next->difference(d, Addr)) {
				bytes_line += d;
				char buf[100];
				sprintf(buf, "db          ?? * %d", d);
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, buf);
			} else {
				bytes_line = want_bytes_line = 1;
				putElement(ELEMENT_TYPE_HIGHLIGHT_DATA_CODE, "db          ??");
			}
		}
	}
	
	endLine();
	endAddr();
}

Stream *AnalyserOutput::getGenerateStream()
{
	return NULL;
}

int AnalyserOutput::generateFile(Address *from, Address *to)
{
	if (analy->active) return OUTPUT_GENERATE_ERR_ANALYSER_NOT_FINISHED;
	if (!from->isValid() || !to->isValid()) return OUTPUT_GENERATE_ERR_INVAL;
	Stream *out = getGenerateStream();
	if (!out) return OUTPUT_GENERATE_ERR_INVAL;
	header();
	int line = 0;
	int len = 0;
	while (from->compareTo(to) <= 0) {
		char buffer[1024];
		if (getLineString(buffer, sizeof buffer, from, line)) {
			// FIXME: remove strlen
			uint wr = strlen(buffer);
			if (out->write(buffer, wr) != wr) return OUTPUT_GENERATE_ERR_STREAM;
			
			int tmplen;
			if (getLineByteLength(tmplen, from, line)) {
				len += tmplen;
			}
			line++;
		} else {
			from->add(len);
			len = 0;
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
	Addr->stringify(tbuf, sizeof tbuf, 0);
	DPRINTF("%s -- ",tbuf);
	if (!addr->isValid() || Addr->compareTo(addr) != 0) {
		assert(addr != Addr);
		DPRINTF("not cached1 --");
		delete addr;
		addr = Addr->clone();
		OutAddr oatmp(addr, 0);
		OutAddr *oa = (OutAddr*)out_addrs->get(out_addrs->find(&oatmp));
		if (!oa) {
			DPRINTF("generate\n");
			if (out_addrs->count() > 1024) {
				reset();
				delete addr;
				addr = Addr->clone();
			}
			oa = new OutAddr(Addr, current_time);
			generateAddr(Addr, oa);
			out_addrs->insert(oa);
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

bool AnalyserOutput::getLineByteLength(int &len, Address *Addr, int line)
{
	OutLine *ol = getLine(Addr, line);
	if (ol) {
		len = ol->bytes;
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

char *AnalyserOutput::externalLink(char *s, uint32 type1, uint32 type2, uint32 type3, uint32 type4, void *special)
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

int	AnalyserOutput::nextLine(Address *&Addr, int &line, int n, Address *max)
{
	int res = 0;
	int len;
	while (n--) {
		if (getLineByteLength(len, Addr, line + 1)) {
			line++;
		} else {
			getLineByteLength(len, Addr, line);
			if (Addr->compareTo(max) >= 0) return res;
			if (!Addr->add(len)) return res;
			line = 0;
		}
		res++;
	}
	return res;
}

int	AnalyserOutput::prevLine(Address *&Addr, int &line, int n, Address *min)
{
//#undef DPRINTF2
//#define DPRINTF2(msg...) {ht_snprintf(tbuf, 1024, msg); fprintf(stderr, "%s", tbuf);}
//	char tbuf[1024];
	DPRINTF2("prev_line(%y, %d, %d, %y)\n", Addr, line, n, min);

	int res = 0;
	int cmp = Addr->compareTo(min);
	
	DPRINTF2("cmp=%d\n", cmp);
	/*
	 *	If we have reached |min| and line==0, we're on top
	 */
	if (cmp < 0 || (cmp == 0 && line == 0)) {
		DPRINTF2("geht nicht\n");
		return 0;
	}
	/*
	 *	A simple case: no address-change, only line-changes.
	 *	Go up while line > 0
	 */
	while (n && line) {
		DPRINTF2("simple\n");
		n--;
		line--;
		res++;
	}
	DPRINTF2("test\n");     
	if (!n) return res;
	DPRINTF2("test2\n");

	/*
	 *	Now it gets complicated. We have to go to an other address.
	 *	First we have to figure out where we should start to search for
	 *	the previous address.
	 */
	int min_length, max_length, min_look_ahead, avg_look_ahead, addr_align;
	if (analy->disasm) {
		analy->disasm->getOpcodeMetrics(min_length, max_length, min_look_ahead, avg_look_ahead, addr_align);
	} else {
		min_look_ahead = 1;
		avg_look_ahead = 1;
	}
	
	int l = n*avg_look_ahead;
	if (l < min_look_ahead) l = min_look_ahead;

	/*
	 *	The disassember whats us to go |l| bytes back
	 */
	 
	Address *search_addr = Addr->clone();
	if (!search_addr->add(-l) || search_addr->compareTo(min) < 0) {
		/*
		 *	It isnt possible, to go |l| bytes back. So we start at |min|.
		 */
		delete search_addr;
		search_addr = min->clone();
	}
	DPRINTF2("search-start: %y\n", search_addr);

	/*
	 *	|prev| contains the previous "logical" location.
	 *	That is some address known to be "atomic".
	 */
	Location *prev = analy->enumLocationsReverse(Addr);
	if (prev) {
		DPRINTF2("prev: %y\n", prev->addr);
		/*
		 *	|prevnext| contains the "end address" of |prev|.
		 *	So we know how long (how much bytes) prev is.
		 */
		Address *prevnext = prev->addr->clone();
		if (prevnext->add(getAddrByteLength(prev->addr))) {
			DPRINTF2("mid-test: prevnext %y\n", prevnext);
			if (prevnext->compareTo(Addr) > 0) {
				/*
				 *   We were in the middle of a location.
				 *	We solve this situation by starting a new search
				 *	with |prev->addr|. This is counted as "one line up".
				 */
				delete Addr;
				delete search_addr;
				delete prevnext;
				Addr = prev->addr->clone();
				line = 0;
				res++;
				DPRINTF2("mid\n");
				return prevLine(Addr, line, n-1, min)+res;
			}
			if (prevnext->compareTo(Addr) == 0) {
				delete Addr;
				delete search_addr;
				delete prevnext;
				Addr = prev->addr->clone();
				line = getLineCount(prev->addr)-1;
				res++;
				DPRINTF2("mid2\n");
				return prevLine(Addr, line, n-1, min)+res;
			}
			DPRINTF2("prev: %y prevnext: %y search_addr: %y\n", prev->addr, prevnext, search_addr);
			Address *oldprevnext = prevnext->clone();
			if (prevnext->add(l) && prevnext->compareTo(Addr) >= 0) {
				delete search_addr;
				search_addr = oldprevnext;
				DPRINTF2("prevnext: %y Addr: %y\n", prevnext, Addr);
				DPRINTF2("search_addr: %y\n", search_addr);
				DPRINTF2("mid3\n");
			} else {
				delete oldprevnext;
			}
		}
		delete prevnext;
	}

	int search_line = 0;
	if (search_addr->compareTo(min) < 0) {
		/*
		 *	We have to start the search at |min|.
		 */
		DPRINTF2("search_addr < min\n");
		delete search_addr;
		search_addr = min->clone();
	}

	Address *addrbuf[1024];
	int linebuf[1024];
	int i = 0;
	int len;

	Address *next_addr = search_addr->clone();
	while (1) {
		DPRINTF2("search_addr: (%y, %d) ", search_addr, search_line);
		DPRINTF2("next_addr: %y \n", next_addr);
		if (search_addr->compareTo(Addr) >= 0) {
			if (search_line >= line || (search_addr->compareTo(Addr) > 0)) break;
		}
		if (getLineByteLength(len, search_addr, search_line)) {
			next_addr->add(len);
			search_line++;
		} else {
			delete search_addr;
			search_addr = next_addr->clone();
			search_line = 0;
			continue;
		}
		addrbuf[i & 1023] = search_addr->clone();
		linebuf[i & 1023] = search_line-1;
		i++;
		if (i >= 1023) break;
	}

	delete next_addr;
	delete search_addr;
	
	if (!i) {
		DPRINTF2("no i!\n");
		return res;
	}
	delete Addr;
	if (i >= n) {
		Addr = addrbuf[(i-n) & 1023]->clone();
		line = linebuf[(i-n) & 1023];
		res += n;
		n = 0;
	} else {
		Addr = addrbuf[0]->clone();
		line = linebuf[0];
		res += i;
		n -= i;
	}

	for (int j=0; j < i; j++) delete addrbuf[j];

	if (n) return prevLine(Addr, line, n, min) + res;
	return res;
}

void AnalyserOutput::putElement(int element_type, const char *element)
{
	write(element);
}

void AnalyserOutput::reset()
{
	delete addr;
	addr = new InvalidAddress;
	cur_out_addr = NULL;
	out_addrs->delAll();
}

void AnalyserOutput::write(const char *s)
{
	int len = elementLength(s);
	len = MIN(len, work_buffer_end-work_buffer);
	memcpy(work_buffer, s, len);
	work_buffer += len;
}

void AnalyserOutput::write(const char *s, int n)
{
	n = MIN(n, work_buffer_end-work_buffer);
	memcpy(work_buffer, s, n);
	work_buffer += n;
}
