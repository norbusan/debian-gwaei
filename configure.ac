AC_INIT(gwaei, 3.0.2)
AC_CONFIG_SRCDIR([src/libwaei/engine.c])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

RELEASE=1
AC_SUBST(RELEASE)

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

##General Dependencies
GLIB_REQUIRED_VERSION=2.25.0
GIO_REQUIRED_VERSION=2.25.0
GTHREAD_REQUIRED_VERSION=2.25.0
LIBCURL_REQUIRED_VERSION=7.21.0
GMODULE_EXPORT_REQUIRED_VERSION=2.25.0
#GTK Base Dependencies
GTK3_REQUIRED_VERSION=3.0.0
GDU_REQUIRED_VERSION=0.13.0
#GTK Optional Dependencies
LIBUNIQUE_REQUIRED_VERSION=1.0.0

#Custom arguments
AC_ARG_ENABLE([win32],
[  --enable-win32         turn on win32 (do not touch: experimental)],
[case "${enableval}" in
yes) win32=true ;;
no)  win32=false ;;
*) AC_MSG_ERROR([bad value ${enableval} for --enable-win32]) ;;
esac],[win32=false])
AM_CONDITIONAL([ENABLE_WIN32], [test x$win32 = xtrue])

AC_ARG_WITH([ncurses],
[  --without-ncurses         disable ncurses],
[case "${withval}" in
yes) ncurses=true ;;
no)  ncurses=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-ncurses]) ;;
esac],[ncurses=true])
AM_CONDITIONAL([WITH_NCURSES], [test x$ncurses = xtrue])

AC_ARG_WITH([gnome],
[  --without-gnome         turn off the gnome gui interface],
[case "${withval}" in
yes) gnome=true ;;
no)  gnome=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-gnome]) ;;
esac],[gnome=true])
AM_CONDITIONAL([WITH_GNOME], [test x$gnome = xtrue || test x$win32 = xtrue])

AC_ARG_WITH([libunique],
[  --without-libunique         disable libunique],
[case "${withval}" in
yes) libunique=true ;;
no)  libunique=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-libunique]) ;;
esac],[libunique=true])
AM_CONDITIONAL([WITH_LIBUNIQUE], [test x$libunique = xtrue && test x$gnome = xtrue && test x$win32 = xfalse])

AC_CHECK_LIB(m, sqrt)

PKG_CHECK_MODULES(BASE, glib-2.0           >= $GLIB_REQUIRED_VERSION
                        gobject-2.0        >= $GLIB_REQUIRED_VERSION
                        gio-2.0            >= $GIO_REQUIRED_VERSION
                        gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                        gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       
                        libcurl            >= $LIBCURL_REQUIRED_VERSION       )
AC_SUBST(BASE_CFLAGS)
AC_SUBST(BASE_LIBS)
GNOME_DOC_INIT(,,[:]) 


if test $gnome = true || test $win32 = true; then
  PKG_CHECK_MODULES(GNOME, gtk+-3.0           >= $GTK3_REQUIRED_VERSION
                           glib-2.0           >= $GLIB_REQUIRED_VERSION
                           gobject-2.0        >= $GLIB_REQUIRED_VERSION
                           gio-2.0            >= $GIO_REQUIRED_VERSION
                           gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                           gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       
                           libcurl            >= $LIBCURL_REQUIRED_VERSION
                           gnome-doc-utils    >= $GDU_REQUIRED_VERSION           )
  AC_SUBST(GNOME_CFLAGS)
  AC_SUBST(GNOME_LIBS)
  GNOME_DOC_INIT
fi


if test $gnome = true && test $libunique = true && test $win32 = false; then
  PKG_CHECK_MODULES(LIBUNIQUE, unique-3.0  >= $LIBUNIQUE_REQUIRED_VERSION)
  AC_SUBST(LIBUNIQUE_CFLAGS)
  AC_SUBST(LIBUNIQUE_LIBS)
fi

if test $ncurses = true && test $win32 = false; then
  NCURSES_LIBS=" -lncursesw "
  AC_SUBST(NCURSES_LIBS)
fi


GLIB_GSETTINGS

AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(KILLALL, killall)
AC_PATH_PROG(RPMDIR, rpm)
AC_PATH_PROG(RPMBUILD, rpmbuild)
AC_PATH_PROG(DPKG_DEB, dpkg-deb)
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

AC_CONFIG_FILES([Makefile src/Makefile src/libwaei/Makefile src/libwaei/include/libwaei/Makefile src/waei/Makefile src/waei/include/waei/Makefile src/gwaei/Makefile src/gwaei/include/gwaei/Makefile mandir/Makefile src/gwaei/help/Makefile src/gwaei/help/gwaei.omf src/gwaei/help/C/gwaei.xml src/gwaei/ui/Makefile src/desktop/Makefile src/images/Makefile src/schemas/Makefile rpm/gwaei.spec rpm/fedora/SPECS/gwaei.spec po/Makefile.in src/kpengine/Makefile] src/libwaei/doxyfile src/waei/doxyfile src/gwaei/doxyfile)

AC_OUTPUT



echo ""
if test $gnome = true || test $win32 = true; then
  echo -e "* Gnome is\tENABLED"
else
  echo -e "* Gnome is\tDISABLED"
fi
if test $libunique = true && test $gnome = true && test $win32 = false; then
  echo -e "* Libunique is\tENABLED"
else
  echo -e "* Libunique is\tDISABLED (Requires Gnome)"
fi
if test $ncurses = true && test $win32 = false; then
  echo -e "* Ncurses is\tENABLED"
else
  echo -e "* Ncurses is\tDISABLED"
fi

echo ""

if test ${win32} = true; then
  echo -e "* Win32 is\tENABLED (Only for developer use.  Not compatibile with Libunique/Libsexy/Ncurses)"
else
  echo -e "* Win32 is\tDISABLED (Only for developer use)"
fi


echo ""
echo "If these aren't the options you wanted, please rerun the configure script."


