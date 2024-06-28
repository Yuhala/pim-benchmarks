

#include "poly.h"
#include "common.h"

#include "bench.h"

extern chunkSize;
extern pthread_t threads[NUM_THREADS];
extern int threadIDs[NUM_THREADS];
extern ThreadArgs threadArgs[NUM_THREADS];
extern double threadExecTimes[NUM_THREADS];

void *poly_multi_coeffwise_multithread(void *arg);
void *poly_multi_ntt_multithread(void *arg);
void *poly_multi_naive_multithread(void *arg);

void poly_multi_naive(int *p, int *q, int *prod, int n, double *thread_time)
{
#ifdef USE_THREADS

    // start_clock(&t);
    chunkSize = n / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threadArgs[i].p = p;
        threadArgs[i].q = q;
        threadArgs[i].result = prod;
        threadArgs[i].threadId = i;
        threadArgs[i].n = n;
        pthread_create(&threads[i], NULL, poly_multi_naive_multithread, (void *)&threadArgs[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double total_time = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        total_time += threadExecTimes[i];
    }
    total_time /= NUM_THREADS;
    *thread_time = total_time;
#else

    for (int i = 0; i < n; i++)
    {

        for (int j = 0; j < n; j++)
        {
            prod[i + j] += (p[i] * q[j]) % MODULUS;
        }
    }
#endif
}

void *poly_multi_naive_multithread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    Timer t;

    // start_clock(&t);
    int tid = args->threadId;

    int start = tid * chunkSize;
    int end = (tid + 1) * chunkSize;
    // printf("Thread %d working from start: %d to stop: %d >>>>>>\n", tid, start, end);

    for (int i = start; i < end; i++)
    {
        // res[i] = (p[i] + q[i]) % MODULUS;

        for (int j = start; j < end; j++)
        {
            // prod[i + j] += (p[i] * q[j]) % MODULUS;
            args->result[i + j] += (args->p[i] * args->q[j]) % MODULUS;
        }
    }
    // stop_clock(&t);
    // threadExecTimes[tid] = (double)(t.stop - t.start);
}

void poly_multi_coeffwise(int *p, int *q, int *prod, int n, double *thread_time)
{
    Timer t;
#ifdef USE_THREADS

    // start_clock(&t);
    chunkSize = n / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threadArgs[i].p = p;
        threadArgs[i].q = q;
        threadArgs[i].result = prod;
        threadArgs[i].threadId = i;
        pthread_create(&threads[i], NULL, poly_multi_coeffwise_multithread, (void *)&threadArgs[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double total_time = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        total_time += threadExecTimes[i];
    }
    total_time /= NUM_THREADS;
    *thread_time = total_time;

#else
    for (int i = 0; i < n; i++)
    {
        prod[i] = (p[i] * q[i]) % MODULUS;
    }
#endif
}

void *poly_multi_coeffwise_multithread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    Timer t;

    // start_clock(&t);
    int tid = args->threadId;

    int start = tid * chunkSize;
    int end = (tid + 1) * chunkSize;
    // printf("Thread %d working from start: %d to stop: %d >>>>>>\n", tid, start, end);

    for (int i = start; i < end; i++)
    {
        // res[i] = (p[i] + q[i]) % MODULUS;
        args->result[i] = (args->p[i] * args->q[i]) % MODULUS;
    }
    // stop_clock(&t);
    // threadExecTimes[tid] = (double)(t.stop - t.start);
    //  printf(" Thread %d time = %f >>>>>\n", tid, threadExecTimes[tid]);
}

void *poly_multi_ntt_multithread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    Timer t;

    // start_clock(&t);
    int tid = args->threadId;

    int start = tid * chunkSize;
    int end = (tid + 1) * chunkSize;
    // printf("Thread %d working from start: %d to stop: %d >>>>>>\n", tid, start, end);

    for (int i = start; i < end; i++)
    {
        // res[i] = (p[i] + q[i]) % MODULUS;
        args->result = (args->p[i] * args->q[i]) % MODULUS;
    }
    // stop_clock(&t);
    // threadExecTimes[tid] = (double)(t.stop - t.start);
}

void poly_multi_ntt(int *p, int *q, int *res, int n, double *thread_time)
{
}
