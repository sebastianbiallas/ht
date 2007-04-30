#ifndef EVALTYPE_H
#define EVALTYPE_H

#define MAX_EVALFUNC_PARAMS	8

#include "io/types.h"
/*
 *	Types
 */

typedef enum {
	TYPE_UNKNOWN,
	TYPE_BYTE,
	TYPE_WORD,
	TYPE_DWORD
} eval_inttype;

typedef struct {
	uint64 value;
	eval_inttype type;
} eval_int;

typedef struct {
	double value;
} eval_float;

typedef struct {
	char *value;
	int len;
} eval_str;

typedef enum {
	SCALAR_NULL=0,
	SCALAR_INT,
	SCALAR_STR,
	SCALAR_FLOAT,
	SCALAR_ANY,
	SCALAR_VARARGS
} eval_scalartype;

typedef union {
	eval_int integer;
	eval_str str;
	eval_float floatnum;
} eval_scalarbody;

typedef struct {
	eval_scalartype type;
	eval_scalarbody scalar;
} eval_scalar;

typedef struct {
	int count;
	eval_scalar *scalars;
} eval_scalarlist;

typedef struct {
	const char *name;
	void *func;
	eval_scalartype ptype[MAX_EVALFUNC_PARAMS];
	const char *desc;
} eval_func;

typedef enum {
	PROTOMATCH_OK=0,
	PROTOMATCH_NAME_FAIL,
	PROTOMATCH_PARAM_FAIL
} eval_protomatch;

#endif /* EVALTYPE_H */

