/* 
 *	HT Editor
 *	elf_analy.h
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

#ifndef elf_analy_h
#define elf_analy_h

#include "analy.h"
#include "htelf.h"

class elf_analyser: public analyser {
public:
	ht_elf_shared_data 	*elf_shared;
	ht_streamfile		*file;
	area				*validarea;

			void		init(ht_elf_shared_data *elf_shared, ht_streamfile *File);
			int 		load(ht_object_stream *f);
	virtual	void		done();
	virtual	OBJECT_ID	object_id();

	virtual	UINT		bufptr(ADDR Addr, byte *buf, int size);
	virtual   assembler *create_assembler();
	virtual	FILEADDR	file_addr(ADDR Addr);
	virtual	char		*get_addr_section_name(ADDR Addr);
	virtual	char		*get_name();
	virtual   char		*get_type();
	virtual	void 	init_code_analyser();
			void		init_insert_symbols(int shidx);
	virtual	void 	init_unasm();
	virtual	void 	log(char *msg);
	virtual	ADDR		next_valid(ADDR Addr);
	virtual	void		store(ht_object_stream *f);
	virtual	int		query_config(int mode);
	virtual	ADDR		vaddr(FILEADDR fileaddr);
	virtual	bool 	valid_addr(ADDR Addr, tsectype action);
};

#endif
