
/*  A Bison parser, made from evalparse.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	EVAL_INT	257
#define	EVAL_STR	258
#define	EVAL_FLOAT	259
#define	EVAL_IDENT	260
#define	EVAL_LAND	261
#define	EVAL_LXOR	262
#define	EVAL_LOR	263
#define	EVAL_EQ	264
#define	EVAL_NE	265
#define	EVAL_STR_EQ	266
#define	EVAL_STR_NE	267
#define	EVAL_LT	268
#define	EVAL_LE	269
#define	EVAL_GT	270
#define	EVAL_GE	271
#define	EVAL_STR_LT	272
#define	EVAL_STR_LE	273
#define	EVAL_STR_GT	274
#define	EVAL_STR_GE	275
#define	EVAL_SHL	276
#define	EVAL_SHR	277
#define	NEG	278
#define	EVAL_POW	279

#line 4 "evalparse.y"


#define YYPARSE_PARAM resultptr

#include "evaltype.h"
#include "eval.h"

void yyerror (char *s)
{
	set_eval_error(s);
}


#line 18 "evalparse.y"
typedef union {
	scalar_t scalar;
	char *ident;
	scalarlist_t scalars;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		79
#define	YYFLAG		-32768
#define	YYNTBASE	40

#define YYTRANSLATE(x) ((unsigned)(x) <= 279 ? yytranslate[x] : 45)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    35,    16,     2,    38,
    39,    33,    32,     7,    31,    10,    34,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     9,     2,     2,
     2,     2,     8,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    15,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    14,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
    11,    12,    13,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    36,    37
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    12,    16,    20,    24,    28,
    32,    36,    40,    44,    48,    52,    56,    60,    64,    68,
    72,    76,    80,    84,    88,    92,    96,   100,   104,   108,
   112,   115,   119,   125,   129,   134,   136,   137,   139,   141,
   145
};

static const short yyrhs[] = {    41,
     0,     3,     0,     4,     0,     5,     0,    41,    33,    41,
     0,    41,    34,    41,     0,    41,    35,    41,     0,    41,
    32,    41,     0,    41,    31,    41,     0,    41,    16,    41,
     0,    41,    14,    41,     0,    41,    15,    41,     0,    41,
    37,    41,     0,    41,    29,    41,     0,    41,    30,    41,
     0,    41,    17,    41,     0,    41,    18,    41,     0,    41,
    23,    41,     0,    41,    24,    41,     0,    41,    21,    41,
     0,    41,    22,    41,     0,    41,    11,    41,     0,    41,
    12,    41,     0,    41,    13,    41,     0,    41,    19,    41,
     0,    41,    20,    41,     0,    41,    27,    41,     0,    41,
    28,    41,     0,    41,    25,    41,     0,    41,    26,    41,
     0,    31,    41,     0,    38,    41,    39,     0,    41,     8,
    41,     9,    41,     0,    41,    10,    41,     0,    44,    38,
    42,    39,     0,    44,     0,     0,    43,     0,    41,     0,
    43,     7,    43,     0,     6,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    54,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    95,   102,   110,   117,   119,   120,
   128
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","EVAL_INT",
"EVAL_STR","EVAL_FLOAT","EVAL_IDENT","','","'?'","':'","'.'","EVAL_LAND","EVAL_LXOR",
"EVAL_LOR","'|'","'^'","'&'","EVAL_EQ","EVAL_NE","EVAL_STR_EQ","EVAL_STR_NE",
"EVAL_LT","EVAL_LE","EVAL_GT","EVAL_GE","EVAL_STR_LT","EVAL_STR_LE","EVAL_STR_GT",
"EVAL_STR_GE","EVAL_SHL","EVAL_SHR","'-'","'+'","'*'","'/'","'%'","NEG","EVAL_POW",
"'('","')'","input","scalar","scalarlist_or_null","scalarlist","identifier", NULL
};
#endif

static const short yyr1[] = {     0,
    40,    41,    41,    41,    41,    41,    41,    41,    41,    41,
    41,    41,    41,    41,    41,    41,    41,    41,    41,    41,
    41,    41,    41,    41,    41,    41,    41,    41,    41,    41,
    41,    41,    41,    41,    41,    41,    42,    42,    43,    43,
    44
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     2,     3,     5,     3,     4,     1,     0,     1,     1,     3,
     1
};

static const short yydefact[] = {     0,
     2,     3,     4,    41,     0,     0,     1,    36,    31,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    37,    32,
     0,    34,    22,    23,    24,    11,    12,    10,    16,    17,
    25,    26,    20,    21,    18,    19,    29,    30,    27,    28,
    14,    15,     9,     8,     5,     6,     7,    13,    39,     0,
    38,     0,    35,     0,    33,    40,     0,     0,     0
};

static const short yydefgoto[] = {    77,
    69,    70,    71,     8
};

static const short yypact[] = {    36,
-32768,-32768,-32768,-32768,    36,    36,   125,   -37,   -35,    65,
    36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
    36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
    36,    36,    36,    36,    36,    36,    36,    36,    36,-32768,
    97,   152,    31,   177,   201,   224,   246,   267,   288,   288,
   288,   288,   305,   305,   305,   305,   305,   305,   305,   305,
   314,   314,   319,   319,   -35,   -35,   -35,-32768,   125,   -36,
    -3,    36,-32768,    36,   125,-32768,     7,     8,-32768
};

static const short yypgoto[] = {-32768,
     0,-32768,   -65,-32768
};


#define	YYLAST		356


static const short yytable[] = {     7,
    39,    38,    73,    74,     9,    10,    78,    79,    76,     0,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
    61,    62,    63,    64,    65,    66,    67,    68,     1,     2,
     3,     4,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,     5,    38,     0,     0,
     0,    75,    11,     6,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
     0,    38,     0,    40,    11,    72,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    11,    38,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
     0,    38,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,     0,    38,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,     0,    38,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,     0,    38,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    34,    35,    36,    37,     0,
    38,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,     0,    38,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,     0,    38,-32768,-32768,-32768,-32768,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
    35,    36,    37,     0,    38,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    31,    32,    33,    34,    35,    36,    37,
     0,    38,-32768,-32768,    33,    34,    35,    36,    37,     0,
    38,    35,    36,    37,     0,    38
};

static const short yycheck[] = {     0,
    38,    37,    39,     7,     5,     6,     0,     0,    74,    -1,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,    38,     3,     4,
     5,     6,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    33,    34,    35,    31,    37,    -1,    -1,
    -1,    72,     8,    38,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    -1,    37,    -1,    39,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    34,    35,     8,    37,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    -1,    37,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    34,    35,    -1,    37,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    34,    35,    -1,    37,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    33,    34,    35,    -1,    37,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    -1,
    37,    16,    17,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
    35,    -1,    37,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    34,    35,    -1,    37,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
    33,    34,    35,    -1,    37,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    -1,    37,    29,    30,    31,    32,    33,    34,    35,    -1,
    37,    33,    34,    35,    -1,    37
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 54 "evalparse.y"
{ *(scalar_t*)resultptr=yyvsp[0].scalar; ;
    break;}
case 2:
#line 57 "evalparse.y"
{ yyval.scalar = yyvsp[0].scalar; ;
    break;}
case 3:
#line 58 "evalparse.y"
{ yyval.scalar = yyvsp[0].scalar; ;
    break;}
case 4:
#line 59 "evalparse.y"
{ yyval.scalar = yyvsp[0].scalar; ;
    break;}
case 5:
#line 60 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '*')) YYERROR; ;
    break;}
case 6:
#line 61 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '/')) YYERROR; ;
    break;}
case 7:
#line 62 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '%')) YYERROR; ;
    break;}
case 8:
#line 63 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '+')) YYERROR; ;
    break;}
case 9:
#line 64 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '-')) YYERROR; ;
    break;}
case 10:
#line 65 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '&')) YYERROR; ;
    break;}
case 11:
#line 66 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '|')) YYERROR; ;
    break;}
case 12:
#line 67 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, '^')) YYERROR; ;
    break;}
case 13:
#line 68 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_POW)) YYERROR; ;
    break;}
case 14:
#line 69 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_SHL)) YYERROR; ;
    break;}
case 15:
#line 70 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_SHR)) YYERROR; ;
    break;}
case 16:
#line 71 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_EQ)) YYERROR; ;
    break;}
case 17:
#line 72 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_NE)) YYERROR; ;
    break;}
case 18:
#line 73 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_GT)) YYERROR; ;
    break;}
case 19:
#line 74 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_GE)) YYERROR; ;
    break;}
case 20:
#line 75 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_LT)) YYERROR; ;
    break;}
case 21:
#line 76 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_LE)) YYERROR; ;
    break;}
case 22:
#line 77 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_LAND)) YYERROR; ;
    break;}
case 23:
#line 78 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_LXOR)) YYERROR; ;
    break;}
case 24:
#line 79 "evalparse.y"
{ if (!scalar_op(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_LOR)) YYERROR; ;
    break;}
case 25:
#line 80 "evalparse.y"
{ scalar_strop(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_STR_EQ); ;
    break;}
case 26:
#line 81 "evalparse.y"
{ scalar_strop(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_STR_NE); ;
    break;}
case 27:
#line 82 "evalparse.y"
{ scalar_strop(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_STR_GT); ;
    break;}
case 28:
#line 83 "evalparse.y"
{ scalar_strop(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_STR_GE); ;
    break;}
case 29:
#line 84 "evalparse.y"
{ scalar_strop(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_STR_LT); ;
    break;}
case 30:
#line 85 "evalparse.y"
{ scalar_strop(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar, EVAL_STR_LE); ;
    break;}
case 31:
#line 86 "evalparse.y"
{ scalar_negset(&yyval.scalar, &yyvsp[0].scalar); ;
    break;}
case 32:
#line 87 "evalparse.y"
{ yyval.scalar = yyvsp[-1].scalar; ;
    break;}
case 33:
#line 88 "evalparse.y"
{ scalar_miniif(&yyval.scalar, &yyvsp[-4].scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar); ;
    break;}
case 34:
#line 90 "evalparse.y"
{
			scalar_concat(&yyval.scalar, &yyvsp[-2].scalar, &yyvsp[0].scalar);
			scalar_destroy(&yyvsp[-2].scalar);
			scalar_destroy(&yyvsp[0].scalar);
		;
    break;}
case 35:
#line 96 "evalparse.y"
{
			int r=evalfunc(&yyval.scalar, yyvsp[-3].ident, &yyvsp[-1].scalars);
			scalarlist_destroy(&yyvsp[-1].scalars);
			free(yyvsp[-3].ident);
			if (!r) YYERROR;
		;
    break;}
case 36:
#line 103 "evalparse.y"
{
			int r=evalsymbol(&yyval.scalar, yyvsp[0].ident);
			free(yyvsp[0].ident);
			if (!r) YYERROR;
		;
    break;}
case 37:
#line 111 "evalparse.y"
{
			scalarlist_t s;
			s.count=0;
			s.scalars=NULL;
			yyval.scalars = s;
		;
    break;}
case 38:
#line 117 "evalparse.y"
{ yyval.scalars = yyvsp[0].scalars; ;
    break;}
case 39:
#line 119 "evalparse.y"
{ scalarlist_set(&yyval.scalars, &yyvsp[0].scalar); ;
    break;}
case 40:
#line 121 "evalparse.y"
{
			scalarlist_concat(&yyval.scalars, &yyvsp[-2].scalars, &yyvsp[0].scalars);
			scalarlist_destroy_gentle(&yyvsp[-2].scalars);
			scalarlist_destroy_gentle(&yyvsp[0].scalars);
		;
    break;}
case 41:
#line 128 "evalparse.y"
{ yyval.ident = yyvsp[0].ident; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 131 "evalparse.y"

