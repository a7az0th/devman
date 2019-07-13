#pragma once

#include "cuew.h"

#include <assert.h>
#include <string>
#include <string.h> //GCC looks for memcpy here
#include <vector>
#include <map>

namespace a7az0th {

struct Device;
struct ThreadData;
// A structure managing host-to-device buffer transfers
// Buffers are just a means to transfer data back and forth device and host
// The buffer is in no way responsible for managing what goes where.
// The programmer is responsible for managing device contexts and setting the right
// context on the CUDA stack prior to every buffer invocation
struct DeviceBuffer {

public:
	friend struct Device;
	friend struct ThreadData;

	DeviceBuffer(std::string& name=std::string("unnamed"), int emulate=0): 
		name(name), 
		emulate(emulate), 
		buffer(NULL), 
		size(0) 
	{
		//blank
	}
	~DeviceBuffer() { free(); }

	// Allocate a device buffer of given size
	int alloc(size_t size);
	// Free any resources owned
	int free();
	// Synchronously upload a given host buffer to the device
	int upload(void* host, size_t size);

	// Upload a given host buffer to the device ASYNCHRONOUSLY
	int uploadAsync(void* host, size_t size, CUstream stream);

	//Download a device buffer into the given host pointer. Pointer MUST point to a large enough buffer!
	int download(void* host);

	// Download buffer from the device ASYNCHRONOUSLY
	int downloadAsync(void* host, CUstream stream);
	
	// Returns the device pointer associated with this DeviceBuffer
	const void* get() const { return buffer; }
	
	// Returns the size of the buffer allocated on the device
	size_t getSize() const { return size; }
private:
	std::string name; // Name of this buffer.
	void* buffer; // Pointer to the buffer on the device
	size_t size; // Size of the buffer in bytes
	const int emulate; // True if the buffer is in emulation mode. aka it is allocated on the CPU
};


// A containter for CUDA device information
struct Device {
	friend struct DeviceManager;
	//Methods

	Device(int emulate = 0): 
		emulate(emulate),
		handle(-1),
		context(nullptr),
		program(nullptr)
	{
	}

	void freeMem() {
		CUresult res = CUDA_SUCCESS;
		if (program) {
			res = cuModuleUnload(program);
			assert(res == CUDA_SUCCESS);
			program = nullptr;
		}
		if (context) {
			res = cuCtxDestroy(context);
			context = nullptr;
		}
	}

	~Device() {
		freeMem();
	}

	std::string getInfo() const; // Produce a string out of the contained device info and return it

	CUresult setSource(const std::string& ptxFile);

	//Return true if the device is emulating a CUDA device
	int isEmulator() const { return emulate; }

	//Parameters
	struct Params {
		std::string name;         //< Name of the device as returned by the CUDA API
		size_t memory;            //< Total memory in bytes
		size_t sharedMemPerBlock; //< Total shared memory per block in bytes
		int ccmajor;              //< CUDA Compute Capability major version number
		int ccminor;              //< CUDA Compute Capability minor version number
		int warpSize;             //< Number of threads in a warp. Most probably 32
		int multiProcessorCount;  //< Number of SMs in the GPU
		int maxThreadsPerBlock;   //< Maximum number of threads that can work concurrently in a block
		int maxThreadsPerMP;      //< Maximum number of threads that can work concurrently in a SM
		int devId;                //< The index of this device
		int busId;                //< Index of the PCI bus on which the device is mounted
		int tccMode;              //< True if device is running in Tesla Compute Cluster mode
		int clockRate;            //< Device clock rate in MHz

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

	// Get the device context
	CUcontext getContext() const {
		return context;
	}

	CUmodule getProgram() const {
		return program;
	}

	void makeCurrent() const {
		CUresult err = CUDA_SUCCESS;
		CUcontext currentCtx = nullptr;
		err = cuCtxGetCurrent(&currentCtx);
		assert(err == CUDA_SUCCESS);

		if (currentCtx == context) {
			//The context is already current. Nothing to do
			return;
		}

		err = cuCtxSetCurrent(context);
		assert(err == CUDA_SUCCESS);
	}

private:
	//Set emulation mode for this device. Valid to be called only during initialization
	void setEmulation(int val) { emulate = val; }

	CUdevice handle;   //< Handle to the CUDA device
	CUcontext context; //< Handle to the CUDA context associated with this device
	CUmodule program;  //< Handle to the compiled program
	int emulate;
};

enum class DeviceError {
	Success,
	NoDevicesFound,
	PtxSourceNotFound,
	InvalidPtx,
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

// Wrapper for a device program
struct Kernel {
	friend struct ThreadData;
	// @param name The name of the program entry point
	// @param program The device module handle that was obtained from compiling the GPU code
	Kernel(std::string name, CUmodule program);
	~Kernel();

	// Add a pointer parameter to the kernel execution
	void addParamPtr(const void* ptr);
	// Add an integer parameter to the kernel execution
	void addParamInt(int i);

	// Get a handle to the kernel function. Passed to cuLaunchKernel
	CUfunction handle() { return function; }
private:
	CUfunction function;  // Handle to the kernel function.

	int offset; // Current location in the pool array
	char pool[4096]; // Memory pool for storing the kernel arguments

	int numParams; // Number of kernel params
	void* params[1024]; //The actual pointer array of arguments passed on to the kernel
};

// Represents a launch thread
// used to launch kernels on the device asynchronously
struct ThreadData {

	ThreadData(Device& device);
	~ThreadData();

	// Launched a kernel on the device with the given work size
	// @param kernel The kernel to launch
	// @param workSize The size of the job (how many threads to launch)
	CUresult launch(const Kernel& kernel, const int workSize);
	// Wait for the kernel launch to finish
	void wait() const;

	// Frees all resources owned. Safe to be called multile times
	void freeMem();

	CUstream getStream() const;
private:
	Device &device;  // Reference to the device on which we launch
	CUstream stream; // The cuda stream used for async lauches
};


} //namespace a7az0th
