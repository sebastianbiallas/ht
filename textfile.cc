/*
 *	HT Editor
 *	textfile.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "htdebug.h"
#include "textfile.h"
#include "tools.h"

#define TEXTFILE_READSIZE				256
#define TEXTFILE_MAX_LINELEN				512

#define TEXTFILE_MAX_LINEENDLEN			2

//#define TEXTFILE_MAX_UPDATE_PARSE_STEPS		64

/*
 *	CLASS ht_textfile
 */

/*
 *	CLASS ht_layer_textfile
 */
 
void ht_layer_textfile::init(ht_textfile *textfile, bool own_textfile)
{
	ht_textfile::init(textfile, own_textfile);
}

bool ht_layer_textfile::convert_ofs2line(FILEOFS o, UINT *line, UINT *pofs)
{
	return ((ht_textfile*)streamfile)->convert_ofs2line(o, line, pofs);
}

bool ht_layer_textfile::convert_line2ofs(UINT line, UINT pofs, FILEOFS *o)
{
	return ((ht_textfile*)streamfile)->convert_line2ofs(line, pofs, o);
}

void ht_layer_textfile::delete_lines(UINT line, UINT count)
{
	((ht_textfile*)streamfile)->delete_lines(line, count);
}

void ht_layer_textfile::delete_chars(UINT line, UINT ofs, UINT count)
{
	((ht_textfile*)streamfile)->delete_chars(line, ofs, count);
}

bool ht_layer_textfile::get_char(UINT line, char *ch, UINT pos)
{
	return ((ht_textfile*)streamfile)->get_char(line, ch, pos);
}

bool ht_layer_textfile::getline(UINT line, UINT pofs, void *buf, UINT buflen, UINT *retlen, lexer_state *state)
{
	return ((ht_textfile*)streamfile)->getline(line, pofs, buf, buflen, retlen, state);
}

UINT ht_layer_textfile::getlinelength(UINT line)
{
	return ((ht_textfile*)streamfile)->getlinelength(line);
}

void ht_layer_textfile::insert_lines(UINT before, UINT count, void **line_ends, int *line_end_lens)
{
	((ht_textfile*)streamfile)->insert_lines(before, count, line_ends, line_end_lens);
}

void ht_layer_textfile::insert_chars(UINT line, UINT ofs, void *chars, UINT len)
{
	((ht_textfile*)streamfile)->insert_chars(line, ofs, chars, len);
}

bool ht_layer_textfile::has_line(UINT line)
{
	return ((ht_textfile*)streamfile)->has_line(line);
}

UINT ht_layer_textfile::linecount()
{
	return ((ht_textfile*)streamfile)->linecount();
}

void ht_layer_textfile::set_layered_assume(ht_streamfile *s, bool changes_applied)
{
/*	ht_streamfile *q = streamfile->get_layered();
	if (q)*/ ((ht_textfile*)streamfile)->set_layered_assume(s, changes_applied);
/*     else
		streamfile = s;*/
}

void ht_layer_textfile::set_lexer(ht_syntax_lexer *lexer)
{
	((ht_textfile*)streamfile)->set_lexer(lexer);
}

/*
 *	CLASS ht_ltextfile_line
 */

ht_ltextfile_line::~ht_ltextfile_line()
{
	if (is_in_memory) free(in_memory.data);
}

/*
 *	CLASS ht_ltextfile
 */
 
void	ht_ltextfile::init(ht_streamfile *streamfile, bool own_streamfile, ht_syntax_lexer *l)
{
	ht_textfile::init(streamfile, own_streamfile);
	int e = get_error();
	if (get_error()) return;
	lexer = l;
	lines = new ht_clist();
	lines->init();
	orig_lines = new ht_clist();
	orig_lines->init();
	reread();
	first_parse_dirty_line = linecount();
	first_nofs_dirty_line = linecount();
	dirty = false;
	ofs = 0;
}

void ht_ltextfile::done()
{
	lines->destroy();
	delete lines;
	orig_lines->destroy();
	delete orig_lines;
	ht_textfile::done();
}

