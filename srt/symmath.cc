/*
 *	HT Editor
 *	symmath.cc
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

#include <string.h>

#include "tools.h"
#include "symmath.h"

#define ATOM_SYM_INT		MAGIC32("SMA\x00")
#define ATOM_SYM_INT_SYMBOL	MAGIC32("SMA\x01")
#define ATOM_SYM_INT_CONST	MAGIC32("SMA\x02")

#define ATOM_SYM_BOOL		MAGIC32("SMA\x10")
#define ATOM_SYM_BOOL_SYMBOL	MAGIC32("SMA\x11")
#define ATOM_SYM_BOOL_INTCMP	MAGIC32("SMA\x12")

/* C operator precedence (for output) */

b_op copp_mul[] = {
	b_mul,
	b_div,
	b_mod,
	b_invalid
};

b_op copp_add[] = {
	b_add,
	b_sub,
	b_invalid
};

b_op copp_bin[] = {
	b_and,
	b_or,
	b_xor,
	b_invalid
};

b_op *c_op_prec[] = {
	copp_bin,			/* lowest */
	copp_add,
	copp_mul,			/* biggest */
	NULL
};

uint get_op_prec(b_op bop, b_op **op_prec)
{
	uint k = 0;
	while (*op_prec) {
		b_op *l = *op_prec;
		while (*l != b_invalid) {
			if (*l == bop) return k;
			l++;
		}
		op_prec++;
		k++;
	}
	return 0;
}

/*
 *	CLASS sym_int_token
 */

bool sym_int_token::evaluate(uint *i)
{
	return false;
}

void sym_int_token::simplify()
{
}

/*
 *	CLASS sym_int_symbol
 */

sym_int_symbol::sym_int_symbol(char *n)
{
	name = strdup(n);
}

sym_int_symbol::~sym_int_symbol()
{
	free(name);
}

bool sym_int_symbol::compare_eq(sym_int_token *t)
{
	sym_int_symbol *s = (sym_int_symbol*)t;
	return (strcmp(name, s->name) == 0);
}

Object *sym_int_symbol::clone() const
{
	sym_int_symbol *p = new sym_int_symbol(name);
	return p;
}

bool sym_int_symbol::evaluate(uint *i)
{
	return false;
}

int sym_int_symbol::nstrfy(char *buf, int n)
{
	int q = strlen(name)+1;
	if (q > n) q = n;
	memmove(buf, name, q);
	return q-1;
}

ObjectID sym_int_symbol::getObjectID() const
{
	return ATOM_SYM_INT_SYMBOL;
}

/*
 *	CLASS sym_int_const
 */

sym_int_const::sym_int_const(uint v)
{
	value = v;
}
 
bool sym_int_const::compare_eq(sym_int_token *t)
{
	sym_int_const *s = (sym_int_const*)t;
	return (value == s->value);
}

Object *sym_int_const::clone() const
{
	return new sym_int_const(value);
}

bool sym_int_const::evaluate(uint *i)
{
	*i = value;
	return true;	
}
 
int sym_int_const::nstrfy(char *buf, int n)
{
/* FIXME: use n */
	if (value < 16) return sprintf(buf, "%d", value);
	return sprintf(buf, "0x%x", value);
}
 
ObjectID sym_int_const::getObjectID() const
{
	return ATOM_SYM_INT_CONST;
}

/*
 *	CLASS sym_int_token_rec
 */
 
class sym_int_token_rec: public Object {
public:
	u_op uop;
	b_op bop;
	sym_int_token *token;

	sym_int_token_rec(u_op u, b_op b, sym_int_token *t)
	{
		uop = u;
		bop = b;
		token = t;		
	}

	~sym_int_token_rec()
	{
		token->done();
		delete token;
	}

	Object *clone() const
	{
		return new sym_int_token_rec(uop, bop, (sym_int_token*)token->clone());
	}
}; 

/*
 *	CLASS sym_int
 */

static b_op **output_op_prec = c_op_prec;

