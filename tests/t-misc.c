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
check_cmp(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_i64(13, &u)) {
        abort();
    }
    if (zz_cmp(&u, 1) != ZZ_GT || zz_cmp(&u, 100) != ZZ_LT) {
        abort();
    }
    if (zz_cmp(&u, -100) != ZZ_GT) {
        abort();
    }
    if (zz_from_i64(13, &u) || zz_cmp(&u, &u) != ZZ_EQ) {
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

    if (zz_init(&u) || zz_from_i64(0, &u)) {
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

    if (zz_init(&u) || zz_from_i64(4, &u)) {
        abort();
    }
    if (zz_init(&v) || zz_from_i64(0, &v)) {
        abort();
    }
    if (zz_sqrtrem(&u, &u, &v) || zz_cmp(&u, 2) != ZZ_EQ
        || zz_cmp(&v, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_sqrtrem(&v, &v, &u) || zz_cmp(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(-1, &u) || zz_sqrtrem(&u, &v, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
}

void
check_bin(void)
{
    zz_t u;

    if (zz_init(&u) || zz_bin(13, 5, &u) || zz_cmp(&u, 1287) != ZZ_EQ) {
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

    if (zz_init(&u) || zz_init(&v) || zz_from_i64(-2, &u)
        || zz_from_i64(6, &v))
    {
        abort();
    }
    if (zz_init(&a) || zz_gcdext(&u, &v, &a, NULL, NULL)
        || zz_cmp(&a, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, &a, NULL) || zz_cmp(&a, -1) != ZZ_EQ) {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, NULL, &a) || zz_cmp(&a, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_gcdext(&u, &v, &a, NULL, NULL)
        || zz_cmp(&a, 6) != ZZ_EQ)
    {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, &a, NULL) || zz_cmp(&a, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_gcdext(&u, &v, NULL, NULL, &a) || zz_cmp(&a, 1) != ZZ_EQ) {
        abort();
    }
    if (zz_init(&b) || zz_gcdext(&u, &v, &a, &b, NULL)
        || zz_cmp(&b, 0) != ZZ_EQ)
    {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
    zz_clear(&a);
    zz_clear(&b);
}

void
check_fromto_double(void)
{
    zz_t u;
    double d;

    if (zz_init(&u) || zz_from_double(INFINITY, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_from_double(1092.2666666666667, &u) || zz_cmp(&u, 1092) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(1, &u) || zz_mul_2exp(&u, 2000, &u)) {
        abort();
    }
    if (zz_to_double(&u, &d) != ZZ_BUF) {
        abort();
    }
    if (zz_from_i64(9007199254740993, &u) || zz_to_double(&u, &d)
        || d != 9007199254740992.0)
    {
        abort();
    }
    if (zz_from_i64(18014398509481987, &u) || zz_to_double(&u, &d)
        || d != 1.8014398509481988e+16)
    {
        abort();
    }
    if (zz_from_i64(1, &u) || zz_mul_2exp(&u, 1024, &u)
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

    if (zz_init(&u) || zz_from_i64(1, &u)
        || zz_sizeinbase(&u, 42, NULL) != ZZ_VAL)
    {
        abort();
    }
    zz_clear(&u);
}

void
check_fromto_i32(void)
{
    zz_t u;
    int32_t v = 123, val;

    if (zz_init(&u) || zz_from_i32(v, &u)) {
        abort();
    }
    if (zz_to_i32(&u, &val) || val != v) {
        abort();
    }
    v = -42;
    if (zz_from_i32(v, &u)) {
        abort();
    }
    if (zz_to_i32(&u, &val) || val != v) {
        abort();
    }
    v = 0;
    if (zz_from_i32(v, &u)) {
        abort();
    }
    if (zz_to_i32(&u, &val) || val != v) {
        abort();
    }
    if (zz_from_i64(1LL<<33, &u)) {
        abort();
    }
    if (zz_to_i32(&u, &val) != ZZ_VAL) {
        abort();
    }
    if (zz_from_i64(-(1LL<<33), &u)) {
        abort();
    }
    if (zz_to_i32(&u, &val) != ZZ_VAL) {
        abort();
    }
    if (zz_from_i32(1, &u) || zz_mul_2exp(&u, 65, &u)
        || zz_to_i32(&u, &val) != ZZ_VAL)
    {
        abort();
    }
    zz_clear(&u);
}

void
check_fromto_i64(void)
{
    zz_t u;
    int64_t val;

    if (zz_init(&u) || zz_from_i64(0, &u)) {
        abort();
    }
    if (zz_to_i64(&u, &val) || val) {
        abort();
    }
    zz_clear(&u);
}

void
check_fac_outofmem(void)
{
    zz_set_memory_funcs(my_malloc, my_realloc, my_free);
    max_size = 32*1000*1000;
    for (size_t i = 0; i < 7; i++) {
        uint64_t x = 12811 + (uint64_t)(rand() % 12173);
        zz_t mx;

        if (zz_init(&mx)) {
            abort();
        }
        while (1) {
            zz_err r = zz_fac(x, &mx);

            if (r != ZZ_OK) {
                if (r == ZZ_MEM) {
                    atomic_store(&total_size, 0);
                    break;
                }
                abort();
            }
            x *= 2;
        }
        zz_clear(&mx);
    }
    zz_set_memory_funcs(NULL, NULL, NULL);
}

int main(void)
{
    zz_testinit();

    zz_info info;

    if (zz_setup(&info) || (info.digit_bytes != 4 && info.digit_bytes != 8)) {
        abort();
    }
    check_cmp();
    check_cmp_bulk();
    check_lsbpos();
    check_sqrtrem();
    check_bin();
    check_isodd_bulk();
    check_gcdext();
    check_fromto_double();
    check_sizeinbase();
    check_fromto_i32();
    check_fromto_i64();
    check_fac_outofmem();
    zz_finish();
    zz_testclear();
    return 0;
}
