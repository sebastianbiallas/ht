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
#include "analy_x86.h"
#include "codeanaly.h"
#include "coff_analy.h"
#include "dataanaly.h"
#include "elf_analy.h"
#include "ne_analy.h"
#include "pe_analy.h"

#include "htatom.h"

BUILDER(ATOM_ANALY_ALPHA, analy_alpha_disassembler)
BUILDER(ATOM_ANALY_X86, analy_x86_disassembler)
BUILDER(ATOM_CODE_ANALYSER, code_analyser)
BUILDER(ATOM_DATA_ANALYSER, data_analyser)
BUILDER(ATOM_ADDRQUEUEMEMBER, addrqueueitem);
BUILDER(ATOM_ADDR_XREF, addr_xref);
BUILDER(ATOM_HT_ADDR, ht_addr);
BUILDER(ATOM_PE_ANALYSER, pe_analyser)
BUILDER(ATOM_ELF_ANALYSER, elf_analyser)
BUILDER(ATOM_COFF_ANALYSER, coff_analyser)
BUILDER(ATOM_NE_ANALYSER, ne_analyser)

bool init_analyser()
{
	REGISTER(ATOM_ANALY_ALPHA, analy_alpha_disassembler)
	REGISTER(ATOM_ANALY_X86, analy_x86_disassembler)
	REGISTER(ATOM_CODE_ANALYSER, code_analyser)
	REGISTER(ATOM_DATA_ANALYSER, data_analyser)
	REGISTER(ATOM_ADDRQUEUEMEMBER, addrqueueitem);
	REGISTER(ATOM_ADDR_XREF, addr_xref);
	REGISTER(ATOM_HT_ADDR, ht_addr);
	REGISTER(ATOM_PE_ANALYSER, pe_analyser)
	REGISTER(ATOM_ELF_ANALYSER, elf_analyser)
	REGISTER(ATOM_COFF_ANALYSER, coff_analyser)
	REGISTER(ATOM_NE_ANALYSER, ne_analyser)
	return true;
}

void done_analyser()
{
	UNREGISTER(ATOM_ANALY_ALPHA, analy_alpha_disassembler)
	UNREGISTER(ATOM_ANALY_X86, analy_x86_disassembler)
	UNREGISTER(ATOM_CODE_ANALYSER, code_analyser)
	UNREGISTER(ATOM_DATA_ANALYSER, data_analyser)
	UNREGISTER(ATOM_ADDRQUEUEMEMBER, addrqueueitem);
	UNREGISTER(ATOM_ADDR_XREF, addr_xref);
	UNREGISTER(ATOM_HT_ADDR, ht_addr);
	UNREGISTER(ATOM_PE_ANALYSER, pe_analyser)
	UNREGISTER(ATOM_ELF_ANALYSER, elf_analyser)
	UNREGISTER(ATOM_COFF_ANALYSER, coff_analyser)
	UNREGISTER(ATOM_NE_ANALYSER, ne_analyser)
}

