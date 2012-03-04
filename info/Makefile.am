AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = ${my_CPPFLAGS} -I$(top_srcdir) -I$(top_srcdir)/io/${IO_DIR} \
              -I$(top_srcdir)/io -I$(top_srcdir)/eval
AM_CFLAGS   = ${my_CFLAGS}
AM_CXXFLAGS = ${my_CXXFLAGS}

noinst_LIBRARIES = libinfo.a

libinfo_a_SOURCES = infoview.cc infoview.h
