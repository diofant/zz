/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include <assert.h>

#if defined(__MINGW32__) && defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#endif
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include <gmp.h>
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
#if defined(__MINGW32__) && defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

#define CNST_LIMB(C) ((mp_limb_t) C##LL)

#if HAVE_LIMB_BIG_ENDIAN
#define HOST_ENDIAN     1
#endif
#if HAVE_LIMB_LITTLE_ENDIAN
#define HOST_ENDIAN     (-1)
#endif
#ifndef HOST_ENDIAN
static const mp_limb_t  endian_test = (CNST_LIMB(1) << (GMP_LIMB_BITS-7)) - 1;
#define HOST_ENDIAN     (* (signed char *) &endian_test)
#endif

#define ASSERT(expr) do { assert(expr); } while(0)
#define ASSERT_ALWAYS ASSERT
#define ASSERT_ALWAYS_LIMB(limb)                    \
    do {                                            \
        mp_limb_t  __nail = (limb) & GMP_NAIL_MASK; \
        ASSERT_ALWAYS (__nail == 0);                \
    } while (0)
#define ASSERT_LIMB ASSERT_ALWAYS_LIMB
#define ASSERT_ALWAYS_MPN(ptr, size)               \
    do {                                           \
        /* let whole loop go dead when no nails */ \
        if (GMP_NAIL_BITS != 0) {                  \
            mp_size_t  __i;                        \
            for (__i = 0; __i < (size); __i++) {   \
                ASSERT_ALWAYS_LIMB ((ptr)[__i]);   \
            }                                      \
        } \
    } while (0)

#define BITS_TO_LIMBS(n)  (((n) + (GMP_NUMB_BITS - 1)) / GMP_NUMB_BITS)

#define MPN_OVERLAP_P(xp, xsize, yp, ysize)	\
  ((xp) + (xsize) > (yp) && (yp) + (ysize) > (xp))

#define MPN_SAME_OR_SEPARATE_P(xp, yp, size) \
  MPN_SAME_OR_SEPARATE2_P(xp, size, yp, size)
#define MPN_SAME_OR_SEPARATE2_P(xp, xsize, yp, ysize) \
  ((xp) == (yp) || ! MPN_OVERLAP_P (xp, xsize, yp, ysize))

#define MPN_NORMALIZE(DST, NLIMBS)          \
    do {                                    \
        while ((NLIMBS) > 0) {              \
            if ((DST)[(NLIMBS) - 1] != 0) { \
                break;                      \
            }                               \
            (NLIMBS)--;                     \
        }                                   \
    } while (0)

#define BSWAP_LIMB(dst, src)                   \
    do {                                       \
        (dst) =	(((src) << 56)                 \
                 + (((src) & 0xFF00) << 40)    \
                 + (((src) & 0xFF0000) << 24)  \
                 + (((src) & 0xFF000000) << 8) \
                 + (((src) >> 8) & 0xFF000000) \
                 + (((src) >> 24) & 0xFF0000)  \
                 + (((src) >> 40) & 0xFF00)    \
                 + ((src) >> 56));             \
    } while (0)

#define BSWAP_LIMB_FETCH(limb, src)  BSWAP_LIMB (limb, *(src))

#define MPN_BSWAP(dst, src, size)                         \
    do {                                                  \
        mp_ptr     __dst = (dst);                         \
        mp_srcptr  __src = (src);                         \
        mp_size_t  __size = (size);                       \
        mp_size_t  __i;                                   \
        ASSERT ((size) >= 0);                             \
        ASSERT (MPN_SAME_OR_SEPARATE_P (dst, src, size)); \
        for (__i = 0; __i < __size; __i++) {              \
            BSWAP_LIMB_FETCH (*__dst, __src);             \
            __dst++;                                      \
            __src++;                                      \
        }                                                 \
    } while (0)

#define MPN_BSWAP_REVERSE(dst, src, size)                \
    do {                                                 \
        mp_ptr     __dst = (dst);                        \
        mp_size_t  __size = (size);                      \
        mp_srcptr  __src = (src) + __size - 1;           \
        mp_size_t  __i;                                  \
        ASSERT ((size) >= 0);                            \
        ASSERT (! MPN_OVERLAP_P (dst, size, src, size)); \
        for (__i = 0; __i < __size; __i++) {             \
            BSWAP_LIMB_FETCH (*__dst, __src);            \
            __dst++;                                     \
            __src--;                                     \
        }                                                \
    } while (0)

#define MPN_REVERSE(dst, src, size)                      \
    do {                                                 \
        mp_ptr     __dst = (dst);                        \
        mp_size_t  __size = (size);                      \
        mp_srcptr  __src = (src) + __size - 1;           \
        mp_size_t  __i;                                  \
        ASSERT ((size) >= 0);                            \
        ASSERT (! MPN_OVERLAP_P (dst, size, src, size)); \
        for (__i = 0; __i < __size; __i++) {             \
            *__dst = *__src;                             \
            __dst++;                                     \
            __src--;                                     \
        }                                                \
    } while (0)

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

