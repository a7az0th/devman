#include "cuda.h"
#include "utils.h"


extern "C"
KERNEL void greyscale(float3 *buffIn, float3* buffOut) {
	const int i = threadIdx.x + blockIdx.x * blockDim.x;
	//for (int i = tid; i < n; i += blockDim.x * gridDim.x) {
	const float3 col = buffIn[i];
	const float R = col.x;
	const float G = col.y;
	const float B = col.z;
	const float luminance = 0.2126f*R + 0.7152f*G + 0.0722f*B;
	buffOut[i] = make_float3(luminance, luminance, luminance);
}
