/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#ifndef IMPL_ZZ_H
#define IMPL_ZZ_H

#include <setjmp.h>

extern _Thread_local jmp_buf zz_env;
/* Function should include if(TMP_OVERFLOW){...} workaround in
   case it calls any mpn_*() API, which does memory allocation for
   temporary storage.  Not all functions do this, sometimes it's
   obvious (e.g. mpn_cmp() or mpn_add/sub()), sometimes - not (e.g.
   mpn_get/set_str() for power of 2 bases).  Though, these details
   aren't documented and if you feel that in the given case things
   might be changed - please add a workaround. */
#define TMP_OVERFLOW (setjmp(zz_env) == 1)

zz_err _zz_resize(int64_t size, zz_t *u);

#define TMP_MPZ(z, u)                               \
    mpz_t z;                                        \
                                                    \
    z->_mp_d = u->digits;                           \
    z->_mp_size = (u->negative ? -1 : 1) * u->size; \
    z->_mp_alloc = u->alloc;

#endif /* IMPL_ZZ_H */
