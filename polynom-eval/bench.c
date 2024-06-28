#include <dpu.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "poly.h"
#include "bench.h"

// Polynomial degrees
#define POLY_SIZES 6

// #define USE_BIG_POLY 1

int chunkSize;
pthread_t threads[NUM_THREADS];
int threadIDs[NUM_THREADS];
ThreadArgs threadArgs[NUM_THREADS];
double threadExecTimes[NUM_THREADS];
double clock_ticks;

int *p;	  // polynomial p
int *q;	  // polynomial q
int *res; // result of polynomial operation

/**
 * Number of elements per DPU
 */
int N_D;
/**
 * Number of elements per tasklet
 */
int N_T;

int n_dpu_8bytes;

int poly_sizes[] = {1 << 10, 1 << 12, 1 << 14, 1 << 16, 1 << 18, 1 << 20};

int big_poly[] = {1 << 22, 1 << 24, 1 << 26, 1 << 28, 1 << 30, 1 << 32};

int dpu_num[] = {64, 128, 256, 512, 1024, 2048};
#define DPU_NUM 6

// forward declarations
void printHelp();

void start_clock(Timer *timer)
{
	timer->start = clock();
}

void stop_clock(Timer *timer)
{
	timer->stop = clock();
}

void start_dpu_timer(dpu_timer *timer)
{
	timer->start = clock();
}

void stop_dpu_timer(dpu_timer *timer)
{
	timer->stop = clock();
}

/**
 * Find the exponent of a power of 2
 * For example: n = 1024 --> exponent = 10
 */
int findExponent(int n)
{
	int exponent = 0;
	while (n > 1)
	{
		n /= 2;
		exponent++;
	}
	return exponent;
}

void test()
{
	int q[] = {1, 2, 3};
	int p[] = {2, 4, 0};
	int expected[] = {2, 8, 14, 0, 12};

	int *pq = generate_zero_poly(5);
	double thread_time = 0;

	poly_multi_naive(p, q, pq, 5, &thread_time);

	printf("Product pq: ");
	for (int i = 0; i < 5; i++)
	{
		printf("%d, ", pq[i]);
	}
	printf("\n");
	free(pq);
}

/**
 * Write results to a file.
 * Functions with similar prefix "write_results" vary the last value, for ex N
 * If we are varying DPUs, we do: write_results_dpu
 */
void write_results_n(const char *path, int n, double time, const char *col_name)
{

	FILE *fptr = fopen(path, "ab+");

	// TODO: write CSV header

	// write values:exp, n, cpu_time
	fprintf(fptr, "%d, %d, %f\n", n, time);
	fclose(fptr);
}

void write_results(const char *path, Timer *timer)
{
	FILE *fptr = fopen(path, "ab+");
	timer->exp = findExponent(timer->n);

	// write values:exp, n, cpu_time
	fprintf(fptr, "%d, %d, %f\n", timer->exp, timer->n, timer->avg_time);
	fclose(fptr);
}

// check if folder exists
bool folder_exists(const char *path)
{
	struct stat sb;
	if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		return true;
	}
	else
	{
		false;
	}
}

/**
 * Write DPU results to file
 */
void write_dpu_results(const char *path, dpu_timer *timer)
{
	FILE *fptr = fopen(path, "a+");
	timer->exp = findExponent(timer->n);
	// CSV header: exp, num_dpus, n, dpu_exec_time, dpu_time_launch, cpu-dpu, dpu-cpu
	fprintf(fptr, "%d, %d, %d, %f, %f, %f, %f\n", timer->exp, timer->num_dpus, timer->n, timer->dpu_exec_time, timer->exec_time_launch, timer->cpu_dpu_copy_time, timer->dpu_cpu_copy_time);
	fclose(fptr);
}

void log_raw_results(const char *path, double value)
{
	// open file in append mode
	double secs = value / CLOCKS_PER_SEC;
	double milli_secs = secs * 1000;
	FILE *fptr = fopen(path, "a+");
	fprintf(fptr, "%f\n", milli_secs);
	fclose(fptr);
}

