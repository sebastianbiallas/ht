/* 
 *	HT Editor
 *	htpeexp.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTPEEXP_H__
#define __HTPEEXP_H__

#include "htdata.h"
#include "formats.h"

extern format_viewer_if htpeexports_if;

/*
 *	CLASS ht_pe_export_viewer
 */

class ht_pe_export_viewer: public ht_uformat_viewer {
public:
			void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *group);
	virtual	void done();
/* overwritten */
	virtual	char *func(UINT i, bool execute);
	virtual	int ref_sel(ID id_low, ID id_high);
};

/*
 *	CLASS ht_pe_export_function
 */

class ht_pe_export_function: public ht_data {
public:
	int ordinal;

	int byname;
	char *name;
	dword address;

	ht_pe_export_function();
	ht_pe_export_function(int address, int ordinal);
	ht_pe_export_function(int address, int ordinal, char *name);
	~ht_pe_export_function();
};

struct ht_pe_export {
	ht_clist *funcs;
};

/*
 *	CLASS ht_pe_export_mask
 */

class ht_pe_export_mask: public ht_sub {
protected:
	ht_pe_export *exp;
	UINT *sort_path;
	UINT *sort_ord;
	UINT *sort_va;
	UINT *sort_name;
	char *firstline;
public:
			void init(ht_streamfile *file, ht_pe_export *exp, char *firstline);
	virtual 	void done();
/* overwritten */
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	void first_line_id(ID *id1, ID *id2);
	virtual	bool getline(char *line, ID id1, ID id2);
	virtual	void last_line_id(ID *id1, ID *id2);
	virtual	int next_line_id(ID *id1, ID *id2, int n);
	virtual	int prev_line_id(ID *id1, ID *id2, int n);
/* new */
			void sortbyord();
			void sortbyva();
			void sortbyname();
			void unsort();
};

#endif /* !__HTPEEXP_H__ */
