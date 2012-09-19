## Process this file with automake to produce Makefile.in

noinst_HEADERS = gettext.h private.h

SUBDIRS = libwaei schemas 

if !OS_MINGW
SUBDIRS += waei
endif

if WITH_GNOME
SUBDIRS += gwaei kpengine desktop images
endif

