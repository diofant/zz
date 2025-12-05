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
#include <sys/resource.h>
#include <time.h>

#include "tests/tests.h"

ZZ_UNOP_REF(neg)
ZZ_UNOP_REF(abs)

#define zz_com zz_invert
ZZ_UNOP_REF(com)

zz_err
zz_sqrt(const zz_t *u, zz_t *v)
{
    return zz_sqrtrem(u, v, NULL);
}
ZZ_UNOP_REF(sqrt)

zz_un_ex examples[] = {{"147573952589676412928"},
                       {"-147573952589676412928"},
                       {"-1"},
                       {"2"},
                       {"0"}};

TEST_UNOP(neg, examples, 512, true, 1000000)
TEST_UNOP(abs, examples, 512, true, 1000000)
TEST_UNOP(com, examples, 512, true, 1000000)

zz_un_ex sqrt_examples[] = {{"147573952589676412928"}, {"2"}, {"0"}};

TEST_UNOP(sqrt, sqrt_examples, 512, false, 1000000)

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_neg_bulk();
    check_abs_bulk();
    check_com_bulk();
    check_sqrt_bulk();
    zz_finish();
    return 0;
}
