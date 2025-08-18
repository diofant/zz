# Process this file with autoconf to produce a configure script.

define(ZZ_COPYRIGHT,[[
Copyright (C) 2024, 2025 Sergey B Kirpichev

This file is part of the ZZ Library.

The ZZ Library is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 3 of the License, or (at your option)
any later version.  See <https://www.gnu.org/licenses/>.

]])
define(ZZ_COPYRIGHT_C,[[/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of  the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/]])

AC_COPYRIGHT(ZZ_COPYRIGHT)
AH_TOP(ZZ_COPYRIGHT_C)

AC_PREREQ(2.62)

AC_INIT([zz],[0.1],[skirpichev@gmail.com])
#AC_CONFIG_SRCDIR(zz.h)
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([gnu no-dependencies])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_PROG_MAKE_SET

# Check for which system.
AC_CANONICAL_HOST

# Disable unnecessary libtool tests...
define([AC_LIBTOOL_LANG_CXX_CONFIG],[:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG],[:])dnl
define([AC_LIBTOOL_LANG_GCJ_CONFIG],[:])dnl

# Checks for programs.
AC_PROG_CC
AC_LANG(C)
LT_INIT

# Checks for libraries...
AC_CHECK_LIB(gmp,__gmpz_init,,AC_MSG_ERROR([GNU GMP library is required.]))

# Checks for header files.
AC_CHECK_HEADERS([assert.h ctype.h float.h inttypes.h math.h setjmp.h
                  stdlib.h string.h stdbool.h stddef.h stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

AC_CONFIG_FILES([makefile zz.pc])
AC_OUTPUT
