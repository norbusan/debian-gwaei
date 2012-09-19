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
ARCH = @ARCH@

DPKG_DEB=@DPKG_DEB@
RPMBUILD=@RPMBUILD@
RPM_SOURCE=`@RPMDIR@ --eval '%{_sourcedir}'`
RPM_RPMS=`@RPMDIR@ --eval '%{_rpmdir}'`

news_DATA = NEWS
copying_DATA = COPYING
changelog_DATA = ChangeLog
newsdir = $(docdir)
copyingdir = $(docdir)
changelogdir = $(docdir)

CONFIGURE_OPTIONS = --prefix $(prefix) --bindir=$(bindir) --sbindir=$(sbindir) --libexecdir=$(libexecdir) --sysconfdir=$(sysconfdir) --sharedstatedir=$(sharedstatedir) --localstatedir=$(localstatedir) --libdir=$(libdir) --includedir=$(includedir) --oldincludedir=$(oldincludedir) --datarootdir=$(datarootdir) --datadir=$(datadir) --infodir=$(infodir) --localedir=$(localedir) --mandir=$(mandir) --docdir=$(docdir)

distclean-local:
##	@ rm -rf $(top_srcdir)/debian

deb: dist
	dpkg-buildpackage -us -uc -rfakeroot -b
	@ echo "Success! Finished creating the deb package.  Please have a sugary day."

ubuntu-deb:
	@ $(top_srcdir)/configure --prefix /usr --sysconfdir /etc --docdir=/usr/share/doc/@PACKAGE@ --disable-schemas-install  > /dev/null
	@ make deb
  

fedora-rpm: dist
	@ make clean > /dev/null
	@ $(top_srcdir)/rpm/create_rpms.sh `pwd` @PACKAGE@ @VERSION@

uninstall-local:
	@ rm -rf $(DESTDIR)$(datadir)/@PACKAGE@



##DISTCLEANFILES = gnome-doc-utils.make
DISTCLEANFILES = intltool-extract intltool-merge intltool-update po/.intltool-merge-cache
EXTRA_DIST = m4/ChangeLog  config.rpath m4/ChangeLog config.rpath m4/ChangeLog mandir/gwaei.1 mandir/waei.1 autogen.sh rpm/create_rpms.sh $(INTLTOOL_FILES)


