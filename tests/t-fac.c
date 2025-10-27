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
#include <sys/resource.h>
#include <time.h>

#include "zz.h"

void check_fac_outofmem(void)
{
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
                    break;
                }
                abort();
            }
            x *= 2;
        }
        zz_clear(&mx);
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_setup(NULL);

    struct rlimit new, old;

    if (getrlimit(RLIMIT_AS, &old)) {
        perror("getrlimit");
        return 1;
    }
    new.rlim_max = old.rlim_max;
    new.rlim_cur = 32*1000*1000;
    if (setrlimit(RLIMIT_AS, &new)) {
        perror("setrlimit");
        return 1;
    }
    check_fac_outofmem();
    return 0;
}
