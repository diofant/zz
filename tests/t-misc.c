/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include "tests/tests.h"

void
check_cmp_sl(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(13, &u)) {
        abort();
    }
    if (zz_cmp_sl(&u, 1) != ZZ_GT || zz_cmp_sl(&u, 100) != ZZ_LT) {
        abort();
    }
    if (zz_cmp_sl(&u, -100) != ZZ_GT) {
        abort();
    }
    zz_clear(&u);
}

void
check_cmp(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(13, &u)) {
        abort();
    }
    if (zz_cmp(&u, &u) != ZZ_EQ) {
        abort();
    }
    zz_clear(&u);
}

void
check_cmp_bulk(void)
{
    zz_bitcnt_t bs = 512;

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u, v;

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }
        if (zz_init(&v) || zz_random(bs, true, &v)) {
            abort();
        }

        TMP_MPZ(mu, &u);
        TMP_MPZ(mv, &v);
        if (zz_cmp(&u, &v) != mpz_cmp(mu, mv)) {
            abort();
        }
        zz_clear(&u);
        zz_clear(&v);
    }
}

void
check_lsbpos(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(0, &u)) {
        abort();
    }
    if (zz_lsbpos(&u) != 0) {
        abort();
    }
    zz_clear(&u);
}

void
check_sqrtrem(void)
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_sl(4, &u)) {
        abort();
    }
    if (zz_init(&v) || zz_from_sl(0, &v)) {
        abort();
    }
    if (zz_sqrtrem(&u, &u, &v) || zz_cmp_sl(&u, 2) != ZZ_EQ
        || zz_cmp_sl(&v, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_sqrtrem(&v, &v, &u) || zz_cmp_sl(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_sl(-1, &u) || zz_sqrtrem(&u, &v, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
}

void
check_bin(void)
{
    zz_t u;

    if (zz_init(&u) || zz_bin(13, 5, &u) || zz_cmp_sl(&u, 1287) != ZZ_EQ) {
        abort();
    }
    zz_clear(&u);
}

void
check_isodd_bulk(void)
{
    zz_bitcnt_t bs = 512;

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u;

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }

        TMP_MPZ(mu, &u);
        if (zz_isodd(&u) != mpz_odd_p(mu)) {
            abort();
        }
        zz_clear(&u);
    }
}

void
check_gcdext(void)
{
    zz_t u, v, a, b;

    if (zz_init(&u) || zz_init(&v) || zz_from_sl(-2, &u)
        || zz_from_sl(6, &v))
    {
        abort();
    }
    if (zz_init(&a) || zz_gcdext(&u, &v, &a, NULL, NULL)
        || zz_cmp_sl(&a, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, &a, NULL) || zz_cmp_sl(&a, -1) != ZZ_EQ) {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, NULL, &a) || zz_cmp_sl(&a, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_sl(0, &u) || zz_gcdext(&u, &v, &a, NULL, NULL)
        || zz_cmp_sl(&a, 6) != ZZ_EQ)
    {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, &a, NULL) || zz_cmp_sl(&a, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, NULL, &a) || zz_cmp_sl(&a, 1) != ZZ_EQ) {
        abort();
    }
    if (zz_init(&b) || zz_gcdext(&u, &v, &a, &b, NULL)
        || zz_cmp_sl(&b, 0) != ZZ_EQ)
    {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
    zz_clear(&a);
    zz_clear(&b);
}

void
check_to_double(void)
{
    zz_t u;
    double d;

    if (zz_init(&u) || zz_from_sl(1, &u) || zz_mul_2exp(&u, 2000, &u)) {
        abort();
    }
    if (zz_to_double(&u, &d) != ZZ_BUF) {
        abort();
    }
    if (zz_from_sl(9007199254740993, &u) || zz_to_double(&u, &d)
        || d != 9007199254740992.0)
    {
        abort();
    }
    if (zz_from_sl(18014398509481987, &u) || zz_to_double(&u, &d)
        || d != 1.8014398509481988e+16)
    {
        abort();
    }
    if (zz_from_sl(1, &u) || zz_mul_2exp(&u, 1024, &u)
        || zz_to_double(&u, &d) != ZZ_BUF)
    {
        abort();
    }
    zz_clear(&u);
}

void
check_sizeinbase(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(1, &u)
        || zz_sizeinbase(&u, 42, NULL) != ZZ_VAL)
    {
        abort();
    }
    zz_clear(&u);
}

void
check_to_sl(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(0, &u)) {
        abort();
    }

    zz_slimb_t limb;

    if (zz_to_sl(&u, &limb) || limb) {
        abort();
    }
    zz_clear(&u);
}

int main(void)
{
    zz_testinit();

    zz_info info;

    if (zz_setup(&info) || (info.limb_bytes != 4 && info.limb_bytes != 8)) {
        abort();
    }
    check_cmp_sl();
    check_cmp();
    check_cmp_bulk();
    check_lsbpos();
    check_sqrtrem();
    check_bin();
    check_isodd_bulk();
    check_gcdext();
    check_to_double();
    check_sizeinbase();
    check_to_sl();
    zz_finish();
    zz_testclear();
    return 0;
}
