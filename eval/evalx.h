#ifndef __EVALX_H__
#define __EVALX_H__

#include "evaltype.h"
#include "eval.h"

#ifdef __cplusplus
extern "C"
#endif
int eval(eval_scalar *r, const char *str, eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context);

#endif /* __EVALX_H__ */
