/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#ifndef TESTS_TESTS_H
#define TESTS_TESTS_H

#include "zz-impl.h"

#include <errno.h>
#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif
#include <stdatomic.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

void zz_testinit(void);
zz_err zz_random(zz_bitcnt_t bc, bool s, zz_t *u);

#endif /* TESTS_TESTS_H */
