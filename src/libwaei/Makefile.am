
SUBDIRS = include/libwaei

LIBRARY_VERSION = 2:0:0
ACLOCAL_AMFLAGS = -I m4
datadir = @datadir@
DEFINITIONS =-DDATADIR2=\"$(datadir)\" -DGWAEI_LOCALEDIR=\"$(GWAEI_LOCALEDIR)\" 

lib_LTLIBRARIES =libwaei.la
libwaei_la_SOURCES =libwaei.c dictionary.c dictionary-installer.c dictionary-callbacks.c edictionary.c kanjidictionary.c exampledictionary.c unknowndictionary.c dictionarylist.c query.c range.c utilities.c io.c regex.c search.c history.c result.c preferences.c vocabulary.c word.c
libwaei_la_LDFLAGS = -no-undefined -version-info $(LIBRARY_VERSION)  $(LIBWAEI_LIBS) $(MECAB_LIBS)
libwaei_la_CPPFLAGS = -I$(top_srcdir)/src/libwaei/include $(LIBWAEI_CFLAGS) $(DEFINITIONS) 

if WITH_MECAB
MECAB_DEFS =-DWITH_MECAB
libwaei_la_SOURCES += morphology.c
libwaei_la_CPPFLAGS +=$(MECAB_DEFS)
endif

if OS_MINGW
libwaei_la_LDFLAGS +=-Wl,-subsystem,windows 
MINGW_DEFS =-DCURL_STATICLIB 
libwaei_la_LDFLAGS +=$(MINGW_LIBS)
libwaei_la_CPPFLAGS +=$(MINGW_CFLAGS) $(MINGW_DEFS)
endif