void ht_ltextfile::cache_invd()
{
	lines->empty();
	UINT c=orig_lines->count();
	for (UINT i=0; i<c; i++) {
		ht_ltextfile_line *l = new ht_ltextfile_line();
		*l = *((ht_ltextfile_line*)orig_lines->get(i));
		lines->append(l);
	}
	dirty_parse(0);
/*	reread();*/
	dirty = false;
}

void ht_ltextfile::cache_flush()
{
}

bool ht_ltextfile::convert_line2ofs(UINT line, UINT pofs, FILEOFS *o)
{
	ht_ltextfile_line *x;
	x=fetch_line_nofs_ok(line);
	if (x) {
		UINT m=getlinelength_i(x);
		if (pofs<m) {
			*o=x->nofs+pofs;
		} else {
			*o=x->nofs+m;
		}
		return true;
	}
	return false;
}

bool ht_ltextfile::convert_ofs2line(FILEOFS o, UINT *line, UINT *pofs)
{
	UINT l=0, r=linecount();
	if (!r) return 0; else r--;
	UINT m=0;
	ht_ltextfile_line *x, *y;
	while (l<=r) {
		m=(l+r) / 2;
		x=fetch_line_nofs_ok(m);
		y=fetch_line_nofs_ok(m+1);
		if ((x->nofs < o) && ((y && (y->nofs <= o)) || !y)) l=m+1; else
		if (x->nofs > o) r=m-1; else break;
	}
	
/* FIXME: debug */
	x=fetch_line_nofs_ok(m);
	if (x) assert(o>=x->nofs);
	x=fetch_line_nofs_ok(m+1);
	if (x) assert(o<x->nofs);
/**/

	x=fetch_line_nofs_ok(m);
	
	*line=m;
	*pofs=o - x->nofs;
	return true;
}

void	ht_ltextfile::copy_to(ht_stream *stream)
{
/*	UINT c=linecount();
	char buf[TEXTFILE_MAX_LINELEN+1];
	for (UINT i=0; i<c; i++) {
		ht_ltextfile_line *e=fetch_line(i);
		if (e) {
			UINT s;
			getline(i, 0, buf, TEXTFILE_MAX_LINELEN+1, &s, NULL);
			stream->write(buf, s);
			stream->write(e->lineend, e->lineendlen);
		}
	}*/
#define STREAM_COPYBUF_SIZE (64*1024)
	const UINT bufsize=STREAM_COPYBUF_SIZE;
	byte *buf=(byte*)malloc(bufsize);
	UINT r;
	do {
		r=read(buf, bufsize);
		stream->write(buf, r);
	} while (r);
	free(buf);
}

void ht_ltextfile::delete_chars(UINT line, UINT ofs, UINT count)
{
	ht_ltextfile_line *e=fetch_line_into_memory(line);
	if (e) {
		char *ostr=e->in_memory.data;
		UINT olen=e->in_memory.len;
		
		if (ofs<olen) {
			if (ofs+count>olen) count=olen-ofs;
			char *nstr=(char*)malloc(olen-count);
			memcpy(nstr, ostr, ofs);
			memcpy(nstr+ofs, ostr+ofs+count, olen-ofs-count);
			free(ostr);
			e->in_memory.data=nstr;
			e->in_memory.len=olen-count;
			dirty_parse(line+1);
			dirty_nofs(line+1);
			dirty=true;
		}
	}
}

void ht_ltextfile::delete_lines(UINT line, UINT count)
{
	lines->del_multiple(line, count);
	dirty_parse(line);
	dirty_nofs(line);
	dirty=true;
}

void ht_ltextfile::dirty_parse(UINT line)
{
	if (line<first_parse_dirty_line) first_parse_dirty_line=line;
}

void ht_ltextfile::dirty_nofs(UINT line)
{
	if (line<first_nofs_dirty_line) first_nofs_dirty_line=line;
}

int	ht_ltextfile::extend(UINT newsize)
{
	/* FIXME: nyi */
	return ENOSYS;
}

ht_ltextfile_line *ht_ltextfile::fetch_line(UINT line)
{
	return (ht_ltextfile_line*)lines->get(line);
}

ht_ltextfile_line *ht_ltextfile::fetch_line_nofs_ok(UINT line)
{
	if (is_dirty_nofs(line)) update_nofs(line);
	return fetch_line(line);
}
			
