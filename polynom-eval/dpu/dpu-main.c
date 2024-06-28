/**
 * Author: Peterson Yuhala
 * Testing PIM processing on DPUs
 */

#include <stdio.h>
#include <stdint.h>
#include <mram.h>
#include <stdint.h>
#include <defs.h>
#include <barrier.h>

#include <alloc.h>
#include <barrier.h>
#include "../common.h"
#include "dpu-poly.h"

#include <mutex.h>

MUTEX_INIT(time_mutex); //

// for perf
#include <perfcounter.h>

// #define SET_PERFCOUNTER 1

/**
 * Number of data elements processed per tasklet
 */
__host int operation;

// extern host_sum;

BARRIER_INIT(mem_barrier, NUM_TASKLETS);

extern __mram_noinit int dpu_res[MAX_BUFFER_SIZE];

extern dpu_args dpu_input;

__host dpu_results_t DPU_RESULTS;

/**
 * All cycle runtimes for DPUs
 */

extern int nb_cycles;
extern int N_D;
extern int N_T;

/**
 * Pointer offsets in MRAM
 */
#define P_OFFSET DPU_MRAM_HEAP_POINTER + (tid * N_T) * sizeof(int)
#define Q_OFFSET DPU_MRAM_HEAP_POINTER + (N_D + tid * N_T) * sizeof(int)
#define RES_OFFSET DPU_MRAM_HEAP_POINTER + (2 * N_D + tid * N_T) * sizeof(int)

int main()
{
#ifdef SET_PERFCOUNTER
    perfcounter_config(COUNT_CYCLES, true);
#endif

    N_D = dpu_input.N_D;
    N_T = dpu_input.N_T;

    uint8_t tid = me();
    dpu_result_t *result = &DPU_RESULTS.tasklet_result[tid];

    /* Initialize once the cycle counter */
    if (tid == 0)
    {
        perfcounter_config(COUNT_CYCLES, true);
        mem_reset();
    }

    // All tasklets start at the same time
    // printf("Tasklet %d, N_D = %d | N_T = %d \n", tid, dpu_input.N_D, dpu_input.N_T);
    barrier_wait(&mem_barrier);

    /**
     * Start addresses of p, q and res in MRAM
     */
    // void *p_ptr = DPU_MRAM_HEAP_POINTER + (tid * N_T) * sizeof(int);
    // void *q_ptr = DPU_MRAM_HEAP_POINTER + (N_D + tid * N_T) * sizeof(int);
    // void *res_ptr = DPU_MRAM_HEAP_POINTER + (2 * N_D + tid * N_T) * sizeof(int);

    int dpu_transfer_size = N_D * sizeof(int);
    // Polynomial vectors in MRAM
    int *p_mram = (int *)(DPU_MRAM_HEAP_POINTER);                     //(int *)(P_OFFSET);
    int *q_mram = (int *)(DPU_MRAM_HEAP_POINTER + dpu_transfer_size); //(int *)(Q_OFFSET);
    int *res_mram = (int *)(DPU_MRAM_HEAP_POINTER + 2 * dpu_transfer_size);

    // Run the algorithm on WRAM data

    switch (dpu_input.dpu_op)
    {
    case POLY_ADD:
        dpu_poly_add(p_mram, q_mram, res_mram);
        break;
    case POLY_MULTI_COEFFWISE:
        dpu_poly_multi_coeffwise(p_mram, q_mram, res_mram);
        break;
    case POLY_MULTI_NAIVE:
        dpu_poly_multi_naive(p_mram, q_mram, res_mram); // TODO
        break;
    case POLY_MULTI_FFT:
    default:
        break;
    }

    // Copy results fro WRAM to MRAM

#ifdef SET_PERFCOUNTER
    result->cycles = (uint32_t)perfcounter_get();
#endif

    // printf("Tasklet %d: Number of cyles = %d \n",tid, result->cycles);

    // printf("Tasklet %d, N_D = %d | N_T = %d \n", tid, dpu_input.N_D, dpu_input.N_T);

    // mutex_lock(time_mutex);
    // total_dpu_cycles += nb_cycles;
    // mutex_unlock(time_mutex);

    // TODO add

    return 0;
}