struct op_int_prop {
	bool has_prop;
	uint value;
};

struct op_int_int_prop {
	bool has_prop;
	uint value1;
	uint value2;
};

op_int_prop op_neutrals[NUM_VALID_BOPS] = {
//    b_mul=0,
	{true, 1},
//    b_div,
	{true, 1},
//    b_mod,
	{false},
//    b_add,
	{true, 0},
//    b_sub,
	{true, 0},
//    b_and,
	{true, 0xffffffff},
//    b_or,
	{true, 0},
//    b_xor
	{true, 0}
};

bool op_commutative[NUM_VALID_BOPS] = {
//    b_mul=0,
	true,
//    b_div,
	false,
//    b_mod,
	false,
//    b_add,
	true,
//    b_sub,
	false,
//    b_and,
	true,
//    b_or,
	true,
//    b_xor
	true
};

op_int_int_prop op_destructive[NUM_VALID_BOPS] = {
//    b_mul=0,
	{true, 0, 0},
//    b_div,
	{false},
//    b_mod,
	{true, 1, 0},
//    b_add,
	{false},
//    b_sub,
	{false},
//    b_and,
	{true, 0, 0},
//    b_or,
	{true, 0xffffffff, 0xffffffff},
//    b_xor
	{false}
};

sym_int::sym_int()
{
	tokens = new Array(true);
}

sym_int::~sym_int()
{
	delete tokens;
}

void sym_int::b_operate(b_op bop, sym_int_token *t)
{
	if (t->getObjectID() == ATOM_SYM_INT) {
		sym_int *i = (sym_int*)t;
		if (i->tokens->count() == 1) {
			sym_int_token_rec *r = (sym_int_token_rec*)i->tokens->get(0);
			b_operate(bop, r->token);
			return;
		}
	}
	tokens->insert(new sym_int_token_rec(u_null, bop, t));
}

void sym_int::clear()
{
	tokens->delAll();
}

bool sym_int::compare_eq(sym_int_token *t)
{
// FIXME: implement me !
	return false;
}

bool sym_int::comp_eq(sym_int_token *a, sym_int_token *b)
{
	if (a->getObjectID() == b->getObjectID()) {
		return a->compare_eq(b);
	}
	return false;
}

Object *sym_int::clone() const
{
	sym_int *p = new sym_int();
	p->tokens = (Container*)tokens->clone();
	return p;
}

bool sym_int::evaluate(uint *i)
{
	int c = tokens->count();
	uint l;
	for (int j = 0; j < c; j++) {
		uint k;
		sym_int_token_rec *r = (sym_int_token_rec*)(*tokens)[j];
		if (!r->token->evaluate(&k)) return false;
		if (j == 0) l = k;
		switch (r->uop) {
			case u_null: break;
			case u_minus: k = -k; break;
			case u_not: k = ~k; break;
		}
		switch (r->bop) {
			case b_invalid: break;
			case b_mul: l *= k; break;
			case b_div: if (k) l /= k; else return false; break;
			case b_mod: if (k) l %= k; else return false; break;
			case b_add: l += k; break;
			case b_sub: l -= k; break;
			case b_and: l &= k; break;
			case b_or:  l |= k; break;
			case b_xor: l ^= k; break;
		}
	}
	*i = l;
	return true;
}