/**
 * Delete all results
 */
void clean_results()
{
	system("rm ./results/*.csv");
}

void log_stats(Timer *timer)
{
	// get elapsed time in seconds
	// timer->time_elapsed = (double)(timer->stop - timer->start) / CLOCKS_PER_SEC;
	// timer->time_elapsed -= (thread_init_time * timer->num_runs); //TODO

	// double avg_time = timer->time_elapsed / timer->num_runs;
	//  printf("Avg. elapsed time = %fs\n", timer->time_elapsed);

	timer->time_elapsed /= CLOCKS_PER_SEC;
	double avg_time = timer->time_elapsed / timer->num_runs;

	timer->avg_time = timer->time_elapsed / timer->num_runs;

	// create file name
	char path[64] = "./results/";
	strcat(path, timer->bench_name);

	write_results(path, timer);

	// write_results_n(path, timer->n, avg_time, "xx");
}

/**
 * Generate random polynomial of degree n -1.
 * All coefficients are taken modulus Q
 */
int *generate_poly(int n)
{

	int *polynomial = (int *)malloc(n * sizeof(int));

	for (int i = 0; i < n; i++)
	{
		polynomial[i] = rand() % MODULUS;
		// polynomial[i] = i % MODULUS;
	}
	return polynomial;
}

int allocate_polynomials(int n, poly_op op)
{
	p = generate_poly(n);
	q = generate_poly(n);

	int res_size = 0;

	switch (op)
	{
	case POLY_ADD:
	case POLY_MULTI_COEFFWISE:
		res_size = n;
		res = generate_poly(res_size);
		break;

	case POLY_MULTI_NAIVE:
		res_size = 2 * n; // the actual size is 2n - 1; we use 2n to have a multiple of 2
		res = generate_zero_poly(res_size);
		break;

	case POLY_MULTI_FFT:
		res_size = 2 * n;
		res = generate_poly(res_size);
		break;

	default:
		break;
	}

	return res_size;
}

void free_polynomials()
{
	free(p);
	free(q);
	free(res);
}

void print_results_buffer(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("Res[%d] =  %d \n", i, res[i]);
	}
}
/**
 * Generate polynomial of degree n - 1 with zero coefficients
 */
int *generate_zero_poly(int n)
{

	int *polynomial = (int *)malloc(n * sizeof(int));

	for (int i = 0; i < n; i++)
	{
		polynomial[i] = 0;
	}
	return polynomial;
}

void run_cpu_bench()
{

	/**
	 * CPU benchmarks
	 */

	bench_poly_add();
	printf("----- CPU bench: addition complete ----\n");

	bench_poly_multi_coeffwise();
	printf("----- CPU bench: coeffwise multi complete ----\n");

	bench_poly_multi_naive();
	printf("----- CPU bench: naive multi complete ----\n");
}

/**
 * Benchmark polynomial summation for various sizes
 */
void bench_poly_add()
{
	Timer t;
	t.num_runs = RUNS;
	t.init_time = 0;
	strcpy(t.bench_name, "cpu_poly_add.csv");
	char cpu_time_path[256];

	int size;
	double thread_time;
	double total;
	// int *p, *q, *pq;

	for (int i = 0; i < POLY_SIZES; i++)
	{
		thread_time = 0;
		total = 0;
		// Generate polynomials

#ifdef USE_BIG_POLY
		size = big_poly[i];
#else
		size = poly_sizes[i];
#endif

		allocate_polynomials(size, POLY_ADD);

		t.n = size;
		int exp = findExponent(size);
		// sprintf(cpu_time_path, "%s %d %s", "./results/cpu/addition/cpu_time_", exp, ".csv");
		sprintf(cpu_time_path, "./results/cpu/addition/cpu_time_%d.csv", exp);

		/**
		 * Measure and log CPU execution times
		 */
		for (int run_ctr = 0; run_ctr < WARM_UP + RUNS; run_ctr++)
		{

			start_clock(&t);
			poly_add(p, q, res, size, &thread_time);
			stop_clock(&t);
			clock_ticks = (double)(t.stop - t.start);

			// only log after warm up
			if (run_ctr >= WARM_UP)
			{
				log_raw_results(cpu_time_path, clock_ticks);
			}
		}

		// stop bench and log stats
		// stop_clock(&t);
		// t.time_elapsed = total;

		// log_stats(&t);
		// printf("-----------> Bench: %s n = : %d elapsed time = %f\n", t.bench_name, size, t.time_elapsed / RUNS);

		// print_results_buffer(2);

		// cleanup
		free_polynomials();
	}
}

