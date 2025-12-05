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

ZZ_BINOP_REF(add)
ZZ_BINOP_REF(sub)
ZZ_BINOP_REF(mul)

ZZ_BINOP_REF(and)
#define zz_ior zz_or
ZZ_BINOP_REF(ior)
ZZ_BINOP_REF(xor)

_zz_bin_ex examples [] = {{"1", "147573952589676412928"},
                          {"1", "-147573952589676412928"},
                          {"-2", "-1"},
                          {"-1", "-1"},
                          {"0", "-1"},
                          {"-1", "2"},
                          {"2", "0"},
                          {"0", "0"}};

TEST_BINOP(add, examples, 512, true, 100000)
TEST_BINOP(sub, examples, 512, true, 100000)
TEST_BINOP(mul, examples, 512, true, 100000)

TEST_BINOP(and, examples, 512, true, 100000)
TEST_BINOP(ior, examples, 512, true, 100000)
TEST_BINOP(xor, examples, 512, true, 100000)

int main(void)
{
    srand((unsigned int)time(NULL));
    _zz_testinit();
    zz_setup(NULL);
    check_add_bulk();
    check_sub_bulk();
    check_mul_bulk();
    check_and_bulk();
    check_ior_bulk();
    check_xor_bulk();
    zz_finish();
    return 0;
}
