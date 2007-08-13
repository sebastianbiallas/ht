/* 
 *	HT Editor
 *	htmachohd.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#include "machostruc.h"
#include "atom.h"
#include "htmacho.h"
#include "htmachohd.h"
#include "httag.h"
#include "formats.h"
#include "snprintf.h"

static ht_mask_ptable machoheader[]=
{
	{"magic",		STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cputype",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"cpusubtype",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"filetype",		STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"number of cmds",	STATICTAG_EDIT_DWORD_VE("00000010")},
	{"size of cmds",	STATICTAG_EDIT_DWORD_VE("00000014")},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000018")},
	{0, 0}
};

static ht_mask_ptable macho_segment_header[]=
{
	{"cmd",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cmdsize",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"name",
STATICTAG_EDIT_CHAR("00000008")STATICTAG_EDIT_CHAR("00000009")
STATICTAG_EDIT_CHAR("0000000a")STATICTAG_EDIT_CHAR("0000000b")
STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")
STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")
STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")
STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")
STATICTAG_EDIT_CHAR("00000014")STATICTAG_EDIT_CHAR("00000015")
STATICTAG_EDIT_CHAR("00000016")STATICTAG_EDIT_CHAR("00000017")
},
	{"virtual address",	STATICTAG_EDIT_DWORD_VE("00000018")},
	{"virtual size",	STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"file offset",		STATICTAG_EDIT_DWORD_VE("00000020")},
	{"file size",		STATICTAG_EDIT_DWORD_VE("00000024")},
	{"max VM protection",	STATICTAG_EDIT_DWORD_VE("00000028")},
	{"init VM protection",	STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"number of sections",	STATICTAG_EDIT_DWORD_VE("00000030")},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000034")},
	{0, 0}
};

static ht_mask_ptable macho_segment_64_header[]=
{
	{"cmd",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cmdsize",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"name",
STATICTAG_EDIT_CHAR("00000008")STATICTAG_EDIT_CHAR("00000009")
STATICTAG_EDIT_CHAR("0000000a")STATICTAG_EDIT_CHAR("0000000b")
STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")
STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")
STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")
STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")
STATICTAG_EDIT_CHAR("00000014")STATICTAG_EDIT_CHAR("00000015")
STATICTAG_EDIT_CHAR("00000016")STATICTAG_EDIT_CHAR("00000017")
},
	{"virtual address",	STATICTAG_EDIT_QWORD_VE("00000018")},
	{"virtual size",	STATICTAG_EDIT_QWORD_VE("00000020")},
	{"file offset",		STATICTAG_EDIT_QWORD_VE("00000028")},
	{"file size",		STATICTAG_EDIT_QWORD_VE("00000030")},
	{"max VM protection",	STATICTAG_EDIT_DWORD_VE("00000038")},
	{"init VM protection",	STATICTAG_EDIT_DWORD_VE("0000003c")},
	{"number of sections",	STATICTAG_EDIT_DWORD_VE("00000040")},
	{"flags",		STATICTAG_EDIT_DWORD_VE("00000044")},
	{0, 0}
};

static ht_mask_ptable macho_section_header[]=
{
	{"section name",
STATICTAG_EDIT_CHAR("00000000")STATICTAG_EDIT_CHAR("00000001")
STATICTAG_EDIT_CHAR("00000002")STATICTAG_EDIT_CHAR("00000003")
STATICTAG_EDIT_CHAR("00000004")STATICTAG_EDIT_CHAR("00000005")
STATICTAG_EDIT_CHAR("00000006")STATICTAG_EDIT_CHAR("00000007")
STATICTAG_EDIT_CHAR("00000008")STATICTAG_EDIT_CHAR("00000009")
STATICTAG_EDIT_CHAR("0000000a")STATICTAG_EDIT_CHAR("0000000b")
STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")
STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")
},
	{"segment name",
STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")
STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")
STATICTAG_EDIT_CHAR("00000014")STATICTAG_EDIT_CHAR("00000015")
STATICTAG_EDIT_CHAR("00000016")STATICTAG_EDIT_CHAR("00000017")
STATICTAG_EDIT_CHAR("00000018")STATICTAG_EDIT_CHAR("00000019")
STATICTAG_EDIT_CHAR("0000001a")STATICTAG_EDIT_CHAR("0000001b")
STATICTAG_EDIT_CHAR("0000001c")STATICTAG_EDIT_CHAR("0000001d")
STATICTAG_EDIT_CHAR("0000001e")STATICTAG_EDIT_CHAR("0000001f")
},
	{"virtual address",		STATICTAG_EDIT_DWORD_VE("00000020")},
	{"virtual size",		STATICTAG_EDIT_DWORD_VE("00000024")},
	{"file offset",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"alignment",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"relocation file offset",	STATICTAG_EDIT_DWORD_VE("00000030")},
	{"number of relocation entries",STATICTAG_EDIT_DWORD_VE("00000034")},
	{"flags",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"reserved1",			STATICTAG_EDIT_DWORD_VE("0000003c")},
	{"reserved2",			STATICTAG_EDIT_DWORD_VE("00000040")},
	{0, 0}
};

static ht_mask_ptable macho_section_64_header[]=
{
	{"section name",
STATICTAG_EDIT_CHAR("00000000")STATICTAG_EDIT_CHAR("00000001")
STATICTAG_EDIT_CHAR("00000002")STATICTAG_EDIT_CHAR("00000003")
STATICTAG_EDIT_CHAR("00000004")STATICTAG_EDIT_CHAR("00000005")
STATICTAG_EDIT_CHAR("00000006")STATICTAG_EDIT_CHAR("00000007")
STATICTAG_EDIT_CHAR("00000008")STATICTAG_EDIT_CHAR("00000009")
STATICTAG_EDIT_CHAR("0000000a")STATICTAG_EDIT_CHAR("0000000b")
STATICTAG_EDIT_CHAR("0000000c")STATICTAG_EDIT_CHAR("0000000d")
STATICTAG_EDIT_CHAR("0000000e")STATICTAG_EDIT_CHAR("0000000f")
},
	{"segment name",
STATICTAG_EDIT_CHAR("00000010")STATICTAG_EDIT_CHAR("00000011")
STATICTAG_EDIT_CHAR("00000012")STATICTAG_EDIT_CHAR("00000013")
STATICTAG_EDIT_CHAR("00000014")STATICTAG_EDIT_CHAR("00000015")
STATICTAG_EDIT_CHAR("00000016")STATICTAG_EDIT_CHAR("00000017")
STATICTAG_EDIT_CHAR("00000018")STATICTAG_EDIT_CHAR("00000019")
STATICTAG_EDIT_CHAR("0000001a")STATICTAG_EDIT_CHAR("0000001b")
STATICTAG_EDIT_CHAR("0000001c")STATICTAG_EDIT_CHAR("0000001d")
STATICTAG_EDIT_CHAR("0000001e")STATICTAG_EDIT_CHAR("0000001f")
},
	{"virtual address",		STATICTAG_EDIT_QWORD_VE("00000020")},
	{"virtual size",		STATICTAG_EDIT_QWORD_VE("00000028")},
	{"file offset",			STATICTAG_EDIT_DWORD_VE("00000030")},
	{"alignment",			STATICTAG_EDIT_DWORD_VE("00000034")},
	{"relocation file offset",	STATICTAG_EDIT_DWORD_VE("00000038")},
	{"number of relocation entries",STATICTAG_EDIT_DWORD_VE("0000003c")},
	{"flags",			STATICTAG_EDIT_DWORD_VE("00000040")},
	{"reserved1",			STATICTAG_EDIT_DWORD_VE("00000044")},
	{"reserved2",			STATICTAG_EDIT_DWORD_VE("00000048")},
	{"reserved3",			STATICTAG_EDIT_DWORD_VE("0000004c")},
	{0, 0}
};

static ht_mask_ptable macho_thread_header[]=
{
	{"cmd",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"cmdsize",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"flavor",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"count (of 32bit words)",	STATICTAG_EDIT_DWORD_VE("0000000c")},
	{0, 0}
};

static ht_mask_ptable macho_ppc_thread_state[]=
{
	{"srr0",		STATICTAG_EDIT_DWORD_VE("00000000")},
	{"srr1",		STATICTAG_EDIT_DWORD_VE("00000004")},
	{"flavor",		STATICTAG_EDIT_DWORD_VE("00000008")},
	{"r0",			STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"r1",			STATICTAG_EDIT_DWORD_VE("00000010")},
	{"r2",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"r3",			STATICTAG_EDIT_DWORD_VE("00000018")},
	{"r4",			STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"r5",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"r6",			STATICTAG_EDIT_DWORD_VE("00000024")},
	{"r7",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"r8",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"r9",			STATICTAG_EDIT_DWORD_VE("00000030")},
	{"r10",			STATICTAG_EDIT_DWORD_VE("00000034")},
	{"r11",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"r12",			STATICTAG_EDIT_DWORD_VE("0000003c")},
	{"r13",			STATICTAG_EDIT_DWORD_VE("00000040")},
	{"r14",			STATICTAG_EDIT_DWORD_VE("00000044")},
	{"r15",			STATICTAG_EDIT_DWORD_VE("00000048")},
	{"r16",			STATICTAG_EDIT_DWORD_VE("0000004c")},
	{"r17",			STATICTAG_EDIT_DWORD_VE("00000050")},
	{"r18",			STATICTAG_EDIT_DWORD_VE("00000054")},
	{"r19",			STATICTAG_EDIT_DWORD_VE("00000058")},
	{"r20",			STATICTAG_EDIT_DWORD_VE("0000005c")},
	{"r21",			STATICTAG_EDIT_DWORD_VE("00000060")},
	{"r22",			STATICTAG_EDIT_DWORD_VE("00000064")},
	{"r23",			STATICTAG_EDIT_DWORD_VE("00000068")},
	{"r24",			STATICTAG_EDIT_DWORD_VE("0000006c")},
	{"r25",			STATICTAG_EDIT_DWORD_VE("00000070")},
	{"r26",			STATICTAG_EDIT_DWORD_VE("00000074")},
	{"r27",			STATICTAG_EDIT_DWORD_VE("00000078")},
	{"r28",			STATICTAG_EDIT_DWORD_VE("0000007c")},
	{"r29",			STATICTAG_EDIT_DWORD_VE("00000080")},
	{"r30",			STATICTAG_EDIT_DWORD_VE("00000084")},
	{"r31",			STATICTAG_EDIT_DWORD_VE("00000088")},
	{"cr",			STATICTAG_EDIT_DWORD_VE("0000008c")},
	{"xer",			STATICTAG_EDIT_DWORD_VE("00000090")},
	{"lr",			STATICTAG_EDIT_DWORD_VE("00000094")},
	{"ctr",			STATICTAG_EDIT_DWORD_VE("00000098")},
	{"mq",			STATICTAG_EDIT_DWORD_VE("0000009c")},
	{"vrsave",		STATICTAG_EDIT_DWORD_VE("000000a0")},
	{0, 0}
};

static ht_mask_ptable macho_i386_thread_state[]=
{
	{"eax",			STATICTAG_EDIT_DWORD_VE("00000000")},
	{"ebx",			STATICTAG_EDIT_DWORD_VE("00000004")},
	{"ecx",			STATICTAG_EDIT_DWORD_VE("00000008")},
	{"edx",			STATICTAG_EDIT_DWORD_VE("0000000c")},
	{"edi",			STATICTAG_EDIT_DWORD_VE("00000010")},
	{"esi",			STATICTAG_EDIT_DWORD_VE("00000014")},
	{"ebp",			STATICTAG_EDIT_DWORD_VE("00000018")},
	{"esp",			STATICTAG_EDIT_DWORD_VE("0000001c")},
	{"ss",			STATICTAG_EDIT_DWORD_VE("00000020")},
	{"eflags",		STATICTAG_EDIT_DWORD_VE("00000024")},
	{"eip",			STATICTAG_EDIT_DWORD_VE("00000028")},
	{"cs",			STATICTAG_EDIT_DWORD_VE("0000002c")},
	{"ds",			STATICTAG_EDIT_DWORD_VE("00000030")},
	{"es",			STATICTAG_EDIT_DWORD_VE("00000034")},
	{"fs",			STATICTAG_EDIT_DWORD_VE("00000038")},
	{"gs",			STATICTAG_EDIT_DWORD_VE("0000003c")},
	{0, 0}
};

static ht_view *htmachoheader_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_macho_shared_data *macho_shared=(ht_macho_shared_data *)group->get_shared_data();
	
	ht_uformat_viewer *v=new ht_uformat_viewer();
	v->init(b, DESC_MACHO_HEADER, VC_EDIT, file, group);
	ht_mask_sub *m = new ht_mask_sub();
	m->init(file, 0);
	char info[128];
	ht_snprintf(info, sizeof info, "* Mach-O header at offset %08qx", macho_shared->header_ofs);

	bool isbigendian;
	switch (macho_shared->image_endianess) {
	case little_endian: isbigendian = false; break;
	case big_endian: isbigendian = true; break;
	}

	m->add_mask(info);
	m->add_staticmask_ptable(machoheader, macho_shared->header_ofs, isbigendian);

	FileOfs ofs = macho_shared->header_ofs;
	if (macho_shared->_64) {
		ofs += 8*4;
	} else {
		ofs += 7*4;
	}
	for (uint i=0; i<macho_shared->cmds.count; i++) {
		switch (macho_shared->cmds.cmds[i]->cmd.cmd) {
			case LC_SEGMENT: {
				MACHO_SEGMENT_COMMAND *c = (MACHO_SEGMENT_COMMAND *)macho_shared->cmds.cmds[i];
			    	char info[128];
				ht_snprintf(info, sizeof info, "** segment %s", c->segname);
				m->add_mask(info);
				m->add_staticmask_ptable(macho_segment_header, ofs, isbigendian);
				FileOfs sofs = sizeof (MACHO_SEGMENT_COMMAND);
				for (uint j=0; j < c->nsects; j++) {
					ht_snprintf(info, sizeof info, "**** section %d ****", j);
					m->add_mask(info);
					m->add_staticmask_ptable(macho_section_header, ofs+sofs, isbigendian);
					sofs += 9*4+16+16;
				}
				break;
			}
			case LC_SEGMENT_64: {
				MACHO_SEGMENT_64_COMMAND *c = (MACHO_SEGMENT_64_COMMAND *)macho_shared->cmds.cmds[i];
			    	char info[128];
				ht_snprintf(info, sizeof info, "** segment64 %s", c->segname);
				m->add_mask(info);
				m->add_staticmask_ptable(macho_segment_64_header, ofs, isbigendian);
				FileOfs sofs = sizeof (MACHO_SEGMENT_64_COMMAND);
				for (uint j=0; j < c->nsects; j++) {
					ht_snprintf(info, sizeof info, "**** section %d ****", j);
					m->add_mask(info);
					m->add_staticmask_ptable(macho_section_64_header, ofs+sofs, isbigendian);
					sofs += 2*8+8*4+16+16;
				}
				break;
			}
			case LC_SYMTAB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** SYMTAB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_SYMSEG: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** SYMSEG cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_UNIXTHREAD:
			case LC_THREAD: {
				MACHO_THREAD_COMMAND *c = &macho_shared->cmds.cmds[i]->thread;
			    	char info[128];
				ht_snprintf(info, sizeof info, "** %s", (macho_shared->cmds.cmds[i]->cmd.cmd == LC_UNIXTHREAD) ? "UNIXTHREAD" : "THREAD");
				m->add_mask(info);
				m->add_staticmask_ptable(macho_thread_header, ofs, isbigendian);
				switch (macho_shared->header.cputype) {
				case MACHO_CPU_TYPE_I386:
					switch (c->flavor) {
					case -1:
						m->add_staticmask_ptable(macho_i386_thread_state, ofs+4*4/*4 32bit words in thread_header*/, isbigendian);
						break;
					}
					break;
				case MACHO_CPU_TYPE_POWERPC:
					switch (c->flavor) {
					case FLAVOR_PPC_THREAD_STATE:
						m->add_staticmask_ptable(macho_ppc_thread_state, ofs+4*4/*4 32bit words in thread_header*/, isbigendian);
						break;
					}
					break;
				}
				break;
			}