int sym_int::nstrfy(char *buf, int n)
{
	b_op lbop;
	int l = 0;
	int c = tokens->count();
	lbop = b_invalid;
	uint para_count = 0;
	for (int i = 0; i < c; i++) {
		sym_int_token_rec *r = (sym_int_token_rec*)(*tokens)[i];
		bool para = ((lbop != b_invalid) && (r->bop != b_invalid) &&
			(get_op_prec(r->bop, output_op_prec) > get_op_prec(lbop,
			output_op_prec)));
		if (para) para_count++;
		lbop = r->bop;
	}
	for (uint i = 0; i < para_count; i++) buf[l++] = '(';
	lbop = b_invalid;
	for (int i = 0; i < c; i++) {
		sym_int_token_rec *r = (sym_int_token_rec*)(*tokens)[i];
		bool para = ((lbop != b_invalid) && (r->bop != b_invalid) &&
			(get_op_prec(r->bop, output_op_prec) > get_op_prec(lbop,
			output_op_prec)));

		if (para) buf[l++] = ')';

		switch (r->uop) {
			case u_null: break;
			case u_minus: buf[l++] = '-'; break;
			case u_not: buf[l++] = '~'; break;
		}

		switch (r->bop) {
			case b_invalid: break;
			case b_mul: buf[l++] = '*'; break;
			case b_div: buf[l++] = '/'; break;
			case b_mod: buf[l++] = '%'; break;
			case b_add: buf[l++] = '+'; break;
			case b_sub: buf[l++] = '-'; break;
			case b_and: buf[l++] = '&'; break;
			case b_or:  buf[l++] = '|'; break;
			case b_xor: buf[l++] = '^'; break;
		}
//          buf[l++]='{';
		l += r->token->nstrfy(buf+l, n-l);
//          buf[l++]='}';

		lbop = r->bop;
	}
	buf[l] = 0;
	return l;
}

ObjectID sym_int::getObjectID() const
{
	return ATOM_SYM_INT;
}

void sym_int::replace(sym_int_token *token, sym_int_token *by)
{
	int c = tokens->count();
	for (int i = 0; i < c; i++) {
		sym_int_token_rec *r = (sym_int_token_rec*)(*tokens)[i];
		if (r->token->getObjectID() == getObjectID()) {
			((sym_int*)r->token)->replace(token, by);
		} else if (comp_eq(r->token, token)) {
			r->token->done();
			delete r->token;
			r->token = (sym_int_token*)by->clone();
		}
	}		
}

void sym_int::set(sym_int_token *t)
{
	clear();
	if (t->getObjectID() == ATOM_SYM_INT) {
		sym_int *i = (sym_int*)t;
		int c = i->tokens->count();
		for (int k=0; k<c; k++) {
			sym_int_token_rec *r = (sym_int_token_rec*)(*i->tokens)[k];
			tokens->insert(new sym_int_token_rec(r->uop, r->bop, (sym_int_token*)r->token->clone()));
		}
		t->done();
		delete t;
		return;
	}
	tokens->insert(new sym_int_token_rec(u_null, b_invalid, t));
}

