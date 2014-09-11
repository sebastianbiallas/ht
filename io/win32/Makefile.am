AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = ${my_CPPFLAGS} -I$(top_srcdir) -I$(top_srcdir)/io
AM_CFLAGS   = ${my_CFLAGS}
AM_CXXFLAGS = ${my_CXXFLAGS}

noinst_LIBRARIES = libhtio.a
libhtio_a_SOURCES = syskeyb.cc sysfile.cc sysclipboard.cc sysdisplay.cc types.h
