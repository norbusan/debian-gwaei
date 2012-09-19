## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@

desktopdir   = $(datadir)/applications
desktop_in_files = gwaei.desktop.in
desktop_DATA   = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

distclean:
	rm -f $(PACKAGE).desktop
	rm -f Makefile

EXTRA_DIST = $(PACKAGE).desktop.in

