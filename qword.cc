/*
 *	HT Editor
 *	qword.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

uint64 int_to_qword(int i)
{
	uint64 res;
	res.lo = i;
	res.hi = 0;
	return res;
}

sint64 int_to_sint64(int i)
{
	sint64 res;
	res.lo = i;
	res.hi = (i<0) ? ((dword)-1) : 0;
	return res;
}

uint64 sint64_to_uint64(sint64 s)
{
	uint64 res;
     res.lo = s.lo;
     res.hi = s.hi;
     return res;
}

sint64 uint64_to_sint64(uint64 u)
{
	sint64 res;
     res.lo = u.lo;
     res.hi = u.hi;
     return res;
}

sint64 to_sint64(int i)
{
	sint64 res;
	res.lo = i;
	res.hi = (i<0) ? ((dword)-1) : 0;
	return res;
}

uint64 to_uint64(UINT i)
{
	uint64 res;
	res.lo = i;
	res.hi = 0;
	return res;
}

sint64 to_sint64(const uint64 &u)
{
	sint64 res;
     res.lo = u.lo;
     res.hi = u.hi;
     return res;
}

uint64 to_uint64(const sint64 &s)
{
	uint64 res;
     res.lo = s.lo;
     res.hi = s.hi;
     return res;
}

int qword_cmp(uint64 a, uint64 b)
{
	if (a.hi > b.hi) return 1;
	if (a.hi < b.hi) return -1;
	if (a.lo > b.lo) return 1;
	if (a.lo < b.lo) return -1;
	return 0;
}

uint64 qword_add(uint64 a, uint64 b)
{
	uint64 res;
	res.lo = a.lo + b.lo;
	res.hi = a.hi + b.hi;
	if (a.lo + b.lo < a.lo) res.hi++;
	return res;
}

uint64 qword_sub(uint64 a, uint64 b)
{
	uint64 res;
	res.lo = a.lo - b.lo;
	res.hi = a.hi - b.hi;
	if (b.lo > a.lo) res.hi--;
	return res;
}

static uint64 uint64_and(const uint64 &a, const uint64 &b)
{
	uint64 res;
	res.lo = a.lo & b.lo;
	res.hi = a.hi & b.hi;
	return res;
}

static uint64 uint64_or(const uint64 &a, const uint64 &b)
{
	uint64 res;
	res.lo = a.lo | b.lo;
	res.hi = a.hi | b.hi;
	return res;
}

static uint64 uint64_xor(const uint64 &a, const uint64 &b)
{
	uint64 res;
	res.lo = a.lo ^ b.lo;
	res.hi = a.hi ^ b.hi;
	return res;
}

static uint64 uint64_shl(const uint64 &A, int b)
{
	uint64 res;
     uint64 a = A;
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

static uint64 uint64_shr(const uint64 &A, int b)
{
	uint64 res;
     uint64 a = A;
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

uint64 qword_mul(uint64 a, uint64 b)
{
	uint64 x = a, y = b, z = to_uint64(0);
	while (y != to_uint64(0)) {
		if (y.lo & 1) z += x;
		x = uint64_shl(x, 1);
		y = uint64_shr(y, 1);
	}
	return z;
}

static uint64 uint64_div(const uint64 &a, const uint64 &b)
{
 	// FIXME: should ignore sign
	uint64 null = to_uint64(0);
	uint64 x = a, y = b, q = null, t = to_uint64(1);
	bool ycarry = false;
	bool carry = false;
	while ((y < x) && (!ycarry)) {
		uint64 ynew = uint64_shl(y, 1);
		if (ynew < y) ycarry = true;
		y = ynew;
		t = uint64_shl(t, 1);
	}
	if (t == null) carry = true;
	while ((t != null) || carry) {
		if ((x >= y) && !ycarry) {
			x = x - y;
			q = q + t;
		}
		if (ycarry) {
			y = uint64_shr(y, 1);
			y.hi |= 0x80000000;
			ycarry = false;
		} else {
			y = uint64_shr(y, 1);
		}
		if (carry) {
			t.lo = 0;
			t.hi = 0x80000000;
			carry = false;
		} else {
			t = uint64_shr(t, 1);
		}
	}
	return q;
}

/* operators */

uint64 operator + (const uint64 &a, const uint64 &b)
{
	return qword_add(a, b);
}

// prefix
uint64& operator ++(uint64 &a)
{
	return a += to_uint64(1);
}

// postfix
uint64 operator ++(uint64 &a, int b)
{
	uint64 t = a;
	++a;
	return t;
}

uint64& operator += (uint64 &a, const uint64 &b)
{
	a = a + b;
	return a;
}

uint64 operator - (const uint64 &a, const uint64 &b)
{
	return qword_sub(a, b);
}

// prefix
uint64& operator --(uint64 &a)
{
	a -= to_uint64(1);
	return a;
}

