AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = ${my_CPPFLAGS} -I$(top_srcdir) -I$(top_srcdir)/io \
              ${curses_CPPFLAGS}
AM_CFLAGS   = ${my_CFLAGS}
AM_CXXFLAGS = ${my_CXXFLAGS}

noinst_LIBRARIES = libhtio.a
libhtio_a_SOURCES = event.cc sysfile.cc sysdisplay.cc syskeyb.cc sysinit.cc \
sysclipboard.cc types.h
#htcurses.cc htcurses.h htkeyb.cc htkeyb.h htsys.cc htsys.h