void sym_int::simplify()
{
	uint c = tokens->count();
	uint d;

	do {
	d = c;
/* I. CONSTANTS */

/* step I.1: "c$d..." (front) */
	while (c>=2) {
		sym_int_token_rec *a = (sym_int_token_rec*)tokens->get(0);
		sym_int_token_rec *b = (sym_int_token_rec*)(*tokens)[1];
		b_op rop;
		sym_int_token *rtoken;
		if (!simplify_reduce_const(b_invalid, a->token, b->bop, b->token, &rop, &rtoken)) break;
		tokens->del(0);
		tokens->del(0);
// FIXME: wrongwrong
		tokens->insert(new sym_int_token_rec(u_null, b_invalid, rtoken));
		c--;
	}

/* step I.2: reduce all "...$c%d..." where allowed
   (c,d constant, $,% operators) */
	for (uint i=1; i<c-1; i++) {
		do {
			if (i >= c-1) break;
			sym_int_token_rec *a = (sym_int_token_rec*)(*tokens)[i];
			sym_int_token_rec *b = (sym_int_token_rec*)(*tokens)[i+1];
			b_op rop;
			sym_int_token *rtoken;
			if (!simplify_reduce_const(a->bop, a->token, b->bop, b->token, &rop, &rtoken)) break;
			tokens->del(tokens->findByIdx(i));
			tokens->del(tokens->findByIdx(i));
			tokens->insert_before(new sym_int_token_rec(u_null, rop, rtoken), i);
			c--;
		} while (c>=2);
	}

/* II. NEUTRAL OPERATIONS
   reduce all "N$x..." (front, commutative) and "...x$N..." (everywhere) to "x"
   ($ operator, N neutral element for $) */

/* step II.1: "N$x..." (front, commutative) */
	while (c>=2) {
		sym_int_token_rec *a = (sym_int_token_rec*)tokens->get(0);
		sym_int_token_rec *b = (sym_int_token_rec*)tokens->get(1);
		if (!op_commutative[b->bop]) break;
		if (!simplify_reduce_neutral(b->bop, a->token)) break;
		tokens->del(0);	// delete N/a
		tokens->remove(0);	// remove x/b
		tokens->prepend(new sym_int_token_rec(u_null, b_invalid, b->token));
		c--;
	}

/* step II.2: "...x$N..." (everywhere) */
	for (uint i=0; i<c-1; i++) {
		do {
			if (i >= c-1) break;
			sym_int_token_rec *a = (sym_int_token_rec*)tokens->get(i);
			sym_int_token_rec *b = (sym_int_token_rec*)tokens->get(i+1);
			if (!simplify_reduce_neutral(b->bop, b->token)) break;
			tokens->remove(i);	// remove x/a
			tokens->del(i);	// delete N/b
			tokens->insert_before(new sym_int_token_rec(a->uop, a->bop, a->token), i);
			c--;
		} while (c>=2);
	}
/**/
/* III. DESTRUCTIVE OPERATIONS
   reduce all destructive (e.g. "...*0" -> 0) sub-expressions */

/* step III.1: "y$x..." (front, commutative) */
	while (c>=2) {
		sym_int_token_rec *a = (sym_int_token_rec*)tokens->get(0);
		sym_int_token_rec *b = (sym_int_token_rec*)tokens->get(1);
		if (!op_commutative[b->bop]) break;
		sym_int_token *repl = simplify_reduce_destructive(b->bop, a->token);
		if (!repl) break;
		tokens->del(0);	// delete y
		tokens->del(0);	// delete x
		tokens->prepend(new sym_int_token_rec(u_null, b_invalid, repl));
		c--;
	}

/* step III.2: "...x$y..." (tail) */
	for (uint i=1; i<c; i++) {
		do {
			if (i >= c) break;
			sym_int_token_rec *a = (sym_int_token_rec*)tokens->get(i);
			sym_int_token *repl = simplify_reduce_destructive(a->bop, a->token);
			if (!repl) break;
			uint dc = i+1;
			tokens->del_multiple(0, dc);
			tokens->prepend(new sym_int_token_rec(u_null, b_invalid, repl));
			c -= dc-1;
		} while (c>=2);
	}
/**/
/* IV. INVERSE OPERATIONS
   e.g. "...-x+x", "...^x^x", etc. */

/* step IV.1: "...$x$xi..." (everywhere) */
	for (uint i=0; i<c-1; i++) {
		do {
			if (i >= c-1) break;
			sym_int_token_rec *a = (sym_int_token_rec*)tokens->get(i);
			sym_int_token_rec *b = (sym_int_token_rec*)tokens->get(i+1);
			sym_int_token *repl;
			if (!simplify_reduce_inverse(a->bop, a->token, b->bop, b->token, &repl)) break;
			tokens->del(i);
			tokens->del(i);
			c-=2;
			if (repl) {
				tokens->insert_before(new sym_int_token_rec(a->uop, a->bop, repl), i);
				c++;
			}
		} while (c>=2);
	}
/**/
	} while (d != c);
// op_destructive
}

bool sym_int::simplify_reduce_inverse(b_op oa, sym_int_token *a, b_op ob, sym_int_token *b, sym_int_token **repl)
{
	*repl = NULL;
	bool eq = (a->getObjectID() == b->getObjectID()) ? a->compare_eq(b) : false;
	if (eq) {
		if (oa == b_invalid) {
			if ((ob == b_sub) || (ob == b_xor)) {
			// a ^ a, a - a -> 0
				*repl = new sym_int_const(0);
				return true;
			} else if ((ob == b_and) || (ob == b_or)) {
			// a & a, a | a -> a
				*repl = (sym_int_token*)a->clone();
				return true;
			}
		} else if ((oa == b_add) && (ob == b_sub)) {
		// +x-x -> _
			return true;
		} else if ((oa == b_sub) && (ob == b_add)) {
		// -x+x -> _
			return true;
		}
	}
	return false;
}

