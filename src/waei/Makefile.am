SUBDIRS = include/waei 

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = waei

datadir = @datadir@
DEFINITIONS =-DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DGWAEI_LOCALEDIR=\"$(GWAEI_LOCALEDIR)\" -DG_SEAL_ENABLE -Wall 

waei_SOURCES = waei.c application.c search-data.c console.c console-output.c console-callbacks.c
waei_LDADD =  $(WAEI_LIBS) ../libwaei/libwaei.la
waei_CPPFLAGS = $(DEFINITIONS) $(WAEI_CFLAGS) $(WAEI_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/waei/include


if OS_MINGW
MINGW_DEFS =-DCURL_STATICLIB 
waei_LDADD +=$(MINGW_LIBS)
waei_CPPFLAGS +=$(MINGW_CFLAGS) $(MINGW_DEFS)
endif


