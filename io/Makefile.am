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
libcomio_a_SOURCES = htio.cc htio.h
INCLUDES = -I. -I.. -I$(MY_IO_DIR)

SUBDIRS = $(MY_IO_DIR)
EXTRA_SUBDIRS = djgpp posix win32