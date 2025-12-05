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

#include "zz-impl.h"

typedef struct {
    char *u;
    char *v;
} zz_bin_ex;

#define zz_from_dec(s, u) zz_from_str((int8_t *)s, strlen(s), 10, u)
#define ABS(x) ((x) >= 0 ? (x) : -(x))

#define ZZ_BINOP_REF(op)                                \
    zz_err                                              \
    zz_ref_##op(const zz_t *u, const zz_t *v, zz_t *w)  \
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

#define ZZ_MIXBINOP_REF(op)                                \
    ZZ_BINOP_REF(op)                                       \
    zz_err                                                 \
    zz_ref_##op##_sl(const zz_t *u, zz_slimb_t v, zz_t *w) \
    {                                                      \
        zz_t tmp;                                          \
                                                           \
        if (zz_init(&tmp) || zz_from_sl(v, &tmp)           \
            || zz_ref_##op(u, &tmp, w))                    \
        {                                                  \
            zz_clear(&tmp);                                \
            return ZZ_MEM;                                 \
        }                                                  \
        zz_clear(&tmp);                                    \
        return ZZ_OK;                                      \
    }                                                      \
    zz_err                                                 \
    zz_ref_sl_##op(zz_slimb_t u, const zz_t *v, zz_t *w)   \
    {                                                      \
        zz_t tmp;                                          \
                                                           \
        if (zz_init(&tmp) || zz_from_sl(u, &tmp)           \
            || zz_ref_##op(&tmp, v, w))                    \
        {                                                  \
            zz_clear(&tmp);                                \
            return ZZ_MEM;                                 \
        }                                                  \
        zz_clear(&tmp);                                    \
        return ZZ_OK;                                      \
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
        if (zz_iszero(&v)                                     \
            && ((zz_ref_##op) == zz_ref_fdiv_q                \
                || (zz_ref_##op) == zz_ref_fdiv_r))           \
        {                                                     \
            zz_clear(&u);                                     \
            zz_clear(&v);                                     \
            zz_clear(&w);                                     \
            break;                                            \
        }                                                     \
        if (zz_iszero(&u) && zz_iszero(&v)                    \
            && (zz_ref_##op) == zz_ref_lcm)                   \
        {                                                     \
            zz_clear(&u);                                     \
            zz_clear(&v);                                     \
            zz_clear(&w);                                     \
            break;                                            \
        }                                                     \
        if (zz_init(&r) || zz_ref_##op(&u, &v, &r)) {         \
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

#define TEST_MIXBINOP_EXAMPLE(op, lhs, rhs)                              \
    TEST_BINOP_EXAMPLE(op, lhs, rhs)                                     \
    do {                                                                 \
        zz_t u, v, w, r;                                                 \
                                                                         \
        if (zz_init(&u) || zz_init(&v) || zz_init(&w)) {                 \
            abort();                                                     \
        }                                                                \
        if (zz_copy(lhs, &u) || zz_copy(rhs, &v)) {                      \
            abort();                                                     \
        }                                                                \
        if (zz_iszero(&v)                                                \
            && ((zz_ref_##op##_sl) == zz_ref_fdiv_q_sl                   \
                || (zz_ref_##op##_sl) == zz_ref_fdiv_r_sl))              \
        {                                                                \
            zz_clear(&u);                                                \
            zz_clear(&v);                                                \
            zz_clear(&w);                                                \
            break;                                                       \
        }                                                                \
        if (zz_init(&r)) {                                               \
            abort();                                                     \
        }                                                                \
                                                                         \
        zz_slimb_t limb;                                                 \
                                                                         \
        if (zz_to_sl(&v, &limb) == ZZ_OK) {                              \
            if (zz_ref_##op##_sl(&u, limb, &r)) {                        \
                abort();                                                 \
            }                                                            \
            if (zz_##op##_sl(&u, limb, &w) || zz_cmp(&w, &r) != ZZ_EQ) { \
                abort();                                                 \
            }                                                            \
        }                                                                \
        if (zz_to_sl(&u, &limb) == ZZ_OK) {                              \
            if (zz_ref_sl_##op(limb, &v, &r)) {                          \
                abort();                                                 \
            }                                                            \
            if (zz_sl_##op(limb, &v, &w) || zz_cmp(&w, &r) != ZZ_EQ) {   \
                abort();                                                 \
            }                                                            \
        }                                                                \
        zz_clear(&u);                                                    \
        zz_clear(&v);                                                    \
        zz_clear(&w);                                                    \
        zz_clear(&r);                                                    \
    } while (0);

void zz_testinit(void);
zz_err zz_random(zz_bitcnt_t bc, bool s, zz_t *u);

#define TEST_BINOP(op, ex, bs, neg, nex)                       \
    void                                                       \
    check_##op##_bulk(void)                                    \
    {                                                          \
        size_t ex_len = sizeof(ex)/sizeof(zz_bin_ex);          \
                                                               \
        for (size_t i = 0; i < ex_len; i++) {                  \
            zz_t lhs, rhs;                                     \
                                                               \
            if (zz_init(&lhs) || zz_from_dec(ex[i].u, &lhs)) { \
                abort();                                       \
            }                                                  \
            if (zz_init(&rhs) || zz_from_dec(ex[i].v, &rhs)) { \
                abort();                                       \
            }                                                  \
            TEST_BINOP_EXAMPLE(op, &lhs, &rhs);                \
            zz_clear(&lhs);                                    \
            zz_clear(&rhs);                                    \
        }                                                      \
        for (size_t i = 0; i < nex; i++) {                     \
            zz_t lhs, rhs;                                     \
                                                               \
            if (zz_init(&lhs) || zz_random(bs, neg, &lhs)) {   \
                abort();                                       \
            }                                                  \
            if (zz_init(&rhs) || zz_random(bs, neg, &rhs)) {   \
                abort();                                       \
            }                                                  \
            TEST_BINOP_EXAMPLE(op, &lhs, &rhs);                \
            zz_clear(&lhs);                                    \
            zz_clear(&rhs);                                    \
        }                                                      \
    }

#define TEST_MIXBINOP(op, ex, bs, neg, nex)                    \
    void                                                       \
    check_##op##_bulk(void)                                    \
    {                                                          \
        size_t ex_len = sizeof(ex)/sizeof(zz_bin_ex);          \
                                                               \
        for (size_t i = 0; i < ex_len; i++) {                  \
            zz_t lhs, rhs;                                     \
                                                               \
            if (zz_init(&lhs) || zz_from_dec(ex[i].u, &lhs)) { \
                abort();                                       \
            }                                                  \
            if (zz_init(&rhs) || zz_from_dec(ex[i].v, &rhs)) { \
                abort();                                       \
            }                                                  \
            TEST_MIXBINOP_EXAMPLE(op, &lhs, &rhs);             \
            zz_clear(&lhs);                                    \
            zz_clear(&rhs);                                    \
        }                                                      \
        for (size_t i = 0; i < nex; i++) {                     \
            zz_t lhs, rhs;                                     \
                                                               \
            if (zz_init(&lhs) || zz_random(bs, neg, &lhs)) {   \
                abort();                                       \
            }                                                  \
            if (zz_init(&rhs) || zz_random(bs, neg, &rhs)) {   \
                abort();                                       \
            }                                                  \
            TEST_MIXBINOP_EXAMPLE(op, &lhs, &rhs);             \
            zz_clear(&lhs);                                    \
            zz_clear(&rhs);                                    \
        }                                                      \
    }

#define ZZ_UNOP_REF(op)                                \
    zz_err                                             \
    zz_ref_##op(const zz_t *u, zz_t *v)                \
    {                                                  \
        mpz_t z;                                       \
        TMP_MPZ(mu, u);                                \
        if (TMP_OVERFLOW) {                            \
            return ZZ_MEM;                             \
        }                                              \
        mpz_init(z);                                   \
        mpz_##op(z, mu);                               \
                                                       \
        zz_t tmp = {z->_mp_size < 0, ABS(z->_mp_size), \
                    ABS(z->_mp_size),                  \
                    z->_mp_d};                         \
        if (zz_copy(&tmp, v)) {                        \
            mpz_clear(z);                              \
            return ZZ_MEM;                             \
        }                                              \
        mpz_clear(z);                                  \
        return ZZ_OK;                                  \
    }

#define TEST_UNOP_EXAMPLE(op, arg)                        \
    do {                                                  \
        zz_t u, v, r;                                     \
                                                          \
        if (zz_init(&u) || zz_init(&v)) {                 \
            abort();                                      \
        }                                                 \
        if (zz_copy(arg, &u)) {                           \
            abort();                                      \
        }                                                 \
        if (zz_init(&r) || zz_ref_##op(&u, &r)) {         \
            abort();                                      \
        }                                                 \
        if (zz_##op(&u, &v) || zz_cmp(&v, &r) != ZZ_EQ) { \
            abort();                                      \
        }                                                 \
        if (zz_copy(&u, &v) || zz_##op(&v, &v)            \
            || zz_cmp(&v, &r) != ZZ_EQ) {                 \
            abort();                                      \
        }                                                 \
        zz_clear(&u);                                     \
        zz_clear(&v);                                     \
        zz_clear(&r);                                     \
    } while (0);

typedef struct {
    char *u;
} zz_un_ex;

#define TEST_UNOP(op, ex, bs, neg, nex)                        \
    void                                                       \
    check_##op##_bulk(void)                                    \
    {                                                          \
        size_t ex_len = sizeof(ex)/sizeof(zz_un_ex);           \
                                                               \
        for (size_t i = 0; i < ex_len; i++) {                  \
            zz_t arg;                                          \
                                                               \
            if (zz_init(&arg) || zz_from_dec(ex[i].u, &arg)) { \
                abort();                                       \
            }                                                  \
            TEST_UNOP_EXAMPLE(op, &arg);                       \
            zz_clear(&arg);                                    \
        }                                                      \
        for (size_t i = 0; i < nex; i++) {                     \
            zz_t arg;                                          \
                                                               \
            if (zz_init(&arg) || zz_random(bs, neg, &arg)) {   \
                abort();                                       \
            }                                                  \
            TEST_UNOP_EXAMPLE(op, &arg);                       \
            zz_clear(&arg);                                    \
        }                                                      \
    }

#endif /* TESTS_TESTS_H */
