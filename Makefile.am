AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS	= ht

ANALYSER_DIR	= analyser
ASM_DIR		= asm
INFO_DIR	= info
COMIO_DIR	= io
IO_DIR		= io/@IO_DIR@
OUTPUT_DIR	= output
EVAL_DIR	= eval
MINILZO_DIR	= minilzo
SRT_DIR		= srt

EXTRA_DIST	= TODO ChangeLog README KNOWNBUGS INSTALL COPYING AUTHORS move-if-change
SUBDIRS		= tools analyser asm doc info io output eval minilzo srt

INCLUDES	= -I$(ANALYSER_DIR) -I$(ASM_DIR) -I$(INFO_DIR) -I$(IO_DIR) -I$(COMIO_DIR) -I$(OUTPUT_DIR) -I$(EVAL_DIR) -I$(MINILZO_DIR) -I$(SRT_DIR) -I.

ht_LDADD	= $(ANALYSER_DIR)/libanalyser.a $(ASM_DIR)/libasm.a $(INFO_DIR)/libinfo.a $(IO_DIR)/libhtio.a $(OUTPUT_DIR)/liboutput.a $(COMIO_DIR)/libcomio.a $(EVAL_DIR)/libhteval.a $(MINILZO_DIR)/liblzo.a $(SRT_DIR)/libsrt.a
ht_LDFLAGS	= @HT_LDFLAGS@
ht_LIBS		= @HT_LIBS@

ht_SOURCES	= htdoc.c htdoc.h blockop.cc blockop.h coff_s.h common.cc common.h \
config.h cstream.cc cstream.h demangle.h elfstruc.h htendian.cc \
htendian.h formats.cc formats.h global.h htanaly.cc htanaly.h htapp.cc \
htapp.h htatom.cc htatom.h htcfg.cc htcfg.h htclipboard.cc htclipboard.h \
htcoff.cc htcoff.h htcoffhd.cc htcoffhd.h htctrl.cc htctrl.h htdata.cc htdata.h \
htdebug.cc htdebug.h htdialog.cc htdialog.h htelf.cc htelf.h htelfhd.cc \
htelfhd.h htelfimg.cc htelfimg.h htelfphs.cc htelfphs.h htelfshs.cc htelfshs.h \
htelfsym.cc htelfsym.h htelfrel.cc htelfrel.h htfinfo.cc htfinfo.h htformat.cc \
htformat.h hthex.cc hthex.h hthist.cc hthist.h htidle.cc \
htidle.h htinfo.cc htinfo.h htiobox.cc htiobox.h htle.cc htle.h htleent.cc \
htleent.h htlehead.cc htlehead.h htleimg.cc htleimg.h htleobj.cc htleobj.h \
htlepage.cc htlepage.h htmenu.cc htmenu.h htmz.cc htmz.h \
htmzhead.cc htmzhead.h htmzimg.cc htmzimg.h htmzrel.cc htmzrel.h htne.cc \
htne.h htneent.cc htneent.h htnehead.cc htnehead.h htnenms.cc htnenms.h \
htneobj.cc htneobj.h htnewexe.cc htnewexe.h htobj.cc htobj.h htpal.cc \
htpal.h htpe.cc htpe.h htpedimp.cc htpedimp.h htpeexp.cc htpeexp.h \
htpehead.cc htpehead.h htpeimg.cc htpeimg.h htpeimp.cc htpeimp.h htperes.cc \
htperes.h htreg.cc htreg.h htsearch.cc htsearch.h \
htstring.cc htstring.h htstruct.cc htstruct.h httag.cc httag.h httree.cc \
httree.h htvfs.cc htvfs.h lestruct.h main.cc mzstruct.h nestruct.h pestruct.h \
regex.h store.cc store.h stream.cc stream.h tools.cc tools.h vxd.cc vxd.h \
vxdserv.cc vxdserv.h cplus-dem.c demangle.h regex.c regex.h \
syntax.cc syntax.h textfile.cc textfile.h textedit.cc textedit.h \
classread.cc classview.cc class.h htexcept.cc htexcept.h httext.cc httext.h hteval.cc hteval.h \
relfile.cc relfile.h process.cc process.h mfile.cc mfile.h elfstruc.cc \
pestruct.cc coff_s.cc mzstruct.cc defreg.h defreg.c htdisasm.cc htdisasm.h \
htcoffimg.cc htcoffimg.h nestruct.cc htneimg.cc htneimg.h htneimp.cc htneimp.h \
cmds.cc cmds.h

dist2: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) choIf $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

dist-all2: dist2

htdoc.c htdoc.h: doc/hthelp.info tools/bin2c
	tools/bin2c -Nhtinfo doc/hthelp.info htdoc.c htdoc.h
