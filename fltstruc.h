/* 
 *	HT Editor
 *	fltstruc.h
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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

#ifndef __FLTSTRUC_H__
#define __FLTSTRUC_H__

#include "global.h"

#define FLTMAG0 'b'
#define FLTMAG1 'F'
#define FLTMAG2 'L'
#define FLTMAG3 'T'

struct flat_hdr {
	byte magic[4];			/* bFLT magic */
	dword version;			/* binfmt_flat version */
	dword entry;			/* Offset of first executable instruction with text segment from beginning of file*/
	dword data_start;		/* Offset of data segment from beginning of file*/
	dword data_end;			/* Offset of end of data segment from beginning of file*/
	dword bss_end;			/* Offset of end of bss segment from beginning of file*/
					/* (It is assumed that data_end through bss_end for bss segment.) */
	dword stack_size;		/* Size of stack, in bytes */
	dword reloc_start;		/* Offset of relocation records from beginning of file */
	dword reloc_count;		/* Number of relocation records */
	dword filler[7];		/* Reservered, set to zero */
};

#define FLAT_RELOC_TYPE_TEXT 0
#define FLAT_RELOC_TYPE_DATA 1
#define FLAT_RELOC_TYPE_BSS 2

struct flat_reloc {
	dword value; /* 2: type, 30: offset */
};

extern byte FLAT_HEADER_struct[20];

#endif /* __FLTSTRUC_H__ */
