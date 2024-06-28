#ifndef POLY_H_
#define POLY_H_


void poly_add(int *p, int *q, int *res, int n, double *thread_time);
//void poly_multi_naive(int *p, int *q, int *res, int n);
void poly_multi_coeffwise(int *p, int *q, int *res, int n, double *thread_time);
void poly_multi_naive(int *p, int *q, int *prod, int n, double *thread_time);

void poly_multi_ntt(int *p, int *q, int *res, int n, double* thread_time);

#endif // POLY_H_