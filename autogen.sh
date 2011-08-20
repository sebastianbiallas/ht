#!/bin/sh
# script to prepare HT sources
aclocal -I . \
&& autoheader \
&& automake --add-missing \
&& autoconf \
|| exit 1

echo HT sources are now prepared. To build here, run:
echo " ./configure"
echo " make ; make htdoc.h ; make"
