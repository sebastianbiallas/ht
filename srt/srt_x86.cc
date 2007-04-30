/*
 *	HT Editor
 *	srt_x86.cc
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

#include "htctrl.h"		// FIXME: remove this
#include "htexcept.h"
#include "htiobox.h"
#include "srt_x86.h"
#include "x86dis.h"

#define X86_FLAG_CARRY		0
#define X86_FLAG_PARITY		1
#define X86_FLAG_AUX		2
#define X86_FLAG_ZERO		3
#define X86_FLAG_SIGNED		4
#define X86_FLAG_OVERFLOW	5

#define X86_FLAGS			6

char *srt_x86_flags[X86_FLAGS] = {
	"carry", "parity", "aux",
	"zero", "signed", "overflow"
};

struct CPU {
	sym_int *regs[8];
	sym_bool *flags[X86_FLAGS];
};

char *srt_x86_idx2reg(uint idx)
{
	if (idx >= 8) {
		return srt_x86_flags[(idx-8) % X86_FLAGS];
	}
	return x86_regs[2][idx & 7];
}

void srt_x86_setreg(CPU *cpu, uint idx, Object *o)
{
	if (idx >= 8) {
		sym_bool **k;
		k = &cpu->flags[(idx-8) % X86_FLAGS];
		(*k)->done();
		delete *k;
		*k = (sym_bool *)o;
	} else {
		sym_int **k;
		k = &cpu->regs[idx & 7];
		(*k)->done();
		delete *k;
		*k = (sym_int *)o;
	}
}

/*
 *	CLASS sym_int_reg_x86
 */

class sym_int_reg_x86: public sym_int_reg {
public:
	sym_int_reg_x86(uint r): sym_int_reg(r)
	{
	}

	Object *clone() const
	{
		return new sym_int_reg_x86(regidx);
	}

	virtual int nstrfy(char *buf, int n)
	{
		return sprintf(buf,"%s", srt_x86_idx2reg(regidx));
	}
}; 

/*
 *	srt_x86
 */

state_mod *srt_x86_flag(uint flagidx, sym_bool_token *value)
{
	state_mod *f = new state_mod();
	f->ismem = false;
	f->dest.regidx = 8 + flagidx;
	f->isbool = true;
	f->value.boolean = value;
	return f;
}

state_mod *srt_x86_reg(uint regidx, sym_int_token *value)
{
	state_mod *r = new state_mod();
	r->ismem = false;
	r->dest.regidx = regidx;
	r->isbool = false;
	r->value.integer = value;
	return r;
}

void srt_x86_flags_std(Container *rm, x86dis_insn *insn, sym_int *cond)
{
	state_mod *zf = new state_mod();
	zf->ismem = false;
	zf->dest.regidx = 8 + X86_FLAG_ZERO;
	zf->isbool = true;

	state_mod *sf = new state_mod();
	sf->ismem = false;
	sf->dest.regidx = 8 + X86_FLAG_SIGNED;
	sf->isbool = true;

	sym_int *i;
	sym_bool *b;

	switch (insn->op[0].size) {
		case 1:
			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0xff));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_eq, new sym_int_const(0)));
			zf->value.boolean = b;

			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0x80));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_ne, new sym_int_const(0)));
			sf->value.boolean = b;
			break;
		case 2:
			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0xffff));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_eq, new sym_int_const(0)));
			zf->value.boolean = b;

			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0x8000));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_ne, new sym_int_const(0)));
			sf->value.boolean = b;
			break;
		case 4:
			i = (sym_int*)cond->clone();
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_eq, new sym_int_const(0)));
			zf->value.boolean = b;

			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0x80000000));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_ne, new sym_int_const(0)));
			sf->value.boolean = b;
			break;
	}
	rm->insert(zf);
	rm->insert(sf);
}

void srt_x86_flags_carry(Container *rm, x86dis_insn *insn, sym_int *cond, sym_bool *carry)
{
	state_mod *cf = new state_mod();
	cf->ismem = false;
	cf->dest.regidx = 8 + X86_FLAG_CARRY;
	cf->isbool = true;

	sym_int *i;
	sym_bool *b;

	switch (insn->op[0].size) {
		case 1:
			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0x100));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_ne, new sym_int_const(0)));
			cf->value.boolean = b;
			break;
		case 2:
			i = (sym_int*)cond->clone();
			i->b_operate(b_and, new sym_int_const(0x10000));
			b = new sym_bool();
			b->set(new sym_bool_intcmp(i, c_ne, new sym_int_const(0)));
			cf->value.boolean = b;
			break;
		case 4:
			cf->value.boolean = (sym_bool_token*)carry->clone();
			break;
	}
	rm->insert(cf);
}

