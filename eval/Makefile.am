YFLAGS=-d

LFLAGS=

AUTOMAKE_OPTIONS=foreign

noinst_LIBRARIES = libhteval.a
libhteval_a_SOURCES = evalparse.y evalparse.h eval.cc eval.h lex.l lex.h evalx.c evalx.h evaltype.h
INCLUDES = -I$(srcdir) -I$(top_srcdir)

#bin_PROGRAMS = testeval
#testeval_SOURCES = testeval.c
#testeval_LDADD = libhteval.a
#testeval_LDFLAGS = -lm
