#ifndef COMMON_H_
#define COMMON_H_

#define BUFFER_SIZE (1 << 7) // 128: must be 8 byte aligned
#define NUM_TASKLETS 16      // 11
#define MODULUS 65537        // (2^16 + 1)

#define NUM_THREADS 4
#define USE_THREADS 1

#include <stdint.h>


#define INT 1

/**
 * To avoid
 */
#define MAX_BUFFER_SIZE (1 << 16)

/* Use blocks of 256 bytes */
#define BLOCK_SIZE (256) // wram/mram block sizes

// Value of math.pi
#define PI 3.14159265358979323846

// Complex number
typedef struct
{
    double real;
    double imag;
} Complex;

typedef struct dpu_set_t dpu_set_t;

typedef struct {
    int* p;
    int* q;
    int* result;
    int threadId;   
    int n;

} ThreadArgs;


enum poly_op
{
    POLY_ADD, // 0
    POLY_MULTI_NAIVE,
    POLY_MULTI_COEFFWISE,
    POLY_MULTI_FFT

};
typedef enum poly_op poly_op;

typedef struct dpu_args
{
    int N_D;
    int N_T;
    int num_runs;
    int dpu_id;
    poly_op dpu_op;
} dpu_args;

typedef struct
{
    uint32_t cycles;
} dpu_result_t;

typedef struct
{
    dpu_result_t tasklet_result[NUM_TASKLETS];
} dpu_results_t;

// Copyright: PrIM bench
#define divceil(n, m) (((n)-1) / (m) + 1)
#define roundup(n, m) ((n / m) * m + m)

// Integer types
//  Data type
#ifdef UINT32
#define T uint32_t
#elif UINT64
#define T uint64_t
#elif INT32
#define T int32_t
#elif INT64
#define T int64_t
#elif INT
#define T int
#endif

#endif // COMMON_H_