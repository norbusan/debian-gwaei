## Process this file with automake to produce Makefile.in

SUBDIRS = src po mandir

if WITH_GNOME
SUBDIRS += src/schemas src/gtk-builder-xml src/kpengine src/images src/desktop help
endif

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
  

##rpm: dist
##	@ make clean > /dev/null
##	@ touch $(HOME)/.rpmmacros
##	@ cp -f $(HOME)/.rpmmacros $(HOME)/.rpmmacros.backup
##	@ echo "%_topdir $(PWD)/rpm" > $(HOME)/.rpmmacros
##	@ mkdir -p $(top_srcdir)/rpm/SOURCES $(top_srcdir)/rpm/SPECS $(top_srcdir)/rpm/BUILD $(top_srcdir)/rpm/RPMS/@ARCH@ $(top_srcdir)/rpm/SRPMS/@ARCH@
##	@ cp @PACKAGE@-@VERSION@.tar.gz $(top_srcdir)/rpm/SOURCES
##	@ ${RPMBUILD} -ba --target @ARCH@ $(top_srcdir)/rpm/@PACKAGE@.spec
##	@ cp $(RPM_RPMS)/@ARCH@/@PACKAGE@-@VERSION@-@RELEASE@.@ARCH@.rpm $(top_srcdir)
##	@ rm -rf $(top_srcdir)/rpm/SOURCES $(top_srcdir)/rpm/SPECS $(top_srcdir)/rpm/BUILD $(top_srcdir)/rpm/RPMS $(top_srcdir)/rpm/SRPMS
##	@ rm -f $(HOME)/.rpmmacros
##	@ mv -f $(HOME)/.rpmmacros.backup $(HOME)/.rpmmacros
##	@ echo "Success! Finished creating the rpm packge. Please have a sugary day."

fedora-rpm: dist
	@ make clean > /dev/null
	@ $(top_srcdir)/rpm/create_rpms.sh `pwd` @PACKAGE@ @VERSION@

uninstall-local:
	@ rm -rf $(DESTDIR)$(datadir)/@PACKAGE@


##DISTCLEANFILES = gnome-doc-utils.make
EXTRA_DIST = m4/ChangeLog  config.rpath m4/ChangeLog config.rpath m4/ChangeLog help/Makefile.am.gnome help/Makefile.am.none mandir/gwaei.1 mandir/waei.1 autogen.sh rpm/create_rpms.sh


