#ifndef BENCH_H_
#define BENCH_H_

#include "common.h"

#include <pthread.h>
#include <time.h>

#define DPU_BINARY "./main.dpu"

#define WARM_UP 20
#define RUNS 100 // number of runs for a benchmark

/**
 * @struct dpu_runtime
 * @brief DPU execution times
 *
 */
typedef struct dpu_timer
{
    int n;
    int num_runs;
    int num_dpus;
    double global_start;
    double global_stop_time;
    double cpu_dpu_copy_time;
    double dpu_cpu_copy_time;
    double dpu_exec_time;
    clock_t start;
    clock_t stop;
    char bench_name[32];
    double exec_time_prepare;
    double exec_time_load;
    double exec_time_alloc;
    double exec_time_copy_in;
    double exec_time_launch;
    double exec_time_copy_out;
    double exec_time_free;
    int exp;

} dpu_timer;

typedef struct
{
    clock_t start;
    clock_t stop;
    int num_runs;
    double time_elapsed;
    double avg_time;
    double init_time;
    char bench_name[32];
    int n;   // polynomial size
    int exp; // exponent of 2 corresponding to n
} Timer;

// Timer functions
void start_clock(Timer *timer);
void stop_clock(Timer *timer);
void log_stats(Timer *timer);
void start_dpu_timer(dpu_timer *timer);
void stop_dpu_timer(dpu_timer *timer);

// Benchmarking polynomial operations
int findExponent(int n);
int *generate_poly(int n);
int *generate_zero_poly(int n);
void bench_poly_add();
void bench_poly_multi_naive();
void bench_poly_multi_coeffwise();
int allocate_polynomials(int n, poly_op op);
void free_polynomials();
void print_results_buffer(int n);

// DPU bench
void populate_dpu_mram(dpu_set_t dpu_set, dpu_set_t dpu, int n, int num_dpus, poly_op op);
uint32_t get_dpu_cycles(dpu_set_t dpu_set, dpu_set_t dpu, int n, int num_dpus);

void get_dpu_polynomial_result(dpu_set_t dpu_set, dpu_set_t dpu, int n, int num_dpus, poly_op op);


void bench_dpu_addition(int vary_dpus);
void bench_dpu_coeffwise_multi(int vary_dpus);
void bench_dpu_naive_multi(int vary_dpus);

// Writing results
void write_results_n(const char *path, int n, double time, const char *col_name);

void write_results(const char *path, Timer *timer);
void write_dpu_results(const char *path, dpu_timer *timer);

void log_raw_results(const char *path, double clock_cycles);

#endif // BENCH_H_