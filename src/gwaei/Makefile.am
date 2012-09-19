SUBDIRS = include/gwaei help mingw

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
DEFINITIONS =-DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DGWAEI_LOCALEDIR=\"$(GWAEI_LOCALEDIR)\" -Wall 

gwaei_SOURCES = gwaei.c application.c application-callbacks.c window.c window-callbacks.c dictionarystore.c dictionarystore-callbacks.c searchwindow.c searchwindow-callbacks.c searchwindow-output.c search-data.c printing.c radicalswindow.c radicalswindow-callbacks.c kanjipadwindow-callbacks.c kanjipad-drawingarea.c kanjipad-candidatearea.c  kanjipadwindow.c settingswindow.c settingswindow-callbacks.c dictionaryinstallwindow.c dictionaryinstallwindow-callbacks.c  installprogresswindow.c installprogresswindow-callbacks.c vocabularywindow.c vocabularywindow-callbacks.c vocabularywordstore.c vocabularyliststore.c addvocabularywindow.c addvocabularywindow-callbacks.c flashcardwindow.c flashcardwindow-callbacks.c flashcardstore.c texttagtable.c

gwaei_LDADD =  $(GWAEI_LIBS) ../libwaei/libwaei.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GWAEI_CFLAGS) $(GWAEI_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/gwaei/include

if WITH_HUNSPELL
gwaei_LDADD +=$(HUNSPELL_LIBS)
gwaei_CPPFLAGS +=$(HUNSPELL_CFLAGS) -DHUNSPELL_MYSPELL_DICTIONARY_PATH=\"$(HUNSPELL_MYSPELL_DICTIONARY_PATH)\" -DWITH_HUNSPELL
gwaei_SOURCES += spellcheck.c spellcheck-callbacks.c
endif


if WITH_MECAB
MECAB_DEFS =-DWITH_MECAB
gwaei_CPPFLAGS +=$(MECAB_DEFS)
endif

if OS_CYGWIN
gwaei_CPPFLAGS +=-DOS_CYGWIN
endif
if OS_MINGW
gwaei_LDFLAGS =-Wl,-subsystem,windows 
MINGW_DEFS =-DCURL_STATICLIB -DOS_MINGW
gwaei_LDADD +=$(MINGW_LIBS) mingw/gwaei.res
gwaei_CPPFLAGS +=$(MINGW_CFLAGS) $(MINGW_DEFS)
endif

pkgdata_DATA = addvocabularywindow.ui dictionaryinstallwindow.ui installprogresswindow.ui kanjipadwindow.ui radicalswindow.ui searchwindow.ui settingswindow.ui vocabularywindow.ui flashcardwindow.ui
EXTRA_DIST = addvocabularywindow.ui dictionaryinstallwindow.ui installprogresswindow.ui kanjipadwindow.ui radicalswindow.ui searchwindow.ui settingswindow.ui vocabularywindow.ui flashcardwindow.ui

