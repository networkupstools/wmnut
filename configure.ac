dnl ====================================================================
dnl | WMNut: configure.ac                                              |
dnl ====================================================================

AC_INIT([wmnut],[0.71],[https://github.com/networkupstools/wmnut/issues])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR(src/wmnut.c)
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

dnl AC_SUBST(VERSION)
dnl
dnl ISODATE="`date +%Y-%m-%d`"
dnl AC_SUBST(ISODATE)

dnl FIXME: ...or check if a newer C standard is enabled
AC_MSG_CHECKING([whether we build in C99/GNU99 mode])
AS_CASE(["${CC}${CFLAGS}"],
	[*99*], [AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)
	 AC_MSG_WARN([If your build fails due to standard system header method and type names, or ISO C90 warnings, make sure you build in C99 mode (or newer) e.g. by passing '-std=gnu99' to GCC/CLANG])
	])

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
dnl Checks for standard header files.
dnl ===========================
AC_HEADER_SYS_WAIT

AC_CHECK_HEADER([limits.h],
	[AC_DEFINE([HAVE_LIMITS_H], [1],
		[Define to 1 if you have <limits.h>.])])

AC_CHECK_HEADER([stdlib.h],
	[AC_DEFINE([HAVE_STDLIB_H], [1],
		[Define to 1 if you have <stdlib.h>.])])

AC_CHECK_DECLS(realpath, [], [],
[#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
])

AC_CHECK_HEADERS(unistd.h stdio.h string.h signal.h ctype.h stdarg.h sys/time.h time.h X11/X.h X11/Xlib.h X11/xpm.h X11/extensions/shape.h X11/Xatom.h)

dnl ===========================
dnl Checks for standard library functions.
dnl ===========================
dnl AC_FUNC_MALLOC

AC_CHECK_FUNCS(usleep snprintf strdup)

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

X_LIBRARY_PATH="$x_libraries"

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

if test x"$enable_debug" = xyes; then
	DBGFLAGS="-Wall -g -pedantic"

	dnl Avoid "-ansi": it requires C89 which the modern system
	dnl headers are not compatible with generally
	dnl DBGFLAGS="$DBGFLAGS -ansi"
fi

AC_ARG_ENABLE(Werror,
	[  --enable-Werror         make compiler warnings fatal [default=no]],, enable_Werror=no)

if test x"$enable_Werror" = xyes; then
	DBGFLAGS="$DBGFLAGS -Werror"
fi
AC_SUBST([DBGFLAGS])

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

dnl If you really must build against older NUT, tweak the version here
dnl (perhaps 2.4.0+ may do) -- beware the build warnings and possible
dnl data type incompatibility however! See project commit history to
dnl maybe revert the changes (done in 2023) needed for the modern NUT.
dnl It may be easier to just take wmnut-0.67 or an older release then.
PKG_CHECK_MODULES(NUT, libupsclient >= 2.8.0)
NUT_HEADER="$NUT_CFLAGS"
NUT_LIB="$NUT_LIBS"

NUT_VERSION=""
AC_MSG_CHECKING([for precise NUT version we are building against])
AS_IF([test x"$PKG_CONFIG" != x], [
	NUT_VERSION="`$PKG_CONFIG --modversion libupsclient`" || NUT_VERSION=""
])
AS_IF([test x"$NUT_VERSION" = x],
	[AC_MSG_RESULT([could not determine])
	 AC_DEFINE_UNQUOTED([HAVE_NUT_VERSION], [0], [Do we know the precise NUT version WMNut was built against?])
	],
	[AC_MSG_RESULT([${NUT_VERSION}])
	 AC_DEFINE_UNQUOTED([NUT_VERSION], ["${NUT_VERSION}"], [Precise NUT version WMNut was built against])
	 AC_DEFINE_UNQUOTED([HAVE_NUT_VERSION], [1], [Do we know the precise NUT version WMNut was built against?])
	]
)

dnl NOTE: `upscli_str_contains_token()` is expected since NUT v2.8.3
myCFLAGS="$CFLAGS"
myLIBS="$LIBS"
CFLAGS="$NUT_CFLAGS"
LIBS="$NUT_LIBS"
AC_CHECK_FUNCS(upscli_strerror upscli_str_contains_token)
CFLAGS="$myCFLAGS"
LIBS="$myLIBS"

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
AC_SUBST(NUT_VERSION)

dnl ===========================
dnl Checks for non-standard library functions (might come from libs/headers found above).
dnl ===========================

AC_CHECK_FUNCS(xcalloc xmalloc)

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	doc/Makefile
	icons/Makefile
	contrib/Makefile
	contrib/wmnut-mdk.spec
	contrib/wmnut.spec
	contrib/wmnut.ebuild
	])
AC_OUTPUT
