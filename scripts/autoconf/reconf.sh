#!/bin/sh

cd ../..
rm -f config.cache

aclocal -I m4
autoconf 
#autoheader
automake -a -c
exit