/**
 * Benchmark naive polynomial multiplication on CPU
 */
void bench_poly_multi_naive()
{
	Timer t;
	t.num_runs = RUNS;
	strcpy(t.bench_name, "cpu_poly_multi_naive.csv");
	char cpu_time_path[256];

	int n;
	double thread_time;
	double total;

	for (int i = 0; i < POLY_SIZES - 1; i++)
	{

// Generate polynomials
#ifdef USE_BIG_POLY
		n = big_poly[i];
#else
		n = poly_sizes[i];
#endif
		thread_time = 0;
		total = 0;
		allocate_polynomials(n, POLY_MULTI_NAIVE);

		t.n = n;

		int exp = findExponent(n);
		// sprintf(cpu_time_path, "%s %d %s", "./results/cpu/naive_multi/cpu_time_", exp, ".csv");
		sprintf(cpu_time_path, "./results/cpu/naive_multi/cpu_time_%d.csv", exp);

		/**
		 * Measure and log CPU execution times
		 */
		for (int run_ctr = 0; run_ctr < WARM_UP + RUNS; run_ctr++)
		{

			start_clock(&t);
			poly_multi_naive(p, q, res, n, &thread_time);
			stop_clock(&t);
			clock_ticks = (double)(t.stop - t.start);
			// only log after warm up
			if (run_ctr >= WARM_UP)
			{
				log_raw_results(cpu_time_path, clock_ticks);
			}
		}

		// cleanup
		free_polynomials();
	}
}
void bench_poly_multi_coeffwise()
{
	Timer t;
	t.num_runs = RUNS;
	strcpy(t.bench_name, "cpu_poly_multi_coeffwise.csv");

	char cpu_time_path[256];

	int size;
	double thread_time;
	double total;

	for (int i = 0; i < POLY_SIZES; i++)
	{

// Generate polynomials
#ifdef USE_BIG_POLY
		size = big_poly[i];
#else
		size = poly_sizes[i];
#endif
		allocate_polynomials(size, POLY_MULTI_COEFFWISE);
		thread_time = 0;
		total = 0;

		t.n = size;
		int exp = findExponent(size);
		// sprintf(cpu_time_path, "%s %d %s", "./results/cpu/coeffwise_multi/cpu_time_", exp, ".csv");
		sprintf(cpu_time_path, "./results/cpu/coeffwise_multi/cpu_time_%d.csv", exp);

		/**
		 * Measure and log CPU execution times
		 */
		for (int run_ctr = 0; run_ctr < WARM_UP + RUNS; run_ctr++)
		{

			start_clock(&t);
			poly_multi_coeffwise(p, q, res, size, &thread_time);
			stop_clock(&t);
			clock_ticks = (double)(t.stop - t.start);

			// only log after warm up
			if (run_ctr >= WARM_UP)
			{
				log_raw_results(cpu_time_path, clock_ticks);
			}
		}

		// cleanup
		free_polynomials();
	}
}
void bench_poly_multi_fft()
{
	Timer t;
	t.num_runs = RUNS;
	strcpy(t.bench_name, "cpu_poly_multi_fft.csv");

	int size;

	for (int i = 0; i < POLY_SIZES; i++)
	{

		// Generate polynomials
		size = poly_sizes[i];
		allocate_polynomials(size, POLY_MULTI_FFT);

		t.n = size;
		// start bench
		start_clock(&t);
		for (int j = 0; j < RUNS; j++)
		{
			// poly_multi_coeffwise(p, q, res, size);
		}
		// stop bench and log stats
		stop_clock(&t);
		log_stats(&t);
		printf("-----------> Bench: %s n = : %d elapsed time = %f\n", t.bench_name, size, t.time_elapsed / RUNS);

		// cleanup
		free_polynomials();
	}
}