// postfix
uint64 operator --(uint64 &a, int b)
{
	uint64 t = a;
	--a;
	return t;
}

uint64& operator -= (uint64 &a, const uint64 &b)
{
	a = a - b;
	return a;
}

uint64 operator *(const uint64 &a, const uint64 &b)
{
	return qword_mul(a, b);
}

uint64& operator *= (uint64 &a, const uint64 &b)
{
	a = a * b;
	return a;
}

uint64 operator /(const uint64 &a, const uint64 &b)
{
	return uint64_div(a, b);
}

uint64& operator /= (uint64 &a, const uint64 &b)
{
	a = a / b;
	return a;
}

uint64 operator %(const uint64 &a, const uint64 &b)
{
	// FIXME: PLEASE FIX ME
	uint64 d = a / b;
	uint64 db = d * b;
	uint64 adb = a-db;
	return adb;
}

uint64& operator %= (uint64 &a, const uint64 &b)
{
	a = a % b;
	return a;
}

uint64 operator &(const uint64 &a, const uint64 &b)
{
	return uint64_and(a, b);
}

uint64& operator &= (uint64 &a, uint64 b)
{
	a = a & b;
	return a;
}

uint64 operator |(const uint64 &a, const uint64 &b)
{
	return uint64_or(a, b);
}

uint64& operator |= (uint64 &a, const uint64 &b)
{
	a = a | b;
	return a;
}

uint64 operator ^(const uint64 &a, const uint64 &b)
{
	return uint64_xor(a, b);
}

uint64& operator ^= (uint64 &a, const uint64 &b)
{
	a = a ^ b;
	return a;
}

uint64 operator >> (const uint64 &a, byte b)
{
	return uint64_shr(a, b);
}

uint64& operator >>= (uint64 &a, byte b)
{
	return ((a = a >> b));
}

uint64 operator << (const uint64 &a, byte b)
{
	return uint64_shl(a, b);
}

uint64& operator <<= (uint64 &a, byte b)
{
	return ((a = a << b));
}

uint64 operator ~(const uint64 &a)
{
 	uint64 r;
	r.lo = ~a.lo;
	r.hi = ~a.hi;
	return r;
}

uint64 operator -(const uint64 &a)
{
	uint64 r = a;
	r.lo = ~r.lo;
	r.hi = ~r.hi;
	return r+to_uint64(1);
}

bool operator < (const uint64 &a, const uint64 &b) { return qword_cmp(a, b) < 0; }

bool operator <= (const uint64 &a, const uint64 &b) { return qword_cmp(a, b) <= 0; }

bool operator > (const uint64 &a, const uint64 &b) { return qword_cmp(a, b) > 0; }

bool operator >= (const uint64 &a, const uint64 &b) { return qword_cmp(a, b) >= 0; }

bool operator == (const uint64 &a, const uint64 &b) { return qword_cmp(a, b) == 0; }

bool operator != (const uint64 &a, const uint64 &b) { return qword_cmp(a, b) != 0; }

bool operator !(const uint64 &a) { return qword_cmp(a, to_uint64(0)) == 0; }

/*
 *	signed int 64
 */

int sint64_cmp(sint64 a, sint64 b)
{
	if ((int)a.hi > (int)b.hi) return 1;
	if ((int)a.hi < (int)b.hi) return -1;
     // a.hi == b.hi => both numbers have the same sign
	if (a.lo > b.lo) return 1;
	if (a.lo < b.lo) return -1;
	return 0;
}

sint64 sint64_add(sint64 a, sint64 b)
{
	sint64 res;
	res.lo = a.lo + b.lo;
	res.hi = a.hi + b.hi;
	if (a.lo + b.lo < a.lo) res.hi++;
	return res;
}

sint64 sint64_sub(sint64 a, sint64 b)
{
	sint64 res;
	res.lo = a.lo - b.lo;
	res.hi = a.hi - b.hi;
	if (b.lo > a.lo) res.hi--;
	return res;
}

static sint64 sint64_and(const sint64 &a, const sint64 &b)
{
	sint64 res;
	res.lo = a.lo & b.lo;
	res.hi = a.hi & b.hi;
	return res;
}

static sint64 sint64_or(const sint64 &a, const sint64 &b)
{
	sint64 res;
	res.lo = a.lo | b.lo;
	res.hi = a.hi | b.hi;
	return res;
}

static sint64 sint64_xor(const sint64 &a, const sint64 &b)
{
	sint64 res;
	res.lo = a.lo ^ b.lo;
	res.hi = a.hi ^ b.hi;
	return res;
}

