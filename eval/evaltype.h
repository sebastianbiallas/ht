#ifndef EVALTYPE_H
#define EVALTYPE_H

#define MAX_EVALFUNC_PARAMS	8

//#ifdef HT_QWORD
#include "qword.h"

/*
 *	Types
 */

typedef enum {
	TYPE_UNKNOWN,
	TYPE_BYTE,
	TYPE_WORD,
	TYPE_DWORD
} inttype_t;

typedef struct {
	qword value;
	inttype_t type;
} int_t;

typedef struct {
	double value;
} ht_float_t;

typedef struct {
	char *value;
	int len;
} str_t;

typedef enum {
	SCALAR_NULL=0,
	SCALAR_INT,
	SCALAR_STR,
	SCALAR_FLOAT,
	SCALAR_VARARGS
} scalartype_t;

typedef union {
	int_t integer;
	str_t str;
	ht_float_t floatnum;
} scalarbody_t;

typedef struct {
	scalartype_t type;
	scalarbody_t scalar;
} scalar_t;

typedef struct {
	int count;
	scalar_t *scalars;
} scalarlist_t;

typedef struct {
	char *name;
	void *func;
	scalartype_t ptype[MAX_EVALFUNC_PARAMS];
	char *desc;
} evalfunc_t;

typedef enum {
	PROTOMATCH_OK=0,
	PROTOMATCH_NAME_FAIL,
	PROTOMATCH_PARAM_FAIL
} protomatch_t;

#endif /* EVALTYPE_H */

