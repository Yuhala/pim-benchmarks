#
# Get final results from raw data
#

import csv
import statistics
import os


powers = [10, 12, 14, 16, 18, 20]

def get_median(input_file):
    # read data from CSV file
    data = []
    with open(input_file, 'r') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            try:
                data.append(float(row[0]))  # Assuming the column contains numerical values
            except ValueError:
                pass  # Ignore non-numeric values
    # Calculate the median
    median = statistics.median(data)
    return median

def write_cpu_results(bench_name):
    base_path = f"./cpu/{bench_name}"
    final_results_path = f"./cpu/cpu_{bench_name}.csv"
    
    # Check if file exists
    file_exists = os.path.exists(final_results_path)
    
    # Open CSV file in write mode
    with open(final_results_path, 'a+', newline='') as csvfile:
        writer = csv.writer(csvfile, delimiter=",")
        # write column header only if file does not exist
        if not file_exists:
            writer.writerow(['power_n', 'cpu_time(ms)'])
    
        for n in powers:
            file_path = f"{base_path}/cpu_time_{n}.csv"
            median = get_median(file_path)
            writer.writerow([n, median])
            
        
def write_dpu_results(bench_name):
    base_path = f"./dpu/{bench_name}"
    final_results_path = f"./dpu/dpu_{bench_name}.csv"
    
    # Check if file exists
    file_exists = os.path.exists(final_results_path)
    
    # Open CSV file in write mode
    with open(final_results_path, 'a+', newline='') as csvfile:
        writer = csv.writer(csvfile, delimiter=",")
        # write column header only if file does not exist
        if not file_exists:
            writer.writerow(['power_n', 'cpu_dpu_copy_time(ms)','dpu_time(ms)','dpu_cpu_copy_time(ms)'])
    
        for n in powers:
            cpu_dpu_file = f"{base_path}/cpu_dpu_copy_{n}.csv"
            exec_time_file = f"{base_path}/dpu_time_{n}.csv"
            dpu_cpu_file = f"{base_path}/dpu_cpu_copy_{n}.csv"             
             
             
            cpu_dpu_copy_median = get_median(cpu_dpu_file)
            exec_time_median = get_median(exec_time_file)
            dpu_cpu_copy_median = get_median(dpu_cpu_file)
            
            writer.writerow([n, cpu_dpu_copy_median, exec_time_median, dpu_cpu_copy_median])



#write_cpu_results("addition")
#write_cpu_results("coeffwise_multi")
#write_cpu_results("naive_multi")


write_dpu_results("addition")
write_dpu_results("coeffwise_multi")
write_dpu_results("naive_multi")