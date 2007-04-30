/* 
 *	HT Editor
 *	data_analy.cc
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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
#include "io/types.h"
#include "snprintf.h"

//#undef DPRINTF
//#define DPRINTF(msg...) {global_analyser_address_string_format = ADDRESS_STRING_FORMAT_LEADING_ZEROS;char buf[1024]; ht_snprintf(buf, sizeof buf, ##msg); fprintf(stdout, buf);}
#define DPRINTF(msg...)

void analyser_put_addrtype(ObjectStream &f, const taddr_type *at)
{
	byte t = (taddr_typetype)at->type;
	f.putInt(t, 1, "type");
	switch (t) {
		case dt_unknown:
			break;
		case dt_code:
			f.putInt(at->code_subtype, 1, "type");
			break;
		case dt_unknown_data:
			break;
		case dt_int:
			f.putInt(at->int_subtype, 1, "type");
			f.putInt(at->length, 1, "length");
			break;
		case dt_float:
			f.putInt(at->float_subtype, 1, "type");
			f.putInt(at->length, 1, "length");
			break;
		case dt_array:
			f.putInt(at->array_subtype, 1, "type");
			f.putInt(at->length, 1, "length");
			break;
	}     
}

void analyser_get_addrtype(ObjectStream &f, taddr_type *at)
{
	at->type = (taddr_typetype)f.getInt(1, "type");
	at->length = 0;
	switch (at->type) {
		case dt_unknown:
			break;
		case dt_code:
			at->code_subtype = (taddr_code_subtype)f.getInt(1, "type");
			break;
		case dt_unknown_data:
			break;
		case dt_int:
			at->int_subtype = (taddr_int_subtype)f.getInt(1, "type");
			at->length = f.getInt(1, "length");
			break;
		case dt_float:
			at->float_subtype = (taddr_float_subtype)f.getInt(1, "type");
			at->length = f.getInt(1, "length");
			break;
		case dt_array:
			at->array_subtype = (taddr_array_subtype)f.getInt(1, "type");
			at->length = f.getInt(1, "length");
			break;
	}
}

DataAnalyser::DataAnalyser()
{
}

void	DataAnalyser::init(Analyser *Analy)
{
	analy = Analy;
}

void	DataAnalyser::load(ObjectStream &f)
{
}

void	DataAnalyser::done()
{
}

ObjectID DataAnalyser::getObjectID() const
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

void	DataAnalyser::store(ObjectStream &f) const
{
}