/*void srt_x86_flags_overflow(cpu *CPU, x86dis_insn *insn, uint32 a, uint32 b, uint32 c)
{
	bool overflow_on = false;
	switch (insn->op[0].size) {
		case 1:
			if (((a & 0x80) == (b & 0x80)) && ((a & 0x80) != (c & 0x80))) {
				overflow_on = true;
				CPU->context.d.eflags |= FLAGS_OVERFLOW;
			} else {
				CPU->context.d.eflags &= ~FLAGS_OVERFLOW;
			}
			break;
		case 2:
			if (((a & 0x8000) == (b & 0x8000)) && ((a & 0x8000) != (c & 0x8000))) {
				CPU->context.d.eflags |= FLAGS_OVERFLOW;
			} else {
				CPU->context.d.eflags &= ~FLAGS_OVERFLOW;
			}
			break;
		case 4:
			if (((a & 0x80000000) == (b & 0x80000000)) && ((a & 0x80000000) != (c & 0x80000000))) {
				CPU->context.d.eflags |= FLAGS_OVERFLOW;
			} else {
				CPU->context.d.eflags &= ~FLAGS_OVERFLOW;
			}
			break;
	}
}*/

sym_int *srt_x86_mkreg(CPU *cpu, uint regidx)
{
	return (sym_int*)cpu->regs[regidx]->clone();
}

sym_int_token *srt_x86_mkaddr(CPU *cpu, x86_insn_op *o)
{
	sym_int *a = new sym_int();
	bool first = true;
	if (o->mem.index != X86_REG_NO) {
		a->b_operate(b_invalid, srt_x86_mkreg(cpu, o->mem.index));
		if (o->mem.scale > 1)
			a->b_operate(b_mul, new sym_int_const(o->mem.scale));
		first = false;
	}
	if (o->mem.base != X86_REG_NO) {
		a->b_operate(first ? b_invalid : b_add, srt_x86_mkreg(cpu, o->mem.base));
		first = false;
	}
	if (o->mem.hasdisp && o->mem.disp) {
		uint32 D = o->mem.disp;
		b_op op = first ? b_invalid : ((D&0x80000000) ? b_sub : b_add);
		if (D&0x80000000) D = -D;
		a->b_operate(op, new sym_int_const(D));
		first = false;
	}
	return a;
}

void srt_x86_mkdest(CPU *cpu, state_mod *m, x86_insn_op *d)
{
	switch (d->type) {
		case X86_OPTYPE_REG:
			m->ismem = false;
			m->dest.regidx = d->reg;
			break;
		case X86_OPTYPE_MEM:
			m->ismem = true;
			m->dest.mem.addr = srt_x86_mkaddr(cpu, d);
			m->dest.mem.size = d->size;
			m->dest.mem.endian = srte_le;
			break;
		default:
			throw ht_io_exception("unknown dest type: %d", d->type);
	}
}

sym_int *srt_x86_mkvalue(CPU *cpu, x86_insn_op *o)
{
	sym_int *r = new sym_int();
	switch (o->type) {
		case X86_OPTYPE_IMM: {
			r->set(new sym_int_const(o->imm));
			break;
		}
		case X86_OPTYPE_REG: {
			r->set(srt_x86_mkreg(cpu, o->reg));
			break;
		}
		case X86_OPTYPE_MEM:
			r->set(new sym_int_mem(srt_x86_mkaddr(cpu, o), o->size, srte_le));
			break;
		default:
			throw ht_io_exception("unknown op type: %d", o->type);
	}
	return r;
}

void srt_x86_destmod(CPU *cpu, Container *rm, x86_insn_op *op, sym_int_token *value)
{
	state_mod *m = new state_mod();
	srt_x86_mkdest(cpu, m, op);
	m->isbool = false;
	m->value.integer = value;
	rm->insert(m);
}

/*
 *	COMMANDS
 */

