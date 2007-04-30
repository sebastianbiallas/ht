/* 
 *	HT Editor
 *	srt.h
 *
 *	Copyright (C) 2001, 2002 Stefan Weyergraf
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

#ifndef __SRT_H__
#define __SRT_H__

#include "analy.h"
#include "symmath.h"

enum srt_endian { srte_be, srte_le }; 

/*
 *	CLASS state_mod
 */

#define MTYPE_REG		0
#define MTYPE_FLAG		1
#define MTYPE_MEM		2

class state_mod: public Object {
public:
	bool ismem;
	union {
		uint regidx;
		struct {
			sym_int_token *addr;
			uint size;
			srt_endian endian;
		} mem;		    
	} dest;

	bool isbool;
	union {
		sym_int_token *integer;
		sym_bool_token *boolean;
	} value;

	~state_mod();
};

/*
 *	CLASS sym_int_reg
 */

class sym_int_reg: public sym_int_token {
public:
	uint regidx;

	sym_int_reg(uint r);
/* overwritten */
	virtual bool compare_eq(sym_int_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(uint *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
}; 

/*
 *	CLASS sym_int_mem
 */
 
class sym_int_mem: public sym_int_token {
public:
	sym_int_token *addr;
	uint size;
	srt_endian endian;

	sym_int_mem(sym_int_token *a, uint s, srt_endian e);
/* overwritten */
	virtual bool compare_eq(sym_int_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(uint *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
};

/***/

char *srt_endian_to_str(srt_endian endian);

void test_srt(Analyser *analy, Address *addr);

#endif /* __SRT_H__ */
