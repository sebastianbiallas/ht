#include "evaltype.h"
#include "eval.h"
#include "lex.h"

extern int yyparse(eval_scalar *result);
	
int eval(eval_scalar *r, const char *str, eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context)
{
	void *oldbuffer = lex_current_buffer();
	void *strbuffer;
	eval_scalar result;
/*     if (get_helpmode()) {
		eval_scalar *hs = get_helpstring();
		scalar_create_str_c(hs, "");
		strbuffer = lex_scan_string_buffer("NaF()");
	} else {*/
		strbuffer = lex_scan_string_buffer(str);
/*     }*/

	DEBUG_DUMP("evaluating \"%s\":", str);
	DEBUG_DUMP_INDENT_IN;
	
	clear_eval_error();
	
	eval_set_context(context);
	eval_set_func_handler(func_handler);
	eval_set_symbol_handler(symbol_handler);

	result.type=SCALAR_NULL;
	yyparse(&result);
	
	lex_delete_buffer(strbuffer);
	if (oldbuffer) lex_switch_buffer(oldbuffer);
	
/*     if (get_helpmode()) {
		eval_scalar *hs = get_helpstring();
		*r = *hs;
		hs->type = SCALAR_NULL;
		clear_eval_error();
	} else {*/
		if (result.type == SCALAR_NULL) return 0;
		*r = result;
		if (get_eval_error(0, 0)) return 0;
/*	}*/
	
	DEBUG_DUMP_INDENT_OUT;
	DEBUG_DUMP_SCALAR(r, "eval result:");

	return 1;
}
