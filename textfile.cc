/*
 *	HT Editor
 *	textfile.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#include "except.h"
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
ht_textfile::ht_textfile(File *file, bool own_file)
	: FileLayer(file, own_file)
{
}

/*
 *	CLASS ht_layer_textfile
 */
 
ht_layer_textfile::ht_layer_textfile(ht_textfile *textfile, bool own_textfile)
	: ht_textfile(textfile, own_textfile)
{
}

bool ht_layer_textfile::convert_ofs2line(FileOfs o, uint *line, uint *pofs) const
{
	return ((ht_textfile*)mFile)->convert_ofs2line(o, line, pofs);
}

bool ht_layer_textfile::convert_line2ofs(uint line, uint pofs, FileOfs *o) const
{
	return ((ht_textfile*)mFile)->convert_line2ofs(line, pofs, o);
}

void ht_layer_textfile::delete_lines(uint line, uint count)
{
	((ht_textfile*)mFile)->delete_lines(line, count);
}

void ht_layer_textfile::delete_chars(uint line, uint ofs, uint count)
{
	((ht_textfile*)mFile)->delete_chars(line, ofs, count);
}

bool ht_layer_textfile::get_char(uint line, char *ch, uint pos)
{
	return ((ht_textfile*)mFile)->get_char(line, ch, pos);
}

bool ht_layer_textfile::getline(uint line, uint pofs, void *buf, uint buflen, uint *retlen, lexer_state *state)
{
	return ((ht_textfile*)mFile)->getline(line, pofs, buf, buflen, retlen, state);
}

uint ht_layer_textfile::getlinelength(uint line) const
{
	return ((ht_textfile*)mFile)->getlinelength(line);
}

void ht_layer_textfile::insert_lines(uint before, uint count, void **line_ends, int *line_end_lens)
{
	((ht_textfile*)mFile)->insert_lines(before, count, line_ends, line_end_lens);
}

void ht_layer_textfile::insert_chars(uint line, uint ofs, void *chars, uint len)
{
	((ht_textfile*)mFile)->insert_chars(line, ofs, chars, len);
}

bool ht_layer_textfile::has_line(uint line)
{
	return ((ht_textfile*)mFile)->has_line(line);
}

uint ht_layer_textfile::linecount() const
{
	return ((ht_textfile*)mFile)->linecount();
}

void ht_layer_textfile::set_layered_assume(File *s, bool ownNewLayered, bool changes_applied)
{
/*	File *q = streamfile->get_layered();
	if (q)*/ ((ht_textfile*)mFile)->set_layered_assume(s, ownNewLayered, changes_applied);
/*     else
		streamfile = s;*/
}

