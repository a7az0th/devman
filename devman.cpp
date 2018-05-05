#include "devman.h"

#include <assert.h>

using namespace a7az0th;

typedef CUresult GPUResult;
#define GPU_SUCCESS (CUDA_SUCCESS);

// Method is called when a cuda error is encountered to print information
// error code, in what file, what line and which funtion was executed at the time of the error
void printMessage(int error, const char* file, int line, const char* func) {
	printf("CUDA Error %d encountered at %s[%d] in function %s\n", error, file, line, func);
}

#define checkError(err)                                       \
if ((err) != CUDA_SUCCESS) {                                   \
	printMessage(int(err), __FILE__, __LINE__, __FUNCTION__); \
	return (err);                                             \
}

int pushContext(int emulate, CUcontext& ctx, CUresult& err) {
	err = CUDA_SUCCESS;

	if (emulate) return false;

	CUcontext current = nullptr;
	err = cuCtxGetCurrent(&current);
	if (err != CUDA_SUCCESS) {
		return false;
	}

	if (current == ctx) {
		return false;
	} else {
		err = cuCtxPushCurrent(ctx);
		if (err != CUDA_SUCCESS) {
			return false;
		} else {
			return true;
		}
	}
}

void popContext(int needPop) {
	if (needPop) {
		cuCtxPopCurrent(NULL);
	}
}

struct PushContextRAII {
public:
	PushContextRAII(int modeCPU, CUcontext& ctx, CUresult& err) { needPop = pushContext(modeCPU, ctx, err); }
	~PushContextRAII() { popContext(needPop); }
private:
	int needPop;
};

DeviceManager::DeviceManager() : initialized(0), numDevices(0) {}
DeviceManager::~DeviceManager() { deinit(); }

int DeviceManager::init(int emulation) {

	GPUResult err = GPU_SUCCESS;
	//Nothing to do if already initialized.
	if (initialized) return err;

	if (cuewInit(CUEW_INIT_CUDA) != CUEW_SUCCESS) {
		printf("CUDA could not be initialized! Setting up the CPU as a CUDA emulation device!\n");
		numDevices = 1;
		devices.resize(numDevices);
		const int devIdx = 0;
		Device& devInfo = devices[devIdx];
		devInfo.setEmulation(1);
		devInfo.params.devId = devIdx;
		devInfo.params.name = std::string("Emulator");
	} else {

		err = cuInit(0);
		checkError(err);

		err = cuDeviceGetCount(&numDevices);
		checkError(err);

		devices.resize(numDevices);
		for (int i = 0; i < numDevices; i++) {
			Device& devInfo = devices[i];
			devInfo.setEmulation(emulation);
			int err = getDeviceInfo(i, devInfo);
			checkError(err);
		}
	}

	initialized = 1;

	return err;
}

Device& DeviceManager::getDevice(int index) {
	return devices[index];
}

int DeviceManager::deinit() {
	initialized = 0;
	numDevices = 0;
	GPUResult err = GPU_SUCCESS;
	//err = cudaDeviceReset();
	checkError(err);
	return err;
}

int DeviceManager::getDeviceInfo(int deviceIndex, Device &devInfo) {

	GPUResult err = GPU_SUCCESS;

	devInfo.params.devId = deviceIndex;
	CUdevice device = 0;
	err = cuDeviceGet(&device, deviceIndex);
	checkError(err);
	devInfo.handle = device;

	char name[256];
	err = cuDeviceGetName(name, 256, device);
	checkError(err);
	devInfo.params.name = std::string(name);

	CUdevprop prop;
	err = cuDeviceGetProperties(&prop, device);
	checkError(err);

	devInfo.params.maxThreadsPerBlock = prop.maxThreadsPerBlock;
	devInfo.params.sharedMemPerBlock = prop.sharedMemPerBlock;
	devInfo.params.warpSize = prop.SIMDWidth;
	devInfo.params.clockRate = prop.clockRate;

	int value;
	CUdevice_attribute attrib;

	attrib = CUdevice_attribute::CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR;
	err = cuDeviceGetAttribute(&value, attrib, device);
	checkError(err);
	devInfo.params.ccmajor = value;

	attrib = CUdevice_attribute::CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR;
	err = cuDeviceGetAttribute(&value, attrib, device);
	checkError(err);
	devInfo.params.ccminor = value;

	attrib = CUdevice_attribute::CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR;
	err = cuDeviceGetAttribute(&value, attrib, device);
	checkError(err);
	devInfo.params.maxThreadsPerMP = value;

	attrib = CUdevice_attribute::CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT;
	err = cuDeviceGetAttribute(&value, attrib, device);
	checkError(err);
	devInfo.params.multiProcessorCount = value;

	attrib = CUdevice_attribute::CU_DEVICE_ATTRIBUTE_PCI_BUS_ID;
	err = cuDeviceGetAttribute(&value, attrib, device);
	checkError(err);
	devInfo.params.busId = value;

	attrib = CUdevice_attribute::CU_DEVICE_ATTRIBUTE_TCC_DRIVER;
	err = cuDeviceGetAttribute(&value, attrib, device);
	checkError(err);
	devInfo.params.tccMode = value;

	size_t bytes;
	err = cuDeviceTotalMem(&bytes, device);
	checkError(err);
	devInfo.params.memory = bytes;

	CUcontext ctx = nullptr;
	int flags = 0;
	err = cuCtxCreate(&ctx, flags, device);
	checkError(err);
	devInfo.context = ctx;

	return err != GPU_SUCCESS;
}

