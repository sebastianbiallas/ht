/*
 *	HT Editor
 *	tools.cc
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

#include "tools.h"

#include "data.h"
#include "htdebug.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

uint32 delinearize(uint32 d)
{
	return d*0x8088405+1;	/* there's magic in here... */
}

uint64 delinearize64(uint64 d)
{
	return (uint64(delinearize(d))<<32)| delinearize(d>>32); /* there's less magic in here... */
}

int compare_keys_uint_delinear(Object *key_a, Object *key_b)
{
	uint a = delinearize(((UInt*)key_a)->value);
	uint b = delinearize(((UInt*)key_b)->value);
	if (a>b) return 1; else if (a<b) return -1;
	return 0;
}

int *random_permutation(int max)
{
	if (!max) return NULL;
	int *table= ht_malloc(max * sizeof(int));
	int i,j,k,l,m;
	for (i=0; i<max; i++) table[i] = i;
	for (i=0; i<max; i++) {
		k=rand()%(max);
		l=rand()%(max);
		m=rand()%(max);
		j=table[k];
		table[k]=table[l];
		table[l]=j;
		j=table[i];
		table[i]=table[m];
		table[m]=j;
	}
	return table;
}

/*
 *	entropy shit
 */
#define LN2 0.693147180559945309417232121458177

double calc_entropy(byte *buf, int size)
{
	int p[256];
	if (!size) return 0.0;
	memset(p, 0, sizeof p);
	for (int i=0; i<size; i++) {
		p[*buf]++;
		buf++;
	}
	double result = 0.0;
	for (int i=0; i<256; i++) {
		if (p[i]) {
			double pi = p[i];
			pi /= size;
			result += pi * log(pi) / LN2;
		}
	}
	return -result;
}

/*
 *	buffer size must be 64 bytes
 *	return value will be in range from 0 to 100
 */
int calc_entropy2(byte *buf, int size)
{
	int p[256];
	int result = 0;
	if (size<3) return 0;
	memset(p, 0, sizeof p);
	// pass1
	byte *b = buf;
	for (int i=0; i < size; i++) {
		if (p[*b]++ == 0) {
			result ++;
		}
		b++;
	}
	memset(p, 0, sizeof p);
	b = buf;
	// pass2
	size--;
	for (int i=0; i < size; i++) {
		int d = b[0]-b[1];
		if (d<0) d = -d;
		if (p[d]++ == 0) {
			result ++;
		}
		b++;
	}
	memset(p, 0, sizeof p);
	b = buf;
	// pass3
	size--;
	for (int i=0; i < size; i++) {
		int d = b[0]-b[1];
		int e = b[1]-b[2];
		if (d<0) d = -d;
		if (e<0) e = -e;
		d = d-e;
		if (d<0) d = -d;
		if (p[d]++ == 0) {
			result ++;
		}
		b++;
	}
	return (result-3)*100/(size*3+3);
}

/*
 * "out of memory" - handlers
 */

int out_of_memory(int size)
{
	printf("Out of memory.");
	exit(1);
	return OUT_OF_MEMORY_FAIL;
}

int (*out_of_memory_func)(int size)=&out_of_memory;

void *smalloc(size_t size)
{
	void *p;
retry:
	if ((p=malloc(size))) return p;
	switch (out_of_memory_func(size)) {
		case OUT_OF_MEMORY_IGNORE:
			return NULL;
		case OUT_OF_MEMORY_RETRY:
			goto retry;
		default:
			exit(666);
	}
}

void *smalloc0(size_t size)
{
	void *p;
retry0:
	if ((p=malloc(size))) return memset(p, 0, size);
	switch (out_of_memory_func(size)) {
		case OUT_OF_MEMORY_IGNORE:
			return NULL;
		case OUT_OF_MEMORY_RETRY:
			goto retry0;
		default:
			exit(666);
	}
}