/*			case LC_THREAD: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** THREAD cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_UNIXTHREAD: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** UNIXTHREAD cmdsize %08x", macho_shared->cmds[i]->cmdsize);
				m->add_mask(info);
				break;
			}*/
			case LC_LOADFVMLIB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** LOADFVMLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_IDFVMLIB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** IDFVMLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_IDENT: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** IDENT (obsolete) cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_FVMFILE: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** FVMFILE cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_PREPAGE: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** PREPAGE cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_DYSYMTAB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** DYSYMTAB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOAD_DYLIB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** LOAD_DYLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_ID_DYLIB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** ID_DYLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_LOAD_DYLINKER: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** LOAD_DYLINKER cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_ID_DYLINKER: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** ID_DYLINKER cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_PREBOUND_DYLIB: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** PREBOUND_DYLIB cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			case LC_UUID: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** UUID cmdsize %08x", macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
				break;
			}
			default: {
			    	char info[128];
				ht_snprintf(info, sizeof info, "** unsupported load command %08x, size %08x", macho_shared->cmds.cmds[i]->cmd.cmd, macho_shared->cmds.cmds[i]->cmd.cmdsize);
				m->add_mask(info);
			}
		}
		ofs += macho_shared->cmds.cmds[i]->cmd.cmdsize;
	}
	v->insertsub(m);
	return v;
}

format_viewer_if htmachoheader_if = {
	htmachoheader_init,
	0
};
