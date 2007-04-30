/* 
 *	HT Editor
 *	symmath.h
 *
 *	Copyright (C) 2001, 2002 Stefan Weyergraf
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

#ifndef __SYMMATH_H__
#define __SYMMATH_H__

#include "data.h"

enum c_op {
    c_invalid,
    c_eq,
    c_ne,
    c_gt,
    c_ge,
    c_lt,
    c_le
};

#define NUM_VALID_LOPS		8

enum l_op {
    l_invalid=-1,
    l_and=0,
    l_or,
    l_eq,
    l_ne,
    l_gt,
    l_ge,
    l_lt,
    l_le
};

enum n_op {
    n_null,
    n_not
};

#define NUM_VALID_BOPS		8

enum b_op {
    b_invalid=-1,
    b_mul=0,
    b_div,
    b_mod,
    b_add,
    b_sub,
    b_and,
    b_or,
    b_xor
};

enum u_op {
    u_null,
    u_minus,
    u_not
};

/* C bin operator precedence (for output) */

extern b_op *c_op_prec[];

/*
 *	CLASS sym_int_token
 */

class sym_int_token: public Object {
public:
/* new */
	virtual bool compare_eq(sym_int_token *t)=0;
	virtual bool evaluate(uint *i);
	virtual void simplify();
	virtual int nstrfy(char *buf, int n)=0;
};

/*
 *	CLASS sym_int
 */

class sym_int: public sym_int_token {
protected:
	void clear();
	bool simplify_reduce_const(b_op oa, sym_int_token *a, b_op ob, sym_int_token *b, b_op *res_op, sym_int_token **res_token);
	sym_int_token *simplify_reduce_destructive(b_op o, sym_int_token *x);
	bool simplify_reduce_inverse(b_op oa, sym_int_token *a, b_op ob, sym_int_token *b, sym_int_token **repl);
	bool simplify_reduce_neutral(b_op o, sym_int_token *x);
public:
	Container *tokens;

	sym_int();
	~sym_int();
/* overwritten */
	virtual bool compare_eq(sym_int_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(uint *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
	virtual void simplify();
/* new */
		   void b_operate(b_op bop, sym_int_token *t);
		   bool comp_eq(sym_int_token *a, sym_int_token *b);
		   void set(sym_int_token *t);
		   void u_operate(u_op uop);
		   void replace(sym_int_token *token, sym_int_token *by);
}; 

/*
 *	CLASS sym_int_symbol
 */

class sym_int_symbol: public sym_int_token {
public:
	char *name;

	sym_int_symbol(char *name);
	~sym_int_symbol();
/* overwritten */
	virtual bool compare_eq(sym_int_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(uint *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
};

/*
 *	CLASS sym_int_const
 */

class sym_int_const: public sym_int_token {
public:
	uint value;

	sym_int_const(uint value);
/* overwritten */
	virtual bool compare_eq(sym_int_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(uint *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
};

/*
 *	CLASS sym_bool_token
 */

class sym_bool_token: public Object {
public:
/* new */
	virtual bool compare_eq(sym_bool_token *t)=0;
	virtual bool evaluate(bool *i);
	virtual int nstrfy(char *buf, int n)=0;
	virtual void simplify();
};

/*
 *	CLASS sym_bool_symbol
 */

class sym_bool_symbol: public sym_bool_token {
public:
	char *name;

	sym_bool_symbol(char *name);
	~sym_bool_symbol();
/* overwritten */
	virtual bool compare_eq(sym_bool_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(bool *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
};

/*
 *	CLASS sym_bool_const
 */

class sym_bool_const: public sym_bool_token {
public:
	bool value;

	sym_bool_const(bool value);
/* new */
	virtual bool compare_eq(sym_bool_token *t);
	virtual bool evaluate(bool *i);
	virtual int nstrfy(char *buf, int n);
};

/*
 *	CLASS sym_bool_intcmp
 */

class sym_bool_intcmp: public sym_bool_token {
public:
	sym_int_token *int1;
	c_op cop;
	sym_int_token *int2;

	sym_bool_intcmp(sym_int_token *int1, c_op cop, sym_int_token *int2);
/* overwritten */	
	virtual bool compare_eq(sym_bool_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(bool *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
	virtual void simplify();
};

/*
 *	CLASS sym_bool
 */

class sym_bool: public sym_bool_token {
protected:
	Container *tokens;

	void clear();
public:
	sym_bool();
	~sym_bool();
/* overwritten */
	virtual bool compare_eq(sym_bool_token *t);
	virtual Object *clone() const;
	virtual bool evaluate(bool *i);
	virtual int nstrfy(char *buf, int n);
	virtual ObjectID getObjectID() const;
	virtual void simplify();
/* new */
		   void l_operate(l_op l, sym_bool_token *t);
		   void set(sym_bool_token *t);
		   void n_operate(n_op n);
}; 

#endif /* __SYMMATH_H__ */