/**
 * Copies input polynomials to the DPUs.
 *
 */
void populate_dpu_mram(dpu_set_t dpu_set, dpu_set_t dpu, int n, int num_dpus, poly_op op)
{
	// int res_size = allocate_polynomials(n, op); // todo: do outside of this routine, so it is not benchmarked

	int dpu_id;

	dpu_args dpu_input;
	dpu_input.N_D = N_D;
	dpu_input.N_T = N_T;
	dpu_input.dpu_op = op;
	dpu_input.num_runs = RUNS;

	// void *p_ptr = DPU_MRAM_HEAP_POINTER + (tid * N_T) * sizeof(int);
	// void *q_ptr = DPU_MRAM_HEAP_POINTER + (N_D + tid * N_T) * sizeof(int);
	// void *res_ptr = DPU_MRAM_HEAP_POINTER + (2 * N_D + tid * N_T) * sizeof(int);

	/**
	 * Copy useful DPU arguments to DPUs
	 */
	DPU_FOREACH(dpu_set, dpu, dpu_id)
	{
		dpu_input.dpu_id = dpu_id;
		DPU_ASSERT(dpu_prepare_xfer(dpu, &dpu_input));
	}
	DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, "dpu_input", 0, sizeof(dpu_args), DPU_XFER_DEFAULT));

	/**
	 * Copy buffer chunks to the DPUs
	 * - copy p
	 * - copy q
	 */
	// copy polynomial p

	DPU_FOREACH(dpu_set, dpu, dpu_id)
	{
		// DPU_ASSERT(dpu_prepare_xfer(dpu, &p[dpu_id * N_D]));
		DPU_ASSERT(dpu_prepare_xfer(dpu, p + N_D * dpu_id));
	}
	/**
	 * Execute the above transfer: copies from source address: &p[dpu_id*N_D] to destination address: "buffer"
	 */
	// DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, "dpu_p", 0, N_D, DPU_XFER_DEFAULT));
	DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, 0, N_D * sizeof(int), DPU_XFER_DEFAULT));

	// copy polynomial q
	DPU_FOREACH(dpu_set, dpu, dpu_id)
	{
		// DPU_ASSERT(dpu_prepare_xfer(dpu, &q[dpu_id * N_D]));
		DPU_ASSERT(dpu_prepare_xfer(dpu, q + N_D * dpu_id));
	}
	// DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, "dpu_q", 0, N_D, DPU_XFER_DEFAULT));
	DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, N_D * sizeof(int), N_D * sizeof(int), DPU_XFER_DEFAULT));
}

/**
 * Returns total runtime cycles from DPUs
 */
uint32_t get_dpu_cycles(dpu_set_t dpu_set, dpu_set_t dpu, int n, int num_dpus)
{
	printf("Retrieve results\n");
	uint32_t dpu_cycles;
	uint32_t total_cycles = 0;
	bool status = true;
	dpu_results_t results[num_dpus];
	uint32_t each_dpu;

	// Get DPU tasklet cycles
	DPU_FOREACH(dpu_set, dpu, each_dpu)
	{
		DPU_ASSERT(dpu_prepare_xfer(dpu, &results[each_dpu]));
	}
	DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_FROM_DPU, "DPU_RESULTS", 0, sizeof(dpu_results_t), DPU_XFER_DEFAULT));

	DPU_FOREACH(dpu_set, dpu, each_dpu)
	{
		bool dpu_status;
		dpu_cycles = 0;

		// Retrieve tasklet results
		for (unsigned int each_tasklet = 0; each_tasklet < NUM_TASKLETS; each_tasklet++)
		{
			dpu_result_t *result = &results[each_dpu].tasklet_result[each_tasklet];

			if (result->cycles > dpu_cycles)
				dpu_cycles = result->cycles;
		}
		total_cycles += dpu_cycles;
		// printf("DPU execution time  = %g cycles\n", (double)dpu_cycles);
		// printf("performance         = %g cycles/byte\n", (double)dpu_cycles / n);
	}

	return total_cycles;
}

