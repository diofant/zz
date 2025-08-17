/*

compile:
$ cc -I. -L.libs example-pthreads.c -lzz -lpthread

run:
$ LD_LIBRARY_PATH=.libs/ ./a.out 7
0: 1
1: 1
2: 1
3: 1
4: 1
5: 1
6: 1

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/resource.h>
#include "zz.h"

typedef struct {
    int ret;
    zz_t z;
} data_t;

void *
worker(void *args)
{
    data_t *d = (data_t *)args;

    for (size_t i = 0; i < 100; i++) {
        if (zz_mul(&d->z, &d->z, &d->z)) {
            d->ret = 1;
            return NULL;
        }
    }
    d->ret = 0;
    return NULL;
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "pass nthreads argument\n");
        return 1;
    }

    size_t nthreads = atoi(argv[1]);
    pthread_t *tid = malloc(nthreads * sizeof(pthread_t));
    data_t *d = malloc(nthreads * sizeof(data_t));
    struct rlimit  old, new;

    if (getrlimit(RLIMIT_AS, &old)) {
        fprintf(stderr, "can't query memory limits\n");
        return 1;
    }
    new.rlim_max = old.rlim_max;
    new.rlim_cur = 64*1000*1000;
    if (setrlimit(RLIMIT_AS, &new)) {
        fprintf(stderr, "can't set memory limits\n");
        return 1;
    }
    zz_setup(NULL);
    for (size_t i = 0; i < nthreads; i++) {
        if (zz_init(&d[i].z) || zz_from_i64(10 + 201*i, &d[i].z)) {
            printf("Can't init %lu's integer\n", i);
            return 1;
        }
        int ret = pthread_create(&tid[i], NULL, worker, (void *)(d + i));
        if (ret) {
            printf("Error in pthread_create: %s\n", strerror(ret));
        }
    }
    for (size_t i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
        printf("%lu: %d\n", i, d[i].ret);
        zz_clear(&d[i].z);
    }
    free(d);
    free(tid);
    if (setrlimit(RLIMIT_AS, &old)) {
        fprintf(stderr, "can't set memory limits back\n");
        return 1;
    }
    return 0;
}
