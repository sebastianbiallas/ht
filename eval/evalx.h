#ifndef __EVALX_H__
#define __EVALX_H__

#include "evaltype.h"

int eval(scalar_t *r, const char *str, eval_func_handler_t func_handler, eval_symbol_handler_t symbol_handler, void *context);

#endif /* __EVALX_H__ */
