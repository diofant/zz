/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

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
    size_t nex = 1000000;

    for (size_t i = 0; i < nex; i++) {
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
check_lsbpos()
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

static zz_layout int_layout = {30, 4, -1, -1};

void
check_export()
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(123, &u)) {
        abort();
    }
    if (zz_export(&u, int_layout, 0, 0) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
}

void
check_to_str()
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(123, &u)) {
        abort();
    }
    if (zz_to_str(&u, 38, NULL, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
}

void
check_from_str()
{
    zz_t u;

    if (zz_init(&u) || zz_from_str((int8_t *)" ", 1, 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_init(&u) || zz_from_str((int8_t *)"-", 1, 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_from_str((int8_t *)"_", 1, 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_from_str((int8_t *)"1__", 3, 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_from_str((int8_t *)"1_3", 3, 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_from_str((int8_t *)" ", 1, 42, &u) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
}

void
check_div(void)
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_sl(4, &u)) {
        abort();
    }
    if (zz_init(&v) || zz_from_sl(2, &v)) {
        abort();
    }
    if (zz_div(&u, &v, NULL, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_from_sl(0, &v) || zz_div(&u, &v, &v, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
}

void
check_div_sl()
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(1, &u)) {
        abort();
    }
    if (zz_div_sl(&u, 0, &u, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
}

void
check_sl_div()
{
    zz_t v;

    if (zz_init(&v) || zz_from_sl(0, &v)) {
        abort();
    }
    if (zz_sl_div(1, &v, &v, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_from_sl(1, &v) || zz_sl_div(1, &v, NULL, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&v);
}

void
check_quo_2exp()
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_sl(0x7fffffffffffffffLL, &u)) {
        abort();
    }
    if (zz_mul_2exp(&u, 1, &u) || zz_add_sl(&u, 1, &u)
        || zz_mul_2exp(&u, 64, &u) || zz_quo_2exp(&u, 64, &u))
    {
        abort();
    }
    if (u.negative || u.alloc < 1 || u.size != 1
        || u.digits[0] != 0xffffffffffffffffULL)
    {
        abort();
    }
    if (zz_init(&v) || zz_from_sl(0x7fffffffffffffffLL, &v)) {
        abort();
    }
    if (zz_mul_2exp(&v, 1, &v) || zz_add_sl(&v, 1, &v)
        || zz_cmp(&u, &v) != ZZ_EQ)
    {
        abort();
    }
#if ZZ_LIMB_T_BITS == 64
    if (zz_from_sl(1, &u) || zz_mul_2exp(&u, 64, &u)
        || zz_pow(&u, ((zz_limb_t)1<<63), &u) != ZZ_BUF)
    {
        abort();
    }
#endif
    zz_clear(&u);
    zz_clear(&v);
}

void
check_pow()
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(2, &u)) {
        abort();
    }
    if (zz_pow(&u, 2, &u) || zz_cmp_sl(&u, 4) != ZZ_EQ) {
        abort();
    }
    if (zz_pow(&u, 0, &u) || zz_cmp_sl(&u, 1) != ZZ_EQ) {
        abort();
    }
    if (zz_pow(&u, 123, &u) || zz_cmp_sl(&u, 1) != ZZ_EQ) {
        abort();
    }
    if (zz_from_sl(0, &u) || zz_pow(&u, 123, &u) || zz_cmp_sl(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    zz_clear(&u);
}

void
check_sqrtrem()
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
check_powm()
{
    zz_t u, v, w;

    if (zz_init(&u) || zz_from_sl(12, &u)) {
        abort();
    }
    if (zz_init(&v) || zz_from_sl(4, &v)) {
        abort();
    }
    if (zz_init(&w) || zz_from_sl(7, &w)) {
        abort();
    }
    if (zz_powm(&u, &v, &w, &u) || zz_cmp_sl(&u, 2) != ZZ_EQ) {
        abort();
    }
    if (zz_from_sl(12, &u) || zz_powm(&u, &v, &w, &v)
        || zz_cmp_sl(&v, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_sl(4, &v) || zz_powm(&u, &v, &w, &w)
        || zz_cmp_sl(&w, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_sl(0, &w) || zz_powm(&u, &v, &w, &w) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
    zz_clear(&w);
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
check_bytes(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_bytes(NULL, 0, false, &u)
        || zz_cmp_sl(&u, 0) != ZZ_EQ)
    {
        abort();
    }

    uint8_t *buf = malloc(1);

    if (zz_from_sl(1, &u) || zz_mul_2exp(&u, 64, &u) || zz_neg(&u, &u)
        || zz_to_bytes(&u, 1, true, &buf) != ZZ_BUF)
    {
        abort();
    }
    if (zz_to_bytes(&u, 1, false, &buf) != ZZ_BUF) {
        abort();
    }
    if (zz_from_sl(1, &u) || zz_mul_2exp(&u, 64, &u)
        || zz_to_bytes(&u, 1, true, &buf) != ZZ_BUF)
    {
        abort();
    }
    free(buf);
    zz_clear(&u);
}

void
check_isodd_bulk(void)
{
    zz_bitcnt_t bs = 512;
    size_t nex = 1000000;

    for (size_t i = 0; i < nex; i++) {
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
check_shifts(void)
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_sl(0, &u) || zz_init(&v)) {
        abort();
    }
    if (zz_mul_2exp(&u, 123, &v) || zz_cmp_sl(&v, 0)) {
        abort();
    }
    if (zz_quo_2exp(&u, 123, &v) || zz_cmp_sl(&v, 0)) {
        abort();
    }
    if (zz_from_dec("-340282366920938463444927863358058659840", &u)
        || zz_quo_2exp(&u, 64, &v))
    {
        abort();
    }
    if (zz_from_dec("-18446744073709551615", &u)
        || zz_cmp(&u, &v) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_dec("-514220174162876888173427869549172"
                    "032807104958010493707296440352", &u)
        || zz_quo_2exp(&u, 206, &v) || zz_cmp_sl(&v, -6) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_dec("-62771017353866807634955070562867279"
                    "52638980837032266301441", &u)
        || zz_quo_2exp(&u, 128, &v))
    {
        abort();
    }
    if (zz_from_dec("-18446744073709551616", &u) || zz_cmp(&u, &v)) {
        abort();
    }
    if (zz_from_sl(-1, &u) || zz_quo_2exp(&u, 1, &v)
        || zz_cmp_sl(&v, -1) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_sl(1, &u) ||
        zz_mul_2exp(&u, ZZ_BITS_MAX, &u) != ZZ_MEM)
    {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
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
    check_export();
    check_to_str();
    check_from_str();
    check_div();
    check_div_sl();
    check_sl_div();
    check_quo_2exp();
    check_pow();
    check_sqrtrem();
    check_powm();
    check_bin();
    check_bytes();
    check_isodd_bulk();
    check_gcdext();
    check_to_double();
    check_shifts();
    check_sizeinbase();
    zz_finish();
    return 0;
}
