/* 
 *	HT Editor
 *	analy_register.h
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

#ifndef ANALY_REGISTER
#define ANALY_REGISTER

#include "tools.h"

/*
 *	AAS.: analyser disassembler
 */
#define ATOM_ANALY_ALPHA	MAGICD("AAS\x00")
#define ATOM_ANALY_X86 MAGICD("AAS\x01")
#define ATOM_ANALY_IA64 MAGICD("AAS\x02")
#define ATOM_ANALY_IL MAGICD("AAS\x03")
#define ATOM_ANALY_JAVA MAGICD("AAS\x04")
#define ATOM_ANALY_PPC MAGICD("AAS\x05")

/*
 *	ANX.: analyser objects
 */
#define ATOM_CODE_ANALYSER MAGICD("ANX\x00")
#define ATOM_DATA_ANALYSER MAGICD("ANX\x01")

#define ATOM_ADDR_QUEUE_ITEM MAGICD("ANX\x80")
#define ATOM_ADDR_XREF MAGICD("ANX\x81")

/*
 *	ANA.: analysers
 */
#define ATOM_PE_ANALYSER MAGICD("ANA\x50")
#define ATOM_ELF_ANALYSER MAGICD("ANA\x51")
#define ATOM_COFF_ANALYSER MAGICD("ANA\x52")
#define ATOM_NE_ANALYSER MAGICD("ANA\x53")
#define ATOM_CLASS_ANALYSER MAGICD("ANA\x54")
#define ATOM_LE_ANALYSER MAGICD("ANA\x55")
#define ATOM_MACHO_ANALYSER MAGICD("ANA\x56")
#define ATOM_FLT_ANALYSER MAGICD("ANA\x57")
#define ATOM_XBE_ANALYSER MAGICD("ANA\x58")
#define ATOM_PEF_ANALYSER MAGICD("ANA\x59")

/*
 *	ADR.: addresses
 */
#define ATOM_ADDRESS_INVALID MAGICD("ADR\x00")
#define ATOM_ADDRESS_FLAT_32 MAGICD("ADR\x01")
#define ATOM_ADDRESS_FLAT_64 MAGICD("ADR\x02")

#define ATOM_ADDRESS_X86_FLAT_32 MAGICD("ADR\x10")
#define ATOM_ADDRESS_X86_1616 MAGICD("ADR\x11")
#define ATOM_ADDRESS_X86_1632 MAGICD("ADR\x12")

bool init_analyser();
void done_analyser();

#endif
