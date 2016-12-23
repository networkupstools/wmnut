dnl ====================================================================
dnl | WMNut: configure.ac                                              |
dnl ====================================================================

AC_INIT([wmnut],[0.65])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR(src/wmnut.c)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

dnl AC_SUBST(VERSION)
dnl
dnl ISODATE=`date +%Y-%m-%d`
dnl AC_SUBST(ISODATE)


dnl
dnl Specify paths to look for libraries and headers
dnl ===============================================
AC_ARG_WITH(libs-from,
       [  --with-libs-from        pass compiler flags to look for libraries],
       [lib_search_path="$withval $lib_search_path"])

AC_ARG_WITH(incs-from,
       [  --with-incs-from        pass compiler flags to look for header files],
       [inc_search_path="$withval $inc_search_path"])

dnl ===========================
dnl check for getopt specifics.
dnl ===========================
AC_CHECK_LIB(gnugetopt, getopt_long)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_FUNCS(getopt_long)

dnl ===========================================
dnl             Stuff that uses X
dnl ===========================================

AC_PATH_XTRA

X_LIBRARY_PATH=$x_libraries

XCFLAGS="$X_CFLAGS"

XLFLAGS="$X_LIBS"

XLIBS="-lX11 $X_EXTRA_LIBS"

lib_search_path="$lib_search_path $XLFLAGS"
inc_search_path="$inc_search_path $XCFLAGS"

AC_SUBST(X_LIBRARY_PATH)

dnl XBell Extension
dnl ===============
dnl AC_CHECK_LIB(X11, XBell, [XLIBS="$XLIBS -lX11"],
dnl     [echo "The XBell extension stuff could not be found in the X client libraries"
dnl      exit 1],
dnl     $X_LIBS $X_EXTRA_LIBS -lX11)
	
dnl Shape extension
dnl ===============
AC_CHECK_LIB(Xext, XShapeCombineMask, [XLIBS="$XLIBS -lXext"],
    [echo "The shape extension stuff could not be found in the X client libraries"
     exit 1],
    $X_LIBS $X_EXTRA_LIBS -lX11)

dnl XPM library
dnl ===========
AC_CHECK_LIB(Xpm, XpmCreatePixmapFromData, [XLIBS="$XLIBS -lXpm"],
    [echo "The libXpm library was not found, but is necessary to build this library"
     exit 1],
    $X_LIBS $X_EXTRA_LIBS -lX11)

AC_SUBST(XCFLAGS)
AC_SUBST(XLFLAGS)
AC_SUBST(XLIBS)

AC_SUBST(X_EXTRA_LIBS)

dnl ===============================================
dnl             End of stuff that uses X
dnl ===============================================
	
dnl =========
dnl Debugging
dnl =========
AC_ARG_ENABLE(debug,
             [  --enable-debug          turn on debugging [default=no]],, enable_debug=no)
if test "$enable_debug" = yes; then
  DFLAGS="-Wall -g -ansi -pedantic"
fi
AC_SUBST(DFLAGS)


LDFLAGS="$XLIBS $X_EXTRA_LIBS"


dnl ===============
dnl Dockapp library
dnl ===============
dnl AC_CHECK_LIB(libdockapp, DAInitialize, [DOCKLIBS="$DOCKLIBS -ldockapp"],
dnl	[echo "The libdockapp library was not found, but is necessary to build this library"
dnl	exit 1],
dnl	$DOCKLIBS -ldockapp)
 
dnl AC_SUBST(DOCKLIBS)

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS

dnl ===============================================
dnl Checks for NUT client library
dnl ===============================================

PKG_CHECK_MODULES(NUT, libupsclient  >= 2.4.0)
NUT_HEADER="$NUT_CFLAGS"
NUT_LIB="$NUT_LIBS"

dnl Checks for typedefs, structures, and compiler characteristics.

dnl =============================
dnl Checks for library functions.
dnl =============================

LIBRARY_SEARCH_PATH="$lib_search_path"
HEADER_SEARCH_PATH="$inc_search_path"

AC_SUBST(LIBRARY_SEARCH_PATH)
AC_SUBST(HEADER_SEARCH_PATH)
AC_SUBST(NUT_HEADER)
AC_SUBST(NUT_LIB)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(unistd.h stdlib.h stdio.h string.h signal.h ctype.h stdarg.h X11/Xlib.h X11/xpm.h X11/extensions/shape.h X11/Xatom.h)

dnl Checks for header files.
AC_HEADER_STDC


dnl Checks for library functions.
dnl AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
		src/Makefile
		doc/Makefile
		icons/Makefile
		contrib/Makefile
		contrib/wmnut-mdk.spec
		contrib/wmnut.spec
		contrib/wmnut.ebuild])
AC_OUTPUT