void srt_x86_add(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_add, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
	sym_bool *carry = new sym_bool();
	carry->set(new sym_bool_intcmp(
		(sym_int*)v->clone(), c_lt,
		srt_x86_mkvalue(cpu, &insn->op[1])));
	srt_x86_flags_carry(rm, insn, v, carry);
	delete carry;
// FIXME: overflow flag
}

void srt_x86_and(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_and, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
}

void srt_x86_cmp(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_sub, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_flags_std(rm, insn, v);
	sym_bool *carry = new sym_bool();
	carry->set(new sym_bool_intcmp(
		srt_x86_mkvalue(cpu, &insn->op[0]), c_lt,
		srt_x86_mkvalue(cpu, &insn->op[1])));
	srt_x86_flags_carry(rm, insn, v, carry);
	delete carry;
// FIXME: overflow flag
}

void srt_x86_dec(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_sub, new sym_int_const(1));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
}

void srt_x86_div(CPU *cpu, Container *rm, x86dis_insn *insn)
{
// FIXME: DX:AX / op -> EAX, DX:AX % op -> EDX
	sym_int *q = srt_x86_mkvalue(cpu, &insn->op[0]);
	q->b_operate(b_div, srt_x86_mkvalue(cpu, &insn->op[1]));

	sym_int *r = srt_x86_mkvalue(cpu, &insn->op[0]);
	r->b_operate(b_mod, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], q);

	state_mod *m = new state_mod();
	m->ismem = false;
	m->dest.regidx = X86_REG_DX;
	m->isbool = false;
	m->value.integer = r;
	rm->insert(m);

	srt_x86_flags_std(rm, insn, q);
// FIXME: flags
/*	sym_bool *carry = new sym_bool();
	carry->set(new sym_bool_intcmp(
		(sym_int*)v->clone(), c_lt,
		srt_x86_mkvalue(cpu, &insn->op[1])));
	srt_x86_flags_carry(rm, insn, v, carry);
	delete carry;*/
}

void srt_x86_inc(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_add, new sym_int_const(1));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
}

void srt_x86_lea(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	state_mod *m = new state_mod();
	srt_x86_mkdest(cpu, m, &insn->op[0]);
	if (insn->op[1].type != X86_OPTYPE_MEM) throw ht_io_exception("internal error in %s at %d", __FILE__, __LINE__);
	m->isbool = false;
	m->value.integer = srt_x86_mkaddr(cpu, &insn->op[1]);
	rm->insert(m);
}

void srt_x86_mov(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	state_mod *m = new state_mod();
	srt_x86_mkdest(cpu, m, &insn->op[0]);
	m->isbool = false;
	m->value.integer = srt_x86_mkvalue(cpu, &insn->op[1]);
	rm->insert(m);
}

void srt_x86_mul(CPU *cpu, Container *rm, x86dis_insn *insn)
{
// FIXME: op * AX = DX:AX
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_mul, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

/*	state_mod *m = new state_mod();
	m->ismem = false;
	m->dest.regidx = ;
	m->isbool = false;
	m->value.integer = v;
	rm->insert(m);*/

	srt_x86_flags_std(rm, insn, v);
// FIXME: flags
/*	sym_bool *carry = new sym_bool();
	carry->set(new sym_bool_intcmp(
		(sym_int*)v->clone(), c_lt,
		srt_x86_mkvalue(cpu, &insn->op[1])));
	srt_x86_flags_carry(rm, insn, v, carry);
	delete carry;*/
}

void srt_x86_or(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_or, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
}

void srt_x86_pop(CPU *cpu, Container *rm, x86dis_insn *insn)
{
/*	uint32 a;
	read(context.w.ss, context.d.esp, &a, 4);
	context.d.esp += 4;
	return a;*/
	state_mod *m;
	int size = 4;

	m = new state_mod();
	srt_x86_mkdest(cpu, m, &insn->op[0]);
	m->isbool = false;
	m->value.integer = new sym_int_mem(srt_x86_mkreg(cpu, X86_REG_SP), size, srte_le);
	rm->insert(m);

	sym_int *sp = srt_x86_mkreg(cpu, X86_REG_SP);
	sp->b_operate(b_add, new sym_int_const(size));

	m = new state_mod();
	m->ismem = false;
	m->dest.regidx = X86_REG_SP;
	m->isbool = false;
	m->value.integer = sp;
	rm->insert(m);
}

