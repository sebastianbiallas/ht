/* 
 *	HT Editor
 *	analy_register.h
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

#ifndef ANALY_REGISTER
#define ANALY_REGISTER

#include "tools.h"

#define ATOM_ANALY_ALPHA MAGICD("ANA\x00")
#define ATOM_ANALY_X86 MAGICD("ANA\x01")
#define ATOM_CODE_ANALYSER MAGICD("ANA\x02")
#define ATOM_DATA_ANALYSER MAGICD("ANA\x03")

#define ATOM_ADDRQUEUEMEMBER MAGICD("ANA\x10")
#define ATOM_ADDR_XREF MAGICD("ANA\x11")
#define ATOM_HT_ADDR MAGICD("ANA\x12")

#define ATOM_PE_ANALYSER MAGICD("ANA\x20")
#define ATOM_ELF_ANALYSER MAGICD("ANA\x21")
#define ATOM_COFF_ANALYSER MAGICD("ANA\x22")
#define ATOM_NE_ANALYSER MAGICD("ANA\x23")

bool init_analyser();
void done_analyser();

#endif
