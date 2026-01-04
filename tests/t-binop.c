/*
    Copyright (C) 2024-2026 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#include "tests/tests.h"

#define ZZ_BINOP_REF(op)                                \
    zz_err                                              \
    zz_ref_##op(const zz_t *u, const zz_t *v, zz_t *w)  \
    {                                                   \
        mpz_t z;                                        \
        TMP_MPZ(mu, u);                                 \
        TMP_MPZ(mv, v);                                 \
        if (TMP_OVERFLOW) {                             \
            return ZZ_MEM;                              \
        }                                               \
        mpz_init(z);                                    \
        mpz_##op(z, mu, mv);                            \
                                                        \
        zz_t tmp = {z->_mp_size < 0, abs(z->_mp_size),  \
                    abs(z->_mp_size),                   \
                    z->_mp_d};                          \
        if (zz_copy(&tmp, w)) {                         \
            mpz_clear(z);                               \
            return ZZ_MEM;                              \
        }                                               \
        mpz_clear(z);                                   \
        return ZZ_OK;                                   \
    }

#define TEST_BINOP_EXAMPLE(op, lhs, rhs)                      \
    do {                                                      \
        zz_t u, v, w, r;                                      \
                                                              \
        if (zz_init(&u) || zz_init(&v) || zz_init(&r)         \
            || zz_init(&w))                                   \
        {                                                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(lhs, &u) || zz_copy(rhs, &v)) {           \
            abort();                                          \
        }                                                     \
                                                              \
        zz_err ret = zz_##op(&u, &v, &w);                     \
                                                              \
        if (ret == ZZ_VAL) {                                  \
            zz_clear(&u);                                     \
            zz_clear(&v);                                     \
            zz_clear(&w);                                     \
            zz_clear(&r);                                     \
            continue;                                         \
        }                                                     \
        else if (ret != ZZ_OK) {                              \
            abort();                                          \
        }                                                     \
        if (zz_ref_##op(&u, &v, &r)                           \
            || zz_cmp(&w, &r) != ZZ_EQ)                       \
        {                                                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(&u, &w) || zz_##op(&w, &v, &w)            \
            || zz_cmp(&w, &r) != ZZ_EQ) {                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(&v, &w) || zz_##op(&u, &w, &w)            \
            || zz_cmp(&w, &r) != ZZ_EQ) {                     \
            abort();                                          \
        }                                                     \
        zz_clear(&u);                                         \
        zz_clear(&v);                                         \
        zz_clear(&w);                                         \
        zz_clear(&r);                                         \
    } while (0);

#define TEST_MIXBINOP_EXAMPLE(op, lhs, rhs)                   \
    TEST_BINOP_EXAMPLE(op, lhs, rhs)                          \
    do {                                                      \
        zz_t u, v, w, r;                                      \
                                                              \
        if (zz_init(&u) || zz_init(&v) || zz_init(&w)         \
            || zz_init(&r))                                   \
        {                                                     \
            abort();                                          \
        }                                                     \
        if (zz_copy(lhs, &u) || zz_copy(rhs, &v)) {           \
            abort();                                          \
        }                                                     \
                                                              \
        int64_t val;                                          \
                                                              \
        if (zz_to_i64(&v, &val) == ZZ_OK) {                   \
            zz_err ret = zz_##op##_i64(&u, val, &w);          \
                                                              \
            if (ret == ZZ_VAL) {                              \
                zz_clear(&u);                                 \
                zz_clear(&v);                                 \
                zz_clear(&w);                                 \
                zz_clear(&r);                                 \
                continue;                                     \
            }                                                 \
            else if (ret) {                                   \
                abort();                                      \
            }                                                 \
            if (zz_ref_##op(&u, &v, &r)                       \
                || zz_cmp(&w, &r) != ZZ_EQ)                   \
            {                                                 \
                abort();                                      \
            }                                                 \
            if (zz_copy(&u, &w) || zz_##op##_i64(&w, val, &w) \
                || zz_cmp(&w, &r) != ZZ_EQ)                   \
            {                                                 \
                abort();                                      \
            }                                                 \
        }                                                     \
        if (zz_to_i64(&u, &val) == ZZ_OK) {                   \
            zz_err ret = zz_i64_##op(val, &v, &w);            \
                                                              \
            if (ret == ZZ_VAL) {                              \
                zz_clear(&u);                                 \
                zz_clear(&v);                                 \
                zz_clear(&w);                                 \
                zz_clear(&r);                                 \
                continue;                                     \
            }                                                 \
            else if (ret) {                                   \
                abort();                                      \
            }                                                 \
            if (zz_ref_##op(&u, &v, &r)                       \
                || zz_cmp(&w, &r) != ZZ_EQ)                   \
            {                                                 \
                abort();                                      \
            }                                                 \
            if (zz_copy(&v, &w) || zz_i64_##op(val, &w, &w)   \
                || zz_cmp(&w, &r) != ZZ_EQ)                   \
            {                                                 \
                abort();                                      \
            }                                                 \
        }                                                     \
        zz_clear(&u);                                         \
        zz_clear(&v);                                         \
        zz_clear(&w);                                         \
        zz_clear(&r);                                         \
    } while (0);

#define TEST_BINOP(op, bs, neg)                                \
    void                                                       \
    check_##op##_bulk(void)                                    \
    {                                                          \
        for (size_t i = 0; i < nsamples; i++) {                \
            zz_t lhs, rhs;                                     \
                                                               \
            if (zz_init(&lhs) || zz_random(bs, neg, &lhs)) {   \
                abort();                                       \
            }                                                  \
            if (zz_init(&rhs) || zz_random(bs, neg, &rhs)) {   \
                abort();                                       \
            }                                                  \
            TEST_BINOP_EXAMPLE(op, &lhs, &rhs);                \
            zz_clear(&lhs);                                    \
            zz_clear(&rhs);                                    \
        }                                                      \
    }

#define TEST_MIXBINOP(op, bs, neg)                             \
    void                                                       \
    check_##op##_bulk(void)                                    \
    {                                                          \
        for (size_t i = 0; i < nsamples; i++) {                \
            zz_t lhs, rhs;                                     \
                                                               \
            if (zz_init(&lhs) || zz_random(bs, neg, &lhs)) {   \
                abort();                                       \
            }                                                  \
            if (zz_init(&rhs) || zz_random(bs, neg, &rhs)) {   \
                abort();                                       \
            }                                                  \
            TEST_MIXBINOP_EXAMPLE(op, &lhs, &rhs);             \
            zz_clear(&lhs);                                    \
            zz_clear(&rhs);                                    \
        }                                                      \
    }

#define zz_i64_add(x, y, r) zz_add_i64((y), (x), (r))
#define zz_i64_mul(x, y, r) zz_mul_i64((y), (x), (r))

ZZ_BINOP_REF(add)
ZZ_BINOP_REF(sub)
ZZ_BINOP_REF(mul)

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
zz_fdiv_q_i64(const zz_t *u, int64_t v, zz_t *w)
{
    return zz_div_i64(u, v, w, NULL);
}

zz_err
zz_fdiv_r_i64(const zz_t *u, int64_t v, zz_t *w)
{
    return zz_div_i64(u, v, NULL, w);
}

zz_err
zz_i64_fdiv_q(int64_t u, const zz_t *v, zz_t *w)
{
    return zz_i64_div(u, v, w, NULL);
}

zz_err
zz_i64_fdiv_r(int64_t u, const zz_t *v, zz_t *w)
{
    return zz_i64_div(u, v, NULL, w);
}

ZZ_BINOP_REF(fdiv_q)
ZZ_BINOP_REF(fdiv_r)

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

zz_err
zz_ref_mul_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w)
{
    mpz_t z;
    TMP_MPZ(mu, u);
    if (TMP_OVERFLOW) {
        return ZZ_MEM;
    }
    mpz_init(z);
    mpz_mul_2exp(z, mu, v);

    zz_t tmp = {z->_mp_size < 0, abs(z->_mp_size),
                abs(z->_mp_size),
                z->_mp_d};
    if (zz_copy(&tmp, w)) {
        mpz_clear(z);
        return ZZ_MEM;
    }
    mpz_clear(z);
    return ZZ_OK;
}

zz_err
zz_ref_quo_2exp(const zz_t *u, zz_bitcnt_t v, zz_t *w)
{
    mpz_t z;
    TMP_MPZ(mu, u);
    if (TMP_OVERFLOW) {
        return ZZ_MEM;
    }
    mpz_init(z);
    mpz_fdiv_q_2exp(z, mu, v);

    zz_t tmp = {z->_mp_size < 0, abs(z->_mp_size),
                abs(z->_mp_size),
                z->_mp_d};
    if (zz_copy(&tmp, w)) {
        mpz_clear(z);
        return ZZ_MEM;
    }
    mpz_clear(z);
    return ZZ_OK;
}

TEST_MIXBINOP(add, 512, true)
TEST_MIXBINOP(sub, 512, true)
TEST_MIXBINOP(mul, 512, true)

TEST_MIXBINOP(fdiv_q, 512, true)
TEST_MIXBINOP(fdiv_r, 512, true)

TEST_BINOP(and, 512, true)
TEST_BINOP(ior, 512, true)
TEST_BINOP(xor, 512, true)

TEST_BINOP(gcd, 512, true)
TEST_BINOP(lcm, 512, true)

void
check_binop_examples(void)
{
    zz_t u, v;

    if (zz_init(&u) || zz_init(&v)) {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_from_i64(0, &v) || zz_add(&u, &v, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(1, &v) || zz_add(&u, &v, &u) || zz_cmp_i64(&u, 1) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_add_i64(&u, 0, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_add_i64(&u, 1, &u)
        || zz_cmp_i64(&u, 1) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(0, &v) || zz_mul(&u, &v, &u) || zz_cmp_i64(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(1, &u) || zz_mul_i64(&u, 0, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_div_i64(&u, 1, &u, NULL) || zz_cmp_i64(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_div_i64(&u, 1, NULL, &u) || zz_cmp_i64(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(2, &u) || zz_div_i64(&u, 2, NULL, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(2, &v) || zz_and(&u, &v, &u) || zz_cmp_i64(&u, 0) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(-1, &u) || zz_from_i64(-1, &v) || zz_and(&u, &v, &u)
        || zz_cmp_i64(&u, -1) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(1, &u) || zz_from_i64(2, &v) || zz_and(&u, &v, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(2, &v) || zz_or(&u, &v, &u) || zz_cmp_i64(&u, 2) != ZZ_EQ) {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_from_i64(2, &v) || zz_or(&v, &u, &u)
        || zz_cmp_i64(&u, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(-1, &u) || zz_from_i64(-1, &v) || zz_or(&u, &v, &u)
        || zz_cmp_i64(&u, -1) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(12, &u) || zz_from_i64(-1, &v) || zz_or(&u, &v, &u)
        || zz_cmp_i64(&u, -1) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_from_i64(2, &v) || zz_xor(&v, &u, &u)
        || zz_cmp_i64(&u, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_from_i64(2, &v) || zz_xor(&u, &v, &u)
        || zz_cmp_i64(&u, 2) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(-1, &u) || zz_from_i64(-1, &v) || zz_xor(&u, &v, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(0, &u) || zz_from_i64(0, &v) || zz_lcm(&u, &v, &u)
        || zz_cmp_i64(&u, 0) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(4, &u)) {
        abort();
    }
    if (zz_from_i64(2, &v)) {
        abort();
    }
    if (zz_div(&u, &v, NULL, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_from_i64(0, &v) || zz_div(&u, &v, &v, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_from_i64(1, &u)) {
        abort();
    }
    if (zz_div_i64(&u, 0, &u, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_from_i64(0, &v)) {
        abort();
    }
    if (zz_i64_div(1, &v, &v, NULL) != ZZ_VAL) {
        abort();
    }
    if (zz_from_i64(1, &v) || zz_i64_div(1, &v, NULL, NULL) != ZZ_VAL) {
        abort();
    }
    zz_clear(&u);
    zz_clear(&v);
}

void
check_lshift_bulk(void)
{
    zz_bitcnt_t bs = 512;

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u, w, r;
        zz_bitcnt_t v = (zz_bitcnt_t)rand() % 12345;

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }
        if (zz_init(&w) || zz_mul_2exp(&u, v, &w)) {
            abort();
        }
        if (zz_init(&r) || zz_ref_mul_2exp(&u, v, &r)
            || zz_cmp(&w, &r) != ZZ_EQ)
        {
            abort();
        }
        zz_clear(&u);
        zz_clear(&w);
        zz_clear(&r);
    }
}

void
check_rshift_bulk(void)
{
    zz_bitcnt_t bs = 512;

    for (size_t i = 0; i < nsamples; i++) {
        zz_t u, w, r;
        zz_bitcnt_t v = (zz_bitcnt_t)rand();

        if (zz_init(&u) || zz_random(bs, true, &u)) {
            abort();
        }
        if (zz_init(&w) || zz_quo_2exp(&u, v, &w)) {
            abort();
        }
        if (zz_init(&r) || zz_ref_quo_2exp(&u, v, &r)
            || zz_cmp(&w, &r) != ZZ_EQ)
        {
            abort();
        }
        zz_clear(&u);
        zz_clear(&w);
        zz_clear(&r);
    }
}

#define zz_from_dec(s, u) zz_from_str(s, strlen(s), 10, u)

void
check_shift_examples(void)
{
    zz_t u, v;

    if (zz_init(&u) || zz_from_i64(0, &u) || zz_init(&v)) {
        abort();
    }
    if (zz_mul_2exp(&u, 123, &v) || zz_cmp_i64(&v, 0)) {
        abort();
    }
    if (zz_quo_2exp(&u, 123, &v) || zz_cmp_i64(&v, 0)) {
        abort();
    }
    if (zz_from_dec("-340282366920938463444927863358058659840", &u)
        || zz_quo_2exp(&u, 64, &v))
    {
        abort();
    }
    if (zz_from_dec("-18446744073709551615", &u)
        || zz_cmp(&u, &v) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_dec("-514220174162876888173427869549172"
                    "032807104958010493707296440352", &u)
        || zz_quo_2exp(&u, 206, &v) || zz_cmp_i64(&v, -6) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_dec("-62771017353866807634955070562867279"
                    "52638980837032266301441", &u)
        || zz_quo_2exp(&u, 128, &v))
    {
        abort();
    }
    if (zz_from_dec("-18446744073709551616", &u) || zz_cmp(&u, &v)) {
        abort();
    }
    if (zz_from_i64(-1, &u) || zz_quo_2exp(&u, 1, &v)
        || zz_cmp_i64(&v, -1) != ZZ_EQ)
    {
        abort();
    }
    if (zz_from_i64(1, &u) ||
        zz_mul_2exp(&u, ZZ_BITS_MAX, &u) != ZZ_MEM)
    {
        abort();
    }
    if (zz_from_i64(0x7fffffffffffffffLL, &u)) {
        abort();
    }
    if (zz_mul_2exp(&u, 1, &u) || zz_add_i64(&u, 1, &u)
        || zz_mul_2exp(&u, 64, &u) || zz_quo_2exp(&u, 64, &u))
    {
        abort();
    }
    if (u.negative || u.alloc < 1 || u.size != 1
        || u.digits[0] != 0xffffffffffffffffULL)
    {
        abort();
    }
    if (zz_from_i64(0x7fffffffffffffffLL, &v)) {
        abort();
    }
    if (zz_mul_2exp(&v, 1, &v) || zz_add_i64(&v, 1, &v)
        || zz_cmp(&u, &v) != ZZ_EQ)
    {
        abort();
    }
#if ZZ_LIMB_T_BITS == 64
    if (zz_from_i64(1, &u) || zz_mul_2exp(&u, 64, &u)
        || zz_pow(&u, ((zz_limb_t)1<<63), &u) != ZZ_BUF)
    {
        abort();
    }
#endif
    zz_clear(&u);
    zz_clear(&v);
}

void
check_square_outofmem(void)
{
    zz_set_memory_funcs(my_malloc, my_realloc, my_free);
    max_size = 64*1000*1000;
    for (size_t i = 0; i < 7; i++) {
        int64_t x = 49846727467293 + rand();
        zz_t mx;

        if (zz_init(&mx) || zz_from_i64(x, &mx)) {
            abort();
        }
        while (1) {
            zz_err r = zz_mul(&mx, &mx, &mx);

            if (r != ZZ_OK) {
                if (r == ZZ_MEM) {
                    break;
                }
                abort();
            }
        }
        zz_clear(&mx);
    }
    zz_set_memory_funcs(NULL, NULL, NULL);
}

#if HAVE_PTHREAD_H
typedef struct {
    int ret;
    zz_t z;
} data_t;

void *
worker(void *args)
{
    data_t *d = (data_t *)args;

    while (1) {
        zz_err ret = zz_mul(&d->z, &d->z, &d->z);

        if (ret != ZZ_OK) {
            if (ret == ZZ_MEM) {
                break;
            }
            d->ret = 1;
            return NULL;
        }
    }
    d->ret = 0;
    return NULL;
}

void
check_square_outofmem_pthread(void)
{
    zz_set_memory_funcs(my_malloc, my_realloc, my_free);
    max_size = 64*1000*1000;

    size_t nthreads = 7;
    int ret, succ = 0;

    pthread_t *tid = malloc(nthreads * sizeof(pthread_t));
    data_t *d = malloc(nthreads * sizeof(data_t));
    for (size_t i = 0; i < nthreads; i++) {
        if (zz_init(&d[i].z) || zz_from_i64(10 + 201*(int)i, &d[i].z)) {
            abort();
        }
        ret = pthread_create(&tid[i], NULL, worker, (void *)(d + i));
        if (!ret) {
            succ |= (1<<i);
        }
        else if (ret != EAGAIN) {
            perror("pthread_create");
            abort();
        }
    }
    if (!succ) {
        abort();
    }
    for (size_t i = 0; i < nthreads; i++) {
        if (succ & (1<<i)) {
            pthread_join(tid[i], NULL);
            if (d[i].ret) {
                abort();
            }
            zz_clear(&d[i].z);
        }
    }
    free(d);
    free(tid);
    zz_set_memory_funcs(NULL, NULL, NULL);
}
#endif /* HAVE_PTHREAD_H */

int
main(void)
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
    check_binop_examples();
    check_lshift_bulk();
    check_rshift_bulk();
    check_shift_examples();
    check_square_outofmem();
#if HAVE_PTHREAD_H
    check_square_outofmem_pthread();
#endif
    zz_finish();
    zz_testclear();
    return 0;
}
