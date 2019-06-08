#include "cuda.h"
#include "utils.h"


extern "C"
KERNEL void dummyGlobal(float *C, float *A, float *B) {
    
	const int idx = getGlobalID(0);

	const int x = idx % WIDTH;
	const int y = idx / WIDTH;

    float sum;

	for (int i = 0; i < 3*10000000; i++) {
		sum = 0.0f;
		for (int k=0; k<WIDTH; k++) {
		    sum += A[y*WIDTH+k] * B[k*WIDTH+x];
		}
	}

    C[y*WIDTH+x] = sum;
}


extern "C"
KERNEL void dummyShared(float *C, float *A, float *B) {
    
	const int idx = getGlobalID(0);

	const int x = idx % WIDTH;
	const int y = idx / WIDTH;

	__shared__ float localA[WIDTH][WIDTH];
	__shared__ float localB[WIDTH][WIDTH];

	localA[x][y] = A[y*WIDTH+x];
	localB[x][y] = B[y*WIDTH+x];
	__syncthreads();

    float sum;

	for (int i = 0; i < 3*10000000; i++) {
		sum = 0.0f;
		for (int k=0; k<WIDTH; k++) {
		    sum += localA[y][k] * localB[k][x];
		}
	}

    C[y*WIDTH+x] = sum;
}
