
#include <stdio.h>

#include "common.h"
#include "bench.h"

// forward declarations
static void printCPU_usage();

void run_cpu_bench(poly_op op, int n)
{

    /**
     * CPU benchmarks
     */

    switch (op)
    {
    case POLY_ADD:
        printf("----- Running CPU polynomial addition benchmark ----\n");
        bench_poly_add(n);
        break;

    case POLY_MULTI_COEFFWISE:
        printf("----- Running CPU point-wise polynomial multiplication benchmark ----\n");
        bench_poly_multi_coeffwise(n);
        break;

    case POLY_MULTI_NAIVE:
        printf("----- Running CPU naive polynomial multiplication benchmark ----\n");
        bench_poly_multi_naive(n);
        break;

    default:
        printCPU_usage();
        exit(0);
        break;
    }
}

static void printCPU_usage()
{
    printf("-----------------  Usage  -----------------\n"
           "- ./cpu-poly-bench <benchmark name> <polynomial size>)\n"
           "- E.g., ./cpu-poly-bench addition 1024\n"
           "---------------------------------------------\n");
}

int main(int argc, char **argv)
{
    srand(time(NULL)); // initialize seed once
    poly_op op;

    if (argc != 3)
    {
        printCPU_usage();
        exit(0);
    }

    if (!folder_exists("./results"))
    {
        printf("You must run script to create results folder.\n");
        exit(0);
    }

    int n = atoi(argv[2]);

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

    run_cpu_bench(op, n);

    return 0;
}