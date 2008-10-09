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
#define ATOM_ANALY_ALPHA	MAGIC32("AAS\x00")
#define ATOM_ANALY_X86		MAGIC32("AAS\x01")
#define ATOM_ANALY_IA64		MAGIC32("AAS\x02")
#define ATOM_ANALY_IL		MAGIC32("AAS\x03")
#define ATOM_ANALY_JAVA		MAGIC32("AAS\x04")
#define ATOM_ANALY_PPC		MAGIC32("AAS\x05")
#define ATOM_ANALY_ARM          MAGIC32("AAS\x06")
#define ATOM_ANALY_AVR          MAGIC32("AAS\x07")

/*
 *	ANX.: analyser objects
 */
#define ATOM_CODE_ANALYSER	MAGIC32("ANX\x00")
#define ATOM_DATA_ANALYSER	MAGIC32("ANX\x01")

#define ATOM_ADDR_QUEUE_ITEM	MAGIC32("ANX\x80")
#define ATOM_ADDR_XREF		MAGIC32("ANX\x81")

/*
 *	ANA.: analysers
 */
#define ATOM_PE_ANALYSER	MAGIC32("ANA\x50")
#define ATOM_ELF_ANALYSER	MAGIC32("ANA\x51")
#define ATOM_COFF_ANALYSER	MAGIC32("ANA\x52")
#define ATOM_NE_ANALYSER	MAGIC32("ANA\x53")
#define ATOM_CLASS_ANALYSER	MAGIC32("ANA\x54")
#define ATOM_LE_ANALYSER	MAGIC32("ANA\x55")
#define ATOM_MACHO_ANALYSER	MAGIC32("ANA\x56")
#define ATOM_FLT_ANALYSER	MAGIC32("ANA\x57")
#define ATOM_XBE_ANALYSER	MAGIC32("ANA\x58")
#define ATOM_PEF_ANALYSER	MAGIC32("ANA\x59")
#define ATOM_XEX_ANALYSER	MAGIC32("ANA\x5a")

/*
 *	ADR.: addresses
 */
#define ATOM_ADDRESS_INVALID	MAGIC32("ADR\x00")
#define ATOM_ADDRESS_FLAT_32	MAGIC32("ADR\x01")
#define ATOM_ADDRESS_FLAT_64	MAGIC32("ADR\x02")

#define ATOM_ADDRESS_X86_FLAT_32 MAGIC32("ADR\x10")
#define ATOM_ADDRESS_X86_1616	MAGIC32("ADR\x11")
#define ATOM_ADDRESS_X86_1632	MAGIC32("ADR\x12")

bool init_analyser();
void done_analyser();

#endif
