## pim-benchmarks
- This repository contains micro-benchmarks implementing polynomial operations on [ UPMEM processing in memory (PIM)](https://www.upmem.com/) hardware.
- The main polynomial evaluations evaluated are: `point-wise addition`, `point-wise multiplication`, and `naive polynomial multiplication` or `convolutions`.
- There are corresponding CPU-based benchmarks for the same operation.
- These evaluations accompany our SRDS 2024 paper: ` Evaluating the Potential of In-Memory Processing to Accelerate Homomorphic Encryption`.
  
## How to run the benchmarks
- The benchmarks can be tested on a server with real PIM hardware or on one without real PIM hardware (emulated PIM).
- Install the [UPMEM SDK](https://sdk.upmem.com/).
- For example, the following sets up the SDK on a system with `Ubuntu 20.04 LTS`.
```
wget http://sdk-releases.upmem.com/2024.1.0/ubuntu_20.04/upmem-2024.1.0-Linux-x86_64.tar.gz
tar -xvf upmem-2024.1.0-Linux-x86_64.tar.gz
source /path/to/upmem-sdk/upmem_env.sh
```
- Clone this repo and cd into the `polynom-eval` folder.
```
git clone https://github.com/Yuhala/pim-benchmarks.git && cd pim-benchmarks/polynom-eval
```
- Setup benchmark results folders by the running the associated scripts.
```
./create_results_folders.sh
```
- Build the benchmarks by running `make`. This builds two executables: `dpu-poly-bench` and `cpu-poly-bench`.
- To run a specific benchmark, e.g., pointwise addition, do:
```
./dpu-poly-bench <benchmark name> <num DPUs> <polynomial size>
```
- `benchmark name` can be: `addition`, `naive_multi` (i.e., convolutions/school-book multiplication), or `cw_multi` (coefficient-wise multiplication).
- `num DPUs` is the number of DPUs to be used. For emulated PIM, the value should be `1`. For real PIM hardware, always use a number of DPUs which is a power of 2, e.g., `32`, `64`, etc.
- `polynomial size` represents the size of the polynomial, i.e., number of coefficients in the polynomial. This value should be a power of 2, e.g., 1024 (2^10), etc.
- For example, the command below runs the polynomial addition benchmark for two polynomials of size 1024 on 32 DPUs. The input polynomials (p and q) are first generated on the host (coefficients modulo some value Q), are split and copied to the DPUs, and polynomial addition performed on each DPU's data partition. The results are then sent back to the host.
```
./dpu-poly-bench addition 32 1024
```
- Each benchmark is run `100` times with `20` warm-up runs.
- To run the corresponding CPU-based benchmarks, replace `dpu-poly-bench` with `cpu-poly-bench`. For example:
```
./cpu-poly-bench addition 32 1024
```

## Analysing and plotting the results
- TODO
## Advanced configurations
- TODO


## Disclaimer
- Some parts of the code are a bit messy and can/will be refactored eventually.


