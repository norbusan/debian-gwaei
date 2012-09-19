## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RSYNC = @RSYNC@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei

install-data-local: install-symbolic-link
uninstall-local: uninstall-symbolic-link

install-symbolic-link: install-binPROGRAMS
if ENABLE_WIN32
	rm -f $(DESTDIR)$(bindir)/waei.exe
	cp $(DESTDIR)$(bindir)/gwaei.exe $(DESTDIR)$(bindir)/waei.exe
else
	rm -f $(DESTDIR)$(bindir)/waei
	cp $(DESTDIR)$(bindir)/gwaei $(DESTDIR)$(bindir)/waei
endif
uninstall-symbolic-link:
if ENABLE_WIN32
	rm -f $(DESTDIR)$(bindir)/waei.exe
else
	rm -f $(DESTDIR)$(bindir)/waei
endif

datadir = @datadir@
localedir = $(datadir)/locale
DEPS_DEFS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DRSYNC=\"$(RSYNC)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR=\"$(datadir)\" -DLIBDIR=\"$(libdir)\"
#AM_CFLAGS = --pedantic -std=c99 -O2 -g


gwaei_SOURCES = main.c dictinfo-object.c dictlist-object.c queryline-object.c engine.c utilities.c io.c regex.c console-main-interface.c searchitem-object.c historylist-object.c resultline-object.c
gwaei_SOURCES += include/gwaei/definitions.h include/gwaei/main.h include/gwaei/search-objects.h include/gwaei/dictionary-objects.h include/gwaei/dictinfo-object.h include/gwaei/dictlist-object.h include/gwaei/queryline-object.h include/gwaei/engine.h include/gwaei/utilities.h include/gwaei/io.h include/gwaei/regex.h include/gwaei/console-main-interface.h include/gwaei/search-objects.h include/gwaei/searchitem-object.h include/gwaei/historylist-object.h include/gwaei/resultline-object.h include/gwaei/preferences.h
gwaei_LDADD = $(DEPS_LIBS) @LIBINTL@
##gwaei_LDFLAGS =
AM_CPPFLAGS =$(DEPS_CFLAGS)$(DEPS_DEFS)
INCLUDES = -I $(top_srcdir)/src/include

##WINDOWS SETUP
if ENABLE_WIN32
#EMPTY
endif
##LINUX/UNIX/BSD SETUP
if WITH_GNOME
gwaei_SOURCES += gtk-gconf-preferences.c 
gwaei_SOURCES += gtk-main-callbacks.c include/gwaei/gtk-main-callbacks.h gtk-main-interface.c gtk-settings-callbacks.c include/gwaei/gtk-settings-callbacks.h gtk-settings-interface.c gtk-radicals-interface.c gtk-radicals-callbacks.c include/gwaei/gtk-radicals-callbacks.h gtk-printing.c include/gwaei/gtk-main-interface.h include/gwaei/gtk-settings-interface.h include/gwaei/gtk-radicals-interface.h include/gwaei/gtk-printing.h include/gwaei/gtk.h include/gwaei/gtk-settings-interface-install-line.h gtk-kanjipad-callbacks.c include/gwaei/gtk-kanjipad-callbacks.h  gtk-kanjipad-interface.c include/gwaei/gtk-kanjipad-interface.h gtk-kanjipad-interface-candidatearea.c include/gwaei/gtk-kanjipad-interface-candidatearea.h gtk-kanjipad-interface-drawingarea.c include/gwaei/gtk-kanjipad-interface-drawingarea.h gtk-main-interface-tabs.c include/gwaei/gtk-main-interface-tabs.h gtk-settings-interface-install-line.c
gwaei_LDADD += $(GNOME_LIBS)
GNOME_DEFS =-DWITH_GTK
AM_CPPFLAGS +=$(GNOME_CFLAGS)$(GNOME_DEFS)
else
gwaei_SOURCES += none-preferences.c
endif

if WITH_LIBUNIQUE
LIBUNIQUE_DEFS =-DWITH_LIBUNIQUE
gwaei_SOURCES += gtk-main-interface-unique.c include/gwaei/gtk-main-interface-unique.h
gwaei_LDADD += $(LIBUNIQUE_LIBS)
AM_CPPFLAGS +=$(LIBUNIQUE_CFLAGS)$(LIBUNIQUE_DEFS)
endif

if WITH_LIBSEXY
LIBSEXY_DEFS =-DWITH_LIBSEXY
gwaei_SOURCES += gtk-main-interface-sexy.c include/gwaei/gtk-main-interface-sexy.h
gwaei_LDADD += $(LIBSEXY_LIBS)
AM_CPPFLAGS +=$(LIBSEXY_CFLAGS)$(LIBSEXY_DEFS)
endif

if WITH_NCURSES
NCURSES_DEFS =-DWITH_NCURSES
gwaei_SOURCES += ncurses-main-interface.c include/gwaei/ncurses-main-interface.h
gwaei_LDADD += $(NCURSES_LIBS)
AM_CPPFLAGS +=$(NCURSES_DEFS)
endif