void get_dpu_polynomial_result(dpu_set_t dpu_set, dpu_set_t dpu, int n, int num_dpus, poly_op op)
{
	// Get DPU sub results
	uint32_t dpu_id;
	int res_size = 0;

	switch (op)
	{
	case POLY_ADD:
	case POLY_MULTI_COEFFWISE:
		res_size = n;
		break;

	case POLY_MULTI_NAIVE:
		res_size = 2 * n; // real size is 2n - 1; we use 2n to have a multiple of 2
		break;

	case POLY_MULTI_FFT:
		res_size = 2 * n;
		break;

	default:
		break;
	}

	uint32_t chunk_per_dpu = n / num_dpus;

	if (op == POLY_MULTI_NAIVE || op == POLY_MULTI_FFT)
	{
		DPU_FOREACH(dpu_set, dpu, dpu_id)
		{
			DPU_ASSERT(dpu_prepare_xfer(dpu, res + 2 * N_D * dpu_id));
		}

		DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, 2 * N_D * sizeof(T), 2 * N_D * sizeof(T), DPU_XFER_DEFAULT));
	}
	else
	{
		DPU_FOREACH(dpu_set, dpu, dpu_id)
		{
			DPU_ASSERT(dpu_prepare_xfer(dpu, res + N_D * dpu_id));
		}

		DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, 2 * N_D * sizeof(T), N_D * sizeof(T), DPU_XFER_DEFAULT));
	}
}

