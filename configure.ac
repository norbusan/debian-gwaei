AC_INIT(gwaei, 3.4.3)
AC_CONFIG_SRCDIR([src/libwaei/engine.c])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

RELEASE=1
AC_SUBST(RELEASE)

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

##General Dependencies
GLIB_REQUIRED_VERSION=2.31.0
GIO_REQUIRED_VERSION=2.31.0
GTHREAD_REQUIRED_VERSION=2.31.0
LIBCURL_REQUIRED_VERSION=7.20.0
GMODULE_EXPORT_REQUIRED_VERSION=2.31.0
#GTK Base Dependencies
GTK3_REQUIRED_VERSION=3.3.0
GDU_REQUIRED_VERSION=0.13.0
HUNSPELL_REQUIRED_VERSION=1.3.0
ZLIB_REQUIRED_VERSION=1.2.5

AC_CANONICAL_HOST
AM_CONDITIONAL([OS_MINGW],false)
case $host_os in
  *mingw*)
  AC_DEFINE([OS_MINGW],[1],[Mingw Host])
  AM_CONDITIONAL([OS_MINGW],true)
  OS_MINGW=1
  ;;
esac
AM_CONDITIONAL([OS_CYGWIN],false)
case $host_os in
  *cygwin*)
  AC_DEFINE([OS_CYGWIN],[1],[Cygwin Host])
  AM_CONDITIONAL([OS_CYGWIN],true)
  OS_CYGWIN=1
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
AM_CONDITIONAL([WITH_GNOME], [test x$gnome = xtrue])


AC_ARG_WITH([hunspell],
[  --without-hunspell        turn off Hunspell spellchecking and morphological analysis],
[case "${withval}" in
yes) hunspell=true ;;
no)  hunspell=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --with-hunspell]) ;;
esac],[hunspell=true])
AM_CONDITIONAL([WITH_HUNSPELL], [test x$hunspell = xtrue])


AC_ARG_WITH([mecab],
[  --without-mecab        turn off Mecab morphological analysis],
[case "${withval}" in
yes) mecab=true ;;
no)  mecab=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --with-mecab]) ;;
esac],[mecab=true])
AM_CONDITIONAL([WITH_MECAB], [test x$mecab = xtrue])

AC_CHECK_LIB(m, sqrt)

GNOME_DOC_INIT(,,[:]) 

PKG_CHECK_MODULES(LIBWAEI, glib-2.0           >= $GLIB_REQUIRED_VERSION
                           gobject-2.0        >= $GLIB_REQUIRED_VERSION
                           gio-2.0            >= $GIO_REQUIRED_VERSION
                           gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                           gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       
                           libcurl            >= $LIBCURL_REQUIRED_VERSION      
                           zlib               >= $ZLIB_REQUIRED_VERSION       )
AC_SUBST(LIBWAEI_CFLAGS)
AC_SUBST(LIBWAEI_LIBS)

PKG_CHECK_MODULES(WAEI, glib-2.0           >= $GLIB_REQUIRED_VERSION
                        gobject-2.0        >= $GLIB_REQUIRED_VERSION
                        gio-2.0            >= $GIO_REQUIRED_VERSION
                        gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                        gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       )
AC_SUBST(WAEI_CFLAGS)
AC_SUBST(WAEI_LIBS)

if test x$gnome = xtrue; then
  PKG_CHECK_MODULES(GWAEI, gtk+-3.0           >= $GTK3_REQUIRED_VERSION
                           glib-2.0           >= $GLIB_REQUIRED_VERSION
                           gobject-2.0        >= $GLIB_REQUIRED_VERSION
                           gio-2.0            >= $GIO_REQUIRED_VERSION
                           gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                           gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       
                           libcurl            >= $LIBCURL_REQUIRED_VERSION )
  AC_SUBST(GWAEI_CFLAGS)
  AC_SUBST(GWAEI_LIBS)
  GNOME_DOC_INIT
fi

if test x$hunspell = xtrue; then
  PKG_CHECK_MODULES(HUNSPELL, hunspell           >= $HUNSPELL_REQUIRED_VERSION)
  AC_SUBST(HUNSPELL_CFLAGS)
  AC_SUBST(HUNSPELL_LIBS)
fi

if test x$mecab = xtrue; then
  AC_CHECK_HEADER([mecab.h],
                  [AC_DEFINE([HAVE_MECAB], [0], [Mecab]) MECAB_LIBS="-lmecab -lstdc++" AC_SUBST(MECAB_LIBS)], 
                  [AC_MSG_ERROR([Could not find mecab.h! Make sure you install the mecab development files or or you can disable mecab support with ./configure --without-mecab])])
AM_CONDITIONAL([HAVE_MECAB],true)
else
AM_CONDITIONAL([HAVE_MECAB],false)
fi


myspelldictpath=${datadir}/myspell/dicts
AC_ARG_WITH(myspell_dict_path, AS_HELP_STRING([--with-myspell-dict-path=PATH],[path to myspell dictionaries]))

if test "x$with_myspell_dict_path" != "x" ; then
   myspelldictpath=$with_myspell_dict_path
fi
HUNSPELL_MYSPELL_DICTIONARY_PATH="$myspelldictpath"
AC_SUBST(HUNSPELL_MYSPELL_DICTIONARY_PATH)



GLIB_GSETTINGS

AC_PATH_PROG(PERL, perl)

AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_HEADER_STDC

AC_PROG_INTLTOOL([0.40.0])
GETTEXT_PACKAGE=gwaei
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

GWAEI_LOCALEDIR=[${datadir}/locale]
AC_SUBST(GWAEI_LOCALEDIR)

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile src/Makefile src/libwaei/Makefile src/libwaei/include/libwaei/Makefile src/waei/Makefile src/waei/include/waei/Makefile src/gwaei/Makefile src/gwaei/mingw/Makefile src/gwaei/include/gwaei/Makefile mandir/Makefile src/gwaei/help/Makefile src/gwaei/help/gwaei.omf src/gwaei/help/C/gwaei.xml src/desktop/Makefile src/images/Makefile src/schemas/Makefile rpm/gwaei.spec rpm/fedora/SPECS/gwaei.spec po/Makefile.in src/kpengine/Makefile src/libwaei/doxyfile src/waei/doxyfile src/gwaei/doxyfile])

AC_OUTPUT


echo ""

if test x$gnome = xtrue; then
  echo -e "* Gnome is\tENABLED"
else
  echo -e "* Gnome is\tDISABLED"
fi

if test x$hunspell = xtrue; then
  echo -e "* Hunspell is\tENABLED (Dictionary search path: $myspelldictpath)"
else
  echo -e "* Hunspell is\tDISABLED"
fi


if test x$mecab = xtrue; then
  echo -e "* Mecab is\tENABLED"
else
  echo -e "* Mecab is\tDISABLED"
fi



echo ""
echo "If these aren't the options you wanted, please rerun the configure script."


