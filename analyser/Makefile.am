AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libanalyser.a

libanalyser_a_SOURCES = analy.cc analy.h analy_alpha.cc analy_alpha.h \
analy_names.cc analy_names.h analy_x86.cc analy_x86.h analyinfo.h \
codeanaly.cc codeanaly.h dataanaly.cc dataanaly.h \
language.cc language.h pe_analy.cc pe_analy.h stddata.cc stddata.h TODO \
elf_analy.cc elf_analy.h analy_register.cc analy_register.h \
coff_analy.cc coff_analy.h ne_analy.cc ne_analy.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/asm -I$(top_srcdir)/io/@IO_DIR@ -I$(top_srcdir)/io -I$(top_srcdir)/output

