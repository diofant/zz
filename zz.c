/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

#if defined(__MINGW32__) && defined(__GNUC__)
#  define isinf __builtin_isinf
#endif

#include "zz-impl.h"

#if GMP_NAIL_BITS != 0
#  error "GMP_NAIL_BITS expected to be 0"
#endif
#if GMP_LIMB_BITS != 64
#  error "GMP_LIMB_BITS expected to be 64"
#endif

#if ZZ_LIMB_T_BITS < DBL_MANT_DIG
#  error ZZ_LIMB_T_BITS expected to be more than ZZ_LIMB_T_BITS
#endif

#if defined(_MSC_VER)
#  define _Thread_local __declspec(thread)
#endif

_Thread_local jmp_buf zz_env;

#define TRACKER_SIZE_INCR 64
_Thread_local struct {
    size_t size;
    size_t alloc;
    void **ptrs;
} zz_tracker;

static void *
zz_reallocate_function(void *ptr, size_t old_size, size_t new_size)
{
    if (zz_tracker.size >= zz_tracker.alloc) {
        /* Reallocation shouldn't be required.  Unless...
           you are using the mpz_t from the GNU GMP with
           our memory functions. */
        /* LCOV_EXCL_START */
        void **tmp = zz_tracker.ptrs;

        zz_tracker.alloc += TRACKER_SIZE_INCR;
        zz_tracker.ptrs = realloc(tmp, zz_tracker.alloc * sizeof(void *));
        if (!zz_tracker.ptrs) {
            zz_tracker.alloc -= TRACKER_SIZE_INCR;
            /* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110501 */
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 12
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wuse-after-free"
#endif
            zz_tracker.ptrs = tmp;
#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic pop
#endif
            goto err;
        }
        /* LCOV_EXCL_STOP */
    }
    if (!ptr) {
        void *ret = malloc(new_size);

        if (!ret) {
            goto err; /* LCOV_EXCL_LINE */
        }
        zz_tracker.ptrs[zz_tracker.size] = ret;
        zz_tracker.size++;
        return ret;
    }
    size_t i = zz_tracker.size - 1;

    for (;; i--) {
        if (zz_tracker.ptrs[i] == ptr) {
            break;
        }
    }

    void *ret = realloc(ptr, new_size);

    if (!ret) {
        goto err; /* LCOV_EXCL_LINE */
    }
    zz_tracker.ptrs[i] = ret;
    return ret;
err:
    /* LCOV_EXCL_START */
    for (size_t i = 0; i < zz_tracker.size; i++) {
        free(zz_tracker.ptrs[i]);
        zz_tracker.ptrs[i] = NULL;
    }
    zz_tracker.size = 0;
    zz_tracker.alloc = 0;
    longjmp(zz_env, 1);
    /* LCOV_EXCL_STOP */
}

static void *
zz_allocate_function(size_t size)
{
    return zz_reallocate_function(NULL, 0, size);
}

static void
zz_free_function(void *ptr, size_t size)
{
    for (size_t i = zz_tracker.size - 1; i >= 0; i--) {
        if (zz_tracker.ptrs[i] == ptr) {
            zz_tracker.ptrs[i] = NULL;
            break;
        }
    }
    free(ptr);

    size_t i = zz_tracker.size - 1;

    while (zz_tracker.size > 0) {
        if (zz_tracker.ptrs[i]) {
            break;
        }
        zz_tracker.size--;
        i--;
    }
}

static struct {
    void *(*default_allocate_func)(size_t);
    void *(*default_reallocate_func)(void *, size_t, size_t);
    void (*default_free_func)(void *, size_t);
} zz_state;

zz_err
zz_setup(zz_info *info)
{
    mp_get_memory_functions(&zz_state.default_allocate_func,
                            &zz_state.default_reallocate_func,
                            &zz_state.default_free_func);
    mp_set_memory_functions(zz_allocate_function,
                            zz_reallocate_function,
                            zz_free_function);
    if (info) {
        info->version[0] = __GNU_MP_VERSION;
        info->version[1] = __GNU_MP_VERSION_MINOR;
        info->version[2] = __GNU_MP_VERSION_PATCHLEVEL;
        info->bits_per_limb = ZZ_LIMB_T_BITS;
        info->limb_bytes = sizeof(mp_limb_t);
        info->limbcnt_bytes = sizeof(mp_size_t);
        info->bitcnt_bytes = sizeof(mp_bitcnt_t);
    }
    return ZZ_OK;
}

void
zz_finish(void)
{
    mp_set_memory_functions(zz_state.default_allocate_func,
                            zz_state.default_reallocate_func,
                            zz_state.default_free_func);
}

zz_err
zz_init(zz_t *u)
{
    u->negative = false;
    u->alloc = 0;
    u->size = 0;
    u->digits = NULL;
    return ZZ_OK;
}

zz_err
zz_resize(uint64_t size, zz_t *u)
{
    if (u->alloc >= size) {
        u->size = (zz_size_t)size;
        if (!u->size) {
            u->negative = false;
        }
        return ZZ_OK;
    }
    if (size > ZZ_SIZE_T_MAX/ZZ_LIMB_T_BYTES) {
        return ZZ_MEM;
    }

    zz_size_t alloc = (zz_size_t)size;
    zz_limb_t *t = u->digits;

    u->digits = realloc(u->digits, (size_t)alloc * ZZ_LIMB_T_BYTES);
    if (u->digits) {
        u->alloc = alloc;
        u->size = alloc;
        return ZZ_OK;
    }
    /* LCOV_EXCL_START */
    u->digits = t;
    return ZZ_MEM;
    /* LCOV_EXCL_STOP */
}

void
zz_clear(zz_t *u)
{
    free(u->digits);
    u->negative = false;
    u->alloc = 0;
    u->size = 0;
    u->digits = NULL;
}

static void
zz_normalize(zz_t *u)
{
    while (u->size && u->digits[u->size - 1] == 0) {
        u->size--;
    }
    if (!u->size) {
        u->negative = false;
    }
}

zz_ord
zz_cmp(const zz_t *u, const zz_t *v)
{
    if (u == v) {
        return ZZ_EQ;
    }

    zz_ord sign = u->negative ? ZZ_LT : ZZ_GT;

    if (u->negative != v->negative) {
        return sign;
    }
    else if (u->size != v->size) {
        return (u->size < v->size) ? -sign : sign;
    }

    zz_ord r = mpn_cmp(u->digits, v->digits, u->size);

    return u->negative ? -r : r;
}

