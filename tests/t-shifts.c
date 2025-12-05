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
#include <time.h>

#include "tests/tests.h"

zz_err
zz_ref_mul_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w)
{
    mpz_t z;
    TMP_MPZ(mu, u);
    if (TMP_OVERFLOW) {
        return ZZ_MEM;
    }
    mpz_init(z);
    mpz_mul_2exp(z, mu, v);

    zz_t tmp = {z->_mp_size < 0, ABS(z->_mp_size),
                ABS(z->_mp_size),
                z->_mp_d};
    if (zz_copy(&tmp, w)) {
        mpz_clear(z);
        return ZZ_MEM;
    }
    mpz_clear(z);
    return ZZ_OK;
}

zz_err
zz_ref_quo_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w)
{
    mpz_t z;
    TMP_MPZ(mu, u);
    if (TMP_OVERFLOW) {
        return ZZ_MEM;
    }
    mpz_init(z);
    mpz_fdiv_q_2exp(z, mu, v);

    zz_t tmp = {z->_mp_size < 0, ABS(z->_mp_size),
                ABS(z->_mp_size),
                z->_mp_d};
    if (zz_copy(&tmp, w)) {
        mpz_clear(z);
        return ZZ_MEM;
    }
    mpz_clear(z);
    return ZZ_OK;
}

void
check_lshift_bulk(void)
{
    zz_bitcnt_t bs = 512;
    size_t nex = 1000000;

    for (size_t i = 0; i < nex; i++) {
        zz_t u, w, r;
        zz_bitcnt_t v = (zz_bitcnt_t)rand() % 12345;

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }
        if (zz_init(&w) || zz_mul_2exp(&u, v, &w)) {
            abort();
        }
        if (zz_init(&r) || zz_ref_mul_2exp(&u, v, &r)
            || zz_cmp(&w, &r) != ZZ_EQ)
        {
            abort();
        }
        zz_clear(&u);
        zz_clear(&w);
        zz_clear(&r);
    }
}

void
check_rshift_bulk(void)
{
    zz_bitcnt_t bs = 512;
    size_t nex = 1000000;

    for (size_t i = 0; i < nex; i++) {
        zz_t u, w, r;
        zz_bitcnt_t v = (zz_bitcnt_t)rand();

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }
        if (zz_init(&w) || zz_quo_2exp(&u, v, &w)) {
            abort();
        }
        if (zz_init(&r) || zz_ref_quo_2exp(&u, v, &r)
            || zz_cmp(&w, &r) != ZZ_EQ)
        {
            abort();
        }
        zz_clear(&u);
        zz_clear(&w);
        zz_clear(&r);
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_lshift_bulk();
    check_rshift_bulk();
    zz_finish();
    return 0;
}