void run_dpu_bench(char *bench_name, poly_op op, int num_dpus, int n)
{
	struct dpu_set_t dpu_set, dpu;
	// Total cycles across all DPUs
	double total_dpu_time;
	dpu_timer timer;
	int exp;

	char base_path[128];
	char cpu_dpu_path[256];
	char dpu_cpu_path[256];
	char dpu_time_path[256];

	// create full file path: ./results/dpu/bench_name/dpu_n_100.csv

	// sprintf(base_path, "%s %s %s %s", "./results/dpu/", bench_name, "/");
	exp = findExponent(n);
	sprintf(cpu_dpu_path, "./results/dpu/%s/cpu_dpu_copy_%d.csv", bench_name, exp); // e.g., ./results/dpu/addition/cpu_dpu_copy_10.csv
	sprintf(dpu_cpu_path, "./results/dpu/%s/dpu_cpu_copy_%d.csv", bench_name, exp);
	sprintf(dpu_time_path, "./results/dpu/%s/dpu_time_%d.csv", bench_name, exp);
	// sprintf(dpu_cpu_path, "%s %s %d %s", base_path, "dpu_cpu_copy_", exp, ".csv");
	// sprintf(dpu_time_path, "%s %s %d %s", base_path, "dpu_time_", exp, ".csv");

	// strcat(path_n, bench_name);
	// strcat(path_n, "/");

	timer.n = n;
	timer.num_dpus = num_dpus;

	// int num_dpus = 1;

	DPU_ASSERT(dpu_alloc(num_dpus, NULL, &dpu_set));
	DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL));

	// uint32_t *tsums = (uint32_t *)malloc(sizeof(uint32_t) * NUM_TASKLETS);
	// int n = 1024; // poly_sizes[0];
	// poly_op op = POLY_ADD;
	// TODO: loop here over n and op

	/**
	 * Handle 8-byte alignment issues
	 * Copyright: ETHZ PriM bench
	 */

	const int input_size_8bytes = ((n * sizeof(int)) % 8) != 0 ? roundup(n, 8) : n;
	const int input_size_dpu = divceil(n, num_dpus);																					// Input size per DPU (max.)
	const unsigned int input_size_dpu_8bytes = ((input_size_dpu * sizeof(int)) % 8) != 0 ? roundup(input_size_dpu, 8) : input_size_dpu; // Input size per DPU (max.), 8-byte aligned

	/**
	 * Calculate the sizes of chunks
	 * N_D = number of data elements per DPU = n/num_dpus
	 * N_T = number of data elements per tasklet = N_D/num_tasklets
	 * We choose sizes in such a way that N_D and N_T are whole numbers
	 */
	N_D = input_size_dpu_8bytes;
	printf("---->>> N_D = %d \n", N_D);

	N_T = divceil(N_D, NUM_TASKLETS);
	printf("---->>> N_T = %d \n", N_T);

	int res_size = allocate_polynomials(input_size_8bytes * num_dpus, op);
	printf("----->>> Res size = %d \n", res_size);

	// uint8_t *sums = (uint8_t *)malloc(sizeof(uint8_t) * NUM_TASKLETS);

	/**
	 * Measure host to DPU copy time
	 */
	for (int run_ctr = 0; run_ctr < WARM_UP + RUNS; run_ctr++)
	{

		start_dpu_timer(&timer);
		populate_dpu_mram(dpu_set, dpu, n, num_dpus, op);
		stop_dpu_timer(&timer);

		timer.cpu_dpu_copy_time = (double)(timer.stop - timer.start);

		// log only after warm up
		if (run_ctr >= WARM_UP)
		{
			log_raw_results(cpu_dpu_path, timer.cpu_dpu_copy_time);
		}
	}

	/**
	 * Measure DPU execution time
	 */
	for (int run_ctr = 0; run_ctr < WARM_UP + RUNS; run_ctr++)
	{

		start_dpu_timer(&timer);
		DPU_ASSERT(dpu_launch(dpu_set, DPU_SYNCHRONOUS));
		stop_dpu_timer(&timer);

		timer.exec_time_launch = (double)(timer.stop - timer.start);
		// log only after warm up
		if (run_ctr >= WARM_UP)
		{
			log_raw_results(dpu_time_path, timer.exec_time_launch);
		}
	}
	/**
	 * Measure DPU to host copy time
	 */
	for (int run_ctr = 0; run_ctr < WARM_UP + RUNS; run_ctr++)
	{

		start_dpu_timer(&timer);
		get_dpu_polynomial_result(dpu_set, dpu, n, num_dpus, op);
		stop_dpu_timer(&timer);

		timer.dpu_cpu_copy_time = (double)(timer.stop - timer.start);
		// log only after warm up
		if (run_ctr >= WARM_UP)
		{
			log_raw_results(dpu_cpu_path, timer.dpu_cpu_copy_time);
		}
	}

	// timer.dpu_cpu_copy_time = (double)(timer.stop - timer.start) / CLOCKS_PER_SEC;

	// uint32_t total_dpu_cycles = get_dpu_cycles(dpu_set, dpu, n, num_dpus);

	/*  DPU_FOREACH(dpu_set, dpu)
	{
		DPU_ASSERT(dpu_log_read(dpu, stdout));
	} */

	// Retrieve DPU frequency
	/*
	uint32_t clocks_per_sec;
	DPU_FOREACH(dpu_set, dpu)
	{
		DPU_ASSERT(dpu_copy_from(dpu, "CLOCKS_PER_SEC", 0, &clocks_per_sec,
								 sizeof(uint32_t)));
	}
	total_dpu_time = (double)total_dpu_cycles / (clocks_per_sec * RUNS);
	timer.dpu_exec_time = total_dpu_time / num_dpus;
	*/

	// print_results_buffer(2);

	/**
	 * Write results to results file
	 */
	/*
	char path[64] = "./results/";
	strcat(path, bench_name);
	write_dpu_results(path, &timer);

	printf("Avg. DPU cycles = %d\n", total_dpu_cycles / RUNS);
	printf("Avg. DPU time = %f \n", total_dpu_time);
	*/
	DPU_ASSERT(dpu_free(dpu_set));

	// free(sums);
}

