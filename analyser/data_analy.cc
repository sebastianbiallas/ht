/* 
 *	HT Editor
 *	data_analy.cc
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

#include <stdio.h>
#include <stdlib.h>
#include "analy.h"
#include "analy_register.h"
#include "data_analy.h"
#include "global.h"

//#undef DPRINTF
//#define DPRINTF(msg...) printf(##msg)
#define DPRINTF(msg...)

void analyser_put_addrtype(ht_object_stream *f, const taddr_type *at)
{
	byte t = (taddr_typetype)at->type;
	f->putIntDec(t, 1, "type");
	switch (t) {
		case dt_unknown:
			break;
		case dt_code:
			f->putIntDec(at->code_subtype, 1, "type");
			break;
		case dt_unknown_data:
			break;
		case dt_int:
			f->putIntDec(at->int_subtype, 1, "type");
			f->putIntDec(at->length, 1, "length");
			break;
		case dt_float:
			f->putIntDec(at->float_subtype, 1, "type");
			f->putIntDec(at->length, 1, "length");
			break;
		case dt_array:
			f->putIntDec(at->array_subtype, 1, "type");
			f->putIntDec(at->length, 1, "length");
			break;
	}     
}

int analyser_get_addrtype(ht_object_stream *f, taddr_type *at)
{
	at->type = (taddr_typetype)f->getIntDec(1, "type");
	at->length = 0;
	switch (at->type) {
		case dt_unknown:
			break;
		case dt_code:
			at->code_subtype = (taddr_code_subtype)f->getIntDec(1, "type");
			break;
		case dt_unknown_data:
			break;
		case dt_int:
			at->int_subtype = (taddr_int_subtype)f->getIntDec(1, "type");
			at->length = f->getIntDec(1, "length");
			break;
		case dt_float:
			at->float_subtype = (taddr_float_subtype)f->getIntDec(1, "type");
			at->length = f->getIntDec(1, "length");
			break;
		case dt_array:
			at->array_subtype = (taddr_array_subtype)f->getIntDec(1, "type");
			at->length = f->getIntDec(1, "length");
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
void	DataAnalyser::init(Analyser *Analy)
{
	analy = Analy;
}

int 	DataAnalyser::load(ht_object_stream *f)
{
	return 0;
}

void	DataAnalyser::done()
{
}

OBJECT_ID	DataAnalyser::object_id()
{
	return ATOM_DATA_ANALYSER;
}

void	DataAnalyser::access(Address *Addr, OP op, int size)
{
}

void	DataAnalyser::setAddressType(Address *Addr, taddr_typetype type, int subtype, int length)
{
	setAddressType(analy->newLocation(Addr), type, subtype, length);
}

void	DataAnalyser::setAddressType(Location *Addr, taddr_typetype type, int subtype, int length)
{
	DPRINTF("Addr %y set to %d (%d) length %d\n", Addr->addr, type, subtype, length);
	Addr->type.type = type;
	Addr->type.subtype = subtype;
	Addr->type.length = length;
}

void	DataAnalyser::setCodeAddressType(Address *Addr, taddr_code_subtype subtype)
{
	setAddressType(analy->newLocation(Addr), dt_code, subtype, 0);
}

void	DataAnalyser::setCodeAddressType(Location *Addr, taddr_code_subtype subtype)
{
	setAddressType(Addr, dt_code, subtype, 0);
}

void	DataAnalyser::setIntAddressType(Address *Addr, taddr_int_subtype subtype, int length)
{
	setAddressType(analy->newLocation(Addr), dt_int, subtype, length);
}

void	DataAnalyser::setIntAddressType(Location *Addr, taddr_int_subtype subtype, int length)
{
	setAddressType(Addr, dt_int, subtype, length);
}

void	DataAnalyser::setFloatAddressType(Address *Addr, taddr_float_subtype subtype, int length)
{
	setAddressType(analy->newLocation(Addr), dt_float, subtype, length);
}

void	DataAnalyser::setFloatAddressType(Location *Addr, taddr_float_subtype subtype, int length)
{
	setAddressType(Addr, dt_float, subtype, length);
}

void	DataAnalyser::setArrayAddressType(Address *Addr, taddr_array_subtype subtype, int length)
{
	setAddressType(analy->newLocation(Addr), dt_array, subtype, length);
}

void	DataAnalyser::setArrayAddressType(Location *Addr, taddr_array_subtype subtype, int length)
{
	setAddressType(Addr, dt_array, subtype, length);
}

void	DataAnalyser::store(ht_object_stream *f)
{
}

