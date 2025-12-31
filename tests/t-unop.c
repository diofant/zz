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

ZZ_UNOP_REF(neg)
ZZ_UNOP_REF(abs)

#define zz_com zz_invert
ZZ_UNOP_REF(com)

zz_err
zz_sqrt(const zz_t *u, zz_t *v)
{
    return zz_sqrtrem(u, v, NULL);
}
ZZ_UNOP_REF(sqrt)

zz_un_ex examples[] = {{"147573952589676412928"},
                       {"-147573952589676412928"},
                       {"-1"},
                       {"2"},
                       {"0"}};

TEST_UNOP(neg, examples, 512, true, 1000000)
TEST_UNOP(abs, examples, 512, true, 1000000)
TEST_UNOP(com, examples, 512, true, 1000000)

zz_un_ex sqrt_examples[] = {{"147573952589676412928"}, {"2"}, {"0"}};

TEST_UNOP(sqrt, sqrt_examples, 512, false, 1000000)

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_neg_bulk();
    check_abs_bulk();
    check_com_bulk();
    check_sqrt_bulk();
    zz_finish();
    return 0;
}
