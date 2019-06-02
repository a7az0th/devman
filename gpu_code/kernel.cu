#include "cuda.h"
#include "utils.h"


extern "C"
KERNEL void dummy(int* res) {
	if (getGlobalID(0) == 0) {
		res[0] = 1337;
	}
}