UINT ht_ltextfile::find_linelen_forwd(byte *buf, UINT maxbuflen, FILEOFS ofs, int *le_len)
{
	UINT readlen=(maxbuflen>TEXTFILE_READSIZE) ? TEXTFILE_READSIZE : maxbuflen;
	byte *bufp;
	UINT s;
	UINT len = 0;
	
	if (le_len) *le_len = 0;
	do {
		streamfile->seek(ofs);
		s = streamfile->read(buf, readlen);
		int l;
		bufp = match_lineend_forwd(buf, s, &l);
		if (bufp) {
			len += bufp-buf+l;
			if (le_len) *le_len = l;
			break;
		}
		if (s != readlen) {
			len += s;
			break;
		}
		/* make sure current and next read overlap
		   to guarantee proper lineend-matching */
		if (s > (TEXTFILE_MAX_LINEENDLEN-1)) {
			len += s-(TEXTFILE_MAX_LINEENDLEN-1);
		}
		ofs += s-(TEXTFILE_MAX_LINEENDLEN-1);
	} while (s == readlen);
	if (len > TEXTFILE_MAX_LINELEN) {
		len = TEXTFILE_MAX_LINELEN;
		if (le_len) *le_len = 0;
	}
	return len;
}

ht_ltextfile_line *ht_ltextfile::fetch_line_into_memory(UINT line)
{
	ht_ltextfile_line *e=fetch_line(line);
	if (e) {
		if (!e->is_in_memory) {
			char *data=(char*)malloc(e->on_disk.len);
			streamfile->seek(e->on_disk.ofs);
			UINT x=streamfile->read(data, e->on_disk.len);

			e->is_in_memory=true;
			e->in_memory.data=data;
			e->in_memory.len=x/*e->data.on_disk.len*/;
		}
	}
	return e;
}

UINT	ht_ltextfile::get_size()
{
	int line = linecount()-1;
	FILEOFS o = 0;
	convert_line2ofs(line, getlinelength(line), &o);
	return o;
}

bool ht_ltextfile::get_char(UINT line, char *ch, UINT pos)
{
	ht_ltextfile_line *e=fetch_line(line);
	if (e) {
		if (e->is_in_memory) {
			if (pos<e->in_memory.len) {
				*ch=e->in_memory.data[pos];
				return true;
			}
		} else {
			if (pos<e->on_disk.len) {
				streamfile->seek(e->on_disk.ofs);
				streamfile->read(ch, 1);
				return true;
			}
		}
	}
	return false;
}


bool ht_ltextfile::getline(UINT line, UINT pofs, void *buf, UINT buflen, UINT *retlen, lexer_state *state)
{
/* <debug> */
/*	bool debug=false;
	if (line & 0x80000000) {
		debug=true;
		line&=0x7fffffff;
	}*/
/* </debug> */

	ht_ltextfile_line *e=fetch_line(line);

	if (e) {
		if (is_dirty_parse(line)) update_parse(line);
		if (is_dirty_nofs(line)) update_nofs(line);

/* <debug> */
/*		if (debug) {
			buf+=sprintf(buf, "%5d:", e->nofs);
			maxbuflen-=9;
		}*/

/* </debug> */

		UINT ret;
		if (e->is_in_memory) {
			UINT l=e->in_memory.len;
			if (l>buflen-1) l=buflen-1;
			if (pofs>l) l=0; else l-=pofs;
			memmove(buf, e->in_memory.data+pofs, l);
			ret=l;
		} else {
			UINT l=e->on_disk.len;
			if (l>buflen-1) l=buflen-1;
			if (pofs>l) l=0; else l-=pofs;
			streamfile->seek(e->on_disk.ofs+pofs);
			streamfile->read(buf, l);
			ret=l;
		}
		if (state) *state=e->instate;
		*retlen=ret;
		return true;
	}
	return false;
}

UINT ht_ltextfile::getlinelength(UINT line)
{
	ht_ltextfile_line *e=fetch_line(line);
	return getlinelength_i(e);
}

