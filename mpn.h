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

#endif /* MPN_H */
