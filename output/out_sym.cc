/*
 *	HT Editor
 *	out_sym.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
 *	Copyright (C) 2002 Stefan Weyergraf (stefan@weyergraf.de)
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

/*
 *
 *	Format of .SYM files:
 * ==========================================================================
 *	1. Header, see (1)
 *	2. "seg_count" "Segment tables", see (2)
 *	3. Terminator ???, see (A)
 *
 *   (1) Header
 * ==========================================================================
 *	The header holds the number of symbols+something ('q_count'),
 *	the number of segments ('seg_count') and the module_name.
 *	I dont know what the other fields stand for...
 *	The header always has 32 bytes.
 *
 *	(2) Segment table (starts 16-byte aligned)
 * ==========================================================================
 *	(2.1) Segment Header
 *	- number of symbols "n"
 *	- pointer to "Symbol pointer table" see (2.3), relative to start of
 *	  this header
 *
 *	(2.2) Symbol table
 *	- usually has "n+1" entries of "Symbol table entry", see (3)
 *	- first entry usually is segment descriptor (informative)
 *
 *	(2.3) Symbol pointer table
 *	- consists of "n" entries of 16-bit pointers to a "Symbol table entry"
 *	  (pointers are relative to "Segment Header")
 *
 *   (3) Symbol table entry
 * ==========================================================================
 *	contains 32-bit integer, the address followed by a counted
 *	(Pascal-style) string.
 *
 *   (A) Appendix A - Unsure
 * ==========================================================================
 *	observed files always had an additional 4 bytes appended.
 *	bytes are either 00,00,00,04 or 00,00,00,06
 */

struct ImageSymHeader {
	dword file_size HTPACKED;		// in 16-byte blocks
	word entry_seg HTPACKED;
	word u0 HTPACKED;				// 0000
	word u1 HTPACKED;				// 0015 (some flags ?, 0014 for 16-bit .SYM ?)
	word seg_count HTPACKED;
	dword u2 HTPACKED;				// 04020002 (some flags ?)
	byte	module_name[16] HTPACKED;
};

struct ImageSymSegHeader {
	word next_rec_ofs HTPACKED;		// in 16-byte blocks (ring list, last points to first)
	word sym_count HTPACKED;
	word sym_ptr_table_ptr HTPACKED;
	word seg_idx HTPACKED;
	dword seg_start HTPACKED;
	dword seg_size HTPACKED;
};

struct ImageSymDescriptor {
	dword address HTPACKED;
//	byte  name_len HTPACKED;
//   name;
//   ^^^^ pascal string
};

/*
 *
 */

#define MAX_BYTES_PER_SEGMENT		0xff00
#define MAX_SYMBOLS_PER_SEGMENT	0x4000

static void write_sym(ht_stream *stream, dword addr, char *name, UINT *bytes_written)
{
	ImageSymDescriptor desc;
	desc.address = addr;
	stream->write(&desc, sizeof desc); // FIXME: endianess !
	putstrp(stream, name);
	*bytes_written += sizeof desc + 1 + strlen(name);
}

static void g(ht_stream *stream, Symbol *s, UINT *bytes_written, UINT *symbols_written, word *ptr_table)
{
	if (*bytes_written >= MAX_BYTES_PER_SEGMENT) return;
	if (*symbols_written >= MAX_SYMBOLS_PER_SEGMENT) return;

	dword addr;
	if (s->location->addr->byteSize() == sizeof addr) {
		s->location->addr->putIntoArray((byte*)&addr);
//          addr -= 0xbff70000;	/* FIXME: hack for kernel32.dll */
	} else {
		addr = 0;
	}
	
	ptr_table[*symbols_written] = *bytes_written;
	write_sym(stream, addr, s->name, bytes_written);
	
	(*symbols_written) ++;
}

static void align16(ht_streamfile *file, UINT *bytes_written)
{
	byte c = 0;
	while (*bytes_written % 16) {
		file->write(&c, 1);
		(*bytes_written) ++;
	}
}
 
int export_to_sym(Analyser *analy, ht_streamfile *file)
{
	if ((!analy) || (!file)) return /*HTML_OUTPUT_ERR_GENERIC*/1;
	if (analy->active) return /*HTML_OUTPUT_ERR_ANALY_NOT_FINISHED*/1;

	char *module_name = "TEST";
	ImageSymHeader head;
	ImageSymSegHeader seg_head;

// write dummy header
	memset(&head, 0, sizeof head);
	file->write(&head, sizeof head);


/* foreach ($seg) { */

// write dummy seg header
	memset(&seg_head, 0, sizeof seg_head);
	file->write(&seg_head, sizeof seg_head);

	UINT bytes_written = sizeof seg_head, symbols_written = 0;

	write_sym(file, 0xff000000, "_TEXT", &bytes_written);

	word *ptr_table = (word*)malloc(MAX_SYMBOLS_PER_SEGMENT * sizeof *ptr_table);

	Symbol *sym = NULL;
	while ((sym = analy->enumSymbols(sym))) {
		g(file, sym, &bytes_written, &symbols_written, ptr_table);
	}
	
	UINT sym_ptr_table_ptr = bytes_written;

	// FIXME: endianess !
	file->write(ptr_table, sizeof *ptr_table * symbols_written);
	bytes_written += sizeof *ptr_table * symbols_written;
	free(ptr_table);

	align16(file, &bytes_written);

	// FIXME: wrong code order, endianess !
	dword terminator = 0x04000000;
	file->write(&terminator, sizeof terminator);
	bytes_written += 4;

	file->seek(0x0020);
// write segment header
	seg_head.next_rec_ofs = 0x0002;
	seg_head.sym_count = symbols_written;
	seg_head.sym_ptr_table_ptr = sym_ptr_table_ptr;
	seg_head.seg_idx = 1;
	seg_head.seg_start = 0;
	seg_head.seg_size = 0x0000ffff;
	// FIXME: endianess !
	file->write(&seg_head, sizeof seg_head);
	
/* } */


	bytes_written += sizeof head;
	file->seek(0);
// write header
	head.file_size = (bytes_written-1) / 16;
	head.entry_seg = 0;
	head.u0 = 0;
	head.u1 = 0x0015;
	head.seg_count = 1;
	head.u2 = 0x04020002;
	memcpy(head.module_name, module_name, MIN(strlen(module_name), sizeof head.module_name));
	// FIXME: endianess !
	file->write(&head, sizeof head);

	return 0;
}