void srt_x86_push(CPU *cpu, Container *rm, x86dis_insn *insn)
{
/*	context.d.esp -= 4;
	write(context.w.ss, context.d.esp, &a, 4);*/
	state_mod *m;
	int size = 4;

	sym_int *sp = srt_x86_mkreg(cpu, X86_REG_SP);
	sp->b_operate(b_sub, new sym_int_const(size));

	m = new state_mod();
	m->ismem = false;
	m->dest.regidx = X86_REG_SP;
	m->isbool = false;
	m->value.integer = sp;
	rm->insert(m);
	
	m = new state_mod();
	m->ismem = true;
	m->dest.mem.addr = (sym_int*)sp->clone();
	m->dest.mem.size = size;
	m->dest.mem.endian = srte_le;
	m->isbool = false;
	m->value.integer = srt_x86_mkvalue(cpu, &insn->op[0]);
	rm->insert(m);
}

void srt_x86_sub(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_sub, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
	sym_bool *carry = new sym_bool();
	carry->set(new sym_bool_intcmp(
		srt_x86_mkvalue(cpu, &insn->op[0]), c_lt,
		srt_x86_mkvalue(cpu, &insn->op[1])));
	srt_x86_flags_carry(rm, insn, v, carry);
	delete carry;
// FIXME: overflow flag
}

void srt_x86_shl(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	sym_int *s = srt_x86_mkvalue(cpu, &insn->op[1]);
	uint S;
	if (!s->evaluate(&S)) throw ht_io_exception("shl/shr with non-constant operand not supported");
	delete s;
	v->b_operate(b_mul, new sym_int_const(1 << S));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
// FIXME: flags
}

void srt_x86_shr(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	sym_int *s = srt_x86_mkvalue(cpu, &insn->op[1]);
	uint S;
	if (!s->evaluate(&S)) throw ht_io_exception("shl/shr with non-constant operand not supported");
	delete s;
	v->b_operate(b_div, new sym_int_const(1 << S));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
// FIXME: flags
}

void srt_x86_test(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_and, srt_x86_mkvalue(cpu, &insn->op[1]));
	srt_x86_flags_std(rm, insn, v);
}

void srt_x86_xor(CPU *cpu, Container *rm, x86dis_insn *insn)
{
	sym_int *v = srt_x86_mkvalue(cpu, &insn->op[0]);
	v->b_operate(b_xor, srt_x86_mkvalue(cpu, &insn->op[1]));

	srt_x86_destmod(cpu, rm, &insn->op[0], v);

	srt_x86_flags_std(rm, insn, v);
	rm->insert(srt_x86_flag(X86_FLAG_CARRY, new sym_bool_const(false)));
	rm->insert(srt_x86_flag(X86_FLAG_OVERFLOW, new sym_bool_const(false)));
}

typedef void (*ecmd_handler)(CPU *cpu, Container *rm, x86dis_insn *insn);

struct ecmd {
	char *name;
	ecmd_handler h;
};

ecmd ecmds[] = {
	{"add",  srt_x86_add},
	{"and",  srt_x86_and},
	{"cmp",  srt_x86_cmp},
	{"dec",  srt_x86_dec},
	{"div",  srt_x86_div},
	{"inc",  srt_x86_inc},
	{"lea",  srt_x86_lea},
	{"mov",  srt_x86_mov},
	{"mul",  srt_x86_mul},
	{"or",   srt_x86_or},
	{"push", srt_x86_push},
	{"pop",  srt_x86_pop},
	{"sub",  srt_x86_sub},
	{"shl",  srt_x86_shl},
	{"shr",  srt_x86_shr},
	{"test", srt_x86_test},
	{"xor",  srt_x86_xor},
	{NULL, NULL}
};

Container *srt_x86_single(CPU *cpu, x86dis_insn *i)
{
	Array *rm = new Array(true);

	ecmd *e = ecmds;
	while (e->name) {
		if (strcmp(i->name, e->name) == 0) {
			e->h(cpu, rm, i);
			return rm;
		}
		e++;
	}
	throw ht_io_exception("unsupported cmd: %s", i->name);
}

void create_cpu(CPU *cpu)
{
	for (uint g = 0; g<8; g++) {
		char s[32];
		sprintf(s, "i%s", srt_x86_idx2reg(g));
		cpu->regs[g] = new sym_int();
		cpu->regs[g]->init();
		cpu->regs[g]->set(new sym_int_symbol(s));
	}

	for (uint g = 0; g<X86_FLAGS; g++) {
		char s[32];
		sprintf(s, "i%s", srt_x86_idx2reg(8+g));
		cpu->flags[g] = new sym_bool();
		cpu->flags[g]->init();
		cpu->flags[g]->set(new sym_bool_symbol(s));
	}
}

