/*
 *	HT Editor
 *	tools.h
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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "htdata.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef NO_NATIVE_UNALIGNED_MOVES
#define UNALIGNED_MOVE(a, b) memcpy(&(a), &(b), sizeof(a))
#define UNALIGNED_MOVE_CONST(a, b, type) {assert(sizeof(a)==sizeof(type));type c = b;memcpy(&(a), &c, sizeof(a));}
#else
#define UNALIGNED_MOVE(a, b) (a) = (b)
#define UNALIGNED_MOVE_CONST(a, b, type) (a) = (b)
#endif

dword delinearize(dword d);

int compare_keys_int_delinear(ht_data *key_a, ht_data *key_b);
int compare_keys_uint_delinear(ht_data *key_a, ht_data *key_b);

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
