/*
 *	HT Editor
 *	qword.h
 *
 *	Copyright (C) 2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __QWORD_H__
#define __QWORD_H__

#include "global.h"		// for types dword and UINT

typedef struct {
	dword hi;
	dword lo;
} qword;

#define QWORD_SET_HI(q, v) (((q).hi) = (v))
#define QWORD_SET_LO(q, v) (((q).lo) = (v))
#define QWORD_GET_HI(q) ((q).hi)
#define QWORD_GET_LO(q) ((q).lo)

#define QWORD_GET_INT(q) ((q).lo)

#ifdef __cplusplus
extern "C" {
#endif
int qword_cmp(qword a, qword b);
qword int_to_qword(int i);
qword qword_add(qword a, qword b);
qword qword_sub(qword a, qword b);
qword qword_mul(qword a, qword b);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
qword to_qword(int i);
qword to_qword(UINT i);

qword operator +(qword a, qword b);
qword operator ++(qword &a);
qword operator ++(qword &a, int b);
qword operator += (qword &a, qword b);
qword operator - (qword a, qword b);
qword operator --(qword &a);
qword operator --(qword &a, int b);
qword operator -= (qword &a, qword b);
qword operator *(qword a, qword b);
qword operator *= (qword &a, qword b);
qword operator /(qword a, qword b);
qword operator /= (qword &a, qword b);
qword operator %(qword a, qword b);
qword operator %= (qword &a, qword b);
qword operator &(qword a, qword b);
qword operator &= (qword &a, qword b);
qword operator |(qword a, qword b);
qword operator |= (qword &a, qword b);
qword operator ^(qword a, qword b);
qword operator ^= (qword &a, qword b);
qword operator >>(qword a, byte b);
qword operator >>=(qword &a, byte b);
qword operator <<(qword a, byte b);
qword operator <<=(qword &a, byte b);
qword operator ~(qword a);
bool operator < (qword a, qword b);
bool operator <= (qword a, qword b);
bool operator > (qword a, qword b);
bool operator >= (qword a, qword b);
bool operator == (qword a, qword b);
bool operator != (qword a, qword b);
bool operator !(qword a);
qword operator -(qword a);

#endif /* __cplusplus */

#endif /* __QWORD_H__ */

