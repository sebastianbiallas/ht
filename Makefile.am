AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS	= ht

ANALYSER_DIR	= analyser
ASM_DIR		= asm
INFO_DIR	= info
COMIO_DIR	= io
IO_DIR		= io/@IO_DIR@
OUTPUT_DIR	= output
EVAL_DIR	= eval
#SRT_DIR		= srt
EXTRA_DIST	= TODO ChangeLog README KNOWNBUGS INSTALL COPYING AUTHORS
SUBDIRS		= tools analyser asm doc info io output eval

INCLUDES	= -I$(srcdir)/$(ANALYSER_DIR) -I$(srcdir)/$(ASM_DIR) -I$(srcdir)/$(INFO_DIR) -I$(srcdir)/$(IO_DIR) -I$(srcdir)/$(COMIO_DIR) -I$(srcdir)/$(OUTPUT_DIR) -I$(srcdir)/$(EVAL_DIR) -I$(srcdir)

ht_LDADD	= $(ANALYSER_DIR)/libanalyser.a $(ASM_DIR)/libasm.a $(INFO_DIR)/libinfo.a $(IO_DIR)/libhtio.a $(OUTPUT_DIR)/liboutput.a $(COMIO_DIR)/libcomio.a $(EVAL_DIR)/libhteval.a @HT_LIBS@ @LZO_LIBS@
ht_LDFLAGS	= @HT_LDFLAGS@

if ENABLE_INCLUDED_LZO
SUBDIRS += minilzo
ht_LDADD += minilzo/liblzo.a
endif

ht_SOURCES	= atom.cc atom.h except.cc except.h data.cc data.h \
str.cc str.h strtools.cc strtools.h endianess.h endianess.cc \
htdoc.c htdoc.h blockop.cc blockop.h coff_s.h \
config.h cstream.cc cstream.h demangle.h elfstruc.h \
formats.cc formats.h htanaly.cc htanaly.h htapp.cc \
htapp.h htcfg.cc htcfg.h htclipboard.cc htclipboard.h \
htcoff.cc htcoff.h htcoffhd.cc htcoffhd.h htctrl.cc htctrl.h \
htdebug.cc htdebug.h htdialog.cc htdialog.h htelf.cc htelf.h htelfhd.cc \
htelfhd.h htelfimg.cc htelfimg.h htelfphs.cc htelfphs.h htelfshs.cc htelfshs.h \
htelfsym.cc htelfsym.h htelfrel.cc htelfrel.h htfinfo.cc htfinfo.h htformat.cc \
htformat.h hthex.cc hthex.h hthist.cc hthist.h htidle.cc \
htidle.h htinfo.h htiobox.cc htiobox.h htle.cc htle.h htleent.cc \
htleent.h htlehead.cc htlehead.h htleimg.cc htleimg.h htleobj.cc htleobj.h \
htlepage.cc htlepage.h htmenu.cc htmenu.h htmz.cc htmz.h \
htmzhead.cc htmzhead.h htmzimg.cc htmzimg.h htmzrel.cc htmzrel.h htne.cc \
htne.h htneent.cc htneent.h htnehead.cc htnehead.h htnenms.cc htnenms.h \
htneobj.cc htneobj.h htnewexe.cc htnewexe.h htobj.cc htobj.h htpal.cc \
htpal.h htpe.cc htpe.h htpedimp.cc htpedimp.h htpeexp.cc htpeexp.h \
htpehead.cc htpehead.h htpeimg.cc htpeimg.h htpeimp.cc htpeimp.h htperes.cc \
htperes.h htreg.cc htreg.h htsearch.cc htsearch.h \
httag.cc httag.h httree.cc \
httree.h lestruct.h main.cc mzstruct.h nestruct.h pestruct.h \
store.cc store.h stream.cc stream.h tools.cc tools.h vxd.cc vxd.h \
vxdserv.cc vxdserv.h cplus-dem.c demangle.h regex.c regex.h \
syntax.cc syntax.h textfile.cc textfile.h textedit.cc textedit.h \
classread.cc classview.cc class.h httext.cc httext.h hteval.cc hteval.h \
relfile.cc relfile.h htprocess.cc htprocess.h mfile.cc mfile.h elfstruc.cc \
pestruct.cc coff_s.cc mzstruct.cc defreg.h defreg.c htdisasm.cc htdisasm.h \
htcoffimg.cc htcoffimg.h nestruct.cc htneimg.cc htneimg.h htneimp.cc htneimp.h \
cmds.cc cmds.h snprintf.cc snprintf.h htpeil.cc htpeil.h \
ilstruct.cc ilstruct.h log.cc log.h classimg.cc classimg.h \
vfs.cc vfs.h vfsview.cc vfsview.h htlevxd.cc htlevxd.h lestruct.cc \
htmacho.cc htmacho.h htmachohd.cc htmachohd.h machostruc.cc machostruc.h \
htmachoimg.cc htmachoimg.h fltstruc.h fltstruc.cc htflt.cc htflt.h htflthd.cc \
htflthd.h htfltimg.cc htfltimg.h xbestruct.cc xbestruct.h htxbehead.cc \
htxbehead.h htxbe.cc htxbe.h htxbeimg.cc htxbeimg.h htxbeimp.cc htxbeimp.h \
pefstruc.cc pefstruc.h htpef.cc htpef.h htpefhd.cc htpefhd.h htpefimg.cc htpefimg.h \
htpefimp.cc htpefimp.h htxex.cc htxex.h htxexhead.cc htxexhead.h xexstruct.cc \
cp-demangle.c cp-demangle.h xexstruct.h htxeximg.cc htxeximg.h safe-ctype.h

dist2: distdir
	$(AMTAR) chof - $(distdir) | BZIP2=$(BZIP2_ENV) bzip2 -c >$(distdir).tar.bz2
	$(am__remove_distdir)

dist-all2: dist2

htdoc.c htdoc.h: $(srcdir)/doc/hthelp.info tools/bin2c
	tools/bin2c -Nhtinfo $(srcdir)/doc/hthelp.info htdoc.c htdoc.h
