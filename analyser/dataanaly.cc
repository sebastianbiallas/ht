/* 
 *	HT Editor
 *	dataanaly.cc
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

/*
 *	this module was designed to regonize common data structures by
 *	analysing the way of accessing the data as well as analysing the
 *	data itself.
 */

#include <stdio.h>
#include <stdlib.h>
#include "analy.h"
#include "analy_register.h"
#include "dataanaly.h"
#include "global.h"

//#undef DPRINTF
//#define DPRINTF(msg...) printf(##msg)

void analyser_put_addrtype(ht_object_stream *f, const taddr_type *at)
{
	byte t = (taddr_typetype)at->type;
	f->put_int_dec(t, 1, "type");
	switch (t) {
		case dt_unknown:
			break;
		case dt_code:
			f->put_int_dec(at->code_subtype, 1, "type");
			break;
		case dt_unknown_data:
			break;
		case dt_int:
			f->put_int_dec(at->int_subtype, 1, "type");
			f->put_int_dec(at->length, 1, "length");
			break;
		case dt_float:
			f->put_int_dec(at->float_subtype, 1, "type");
			f->put_int_dec(at->length, 1, "length");
			break;
		case dt_array:
			f->put_int_dec(at->array_subtype, 1, "type");
			f->put_int_dec(at->length, 1, "length");
			break;
	}     
}

int analyser_get_addrtype(ht_object_stream *f, taddr_type *at)
{
	at->type = (taddr_typetype)f->get_int_dec(1, "type");
	at->length = 0;
	switch (at->type) {
		case dt_unknown:
			break;
		case dt_code:
			at->code_subtype = (taddr_code_subtype)f->get_int_dec(1, "type");
			break;
		case dt_unknown_data:
			break;
		case dt_int:
			at->int_subtype = (taddr_int_subtype)f->get_int_dec(1, "type");
			at->length = f->get_int_dec(1, "length");
			break;
		case dt_float:
			at->float_subtype = (taddr_float_subtype)f->get_int_dec(1, "type");
			at->length = f->get_int_dec(1, "length");
			break;
		case dt_array:
			at->array_subtype = (taddr_array_subtype)f->get_int_dec(1, "type");
			at->length = f->get_int_dec(1, "length");
			break;
	}
	return f->get_error();
}

/*
 *
 *	types to recognize:
 *		- bytes/words/dwords (by access)
 *		- strings (by examinating)
 *			- pascal
 *			- c
 *             - unicode
 *		- arrays (by access)
 *        - records (partially)
 */
void	data_analyser::init(analyser *Analy)
{
	analy = Analy;
}

int 	data_analyser::load(ht_object_stream *f)
{
	return 0;
}

void	data_analyser::done()
{
}

OBJECT_ID	data_analyser::object_id()
{
	return ATOM_DATA_ANALYSER;
}

void	data_analyser::access(ADDR Addr, OP op, int size)
{
}

void	data_analyser::set_addr_type(ADDR Addr, taddr_typetype type, int subtype, int length)
{
	set_addr_type(analy->new_addr(Addr), type, subtype, length);
}

void	data_analyser::set_addr_type(taddr *Addr, taddr_typetype type, int subtype, int length)
{
	DPRINTF("Addr %08lx set to %d (%d) length %d\n", Addr->addr, type, subtype, length);
	Addr->type.type = type;
	Addr->type.subtype = subtype;
	Addr->type.length = length;
}

void	data_analyser::set_code_addr_type(ADDR Addr, taddr_code_subtype subtype)
{
	set_addr_type(analy->new_addr(Addr), dt_code, subtype, 0);
}

void	data_analyser::set_code_addr_type(taddr *Addr, taddr_code_subtype subtype)
{
	set_addr_type(Addr, dt_code, subtype, 0);
}

void	data_analyser::set_int_addr_type(ADDR Addr, taddr_int_subtype subtype, int length)
{
	set_addr_type(analy->new_addr(Addr), dt_int, subtype, length);
}

void	data_analyser::set_int_addr_type(taddr *Addr, taddr_int_subtype subtype, int length)
{
	set_addr_type(Addr, dt_int, subtype, length);
}

void	data_analyser::set_float_addr_type(ADDR Addr, taddr_float_subtype subtype, int length)
{
	set_addr_type(analy->new_addr(Addr), dt_float, subtype, length);
}

void	data_analyser::set_float_addr_type(taddr *Addr, taddr_float_subtype subtype, int length)
{
	set_addr_type(Addr, dt_float, subtype, length);
}

void	data_analyser::set_array_addr_type(ADDR Addr, taddr_array_subtype subtype, int length)
{
	set_addr_type(analy->new_addr(Addr), dt_array, subtype, length);
}

void	data_analyser::set_array_addr_type(taddr *Addr, taddr_array_subtype subtype, int length)
{
	set_addr_type(Addr, dt_array, subtype, length);
}

void	data_analyser::store(ht_object_stream *f)
{
}