UINT ht_ltextfile::getlinelength_i(ht_ltextfile_line *e)
{
	if (e) {
		if (e->is_in_memory) {
			return e->in_memory.len;
		} else {
			return e->on_disk.len;
		}
	}
	return 0;
}

void ht_ltextfile::insert_lines(UINT before, UINT count, void **line_ends, int *line_end_lens)
{
	ht_ltextfile_line *e = fetch_line(before);
	int instate = e->instate;
	while (count--) {
		e = new ht_ltextfile_line();
		e->is_in_memory = true;
		e->instate = instate;
		e->on_disk.ofs = 0xffffffff;
		e->on_disk.len = 0;
		e->in_memory.data = (char*)malloc(1);
		e->in_memory.len = 0;
		e->nofs = 0;
		if (line_ends && line_end_lens) {
			e->lineendlen = *line_end_lens++;
			memmove(e->lineend, *line_ends++, e->lineendlen);
		} else {
			e->lineendlen = 1;
			e->lineend[0] = '\n';
		}
		lines->insert_before(e, before);
	}
	dirty_parse(before);
	dirty_nofs(before);
	dirty = true;
}

void ht_ltextfile::insert_chars(UINT line, UINT ofs, void *chars, UINT len)
{
	ht_ltextfile_line *e=fetch_line(line);
	if (e) {
		UINT olen=e->is_in_memory ? e->in_memory.len : e->on_disk.len;
		if (ofs<=olen) {
			e=fetch_line_into_memory(line);
			char *ostr=e->in_memory.data;

			char *nstr;
			UINT nlen=olen;

			UINT clen=len;

			if (ofs>nlen) nlen=ofs;
			nlen+=clen;
			nstr=(char*)malloc(nlen);

			memcpy(nstr, ostr, ofs);
			memcpy(nstr+ofs, chars, clen);
			memcpy(nstr+ofs+clen, ostr+ofs, olen-ofs);

			e->in_memory.data=nstr;
			e->in_memory.len=nlen;
			dirty_parse(line+1);
			dirty_nofs(line+1);
			dirty=true;
			free(ostr);
		}			
	}
}

bool ht_ltextfile::has_line(UINT line)
{
	return (fetch_line(line)!=NULL);
}

bool ht_ltextfile::is_dirty_nofs(UINT line)
{
	return (line>=first_nofs_dirty_line);
}

bool ht_ltextfile::is_dirty_parse(UINT line)
{
	return (line>=first_parse_dirty_line);
}

UINT ht_ltextfile::linecount()
{
	return lines->count();
}

byte *ht_ltextfile::match_lineend_forwd(byte *buf, UINT buflen, int *le_len)
{
	byte *result=NULL;
	
	byte *n=(byte*)memchr(buf, '\n', buflen);
	if (n) {
		if ((n>buf) && (n[-1] == '\r')) {
			*le_len=2;
			result=n-1;
		} else {
			*le_len=1;
			result=n;
		}
	}
	return result;
}

lexer_state ht_ltextfile::next_instate(UINT line)
{
	byte buf[TEXTFILE_MAX_LINELEN+1];
	lexer_state state = 0;

	UINT buflen;
	if (!getline(line, 0, buf, TEXTFILE_MAX_LINELEN, &buflen, &state)) return state;
	buf[buflen] = 0;
	
	if (lexer) {
		text_pos p;
		char *bufp = (char*)buf;
		UINT toklen;
		bool start_of_line = true;
		p.line = line;
		p.pofs = 0;
		int bufplen = buflen;
		while ((lexer->gettoken(bufp, bufplen, p, start_of_line, &state, &toklen))) {
			bufp += toklen;
			p.pofs += toklen;
			bufplen -= toklen;
			start_of_line = false;
			if (!bufplen) break;
		}
	}
	return state;
}

FILEOFS ht_ltextfile::next_nofs(ht_ltextfile_line *l)
{
	if (l) {
		if (l->is_in_memory) return l->nofs+l->in_memory.len+l->lineendlen; else
			return l->nofs+l->on_disk.len+l->lineendlen;
	}
	return 0;
}

void	ht_ltextfile::pstat(pstat_t *s)
{
	streamfile->pstat(s);
	s->size=get_size();
	s->size_high=0;
}

