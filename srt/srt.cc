/*
 *	HT Editor
 *	srt.cc
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

#include "htatom.h"
#include "htiobox.h"
#include "srt.h"
#include "symmath.h"

/* FIXME: ... find a better way ... */
#include "srt_x86.h"

#define ATOM_SYM_INT_REG		MAGICD("SRT\x00")
#define ATOM_SYM_INT_MEM		MAGICD("SRT\x01")

/*
 *	CLASS state_mod
 */

state_mod::~state_mod()
{
	if (ismem) {
		dest.mem.addr->done();
		delete dest.mem.addr;
	}
	if (isbool) {
		value.boolean->done();
		delete value.boolean;
	} else {
		value.integer->done();
		delete value.integer;
	}
}

/*
 *	CLASS sym_int_reg
 */
 
sym_int_reg::sym_int_reg(UINT r)
{
	regidx = r;
}

bool sym_int_reg::compare_eq(sym_int_token *t)
{
	sym_int_reg *s = (sym_int_reg*)t;
	return (regidx == s->regidx);
}

Object *sym_int_reg::duplicate()
{
	return new sym_int_reg(regidx);
}

bool sym_int_reg::evaluate(bool *i)
{
	return false;
}

int sym_int_reg::nstrfy(char *buf, int n)
{
	return sprintf(buf, "reg%d", regidx);
}

OBJECT_ID sym_int_reg::object_id() const
{
	return ATOM_SYM_INT_REG;
}

/*
 *	CLASS sym_int_mem
 */
 

sym_int_mem::sym_int_mem(sym_int_token *a, UINT s, srt_endian e)
{
	addr = a;
	size = s;
	endian = e;
}

bool sym_int_mem::compare_eq(sym_int_token *t)
{
	return false;
}

Object *sym_int_mem::duplicate()
{
	return new sym_int_mem((sym_int*)addr->duplicate(), size, endian);
}

bool sym_int_mem::evaluate(bool *i)
{
	return false;
}

int sym_int_mem::nstrfy(char *buf, int n)
{
	int l = 0;
	l += sprintf(buf+l, "%s%d[", srt_endian_to_str(endian), size);
	l += addr->nstrfy(buf+l, n-l);
	buf[l++] = ']';
	buf[l] = 0;
	return l;
}

OBJECT_ID sym_int_mem::object_id() const
{
	return ATOM_SYM_INT_MEM;
}

/***/

char *srt_endian_to_str(srt_endian endian)
{
	switch (endian) {
		case srte_be: return "be";
		case srte_le: return "le";
	}
	return "?";
}

void test_srt(Analyser *analy, Address *addr)
{
/* FIXME: ... find a better way ... */
	OBJECT_ID a = analy->disasm->object_id();
	if (a == ATOM_DISASM_X86) {
		srt_x86(analy, addr);
	}
}

