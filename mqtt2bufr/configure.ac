#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mqtt2bufr], [0.6], [edigiacomo@arpa.emr.it])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

dnl Use c++11
AX_CXX_COMPILE_STDCXX_11

AM_INIT_AUTOMAKE([nostdinc subdir-objects])
LT_INIT

dnl Check for jansson
AC_CHECK_LIB([jansson], [json_loads],
             [LIBS="$LIBS -ljansson"],
             [AC_MSG_ERROR([libjansson not found])])
dnl Check for dballe
PKG_CHECK_MODULES([DBALLE], [libdballe >= 6.3],
		  [
		   LIBS="$LIBS $DBALLE_LIBS"
		   CPPFLAGS="$CPPFLAGS $DBALLE_CFLAGS"
		  ])
dnl Check for mosquittopp
AC_MSG_CHECKING([-lmosquittopp])
SAVED_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -lmosquittopp"
AC_LINK_IFELSE(
	       [AC_LANG_PROGRAM([#include <mosquittopp.h>],
				[mosqpp::mosquittopp mqpp])],
				[have_mosquittopp=yes],
				[have_mosquittopp=no])
LDFLAGS=$SAVED_LDFLAGS
AC_MSG_RESULT([$have_mosquittopp])

if test x$have_mosquittopp = xyes
then
	LIBS="$LIBS -lmosquittopp"
else
	AC_MSG_ERROR([libmosquittopp not found])
fi

dnl Check for wibble
PKG_CHECK_MODULES([WIBBLE], [libwibble],
                  [
                   LIBS="$LIBS $WIBBLE_LIBS"
                   CFLAGS="$CFLAGS
                   $WIBBLE_CFLAGS"
                  ])

dnl Check for help2man
AC_CHECK_PROG([HELP2MAN], [help2man], [help2man])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
