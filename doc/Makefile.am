AUTOMAKE_OPTIONS = foreign

MIFLAGS = --no-split

EXTRA_DIST = hthelp.info ht.info ht.html ht.texi

all: hthelp.info ht.info ht.html README

hthelp.info: ht.texi
	${MAKEINFO} ${MIFLAGS} --fill-column=64 --output $@ ht.texi

ht.info: ht.texi
	${MAKEINFO} ${MIFLAGS} --output $@ ht.texi

ht.html: ht.texi
	${MAKEINFO} ${MIFLAGS} --html --output $@ ht.texi

README: ht.texi
	${MAKEINFO} ${MIFLAGS} --no-headers --no-ifinfo --no-iftex --output $@ ht.texi
	cp README ..

clean:
	rm -f README hthelp.info