int DeviceManager::initDevices(std::string& sourceFile) {
	assert(numDevices > 0);
	if (numDevices <= 0) {
		return 1; //NoDevicesFound;
	}
	
	std::string ptx = getFileContents(sourceFile);
	if (ptx[0] == '\0') {
		return 2;//PtxSourceNotFound;
	}
	for (int i = 0; i < numDevices; i++) {
		Device& device = getDevice(i);
		std::string stats = device.getInfo();
		printf("%s\n", stats.c_str());

		CUresult err = device.setSource(ptx);
		if (err != CUDA_SUCCESS) {
			return 3;//InvalidPtx;
		}
	}
	return 0;//Success;
}

std::vector<ThreadData> DeviceManager::initThreadData(int numThreads) {
	std::vector<ThreadData> res(numThreads);
	std::vector<int> threadMap(numThreads);
	for (int i=0; i < numThreads; i++) {
		CUresult err = CUDA_SUCCESS;
		ThreadData& td = res[i];
	
		td.device = &getDevice(i % numDevices);
		err = cuCtxPushCurrent(td.device->context);
		if (err != CUDA_SUCCESS) { return std::vector<ThreadData>(); }//Failure; }
		err = cuStreamCreate(&td.stream, 0);//CU_STREAM_NON_BLOCKING
		if (err != CUDA_SUCCESS) { return std::vector<ThreadData>(); }//Failure; }
		err = cuCtxPopCurrent(NULL);
		if (err != CUDA_SUCCESS) { return std::vector<ThreadData>(); }//Failure; }
	}

	return res;
}

int DeviceBuffer::free() {
	GPUResult err = GPU_SUCCESS;

	PushContextRAII push(emulate, context, err);
	checkError(err);
	
	if (buffer) {
		if (emulate) {
			delete [] static_cast<char*>(buffer);
		} else {
			err = cuMemFree((CUdeviceptr)buffer);
		}
		buffer = NULL;
		size = 0;
	}
	return err != GPU_SUCCESS;
}

int DeviceBuffer::alloc(size_t size) {
	GPUResult err = GPU_SUCCESS;

	PushContextRAII push(emulate, context, err);
	checkError(err);

	if (buffer) {
		free();
	}

	if (emulate) {
		buffer = new char[size];
	} else {
		err = cuMemAlloc((CUdeviceptr*)&buffer, size);
	}
	this->size = size;
	return err != GPU_SUCCESS;
}

int DeviceBuffer::upload(void* host, size_t size, CUstream stream) {
	if (!buffer) return CUDA_ERROR_NOT_INITIALIZED;
	if (size > this->size) return CUDA_ERROR_OUT_OF_MEMORY;

	assert(buffer != nullptr);
	
	GPUResult err = GPU_SUCCESS;
	PushContextRAII push(emulate, context, err);
	checkError(err);

	if (emulate) {
		memcpy(buffer, host, size);
	} else {
		err = cuMemcpyHtoDAsync((CUdeviceptr)buffer, host, size, stream);
	}
	return err != GPU_SUCCESS;
}

int DeviceBuffer::download(void* host, CUstream stream) {
	assert(host != nullptr);
	GPUResult err = GPU_SUCCESS;

	PushContextRAII push(emulate, context, err);
	checkError(err);

	if (emulate) {
		memcpy(host, buffer, size);
	} else {
		err = cuMemcpyDtoHAsync(host, (CUdeviceptr)buffer, size, stream);
		checkError(err);
	}
	return err != GPU_SUCCESS;
}

