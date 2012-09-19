SUBDIRS = include/waei 

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = waei

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE

waei_SOURCES = waei.c console.c
waei_LDADD =  $(BASE_LIBS) ../libwaei/libwaei.la
waei_CPPFLAGS = $(DEFINITIONS) $(BASE_CFLAGS) $(BASE_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/waei/include


##WINDOWS SETUP
if ENABLE_WIN32
WIN_PREFIX = /mingw/usr
WIN32_DEFS = -DENABLE_WIN32 -DLC_MESSAGES=\"C\" -DCURL_STATICLIB 
waei_LDADD += $(WIN32_LIBS)
waei_CPPFLAGS += $(WIN32_CFLAGS) $(WIN32_DEFS) -I$(WIN_PREFIX)/include
endif

##LINUX/UNIX/BSD SETUP
if WITH_NCURSES
NCURSES_DEFS =-DWITH_NCURSES
waei_SOURCES += ncurses.c
waei_LDADD += $(NCURSES_LIBS)
waei_CPPFLAGS +=$(NCURSES_DEFS)
endif

