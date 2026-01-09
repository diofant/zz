/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#ifndef IMPL_ZZ_H
#define IMPL_ZZ_H

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#if defined(__MINGW32__) && defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#endif
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include <gmp.h>
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
#if defined(__MINGW32__) && defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

#include "zz.h"

#define ZZ_DIGIT_T_MAX UINT64_MAX
#define ZZ_DIGIT_T_BYTES 8
#ifndef _WIN32
#  define ZZ_BITS_MAX UINT64_MAX
#  define ZZ_DIGITS_MAX (zz_size_t)(ZZ_BITS_MAX/ZZ_DIGIT_T_BITS)
#else
#  define ZZ_DIGITS_MAX INT32_MAX
#  define ZZ_BITS_MAX (zz_bitcnt_t)INT32_MAX*ZZ_DIGIT_T_BITS
#endif

static _Thread_local jmp_buf zz_env;
/* Function should include if(TMP_OVERFLOW){...} workaround in
   case it calls any mpn_*() API, which does memory allocation for
   temporary storage.  Not all functions do this, sometimes it's
   obvious (e.g. mpn_cmp() or mpn_add/sub()), sometimes - not (e.g.
   mpn_get/set_str() for power of 2 bases).  Though, these details
   aren't documented and if you feel that in the given case things
   might be changed - please add a workaround. */
#define TMP_OVERFLOW (setjmp(zz_env) == 1)

#define TMP_MPZ(z, u)                                        \
    mpz_t z;                                                 \
                                                             \
    assert((u)->size <= INT_MAX);                            \
    z->_mp_d = (u)->digits;                                  \
    z->_mp_size = ((u)->negative ? -1 : 1) * (int)(u)->size; \
    z->_mp_alloc = (int)(u)->alloc;

#define SWAP(T, a, b) \
    do {              \
        T _tmp = a;   \
        a = b;        \
        b = _tmp;     \
    } while (0);

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void
zz_set_memory_funcs(void *(*malloc) (size_t),
                    void *(*realloc) (void *, size_t, size_t),
                    void (*free) (void *, size_t));

#endif /* IMPL_ZZ_H */
