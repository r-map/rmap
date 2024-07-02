#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mqtt2bufr], [1.8], [edigiacomo@arpae.it])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl AC_PROG_CXX
dnl AC_PROG_CC
dnl AC_LANG(C++)
AM_PATH_PYTHON([3.6])

dnl Use c++11
dnl AX_CXX_COMPILE_STDCXX_11

AM_INIT_AUTOMAKE([nostdinc subdir-objects])
LT_INIT

dnl Check for jansson
dnl AC_CHECK_LIB([jansson], [json_loads],
dnl              [LIBS="$LIBS -ljansson"],
dnl              [AC_MSG_ERROR([libjansson not found])])
dnl Check for dballe
dnl PKG_CHECK_MODULES([DBALLE], [libdballe >= 6.3],
dnl 		  [
dnl 		   LIBS="$LIBS $DBALLE_LIBS"
dnl 		   CPPFLAGS="$CPPFLAGS $DBALLE_CFLAGS"
dnl 		  ])
dnl Check for mosquittopp
dnl AC_MSG_CHECKING([-lmosquittopp])
dnl SAVED_LDFLAGS=$LDFLAGS
dnl LDFLAGS="$LDFLAGS -lmosquittopp"
dnl AC_LINK_IFELSE(
dnl 	       [AC_LANG_PROGRAM([#include <mosquittopp.h>],
dnl 				[mosqpp::mosquittopp mqpp])],
dnl 				[have_mosquittopp=yes],
dnl 				[have_mosquittopp=no])
dnl LDFLAGS=$SAVED_LDFLAGS
dnl AC_MSG_RESULT([$have_mosquittopp])

dnl if test x$have_mosquittopp = xyes
dnl then
dnl 	LIBS="$LIBS -lmosquittopp"
dnl else
dnl 	AC_MSG_ERROR([libmosquittopp not found])
dnl fi

dnl Check for help2man
AC_CHECK_PROG([HELP2MAN], [help2man], [help2man])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
