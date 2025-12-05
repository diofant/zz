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

#include "tests/tests.h"

static gmp_randstate_t rnd_state;

void _zz_testinit(void)
{
    gmp_randinit_default(rnd_state);
}

zz_err
_zz_random(zz_bitcnt_t bc, bool s, zz_t *u)
{
    mpz_t z;

    if (TMP_OVERFLOW) {
        return ZZ_MEM;
    }
    mpz_init(z);
    mpz_urandomb(z, rnd_state, bc);
    if (_zz_resize(ABS(z->_mp_size), u)) {
        mpz_clear(z);
        return ZZ_MEM;
    }
    mpn_copyi(u->digits, z->_mp_d, u->size);
    mpz_clear(z);
    u->negative = rand() % 2;
    return ZZ_OK;
}
