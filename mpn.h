/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#ifndef MPN_H
#define MPN_H

/* Export {z, zsize} as array of words data.  The parameters specify the format
   of the data produced.  Same meaning as for mpz_export(). */
void * mpn_export(void *data, size_t *countp, int order,
                  size_t size, int endian, size_t nail, mp_srcptr z,
                  mp_size_t zsize);
/* Set {zp, zsize} from an array of word data at data.  The parameters specify
   the format of the data produced.  Same meaning as for mpz_import(). */
void mpn_import(mp_ptr zp, mp_size_t *zsize, size_t count, int order,
                size_t size, int endian, size_t nail, const void *data);

/* Compute r = b^e mod m.  Requires that m is odd and e > 1.
   Uses scratch space at tp of MAX(mpn_binvert_itch(n), 2n) limbs. */
void mpn_powm(mp_limb_t *rp, const mp_limb_t *bp, mp_size_t bn,
              const mp_limb_t *ep, mp_size_t en, const mp_limb_t *mp,
              mp_size_t n, mp_limb_t *tp);
mp_size_t mpn_binvert_itch(mp_size_t n);

/* Compute r = b^e mod B^n, B is the limb base.
   Requires normalized e.  Uses scratch space of 3n words in tp. */
void mpn_powlo(mp_limb_t *rp, const mp_limb_t *bp,
               const mp_limb_t *ep, mp_size_t en, mp_size_t n,
               mp_limb_t *tp);

/* Compute r = u^(-1) mod B^n, B is the limb base. */
void mpn_binvert(mp_limb_t *rp, const mp_limb_t *up, mp_size_t n,
                 mp_limb_t *scratch);

/* Multiply two n-limb numbers and return the low n limbs of their products. */
void mpn_mullo_n(mp_limb_t *rp, const mp_limb_t *xp,
                 const mp_limb_t *yp, mp_size_t n);

/* Return the value {up,size}*2^exp, and negative if sign<0.  Must have
   a non-zero high limb up[size-1]. */
double mpn_get_d(const mp_limb_t *up, mp_size_t size, mp_size_t sign, long exp);

#endif /* MPN_H */
