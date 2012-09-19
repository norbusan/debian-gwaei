SUBDIRS = include/gwaei help

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
DEFINITIONS =-DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DENCHANT=\"$(ENCHANT)\" -DGWAEI_LOCALEDIR=\"$(GWAEI_LOCALEDIR)\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE -Wall

gwaei_SOURCES = gwaei.c application.c application-callbacks.c window.c dictinfolist.c dictinfolist-callbacks.c searchwindow.c searchwindow-callbacks.c searchwindow-output.c search-data.c spellcheck.c spellcheck-callbacks.c printing.c radicalswindow.c radicalswindow-callbacks.c kanjipadwindow-callbacks.c kanjipad-drawingarea.c kanjipad-candidatearea.c  kanjipadwindow.c settingswindow.c settingswindow-callbacks.c dictionaryinstallwindow.c dictionaryinstallwindow-callbacks.c  installprogresswindow.c installprogresswindow-callbacks.c

gwaei_LDADD =  $(GWAEI_LIBS) ../libwaei/libwaei.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GWAEI_CFLAGS) $(GWAEI_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/gwaei/include

if OS_MINGW
gwaei_LDFLAGS = -Wl,-subsystem,windows 
MINGW_DEFS = -DCURL_STATICLIB 
gwaei_LDADD += $(MINGW_LIBS) 
gwaei_CPPFLAGS += $(MINGW_CFLAGS) $(MINGW_DEFS)
endif

pkgdata_DATA = searchwindow.ui radicalswindow.ui settingswindow.ui dictionaryinstallwindow.ui installprogresswindow.ui kanjipadwindow.ui
EXTRA_DIST =  searchwindow.ui radicalswindow.ui settingswindow.ui dictionaryinstallwindow.ui installprogresswindow.ui kanjipadwindow.ui

