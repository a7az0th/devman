#include "cuda.h"
#include "utils.h"


extern "C"
KERNEL void dummy(float *C, float *A, float *B) {
    
	const int idx = getGlobalID(0);

	const int x = idx % WIDTH;
	const int y = idx / WIDTH;

    float sum = 0.0;

    for (int k=0; k<WIDTH; k++) {
        sum += A[y*WIDTH+k] * B[k*WIDTH+x];
    }

    C[y*WIDTH+x] = sum;
}
