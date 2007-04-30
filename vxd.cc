/* 
 *	HT Editor
 *	vxd.cc
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

#include "vxd.h"
#include "vxdserv.h"

vxd_t *find_vxd(vxd_desc *table, int key)
{
	while (table->key!=-1) {
		if (table->key==key) return &table->vxd;
		table++;
	}
	return 0;
}

const char *find_vxd_service(vxd_service_desc *table, int key)
{
	while (table->key!=-1) {
		if (table->key==key) return table->name;
		table++;
	}
	return 0;
}

vxd_desc vxds[] = {
		{ 0x0001,	{"VMM", vxd_vmm_services} },
		{ 0x0002,	{"DEBUG",	vxd_debug_services} },
		{ 0x0003,	{"VPICD",	vxd_vpicd_services} },
		{ 0x0004,	{"VDMAD",	vxd_vdmad_services} },
		{ 0x0005,	{"VTD", vxd_vtd_services} },
		{ 0x0006,	{"V86MMGR", vxd_v86mmgr_services} },
		{ 0x0007,	{"PAGESWAP", vxd_pageswap_services} },
		{ 0x0009,	{"REBOOT", 0} },
		{ 0x000a,	{"VDD", vxd_vdd_services} },
		{ 0x000b,	{"VSD", vxd_vsd_services} },
		{ 0x000c,	{"VMD", vxd_vmd_services} },
		{ 0x000d,	{"VKD", vxd_vkd_services} },
		{ 0x000e,	{"VCD", vxd_vcd_services} },
		{ 0x000f,	{"VPD", vxd_vpd_services} },
		{ 0x0010,	{"IOS", vxd_ios_services} },
		{ 0x0011,	{"VMCPD",	vxd_vmcpd_services} },
		{ 0x0012,	{"EBIOS",	vxd_ebios_services} },
		{ 0x0014,	{"VNETBIOS", vxd_vnetbios_services} },
		{ 0x0015,	{"DOSMGR", vxd_dosmgr_services} },
		{ 0x0017,	{"SHELL",	vxd_shell_services} },
		{ 0x0018,	{"VMPOOL", vxd_vmpool_services} },
		{ 0x001a,	{"DOSNET", vxd_dosnet_services} },
		{ 0x0020,	{"INT13",	vxd_int13_services} },
		{ 0x0021,	{"PAGEFILE", vxd_pagefile_services} },
		{ 0x0026,	{"VPOWERD", vxd_vpowerd_services} },
		{ 0x0027,	{"VXDLDR", vxd_vxdldr_services} },
		{ 0x0028,	{"NDIS", vxd_ndis_services} },
		{ 0x002a,	{"VWIN32", vxd_vwin32_services} },
		{ 0x002b,	{"VCOMM",	vxd_vcomm_services} },
		{ 0x002c,	{"SPOOLER", 0}	},
		{ 0x0031,	{"NETBEUI", 0}	},
		{ 0x0032,	{"VSERVER", 0}	},
		{ 0x0033,	{"CONFIGMG", vxd_configmg_services} },
		{ 0x0034,	{"CM", vxd_cm_services} },
		{ 0x0036,	{"VFBACKUP", vxd_vfbackup_services} },
		{ 0x0037,	{"VMINI",	vxd_vmini_services} },
		{ 0x0038,	{"VCOND",	vxd_vcond_services} },
		{ 0x003d,	{"BIOS", 0} },
		{ 0x003e,	{"WSOCK",	vxd_wsock_services} },
		{ 0x0040,	{"IFSMGR", vxd_ifsmgr_services} },
		{ 0x0041,	{"VCDFSD", 0} },
		{ 0x0043,	{"PCI", vxd_pci_services}	},
		{ 0x0048,	{"PERF", vxd_perf_services} },
		{ 0x004a,	{"MTRR", vxd_mtrr_services} },
		{ 0x004b,	{"NTKERN", vxd_ntkern_services} },
		{ 0x004c,	{"ACPI", vxd_acpi_services} },
		{ 0x004e,	{"SMCLIB", vxd_smclib_services} },
		{ 0x011f,	{"VFLATD", vxd_vflatd_services} },
		{ 0x0202,	{"SIWDEBUG", 0} },
		{ 0x0449,	{"VJOYD",	vxd_vjoyd_services} },
		{ 0x044a,	{"MMDEVLDR", vxd_mmdevldr_services} },
		{ 0x0480,	{"VNETSUP", vxd_vnetsup_services} },
		{ 0x0481,	{"VREDIR", vxd_vredir_services} },
		{ 0x0483,	{"VSHARE", vxd_vshare_services} },
		{ 0x0486,	{"VFAT", 0} },
		{ 0x0487,	{"NWLINK", 0} },
		{ 0x0488,	{"VTDI", vxd_vtdi_services} },
		{ 0x0489,	{"VIP", vxd_vip_services} },
		{ 0x048a,	{"MSTCP",	vxd_mstcp_services} },
		{ 0x048b,	{"VCACHE", vxd_vcache_services} },
		{ 0x048e,	{"NWREDIR", 0}	},
		{ 0x0491,	{"FILESEC", 0}	},
		{ 0x0492,	{"NWSERVER", 0} },
		{ 0x0496,	{"NDIS2SUP", 0} },
		{ 0x0497,	{"MSODISUP", 0} },
		{ 0x0498,	{"SPLITTER", 0} },
		{ 0x0499,	{"PPPMAC", 0} },
		{ 0x049a,	{"VDHCP",	0} },
		{ 0x049d,	{"LOGGER", 0} },
		{ 0x097c,	{"PCCARD", vxd_pccard_services} },
		{ 0x7a5f,	{"SIWVID", 0} },
		{ -1 }					/* Terminator */
};


