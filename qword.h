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

struct qword {
	dword hi;
     dword lo;
};

#define QWORD_HI(q) ((q).hi)
#define QWORD_LO(q) ((q).lo)

int qword_cmp(qword a, qword b);
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

#endif /* __QWORD_H__ */

