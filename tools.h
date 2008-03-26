/*
 *	HT Editor
 *	tools.h
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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdlib.h>
#include <string.h>
#include "io/types.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define UNALIGNED_MOVE(a, b) memcpy(&(a), &(b), sizeof(a))
#define UNALIGNED_MOVE_CONST(a, b, type) {assert(sizeof(a)==sizeof(type));type c = b;memcpy(&(a), &c, sizeof(a));}

uint32 delinearize(uint32 d);
uint64 delinearize64(uint64 d);

int *random_permutation(int max);

double calc_entropy(byte *buf, int size);
int calc_entropy2(byte *buf, int size);

#define OUT_OF_MEMORY_FAIL 0
#define OUT_OF_MEMORY_IGNORE 1
#define OUT_OF_MEMORY_RETRY 2
extern int (*out_of_memory_func)(int size);
int out_of_memory(int size);

// savety mallocs
void *smalloc(size_t size);
void *smalloc0(size_t size);

#endif /* __TOOLS_H__ */
