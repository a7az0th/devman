#pragma once

#include <cuda.h>
#include <cstring>
#include <vector>
#include <map>

namespace a7az0th {

struct Device;
struct ThreadData;
// A structure managing host-to-device buffer transfers
struct DeviceBuffer {

public:
	friend struct Device;
	friend struct ThreadData;

	~DeviceBuffer() { free(); }

	// Allocate a device buffer of given size
	int alloc(size_t size);
	// Free any resources owned
	int free();
	// Upload a given host buffer to the device
	int upload(void* host, size_t size, CUstream stream = nullptr);
	//Download a device buffer into the given host pointer. Pointer MUST point to a large enough buffer!
	int download(void* host, CUstream stream = nullptr);
	// Returns the device pointer associated with this DeviceBuffer
	const void* get() const { return buffer; }
	// Returns the size of the buffer allocated on the device
	size_t getSize() const { return size; }
private:
	DeviceBuffer(std::string& name, CUcontext context, int emulate=0) : name(name), context(context), emulate(emulate), buffer(NULL), size(0) {}
	
	std::string name;
	void* buffer;
	size_t size;
	const int emulate;
	CUcontext context;
};




struct Kernel;
// A containter for CUDA device information
struct Device {
	//Methods

	Device(int emulate = 0): 
		emulate(emulate),
		handle(-1),
		context(nullptr),
		program(nullptr)
	{
	}

	~Device() {
		CUresult res = CUDA_SUCCESS;
		if (program) { res = cuModuleUnload(program); }
		if (context) { res = cuCtxDestroy(context); }
	}

	std::string getInfo() const; //< Produces a string out of the contained device info and returns it

	CUresult setSource(const std::string& ptxSource);
	CUresult launch(const Kernel& kernel, CUstream stream = nullptr);

	//Returns true if the device is emulating a CUDA device
	int isEmulator() { return emulate; }
	void setEmulation(int val) { emulate = val; }

	DeviceBuffer* getBuffer(std::string name) {
		DeviceBuffer* res = nullptr;

		const auto& it = buffers.find(name);
		if (it == buffers.end()) {
			res = new DeviceBuffer(name, context, emulate);
			buffers[name] = res;
		} else {
			res = it->second;
		}
		return res;
	}

	//Parameters
	struct Params {
		std::string name;                  //< Name of the device as returned by the CUDA API
		size_t memory;                     //< Total memory in bytes
		size_t sharedMemPerBlock;          //< Total shared memory per block in bytes
		int ccmajor;                       //< CUDA Compute Capability major version number
		int ccminor;                       //< CUDA Compute Capability minor version number
		int warpSize;                      //< Number of threads in a warp. Most probably 32
		int multiProcessorCount;           //< Number of SMs in the GPU
		int maxThreadsPerBlock;            //< Maximum number of threads that can work concurrently in a block
		int maxThreadsPerMP;               //< Maximum number of threads that can work concurrently in a SM
		int devId;                         //< The index of this device
		int busId;                         //< Index of the PCI bus on which the device is mounted
		int tccMode;                       //< True if device is running in Tesla Compute Cluster mode
		int clockRate;                     //< Device clock rate in MHz

		Params() :
			name("Unknown"),
			memory(0),
			sharedMemPerBlock(0),
			ccmajor(-1),
			ccminor(-1),
			warpSize(-1),
			multiProcessorCount(-1),
			maxThreadsPerBlock(-1),
			maxThreadsPerMP(-1),
			devId(-1),
			busId(-1),
			tccMode(-1),
			clockRate(-1)
		{}
	} params;

	CUdevice handle;                   //< Handle to the CUDA device
	CUcontext context;                 //< Handle to the CUDA context associated with this device
	CUmodule program;                  //< Handle to the compiled program

private:
	std::map<std::string, DeviceBuffer*> buffers;
	int emulate;
};

// Main GPU management class. A singleton class. You cannot create it explicitly
// In order to obtain an instance of it you need to call the getInstance() method
// It is responsible for initializing, deinitializing the GPU devices.
// Can be asked to provide information for a particular device
// The process method would start the processing on the GPU
struct DeviceManager {

	// The only way to obtain an instance of an object is through this method
	static DeviceManager& getInstance(int emulation=0) {
		static DeviceManager instance;
		instance.init(emulation);
		return instance;
	}
	// Destructor. Should call deinit();
	~DeviceManager();

	// Return a const reference to the device with given index. Caller MUST ensure that the index is less than numDevices!
	Device& getDevice(int index);
	int getDeviceCount() const { return numDevices; }

	int initDevices(std::string& ptx);

	std::vector<ThreadData> initThreadData(int numThreads);
private:
	// Ask for information about a particular device
	// @param deviceIndex The index of the device that we are querying
	// @param devInfo A container where the device data will be populated
	// @returns 0 on success
	int getDeviceInfo(int deviceIndex, Device &devInfo);

	// Queries the system for GPU devices, sets the numDevices member accordingly, populates the devices list
	// @returns 0 on success 
	int init(int emulation);
	// Frees all resources held by the instance
	int deinit();

	// Check whether the GPU manager has successfully been initialized.
	// returns 1 if the init() method has already been called
	int isInitialized() const { return initialized; }

	int initialized;                 //< A flag to tell us whether the class methods are safe to be called.
	int numDevices;                  //< The number of devices in the system. Populated by init()
	std::vector<Device> devices;     //< An array containing per-device information

	DeviceManager();                               //< Private constructor. We don't want anyone to create objects of this type.
	DeviceManager(DeviceManager const&) = delete;  //< Remove copy constructor. We don't want anyone to copy objects of this type.
	void operator=(DeviceManager const&) = delete; //< Remove operator=. We don't want anyone to copy objects of this type.
};

struct Kernel {
	Kernel(std::string& name, CUmodule program, int numBlocks, int threadsPerBlock, int sharedMem);
	~Kernel();

	void addParamPtr(const void* ptr);
	void addParamInt(int i);

	const std::string name;

	CUfunction handle() { return function; }
	friend struct Device;
private:
	int numBlocks;
	int threadsPerBlock;
	int sharedMem;
	int offset;
	char pool[4096];
	CUfunction function;

	int numParams;
	void* params[1024];
};

struct ThreadData {

	Device* device;
	CUstream stream;

	ThreadData(): device(nullptr), stream(nullptr)
	{}
	~ThreadData() {
		freeMem();
		device = nullptr;
	}
	void freeMem() {
		//blank
	}

	DeviceBuffer* getBuffer(std::string name) {
		DeviceBuffer* res = nullptr;

		const auto& it = buffers.find(name);
		if (it == buffers.end()) {
			res = new DeviceBuffer(name, device->context, device->isEmulator());
			buffers[name] = res;
		} else {
			res = it->second;
		}
		return res;
	}
private:
	std::map<std::string, DeviceBuffer*> buffers;
};

// Read the contents of the file given and return them as string.
// @return Empty string on failure and the file contents on success
std::string getFileContents(const std::string& file);

} //namespace a7az0th
