/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#ifndef ZZ_H
#define ZZ_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __APPLE__
typedef uint64_t zz_limb_t;
typedef int64_t zz_slimb_t;
#else
typedef unsigned long zz_limb_t;
typedef long zz_slimb_t;
#endif

#define ZZ_LIMB_T_MAX UINT64_MAX
#define ZZ_LIMB_T_BITS 64
#define ZZ_LIMB_T_BYTES 8
#define ZZ_LIMB_T_MASK UINT64_MAX

#define ZZ_SLIMB_T_MAX INT64_MAX
#define ZZ_SLIMB_T_MIN INT64_MIN
#define ZZ_SLIMB_T_BITS 64

typedef uint64_t zz_bitcnt_t;

#ifndef _WIN32
typedef int64_t zz_size_t;
#  define ZZ_SIZE_T_MAX INT64_MAX
#  define ZZ_SIZE_T_MIN INT64_MIN
#  define ZZ_SIZE_T_BITS 64
#define ZZ_MAX_BITS UINT64_MAX
#else
typedef int32_t zz_size_t;
#  define ZZ_SIZE_T_MAX INT32_MAX
#  define ZZ_SIZE_T_MIN INT32_MIN
#  define ZZ_SIZE_T_BITS 32
#define ZZ_MAX_BITS ZZ_SIZE_T_MAX*(uint64_t)ZZ_LIMB_T_BITS
#endif

typedef struct {
    bool negative;
    zz_size_t alloc;
    zz_size_t size;
    zz_limb_t *digits;
} zz_t;

typedef enum {
    ZZ_OK = 0,
    ZZ_MEM = -1,
    ZZ_VAL = -2,
    ZZ_BUF = -3,
} zz_err;

typedef struct {
    uint8_t version[3];
    uint8_t bits_per_limb;
    uint8_t limb_bytes;
    uint8_t limbcnt_bytes;
    uint8_t bitcnt_bytes;
} zz_info;

zz_err zz_setup(zz_info *info);
void zz_finish(void);

zz_err zz_init(zz_t *u);
void zz_clear(zz_t *u);

zz_err zz_copy(const zz_t *u, zz_t *v);
zz_err zz_from_sl(zz_slimb_t u, zz_t *v);
zz_err zz_from_str(const int8_t *str, size_t len, int8_t base, zz_t *u);
zz_err zz_from_bytes(const uint8_t *buf, size_t length, bool negative, zz_t *u);

zz_err zz_to_sl(const zz_t *u, zz_slimb_t *v);
zz_err zz_to_double(const zz_t *u, double *d);
zz_err zz_to_str(const zz_t *u, int8_t base, int8_t *str, size_t *len);
zz_err zz_to_bytes(const zz_t *u, size_t length, bool is_signed, uint8_t **buf);

zz_err zz_add(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_add_sl(const zz_t *u, zz_slimb_t v, zz_t *w);
zz_err zz_sub(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_sub_sl(const zz_t *u, zz_slimb_t v, zz_t *w);
zz_err zz_sl_sub(zz_slimb_t u, const zz_t *v, zz_t *w);
zz_err zz_abs(const zz_t *u, zz_t *v);
zz_err zz_neg(const zz_t *u, zz_t *v);
zz_err zz_mul(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_mul_sl(const zz_t *u, zz_slimb_t v, zz_t *w);
zz_err zz_div(const zz_t *u, const zz_t *v, zz_t *q, zz_t *r);
zz_err zz_div_sl (const zz_t *u, zz_slimb_t v, zz_t *q, zz_t *r);
zz_err zz_sl_div (zz_slimb_t u, const zz_t *v, zz_t *q, zz_t *r);

zz_err zz_pow(const zz_t *u, zz_limb_t v, zz_t *w);
zz_err zz_powm(const zz_t *u, const zz_t *v, const zz_t *w, zz_t *x);

typedef enum {
    ZZ_GT = +1,
    ZZ_EQ = 0,
    ZZ_LT = -1,
} zz_ord;

zz_ord zz_cmp(const zz_t *u, const zz_t *v);
zz_ord zz_cmp_sl(const zz_t *u, zz_slimb_t v);

zz_err zz_invert(const zz_t *u, zz_t *v);
zz_err zz_and(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_or(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_xor(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_mul_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w);
zz_err zz_quo_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w);

zz_err zz_sqrtrem(const zz_t *u, zz_t *v, zz_t *w);
zz_err zz_gcdext(const zz_t *u, const zz_t *v, zz_t *g, zz_t *s, zz_t *t);
zz_err zz_lcm(const zz_t *u, const zz_t *v, zz_t *w);

zz_err zz_fac(zz_limb_t u, zz_t *v);
zz_err zz_bin(zz_limb_t n, zz_limb_t k, zz_t *v);

typedef struct {
    uint8_t bits_per_limb;
    uint8_t limb_size;
    int8_t limbs_order;
    int8_t limb_endianness;
} zz_layout;

zz_err zz_import(size_t len, const void *data, zz_layout layout, zz_t *u);
zz_err zz_export(const zz_t *u, zz_layout layout, size_t len, void *data);

zz_err zz_sizeinbase(const zz_t *u, int8_t base, size_t *size);
zz_bitcnt_t zz_bitlen(const zz_t *u);
zz_bitcnt_t zz_lsbpos(const zz_t *u);
zz_bitcnt_t zz_bitcnt(const zz_t *u);
bool zz_iszero(const zz_t *u);
bool zz_isneg(const zz_t *u);
bool zz_isodd(const zz_t *u);

#endif /* ZZ_H */
