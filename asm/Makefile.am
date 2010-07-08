AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libasm.a

libasm_a_SOURCES = alphadis.cc alphadis.h alphaopc.cc alphaopc.h\
asm.cc asm.h x86asm.cc x86asm.h x86dis.cc x86dis.h x86opc.cc x86opc.h\
javadis.cc javadis.h javaopc.cc javaopc.h ildis.cc ildis.h \
ilopc.cc ilopc.h ia64dis.cc ia64dis.h ia64opc.cc ia64opc.h \
ppcopc.cc ppcopc.h ppcdis.cc ppcdis.h armdis.cc armdis.h \
arm-dis.cc dis-asm.h arm.h avrdis.cc avrdis.h avropc.cc avropc.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/io/@IO_DIR@ -I$(top_srcdir)/io
