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

#define zz_sl_add(x, y, r) zz_add_sl((y), (x), (r))
#define zz_sl_mul(x, y, r) zz_mul_sl((y), (x), (r))

ZZ_MIXBINOP_REF(add)
ZZ_MIXBINOP_REF(sub)
ZZ_MIXBINOP_REF(mul)

zz_err
zz_fdiv_q(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_div(u, v, w, NULL);
}

zz_err
zz_fdiv_r(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_div(u, v, NULL, w);
}

zz_err
zz_fdiv_q_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    return zz_div_sl(u, v, w, NULL);
}

zz_err
zz_fdiv_r_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    return zz_div_sl(u, v, NULL, w);
}

zz_err
zz_sl_fdiv_q(zz_slimb_t u, const zz_t *v, zz_t *w)
{
    return zz_sl_div(u, v, w, NULL);
}

zz_err
zz_sl_fdiv_r(zz_slimb_t u, const zz_t *v, zz_t *w)
{
    return zz_sl_div(u, v, NULL, w);
}

ZZ_MIXBINOP_REF(fdiv_q)
ZZ_MIXBINOP_REF(fdiv_r)

ZZ_BINOP_REF(and)
#define zz_ior zz_or
ZZ_BINOP_REF(ior)
ZZ_BINOP_REF(xor)

zz_err
zz_gcd(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_gcdext(u, v, w, NULL, NULL);
}

ZZ_BINOP_REF(gcd)
ZZ_BINOP_REF(lcm)

zz_bin_ex examples [] = {{"1", "147573952589676412928"},
                         {"1", "-147573952589676412928"},
                         {"-2", "-1"},
                         {"-1", "-1"},
                         {"0", "-1"},
                         {"-1", "2"},
                         {"2", "0"}};

TEST_MIXBINOP(add, examples, 512, true, 1000000)
TEST_MIXBINOP(sub, examples, 512, true, 1000000)
TEST_MIXBINOP(mul, examples, 512, true, 1000000)

TEST_MIXBINOP(fdiv_q, examples, 512, true, 1000000)
TEST_MIXBINOP(fdiv_r, examples, 512, true, 1000000)

TEST_BINOP(and, examples, 512, true, 1000000)
TEST_BINOP(ior, examples, 512, true, 1000000)
TEST_BINOP(xor, examples, 512, true, 1000000)

TEST_BINOP(gcd, examples, 512, true, 1000000)
TEST_BINOP(lcm, examples, 512, true, 1000000)

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_testinit();
    zz_setup(NULL);
    check_add_bulk();
    check_sub_bulk();
    check_mul_bulk();
    check_fdiv_q_bulk();
    check_fdiv_r_bulk();
    check_and_bulk();
    check_ior_bulk();
    check_xor_bulk();
    check_gcd_bulk();
    check_lcm_bulk();
    zz_finish();
    return 0;
}
