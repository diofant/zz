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
        if (!buf || zz_to_str(&u, base, buf, &len)) {
            abort();
        }

        zz_t v;

        if (zz_init(&v) || zz_from_str(buf, len, abs(base), &v)
            || zz_cmp(&u, &v) != ZZ_EQ)
        {
            abort();
        }
        free(buf);
        zz_clear(&u);
        zz_clear(&v);
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_str_roundtrip();
    zz_finish();
    zz_testclear();
    return 0;
}