void destroy_cpu(CPU *cpu)
{
	for (uint g = 0; g<8; g++) {
		cpu->regs[g]->done();
		delete cpu->regs[g];
	}

	for (uint g = 0; g<X86_FLAGS; g++) {
		cpu->flags[g]->done();
		delete cpu->flags[g];
	}
}

void srt_x86(Analyser *analy, Address *addr)
{
	x86dis *x = (x86dis*)analy->disasm;
	CPU_ADDR a;
	byte buf[15];

	addr->putIntoCPUAddress(&a);
	CPU cpu;

	create_cpu(&cpu);

/**/
	char str[256];
	sprintf(str, "symbolic register trace (x86)");
	Bounds b, c, d;
	app->getbounds(&c);
	b.w = 3*c.w/4;
	b.h = 3*c.h/4;
	b.x = (c.w-b.w)/2;
	b.y = (c.h-b.h)/2;
	ht_dialog *dialog = new ht_dialog();
	dialog->init(&b, str, FS_KILLER | FS_TITLE | FS_MOVE);
	d.x = 0;
	d.y = 0;
	d.w = b.w-2;
	d.h = 1;
	ht_listbox_title *text = new ht_listbox_title();
	text->init(&d);
	text->setText(3, "insn", "register", "new value");
	dialog->insert(text);
	d.y = 1;
	d.h = b.h-3;
	ht_text_listbox *list;
	NEW_OBJECT(list, ht_text_listbox, &d, 3, 2);
	list->attachTitle(text);
//	char str2[1024];
/**/
#define MAX_INSNS	20
	for (int i=0; i<MAX_INSNS; i++) {
		if (!analy->validAddress(addr, scinitialized)) break;
		uint bz = analy->bufPtr(addr, buf, sizeof buf);
		dis_insn *i = (x86dis_insn*)x->decode(buf, bz, a);
		if (!x->validInsn(i)) break;
		x86dis_insn *xi = (x86dis_insn*)i;
		char *dname = x->str(i, DIS_STYLE_HEX_NOZEROPAD + DIS_STYLE_HEX_ASMSTYLE);
		Container *rm = NULL;

		try{
			rm = srt_x86_single(&cpu, xi);
		} catch (const ht_exception &x) {
			errorbox("error: %s", x.what());
			break;
		}

		uint c = rm->count();
		for (uint i = 0; i < c; i++) {
			state_mod *r = (state_mod*)(*rm)[i];
			char en[256];
			if (r->isbool) {
				r->value.boolean->simplify();
				r->value.boolean->nstrfy(en, sizeof en);
			} else {
				r->value.integer->simplify();
				r->value.integer->nstrfy(en, sizeof en);
			}
			if (r->ismem) {
				char c[256];
				char c2[256];
				r->dest.mem.addr->nstrfy(c, sizeof c);
//				infobox("%s causes memmod:\n%s%d[%s] := '%s'", dname, srt_endian_to_str(r->dest.mem.endian), r->dest.mem.size, c, en);
				sprintf(c2, "%s%d[%s]", srt_endian_to_str(r->dest.mem.endian), r->dest.mem.size, c);
				list->insert_str(0, dname, c2, en);
			} else {
//				infobox("%s causes regmod:\n%s := '%s'", dname, x86_idx2reg(r->dest.regidx), en);
				list->insert_str(0, dname, srt_x86_idx2reg(r->dest.regidx), en);
			}
			dname = "";
		}
		for (uint i = 0; i<c; i++) {
			state_mod *r = (state_mod*)(*rm)[i];
			if (r->isbool) {
				srt_x86_setreg(&cpu, r->dest.regidx, r->value.boolean);
			} else {
				srt_x86_setreg(&cpu, r->dest.regidx, r->value.integer);
			}
		}
		addr->add(x->getSize(xi));
	}
	destroy_cpu(&cpu);
/**/
	list->update();
	dialog->insert(list);
	int r = dialog->run(0);
	if (r == button_ok) {
//		ht_listbox_data data;
/*		list->databuf_get(&data);
		goto_address(data.cursor_id, this);*/
	}
	dialog->done();
	delete dialog;
}

