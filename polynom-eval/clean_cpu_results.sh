#!/bin/bash

# Clean all cpu results
parent_folder="./results/cpu"

find "$parent_folder" -type f -name "*.csv" -delete

echo "Removed all CPU results."