static sint64 sint64_shl(const sint64 &A, int b)
{
	sint64 res;
     sint64 a = A;
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

static sint64 sint64_shr(const sint64 &A, int b)
{
	// FIXME: should care about sign
	sint64 res;
     sint64 a = A;
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

sint64 sint64_mul(sint64 a, sint64 b)
{
	sint64 x = a, y = b, z = to_sint64(0);
	while (y != to_sint64(0)) {
		if (y.lo & 1) z += x;
		x = sint64_shl(x, 1);
		y = sint64_shr(y, 1);
	}
	return z;
}

static sint64 sint64_div(const sint64 &a, const sint64 &b)
{
	sint64 null = to_sint64(0);
	sint64 x = a, y = b, q = null, t = to_sint64(1);
	bool ycarry = false;
	bool carry = false;
	while ((y < x) && (!ycarry)) {
		sint64 ynew = sint64_shl(y, 1);
		if (ynew < y) ycarry = true;
		y = ynew;
		t = sint64_shl(t, 1);
	}
	if (t == null) carry = true;
	while ((t != null) || carry) {
		if ((x >= y) && !ycarry) {
			x = x - y;
			q = q + t;
		}
		if (ycarry) {
			y = sint64_shr(y, 1);
			y.hi |= 0x80000000;
			ycarry = false;
		} else {
			y = sint64_shr(y, 1);
		}
		if (carry) {
			t.lo = 0;
			t.hi = 0x80000000;
			carry = false;
		} else {
			t = sint64_shr(t, 1);
		}
	}
	return q;
}
sint64 operator + (const sint64 &a, const sint64 &b)
{
	return sint64_add(a, b);
}

// prefix
sint64& operator ++(sint64 &a)
{
	return a += to_sint64(1);
}

// postfix
sint64 operator ++(sint64 &a, int b)
{
	sint64 t = a;
	++a;
	return t;
}

sint64& operator += (sint64 &a, const sint64 &b)
{
	a = a + b;
	return a;
}

sint64 operator - (const sint64 &a, const sint64 &b)
{
	return sint64_sub(a, b);
}

// prefix
sint64& operator --(sint64 &a)
{
	a -= to_sint64(1);
	return a;
}

// postfix
sint64 operator --(sint64 &a, int b)
{
	sint64 t = a;
	--a;
	return t;
}

sint64& operator -= (sint64 &a, const sint64 &b)
{
	a = a - b;
	return a;
}

sint64 operator *(const sint64 &a, const sint64 &b)
{
	return sint64_mul(a, b);
}

sint64& operator *= (sint64 &a, const sint64 &b)
{
	a = a * b;
	return a;
}

sint64 operator /(const sint64 &a, const sint64 &b)
{
	return sint64_div(a, b);
}

sint64& operator /= (sint64 &a, const sint64 &b)
{
	a = a / b;
	return a;
}

sint64 operator %(const sint64 &a, const sint64 &b)
{
	// FIXME: PLEASE FIX ME
	sint64 d = a / b;
	sint64 db = d * b;
	sint64 adb = a-db;
	return adb;
}

sint64& operator %= (sint64 &a, const sint64 &b)
{
	a = a % b;
	return a;
}

sint64 operator &(const sint64 &a, const sint64 &b)
{
	return sint64_and(a, b);
}

sint64& operator &= (sint64 &a, sint64 b)
{
	a = a & b;
	return a;
}

sint64 operator |(const sint64 &a, const sint64 &b)
{
	return sint64_or(a, b);
}

sint64& operator |= (sint64 &a, const sint64 &b)
{
	a = a | b;
	return a;
}

sint64 operator ^(const sint64 &a, const sint64 &b)
{
	return sint64_xor(a, b);
}

sint64& operator ^= (sint64 &a, const sint64 &b)
{
	a = a ^ b;
	return a;
}

sint64 operator >> (const sint64 &a, byte b)
{
	return sint64_shr(a, b);
}

sint64& operator >>= (sint64 &a, byte b)
{
	return ((a = a >> b));
}

sint64 operator << (const sint64 &a, byte b)
{
	return sint64_shl(a, b);
}

sint64& operator <<= (sint64 &a, byte b)
{
	return ((a = a << b));
}

sint64 operator ~(const sint64 &a)
{
 	sint64 r;
	r.lo = ~a.lo;
	r.hi = ~a.hi;
	return r;
}

sint64 operator -(const sint64 &a)
{
	sint64 r = a;
	r.lo = ~r.lo;
	r.hi = ~r.hi;
	return r+to_sint64(1);
}

bool operator < (const sint64 &a, const sint64 &b) { return sint64_cmp(a, b) < 0; }

bool operator <= (const sint64 &a, const sint64 &b) { return sint64_cmp(a, b) <= 0; }

bool operator > (const sint64 &a, const sint64 &b) { return sint64_cmp(a, b) > 0; }

bool operator >= (const sint64 &a, const sint64 &b) { return sint64_cmp(a, b) >= 0; }

bool operator == (const sint64 &a, const sint64 &b) { return sint64_cmp(a, b) == 0; }

bool operator != (const sint64 &a, const sint64 &b) { return sint64_cmp(a, b) != 0; }

bool operator !(const sint64 &a) { return sint64_cmp(a, to_sint64(0)) == 0; }

