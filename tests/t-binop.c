/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>

#include "tests/tests.h"

typedef struct {
    char *u;
    char *v;
} zz_bin_ex;

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
        if (zz_init(&u) || zz_init(&v) || zz_init(&r)         \
            || zz_init(&w))                                   \
        {                                                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(lhs, &u) || zz_copy(rhs, &v)) {           \
            abort();                                          \
        }                                                     \
                                                              \
        zz_err ret = zz_##op(&u, &v, &w);                     \
                                                              \
        if (ret == ZZ_VAL) {                                  \
            zz_clear(&u);                                     \
            zz_clear(&v);                                     \
            zz_clear(&w);                                     \
            zz_clear(&r);                                     \
            continue;                                         \
        }                                                     \
        else if (ret != ZZ_OK) {                              \
            abort();                                          \
        }                                                     \
        if (zz_ref_##op(&u, &v, &r)                           \
            || zz_cmp(&w, &r) != ZZ_EQ)                       \
        {                                                     \
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

#define TEST_MIXBINOP_EXAMPLE(op, lhs, rhs)                   \
    TEST_BINOP_EXAMPLE(op, lhs, rhs)                          \
    do {                                                      \
        zz_t u, v, w, r;                                      \
                                                              \
        if (zz_init(&u) || zz_init(&v) || zz_init(&w)         \
            || zz_init(&r))                                   \
        {                                                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(lhs, &u) || zz_copy(rhs, &v)) {           \
            abort();                                          \
        }                                                     \
                                                              \
        zz_slimb_t limb;                                      \
                                                              \
        if (zz_to_sl(&v, &limb) == ZZ_OK) {                   \
            zz_err ret = zz_##op##_sl(&u, limb, &w);          \
                                                              \
            if (ret == ZZ_VAL) {                              \
                zz_clear(&u);                                 \
                zz_clear(&v);                                 \
                zz_clear(&w);                                 \
                zz_clear(&r);                                 \
                continue;                                     \
            }                                                 \
            else if (ret) {                                   \
                abort();                                      \
            }                                                 \
            if (zz_ref_##op##_sl(&u, limb, &r)                \
                || zz_cmp(&w, &r) != ZZ_EQ)                   \
            {                                                 \
                abort();                                      \
            }                                                 \
        }                                                     \
        if (zz_to_sl(&u, &limb) == ZZ_OK) {                   \
            zz_err ret = zz_sl_##op(limb, &v, &w);            \
                                                              \
            if (ret == ZZ_VAL) {                              \
                zz_clear(&u);                                 \
                zz_clear(&v);                                 \
                zz_clear(&w);                                 \
                zz_clear(&r);                                 \
                continue;                                     \
            }                                                 \
            else if (ret) {                                   \
                abort();                                      \
            }                                                 \
            if (zz_ref_sl_##op(limb, &v, &r)                  \
                || zz_cmp(&w, &r) != ZZ_EQ)                   \
            {                                                 \
                abort();                                      \
            }                                                 \
        }                                                     \
        zz_clear(&u);                                         \
        zz_clear(&v);                                         \
        zz_clear(&w);                                         \
        zz_clear(&r);                                         \
    } while (0);

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

#define zz_sl_add(x, y, r) zz_add_sl((y), (x), (r))
#define zz_sl_mul(x, y, r) zz_mul_sl((y), (x), (r))

ZZ_MIXBINOP_REF(add)
ZZ_MIXBINOP_REF(sub)
ZZ_MIXBINOP_REF(mul)

zz_err
zz_fdiv_q(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_div(u, v, w, NULL);
}

zz_err
zz_fdiv_r(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_div(u, v, NULL, w);
}

zz_err
zz_fdiv_q_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    return zz_div_sl(u, v, w, NULL);
}

zz_err
zz_fdiv_r_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    return zz_div_sl(u, v, NULL, w);
}

zz_err
zz_sl_fdiv_q(zz_slimb_t u, const zz_t *v, zz_t *w)
{
    return zz_sl_div(u, v, w, NULL);
}

zz_err
zz_sl_fdiv_r(zz_slimb_t u, const zz_t *v, zz_t *w)
{
    return zz_sl_div(u, v, NULL, w);
}

ZZ_MIXBINOP_REF(fdiv_q)
ZZ_MIXBINOP_REF(fdiv_r)

ZZ_BINOP_REF(and)
#define zz_ior zz_or
ZZ_BINOP_REF(ior)
ZZ_BINOP_REF(xor)

zz_err
zz_gcd(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_gcdext(u, v, w, NULL, NULL);
}

ZZ_BINOP_REF(gcd)
ZZ_BINOP_REF(lcm)

zz_bin_ex examples [] = {{"1", "147573952589676412928"},
                         {"1", "-147573952589676412928"},
                         {"-2", "-1"},
                         {"-1", "-1"},
                         {"0", "-1"},
                         {"-1", "2"},
                         {"2", "0"},
                         {"0", "0"}};

TEST_MIXBINOP(add, examples, 512, true, 1000000)
TEST_MIXBINOP(sub, examples, 512, true, 1000000)
TEST_MIXBINOP(mul, examples, 512, true, 1000000)

TEST_MIXBINOP(fdiv_q, examples, 512, true, 1000000)
TEST_MIXBINOP(fdiv_r, examples, 512, true, 1000000)

TEST_BINOP(and, examples, 512, true, 1000000)
TEST_BINOP(ior, examples, 512, true, 1000000)
TEST_BINOP(xor, examples, 512, true, 1000000)

TEST_BINOP(gcd, examples, 512, true, 1000000)
TEST_BINOP(lcm, examples, 512, true, 1000000)

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_add_bulk();
    check_sub_bulk();
    check_mul_bulk();
    check_fdiv_q_bulk();
    check_fdiv_r_bulk();
    check_and_bulk();
    check_ior_bulk();
    check_xor_bulk();
    check_gcd_bulk();
    check_lcm_bulk();
    zz_finish();
    return 0;
}