sym_int_token *sym_int::simplify_reduce_destructive(b_op o, sym_int_token *x)
{
/* e.g. "x*0", "x&0" */
	op_int_int_prop *k = &op_destructive[o];
	if (k->has_prop) {
		uint X;
		if ((x->evaluate(&X)) && (X == k->value1)) {
			return new sym_int_const(k->value2);
		}
	}
	return NULL;
}

bool sym_int::simplify_reduce_neutral(b_op o, sym_int_token *x)
{
/* reduce "x$N" case ($ operator, N neutral element for $) */
	op_int_prop *k = &op_neutrals[o];
	if (k->has_prop) {
		uint i;
		if ((x->evaluate(&i)) && (k->value == i)) {
				return true;
		}
	}
	return false;
}

bool sym_int::simplify_reduce_const(b_op oa, sym_int_token *a, b_op ob, sym_int_token *b, b_op *res_op, sym_int_token **res_token)
{
	uint A, B, C;
	if (!a->evaluate(&A)) return false;
	if (!b->evaluate(&B)) return false;
	if (oa == b_invalid) {
		switch (ob) {
			case b_add: C = A+B; break;
			case b_sub: C = A-B; break;
			case b_mul: C = A*B; break;
			case b_div: if (!B) return false; else C = A/B; break;
			case b_mod: if (!B) return false; else C = A%B; break;
			case b_and: C = A&B; break;
			case b_or:  C = A|B; break;
			case b_xor: C = A^B; break;
			case b_invalid: break;
		}
		*res_op = b_invalid;
	} else if ((oa == b_add) && (ob == b_sub)) {
		C = A-B;
		*res_op = b_add;
	} else if ((oa == b_sub) && (ob == b_add)) {
		C = B-A;
		*res_op = b_add;
	} else if ((oa == b_add) && (ob == b_add)) {
		C = A+B;
		*res_op = b_add;
	} else if ((oa == b_sub) && (ob == b_sub)) {
		C = -A-B;
		*res_op = b_add;
	} else if ((oa == b_or) && (ob == b_or)) {
		C = A|B;
		*res_op = b_or;
	} else if ((oa == b_and) && (ob == b_and)) {
		C = A&B;
		*res_op = b_and;
	} else if ((oa == b_or) && (ob == b_and) && ((A&B) == 0)) {
		C = B;
		*res_op = b_and;
	} else return false;

	if (((int)C<0) && (*res_op == b_add)) {
		*res_op = b_sub;
		C = -C;
	}
	*res_token = new sym_int_const(C);
	return true;
}

void sym_int::u_operate(u_op uop)
{
}

/*
 *	CLASS sym_bool_symbol
 */

sym_bool_symbol::sym_bool_symbol(char *n)
{
	name = strdup(n);
}

sym_bool_symbol::~sym_bool_symbol()
{
	free(name);
}

bool sym_bool_symbol::compare_eq(sym_bool_token *t)
{
	sym_bool_symbol *s = (sym_bool_symbol*)t;
	return (strcmp(name, s->name) == 0);
}

Object *sym_bool_symbol::clone() const
{
	sym_bool_symbol *p = new sym_bool_symbol(name);
	return p;
}

bool sym_bool_symbol::evaluate(bool *i)
{
	return false;
}

int sym_bool_symbol::nstrfy(char *buf, int n)
{
	int q = strlen(name)+1;
	if (q > n) q = n;
	memmove(buf, name, q);
	return q-1;
/* FIXME: use n */
//	return sprintf(buf, "%s", name);
}

