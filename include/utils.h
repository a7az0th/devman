#pragma once

#ifdef __CUDACC__
#define CONSTRUCTOR __device__
#define DESTRUCTOR __device__
#define TEMPLATE __device__
#define KERNEL __global__

#define _void __device__ void
#define _int __device__ int
#define _float __device__ float

#define getGlobalID(X) (blockIdx.x * blockDim.x + threadIdx.x)

#define GPU_ASSERT(X) 

#else

#include "assert.h"

#define CONSTRUCTOR
#define DESTRUCTOR
#define TEMPLATE
#define KERNEL 

#define _void void
#define _int int
#define _float float

#define getGlobalID(X) (X.globalId)

#define GPU_ASSERT(X) assert(X)

#endif 

template<class T>
struct Buffer {
	CONSTRUCTOR Buffer() : buff(nullptr), size(0) {}
	CONSTRUCTOR Buffer(T* ptr, int size) : buff(ptr), size(size) {}
	DESTRUCTOR ~Buffer() {} //We dont own the memory so do nothing

	TEMPLATE T* ptr() const {
		return buff;
	}

	_int count() const {
		return size;
	}

	_void init(T* buff, int size) {
		this->buff = buff;
		this->size = size;
	}
private:
	T* buff;
	int size;
};

template<class T>
struct Matrix {
	CONSTRUCTOR Matrix(): buff(nullptr), cols(0), rows(0) {}
	CONSTRUCTOR Matrix(T* buff, int rows, int cols) : buff(buff), rows(rows), cols(cols) {}
	_void init(T* buff, int rows, int cols) {
		this->buff = buff;
		this->rows = rows;
		this->cols = cols;
	}

	TEMPLATE T* getRow(int index) {
		return &(buff[index*cols]);
	}

	_int numCols() const { return cols; }
	_int numRows() const { return rows; }
private:
	int cols;
	int rows;

	T *buff;
};

#define WIDTH 32
