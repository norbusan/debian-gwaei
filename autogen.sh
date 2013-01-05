#!/bin/sh

set -e

touch ChangeLog
libtoolize --force
glib-gettextize --copy --force
intltoolize --copy --force --automake
gnome-doc-prepare --copy --force
aclocal -I m4 --force
autoheader -f
automake --copy --force --add-missing
autoconf --force

#$gnome-autogen.sh
