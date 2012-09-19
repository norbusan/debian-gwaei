AC_INIT(gwaei, 1.4.2)
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

RELEASE=1
AC_SUBST(RELEASE)

##General Dependencies
GLIB_REQUIRED_VERSION=2.10.0
GTHREAD_REQUIRED_VERSION=2.16.0
LIBCURL_REQUIRED_VERSION=7.18.0
GMODULE_EXPORT_REQUIRED_VERSION=2.16.0
#GTK Base Dependencies
GTK2_REQUIRED_VERSION=2.16.0
GCONF_REQUIRED_VERSION=2.22.0
GDU_REQUIRED_VERSION=0.13.0
#GTK Optional Dependencies
LIBUNIQUE_REQUIRED_VERSION=1.0.0
LIBSEXY_REQUIRED_VERSION=0.1.11

PKG_CHECK_MODULES(DEPS, glib-2.0           >= $GLIB_REQUIRED_VERSION
                        gobject-2.0        >= $GLIB_REQUIRED_VERSION
                        libcurl            >= $LIBCURL_REQUIRED_VERSION
                        gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                        gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       )
AC_CHECK_LIB(m, sqrt)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

ENGLISH_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/edict.gz
AC_SUBST(ENGLISH_DEFAULT_URI)

KANJI_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/kanjidic.gz
AC_SUBST(KANJI_DEFAULT_URI)

RADICALS_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/kradfile.gz
AC_SUBST(RADICALS_DEFAULT_URI)

NAMES_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/enamdict.gz
AC_SUBST(NAMES_DEFAULT_URI)

EXAMPLES_DEFAULT_URI=http://www.csse.monash.edu.au/~jwb/examples.gz
AC_SUBST(EXAMPLES_DEFAULT_URI)

FRENCH_DEFAULT_URI=http://dico.fj.free.fr/fj.utf
AC_SUBST(FRENCH_DEFAULT_URI)

GERMAN_DEFAULT_URI=http://www.bibiko.de/WdJTEUC.zip
AC_SUBST(GERMAN_DEFAULT_URI)

SPANISH_DEFAULT_URI=http://hispadic.googlepages.com/hispamix_euc.zip
AC_SUBST(SPANISH_DEFAULT_URI)

LOAD_ORDER_DEFAULT=English,Mix,Kanji,Radicals,Names,Places,Examples
AC_SUBST(LOAD_ORDER_DEFAULT)


MATCH_FG_DEFAULT=#000000
AC_SUBST(MATCH_FG_DEFAULT)

MATCH_BG_DEFAULT=#CCEECC
AC_SUBST(MATCH_BG_DEFAULT)

HEADER_FG_DEFAULT=#EE1111
AC_SUBST(HEADER_FG_DEFAULT)

HEADER_BG_DEFAULT=#FFDEDE
AC_SUBST(HEADER_BG_DEFAULT)

COMMENT_FG_DEFAULT=#2222DD
AC_SUBST(COMMENT_FG_DEFAULT)


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
AM_CONDITIONAL([WITH_GNOME], [test x$gnome = xtrue])

AC_ARG_WITH([libunique],
[  --without-libunique         disable libunique],
[case "${withval}" in
yes) libunique=true ;;
no)  libunique=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-libunique]) ;;
esac],[libunique=true])
AM_CONDITIONAL([WITH_LIBUNIQUE], [test x$libunique = xtrue && test x$gnome = xtrue])

AC_ARG_WITH([libsexy],
[  --without-libsexy         disable libsexy],
[case "${withval}" in
yes) libsexy=true ;;
no)  libsexy=false ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-libsexy]) ;;
esac],[libsexy=true])
AM_CONDITIONAL([WITH_LIBSEXY], [test x$libsexy = xtrue && test x$gnome = xtrue])




if test ${win32} = true; then
  PKG_CHECK_MODULES(WIN32, gtk+-2.0           >= $GTK2_REQUIRED_VERSION )
  AC_SUBST(WIN32_CFLAGS)
  AC_SUBST(WIN32_LIBS)
elif test $gnome = true; then
  PKG_CHECK_MODULES(GNOME, gtk+-2.0           >= $GTK2_REQUIRED_VERSION
                           gconf-2.0          >= $GCONF_REQUIRED_VERSION
                           gnome-doc-utils    >= $GDU_REQUIRED_VERSION )
  AC_SUBST(GNOME_CFLAGS)
  AC_SUBST(GNOME_LIBS)
  GNOME_DOC_INIT
else
  dnl Do not care if GDU is not found
  GNOME_DOC_INIT(,,[:])
fi

if test $libunique = true && test $gnome = true; then
  PKG_CHECK_MODULES(LIBUNIQUE, unique-1.0  >= $LIBUNIQUE_REQUIRED_VERSION)
  AC_SUBST(LIBUNIQUE_CFLAGS)
  AC_SUBST(LIBUNIQUE_LIBS)
fi

if test $libsexy = true && test $gnome = true; then
  PKG_CHECK_MODULES(LIBSEXY, libsexy  >= $LIBSEXY_REQUIRED_VERSION)
  AC_SUBST(LIBSEXY_CFLAGS)
  AC_SUBST(LIBSEXY_LIBS)
fi

if test $ncurses = true; then
  NCURSES_LIBS=" -lncursesw "
  AC_SUBST(NCURSES_LIBS)
fi





AM_GCONF_SOURCE_2
AC_PATH_PROG(GCONFTOOL, gconftool-2)
AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(KILLALL, killall)
AC_PATH_PROG(RPMDIR, rpm)
AC_PATH_PROG(RPMBUILD, rpmbuild)
AC_PATH_PROG(DPKG_DEB, dpkg-deb)
AC_PATH_PROG(RSYNC, rsync)
AC_PATH_PROG(GUNZIP, gunzip)
AC_PATH_PROG(UNZIP, unzip)

AC_PROG_CC
AM_GNU_GETTEXT([external])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile src/Makefile mandir/Makefile help/Makefile help/gwaei.omf help/C/gwaei.xml src/gtk-builder-xml/Makefile src/images/Makefile src/schemas/Makefile src/desktop/Makefile src/kpengine/Makefile src/kpengine/conv_jdata.pl rpm/gwaei.spec rpm/rpm_gconf_install_post.sh rpm/rpm_gconf_uninstall_pre.sh rpm/fedora/SPECS/gwaei.spec src/schemas/gwaei.schemas src/include/gwaei/definitions.h po/Makefile.in po/Makefile] Doxyfile)
AC_OUTPUT



echo ""
if test $gnome = true; then
  echo -e "* Gnome is\tENABLED"
else
  echo -e "* Gnome is\tDISABLED"
fi
if test $libunique = true && test $gnome = true; then
  echo -e "* Libunique is\tENABLED"
else
  echo -e "* Libunique is\tDISABLED (Requires Gnome)"
fi
if test $libsexy = true && test $gnome = true; then
  echo -e "* Libsexy is\tENABLED"
else
  echo -e "* Libsexy is\tDISABLED (Requires Gnome)"
fi
if test $ncurses = true; then
  echo -e "* Ncurses is\tENABLED"
else
  echo -e "* Ncurses is\tDISABLED"
fi

echo ""

if test ${win32} = true; then
  echo -e "* Win32 is\tENABLED (Only for developer use)"
else
  echo -e "* Win32 is\tDISABLED (Only for developer use)"
fi


echo ""
echo "If these aren't the options you wanted, please rerun the configure script."


