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

void
check_fac_outofmem(void)
{
    for (size_t i = 0; i < 7; i++) {
        uint64_t x = 12811 + (uint64_t)(rand() % 12173);
        zz_t mx;

        if (zz_init(&mx)) {
            abort();
        }
        while (1) {
            zz_err r = zz_fac(x, &mx);

            if (r != ZZ_OK) {
                if (r == ZZ_MEM) {
                    break;
                }
                abort();
            }
            x *= 2;
        }
        zz_clear(&mx);
    }
}

void
check_square_outofmem(void)
{
    for (size_t i = 0; i < 7; i++) {
        int64_t x = 49846727467293 + rand();
        zz_t mx;

        if (zz_init(&mx) || zz_from_sl(x, &mx)) {
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
    size_t nthreads = 7;
    int ret, succ = 0;

    pthread_t *tid = malloc(nthreads * sizeof(pthread_t));
    data_t *d = malloc(nthreads * sizeof(data_t));
    for (size_t i = 0; i < nthreads; i++) {
        if (zz_init(&d[i].z) || zz_from_sl(10 + 201*(int)i, &d[i].z)) {
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
}
#endif /* HAVE_PTHREAD_H */

/* Poor-mans allocator routines with memory constraint.
 * total_size should be reset on ZZ_MEM errors. */

atomic_size_t total_size = 0;
static size_t max_size = 0;

static void *
my_malloc(size_t size)
{
    size_t cur_size = atomic_load(&total_size);

    if (cur_size + size > max_size) {
        return NULL;
    }

    void *ptr = malloc(size);

    if (ptr) {
        atomic_fetch_add(&total_size, size);
    }
    return ptr;
}

static void *
my_realloc(void *ptr, size_t old_size, size_t new_size)
{
    size_t cur_size = atomic_load(&total_size);

    if (cur_size + new_size - old_size > max_size) {
        return NULL;
    }

    void *new_ptr = realloc(ptr, new_size);

    if (new_ptr) {
        atomic_fetch_add(&total_size, new_size);
        atomic_fetch_sub(&total_size, old_size);
    }
    return new_ptr;
}

static void
my_free(void *ptr, size_t size)
{
    free(ptr);
    if (!size) {
        atomic_fetch_sub(&total_size, size);
    }
}

void
check_fac_outofmem2(void)
{
    for (size_t i = 0; i < 7; i++) {
        uint64_t x = 12811 + (uint64_t)(rand() % 12173);
        zz_t mx;

        if (zz_init(&mx)) {
            abort();
        }
        while (1) {
            zz_err r = zz_fac(x, &mx);

            if (r != ZZ_OK) {
                if (r == ZZ_MEM) {
                    atomic_store(&total_size, 0);
                    break;
                }
                abort();
            }
            x *= 2;
        }
        zz_clear(&mx);
    }
}


int
main(void)
{
    srand((unsigned int)time(NULL));
    zz_setup(NULL);
    zz_set_memory_funcs(my_malloc, my_realloc, my_free);
    max_size = 32*1000*1000;
    check_fac_outofmem2();
    zz_set_memory_funcs(NULL, NULL, NULL); /* coverage */
#ifdef HAVE_VALGRIND_VALGRIND_H
    if (RUNNING_ON_VALGRIND) {
        zz_finish();
        return 0;
    }
#endif

    struct rlimit new, old;

    if (getrlimit(RLIMIT_AS, &old)) {
        perror("getrlimit");
        return 1;
    }
    new.rlim_max = old.rlim_max;
    new.rlim_cur = 128*1000*1000;
    if (setrlimit(RLIMIT_AS, &new)) {
        perror("setrlimit");
        return 1;
    }
    new.rlim_cur = 64*1000*1000;
    if (setrlimit(RLIMIT_AS, &new)) {
        perror("setrlimit");
        return 1;
    }
    check_square_outofmem();
#if HAVE_PTHREAD_H
    check_square_outofmem_pthread();
#endif
    new.rlim_cur = 32*1000*1000;
    if (setrlimit(RLIMIT_AS, &new)) {
        perror("setrlimit");
        return 1;
    }
    /* to trigger crash for GMP builds with alloca() enabled */
    if (getrlimit(RLIMIT_STACK, &old)) {
        perror("getrlimit");
        return 1;
    }
    new.rlim_max = old.rlim_max;
    new.rlim_cur = 128*1000;
    if (setrlimit(RLIMIT_STACK, &new)) {
        perror("setrlimit");
        return 1;
    }
    check_fac_outofmem();
    zz_finish();
    return 0;
}
