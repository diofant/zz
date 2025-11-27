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

#include "zz.h"

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
check_add_sl()
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(0, &u)) {
        abort();
    }
    if (zz_add_sl(&u, 2, &u) || zz_cmp_sl(&u, 2) != ZZ_EQ) {
        abort();
    }
    if (zz_from_sl(0, &u) || zz_add_sl(&u, 0, &u)
        || zz_cmp_sl(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    zz_clear(&u);
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
check_mul()
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_sl(2, &u)) {
        abort();
    }
    if (zz_init(&v) || zz_from_sl(3, &v)) {
        abort();
    }
    if (zz_mul(&u, &v, &u) || zz_cmp_sl(&u, 6) != ZZ_EQ) {
        abort();
    }
    if (zz_mul(&u, &v, &v) || zz_cmp_sl(&v, 18) != ZZ_EQ) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
}

void
check_div()
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_sl(4, &u)) {
        abort();
    }
    if (zz_init(&v) || zz_from_sl(2, &v)) {
        abort();
    }
    if (zz_div(&u, &v, ZZ_RNDD, &v, NULL) || zz_cmp_sl(&v, 2) != ZZ_EQ) {
        abort();
    }
    if (zz_div(&u, &v, 123, &u, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_div(&u, &v, ZZ_RNDD, NULL, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
}

void
check_rem_ul()
{
    zz_t u;

    if (zz_init(&u) || zz_from_sl(123, &u)) {
        abort();
    }
    if (zz_rem_ul(&u, 0, ZZ_RNDD, NULL) != ZZ_VAL) {
        abort();
    }

    uint64_t val;

    if (zz_from_sl(111, &u) || zz_rem_ul(&u, 12, ZZ_RNDD, &val)
        || val != 3)
    {
        abort();
    }
    if (zz_from_sl(-111, &u) || zz_rem_ul(&u, 12, ZZ_RNDD, &val)
        || val != 9)
    {
        abort();
    }
    zz_clear(&u);
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
    zz_clear(&u);
    zz_clear(&v);
    zz_clear(&w);
}

int main(void)
{
    zz_info info;

    if (zz_setup(&info) || (info.limb_bytes != 4 && info.limb_bytes != 8)) {
        abort();
    }
    check_cmp_sl();
    check_cmp();
    check_add_sl();
    check_lsbpos();
    check_export();
    check_to_str();
    check_mul();
    check_div();
    check_rem_ul();
    check_quo_2exp();
    check_pow();
    check_sqrtrem();
    check_powm();
    zz_finish();
    return 0;
}
