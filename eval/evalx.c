#include "evaltype.h"
#include "eval.h"
#include "lex.h"

extern int yyparse(eval_scalar *result);
	
int eval(eval_scalar *r, const char *str, eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context)
{
	void *oldbuffer = lex_current_buffer();
	void *strbuffer = lex_scan_string_buffer(str);
	eval_scalar result;

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
	
	if (result.type==SCALAR_NULL) return 0;
	if (get_eval_error(0, 0)) return 0;
		
	*r=result;
	
	DEBUG_DUMP_INDENT_OUT;
	DEBUG_DUMP_SCALAR(r, "eval result:");

	return 1;
}
