#!/bin/bash

# Clean all DPU results
parent_folder="./results/dpu"

find "$parent_folder" -type f -name "*.csv" -delete

echo "Removed all DPU results."