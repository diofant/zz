/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

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
#else
typedef unsigned long zz_limb_t;
#endif
typedef uint64_t zz_bitcnt_t;
#ifndef _WIN32
typedef int64_t zz_size_t;
#else
typedef int32_t zz_size_t;
#endif

#define ZZ_LIMB_T_BITS 64

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
zz_err zz_from_i64(int64_t u, zz_t *v);
zz_err zz_from_str(const char *str, size_t len, int base, zz_t *u);
zz_err zz_from_bytes(const unsigned char *buf, size_t length, bool negative,
                     zz_t *u);

zz_err zz_to_i64(const zz_t *u, int64_t *v);
zz_err zz_to_double(const zz_t *u, double *d);
zz_err zz_to_str(const zz_t *u, int base, char *str, size_t *len);
zz_err zz_to_bytes(const zz_t *u, size_t length, bool is_signed,
                   unsigned char **buf);

zz_err zz_add(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_add_i64(const zz_t *u, int64_t v, zz_t *w);
zz_err zz_sub(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_sub_i64(const zz_t *u, int64_t v, zz_t *w);
zz_err zz_i64_sub(int64_t u, const zz_t *v, zz_t *w);
zz_err zz_abs(const zz_t *u, zz_t *v);
zz_err zz_neg(const zz_t *u, zz_t *v);
zz_err zz_mul(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_mul_i64(const zz_t *u, int64_t v, zz_t *w);
zz_err zz_div(const zz_t *u, const zz_t *v, zz_t *q, zz_t *r);
zz_err zz_div_i64(const zz_t *u, int64_t v, zz_t *q, zz_t *r);
zz_err zz_i64_div(int64_t u, const zz_t *v, zz_t *q, zz_t *r);

static inline zz_err
zz_i64_add(int64_t u, const zz_t *v, zz_t *w)
{
    return zz_add_i64(v, u, w);
}

static inline zz_err
zz_i64_mul(int64_t u, const zz_t *v, zz_t *w)
{
    return zz_mul_i64(v, u, w);
}

#define zz_add(U, V, W)                                   \
    _Generic((U),                                         \
             int64_t: _Generic((V),                       \
                               default: zz_i64_add),      \
             int: _Generic((V),                           \
                               default: zz_i64_add),      \
             default: _Generic((V),                       \
                               int64_t: zz_add_i64,       \
                               int: zz_add_i64,           \
                               default: zz_add))(U, V, W)
#define zz_sub(U, V, W)                                   \
    _Generic((U),                                         \
             int64_t: _Generic((V),                       \
                               default: zz_i64_sub),      \
             int: _Generic((V),                           \
                               default: zz_i64_sub),      \
             default: _Generic((V),                       \
                               int64_t: zz_sub_i64,       \
                               int: zz_sub_i64,           \
                               default: zz_sub))(U, V, W)
#define zz_mul(U, V, W)                                   \
    _Generic((U),                                         \
             int64_t: _Generic((V),                       \
                               default: zz_i64_mul),      \
             int: _Generic((V),                           \
                               default: zz_i64_mul),      \
             default: _Generic((V),                       \
                               int64_t: zz_mul_i64,       \
                               int: zz_mul_i64,           \
                               default: zz_mul))(U, V, W)
#define zz_div(U, V, Q, R)                                   \
    _Generic((U),                                            \
             int64_t: _Generic((V),                          \
                               default: zz_i64_div),         \
             int: _Generic((V),                              \
                               default: zz_i64_div),         \
             default: _Generic((V),                          \
                               int64_t: zz_div_i64,          \
                               int: zz_div_i64,              \
                               default: zz_div))(U, V, Q, R)


zz_err zz_pow(const zz_t *u, uint64_t v, zz_t *w);
zz_err zz_powm(const zz_t *u, const zz_t *v, const zz_t *w, zz_t *x);

typedef enum {
    ZZ_GT = +1,
    ZZ_EQ = 0,
    ZZ_LT = -1,
} zz_ord;

zz_ord zz_cmp(const zz_t *u, const zz_t *v);
zz_ord zz_cmp_i64(const zz_t *u, int64_t v);

#define zz_cmp(U, V)                                   \
    _Generic((U),                                      \
             default: _Generic((V),                    \
                               int64_t: zz_cmp_i64,    \
                               int: zz_cmp_i64,        \
                               default: zz_cmp))(U, V)

zz_err zz_invert(const zz_t *u, zz_t *v);
zz_err zz_and(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_or(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_xor(const zz_t *u, const zz_t *v, zz_t *w);
zz_err zz_mul_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w);
zz_err zz_quo_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w);

zz_err zz_sqrtrem(const zz_t *u, zz_t *v, zz_t *w);
zz_err zz_gcdext(const zz_t *u, const zz_t *v, zz_t *g, zz_t *s, zz_t *t);
zz_err zz_lcm(const zz_t *u, const zz_t *v, zz_t *w);

zz_err zz_fac(uint64_t u, zz_t *v);
zz_err zz_bin(uint64_t n, uint64_t k, zz_t *v);

typedef struct {
    uint8_t bits_per_limb;
    uint8_t limb_size;
    int8_t limbs_order;
    int8_t limb_endianness;
} zz_layout;

zz_err zz_import(size_t len, const void *data, zz_layout layout, zz_t *u);
zz_err zz_export(const zz_t *u, zz_layout layout, size_t len, void *data);

zz_err zz_sizeinbase(const zz_t *u, int base, size_t *size);
zz_bitcnt_t zz_bitlen(const zz_t *u);
zz_bitcnt_t zz_lsbpos(const zz_t *u);
zz_bitcnt_t zz_bitcnt(const zz_t *u);
bool zz_iszero(const zz_t *u);
bool zz_isneg(const zz_t *u);
bool zz_isodd(const zz_t *u);

#endif /* ZZ_H */
