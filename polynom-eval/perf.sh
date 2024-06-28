#!/bin/bash


#perf stat -e instructions,cycles,mem_inst_retired.all_loads,mem_load_retired.l1_hit,mem_load_retired.l1_miss ./bench

#perf stat --all-user --no-big-num -e task-clock,page-faults,instructions,cycles,mem_inst_retired.all_loads,mem_inst_retired.all_stores,mem_load_retired.l1_hit,mem_load_retired.l1_miss,mem_load_retired.fb_hit 

#perf stat --all-user --no-big-num -e task-clock,page-faults,instructions,cycles,mem_inst_retired.all_loads,mem_inst_retired.all_stores,mem_load_retired.l1_hit,mem_load_retired.l1_miss,mem_load_retired.fb_hit ./bench 

perf stat -e cache-references,cache-misses ./bench

#perf record -e cache-misses ./cbr --verbose -c subs/s100k pubs/p50k
#perf report -vf 