#ifndef DPU_POLY_H_
#define DPU_POLY_H_

//void dpu_poly_add(int32_t *p, int32_t *q, int32_t *res, uint32_t task_size);
void dpu_poly_add(int *p, int *q, int *res);
void dpu_poly_multi_coeffwise(int *p, int *q, int *res);

void dpu_poly_multi_naive(int *p, int *q, int *res);



void poly_multi_fft(int* p, int* q, int* res, int n);

void dpu_vector_sum(uint8_t task_size);

#endif // DPU_POLY_H_