ObjectID sym_bool_symbol::getObjectID() const
{
	return ATOM_SYM_BOOL_SYMBOL;
}

/*
 *	CLASS sym_bool_token
 */

bool sym_bool_token::evaluate(bool *i)
{
	return false;
}

void sym_bool_token::simplify()
{
}

/*
 *	CLASS sym_bool_const
 */

sym_bool_const::sym_bool_const(bool v)
{
	value = v;
}

bool sym_bool_const::compare_eq(sym_bool_token *t)
{
	sym_bool_const *s = (sym_bool_const*)t;
	return (value == s->value);
}

bool sym_bool_const::evaluate(bool *i)
{
	*i = value;
	return true;
}

int sym_bool_const::nstrfy(char *buf, int n)
{
	const char *name = value ? "true" : "false";
	int q = strlen(name)+1;
	if (q > n) q = n;
	memmove(buf, name, q);
	return q-1;
//	return sprintf(buf, "%s", value ? "true" : "false");
}

/*
 *	CLASS sym_bool_intcmp
 */

sym_bool_intcmp::sym_bool_intcmp(sym_int_token *i1, c_op c, sym_int_token *i2)
{
	int1 = i1;
	cop = c;
	int2 = i2;
}

bool sym_bool_intcmp::compare_eq(sym_bool_token *t)
{
	sym_bool_intcmp *s = (sym_bool_intcmp*)t;
/* FIXME: i1 == si2 && i2 == si1 */
	return (int1->compare_eq(s->int1) && int2->compare_eq(s->int2) && cop == s->cop);
}

Object *sym_bool_intcmp::clone() const
{
	return new sym_bool_intcmp((sym_int_token*)int1->clone(), cop, (sym_int_token*)int2->clone());
}

bool sym_bool_intcmp::evaluate(bool *i)
{
	uint e1, e2;
	bool e = false;
	if (!int1->evaluate(&e1)) return false;
	if (!int2->evaluate(&e2)) return false;
	switch (cop) {
		case c_invalid: break;
		case c_eq: e = (e1 == e2); break;
		case c_ne: e = (e1 != e2); break;  
		case c_gt: e = (e1 > e2); break;  
		case c_ge: e = (e1 >= e2); break;  
		case c_lt: e = (e1 < e2); break;  
		case c_le: e = (e1 <= e2); break;  
	}
	*i = e;
	return true;
}

int sym_bool_intcmp::nstrfy(char *buf, int n)
{
	int i = 0;
	uint32 C1, C2;
	if (int1->evaluate(&C1) && int2->evaluate(&C2)) {
		bool r;
		bool handled = true;
		switch (cop) {
			case c_eq: r = (C1 == C2); break;
			case c_ne: r = (C1 != C2); break;
			case c_gt: r = (C1 > C2); break;
			case c_ge: r = (C1 >= C2); break;
			case c_lt: r = (C1 < C2); break;
			case c_le: r = (C1 <= C2); break;
			default: handled = false;
		}
		if (handled) {
			i += sprintf(buf, "%s", r ? "true" : "false");
			return i;
		}
	}
	i += int1->nstrfy(buf+i, n-i);
	switch (cop) {
		case c_invalid: break;
		case c_eq:  strcpy(buf+i, "=="); i+=2; break;
		case c_ne:  strcpy(buf+i, "!="); i+=2; break;
		case c_gt:  buf[i++] = '>'; break;
		case c_ge:  strcpy(buf+i, ">="); i+=2; break;
		case c_lt:  buf[i++] = '<'; break;
		case c_le:  strcpy(buf+i, "<="); i+=2; break;
	}
	i += int2->nstrfy(buf+i, n-i);
	return i;
}

ObjectID sym_bool_intcmp::getObjectID() const
{
	return ATOM_SYM_BOOL_INTCMP;
}

void sym_bool_intcmp::simplify()
{
	int1->simplify();
	int2->simplify();
}

/*
 *	CLASS sym_bool_token_rec
 */
 