void ht_layer_textfile::set_lexer(ht_syntax_lexer *lexer)
{
	((ht_textfile*)mFile)->set_lexer(lexer);
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
 
ht_ltextfile::ht_ltextfile(File *streamfile, bool own_streamfile, ht_syntax_lexer *l)
	: ht_textfile(streamfile, own_streamfile)
{
	lexer = l;
	lines = new Array(true);
	orig_lines = new Array(true);
	reread();
	first_parse_dirty_line = linecount();
	first_nofs_dirty_line = linecount();
	dirty = false;
	ofs = 0;
}

ht_ltextfile::~ht_ltextfile()
{
	delete lines;
	delete orig_lines;
}

void ht_ltextfile::cache_invd()
{
	lines->delAll();
	foreach(ht_ltextfile_line, l, *orig_lines, {
		lines->insert(new ht_ltextfile_line(*l));
	});
	dirty_parse(0);
/*	reread();*/
	dirty = false;
}

void ht_ltextfile::cache_flush()
{
}

bool ht_ltextfile::convert_line2ofs(uint line, uint pofs, FileOfs *o) const
{
	ht_ltextfile_line *x;
	x=fetch_line_nofs_ok(line);
	if (x) {
		uint m=getlinelength_i(x);
		if (pofs<m) {
			*o=x->nofs+pofs;
		} else {
			*o=x->nofs+m;
		}
		return true;
	}
	return false;
}

bool ht_ltextfile::convert_ofs2line(FileOfs o, uint *line, uint *pofs) const
{
	uint l=0, r=linecount();
	if (!r) return 0; else r--;
	uint m=0;
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

FileOfs ht_ltextfile::copyAllTo(Stream *stream)
{
/*	uint c=linecount();
	char buf[TEXTFILE_MAX_LINELEN+1];
	for (uint i=0; i<c; i++) {
		ht_ltextfile_line *e=fetch_line(i);
		if (e) {
			uint s;
			getline(i, 0, buf, TEXTFILE_MAX_LINELEN+1, &s, NULL);
			stream->write(buf, s);
			stream->write(e->lineend, e->lineendlen);
		}
	}*/
#define STREAM_COPYBUF_SIZE (64*1024)
	const uint bufsize=STREAM_COPYBUF_SIZE;
	byte *buf = ht_malloc(bufsize);
	uint r;
	FileOfs res = 0;
	do {
		r = read(buf, bufsize);
		stream->writex(buf, r);
		res += r;
	} while (r);
	free(buf);
	return res;
}

void ht_ltextfile::delete_chars(uint line, uint ofs, uint count)
{
	ht_ltextfile_line *e=fetch_line_into_memory(line);
	if (e) {
		char *ostr = e->in_memory.data;
		uint olen = e->in_memory.len;
		
		if (ofs < olen) {
			if (ofs+count>olen) count=olen-ofs;
			char *nstr = ht_malloc(olen-count);
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

void ht_ltextfile::delete_lines(uint line, uint count)
{
	if (count) {
		lines->delRange(line, line+count-1);
		dirty_parse(line);
		dirty_nofs(line);
		dirty=true;
	}
}

void ht_ltextfile::dirty_parse(uint line)
{
	if (line<first_parse_dirty_line) first_parse_dirty_line=line;
}

void ht_ltextfile::dirty_nofs(uint line)
{
	if (line<first_nofs_dirty_line) first_nofs_dirty_line=line;
}

void ht_ltextfile::extend(FileOfs newsize)
{
	/* FIXME: nyi */
	throw IOException(ENOSYS);
}

ht_ltextfile_line *ht_ltextfile::fetch_line(uint line) const
{
	return (ht_ltextfile_line*)(*lines)[line];
}

ht_ltextfile_line *ht_ltextfile::fetch_line_nofs_ok(uint line) const
{
	if (is_dirty_nofs(line)) update_nofs(line);
	return fetch_line(line);
}
			
uint ht_ltextfile::find_linelen_forwd(byte *buf, uint maxbuflen, FileOfs ofs, int *le_len)
{
	uint readlen=(maxbuflen>TEXTFILE_READSIZE) ? TEXTFILE_READSIZE : maxbuflen;
	byte *bufp;
	uint s;
	uint len = 0;
	
	if (le_len) *le_len = 0;
	do {
		mFile->seek(ofs);
		s = mFile->read(buf, readlen);
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

ht_ltextfile_line *ht_ltextfile::fetch_line_into_memory(uint line)
{
	ht_ltextfile_line *e=fetch_line(line);
	if (e) {
		if (!e->is_in_memory) {
			char *data = ht_malloc(e->on_disk.len);
			mFile->seek(e->on_disk.ofs);
			uint x=mFile->read(data, e->on_disk.len);

			e->is_in_memory=true;
			e->in_memory.data=data;
			e->in_memory.len=x/*e->data.on_disk.len*/;
		}
	}
	return e;
}

FileOfs	ht_ltextfile::getSize() const
{
	int line = linecount()-1;
	FileOfs o = 0;
	convert_line2ofs(line, getlinelength(line), &o);
	return o;
}

bool ht_ltextfile::get_char(uint line, char *ch, uint pos)
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
				mFile->seek(e->on_disk.ofs);
				mFile->readx(ch, 1);
				return true;
			}
		}
	}
	return false;
}


bool ht_ltextfile::getline(uint line, uint pofs, void *buf, uint buflen, uint *retlen, lexer_state *state)
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

		uint ret;
		if (e->is_in_memory) {
			uint l=e->in_memory.len;
			if (l>buflen-1) l=buflen-1;
			if (pofs>l) l=0; else l-=pofs;
			memcpy(buf, e->in_memory.data+pofs, l);
			ret=l;
		} else {
			uint l=e->on_disk.len;
			if (l>buflen-1) l=buflen-1;
			if (pofs>l) l=0; else l-=pofs;
			mFile->seek(e->on_disk.ofs+pofs);
			mFile->read(buf, l);
			ret=l;
		}
		if (state) *state=e->instate;
		*retlen=ret;
		return true;
	}
	return false;
}

uint ht_ltextfile::getlinelength(uint line) const
{
	ht_ltextfile_line *e=fetch_line(line);
	return getlinelength_i(e);
}

uint ht_ltextfile::getlinelength_i(ht_ltextfile_line *e) const
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

void ht_ltextfile::insert_lines(uint before, uint count, void **line_ends, int *line_end_lens)
{
	ht_ltextfile_line *e = fetch_line(before);
	int instate = e->instate;
	while (count--) {
		e = new ht_ltextfile_line();
		e->is_in_memory = true;
		e->instate = instate;
		e->on_disk.ofs = 0xffffffff;
		e->on_disk.len = 0;
		e->in_memory.data = ht_malloc(1);
		e->in_memory.len = 0;
		e->nofs = 0;
		if (line_ends && line_end_lens) {
			e->lineendlen = *line_end_lens++;
			memcpy(e->lineend, *line_ends++, e->lineendlen);
		} else {
			e->lineendlen = 1;
			e->lineend[0] = '\n';
		}
		lines->insertBefore(before, e);
	}
	dirty_parse(before);
	dirty_nofs(before);
	dirty = true;
}

void ht_ltextfile::insert_chars(uint line, uint ofs, void *chars, uint len)
{
	ht_ltextfile_line *e=fetch_line(line);
	if (e) {
		uint olen=e->is_in_memory ? e->in_memory.len : e->on_disk.len;
		if (ofs<=olen) {
			e=fetch_line_into_memory(line);
			char *ostr=e->in_memory.data;

			char *nstr;
			uint nlen=olen;

			uint clen=len;

			if (ofs>nlen) nlen=ofs;
			nlen+=clen;
			nstr = ht_malloc(nlen);

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

bool ht_ltextfile::has_line(uint line)
{
	return (fetch_line(line)!=NULL);
}

bool ht_ltextfile::is_dirty_nofs(uint line) const
{
	return (line >= first_nofs_dirty_line);
}

bool ht_ltextfile::is_dirty_parse(uint line) const
{
	return (line >= first_parse_dirty_line);
}

uint ht_ltextfile::linecount() const
{
	return lines->count();
}

byte *ht_ltextfile::match_lineend_forwd(byte *buf, uint buflen, int *le_len)
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

lexer_state ht_ltextfile::next_instate(uint line)
{
	byte buf[TEXTFILE_MAX_LINELEN+1];
	lexer_state state = 0;

	uint buflen;
	if (!getline(line, 0, buf, TEXTFILE_MAX_LINELEN, &buflen, &state)) return state;
	buf[buflen] = 0;
	
	if (lexer) {
		text_pos p;
		char *bufp = (char*)buf;
		uint toklen;
		bool start_of_line = true;
		p.line = line;
		p.pofs = 0;
		int bufplen = buflen;
		int prev_bufplen = -1;
		while ((lexer->gettoken(bufp, bufplen, p, start_of_line, &state, &toklen))) {
			bufp += toklen;
			p.pofs += toklen;
			bufplen -= toklen;
			start_of_line = false;
			if (!bufplen && !prev_bufplen) break;
			prev_bufplen = bufplen;
		}
	}
	return state;
}

FileOfs ht_ltextfile::next_nofs(ht_ltextfile_line *l) const
{
	if (l) {
		if (l->is_in_memory) return l->nofs+l->in_memory.len+l->lineendlen; else
			return l->nofs+l->on_disk.len+l->lineendlen;
	}
	return 0;
}

void	ht_ltextfile::pstat(pstat_t &s) const
{
	mFile->pstat(s);
	s.size = getSize();
}

uint	ht_ltextfile::read(void *buf, uint size)
{
	FileOfs o=tell();
	uint line;
	uint pofs;
	uint c=0;
	byte *b=(byte*)buf;
	
	if (convert_ofs2line(o, &line, &pofs)) while (size) {
		ht_ltextfile_line *l=fetch_line(line);
		if (!l)
			break;
		uint q;
		if (l->is_in_memory) {
			q=l->in_memory.len;
			uint s=q;
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
			uint r;
			uint s=q;
			if (s>pofs) {
				s-=pofs;
				s=MIN(size, s);
				mFile->seek(l->on_disk.ofs+pofs);
				r = mFile->read(b+c, s);
				if (r!=s) break;
				size-=s;
				c+=s;
				pofs+=s;
			}
		}
		uint s=l->lineendlen;
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
	lines->delAll();
	orig_lines->delAll();
	dirty = false;
	FileOfs ofs=0;
	int ll, pll=-1, ln=0;
	bool firstline = true;
	byte buf[TEXTFILE_MAX_LINELEN+1];

	lexer_state state=0;
	ht_ltextfile_line *e, *ce;

	if (lexer) state=lexer->getinitstate();

	uint l=0;
	while ((l=find_linelen_forwd(buf, sizeof buf, ofs, &ll))) {
		mFile->seek(ofs);
		uint x = mFile->read(buf, l-ll);
		buf[x]=0;

		e=new ht_ltextfile_line();
		e->is_in_memory=false;
		e->instate=state;
		e->on_disk.ofs=ofs;
		e->on_disk.len=l-ll;
		e->nofs=ofs;
		e->lineendlen=ll;
		assert( (uint)ll <= sizeof e->lineend);
		mFile->read(e->lineend, ll);
		lines->insert(e);
		ce=new ht_ltextfile_line();
		*ce = *e;
		orig_lines->insert(ce);

		if (lexer) {
			text_pos p;
			char *bufp=(char*)buf;
			uint toklen;
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
		lines->insert(e);
		ce=new ht_ltextfile_line();
		*ce = *e;
		orig_lines->insert(ce);
	}
}

void ht_ltextfile::split_line(uint a, uint pos, void *line_end, int line_end_len)
{
	if (pos == 0) {
		insert_lines(a, 1, &line_end, &line_end_len);
	} else {
		uint l=getlinelength(a);
		if (pos > l) pos = l;
		char *aline = ht_malloc(pos+1);
		uint alinelen;
		getline(a, 0, aline, pos+1, &alinelen, NULL);
	
		insert_lines(a, 1, &line_end, &line_end_len);
		insert_chars(a, 0, aline, alinelen);

		delete_chars(a+1, 0, pos);
		free(aline);
	}
}

void ht_ltextfile::seek(FileOfs offset)
{
	ofs = offset;
}

void ht_ltextfile::setLayered(File *streamfile, bool ownNewLayered)
{
	FileLayer::setLayered(streamfile, ownNewLayered);
	reread();
}

void ht_ltextfile::set_layered_assume(File *streamfile, bool ownNewLayered, bool changes_applied)
{
	if (changes_applied) {
		orig_lines->delAll();
		uint c = lines->count();
		for (uint i=0; i<c; i++) {
			ht_ltextfile_line *l = fetch_line_nofs_ok(i);
			ht_ltextfile_line *m = new ht_ltextfile_line();
			l->on_disk.ofs = l->nofs;
			if (l->is_in_memory) {
				l->on_disk.len = l->in_memory.len;
				free(l->in_memory.data);
				l->is_in_memory = false;
			}
			*m = *l;
			orig_lines->insert(m);
		}
		cache_invd();
	}
	FileLayer::setLayered(streamfile, ownNewLayered);
}

void ht_ltextfile::set_lexer(ht_syntax_lexer *l)
{
	if (lexer != l) {
		lexer = l;
		dirty_parse(0);
	}
}

FileOfs ht_ltextfile::tell() const
{
	return ofs;
}

void	ht_ltextfile::truncate(FileOfs newsize)
{
	/* FIXME: nyi */
	throw IOException(ENOSYS);
}

void ht_ltextfile::update_parse(uint target)
{
	ht_ltextfile_line *e;
	uint line = first_parse_dirty_line;

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

void ht_ltextfile::update_nofs(uint target) const
{
	ht_ltextfile_line *e;
	uint line=first_nofs_dirty_line;

	FileOfs nofs;
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

int ht_ltextfile::vcntl(uint cmd, va_list vargs)
{
	switch (cmd) {
	case FCNTL_MODS_INVD:
		cache_invd();
		return 0;
	case FCNTL_MODS_FLUSH:
		cache_flush();
		return 0;
	case FCNTL_MODS_IS_DIRTY: {
		FileOfs o = va_arg(vargs, FileOfs);
		FileOfs s = va_arg(vargs, FileOfs);
		bool *b = va_arg(vargs, bool*);
		*b = dirty;
		o = 0;	// gcc warns otherwise
		s = 0;    // gcc warns otherwise
		return 0;
	}
	case FCNTL_MODS_CLEAR_DIRTY_RANGE: {
		FileOfs o = va_arg(vargs, FileOfs);
		FileOfs s = va_arg(vargs, FileOfs);
		dirty = false;
		o = s = 0;
		return 0;
	}
	}
	return ht_textfile::vcntl(cmd, vargs);
}

uint	ht_ltextfile::write(const void *buf, uint size)
{
	FileOfs o = tell();
	uint line;
	uint pofs;
	byte *b = (byte*)buf;
	uint r = 0;
	if (convert_ofs2line(o, &line, &pofs)) {
		r = size;
		while (size) {
			int lelen;
			uint s;
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
