#ifndef EVAL_H
#define EVAL_H

/*#define EVAL_DEBUG*/

#define MAX_EVALFUNC_PARAMS	8

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
	int value;
	inttype_t type;
} int_t;

typedef struct {
	double value;
} float_t;

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
	float_t floatnum;
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

/*
 *	Debugging
 */

#ifdef EVAL_DEBUG

extern int debug_dump_ident;

#define DEBUG_DUMP_INDENT {\
	int i;\
	for (i=0; i<debug_dump_ident; i++) printf("\t");\
}

#define DEBUG_DUMP(text...) {\
	DEBUG_DUMP_INDENT;\
	printf(text);\
	printf("\n");\
}
	
#define DEBUG_DUMP_SCALAR(scalarptr, text...) {\
	DEBUG_DUMP_INDENT;\
	printf(text);\
	scalar_dump(scalarptr);\
	printf("\n");\
}

#define DEBUG_DUMP_SCALARLIST(scalarlistptr, text...) {\
	DEBUG_DUMP_INDENT;\
	printf(text);\
	scalarlist_dump(scalarlistptr);\
	printf("\n");\
}

#define DEBUG_DUMP_INDENT_IN debug_dump_ident++
#define DEBUG_DUMP_INDENT_OUT debug_dump_ident--

#else

#define DEBUG_DUMP(text...)
#define DEBUG_DUMP_SCALAR(scalarptr, text...)
#define DEBUG_DUMP_SCALARLIST(scalarlistptr, text...)
#define DEBUG_DUMP_INDENT_IN
#define DEBUG_DUMP_INDENT_OUT

#endif

/*
 *	functions
 */
 
typedef int (*eval_func_handler_t)(scalar_t *result, char *name, scalarlist_t *params);
typedef int (*eval_symbol_handler_t)(scalar_t *result, char *name);

void clear_eval_error();
int get_eval_error(char **str, int *pos); 
void set_eval_error(char *format,...); 

void *eval_get_context();
void eval_set_context(void *context);
void eval_set_func_handler(eval_func_handler_t func_handler);
void eval_set_symbol_handler(eval_symbol_handler_t symbol_handler);
protomatch_t match_evalfunc_proto(char *name, scalarlist_t *params, evalfunc_t *proto);
int exec_evalfunc(scalar_t *result, scalarlist_t *params, evalfunc_t *proto);
int std_eval_func_handler(scalar_t *r, char *fname, scalarlist_t *params, evalfunc_t *protos);

int func_eval(scalar_t *r, str_t *p);

void scalar_context_int(scalar_t *s, int_t *t);
void scalar_context_str(scalar_t *s, str_t *t);
void scalar_context_float(scalar_t *s, float_t *t);

void scalar_create_int(scalar_t *s, int_t *t);
void scalar_create_int_c(scalar_t *s, int i);
void scalar_create_str(scalar_t *s, str_t *t);
void scalar_create_str_c(scalar_t *s, char *cstr);
void scalar_create_float(scalar_t *s, float_t *t);
void scalar_create_float_c(scalar_t *s, double f);

void scalar_destroy(scalar_t *s);
void string_destroy(str_t *s);

#ifdef EVAL_DEBUG
extern void scalar_dump(scalar_t *s);
extern void scalarlist_dump(scalarlist_t *s);
#endif

#endif /* EVAL_H */
