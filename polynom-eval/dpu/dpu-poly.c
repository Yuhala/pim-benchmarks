

/**
 * Author: Peterson Yuhala
 * Testing PIM processing on DPUs
 */

#include <stdio.h>
#include <alloc.h>
#include <mram.h>
#include <stdint.h>
#include <defs.h>
#include <handshake.h>
#include <perfcounter.h>
#include <barrier.h>
#include <mutex.h>

#include "../common.h"
#include "dpu-poly.h"

MUTEX_INIT(sum_mutex); //

//-------------------------------

/**
 * Useful documentation
 * __host: WRAM variable
 * __mram_: MRAM variable
 */

/**
 * Measures the number of cycles taken by DPU program.
 * Use mutex to
 */
uint32_t nb_cycles;

/**
 * Total cycles on all DPUs.
 * Use mutex to add nb_cycles to this value
 */
__host uint32_t total_dpu_cycles;

/**
 * Represents the degree of the polynomial
 */
__host uint32_t dpu_n; // todo remove

/**
 * Represents the DPU id.
 * This value can be used to calculate the start and stop indices of a tasklet
 */
__host int dpu_id;

/**
 * Number of data elements processed per DPU
 */
__host int N_D;

/**
 * Number of data elements processed per tasklet
 */
__host int N_T;

__host dpu_args dpu_input;

/**
 * DPU buffers: data will be copied from the host to these buffers
 */

__mram_noinit int dpu_p[MAX_BUFFER_SIZE];   // represents polynomial p
__mram_noinit int dpu_q[MAX_BUFFER_SIZE];   // represents polynomial q
__mram_noinit int dpu_res[MAX_BUFFER_SIZE]; // represents result of polynomial operation

//-------------------------------

/* Use blocks of 256 bytes */
#define BLOCK_SIZE (256)

__dma_aligned uint8_t DPU_CACHES[NUM_TASKLETS][BLOCK_SIZE];

__mram_noinit uint8_t DPU_BUFFER[BUFFER_SIZE];

/**
 * This program sums the elements of an array using N tasklets
 */

__host uint32_t tsums[NUM_TASKLETS];
//__host uint32_t array_sum = 0;
/**
 * Param task_size represents the chunk of work to be done by each tasklet
 */

// input
__mram_noinit uint8_t dpu_buffer[BUFFER_SIZE];

// output
__mram_noinit uint8_t partial_sums[NUM_TASKLETS];

void dpu_vector_sum(uint8_t task_size)
{
    // all tasklets will wait for the array to be filled before proceeding
    // handshake_wait_for(ARRAY_FILL_TASKLET);
    // get tasklet id
    uint8_t tid = me();       // tasklet ids start at 0
    uint32_t partial_sum = 0; // prevents using stale value after reboot

    // calculate start and stop index based on tasklet id
    // uint8_t start_index = (idx == 0) ? 0 : (idx - 1) * task_size;
    uint8_t start_index = tid * task_size;
    uint8_t stop_index = start_index + task_size;

    // printf("DPU array sum!\n");
    for (uint8_t i = start_index; i < stop_index; i++)
    {
        partial_sum += dpu_buffer[i];
    }
    printf(">>>>>> Partial sum from tasklet %u is: %d \n", tid, partial_sum);
    // mutex_lock(sum_mutex);
    // tsums[tid] = partial_sum;
    // mutex_unlock(sum_mutex);
    /* mutex_lock(sum_mutex);
    dpu_sum += partial_sum;
    mutex_unlock(sum_mutex);
    printf(">>>>>> Tasklet %u reporting: array sum = %d \n", tid, dpu_sum); */
    // return partial_sum;
}

