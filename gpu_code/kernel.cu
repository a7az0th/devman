#include "cuda.h"
#include "../utils.h"


#ifdef __CUDACC__

extern "C"
KERNEL void fillmatrix(int* m0, int* m1, int* res, int size) {
	const int x = threadIdx.x % size;
	const int y = threadIdx.x / size;

	Matrix<int> mm0;
	mm0.init(m0, size, size);

	Matrix<int> mm1;
	mm1.init(m1, size, size);

	Matrix<int> r;
	r.init(res, size, size);


	int& result = r.getRow(y)[x];
	for (int p = 0; p < 10000; p++) {
		result = 0;
		for (int i=0; i < size; i++) {
			result += mm0.getRow(y)[i] * mm1.getRow(i)[x];
		}
	}
}


extern "C"
KERNEL void dummy() {
}

#endif