std::string Device::getInfo() const {
	std::string message = "";

	const float totalMemory = float(params.memory) / float(1024 * 1024 * 1024);
	const float sharedMemory = float(params.sharedMemPerBlock) / float(1024);

	std::string driverMode = "";
	switch(params.tccMode) {
		case 0: driverMode = "WDDM"; break;
		case 1: driverMode = "TCC"; break;
		default:
			driverMode = "Unknown"; break;
	}

	const int buffSize = 1024;
	char buff[buffSize];
	snprintf(buff, buffSize, "Device[%d] is : %s\n\n", params.devId, params.name.c_str()); message += buff;
	snprintf(buff, buffSize, "\tDriver mode                    : %s\n", driverMode.c_str());  message += buff;
	snprintf(buff, buffSize, "\tClock Rate                     : %dMhz\n", params.clockRate/1000);   message += buff;
	snprintf(buff, buffSize, "\tTotal global memory            : %.1f GB\n", totalMemory);    message += buff;
	snprintf(buff, buffSize, "\tShared memory                  : %.1f KB\n", sharedMemory);   message += buff;
	snprintf(buff, buffSize, "\tCompute Capability             : %d.%d\n", params.ccmajor, params.ccminor);     message += buff;
	snprintf(buff, buffSize, "\tWarp size                      : %d\n", params.warpSize);            message += buff;
	snprintf(buff, buffSize, "\tMax threads per block          : %d\n", params.maxThreadsPerBlock);  message += buff;
	snprintf(buff, buffSize, "\tMax threads per multiprocessor : %d\n", params.maxThreadsPerMP);     message += buff;
	snprintf(buff, buffSize, "\tNumber of multiprocessors      : %d\n", params.multiProcessorCount); message += buff;

	message += "\n";
	return message;
}

GPUResult Device::setSource(const std::string& ptxSource) {
	assert(context != nullptr);

	const int bufferSize = (1024 * 50);
	char logBuffer[bufferSize];
	char errorBuffer[bufferSize];

	const int optimizationLevel = 4;

	int numOptions = 0;
	CUjit_option options[20];
	void* optionValues[20];

	options[numOptions] = CU_JIT_OPTIMIZATION_LEVEL;
	optionValues[numOptions] = (void*)optimizationLevel;
	numOptions++;

	// set up size of compilation log buffer
	options[numOptions] = CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES;
	optionValues[numOptions] = (void *)bufferSize;
	numOptions++;

	// set up pointer to the compilation log buffer
	options[numOptions] = CU_JIT_INFO_LOG_BUFFER;
	optionValues[numOptions] = logBuffer;
	numOptions++;

	// set up size of compilation error buffer
	options[numOptions] = CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES;
	optionValues[numOptions] = (void *)bufferSize;
	numOptions++;

	// set up pointer to the compilation error buffer
	options[numOptions] = CU_JIT_ERROR_LOG_BUFFER;
	optionValues[numOptions] = errorBuffer;
	numOptions++;

	CUresult err = CUDA_SUCCESS;
	PushContextRAII push(emulate, context, err);
	checkError(err);

	err = cuModuleLoadDataEx(&program, ptxSource.c_str(), numOptions, options, optionValues);
	checkError(err);

	return GPU_SUCCESS;
}

CUresult Device::launch(const Kernel& ker, CUstream stream) {
	CUresult err = CUDA_SUCCESS;

	PushContextRAII push(emulate, context, err);
	checkError(err);

	if (emulate) {
		//
	} else {
		err = cuLaunchKernel(ker.function,
			ker.numBlocks, 1, 1,
			ker.threadsPerBlock, 1, 1,
			ker.sharedMem,
			stream,
			(void**)ker.params,
			nullptr); //extra
		checkError(err);
	}
	return err;
}

Kernel::Kernel(std::string& name, CUmodule program, int numBlocks, int threadsPerBlock, int sharedMem) : 
	name(name), function(nullptr), offset(0), numParams(0), threadsPerBlock(threadsPerBlock), numBlocks(numBlocks), sharedMem(sharedMem) {

	CUresult err = CUDA_SUCCESS;
	err = cuModuleGetFunction(&function, program, name.c_str());
	assert(err == CUDA_SUCCESS);
}

Kernel::~Kernel() {
	//blank
}

void Kernel::addParamPtr(const void* ptr) {
	
	const int size = sizeof(void*);
	void* dest = (void*)(&pool[offset]);
	memcpy(dest, &ptr, size);

	params[numParams] = dest;
	numParams++;
	offset += size;
}

void Kernel::addParamInt(int i) {

	const int size = sizeof(int);
	void* dest = (void*)(&pool[offset]);
	memcpy(dest, &i, size);

	params[numParams] = dest;
	numParams++;
	offset += size;
}


std::string a7az0th::getFileContents(const std::string& file) {
	std::string res = "";
	FILE* fp = fopen(file.c_str(), "rb");
	if (!fp) {
		return res;
	}

	fseek(fp, 0, SEEK_END);
	unsigned long len = (unsigned long)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* buffer = new char[len+1];
	fread(buffer, sizeof(char), len-1, fp);
	buffer[len] = '\0';
	fclose(fp);

	res = buffer;
	delete[] buffer;

	return res;
}