class sym_bool_token_rec: public Object {
public:
	n_op nop;
	l_op lop;
	sym_bool_token *token;
	
	sym_bool_token_rec(n_op n, l_op l, sym_bool_token *t)
	{
		nop = n;
		lop = l;
		token = t;		
	}
	
	~sym_bool_token_rec()
	{
		token->done();
		delete token;
	}
	
	int nstrfy(char *buf, int n)
	{
		int i = 0;
		switch (nop) {
			case n_null: break;
			case n_not: buf[i++] = '!'; break;
		}
		switch (lop) {
			case l_invalid: break;
			case l_and: strcpy(buf+i, "&&"); i+=2; break;
			case l_or:  strcpy(buf+i, "||"); i+=2; break;
			case l_eq:  strcpy(buf+i, "=="); i+=2; break;
			case l_ne:  strcpy(buf+i, "!="); i+=2; break;
			case l_gt:  buf[i++] = '>'; break;
			case l_ge:  strcpy(buf+i, ">="); i+=2; break;
			case l_lt:  buf[i++] = '<'; break;
			case l_le:  strcpy(buf+i, "<="); i+=2; break;
		}
		return i + token->nstrfy(buf+i, n-i);
	}
	
	Object *clone() const
	{
		return new sym_bool_token_rec(nop, lop, (sym_bool_token*)token->clone());
	}
};

/*
 *	CLASS sym_bool
 */

sym_bool::sym_bool()
{
	tokens = new ht_clist();
	((ht_clist*)tokens)->init();
}

sym_bool::~sym_bool()
{
	tokens->destroy();
	delete tokens;
}

bool sym_bool::compare_eq(sym_bool_token *t)
{
// FIXME: implement me !
	return false;
}

Object *sym_bool::clone() const
{
	sym_bool *p = new sym_bool();
	p->tokens = (ht_list*)tokens->clone();
	return p;
}

void sym_bool::clear()
{
	tokens->destroy();
	delete tokens;

	tokens = new ht_clist();
	((ht_clist*)tokens)->init();
}

bool sym_bool::evaluate(bool *i)
{
	int c = tokens->count();
	bool l;
	for (int j = 0; j < c; j++) {
		bool k;
		sym_bool_token_rec *r = (sym_bool_token_rec*)tokens->get(j);
		if (!r->token->evaluate(&k)) return false;
		if (j == 0) l = k;
		switch (r->nop) {
			case n_null: break;
			case n_not: k = !k; break;
		}
		switch (r->lop) {
			case l_invalid: break;
			case l_and: l = (l && k); break;
			case l_or: l = (l || k); break;
			case l_eq: l = (l == k); break;
			case l_ne: l = (l != k); break;
			case l_gt: l = (l > k); break;
			case l_ge: l = (l >= k); break;
			case l_lt: l = (l < k); break;
			case l_le: l = (l <= k); break;
		}
	}
	*i = l;
	return true;
}

void sym_bool::l_operate(l_op l, sym_bool_token *t)
{
	tokens->insert(new sym_bool_token_rec(n_null, l, t));
}

int sym_bool::nstrfy(char *buf, int n)
{
	int l = 0;
	int c = tokens->count();
	for (int i = 0; i < c; i++) {
		sym_bool_token_rec *r = (sym_bool_token_rec*)tokens->get(i);
		l += r->nstrfy(buf+l, n-l);
	}
	return l;
}

void sym_bool::set(sym_bool_token *t)
{
	clear();
	tokens->insert(new sym_bool_token_rec(n_null, l_invalid, t));
}

void sym_bool::simplify()
{
	int c = tokens->count();
	for (int i = 0; i < c; i++) {
		sym_bool_token_rec *r = (sym_bool_token_rec*)tokens->get(i);
		r->token->simplify();
	}
}

void sym_bool::n_operate(n_op n)
{
}

ObjectID sym_bool::getObjectID() const
{
	return ATOM_SYM_BOOL;
}