UINT	ht_ltextfile::read(void *buf, UINT size)
{
	FILEOFS o=tell();
	UINT line;
	UINT pofs;
	UINT c=0;
	byte *b=(byte*)buf;
	
	if (convert_ofs2line(o, &line, &pofs)) while (size) {
		ht_ltextfile_line *l=fetch_line(line);
		if (!l)
			break;
		UINT q;
		if (l->is_in_memory) {
			q=l->in_memory.len;
			UINT s=q;
			if (s>pofs) {
				s-=pofs;
				s=MIN(size, s);
				memcpy(b+c, l->in_memory.data+pofs, s);
				size-=s;
				c+=s;
				pofs+=s;
			}
		} else {
			q=l->on_disk.len;
			UINT r;
			UINT s=q;
			if (s>pofs) {
				s-=pofs;
				s=MIN(size, s);
				streamfile->seek(l->on_disk.ofs+pofs);
				r=streamfile->read(b+c, s);
				if (r!=s) break;
				size-=s;
				c+=s;
				pofs+=s;
			}
		}
		UINT s=l->lineendlen;
		if ((q+s>pofs) && (pofs >= q)) {
			s-=pofs-q;
			s=MIN(size, s);
			memcpy(b+c, l->lineend+(pofs-q), s);
			size-=s;
			c+=s;
		}
		line++;
		pofs=0;
	}
	ofs += c;
	return c;
}

void ht_ltextfile::reread()
{
	lines->empty();
	orig_lines->empty();
	dirty=false;
	FILEOFS ofs=0;
	int ll, pll=-1, ln=0;
	bool firstline=true;
	byte buf[TEXTFILE_MAX_LINELEN+1];

	lexer_state state=0;
	ht_ltextfile_line *e, *ce;

	if (lexer) state=lexer->getinitstate();

	UINT l=0;
	while ((l=find_linelen_forwd(buf, sizeof buf, ofs, &ll))) {
		if (streamfile->seek(ofs) != 0) break;
		UINT x=streamfile->read(buf, l-ll);
		buf[x]=0;

		e=new ht_ltextfile_line();
		e->is_in_memory=false;
		e->instate=state;
		e->on_disk.ofs=ofs;
		e->on_disk.len=l-ll;
		e->nofs=ofs;
		e->lineendlen=ll;
		assert( (UINT)ll <= sizeof e->lineend);
		streamfile->read(e->lineend, ll);
		lines->append(e);
		ce=new ht_ltextfile_line();
		*ce = *e;
		orig_lines->append(ce);

		if (lexer) {
			text_pos p;
			char *bufp=(char*)buf;
			UINT toklen;
			bool start_of_line=true;
			p.line=ln;
			p.pofs=0;
			while (lexer->gettoken(bufp, l-ll-(bufp-(char*)buf), p, start_of_line, &state, &toklen)) {
				bufp+=toklen;
				p.pofs+=toklen;
				start_of_line=false;
			}
		}

		ofs+=l;
		pll=ll;
		firstline=false;
		ln++;
	}

	if (pll || firstline) {
		ll=0;
		e=new ht_ltextfile_line();
		e->is_in_memory=false;
		e->instate=state;
		e->on_disk.ofs=ofs;
		e->on_disk.len=l-ll;
		e->nofs=ofs;
		e->lineendlen=ll;
		lines->append(e);
		ce=new ht_ltextfile_line();
		*ce = *e;
		orig_lines->append(ce);
	}
}

void ht_ltextfile::split_line(UINT a, UINT pos, void *line_end, int line_end_len)
{
	if (pos == 0) {
		insert_lines(a, 1, &line_end, &line_end_len);
	} else {
		UINT l=getlinelength(a);
		if (pos>l) pos=l;
		char *aline=(char*)malloc(pos+1);
		UINT alinelen;
		getline(a, 0, aline, pos+1, &alinelen, NULL);
	
		insert_lines(a, 1, &line_end, &line_end_len);
		insert_chars(a, 0, aline, alinelen);

		delete_chars(a+1, 0, pos);
		free(aline);
	}
}

