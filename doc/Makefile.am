AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = hthelp.info ht.info ht.html ht.texi

all: hthelp.info ht.info ht.html README

hthelp.info: ht.texi
	${MAKEINFO} --fill-column=64 --output $@ ht.texi

ht.info: ht.texi
	${MAKEINFO} --output $@ ht.texi

ht.html: ht.texi
	${MAKEINFO} --html --output $@ ht.texi

README: ht.texi
	${MAKEINFO} --no-headers --no-ifinfo --no-iftex --output $@ ht.texi
	cp README ..

clean:
	rm README hthelp.info
