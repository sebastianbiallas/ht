/*
 *	HT Editor
 *	qword.cc
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

#include "qword.h"

/*qword::qword()
{
}

qword::qword(int i)
{
	lo = i;
	hi = (i<0) ? ((dword)-1) : 0;
}

qword::qword(UINT i)
{
	lo = i;
	hi = 0;
}*/

qword int_to_qword(int i)
{
	qword res;
	res.lo = i;
	res.hi = (i<0) ? ((dword)-1) : 0;
	return res;
}

qword to_qword(int i)
{
	qword res;
	res.lo = i;
	res.hi = (i<0) ? ((dword)-1) : 0;
	return res;
}

qword to_qword(UINT i)
{
	qword res;
	res.lo = i;
	res.hi = 0;
	return res;
}

int qword_cmp(qword a, qword b)
{
	if (a.hi > b.hi) return 1;
	if (a.hi < b.hi) return -1;
	if (a.lo > b.lo) return 1;
	if (a.lo < b.lo) return -1;
	return 0;
}

static qword qword_add(qword a, qword b)
{
	qword res;
	res.lo = a.lo + b.lo;
	res.hi = a.hi + b.hi;
	if (a.lo + b.lo < a.lo) res.hi++;
	return res;
}

static qword qword_sub(qword a, qword b)
{
	qword res;
	res.lo = a.lo - b.lo;
	res.hi = a.hi - b.hi;
	if (b.lo > a.lo) res.hi--;
	return res;
}

static qword qword_and(qword a, qword b)
{
	qword res;
	res.lo = a.lo & b.lo;
	res.hi = a.hi & b.hi;
	return res;
}

static qword qword_or(qword a, qword b)
{
	qword res;
	res.lo = a.lo | b.lo;
	res.hi = a.hi | b.hi;
	return res;
}

static qword qword_xor(qword a, qword b)
{
	qword res;
	res.lo = a.lo ^ b.lo;
	res.hi = a.hi ^ b.hi;
	return res;
}

qword qword_shl(qword a, int b)
{
	qword res;
	if (b > 32) {
		if (b < 64) {
			a.hi = a.lo;
			a.lo = 0;
			b -= 32;
		} else {
			res.lo = res.hi = 0;
			return res;
		}
	}
	res.hi = a.hi << b;
	res.lo = a.lo << b;
	res.hi |= a.lo >> (32-b);
	return res;
}

qword qword_shr(qword a, int b)
{
	qword res;
	if (b > 32) {
		if (b < 64) {
			a.lo = a.hi;
			a.hi = 0;
			b -= 32;
		} else {
			res.lo = res.hi = 0;
			return res;
		}
	}
	res.hi = a.hi >> b;
	res.lo = a.lo >> b;
	res.lo |= a.hi << (32-b);
	return res;
}

qword qword_mul(qword a, qword b)
{
	qword x = a, y = b, z = to_qword(0);
	while (y != to_qword(0)) {
		if (y.lo & 1) z += x;
		x = qword_shl(x, 1);
		y = qword_shr(y, 1);
	}
	return z;
}

qword qword_div(qword a, qword b)
{
	qword null = to_qword(0);
	qword x = a, y = b, q = null, t = to_qword(1);
	bool ycarry = false;
	bool carry = false;
	while ((y < x) && (!ycarry)) {
		qword ynew = qword_shl(y, 1);
		if (ynew < y) ycarry = true;
		y = ynew;
		t = qword_shl(t, 1);
	}
	if (t == null) carry = true;
	while ((t != null) || carry) {
		if ((x >= y) && !ycarry) {
			x = x - y;
			q = q + t;
		}
		if (ycarry) {
			y = qword_shr(y, 1);
			y.hi |= 0x80000000;
			ycarry = false;
		} else {
			y = qword_shr(y, 1);
		}
		if (carry) {
			t.lo = 0;
			t.hi = 0x80000000;
			carry = false;
		} else {
			t = qword_shr(t, 1);
		}
	}
	return q;
}

/* operators */

qword operator + (qword a, qword b)
{
	return qword_add(a, b);
}

// prefix
qword operator ++(qword &a)
{
	return a += to_qword(1);
}

// postfix
qword operator ++(qword &a, int b)
{
	qword t = a;
	++a;
	return t;
}

qword operator += (qword &a, qword b)
{
	a = a + b;
	return a;
}

qword operator - (qword a, qword b)
{
	return qword_sub(a, b);
}

// prefix
qword operator --(qword &a)
{
	a -= to_qword(1);
	return a;
}

// postfix
qword operator --(qword &a, int b)
{
	qword t = a;
	--a;
	return t;
}

qword operator -= (qword &a, qword b)
{
	a = a - b;
	return a;
}

qword operator *(qword a, qword b)
{
	return qword_mul(a, b);
}

qword operator *= (qword &a, qword b)
{
	a = a * b;
	return a;
}

qword operator /(qword a, qword b)
{
	return qword_div(a, b);
}

qword operator /= (qword &a, qword b)
{
	a = a / b;
	return a;
}

qword operator %(qword a, qword b)
{
	// FIXME: PLEASE FIX ME
	qword d = a / b;
	qword db = d * b;
	qword adb = a-db;
	return adb;
}

qword operator %= (qword &a, qword b)
{
	a = a % b;
	return a;
}

qword operator &(qword a, qword b)
{
	return qword_and(a, b);
}

qword operator &= (qword &a, qword b)
{
	a = a & b;
	return a;
}

qword operator |(qword a, qword b)
{
	return qword_or(a, b);
}

qword operator |= (qword &a, qword b)
{
	a = a | b;
	return a;
}

qword operator ^(qword a, qword b)
{
	return qword_xor(a, b);
}

qword operator ^= (qword &a, qword b)
{
	a = a ^ b;
	return a;
}

qword operator >> (qword a, byte b)
{
	return qword_shr(a, b);
}

qword operator >>= (qword &a, byte b)
{
	return ((a = a >> b));
}

qword operator << (qword a, byte b)
{
	return qword_shl(a, b);
}

qword operator <<= (qword &a, byte b)
{
	return ((a = a << b));
}

qword operator ~(qword a)
{
	a.lo = ~a.lo;
	a.hi = ~a.hi;
	return a;
}

bool operator < (qword a, qword b) { return qword_cmp(a, b) < 0; }

bool operator <= (qword a, qword b) { return qword_cmp(a, b) <= 0; }

bool operator > (qword a, qword b) { return qword_cmp(a, b) > 0; }

bool operator >= (qword a, qword b) { return qword_cmp(a, b) >= 0; }

bool operator == (qword a, qword b) { return qword_cmp(a, b) == 0; }

bool operator != (qword a, qword b) { return qword_cmp(a, b) != 0; }

