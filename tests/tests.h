/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#ifndef TESTS_TESTS_H
#define TESTS_TESTS_H

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

#include "zz-impl.h"

typedef struct {
    char *u;
    char *v;
} _zz_bin_ex;

#define _zz_from_dec(s, u) zz_from_str((int8_t *)s, strlen(s), 10, u)
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define ZZ_BINOP_REF(op)                                \
    zz_err                                              \
    _zz_ref_##op(const zz_t *u, const zz_t *v, zz_t *w) \
    {                                                   \
        mpz_t z;                                        \
        TMP_MPZ(mu, u);                                 \
        TMP_MPZ(mv, v);                                 \
        if (TMP_OVERFLOW) {                             \
            return ZZ_MEM;                              \
        }                                               \
        mpz_init(z);                                    \
        mpz_##op(z, mu, mv);                            \
                                                        \
        zz_t tmp = {z->_mp_size < 0, ABS(z->_mp_size),  \
                    ABS(z->_mp_size),                   \
                    z->_mp_d};                          \
        if (zz_copy(&tmp, w)) {                         \
            mpz_clear(z);                               \
            return ZZ_MEM;                              \
        }                                               \
        mpz_clear(z);                                   \
        return ZZ_OK;                                   \
    }

#define TEST_BINOP_EXAMPLE(op, lhs, rhs)                      \
    do {                                                      \
        zz_t u, v, w, r;                                      \
                                                              \
        if (zz_init(&u) || zz_init(&v) || zz_init(&w)) {      \
            abort();                                          \
        }                                                     \
        if (zz_copy(lhs, &u) || zz_copy(rhs, &v)) {           \
            abort();                                          \
        }                                                     \
        if (zz_init(&r) || _zz_ref_##op(&u, &v, &r)) {        \
            abort();                                          \
        }                                                     \
        if (zz_##op(&u, &v, &w) || zz_cmp(&w, &r) != ZZ_EQ) { \
            abort();                                          \
        }                                                     \
        if (zz_copy(&u, &w) || zz_##op(&w, &v, &w)            \
            || zz_cmp(&w, &r) != ZZ_EQ) {                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(&v, &w) || zz_##op(&u, &w, &w)            \
            || zz_cmp(&w, &r) != ZZ_EQ) {                     \
            abort();                                          \
        }                                                     \
        zz_clear(&u);                                         \
        zz_clear(&v);                                         \
        zz_clear(&w);                                         \
        zz_clear(&r);                                         \
    } while (0);

void _zz_testinit(void);
zz_err _zz_random(zz_bitcnt_t bc, bool s, zz_t *u);

#define TEST_BINOP(op, ex, bs, neg, nex)                        \
    void                                                        \
    check_##op##_bulk(void)                                     \
    {                                                           \
        size_t ex_len = sizeof(ex)/sizeof(_zz_bin_ex);          \
                                                                \
        for (size_t i = 0; i < ex_len; i++) {                   \
            zz_t lhs, rhs;                                      \
                                                                \
            if (zz_init(&lhs) || _zz_from_dec(ex[i].u, &lhs)) { \
                abort();                                        \
            }                                                   \
            if (zz_init(&rhs) || _zz_from_dec(ex[i].v, &rhs)) { \
                abort();                                        \
            }                                                   \
            TEST_BINOP_EXAMPLE(op, &lhs, &rhs);                 \
            zz_clear(&lhs);                                     \
            zz_clear(&rhs);                                     \
        }                                                       \
        for (size_t i = 0; i < nex; i++) {                      \
            zz_t lhs, rhs;                                      \
                                                                \
            if (zz_init(&lhs) || _zz_random(bs, neg, &lhs)) {   \
                abort();                                        \
            }                                                   \
            if (zz_init(&rhs) || _zz_random(bs, neg, &rhs)) {   \
                abort();                                        \
            }                                                   \
            TEST_BINOP_EXAMPLE(op, &lhs, &rhs);                 \
            zz_clear(&lhs);                                     \
            zz_clear(&rhs);                                     \
        }                                                       \
    }

#endif /* TESTS_TESTS_H */
