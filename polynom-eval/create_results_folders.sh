#!/bin/bash

# Create the results folder
mkdir -p results

# Create subfolders for cpu and dpu results
mkdir -p results/cpu
mkdir -p results/cpu/addition
mkdir -p results/cpu/naive_multi
mkdir -p results/cpu/coeffwise_multi


mkdir -p results/dpu
mkdir -p results/dpu/addition
mkdir -p results/dpu/naive_multi
mkdir -p results/dpu/coeffwise_multi

mkdir -p results/dpu/dpu_64
mkdir -p results/dpu/dpu_128
mkdir -p results/dpu/dpu_256
mkdir -p results/dpu/dpu_512
mkdir -p results/dpu/dpu_1024


echo "Results folders created successfully."