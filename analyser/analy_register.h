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

#define ATOM_ANALY_ALPHA	MAGICD("ANA\x00")
#define ATOM_ANALY_X86 MAGICD("ANA\x01")
#define ATOM_ANALY_JAVA MAGICD("ANA\x02")

#define ATOM_ADDR_QUEUE_ITEM MAGICD("ANA\x10")
#define ATOM_ADDR_XREF MAGICD("ANA\x11")

#define ATOM_PE_ANALYSER MAGICD("ANA\x20")
#define ATOM_ELF_ANALYSER MAGICD("ANA\x21")
#define ATOM_COFF_ANALYSER MAGICD("ANA\x22")
#define ATOM_NE_ANALYSER MAGICD("ANA\x23")
#define ATOM_CLASS_ANALYSER MAGICD("ANA\x24")

#define ATOM_CODE_ANALYSER MAGICD("ANA\x40")
#define ATOM_DATA_ANALYSER MAGICD("ANA\x41")

#define ATOM_ADDRESS_INVALID MAGICD("ANA\x50")
#define ATOM_ADDRESS_FLAT_32 MAGICD("ANA\x51")
#define ATOM_ADDRESS_FLAT_64 MAGICD("ANA\x52")

#define ATOM_ADDRESS_X86_FLAT_32 MAGICD("ANA\x60")
#define ATOM_ADDRESS_X86_1616 MAGICD("ANA\x61")
#define ATOM_ADDRESS_X86_1632 MAGICD("ANA\x62")
#define ATOM_ADDRESS_ALPHA_FLAT_32 MAGICD("ANA\x70")

bool init_analyser();
void done_analyser();

#endif
