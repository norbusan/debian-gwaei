PACKAGE = @PACKAGE@
VERSION = @VERSION@
datadir = @datadir@


include $(top_srcdir)/gnome-doc-utils.make
dist-hook: doc-dist-hook
DOC_MODULE = gwaei


cglossarydir = $(datadir)/gnome/help/$(PACKAGE)/C
cglossary_DATA = C/glossary.xml
EXTRA_DIST = C/glossary.xml

uninstall-local:
	@ rm -rf $(DESTDIR)$(datadir)/gnome/help/$(PACKAGE)/C/glossary.xml
	@ rm -rf $(DESTDIR)$(datadir)/gnome/help/$(PACKAGE)/C/gwaei.xml
distclean:
	@ rm -rf C/gwaei.xml
	@ rm -rf Makefile
	@ rm -rf gwaei.omf
	@ rm -rf gwaei-C.omf

