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
#include <math.h>
#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#include <time.h>
#ifdef HAVE_VALGRIND_VALGRIND_H
#  include <valgrind/valgrind.h>
#endif

extern int nsamples;

void zz_testinit(void);
zz_err zz_random(zz_bitcnt_t bc, bool s, zz_t *u);
void zz_testclear(void);

/* Poor-mans allocator routines with memory constraint.
 * total_size should be reset on ZZ_MEM errors. */

extern atomic_size_t total_size;
extern size_t max_size;

void * my_malloc(size_t size);
void * my_realloc(void *ptr, size_t old_size, size_t new_size);
void my_free(void *ptr, size_t size);

void * square_worker(void *args);

#endif /* TESTS_TESTS_H */
