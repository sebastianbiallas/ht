/*
 *	HT Editor
 *	analy_register.cc
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
 
#include "analy_register.h"
#include "common.h"
#include "global.h"

#include "analy_alpha.h"
#include "analy_java.h"
#include "analy_ia64.h"
#include "analy_il.h"
#include "analy_x86.h"
#include "class_analy.h"
#include "code_analy.h"
#include "coff_analy.h"
#include "data_analy.h"
#include "elf_analy.h"
#include "ne_analy.h"
#include "pe_analy.h"
#include "le_analy.h"

#include "htatom.h"

BUILDER(ATOM_ANALY_ALPHA, AnalyAlphaDisassembler)
BUILDER(ATOM_ANALY_X86, AnalyX86Disassembler)
BUILDER(ATOM_ANALY_IA64, AnalyIA64Disassembler)
BUILDER(ATOM_ANALY_IL, AnalyILDisassembler)
BUILDER(ATOM_ANALY_JAVA, AnalyJavaDisassembler)

BUILDER(ATOM_CODE_ANALYSER, CodeAnalyser)
BUILDER(ATOM_DATA_ANALYSER, DataAnalyser)
BUILDER(ATOM_ADDR_QUEUE_ITEM, AddressQueueItem)
BUILDER(ATOM_ADDR_XREF, AddrXRef)

BUILDER(ATOM_PE_ANALYSER, PEAnalyser)
BUILDER(ATOM_ELF_ANALYSER, ElfAnalyser)
BUILDER(ATOM_COFF_ANALYSER, CoffAnalyser)
BUILDER(ATOM_NE_ANALYSER, NEAnalyser)
BUILDER(ATOM_CLASS_ANALYSER, ClassAnalyser)
BUILDER(ATOM_LE_ANALYSER, LEAnalyser)

BUILDER(ATOM_ADDRESS_INVALID, InvalidAddress)
BUILDER(ATOM_ADDRESS_FLAT_32, AddressFlat32)
BUILDER(ATOM_ADDRESS_FLAT_64, AddressFlat64)
BUILDER(ATOM_ADDRESS_X86_FLAT_32, AddressX86Flat32)
BUILDER(ATOM_ADDRESS_X86_1616, AddressX86_1616)
BUILDER(ATOM_ADDRESS_X86_1632, AddressX86_1632)

bool init_analyser()
{
	REGISTER(ATOM_ANALY_ALPHA, AnalyAlphaDisassembler)
	REGISTER(ATOM_ANALY_X86, AnalyX86Disassembler)
	REGISTER(ATOM_ANALY_IA64, AnalyIA64Disassembler)
	REGISTER(ATOM_ANALY_IL, AnalyILDisassembler)
	REGISTER(ATOM_ANALY_JAVA, AnalyJavaDisassembler)

	REGISTER(ATOM_CODE_ANALYSER, CodeAnalyser)
	REGISTER(ATOM_DATA_ANALYSER, DataAnalyser)
	REGISTER(ATOM_ADDR_QUEUE_ITEM, AddressQueueItem)
	REGISTER(ATOM_ADDR_XREF, AddrXRef)

	REGISTER(ATOM_PE_ANALYSER, PEAnalyser)
	REGISTER(ATOM_ELF_ANALYSER, ElfAnalyser)
	REGISTER(ATOM_COFF_ANALYSER, CoffAnalyser)
	REGISTER(ATOM_NE_ANALYSER, NEAnalyser)
	REGISTER(ATOM_CLASS_ANALYSER, ClassAnalyser)
	REGISTER(ATOM_LE_ANALYSER, LEAnalyser)

	REGISTER(ATOM_ADDRESS_INVALID, InvalidAddress)
	REGISTER(ATOM_ADDRESS_FLAT_32, AddressFlat32)
	REGISTER(ATOM_ADDRESS_FLAT_64, AddressFlat64)
	REGISTER(ATOM_ADDRESS_X86_FLAT_32, AddressX86Flat32)
	REGISTER(ATOM_ADDRESS_X86_1616, AddressX86_1616)
	REGISTER(ATOM_ADDRESS_X86_1632, AddressX86_1632)
	return true;
}

void done_analyser()
{
	UNREGISTER(ATOM_ANALY_ALPHA, AnalyAlphaDisassembler)
	UNREGISTER(ATOM_ANALY_X86, AnalyX86Disassembler)
	UNREGISTER(ATOM_ANALY_IA64, AnalyIA64Disassembler)
	UNREGISTER(ATOM_ANALY_IL, AnalyILDisassembler)
	UNREGISTER(ATOM_ANALY_JAVA, AnalyJavaDisassembler)

	UNREGISTER(ATOM_CODE_ANALYSER, CodeAnalyser)
	UNREGISTER(ATOM_DATA_ANALYSER, DataAnalyser)
	UNREGISTER(ATOM_ADDR_QUEUE_ITEM, AddressQueueItem)
	UNREGISTER(ATOM_ADDR_XREF, AddrXRef)

	UNREGISTER(ATOM_PE_ANALYSER, PEAnalyser)
	UNREGISTER(ATOM_ELF_ANALYSER, ElfAnalyser)
	UNREGISTER(ATOM_COFF_ANALYSER, CoffAnalyser)
	UNREGISTER(ATOM_NE_ANALYSER, NEAnalyser)
	UNREGISTER(ATOM_CLASS_ANALYSER, ClassAnalyser)
	UNREGISTER(ATOM_LE_ANALYSER, LEAnalyser)

	UNREGISTER(ATOM_ADDRESS_INVALID, InvalidAddress)
	UNREGISTER(ATOM_ADDRESS_FLAT_32, AddressFlat32)
	UNREGISTER(ATOM_ADDRESS_FLAT_64, AddressFlat64)
	UNREGISTER(ATOM_ADDRESS_X86_FLAT_32, AddressX86Flat32)
	UNREGISTER(ATOM_ADDRESS_X86_1616, AddressX86_1616)
	UNREGISTER(ATOM_ADDRESS_X86_1632, AddressX86_1632)
}

