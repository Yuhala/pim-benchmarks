
#include <stdio.h>

#include "common.h"
#include "bench.h"

static void printHelp()
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