
AM_CPPFLAGS = ${my_CPPFLAGS} -I$(srcdir) -I$(top_srcdir)
AM_CFLAGS   = ${my_CFLAGS}
AM_CXXFLAGS = ${my_CXXFLAGS}
AM_YFLAGS   = -d
AM_LFLAGS   =

AUTOMAKE_OPTIONS=foreign

noinst_LIBRARIES = libhteval.a
libhteval_a_SOURCES = evalparse.y evalparse.h eval.cc eval.h lex.l lex.h evalx.c evalx.h evaltype.h

#bin_PROGRAMS = testeval
#testeval_SOURCES = testeval.c
#testeval_LDADD = libhteval.a
#testeval_LDFLAGS = -lm
