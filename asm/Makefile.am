AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libasm.a

libasm_a_SOURCES = alphadis.cc alphadis.h alphaopc.cc alphaopc.h\
asm.cc asm.h x86asm.cc x86asm.h x86dis.cc x86dis.h x86opc.cc x86opc.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/io/@IO_DIR@ -I$(top_srcdir)/io
