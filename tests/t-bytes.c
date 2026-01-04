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
check_bytes_roundtrip(void)
{
    zz_bitcnt_t bs = 512;

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u, v;

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }

        size_t len = (zz_bitlen(&u) + 7)/8 + 1;
        unsigned char *buf = malloc(len);

        if (!buf || zz_to_bytes(&u, len, zz_isneg(&u), &buf)) {
            abort();
        }
        if (zz_init(&v) || zz_from_bytes(buf, len, zz_isneg(&u), &v)) {
            abort();
        }
        free(buf);
        if (zz_cmp(&u, &v) != ZZ_EQ) {
            abort();
        }
        zz_clear(&u);
        zz_clear(&v);
    }
}

void
check_bytes_examples(void)
{
    zz_t u;

    if (zz_init(&u) || zz_from_bytes(NULL, 0, false, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }

    unsigned char *buf = malloc(1);

    if (zz_from_i64(1, &u) || zz_mul_2exp(&u, 64, &u) || zz_neg(&u, &u)
        || zz_to_bytes(&u, 1, true, &buf) != ZZ_BUF)
    {
        abort();
    }
    if (zz_to_bytes(&u, 1, false, &buf) != ZZ_BUF) {
        abort();
    }
    if (zz_from_i64(1, &u) || zz_mul_2exp(&u, 64, &u)
        || zz_to_bytes(&u, 1, true, &buf) != ZZ_BUF)
    {
        abort();
    }
    free(buf);
    zz_clear(&u);
}

static const uint64_t endian_test = ((uint64_t)(1) << 57) - 1;

void
check_exportimport_roundtrip(void)
{
    zz_bitcnt_t bs = 512;
    const zz_layout bytes_layout = {8, 1, -1, (*(signed char *)&endian_test)};

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u, v;

        if (zz_init(&u) || zz_random(bs, false, &u)) {
            abort();
        }

        size_t len = (zz_bitlen(&u) + 7)/8;
        void *buf = malloc(len);

        if (!buf || zz_export(&u, bytes_layout, len, buf)) {
            abort();
        }
        if (zz_init(&v) || zz_import(len, buf, bytes_layout, &v)) {
            abort();
        }
        free(buf);
        if (zz_cmp(&u, &v) != ZZ_EQ) {
            abort();
        }
        zz_clear(&u);
        zz_clear(&v);
    }
}

void
check_exportimport_examples(void)
{
    zz_t u;
    const zz_layout pyint_layout = {30, 4, -1, (*(signed char *)&endian_test)};

    if (zz_init(&u) || zz_from_i64(123, &u)) {
        abort();
    }
    if (zz_export(&u, pyint_layout, 0, 0) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
}

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_bytes_roundtrip();
    check_bytes_examples();
    check_exportimport_roundtrip();
    check_exportimport_examples();
    zz_finish();
    zz_testclear();
    return 0;
}