void bench_dpu_addition(int vary_dpus)
{
	int num_dpus = 1024;
	if (vary_dpus)
	{

		int n = poly_sizes[POLY_SIZES - 1];
		for (int i = 0; i < DPU_NUM; i++)
		{

			num_dpus = dpu_num[i];
			run_dpu_bench("addition", POLY_ADD, num_dpus, n);
		}
	}

	else
	{

		int n;
		// Run DPU bench for different polynomial sizes
		for (int i = 0; i < POLY_SIZES; i++)
		{

#ifdef USE_BIG_POLY
			n = big_poly[i];
#else
			n = poly_sizes[i];
#endif

			run_dpu_bench("addition", POLY_ADD, num_dpus, n);
			printf("----- DPU bench: addition complete ----\n");
		}
	}
}
void bench_dpu_coeffwise_multi(int vary_dpus)
{
	int num_dpus = 1024;
	if (vary_dpus)
	{

		int n = poly_sizes[POLY_SIZES - 1];
		for (int i = 0; i < DPU_NUM; i++)
		{

			num_dpus = dpu_num[i];
			run_dpu_bench("coeffwise_multi", POLY_MULTI_COEFFWISE, num_dpus, n);
		}
	}

	else
	{

		int n;
		// Run DPU bench for different polynomial sizes
		for (int i = 0; i < POLY_SIZES; i++)
		{

#ifdef USE_BIG_POLY
			n = big_poly[i];
#else
			n = poly_sizes[i];
#endif

			run_dpu_bench("coeffwise_multi", POLY_MULTI_COEFFWISE, num_dpus, n);
			printf("----- DPU bench: coeffwise multi complete ----\n");
		}
	}
}
void bench_dpu_naive_multi(int vary_dpus)
{
	int num_dpus = 1024;
	if (vary_dpus)
	{

		int n = poly_sizes[POLY_SIZES - 1];
		for (int i = 0; i < DPU_NUM; i++)
		{

			num_dpus = dpu_num[i];
			run_dpu_bench("naive_multi", POLY_MULTI_NAIVE, num_dpus, n);
		}
	}

	else
	{

		int n;
		// Run DPU bench for different polynomial sizes
		for (int i = 0; i < POLY_SIZES; i++)
		{

#ifdef USE_BIG_POLY
			n = big_poly[i];
#else
			n = poly_sizes[i];
#endif

			run_dpu_bench("naive_multi", POLY_MULTI_NAIVE, num_dpus, n);
			printf("----- CPU bench: naive multi complete ----\n");
		}
	}
}

void printHelp()
{
	printf("-----------------  Usage  -----------------\n"
		   "- ./dpu-poly-bench <benchmark name> <#DPUs> <polynomial size>)\n"
		   "- E.g., ./dpu-poly-bench addition 32 1024\n"
		   "---------------------------------------------\n");
}

int main(int argc, char **argv)
{
	srand(time(NULL)); // initialize seed once
	poly_op op;

	if (argc != 4)
	{
		printHelp();
		exit(0);
	}

	if (!folder_exists("./results"))
	{
		printf("You must run script to create results folder.\n");
		exit(0);
		
	}

	int num_dpus = atoi(argv[2]);
	int n = atoi(argv[3]);

	if (strcmp(argv[1], "addition") == 0)
	{
		op = POLY_ADD;
	}
	else if (strcmp(argv[1], "naive_multi") == 0)
	{
		op = POLY_MULTI_NAIVE;
	}
	else if (strcmp(argv[1], "cw_multi") == 0)
	{
		op = POLY_MULTI_COEFFWISE;
	}

	switch (op)
	{
	case POLY_ADD:
		printf("----- Running PIM polynomial addition benchmark ----\n");
		run_dpu_bench("addition", POLY_ADD, num_dpus, n);
		break;

	case POLY_MULTI_COEFFWISE:
		printf("----- Running PIM point-wise polynomial multiplication benchmark ----\n");
		run_dpu_bench("coeffwise_multi", POLY_MULTI_COEFFWISE, num_dpus, n);
		break;

	case POLY_MULTI_NAIVE:
		printf("----- Running PIM naive polynomial multiplication benchmark ----\n");
		run_dpu_bench("naive_multi", POLY_MULTI_NAIVE, num_dpus, n);
		break;

	default:
		printHelp();
		exit(0);
		break;
	}

	return 0;
}