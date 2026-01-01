/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tests/tests.h"

void
check_bytes_roundtrip(void)
{
    zz_bitcnt_t bs = 512;
    size_t nex = 1000000;

    for (size_t i = 0; i < nex; i++) {
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

static const zz_limb_t endian_test = ((zz_limb_t)(1)
                                      << (ZZ_LIMB_T_BITS-7)) - 1;

void
check_exportimport_roundtrip(void)
{
    zz_bitcnt_t bs = 512;
    size_t nex = 1000000;

    for (size_t i = 0; i < nex; i++) {
        zz_t u, v;

        if (zz_init(&u) || zz_random(bs, false, &u)) {
            abort();
        }

        size_t len = (zz_bitlen(&u) + 7)/8;
        void *buf = malloc(len);
        zz_layout layout = {8, 1, -1, (*(signed char *)&endian_test)};

        if (!buf || zz_export(&u, layout, len, buf)) {
            abort();
        }
        if (zz_init(&v) || zz_import(len, buf, layout, &v)) {
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

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);

    check_bytes_roundtrip();
    check_exportimport_roundtrip();
    zz_finish();
    return 0;
}
