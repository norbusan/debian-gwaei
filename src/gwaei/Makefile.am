SUBDIRS = include/gwaei ui help

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DENCHANT=\"$(ENCHANT)\" -DGWAEI_LOCALEDIR=\"${GWAEI_LOCALEDIR}\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE

gwaei_SOURCES = gwaei.c common.c main.c main-callbacks.c settings-callbacks.c settings.c radsearchtool.c radsearchtool-callbacks.c printing.c main-tabs.c dictionaryinstall.c installprogress.c dictionarymanager.c settings-listeners.c pluginmanager.c spellcheck.c output-callbacks.c kanjipad.c kanjipad-candidatearea.c kanjipad-callbacks.c kanjipad-drawingarea.c
gwaei_LDADD =  $(GNOME_LIBS) ../libwaei/libwaei.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GNOME_CFLAGS) $(GNOME_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/gwaei/include 

if ENABLE_WIN32
gwaei_LDFLAGS = -Wl,-subsystem,windows 
WIN_PREFIX = /mingw/usr
WIN32_DEFS = -DENABLE_WIN32 -DLC_MESSAGES=\"C\" -DCURL_STATICLIB 
gwaei_LDADD += $(WIN32_LIBS) 
gwaei_CPPFLAGS += $(WIN32_CFLAGS) $(WIN32_DEFS) -I$(WIN_PREFIX)/include
endif

if WITH_LIBUNIQUE
LIBUNIQUE_DEFS =-DWITH_LIBUNIQUE
gwaei_SOURCES += libunique.c
gwaei_LDADD += $(LIBUNIQUE_LIBS)
gwaei_CPPFLAGS +=$(LIBUNIQUE_CFLAGS)$(LIBUNIQUE_DEFS)
endif


