/* 
 *	HT Editor
 *	vxd.h
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

#ifndef __VXD_H_
#define __VXD_H_

struct vxd_service_desc {
	int key;
	const char *name;
};

struct vxd_t {
	const char *name;
	vxd_service_desc *services;
};

struct vxd_desc {
	int key;
	vxd_t vxd;
};

extern vxd_desc vxds[];

vxd_t *find_vxd(vxd_desc *table, int key);
const char *find_vxd_service(vxd_service_desc *table, int key);

#endif /* __VXD_H_ */

