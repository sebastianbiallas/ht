#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef union {
	eval_scalar scalar;
	char *ident;
	eval_scalarlist scalars;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	EVAL_INT	257
# define	EVAL_STR	258
# define	EVAL_FLOAT	259
# define	EVAL_IDENT	260
# define	EVAL_LAND	261
# define	EVAL_LXOR	262
# define	EVAL_LOR	263
# define	EVAL_EQ	264
# define	EVAL_NE	265
# define	EVAL_STR_EQ	266
# define	EVAL_STR_NE	267
# define	EVAL_LT	268
# define	EVAL_LE	269
# define	EVAL_GT	270
# define	EVAL_GE	271
# define	EVAL_STR_LT	272
# define	EVAL_STR_LE	273
# define	EVAL_STR_GT	274
# define	EVAL_STR_GE	275
# define	EVAL_SHL	276
# define	EVAL_SHR	277
# define	NEG	278
# define	EVAL_POW	279


#endif /* not BISON_Y_TAB_H */
