AUTOMAKE_OPTIONS=foreign

if USE_POSIX_DIR
MY_IO_DIR=posix
endif

if USE_DJGPP_DIR
MY_IO_DIR=djgpp
endif

if USE_WIN32_DIR
MY_IO_DIR=win32
endif

noinst_LIBRARIES = libcomio.a
libcomio_a_SOURCES = bounds.h display.cc display.h keyb.cc keyb.h sys.cc sys.h \
file.cc file.h fileofs.h types.h event.h 

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(srcdir)/$(MY_IO_DIR)

SUBDIRS = $(MY_IO_DIR)
EXTRA_SUBDIRS = djgpp posix win32
