## Process this file with automake to produce Makefile.in

SUBDIRS = src po mandir

INTLTOOL_FILES = intltool-extract.in intltool-merge.in intltool-update.in

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool


ACLOCAL_AMFLAGS = -I m4

PACKAGE = @PACKAGE@
VERSION = @VERSION@
top_srcdir = @top_srcdir@

news_DATA = NEWS
copying_DATA = COPYING
changelog_DATA = ChangeLog
newsdir = $(docdir)
copyingdir = $(docdir)
changelogdir = $(docdir)


##DISTCLEANFILES = gnome-doc-utils.make
DISTCLEANFILES = intltool-extract intltool-merge intltool-update po/.intltool-merge-cache
EXTRA_DIST = config.rpath  config.rpath  config.rpath  config.rpath  m4/ChangeLog  config.rpath m4/ChangeLog config.rpath m4/ChangeLog mandir/gwaei.1 mandir/waei.1 autogen.sh rpm/create_rpms.sh $(INTLTOOL_FILES)


