AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = ${my_CPPFLAGS} -I$(top_srcdir) -I$(top_srcdir)/analyser \
              -I$(top_srcdir)/asm -I$(top_srcdir)/io/${IO_DIR} \
              -I$(top_srcdir)/io -I$(top_srcdir)/output
AM_CFLAGS   = ${my_CFLAGS}
AM_CXXFLAGS = ${my_CXXFLAGS}

noinst_LIBRARIES = liboutput.a

liboutput_a_SOURCES = out_html.cc out_html.h out.cc out.h out_ht.cc out_ht.h \
out_sym.cc out_sym.h out_txt.cc out_txt.h

