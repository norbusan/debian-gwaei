#!/bin/sh

gettextize --force \
&& intltoolize --copy --force --automake \
&& gnome-doc-prepare --copy --force \
&& aclocal -I m4 --force \
&& autoheader -f \
&& automake --copy --force --add-missing \
&& autoconf --force

#this line should go after gettextize
