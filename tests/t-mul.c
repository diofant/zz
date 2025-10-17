/*
    Copyright (C) 2024, 2025 Sergey B Kirpichev

    This file is part of the ZZ Library.

    The ZZ Library is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License (LGPL) as
    published by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.  See
    <https://www.gnu.org/licenses/>.
*/

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

#include "zz.h"

void check_square_outofmem()
{
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

void check_square_outofmem_pthread()
{
    size_t nthreads = 7;

    pthread_t *tid = malloc(nthreads * sizeof(pthread_t));
    data_t *d = malloc(nthreads * sizeof(data_t));
    for (size_t i = 0; i < nthreads; i++) {
        if (zz_init(&d[i].z) || zz_from_i64(10 + 201*i, &d[i].z)) {
            abort();
        }
        if (pthread_create(&tid[i], NULL, worker, (void *)(d + i))) {
            abort();
        }
    }
    for (size_t i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
        if (d[i].ret) {
            abort();
        }
        zz_clear(&d[i].z);
    }
    free(d);
    free(tid);
}
#endif /* HAVE_PTHREAD_H */

int main(void)
{
    srand((unsigned int)time(NULL));
    zz_setup(NULL);

    struct rlimit new, old;

    if (getrlimit(RLIMIT_AS, &old)) {
        fprintf(stderr, "can't query memory limits\n");
        return 1;
    }
    new.rlim_max = old.rlim_max;
    new.rlim_max = new.rlim_cur = 64*1000*1000;
    if (setrlimit(RLIMIT_AS, &new)) {
        fprintf(stderr, "can't set memory limits\n");
        return 1;
    }
    check_square_outofmem();
#if HAVE_PTHREAD_H
    check_square_outofmem_pthread();
#endif
    return 0;
}
