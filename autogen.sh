#! /bin/sh

gnome-doc-prepare --copy --force \
&& aclocal --force \
&& autoheader -f \
&& automake --copy --force --add-missing \
&& autoconf --force