void *
mpn_export(void *data, size_t *countp, int order,
           size_t size, int endian, size_t nail, mp_srcptr z, mp_size_t zsize)
{
    size_t count, dummy;
    mp_size_t numb;
    unsigned align;
    mp_srcptr zp = z;

    ASSERT (order == 1 || order == -1);
    ASSERT (endian == 1 || endian == 0 || endian == -1);
    ASSERT (nail <= 8*size);
    ASSERT (nail <  8*size || zsize == 0);

    if (countp == NULL) {
        countp = &dummy;
    }
    if (zsize == 0) {
        *countp = 0;
        return data;
    }
    numb = (mp_size_t)(8*size - nail);
    count = (mpn_sizeinbase(zp, zsize, 2) + numb - 1)/numb;
    *countp = count;
    if (endian == 0) {
        endian = HOST_ENDIAN;
    }
    align = ((char *) data - (char *) NULL) % sizeof (mp_limb_t);
    if (nail == GMP_NAIL_BITS) {
        if (size == sizeof (mp_limb_t) && align == 0) {
            if (order == -1 && endian == HOST_ENDIAN) {
                mpn_copyi ((mp_ptr) data, zp, (mp_size_t) count);
                return data;
            }
            if (order == 1 && endian == HOST_ENDIAN) {
                MPN_REVERSE ((mp_ptr) data, zp, (mp_size_t) count);
                return data;
            }
            if (order == -1 && endian == -HOST_ENDIAN) {
                MPN_BSWAP ((mp_ptr) data, zp, (mp_size_t) count);
                return data;
            }
            if (order == 1 && endian == -HOST_ENDIAN) {
                MPN_BSWAP_REVERSE ((mp_ptr) data, zp, (mp_size_t) count);
                return data;
            }
        }
    }
    {
        mp_limb_t limb, wbitsmask;
        size_t i, numb;
        mp_size_t j, wbytes, woffset;
        unsigned char *dp;
        int lbits, wbits;
        mp_srcptr zend;

        numb = size * 8 - nail;
        /* whole bytes per word */
        wbytes = (mp_size_t) numb / 8;
        /* possible partial byte */
        wbits = numb % 8;
        wbitsmask = (CNST_LIMB(1) << wbits) - 1;
        /* offset to get to the next word */
        woffset = (endian >= 0 ? (mp_size_t)size : - (mp_size_t) size)
                   + (order < 0 ? (mp_size_t)size : - (mp_size_t) size);
        /* least significant byte */
        dp = (unsigned char *) data + ((order >= 0 ? (count-1)*size : 0)
                                       + (endian >= 0 ? size-1 : 0));

#define EXTRACT(N, MASK)                                                 \
        do {                                                             \
            if (lbits >= (N)) {                                          \
                *dp = (unsigned char)(limb MASK);                        \
                limb >>= (N);                                            \
                lbits -= (N);                                            \
            }                                                            \
            else {                                                       \
                mp_limb_t newlimb = (zp == zend ? 0 : *zp++);            \
                *dp = (unsigned char)((limb | (newlimb << lbits)) MASK); \
                limb = newlimb >> ((N)-lbits);                           \
                lbits += GMP_NUMB_BITS - (N);                            \
            }                                                            \
        } while (0)

        zend = zp + zsize;
        lbits = 0;
        limb = 0;
        for (i = 0; i < count; i++) {
            for (j = 0; j < wbytes; j++) {
                EXTRACT (8, + 0);
                dp -= endian;
            }
            if (wbits != 0) {
                EXTRACT (wbits, & wbitsmask);
                dp -= endian;
                j++;
            }
            for ( ; j < size; j++) {
                *dp = '\0';
                dp -= endian;
            }
            dp += woffset;
        }
        ASSERT (zp == z + zsize);
        /* low byte of word after most significant */
        ASSERT (dp == (unsigned char *) data
                + (order < 0 ? count*size : - (mp_size_t) size)
                + (endian >= 0 ? (mp_size_t) size - 1 : 0));
    }
    return data;
}

