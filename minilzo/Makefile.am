AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = ${my_CPPFLAGS} -I$(top_srcdir) -I$(top_srcdir)/asm \
              -I$(top_srcdir)/io/@IO_DIR@ -I$(top_srcdir)/io \
              -I$(top_srcdir)/output
AM_CFLAGS   = ${my_CFLAGS}
AM_CXXFLAGS = ${my_CXXFLAGS}

noinst_LIBRARIES = liblzo.a

liblzo_a_SOURCES = lzoconf.h lzodefs.h minilzo.c minilzo.h