int	ht_ltextfile::seek(FILEOFS offset)
{
	ofs = offset;
	return 0;
}

void ht_ltextfile::set_layered(ht_streamfile *streamfile)
{
	ht_layer_streamfile::set_layered(streamfile);
	reread();
}

void ht_ltextfile::set_layered_assume(ht_streamfile *streamfile, bool changes_applied)
{
	if (changes_applied) {
		orig_lines->empty();
		UINT c = lines->count();
		for (UINT i=0; i<c; i++) {
			ht_ltextfile_line *l = fetch_line_nofs_ok(i);
			ht_ltextfile_line *m = new ht_ltextfile_line();
			l->on_disk.ofs = l->nofs;
			if (l->is_in_memory) {
				l->on_disk.len = l->in_memory.len;
				free(l->in_memory.data);
				l->is_in_memory = false;
			}
			*m = *l;
			orig_lines->append(m);
		}
		cache_invd();
	}
	ht_layer_streamfile::set_layered(streamfile);
}

void ht_ltextfile::set_lexer(ht_syntax_lexer *l)
{
	if (lexer != l) {
		lexer = l;
		dirty_parse(0);
	}
}

FILEOFS ht_ltextfile::tell()
{
	return ofs;
}

int	ht_ltextfile::truncate(UINT newsize)
{
	/* FIXME: nyi */
	return ENOSYS;
}

void ht_ltextfile::update_parse(UINT target)
{
	ht_ltextfile_line *e;
	UINT line = first_parse_dirty_line;

	lexer_state instate=0;
	if (line) {
		instate = next_instate(line-1);
// FIXME: function help works with this already...
/*		e = fetch_line(line);
		instate = e->instate;*/
	} else {
		if (lexer) instate = lexer->getinitstate();
	}

	while (line<=target) {
		e = fetch_line(line);
		if (!e) break;
//		if (e->instate==instate) break;	/* FIXME: valid simplification ?!... */
		e->instate = instate;
		first_parse_dirty_line = line+1;
		instate = next_instate(line);
		line++;
	}
	e = fetch_line(line);
	if (e) e->instate = instate;
}

void ht_ltextfile::update_nofs(UINT target)
{
	ht_ltextfile_line *e;
	UINT line=first_nofs_dirty_line;

	FILEOFS nofs;
	if (line) {
		e=fetch_line(line-1);
		nofs=next_nofs(e);
	} else {
		nofs=0;
	}

	while (line<=target) {
		e=fetch_line(line);
		if (!e) break;
		e->nofs=nofs;
		first_nofs_dirty_line=line+1;
		nofs=next_nofs(e);
		line++;
	}
}

int ht_ltextfile::vcntl(UINT cmd, va_list vargs)
{
	switch (cmd) {
		case FCNTL_MODS_INVD:
				cache_invd();
				return 0;
		case FCNTL_MODS_FLUSH:
				cache_flush();
				return 0;
		case FCNTL_MODS_IS_DIRTY: {
				FILEOFS o=va_arg(vargs, FILEOFS);
				UINT s=va_arg(vargs, UINT);
				bool *b=va_arg(vargs, bool*);
				*b = dirty;
				o = 0;	// gcc warns otherwise
				s = 0;    // gcc warns otherwise
				return 0;
		}
		case FCNTL_MODS_CLEAR_DIRTY: {
			dirty = false;
			return 0;
		}
	}
	return ht_textfile::vcntl(cmd, vargs);
}

UINT	ht_ltextfile::write(const void *buf, UINT size)
{
	FILEOFS o = tell();
	UINT line;
	UINT pofs;
	byte *b = (byte*)buf;
	UINT r = 0;
	if (convert_ofs2line(o, &line, &pofs)) {
		r = size;
		while (size) {
			int lelen;
			UINT s;
			byte *c = match_lineend_forwd(b, size, &lelen);
			if (c) {
				s = c-b;
				split_line(line, pofs, c, lelen);
			} else {
				s = size;
				lelen = 0;
			}
			insert_chars(line, pofs, b, s);
			s += lelen;
			b += s;
			size -= s;
			line++;
			pofs = 0;
		}
	}
	ofs += r;
	return r;
}
