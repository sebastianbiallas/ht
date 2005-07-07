/*
 *	HT Editor
 *	qword.h
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
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

#ifndef __QWORD_H__
#define __QWORD_H__

#include "global.h"		// for types uint32 and uint

typedef struct {
	uint32 lo;
	uint32 hi;
} uint64;

#define uint64 uint64

typedef struct {
	uint32 lo;
	uint32 hi;
} sint64;

// FIXME: dont work for >32 bit systems
#define QWORD_SET_HI(q, v) (((q).hi) = (v))
#define QWORD_SET_LO(q, v) (((q).lo) = (v))
#define QWORD_GET_HI(q) ((q).hi)
#define QWORD_GET_LO(q) ((q).lo)

#define QWORD_GET_INT(q) ((q).lo)
#define QWORD_GET_FLOAT(q) ( (((float)QWORD_GET_HI(q))*4294967296.0) + ((float)QWORD_GET_LO(q)))

#ifdef __cplusplus
extern "C" {
#endif
int qword_cmp(uint64 a, uint64 b);
uint64 int_to_qword(int i);
uint64 sint64_to_uint64(sint64 s);
uint64 qword_add(uint64 a, uint64 b);
uint64 qword_sub(uint64 a, uint64 b);
uint64 qword_mul(uint64 a, uint64 b);

int sint64_cmp(sint64 a, sint64 b);
sint64 int_to_sint64(int i);
sint64 uint64_to_sint64(uint64 u);
sint64 sint64_add(sint64 a, sint64 b);
sint64 sint64_sub(sint64 a, sint64 b);
sint64 sint64_mul(sint64 a, sint64 b);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
sint64 to_sint64(int i);
uint64 to_uint64(uint i);
sint64 to_sint64(const uint64 &u);
uint64 to_uint64(const sint64 &s);
#define to_qword to_uint64

uint64 operator +(const uint64 &a, const uint64 &b);
uint64& operator ++(uint64 &a);
uint64 operator ++(const uint64 &a, int b);
uint64& operator += (uint64 &a, const uint64 &b);
uint64 operator - (const uint64 &a, const uint64 &b);
uint64& operator --(uint64 &a);
uint64 operator --(const uint64 &a, int b);
uint64& operator -= (uint64 &a, const uint64 &b);
uint64 operator *(const uint64 &a, const uint64 &b);
uint64& operator *= (uint64 &a, const uint64 &b);
uint64 operator /(const uint64 &a, const uint64 &b);
uint64& operator /= (uint64 &a, const uint64 &b);
uint64 operator %(const uint64 &a, const uint64 &b);
uint64& operator %= (uint64 &a, const uint64 &b);
uint64 operator &(const uint64 &a, const uint64 &b);
uint64& operator &= (uint64 &a, const uint64 &b);
uint64 operator |(const uint64 &a, const uint64 &b);
uint64& operator |= (uint64 &a, const uint64 &b);
uint64 operator ^(const uint64 &a, const uint64 &b);
uint64& operator ^= (uint64 &a, const uint64 &b);
uint64 operator >>(const uint64 &a, byte b);
uint64& operator >>=(uint64 &a, byte b);
uint64 operator <<(const uint64 &a, byte b);
uint64& operator <<=(uint64 &a, byte b);
uint64 operator ~(const uint64 &a);
uint64 operator -(const uint64 &a);
bool operator !(const uint64 &a);
bool operator < (const uint64 &a, const uint64 &b);
bool operator <= (const uint64 &a, const uint64 &b);
bool operator > (const uint64 &a, const uint64 &b);
bool operator >= (const uint64 &a, const uint64 &b);
bool operator == (const uint64 &a, const uint64 &b);
bool operator != (const uint64 &a, const uint64 &b);

sint64 operator +(const sint64 &a, const sint64 &b);
sint64& operator ++(sint64 &a);
sint64 operator ++(const sint64 &a, int b);
sint64& operator += (sint64 &a, const sint64 &b);
sint64 operator - (const sint64 &a, const sint64 &b);
sint64& operator --(sint64 &a);
sint64 operator --(const sint64 &a, int b);
sint64& operator -= (sint64 &a, const sint64 &b);
sint64 operator *(const sint64 &a, const sint64 &b);
sint64& operator *= (sint64 &a, const sint64 &b);
sint64 operator /(const sint64 &a, const sint64 &b);
sint64& operator /= (sint64 &a, const sint64 &b);
sint64 operator %(const sint64 &a, const sint64 &b);
sint64& operator %= (sint64 &a, const sint64 &b);
sint64 operator &(const sint64 &a, const sint64 &b);
sint64& operator &= (sint64 &a, const sint64 &b);
sint64 operator |(const sint64 &a, const sint64 &b);
sint64& operator |= (sint64 &a, const sint64 &b);
sint64 operator ^(const sint64 &a, const sint64 &b);
sint64& operator ^= (sint64 &a, const sint64 &b);
sint64 operator >>(const sint64 &a, byte b);
sint64& operator >>=(sint64 &a, byte b);
sint64 operator <<(const sint64 &a, byte b);
sint64& operator <<=(sint64 &a, byte b);
sint64 operator ~(const sint64 &a);
sint64 operator -(const sint64 &a);
bool operator !(const sint64 &a);
bool operator < (const sint64 &a, const sint64 &b);
bool operator <= (const sint64 &a, const sint64 &b);
bool operator > (const sint64 &a, const sint64 &b);
bool operator >= (const sint64 &a, const sint64 &b);
bool operator == (const sint64 &a, const sint64 &b);
bool operator != (const sint64 &a, const sint64 &b);

#endif /* __cplusplus */

#endif /* __QWORD_H__ */