zz_ord
zz_cmp_sl(const zz_t *u, zz_slimb_t v)
{
    zz_ord sign = u->negative ? ZZ_LT : ZZ_GT;
    bool v_negative = v < 0;

    if (u->negative != v_negative) {
        return sign;
    }
    else if (u->size != 1) {
        return u->size ? sign : (v ? -sign : ZZ_EQ);
    }

    zz_limb_t digit = (zz_limb_t)imaxabs(v);
    zz_ord r = u->digits[0] != digit;

    if (u->digits[0] < digit) {
        r = ZZ_LT;
    }
    else if (u->digits[0] > digit) {
        r = ZZ_GT;
    }
    return u->negative ? -r : r;
}


zz_err
zz_from_sl(zz_slimb_t u, zz_t *v)
{
    if (!u) {
        v->size = 0;
        v->negative = false;
        return ZZ_OK;
    }

    bool negative = u < 0;
    zz_limb_t uv = (negative ? -((zz_limb_t)(u + 1) - 1) : (zz_limb_t)(u));

    if (zz_resize(1, v)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    v->negative = negative;
    v->digits[0] = uv;
    return ZZ_OK;
}

zz_err
zz_to_sl(const zz_t *u, zz_slimb_t *v)
{
    zz_size_t n = u->size;

    if (!n) {
        *v = 0;
        return ZZ_OK;
    }
    if (n > 2) {
        return ZZ_VAL;
    }

    zz_limb_t uv = u->digits[0];

    if (n > 1) {
        return ZZ_VAL;
    }
    if (u->negative) {
        if (uv <= ZZ_SLIMB_T_MAX + (zz_limb_t)1) {
            *v = -1 - (zz_slimb_t)((uv - 1) & ZZ_SLIMB_T_MAX);
            return ZZ_OK;
        }
    }
    else {
        if (uv <= ZZ_SLIMB_T_MAX) {
            *v = (zz_slimb_t)uv;
            return ZZ_OK;
        }
    }
    return ZZ_VAL;
}

bool
zz_iszero(const zz_t *u)
{
    return u->size == 0;
}

bool
zz_isneg(const zz_t *u)
{
    return u->negative;
}

bool
zz_isodd(const zz_t *u)
{
    return u->size && u->digits[0] & 1;
}

zz_err
zz_copy(const zz_t *u, zz_t *v)
{
    if (u != v) {
        if (!u->size) {
            return zz_from_sl(0, v);
        }
        if (zz_resize((uint64_t)u->size, v)) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        v->negative = u->negative;
        mpn_copyi(v->digits, u->digits, u->size);
    }
    return ZZ_OK;
}

zz_err
zz_abs(const zz_t *u, zz_t *v)
{
    if (u != v && zz_copy(u, v)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    v->negative = false;
    return ZZ_OK;
}

zz_err
zz_neg(const zz_t *u, zz_t *v)
{
    if (u != v && zz_copy(u, v)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    if (v->size) {
        v->negative = !u->negative;
    }
    return ZZ_OK;
}

zz_err
zz_sizeinbase(const zz_t *u, int8_t base, size_t *len)
{
    const int abase = abs(base);

    if (abase < 2 || abase > 36) {
        return ZZ_VAL;
    }
    *len = mpn_sizeinbase(u->digits, u->size, abase);
    return ZZ_OK;
}

zz_err
zz_to_str(const zz_t *u, int8_t base, int8_t *str, size_t *len)
{
    /* Maps 1-byte integer to digit character for bases up to 36. */
    const char *NUM_TO_TEXT = "0123456789abcdefghijklmnopqrstuvwxyz";

    if (base < 0) {
        base = -base;
        NUM_TO_TEXT = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }
    if (base < 2 || base > 36) {
        return ZZ_VAL;
    }

    uint8_t *p = (uint8_t *)str;

    if (u->negative) {
        *(p++) = '-';
    }
    /* We use undocumented feature of mpn_get_str(): u->size >= 0 */
    if ((base & (base - 1)) == 0) {
        *len = mpn_get_str(p, base, u->digits, u->size);
    }
    else { /* generic base, not power of 2, input might be clobbered */
        mp_limb_t *volatile tmp = malloc(ZZ_LIMB_T_BYTES * (size_t)u->alloc);

        if (!tmp || TMP_OVERFLOW) {
            /* LCOV_EXCL_START */
            free(tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }
        mpn_copyi(tmp, u->digits, u->size);
        *len = mpn_get_str(p, base, tmp, u->size);
        free(tmp);
    }
    for (size_t i = 0; i < *len; i++) {
        *p = (uint8_t)NUM_TO_TEXT[*p];
        p++;
    }
    if (u->negative) {
        (*len)++;
    }
    return ZZ_OK;
}

/* Table of digit values for 8-bit string->mpz conversion.
   Note that when converting a base B string, a char c is a legitimate
   base B digit iff DIGIT_VALUE_TAB[c] < B. */
const int8_t DIGIT_VALUE_TAB[] =
{
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
  -1,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,
  51,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

zz_err
zz_from_str(const int8_t *str, size_t len, int8_t base, zz_t *u)
{
    if (base < 2 || base > 36) {
        return ZZ_VAL;
    }

    uint8_t *volatile buf = malloc(len), *p = buf;

    if (!buf) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    memcpy(buf, str, len);

    bool negative = (p[0] == '-');

    p += negative;
    len -= negative;
    if (!len) {
        goto err;
    }
    if (p[0] == '_') {
        goto err;
    }

    const int8_t *digit_value = DIGIT_VALUE_TAB;
    size_t new_len = len;

    for (size_t i = 0; i < len; i++) {
        if (p[i] == '_') {
            if (i == len - 1 || p[i + 1] == '_') {
                goto err;
            }
            new_len--;
            memmove(p + i, p + i + 1, len - i - 1);
        }
        p[i] = (uint8_t)digit_value[p[i]];
        if (p[i] >= base) {
            goto err;
        }
    }
    len = new_len;
    if (zz_resize(1 + (uint64_t)len/2, u) || TMP_OVERFLOW) {
        /* LCOV_EXCL_START */
        free(buf);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    u->negative = negative;
    u->size = (zz_size_t)mpn_set_str(u->digits, p, len, base);
    free(buf);
    if (zz_resize((uint64_t)u->size, u) == ZZ_MEM) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    zz_normalize(u);
    return ZZ_OK;
err:
    free(buf);
    return ZZ_VAL;
}

static bool
zz_tstbit(const zz_t *u, zz_bitcnt_t idx)
{
    zz_size_t limb_idx = (zz_size_t)(idx / ZZ_LIMB_T_BITS);

    if (limb_idx >= u->size) {
        return false;
    }
    return (u->digits[limb_idx] >> (idx%ZZ_LIMB_T_BITS)) & 1;
}

zz_err
zz_to_double(const zz_t *u, double *d)
{
    if (u->size > DBL_MAX_EXP/ZZ_LIMB_T_BITS + 1) {
        *d = u->negative ? -INFINITY : INFINITY;
        return ZZ_BUF;
    }

    zz_bitcnt_t bits = zz_bitlen(u);
    TMP_MPZ(z, u);
    *d = mpz_get_d(z); /* round towards zero */
    if (DBL_MANT_DIG < bits && bits <= DBL_MAX_EXP) {
        bits -= DBL_MANT_DIG + 1;
        if (zz_tstbit(u, bits)) {
            zz_bitcnt_t tz = zz_lsbpos(u);

            if (tz < bits || (tz == bits && zz_tstbit(u, bits + 1))) {
                *d = nextafter(*d, 2 * (*d)); /* round away from zero */
            }
        }
    }
    if (isinf(*d)) {
        return ZZ_BUF;
    }
    return ZZ_OK;
}

zz_err
zz_to_bytes(const zz_t *u, size_t length, bool is_signed, uint8_t **buffer)
{
    zz_t tmp;
    bool is_negative = u->negative;

    if (zz_init(&tmp)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    if (is_negative) {
        if (!is_signed) {
            return ZZ_BUF;
        }
        if (zz_resize(8*(uint64_t)length/ZZ_LIMB_T_BITS + 1, &tmp)) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        if (tmp.size < u->size) {
            goto overflow;
        }
        mpn_zero(tmp.digits, tmp.size);
        tmp.digits[tmp.size - 1] = 1;
        tmp.digits[tmp.size - 1] <<= ((8*length)
                                      % (ZZ_LIMB_T_BITS*(size_t)tmp.size));
        mpn_sub(tmp.digits, tmp.digits, tmp.size, u->digits, u->size);
        zz_normalize(&tmp);
        u = &tmp;
    }

    size_t nbits = zz_bitlen(u);

    if (nbits > 8*length
        || (is_signed && ((!nbits && is_negative)
            || (nbits && (nbits == 8 * length ? !is_negative : is_negative)))))
    {
overflow:
        zz_clear(&tmp);
        return ZZ_BUF;
    }

    size_t gap = length - (nbits + ZZ_LIMB_T_BITS/8 - 1)/(ZZ_LIMB_T_BITS/8);

    /* We use undocumented feature of mpn_get_str(): u->size >= 0 */
    mpn_get_str(*buffer + gap, 256, u->digits, u->size);
    memset(*buffer, is_negative ? 0xFF : 0, gap);
    zz_clear(&tmp);
    return ZZ_OK;
}

zz_err
zz_from_bytes(const uint8_t *buffer, size_t length, bool is_signed, zz_t *u)
{
    if (!length) {
        return zz_from_sl(0, u);
    }
    if (zz_resize(1 + (uint64_t)length/2, u)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    u->size = (zz_size_t)mpn_set_str(u->digits, buffer, length, 256);
    if (zz_resize((uint64_t)u->size, u) == ZZ_MEM) {
        /* LCOV_EXCL_START */
        zz_clear(u);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    zz_normalize(u);
    if (is_signed && zz_bitlen(u) == 8*(size_t)length) {
        if (u->size > 1) {
            mpn_sub_1(u->digits, u->digits, u->size, 1);
            mpn_com(u->digits, u->digits, u->size - 1);
        }
        else {
            u->digits[u->size - 1] -= 1;
        }
        u->digits[u->size - 1] = ~u->digits[u->size - 1];
        assert(ZZ_LIMB_T_BITS*u->size >= 8*length);
        assert(ZZ_LIMB_T_BITS*u->size < 8*length + ZZ_LIMB_T_BITS);

        mp_size_t shift = (mp_size_t)(ZZ_LIMB_T_BITS*(size_t)u->size
                                      - 8*length);

        u->digits[u->size - 1] <<= shift;
        u->digits[u->size - 1] >>= shift;
        u->negative = true;
        zz_normalize(u);
    }
    return ZZ_OK;
}

zz_bitcnt_t
zz_bitlen(const zz_t *u)
{
    return u->size ? (zz_bitcnt_t)mpn_sizeinbase(u->digits, u->size, 2) : 0;
}

zz_bitcnt_t
zz_lsbpos(const zz_t *u)
{
    return u->size ? mpn_scan1(u->digits, 0) : 0;
}

zz_bitcnt_t
zz_bitcnt(const zz_t *u)
{
    return u->size ? mpn_popcount(u->digits, u->size) : 0;
}

zz_err
zz_import(size_t len, const void *digits, zz_layout layout, zz_t *u)
{
    size_t size = (len*layout.bits_per_limb
                   + (ZZ_LIMB_T_BITS - 1))/ZZ_LIMB_T_BITS;

    if (len > SIZE_MAX / layout.bits_per_limb
        || size > INT_MAX
        || zz_resize((uint64_t)size, u))
    {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }

    TMP_MPZ(z, u);
    assert(layout.limb_size*8 >= layout.bits_per_limb);
    mpz_import(z, len, layout.limbs_order, layout.limb_size,
               layout.limb_endianness,
               (size_t)(layout.limb_size*8 - layout.bits_per_limb),
               digits);
    u->size = z->_mp_size;
    return ZZ_OK;
}

zz_err
zz_export(const zz_t *u, zz_layout layout, size_t len, void *digits)
{
    if (len < (zz_bitlen(u) + layout.bits_per_limb
               - 1)/layout.bits_per_limb)
    {
        return ZZ_VAL;
    }
    if (u->size > INT_MAX) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }

    TMP_MPZ(z, u);
    assert(layout.limb_size*8 >= layout.bits_per_limb);
    mpz_export(digits, NULL, layout.limbs_order, layout.limb_size,
               layout.limb_endianness,
               (size_t)(layout.limb_size*8 - layout.bits_per_limb),
               z);
    return ZZ_OK;
}

static zz_err
zz_addsub(const zz_t *u, const zz_t *v, bool subtract, zz_t *w)
{
    bool negu = u->negative, negv = subtract ? !v->negative : v->negative;
    bool same_sign = negu == negv;
    zz_size_t u_size = u->size, v_size = v->size;

    if (u_size < v_size) {
        SWAP(const zz_t *, u, v);
        SWAP(bool, negu, negv);
        SWAP(zz_size_t, u_size, v_size);
    }

    if (zz_resize((uint64_t)u_size + same_sign, w)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    w->negative = negu;
    /* We use undocumented feature of mpn_add/sub(): v_size can be 0 */
    if (same_sign) {
        w->digits[w->size - 1] = mpn_add(w->digits, u->digits, u_size,
                                         v->digits, v_size);
    }
    else if (u_size != v_size) {
        mpn_sub(w->digits, u->digits, u_size, v->digits, v_size);
    }
    else {
        int cmp = mpn_cmp(u->digits, v->digits, u_size);

        if (cmp < 0) {
            mpn_sub_n(w->digits, v->digits, u->digits, u_size);
            w->negative = negv;
        }
        else if (cmp > 0) {
            mpn_sub_n(w->digits, u->digits, v->digits, u_size);
        }
        else {
            w->size = 0;
        }
    }
    zz_normalize(w);
    return ZZ_OK;
}

static zz_err
zz_addsub_sl(const zz_t *u, zz_slimb_t v, bool subtract, zz_t *w)
{
    bool negu = u->negative, negv = subtract ? v >= 0 : v < 0;
    bool same_sign = negu == negv;
    zz_size_t u_size = u->size, v_size = v != 0;
    zz_limb_t digit = (zz_limb_t)imaxabs(v);

    if (!u_size || u_size < v_size) {
        assert(!u_size);
        if (zz_resize((uint64_t)v_size, w)) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        if (v_size) {
            w->digits[0] = digit;
        }
        if (w->size) {
            w->negative = negv;
        }
        return ZZ_OK;
    }

    if (zz_resize((uint64_t)u_size + same_sign, w)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    w->negative = negu;
    if (same_sign) {
        w->digits[w->size - 1] = mpn_add_1(w->digits, u->digits, u_size, digit);
    }
    else if (u_size != 1) {
        mpn_sub_1(w->digits, u->digits, u_size, digit);
    }
    else {
        if (u->digits[0] < digit) {
            w->digits[0] = digit - u->digits[0];
            w->negative = negv;
        }
        else {
            w->digits[0] = u->digits[0] - digit;
        }
    }
    zz_normalize(w);
    return ZZ_OK;
}

zz_err
zz_add(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_addsub(u, v, false, w);
}

zz_err
zz_sub(const zz_t *u, const zz_t *v, zz_t *w)
{
    return zz_addsub(u, v, true, w);
}

zz_err
zz_add_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    return zz_addsub_sl(u, v, false, w);
}

zz_err
zz_sub_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    return zz_addsub_sl(u, v, true, w);
}

zz_err
zz_sl_sub(zz_slimb_t u, const zz_t *v, zz_t *w)
{
    if (zz_neg(v, w)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    return zz_addsub_sl(w, u, false, w);
}

zz_err
zz_mul(const zz_t *u, const zz_t *v, zz_t *w)
{
    if (u->size < v->size) {
        SWAP(const zz_t *, u, v);
    }
    if (!v->size) {
        return zz_from_sl(0, w);
    }
    if (u == w) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(u, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = u == v ? zz_mul(&tmp, &tmp, w) : zz_mul(&tmp, v, w);

        zz_clear(&tmp);
        return ret;
    }
    if (v == w) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(v, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = zz_mul(u, &tmp, w);

        zz_clear(&tmp);
        return ret;
    }
    if (zz_resize((uint64_t)u->size + (uint64_t)v->size, w) || TMP_OVERFLOW) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    w->negative = u->negative != v->negative;
    if (v->size == 1) {
        w->digits[w->size - 1] = mpn_mul_1(w->digits, u->digits, u->size,
                                           v->digits[0]);
    }
    else if (u->size == v->size) {
        if (u != v) {
            mpn_mul_n(w->digits, u->digits, v->digits, u->size);
        }
        else {
            mpn_sqr(w->digits, u->digits, u->size);
        }
    }
    else {
        mpn_mul(w->digits, u->digits, u->size, v->digits, v->size);
    }
    w->size -= w->digits[w->size - 1] == 0;
    assert(w->size >= 1);
    return ZZ_OK;
}

zz_err
zz_mul_sl(const zz_t *u, zz_slimb_t v, zz_t *w)
{
    if (!u->size || !v) {
        return zz_from_sl(0, w);
    }
    if (zz_resize((uint64_t)u->size + 1, w) || TMP_OVERFLOW) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    w->negative = u->negative != (v < 0);
    w->digits[w->size - 1] = mpn_mul_1(w->digits, u->digits, u->size,
                                       (zz_limb_t)imaxabs(v));
    w->size -= w->digits[w->size - 1] == 0;
    assert(w->size >= 1);
    return ZZ_OK;
}

zz_err
zz_div(const zz_t *u, const zz_t *v, zz_t *q, zz_t *r)
{
    if (!v->size) {
        return ZZ_VAL;
    }
    if (!q || !r) {
        if (!q && !r) {
            return ZZ_VAL;
        }
        if (!q) {
            zz_t tmp;

            if (zz_init(&tmp)) {
                return ZZ_MEM; /* LCOV_EXCL_LINE */
            }

            zz_err ret = zz_div(u, v, &tmp, r);

            zz_clear(&tmp);
            return ret;
        }
        else {
            zz_t tmp;

            if (zz_init(&tmp)) {
                return ZZ_MEM; /* LCOV_EXCL_LINE */
            }

            zz_err ret = zz_div(u, v, q, &tmp);

            zz_clear(&tmp);
            return ret;
        }
    }
    if (!u->size) {
        if (zz_from_sl(0, q) || zz_from_sl(0, r)) {
            goto err; /* LCOV_EXCL_LINE */
        }
    }
    else if (u->size < v->size) {
        if (u->negative != v->negative) {
            if (zz_from_sl(-1, q) || zz_add(u, v, r)) {
                goto err; /* LCOV_EXCL_LINE */
            }
        }
        else {
            if (zz_from_sl(0, q) || zz_copy(u, r)) {
                goto err; /* LCOV_EXCL_LINE */
            }
        }
    }
    else {
        if (u == q) {
            zz_t tmp;

            if (zz_init(&tmp) || zz_copy(u, &tmp)) {
                /* LCOV_EXCL_START */
                zz_clear(&tmp);
                return ZZ_MEM;
                /* LCOV_EXCL_STOP */
            }

            zz_err ret = zz_div(&tmp, v, q, r);

            zz_clear(&tmp);
            return ret;
        }
        if (v == q || v == r) {
            zz_t tmp;

            if (zz_init(&tmp) || zz_copy(v, &tmp)) {
                /* LCOV_EXCL_START */
                zz_clear(&tmp);
                return ZZ_MEM;
                /* LCOV_EXCL_STOP */
            }

            zz_err ret = zz_div(u, &tmp, q, r);

            zz_clear(&tmp);
            return ret;
        }

        bool q_negative = (u->negative != v->negative);
        zz_size_t u_size = u->size;

        if (zz_resize((uint64_t)(u_size - v->size) + 1 + q_negative, q)
            || zz_resize((uint64_t)v->size, r) || TMP_OVERFLOW)
        {
            goto err; /* LCOV_EXCL_LINE */
        }
        q->negative = q_negative;
        if (q_negative) {
            q->digits[q->size - 1] = 0;
        }
        r->negative = v->negative;
        mpn_tdiv_qr(q->digits, r->digits, 0, u->digits, u_size, v->digits,
                    v->size);
        zz_normalize(r);
        if (q_negative && r->size) {
            r->size = v->size;
            mpn_sub_n(r->digits, v->digits, r->digits, v->size);
            mpn_add_1(q->digits, q->digits, q->size, 1);
        }
        zz_normalize(q);
        zz_normalize(r);
    }
    return ZZ_OK;
    /* LCOV_EXCL_START */
err:
    zz_clear(q);
    zz_clear(r);
    return ZZ_MEM;
    /* LCOV_EXCL_STOP */
}

zz_err
zz_div_sl (const zz_t *u, zz_slimb_t v, zz_t *q, zz_t *r)
{
    if (!v) {
        return ZZ_VAL;
    }

    zz_limb_t rl, uv = v < 0 ? -((zz_limb_t)(v + 1) - 1) : (zz_limb_t)v;
    bool same_signs = u->negative == (v < 0);

    if (q) {
        if (u->size) {
            if (zz_resize((uint64_t)u->size, q)) {
                return ZZ_MEM; /* LCOV_EXCL_LINE */
            }
            rl = mpn_divrem_1(q->digits, 0, u->digits, u->size, uv);
            if (rl && !same_signs) {
                mpn_add_1(q->digits, q->digits, q->size, 1);
            }
            q->size -= q->digits[q->size - 1] == 0;
            if (q->size) {
                q->negative = !same_signs;
            }
        }
        else {
            q->size = 0;
        }
    }
    if (r) {
        if (!u->size) {
            r->size = 0;
            r->negative = false;
            return ZZ_OK;
        }
        if (zz_resize(1, r)) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        rl = mpn_mod_1(u->digits, u->size, uv);
        if (!rl) {
            r->size = 0;
            r->negative = false;
        }
        else {
            if (!same_signs) {
                rl = uv - rl;
            }
            r->digits[0] = rl;
            r->negative = v < 0;
        }
        return ZZ_OK;
    }
    return q ? ZZ_OK : ZZ_VAL;
}

static zz_slimb_t
fdiv_r(zz_slimb_t a, zz_slimb_t b)
{
    return a/b - (a%b != 0 && (a^b) < 0);
}

zz_err
zz_sl_div (zz_slimb_t u, const zz_t *v, zz_t *q, zz_t *r)
{
    if (!v->size) {
        return ZZ_VAL;
    }

    zz_slimb_t sv;

    if (q) {
        zz_err ret = ZZ_OK;

        if (zz_to_sl(v, &sv)) {
            ret = zz_from_sl((u < 0) == v->negative || !u ? 0 : -1, q);
        }
        else {
            ret = zz_from_sl(fdiv_r(u, sv), q);
        }
        if (ret || !r) {
            return ret; /* LCOV_EXCL_LINE */
        }
    }
    if (r) {
        if (zz_to_sl(v, &sv)) {
            if ((u < 0) == v->negative || !u) {
                return zz_from_sl(u, r);
            }
            return zz_add_sl(v, u, r);
        }
        return zz_from_sl(u - fdiv_r(u, sv)*sv, r);
    }
    return q ? ZZ_OK : ZZ_VAL;
}

zz_err
zz_quo_2exp(const zz_t *u, zz_bitcnt_t shift, zz_t *v)
{
    if (!u->size) {
        v->size = 0;
        return ZZ_OK;
    }
    if (shift > ZZ_MAX_BITS) {
        if (u->negative) {
            return zz_from_sl(-1, v);
        }
        v->size = 0;
        return ZZ_OK;
    }

    zz_size_t whole = (zz_size_t)(shift / ZZ_LIMB_T_BITS);
    zz_size_t size = u->size;

    shift %= ZZ_LIMB_T_BITS;
    if (whole >= size) {
        return zz_from_sl(u->negative ? -1 : 0, v);
    }
    size -= (zz_size_t)whole;

    bool carry = false, extra = true;

    for (mp_size_t i = 0; i < whole; i++) {
        if (u->digits[i]) {
            carry = u->negative;
            break;
        }
    }
    for (mp_size_t i = whole; i < u->size; i++) {
        if (u->digits[i] != ZZ_LIMB_T_MAX) {
            extra = 0;
            break;
        }
    }
    if (zz_resize((uint64_t)size + extra, v)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    v->negative = u->negative;
    if (shift) {
        if (mpn_rshift(v->digits, u->digits + whole, size,
                       (unsigned int)shift))
        {
            carry = u->negative;
        }
    }
    else {
        mpn_copyi(v->digits, u->digits + whole, size);
    }
    if (extra) {
        v->digits[size] = 0;
    }
    if (carry) {
        if (mpn_add_1(v->digits, v->digits, size, 1)) {
            v->digits[size] = 1;
        }
    }
    zz_normalize(v);
    return ZZ_OK;
}

zz_err
zz_mul_2exp(const zz_t *u, zz_bitcnt_t shift, zz_t *v)
{
    if (!u->size) {
        v->size = 0;
        return ZZ_OK;
    }
    if (shift > ZZ_MAX_BITS - zz_bitcnt(u)) {
        return ZZ_MEM;
    }

    zz_size_t whole = (zz_size_t)(shift / ZZ_LIMB_T_BITS);
    zz_size_t u_size = u->size, v_size = u_size + whole;

    shift %= ZZ_LIMB_T_BITS;
    if (zz_resize((uint64_t)v_size + (bool)shift, v)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    v->negative = u->negative;
    if (shift) {
        v->size -= !(bool)(v->digits[v_size] = mpn_lshift(v->digits + whole,
                                                          u->digits, u_size,
                                                          (unsigned int)shift));
    }
    else {
        mpn_copyd(v->digits + whole, u->digits, u_size);
    }
    mpn_zero(v->digits, whole);
    return ZZ_OK;
}

zz_err
zz_invert(const zz_t *u, zz_t *v)
{
    zz_size_t u_size = u->size;

    if (u->negative) {
        if (zz_resize((uint64_t)u_size, v)) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        mpn_sub_1(v->digits, u->digits, u_size, 1);
        v->size -= v->digits[u_size - 1] == 0;
    }
    else if (!u_size) {
        return zz_from_sl(-1, v);
    }
    else {
        if (zz_resize((uint64_t)u_size + 1, v)) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        v->digits[u_size] = mpn_add_1(v->digits, u->digits, u_size, 1);
        v->size -= v->digits[u_size] == 0;
    }
    v->negative = !u->negative;
    return ZZ_OK;
}

zz_err
zz_and(const zz_t *u, const zz_t *v, zz_t *w)
{
    if (!u->size || !v->size) {
        return zz_from_sl(0, w);
    }

    zz_size_t u_size = u->size, v_size = v->size;

    if (u->negative || v->negative) {
        zz_t o1, o2;

        if (zz_init(&o1) || zz_init(&o2)) {
            /* LCOV_EXCL_START */
err:
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }
        if (u->negative) {
            if (zz_invert(u, &o1)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            o1.negative = true;
            u = &o1;
            u_size = u->size;
        }
        if (v->negative) {
            if (zz_invert(v, &o2)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            o2.negative = true;
            v = &o2;
            v_size = v->size;
        }
        if (u_size < v_size) {
            SWAP(const zz_t *, u, v);
            SWAP(zz_size_t, u_size, v_size);
        }
        if (u->negative && v->negative) {
            if (!u_size) {
                zz_clear(&o1);
                zz_clear(&o2);
                return zz_from_sl(-1, w);
            }
            if (zz_resize((uint64_t)u_size + 1, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = true;
            mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
            if (v_size) {
                mpn_ior_n(w->digits, u->digits, v->digits, v_size);
            }
            w->digits[u_size] = mpn_add_1(w->digits, w->digits, u_size, 1);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
        else if (u->negative) {
            assert(v_size > 0);
            if (zz_resize((uint64_t)v_size, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = false;
            mpn_andn_n(w->digits, v->digits, u->digits, v_size);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
        else {
            assert(u_size > 0);
            if (zz_resize((uint64_t)u_size, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = false;
            if (v_size) {
                mpn_andn_n(w->digits, u->digits, v->digits, v_size);
            }
            mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
    }
    if (u_size < v_size) {
        SWAP(const zz_t *, u, v);
        SWAP(zz_size_t, u_size, v_size);
    }
    w->negative = false;
    for (zz_size_t i = v_size; --i >= 0;) {
        if (u->digits[i] & v->digits[i]) {
            v_size = i + 1;
            if (zz_resize((uint64_t)v_size, w)) {
                return ZZ_MEM; /* LCOV_EXCL_LINE */
            }
            mpn_and_n(w->digits, u->digits, v->digits, v_size);
            zz_normalize(w);
            return ZZ_OK;
        }
    }
    w->size = 0;
    return ZZ_OK;
}

zz_err
zz_or(const zz_t *u, const zz_t *v, zz_t *w)
{
    if (!u->size) {
        return zz_copy(v, w);
    }
    if (!v->size) {
        return zz_copy(u, w);
    }

    zz_size_t u_size = u->size, v_size = v->size;

    if (u->negative || v->negative) {
        zz_t o1, o2;

        if (zz_init(&o1) || zz_init(&o2)) {
            /* LCOV_EXCL_START */
err:
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }
        if (u->negative) {
            if (zz_invert(u, &o1)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            o1.negative = true;
            u = &o1;
            u_size = o1.size;
        }
        if (v->negative) {
            if (zz_invert(v, &o2)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            o2.negative = true;
            v = &o2;
            v_size = o2.size;
        }
        if (u_size < v_size) {
            SWAP(const zz_t *, u, v);
            SWAP(zz_size_t, u_size, v_size);
        }
        if (u->negative && v->negative) {
            if (!v_size) {
                zz_clear(&o1);
                zz_clear(&o2);
                return zz_from_sl(-1, w);
            }
            if (zz_resize((uint64_t)v_size + 1, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = true;
            mpn_and_n(w->digits, u->digits, v->digits, v_size);
            w->digits[v_size] = mpn_add_1(w->digits, w->digits, v_size, 1);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
        else if (u->negative) {
            assert(v_size > 0);
            if (zz_resize((uint64_t)u_size + 1, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = true;
            mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
            mpn_andn_n(w->digits, u->digits, v->digits, v_size);
            w->digits[u_size] = mpn_add_1(w->digits, w->digits, u_size, 1);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
        else {
            assert(u_size > 0);
            if (zz_resize((uint64_t)v_size + 1, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = true;
            if (v_size) {
                mpn_andn_n(w->digits, v->digits, u->digits, v_size);
                w->digits[v_size] = mpn_add_1(w->digits, w->digits, v_size, 1);
                zz_normalize(w);
            }
            else {
                w->digits[0] = 1;
            }
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
    }
    if (u_size < v_size) {
        SWAP(const zz_t *, u, v);
        SWAP(zz_size_t, u_size, v_size);
    }
    if (zz_resize((uint64_t)u_size, w)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    w->negative = false;
    mpn_ior_n(w->digits, u->digits, v->digits, v_size);
    if (u_size != v_size) {
        mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
    }
    return ZZ_OK;
}

zz_err
zz_xor(const zz_t *u, const zz_t *v, zz_t *w)
{
    if (!u->size) {
        return zz_copy(v, w);
    }
    if (!v->size) {
        return zz_copy(u, w);
    }

    zz_size_t u_size = u->size, v_size = v->size;

    if (u->negative || v->negative) {
        zz_t o1, o2;

        if (zz_init(&o1) || zz_init(&o2)) {
            /* LCOV_EXCL_START */
err:
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }
        if (u->negative) {
            if (zz_invert(u, &o1)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            o1.negative = true;
            u = &o1;
            u_size = o1.size;
        }
        if (v->negative) {
            if (zz_invert(v, &o2)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            o2.negative = true;
            v = &o2;
            v_size = o2.size;
        }
        if (u_size < v_size) {
            SWAP(const zz_t *, u, v);
            SWAP(zz_size_t, u_size, v_size);
        }
        if (u->negative && v->negative) {
            if (!u_size) {
                zz_clear(&o1);
                zz_clear(&o2);
                return zz_from_sl(0, w);
            }
            if (zz_resize((uint64_t)u_size, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = false;
            mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
            if (v_size) {
                mpn_xor_n(w->digits, u->digits, v->digits, v_size);
            }
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
        else if (u->negative) {
            assert(v_size > 0);
            if (zz_resize((uint64_t)u_size + 1, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = true;
            mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
            mpn_xor_n(w->digits, v->digits, u->digits, v_size);
            w->digits[u_size] = mpn_add_1(w->digits, w->digits, u_size, 1);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
        else {
            assert(u_size > 0);
            if (zz_resize((uint64_t)u_size + 1, w)) {
                goto err; /* LCOV_EXCL_LINE */
            }
            w->negative = true;
            mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
            if (v_size) {
                mpn_xor_n(w->digits, u->digits, v->digits, v_size);
            }
            w->digits[u_size] = mpn_add_1(w->digits, w->digits, u_size, 1);
            zz_normalize(w);
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_OK;
        }
    }
    if (u_size < v_size) {
        SWAP(const zz_t *, u, v);
        SWAP(zz_size_t, u_size, v_size);
    }
    if (zz_resize((uint64_t)u_size, w)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    w->negative = false;
    mpn_xor_n(w->digits, u->digits, v->digits, v_size);
    if (u_size != v_size) {
        mpn_copyi(&w->digits[v_size], &u->digits[v_size], u_size - v_size);
    }
    else {
        zz_normalize(w);
    }
    return ZZ_OK;
}

zz_err
zz_pow(const zz_t *u, zz_limb_t v, zz_t *w)
{
    if (u == w) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(u, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = zz_pow(&tmp, v, w);

        zz_clear(&tmp);
        return ret;
    }
    if (!v) {
        return zz_from_sl(1, w);
    }
    if (!u->size) {
        return zz_from_sl(0, w);
    }
    if (zz_cmp_sl(u, 1) == ZZ_EQ) {
        return zz_from_sl(1, w);
    }
    if (v > MIN(ZZ_LIMB_T_MAX, ZZ_SIZE_T_MAX / (uint64_t)u->size)) {
        return ZZ_BUF;
    }

    zz_size_t w_size = (zz_size_t)(v * (zz_limb_t)u->size);
    zz_limb_t *tmp = malloc((size_t)w_size * ZZ_LIMB_T_BYTES);

    if (!tmp || zz_resize((uint64_t)w_size, w)) {
        /* LCOV_EXCL_START */
        free(tmp);
        zz_clear(w);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    w->negative = u->negative && v%2;
    w->size = (zz_size_t)mpn_pow_1(w->digits, u->digits, u->size, v, tmp);
    free(tmp);
    if (zz_resize((uint64_t)w->size, w)) {
        /* LCOV_EXCL_START */
        zz_clear(w);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    return ZZ_OK;
}

static zz_err
zz_gcd(const zz_t *u, const zz_t *v, zz_t *w)
{
    if (!u->size) {
        if (zz_abs(v, w) == ZZ_MEM) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        return ZZ_OK;
    }
    if (!v->size) {
        if (zz_abs(u, w) == ZZ_MEM) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        return ZZ_OK;
    }

    mp_limb_t shift = MIN(mpn_scan1(u->digits, 0), mpn_scan1(v->digits, 0));
    zz_t *volatile o1 = malloc(sizeof(zz_t));
    zz_t *volatile o2 = malloc(sizeof(zz_t));

    if (!o1 || !o2) {
        goto free; /* LCOV_EXCL_LINE */
    }
    if (zz_init(o1) || zz_init(o2)) {
        goto clear; /* LCOV_EXCL_LINE */
    }
    if (zz_abs(u, o1) || zz_abs(v, o2)) {
        goto clear; /* LCOV_EXCL_LINE */
    }
    if (shift && (zz_quo_2exp(o1, shift, o1) || zz_quo_2exp(o2, shift, o2))) {
        goto clear; /* LCOV_EXCL_LINE */
    }
    u = o1;
    v = o2;
    if (u->size < v->size) {
        SWAP(const zz_t *, u, v);
    }
    if (zz_resize((uint64_t)v->size, w) == ZZ_MEM || TMP_OVERFLOW) {
        goto clear; /* LCOV_EXCL_LINE */
    }
    assert(v->size);
    w->size = (zz_size_t)mpn_gcd(w->digits, u->digits, u->size, v->digits,
                                 v->size);
    w->negative = false;
    zz_clear(o1);
    zz_clear(o2);
    free(o1);
    free(o2);
    if (shift && zz_mul_2exp(w, shift, w)) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    return ZZ_OK;
    /* LCOV_EXCL_START */
clear:
    zz_clear(o1);
    zz_clear(o2);
free:
    free(o1);
    free(o2);
    return ZZ_MEM;
    /* LCOV_EXCL_STOP */
}

zz_err
zz_gcdext(const zz_t *u, const zz_t *v, zz_t *g, zz_t *s, zz_t *t)
{
    if (!s && !t) {
        return zz_gcd(u, v, g);
    }
    if (u->size < v->size) {
        SWAP(const zz_t *, u, v);
        SWAP(zz_t *, s, t);
    }
    if (!v->size) {
        if (g) {
            if (zz_abs(u, g) == ZZ_MEM) {
                return ZZ_MEM; /* LCOV_EXCL_LINE */
            }
        }
        if (s) {
            if (zz_from_sl(u->negative ? -1 : 1, s) == ZZ_MEM) {
                return ZZ_MEM; /* LCOV_EXCL_LINE */
            }
            s->size = u->size > 0;
        }
        if (t) {
            t->size = 0;
            t->negative = false;
        }
        return ZZ_OK;
    }

    zz_t *volatile o1 = malloc(sizeof(zz_t));
    zz_t *volatile o2 = malloc(sizeof(zz_t));
    zz_t *volatile tmp_g = malloc(sizeof(zz_t));
    zz_t *volatile tmp_s = malloc(sizeof(zz_t));

    if (!o1 || !o2 || !tmp_g || !tmp_s) {
        goto free; /* LCOV_EXCL_LINE */
    }

    if (zz_init(o1) || zz_init(o2)
        || zz_init(tmp_g) || zz_init(tmp_s)
        || zz_copy(u, o1) || zz_copy(v, o2)
        || zz_resize((uint64_t)v->size, tmp_g)
        || zz_resize((uint64_t)v->size + 1, tmp_s)
        || TMP_OVERFLOW)
    {
        goto clear; /* LCOV_EXCL_LINE */
    }

    mp_size_t ssize;

    tmp_g->size = (zz_size_t)mpn_gcdext(tmp_g->digits, tmp_s->digits, &ssize,
                                        o1->digits, u->size, o2->digits,
                                        v->size);
    tmp_s->size = (zz_size_t)imaxabs(ssize);
    tmp_s->negative = ((u->negative && ssize > 0)
                       || (!u->negative && ssize < 0));
    tmp_g->negative = false;
    zz_clear(o1);
    zz_clear(o2);
    free(o1);
    free(o2);
    o1 = o2 = NULL;
    if (t) {
        if (zz_mul(u, tmp_s, t) || zz_sub(tmp_g, t, t)
            || zz_div(t, v, t, NULL))
        {
            goto clear; /* LCOV_EXCL_LINE */
        }
    }
    if (s && zz_copy(tmp_s, s) == ZZ_MEM) {
        goto clear; /* LCOV_EXCL_LINE */
    }
    if (g && zz_copy(tmp_g, g) == ZZ_MEM) {
        goto clear; /* LCOV_EXCL_LINE */
    }
    zz_clear(tmp_s);
    zz_clear(tmp_g);
    return ZZ_OK;
    /* LCOV_EXCL_START */
clear:
    zz_clear(o1);
    zz_clear(o2);
    zz_clear(tmp_g);
    zz_clear(tmp_s);
free:
    free(o1);
    free(o2);
    free(tmp_g);
    free(tmp_s);
    return ZZ_MEM;
    /* LCOV_EXCL_STOP */
}

static zz_err
zz_inverse(const zz_t *u, const zz_t *v, zz_t *w)
{
    zz_t g;

    if (zz_init(&g) || zz_gcdext(u, v, &g, w, NULL) == ZZ_MEM) {
        /* LCOV_EXCL_START */
        zz_clear(&g);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    if (zz_cmp_sl(&g, 1) == ZZ_EQ) {
        zz_clear(&g);
        return ZZ_OK;
    }
    zz_clear(&g);
    return ZZ_VAL;
}

zz_err
zz_lcm(const zz_t *u, const zz_t *v, zz_t *w)
{
    zz_t g;

    if (zz_init(&g) || zz_gcd(u, v, &g)) {
        /* LCOV_EXCL_START */
err:
        zz_clear(&g);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    if (zz_div(u, &g, &g, NULL) || zz_mul(&g, v, w)) {
        goto err; /* LCOV_EXCL_LINE */
    }
    zz_clear(&g);
    (void)zz_abs(w, w);
    return ZZ_OK;
}

zz_err
zz_powm(const zz_t *u, const zz_t *v, const zz_t *w, zz_t *res)
{
    if (!w->size) {
        return ZZ_VAL;
    }
    if (u == res) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(u, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = zz_powm(&tmp, v, w, res);

        zz_clear(&tmp);
        return ret;
    }
    if (v == res) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(v, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = zz_powm(u, &tmp, w, res);

        zz_clear(&tmp);
        return ret;
    }
    if (w == res) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(w, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = zz_powm(u, v, &tmp, res);

        zz_clear(&tmp);
        return ret;
    }

    zz_t o1, o2;

    if (zz_init(&o1) || zz_init(&o2)) {
        /* LCOV_EXCL_START */
mem:
        zz_clear(&o1);
        zz_clear(&o2);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    if (v->negative) {
        zz_err ret = zz_inverse(u, w, &o2);

        if (ret == ZZ_VAL) {
            zz_clear(&o1);
            zz_clear(&o2);
            return ZZ_VAL;
        }
        if (ret == ZZ_MEM || zz_abs(v, &o1)) {
            goto mem; /* LCOV_EXCL_LINE */
        }
        u = &o2;
        v = &o1;
    }
    if (u->size > INT_MAX || v->size > INT_MAX || w->size > INT_MAX) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }

    mpz_t z;
    TMP_MPZ(b, u)
    TMP_MPZ(e, v)
    TMP_MPZ(m, w)
    if (TMP_OVERFLOW) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    mpz_init(z);
    mpz_powm(z, b, e, m);
    if (zz_resize((uint64_t)z->_mp_size, res)) {
        /* LCOV_EXCL_START */
        mpz_clear(z);
        goto mem;
        /* LCOV_EXCL_STOP */
    }
    res->negative = false;
    mpn_copyi(res->digits, z->_mp_d, res->size);
    mpz_clear(z);
    if (w->negative && res->size && zz_add(w, res, res)) {
        goto mem; /* LCOV_EXCL_LINE */
    }
    zz_clear(&o1);
    zz_clear(&o2);
    return ZZ_OK;
}

zz_err
zz_sqrtrem(const zz_t *u, zz_t *v, zz_t *w)
{
    if (u->negative) {
        return ZZ_VAL;
    }
    v->negative = false;
    if (!u->size) {
        v->size = 0;
        if (w) {
            w->size = 0;
            w->negative = false;
        }
        return ZZ_OK;
    }
    if (u == v) {
        zz_t tmp;

        if (zz_init(&tmp) || zz_copy(v, &tmp)) {
            /* LCOV_EXCL_START */
            zz_clear(&tmp);
            return ZZ_MEM;
            /* LCOV_EXCL_STOP */
        }

        zz_err ret = zz_sqrtrem(&tmp, v, w);

        zz_clear(&tmp);
        return ret;
    }
    if (zz_resize(((uint64_t)u->size + 1)/2, v) == ZZ_MEM || TMP_OVERFLOW) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }
    if (w) {
        w->negative = false;
        if (zz_resize((uint64_t)u->size, w) == ZZ_MEM) {
            return ZZ_MEM; /* LCOV_EXCL_LINE */
        }
        w->size = (zz_size_t)mpn_sqrtrem(v->digits, w->digits, u->digits,
                                         u->size);
    }
    else {
        mpn_sqrtrem(v->digits, NULL, u->digits, u->size);
    }
    return ZZ_OK;
}

zz_err
zz_fac(zz_limb_t u, zz_t *v)
{
#if ULONG_MAX < ZZ_LIMB_T_MAX
    if (n > ULONG_MAX || k > ULONG_MAX) {
        return ZZ_BUF;
    }
#endif
    if (TMP_OVERFLOW) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }

    mpz_t z;

    mpz_init(z);
    mpz_fac_ui(z, (unsigned long)u);
    if (zz_resize((uint64_t)z->_mp_size, v) == ZZ_MEM) {
        /* LCOV_EXCL_START */
        mpz_clear(z);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    mpn_copyi(v->digits, z->_mp_d, z->_mp_size);
    mpz_clear(z);
    return ZZ_OK;
}

zz_err
zz_bin(zz_limb_t n, zz_limb_t k, zz_t *v)
{
#if ULONG_MAX < ZZ_LIMB_T_MAX
    if (n > ULONG_MAX || k > ULONG_MAX) {
        return ZZ_BUF;
    }
#endif
    if (TMP_OVERFLOW) {
        return ZZ_MEM; /* LCOV_EXCL_LINE */
    }

    mpz_t z;

    mpz_init(z);
    mpz_bin_uiui(z, (unsigned long)n, (unsigned long)k);
    if (zz_resize((uint64_t)z->_mp_size, v) == ZZ_MEM) {
        /* LCOV_EXCL_START */
        mpz_clear(z);
        return ZZ_MEM;
        /* LCOV_EXCL_STOP */
    }
    mpn_copyi(v->digits, z->_mp_d, z->_mp_size);
    mpz_clear(z);
    return ZZ_OK;
}
