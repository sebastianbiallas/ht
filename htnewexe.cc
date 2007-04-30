/* 
 *	HT Editor
 *	htnewexe.cc
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

#include "htnewexe.h"

#include "mzstruct.h"
#include "endianess.h"

FileOfs get_newexe_header_ofs(File *file)
{
	/* look for mz magic */
	byte mzmagic[2];
	file->seek(0);
	file->read(mzmagic, 2);
	if ((mzmagic[0] != IMAGE_MZ_MAGIC0) || (mzmagic[1] != IMAGE_MZ_MAGIC1))
		return 0;
	/* test if reloc_ofs >= 0x40 */
	uint16 reloc_ofs;
	file->seek(24);
	file->read(&reloc_ofs, 2);
	reloc_ofs = createHostInt(&reloc_ofs, 2, little_endian);
	if (reloc_ofs && reloc_ofs < 0x40) return 0;
	/* ok seems to be a newexe */
	FileOfs newexe_ofs;
	file->seek(60);
	file->read(&newexe_ofs, 4);
	newexe_ofs = createHostInt(&newexe_ofs, 4, little_endian);
	return newexe_ofs;
}
