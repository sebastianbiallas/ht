
%pure_parser

%{

#define YYPARSE_PARAM resultptr

#include "evaltype.h"
#include "eval.h"

void yyerror (char *s)
{
	set_eval_error(s);
}

%}

%union {
	eval_scalar scalar;
	char *ident;
	eval_scalarlist scalars;
};

%token <scalar> EVAL_INT
%token <scalar> EVAL_STR
%token <scalar> EVAL_FLOAT
%token <ident> EVAL_IDENT

%type <scalar> scalar
%type <scalars> scalarlist
%type <scalars> scalarlist_or_null
%type <ident> identifier

%left ','
%right '?' ':'
%left '.'
%left EVAL_LAND
%left EVAL_LXOR
%left EVAL_LOR
%left '|'
%left '^'
%left '&'
%nonassoc EVAL_EQ
%nonassoc EVAL_NE
%nonassoc EVAL_STR_EQ
%nonassoc EVAL_STR_NE
%nonassoc EVAL_LT
%nonassoc EVAL_LE
%nonassoc EVAL_GT
%nonassoc EVAL_GE
%nonassoc EVAL_STR_LT
%nonassoc EVAL_STR_LE
%nonassoc EVAL_STR_GT
%nonassoc EVAL_STR_GE
%nonassoc EVAL_SHL
%nonassoc EVAL_SHR
%left '-' '+'
%left '*' '/' '%'

%left NEG '~' '!'
%left EVAL_POW

%%

input:	scalar			{ *(eval_scalar*)resultptr=$1; }
;

scalar:	  EVAL_INT			{ $$ = $1; }
	| EVAL_STR			{ $$ = $1; }
	| EVAL_FLOAT			{ $$ = $1; }
	| scalar '*' scalar		{ if (!scalar_op(&$$, &$1, &$3, '*')) YYERROR; }
	| scalar '/' scalar		{ if (!scalar_op(&$$, &$1, &$3, '/')) YYERROR; }
	| scalar '%' scalar		{ if (!scalar_op(&$$, &$1, &$3, '%')) YYERROR; }
	| scalar '+' scalar		{ if (!scalar_op(&$$, &$1, &$3, '+')) YYERROR; }
	| scalar '-' scalar		{ if (!scalar_op(&$$, &$1, &$3, '-')) YYERROR; }
	| scalar '&' scalar		{ if (!scalar_op(&$$, &$1, &$3, '&')) YYERROR; }
	| scalar '|' scalar		{ if (!scalar_op(&$$, &$1, &$3, '|')) YYERROR; }
	| scalar '^' scalar		{ if (!scalar_op(&$$, &$1, &$3, '^')) YYERROR; }
	| scalar EVAL_POW scalar	{ if (!scalar_op(&$$, &$1, &$3, EVAL_POW)) YYERROR; }
	| scalar EVAL_SHL scalar	{ if (!scalar_op(&$$, &$1, &$3, EVAL_SHL)) YYERROR; }
	| scalar EVAL_SHR scalar	{ if (!scalar_op(&$$, &$1, &$3, EVAL_SHR)) YYERROR; }
	| scalar EVAL_EQ scalar		{ if (!scalar_op(&$$, &$1, &$3, EVAL_EQ)) YYERROR; }
	| scalar EVAL_NE scalar		{ if (!scalar_op(&$$, &$1, &$3, EVAL_NE)) YYERROR; }
	| scalar EVAL_GT scalar		{ if (!scalar_op(&$$, &$1, &$3, EVAL_GT)) YYERROR; }
	| scalar EVAL_GE scalar		{ if (!scalar_op(&$$, &$1, &$3, EVAL_GE)) YYERROR; }
	| scalar EVAL_LT scalar		{ if (!scalar_op(&$$, &$1, &$3, EVAL_LT)) YYERROR; }
	| scalar EVAL_LE scalar		{ if (!scalar_op(&$$, &$1, &$3, EVAL_LE)) YYERROR; }
	| scalar EVAL_LAND scalar	{ if (!scalar_op(&$$, &$1, &$3, EVAL_LAND)) YYERROR; }
	| scalar EVAL_LXOR scalar	{ if (!scalar_op(&$$, &$1, &$3, EVAL_LXOR)) YYERROR; }
	| scalar EVAL_LOR scalar	{ if (!scalar_op(&$$, &$1, &$3, EVAL_LOR)) YYERROR; }
	| scalar EVAL_STR_EQ scalar	{ scalar_strop(&$$, &$1, &$3, EVAL_STR_EQ); }
	| scalar EVAL_STR_NE scalar	{ scalar_strop(&$$, &$1, &$3, EVAL_STR_NE); }
	| scalar EVAL_STR_GT scalar	{ scalar_strop(&$$, &$1, &$3, EVAL_STR_GT); }
	| scalar EVAL_STR_GE scalar	{ scalar_strop(&$$, &$1, &$3, EVAL_STR_GE); }
	| scalar EVAL_STR_LT scalar	{ scalar_strop(&$$, &$1, &$3, EVAL_STR_LT); }
	| scalar EVAL_STR_LE scalar	{ scalar_strop(&$$, &$1, &$3, EVAL_STR_LE); }
	| '-' scalar %prec NEG		{ scalar_negset(&$$, &$2); }
	| '~' scalar 			{ scalar_notset(&$$, &$2); }
	| '!' scalar 			{ scalar_lnotset(&$$, &$2); }
	| '(' scalar ')'			{ $$ = $2; }
	| scalar '?' scalar ':' scalar	{ scalar_miniif(&$$, &$1, &$3, &$5); }
	| scalar '.' scalar
		{
			scalar_concat(&$$, &$1, &$3);
			scalar_destroy(&$1);
			scalar_destroy(&$3);
		}
	| identifier '(' scalarlist_or_null ')'	
		{
			int r=evalfunc(&$$, $1, &$3);
			scalarlist_destroy(&$3);
			free($1);
			if (!r) YYERROR;
		}
	| identifier			
		{
			int r=evalsymbol(&$$, $1);
			free($1);
			if (!r) YYERROR;
		}
;

scalarlist_or_null:	/* empty */
		{
			eval_scalarlist s;
			s.count=0;
			s.scalars=NULL;
			$$ = s;
		}
	| scalarlist			{ $$ = $1; }
;

scalarlist: scalar			{ scalarlist_set(&$$, &$1); }
	| scalarlist ',' scalarlist
		{
			scalarlist_concat(&$$, &$1, &$3);
			scalarlist_destroy_gentle(&$1);
			scalarlist_destroy_gentle(&$3);
		}
;

identifier: EVAL_IDENT			{ $$ = $1; }
;	

%%
