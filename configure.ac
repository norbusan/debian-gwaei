AC_INIT(gwaei, 3.2.0b1)
AC_CONFIG_SRCDIR([src/libwaei/engine.c])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

RELEASE=1
AC_SUBST(RELEASE)

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

##General Dependencies
GLIB_REQUIRED_VERSION=2.30.0
GIO_REQUIRED_VERSION=2.30.0
GTHREAD_REQUIRED_VERSION=2.30.0
LIBCURL_REQUIRED_VERSION=7.20.0
GMODULE_EXPORT_REQUIRED_VERSION=2.30.0
#GTK Base Dependencies
GTK3_REQUIRED_VERSION=3.2.0
GDU_REQUIRED_VERSION=0.13.0

AC_CANONICAL_HOST
AM_CONDITIONAL([OS_MINGW],false)
case $host_os in
  *mingw*)
  AC_DEFINE([OS_MINGW],[1],[Mingw Host])
  AM_CONDITIONAL([OS_MINGW],true)
  OS_MINGW=1
  ;;
esac

#Custom arguments
AC_ARG_WITH([gnome],
[  --without-gnome         turn off the gnome gui interface],
[case "${withval}" in
yes) gnome=true ;;
no)  gnome=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-gnome]) ;;
esac],[gnome=true])
AM_CONDITIONAL([WITH_GNOME], [test x$gnome = xtrue || test x$win32 = xtrue])

AC_CHECK_LIB(m, sqrt)

GNOME_DOC_INIT(,,[:]) 

PKG_CHECK_MODULES(LIBWAEI, glib-2.0           >= $GLIB_REQUIRED_VERSION
                        gobject-2.0        >= $GLIB_REQUIRED_VERSION
                        gio-2.0            >= $GIO_REQUIRED_VERSION
                        gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                        gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       
                        libcurl            >= $LIBCURL_REQUIRED_VERSION       )
AC_SUBST(LIBWAEI_CFLAGS)
AC_SUBST(LIBWAEI_LIBS)

PKG_CHECK_MODULES(WAEI, glib-2.0           >= $GLIB_REQUIRED_VERSION
                        gobject-2.0        >= $GLIB_REQUIRED_VERSION
                        gio-2.0            >= $GIO_REQUIRED_VERSION
                        gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                        gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       )
AC_SUBST(WAEI_CFLAGS)
AC_SUBST(WAEI_LIBS)

if test $gnome = true; then
  PKG_CHECK_MODULES(GWAEI, gtk+-3.0           >= $GTK3_REQUIRED_VERSION
                           glib-2.0           >= $GLIB_REQUIRED_VERSION
                           gobject-2.0        >= $GLIB_REQUIRED_VERSION
                           gio-2.0            >= $GIO_REQUIRED_VERSION
                           gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                           gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       
                           libcurl            >= $LIBCURL_REQUIRED_VERSION
                           gnome-doc-utils    >= $GDU_REQUIRED_VERSION           )
  AC_SUBST(GWAEI_CFLAGS)
  AC_SUBST(GWAEI_LIBS)
  GNOME_DOC_INIT
fi


GLIB_GSETTINGS

AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(ENCHANT, enchant)
if test x$win32 = xtrue; then
  GZIP="gzip.exe"
  AC_SUBST(GZIP)
  UNZIP="unzip.exe"
  AC_SUBST(UNZIP)
else
AC_PATH_PROG(GZIP, gzip)
AC_PATH_PROG(UNZIP, unzip)
fi

AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_HEADER_STDC

AC_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=gwaei
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

GWAEI_LOCALEDIR=[${datadir}/locale]
AC_SUBST(GWAEI_LOCALEDIR)

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile src/Makefile src/libwaei/Makefile src/libwaei/include/libwaei/Makefile src/waei/Makefile src/waei/include/waei/Makefile src/gwaei/Makefile src/gwaei/include/gwaei/Makefile mandir/Makefile src/gwaei/help/Makefile src/gwaei/help/gwaei.omf src/gwaei/help/C/gwaei.xml src/desktop/Makefile src/images/Makefile src/schemas/Makefile rpm/gwaei.spec rpm/fedora/SPECS/gwaei.spec po/Makefile.in src/kpengine/Makefile] src/libwaei/doxyfile src/waei/doxyfile src/gwaei/doxyfile)

AC_OUTPUT


echo ""

if test $gnome = true || test $win32 = true; then
  echo -e "* Gnome is\tENABLED"
else
  echo -e "* Gnome is\tDISABLED"
fi

echo ""
echo "If these aren't the options you wanted, please rerun the configure script."


