## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
BINDIR  = @bindir@
DATADIR2 = @datadir@

LIBDIR  = $(DATADIR2)/$(PACKAGE)
PERL  = @PERL@
KPGENDINEDIR = $(top_srcdir)/src/kpengine

libdir_PROGRAMS = kpengine
libdirdir = $(libdir)/$(PACKAGE)
pkgdata_DATA = jdata.dat

DEFS = -DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DFOR_PILOT_COMPAT -DKP_LIBDIR=\"$(LIBDIR)\" -DBINDIR=\"$(LIBDIR)\"


AM_CFLAGS = $(BASE_CFLAGS) 
INCLUDES =  -I jstroke


kpengine_SOURCES = jstroke/scoring.c jstroke/util.c kpengine.c jstroke/jstroke.h
kpengine_LDADD =  $(BASE_LIBS) 


if ENABLE_WIN32
WINLIBPREFIX = /mingw
WIN32_REGEX_LIB = -L$(WINLIBPREFIX)/lib -lregex
WIN32_DEFS = -DWITH_GTK -DENABLE_WIN32 -DLC_MESSAGES=\"C\" -DCURL_STATICLIB
INCLUDES += $(WIN32_CFLAGS) $(WIN32_DEFS) -I$(WINLIBPREFIX)/include
kpengine_LDADD += $(WIN32_LIBS) $(WIN32_REGEX_LIB)
kpengine_LDFLAGS = -Wl,-subsystem,windows 
endif


## Some special rules are needed for jdata.dat installation
clean-local: clean-jdata
clean-jdata:
	rm -f jdata.dat
jdata.dat: jstroke/strokedata.h conv_jdata.pl
	$(PERL) $(KPGENDINEDIR)/conv_jdata.pl < $(KPGENDINEDIR)/jstroke/strokedata.h > jdata.dat
uninstall-local:
	@ rm -rf $(libdirdir)

EXTRA_DIST = conv_jdata.pl jstroke/jstroke.h jstroke/memowrite.h jstroke/strokedata.h jstroke/jstrokerc.h jstroke/pilotcompat.h