void dpu_poly_add(int *p_mram, int *q_mram, int *res_mram)
{
    uint8_t tid = me();
    int task_size = dpu_input.N_T;

    // Byte index offset in MRAM buffer on which this tasklet operates
    uint32_t start_index = tid * task_size * sizeof(T);
    uint32_t stop_index = start_index + task_size * sizeof(T);

    // Polynomial vector caches in WRAM
    T *p_wram = (T *)mem_alloc(BLOCK_SIZE);
    T *q_wram = (T *)mem_alloc(BLOCK_SIZE);
    T *res_wram = (T *)mem_alloc(BLOCK_SIZE);

    int num_elts_per_block = BLOCK_SIZE / sizeof(int);

    // Run bench num_runs times
    //for (int k = 0; k < dpu_input.num_runs; k++)
    //{
        for (uint32_t i = start_index; i < stop_index; i += BLOCK_SIZE)
        {
            // load cache with chunk of data
            mram_read((__mram_ptr void const *)p_mram + i, p_wram, BLOCK_SIZE);
            mram_read((__mram_ptr void const *)q_mram + i, q_wram, BLOCK_SIZE);

            for (int elt_index = 0; elt_index < num_elts_per_block; elt_index++)
            {
                res_wram[elt_index] = (p_wram[elt_index] + q_wram[elt_index]) % MODULUS;
            }
            mram_write(res_wram, (__mram_ptr void *)res_mram + i, BLOCK_SIZE);
        }

        /* for (int i = 0; i < dpu_input.N_T; i++)
        {
            res[i] = p[i] + q[i] % MODULUS;
        } */
    //}
}

void dpu_poly_multi_coeffwise(int *p_mram, int *q_mram, int *res_mram)
{
    uint8_t tid = me();
    int task_size = dpu_input.N_T;

    // Byte index offset in MRAM buffer on which this tasklet operates
    uint32_t start_index = tid * task_size * sizeof(T);
    uint32_t stop_index = start_index + task_size * sizeof(T);

    // Polynomial vector caches in WRAM
    T *p_wram = (T *)mem_alloc(BLOCK_SIZE);
    T *q_wram = (T *)mem_alloc(BLOCK_SIZE);
    T *res_wram = (T *)mem_alloc(BLOCK_SIZE);

    int num_elts_per_block = BLOCK_SIZE / sizeof(int);

    // Run bench num_runs times
    //for (int k = 0; k < dpu_input.num_runs; k++)
    //{
        for (uint32_t i = start_index; i < stop_index; i += BLOCK_SIZE)
        {
            // load cache with chunk of data
            mram_read((__mram_ptr void const *)p_mram + i, p_wram, BLOCK_SIZE);
            mram_read((__mram_ptr void const *)q_mram + i, q_wram, BLOCK_SIZE);

            for (int elt_index = 0; elt_index < num_elts_per_block; elt_index++)
            {
                res_wram[elt_index] = (p_wram[elt_index] * q_wram[elt_index]) % MODULUS;
            }
            mram_write(res_wram, (__mram_ptr void *)res_mram + i, BLOCK_SIZE);
        }

        /* for (int i = 0; i < dpu_input.N_T; i++)
        {
            res[i] = p[i] + q[i] % MODULUS;
        } */
    //}
}

void dpu_poly_multi_naive(int *p_mram, int *q_mram, int *res_mram)
{
    uint8_t tid = me();
    int task_size = dpu_input.N_T;

    // Byte index offset in MRAM buffer on which this tasklet operates
    uint32_t start_index = tid * task_size * sizeof(T);
    uint32_t stop_index = start_index + task_size * sizeof(T);

    // Polynomial vector caches in WRAM
    T *p_wram = (T *)mem_alloc(BLOCK_SIZE);
    T *q_wram = (T *)mem_alloc(BLOCK_SIZE);
    T *res_wram = (T *)mem_alloc(2 * BLOCK_SIZE);

    int num_elts_per_block = BLOCK_SIZE / sizeof(int);

    // Run bench num_runs times
    //for (int k = 0; k < dpu_input.num_runs; k++)
    //{
        for (uint32_t i = start_index; i < stop_index; i += BLOCK_SIZE)
        {
            // load cache with chunk of data
            mram_read((__mram_ptr void const *)p_mram + i, p_wram, BLOCK_SIZE);
            mram_read((__mram_ptr void const *)q_mram + i, q_wram, BLOCK_SIZE);         

            for (int elt_index_p = 0; elt_index_p < num_elts_per_block; elt_index_p++)
            {
                for (int elt_index_q = 0; elt_index_q < num_elts_per_block; elt_index_q++)
                {
                    res_wram[elt_index_p + elt_index_q] += (p_wram[elt_index_p] * q_wram[elt_index_q]) % MODULUS;
                }
                // res_wram[elt_index] = (p_wram[elt_index] * q_wram[elt_index]) % MODULUS;
            }
            mram_write(res_wram, (__mram_ptr void *)res_mram + i, 2 * BLOCK_SIZE);
        }

        /* for (int i = 0; i < dpu_input.N_T; i++)
        {
            res[i] = p[i] + q[i] % MODULUS;
        } */
    //}
}

void poly_multi_fft(int* p, int* q, int* res, int n){

}