void
mpn_import (mp_ptr zp, mp_size_t *zsize, size_t count, int order,
            size_t size, int endian, size_t nail, const void *data)
{
    ASSERT (order == 1 || order == -1);
    ASSERT (endian == 1 || endian == 0 || endian == -1);
    ASSERT (nail <= 8*size);

    *zsize = (mp_size_t) BITS_TO_LIMBS (count * (8*size - nail));
    mp_ptr z = zp;

    if (endian == 0) {
        endian = HOST_ENDIAN;
    }

    /* Can't use these special cases with nails currently, since they don't
       mask out the nail bits in the input data.  */
    if (nail == 0 && GMP_NAIL_BITS == 0
        && size == sizeof (mp_limb_t)
        && (((char *) data
             - (char *) NULL) % sizeof (mp_limb_t)) == 0 /* align */)
    {
        if (order == -1) {
            if (endian == HOST_ENDIAN) {
                mpn_copyi (zp, (mp_srcptr) data, (mp_size_t) count);
            }
            else /* if (endian == - HOST_ENDIAN) */ {
                MPN_BSWAP (zp, (mp_srcptr) data, (mp_size_t) count);
            }
        } else /* if (order == 1) */ {
            if (endian == HOST_ENDIAN) {
                MPN_REVERSE (zp, (mp_srcptr) data, (mp_size_t) count);
            }
            else /* if (endian == - HOST_ENDIAN) */ {
                MPN_BSWAP_REVERSE (zp, (mp_srcptr) data, (mp_size_t) count);
            }
        }
    }
    else {
        mp_limb_t limb, byte, wbitsmask;
        size_t i, j, numb, wbytes;
        mp_size_t woffset;
        unsigned char *dp;
        int lbits, wbits;

        numb = size * 8 - nail;
        /* whole bytes to process */
        wbytes = (mp_size_t)numb / 8;
        /* partial byte to process */
        wbits = numb % 8;
        wbitsmask = (CNST_LIMB(1) << wbits) - 1;
        /* offset to get to the next word after processing wbytes and wbits */
        woffset = (mp_size_t)(numb + 7) / 8;
        woffset = ((endian >= 0 ? woffset : -woffset)
                   + (order < 0 ? (mp_size_t)size : - (mp_size_t) size));

        /* least significant byte */
        dp = ((unsigned char *) data
              + (order >= 0 ? (count-1)*size : 0) + (endian >= 0 ? size-1 : 0));

#define ACCUMULATE(N)                                     \
        do {                                              \
            ASSERT (lbits < GMP_NUMB_BITS);               \
            ASSERT (limb <= (CNST_LIMB(1) << lbits) - 1); \
                                                          \
            limb |= (mp_limb_t) byte << lbits;            \
            lbits += (N);                                 \
            if (lbits >= GMP_NUMB_BITS) {                 \
                *zp++ = limb & GMP_NUMB_MASK;             \
                lbits -= GMP_NUMB_BITS;                   \
                ASSERT (lbits < (N));                     \
                limb = byte >> ((N) - lbits);             \
            }                                             \
        } while (0)

        limb = 0;
        lbits = 0;
        for (i = 0; i < count; i++) {
            for (j = 0; j < wbytes; j++) {
                byte = *dp;
                dp -= endian;
                ACCUMULATE (8);
            }
            if (wbits != 0) {
                byte = *dp & wbitsmask;
                dp -= endian;
                ACCUMULATE (wbits);
            }
            dp += woffset;
        }
        if (lbits != 0) {
            ASSERT (lbits <= GMP_NUMB_BITS);
            ASSERT_LIMB (limb);
            *zp++ = limb;
        }
        ASSERT (zp == z + *zsize);
        /* low byte of word after most significant */
        ASSERT (dp == (unsigned char *) data
                + (order < 0 ? count*size : - (mp_size_t) size)
                + (endian >= 0 ? (mp_size_t) size - 1 : 0));
    }
    zp = z;
    MPN_NORMALIZE (zp, *zsize);
}

extern void __gmpn_powm(mp_limb_t *rp, const mp_limb_t *bp, mp_size_t bn,
                        const mp_limb_t *ep, mp_size_t en, const mp_limb_t *mp,
                        mp_size_t n, mp_limb_t *tp);
extern mp_size_t __gmpn_binvert_itch(mp_size_t n);
extern void __gmpn_powlo(mp_limb_t *rp, const mp_limb_t *bp,
                         const mp_limb_t *ep, mp_size_t en, mp_size_t n,
                         mp_limb_t *tp);
extern void __gmpn_binvert(mp_limb_t *rp, const mp_limb_t *up, mp_size_t n,
                           mp_limb_t *scratch);
extern void __gmpn_mullo_n(mp_limb_t *rp, const mp_limb_t *xp,
                           const mp_limb_t *yp, mp_size_t n);

void mpn_powm(mp_limb_t *rp, const mp_limb_t *bp, mp_size_t bn,
              const mp_limb_t *ep, mp_size_t en, const mp_limb_t *mp,
              mp_size_t n, mp_limb_t *tp)
{
    __gmpn_powm(rp, bp, bn, ep, en, mp, n, tp);
}

mp_size_t mpn_binvert_itch(mp_size_t n)
{
    return __gmpn_binvert_itch(n);
}

void mpn_powlo(mp_limb_t *rp, const mp_limb_t *bp,
               const mp_limb_t *ep, mp_size_t en, mp_size_t n,
               mp_limb_t *tp)
{
    __gmpn_powlo(rp, bp, ep, en, n, tp);
}

void mpn_binvert(mp_limb_t *rp, const mp_limb_t *up, mp_size_t n,
                 mp_limb_t *scratch)
{
    __gmpn_binvert(rp, up, n, scratch);
}

void mpn_mullo_n(mp_limb_t *rp, const mp_limb_t *xp,
                 const mp_limb_t *yp, mp_size_t n)
{
    __gmpn_mullo_n(rp, xp, yp, n);
}

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
