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
check_str_roundtrip(void)
{
    zz_bitcnt_t bs = 512;

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u;

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }

        int base = 2 + (char)(rand() % 35);
        size_t len;

        (void)zz_sizeinbase(&u, base, &len);

        char *buf = malloc(len + 2);

        if (rand() % 2) {
            base = -base;
        }
        if (!buf || zz_get_str(&u, base, 0, buf)) {
            abort();
        }

        zz_t v;

        if (zz_init(&v) || zz_set_str(buf, abs(base), &v)
            || zz_cmp(&u, &v) != ZZ_EQ)
        {
            abort();
        }
        free(buf);
        zz_clear(&u);
        zz_clear(&v);
    }
}

void
check_str_examples(void)
{
    zz_t u;

    if (zz_init(&u) || zz_set(123, &u)) {
        abort();
    }
    if (zz_get_str(&u, 38, 0, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str(" ", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("-", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("-+", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("+", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("_", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("1__", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("1_3", 2, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str(" ", 42, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str(" +123", 10, &u) || zz_cmp(&u, 123) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("  -123", 10, &u) || zz_cmp(&u, -123) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("123   ", 10, &u) || zz_cmp(&u, 123) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str(" 123   ", 10, &u) || zz_cmp(&u, 123) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str(" 123 321", 10, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("0b11", 0, &u) || zz_cmp(&u, 3) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("0b11", 2, &u) || zz_cmp(&u, 3) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("-0b111", 0, &u) || zz_cmp(&u, -7) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("-0o11", 0, &u) || zz_cmp(&u, -9) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("0x11", 0, &u) || zz_cmp(&u, 17) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("-0x11", 16, &u) || zz_cmp(&u, -17) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("01", 0, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("0x", 0, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("0x__0", 0, &u) != ZZ_VAL) {
        abort();
    }
    if (zz_set_str("0", 0, &u) || zz_cmp(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_set_str("-123", 0, &u) || zz_cmp(&u, -123) != ZZ_EQ) {
        abort();
    }

    char buf[10];

    if (zz_set(0, &u) || zz_get_str(&u, 2, 0, buf) || strcmp(buf, "0")) {
        abort();
    }
    zz_clear(&u);
}

void
check_str_grouping(void)
{
    zz_t u;

    if (zz_init(&u) || zz_set_str("489730051447264218653321153308096162461",
                                  0, &u))
    {
        abort();
    }

    char buf[100];

    if (zz_get_str(&u, 10, 3, buf)
        || strcmp(buf, "489_730_051_447_264_218_653_321_153_308_096_162_461"))
    {
        abort();
    }
    if (zz_get_str(&u, 16, 4, buf)
        || strcmp(buf, "1_706e_93bb_2d8d_c616_4ed4_0f22_ddd7_6a9d"))
    {
        abort();
    }
    if (zz_set_str("123", 0, &u) || zz_get_str(&u, 10, 3, buf)
        || strcmp(buf, "123"))
    {
        abort();
    }
    if (zz_get_str(&u, 10, 4, buf) || strcmp(buf, "123")) {
        abort();
    }
    zz_clear(&u);
}

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup();
    check_str_roundtrip();
    check_str_examples();
    check_str_grouping();
    zz_finish();
    zz_testclear();
    return 0;
}
