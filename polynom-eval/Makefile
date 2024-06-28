CC        = gcc -O3 --std=c99 #--std=c++11
LD         = gcc --std=c99 
DEFINES    =
INCS       = -I/opt/local/include
LIBS       = -L/opt/local/lib
CCFLAGS   = -O -fpic $(DEFINES) -w -I. $(INCS)
LDFLAGS    = -O $(LIBS)
#CXXFLAGS   = -g -fpic $(DEFINES) -w -I. $(INCS)
#LDFLAGS    = -g $(LIBS)
LDLIBFLAGS = -shared

# DPU TOOLCHAIN

DPU_PATHS = `dpu-pkg-config --cflags --libs dpu` -g


# Update to reflect yours
# DPU_FLAGS = -I/home/ubuntu/upmem-sdk/include/dpu -L/home/ubuntu/upmem-sdk/lib -ldpu
UPMEM_CLANG := dpu-upmem-dpurte-clang
TASKLET_FLAGS := -DNR_TASKLETS=16 -DSTACK_SIZE_DEFAULT=256 -DSTACK_SIZE_TASKLET_1=2048 -O2


# RECIPES

SRCS = bench.c polymulti.c polyadd.c 
OBJS = $(SRCS:.c=.o)
BINS = dpu-poly-bench main.dpu

all: $(BINS)

%.o: %.c
	$(CC) $(CCFLAGS) $(DPU_PATHS) -c $< -o $@ 

dpu-poly-bench: $(OBJS)
	$(LD) $(LDFLAGS) -pthread $^ -o $@  $(DPU_PATHS)  

depend:
	makedepend -Y -I. $(SRCS)


main.dpu: dpu/dpu-main.c dpu/dpu-fft.c dpu/dpu-poly.c 
	@$(UPMEM_CLANG) $(TASKLET_FLAGS) -o $@ $^


clean:
	rm -f $(BINS) *.o *.dpu

# DO NOT DELETE

bench.o: bench.h poly.h common.h
fft.o: poly.h common.h
polyadd.o: poly.h common.h 


