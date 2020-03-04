#include "nvml.h"
#include "devman.h"
#include "progress.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define VC_EXTRALEAN
#  include <windows.h>

typedef HMODULE LibHandle;
#  define loadLib(path)         LoadLibraryA(path)
#  define unloadLib(lib)         FreeLibrary(lib)
#  define loadSymbol(lib, symbol)  GetProcAddress(lib, symbol)

#else

typedef void* LibHandle;
#  include <dlfcn.h>
#  define loadLib(path)         dlopen(path, RTLD_NOW)
#  define unloadLib(lib)         dlclose(lib)
#  define loadSymbol(lib, symbol)  dlsym(lib, symbol)
#endif

#define COUNT_OF(X) (sizeof(X)/sizeof(X[0]))

///  
/// Return values for NVML API calls.
/// Copied straight out of nvml.h
///
typedef enum nvmlReturn_enum 
{
    NVML_SUCCESS = 0,                   //!< The operation was successful
    NVML_ERROR_UNINITIALIZED = 1,       //!< NVML was not first initialized with nvmlInit()
    NVML_ERROR_INVALID_ARGUMENT = 2,    //!< A supplied argument is invalid
    NVML_ERROR_NOT_SUPPORTED = 3,       //!< The requested operation is not available on target device
    NVML_ERROR_NO_PERMISSION = 4,       //!< The current user does not have permission for operation
    NVML_ERROR_ALREADY_INITIALIZED = 5, //!< Deprecated: Multiple initializations are now allowed through ref counting
    NVML_ERROR_NOT_FOUND = 6,           //!< A query to find an object was unsuccessful
    NVML_ERROR_INSUFFICIENT_SIZE = 7,   //!< An input argument is not large enough
    NVML_ERROR_INSUFFICIENT_POWER = 8,  //!< A device's external power cables are not properly attached
    NVML_ERROR_DRIVER_NOT_LOADED = 9,   //!< NVIDIA driver is not loaded
    NVML_ERROR_TIMEOUT = 10,            //!< User provided timeout passed
    NVML_ERROR_IRQ_ISSUE = 11,          //!< NVIDIA Kernel detected an interrupt issue with a GPU
    NVML_ERROR_LIBRARY_NOT_FOUND = 12,  //!< NVML Shared Library couldn't be found or loaded
    NVML_ERROR_FUNCTION_NOT_FOUND = 13, //!< Local version of NVML doesn't implement this function
    NVML_ERROR_CORRUPTED_INFOROM = 14,  //!< infoROM is corrupted
    NVML_ERROR_GPU_IS_LOST = 15,        //!< The GPU has fallen off the bus or has otherwise become inaccessible
    NVML_ERROR_RESET_REQUIRED = 16,     //!< The GPU requires a reset before it can be used again
    NVML_ERROR_OPERATING_SYSTEM = 17,   //!< The GPU control device has been blocked by the operating system/cgroups
    NVML_ERROR_LIB_RM_VERSION_MISMATCH = 18,   //!< RM detects a driver/library version mismatch
    NVML_ERROR_IN_USE = 19,             //!< An operation cannot be performed because the GPU is currently in use
    NVML_ERROR_MEMORY = 20,             //!< Insufficient memory
    NVML_ERROR_NO_DATA = 21,            //!<No data
    NVML_ERROR_VGPU_ECC_NOT_SUPPORTED = 22,    //!< The requested vgpu operation is not available on target device, becasue ECC is enabled
    NVML_ERROR_UNKNOWN = 999            //!< An internal driver error occurred
} nvmlReturn_t;
/**
 * Buffer size guaranteed to be large enough for pci bus id
 */
#define NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE      32

/**
 * Buffer size guaranteed to be large enough for pci bus id for ::busIdLegacy
 */
#define NVML_DEVICE_PCI_BUS_ID_BUFFER_V2_SIZE   16

/**
 * PCI information about a GPU device.
 */
typedef struct nvmlPciInfo_st
{
    char busIdLegacy[NVML_DEVICE_PCI_BUS_ID_BUFFER_V2_SIZE]; //!< The legacy tuple domain:bus:device.function PCI identifier (&amp; NULL terminator)
    unsigned int domain;             //!< The PCI domain on which the device's bus resides, 0 to 0xffffffff
    unsigned int bus;                //!< The bus on which the device resides, 0 to 0xff
    unsigned int device;             //!< The device's id on the bus, 0 to 31
    unsigned int pciDeviceId;        //!< The combined 16-bit device id and 16-bit vendor id

    // Added in NVML 2.285 API
    unsigned int pciSubSystemId;     //!< The 32-bit Sub System Device ID

    char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE]; //!< The tuple domain:bus:device.function PCI identifier (&amp; NULL terminator)
} nvmlPciInfo_t;
/** 
 * Generic enable/disable enum. 
 */
typedef enum nvmlEnableState_enum 
{
    NVML_FEATURE_DISABLED    = 0,     //!< Feature disabled 
    NVML_FEATURE_ENABLED     = 1      //!< Feature enabled
} nvmlEnableState_t;
/**
 * Enum to represent NvLink queryable capabilities
 */
typedef enum nvmlNvLinkCapability_enum
{
    NVML_NVLINK_CAP_P2P_SUPPORTED = 0,     // P2P over NVLink is supported
    NVML_NVLINK_CAP_SYSMEM_ACCESS = 1,     // Access to system memory is supported
    NVML_NVLINK_CAP_P2P_ATOMICS   = 2,     // P2P atomics are supported
    NVML_NVLINK_CAP_SYSMEM_ATOMICS= 3,     // System memory atomics are supported
    NVML_NVLINK_CAP_SLI_BRIDGE    = 4,     // SLI is supported over this link
    NVML_NVLINK_CAP_VALID         = 5,     // Link is supported on this device
    // should be last
    NVML_NVLINK_CAP_COUNT
} nvmlNvLinkCapability_t;

/// Typedefs for the NVML functions that we need.
typedef nvmlReturn_t (*nvmlInit_t)(void);
typedef nvmlReturn_t (*nvmlSystemGetDriverVersion_t)(char *version, unsigned int length);
typedef nvmlReturn_t (*nvmlShutdown_t)(void);

static nvmlInit_t nvmlInitialize = nullptr;
static nvmlShutdown_t nvmlDeinitialize = nullptr;
static nvmlSystemGetDriverVersion_t nvmlGetDriverVersion = nullptr;

/// NVLink related:
typedef void* nvmlDevice_t;

#define LOAD_SYMBOL(X) X = reinterpret_cast<X##_t>(loadSymbol(hMod, #X))

typedef nvmlReturn_t (*nvmlDeviceGetHandleByIndex_t)(unsigned int index, nvmlDevice_t *device);
typedef nvmlReturn_t (*nvmlDeviceGetPciInfo_t)(nvmlDevice_t device, nvmlPciInfo_t *pci); 
typedef nvmlReturn_t (*nvmlDeviceGetHandleByUUID_t)(const char *uuid, nvmlDevice_t *device);
typedef nvmlReturn_t (*nvmlDeviceGetHandleByPciBusId_t)(const char *pciBusId, nvmlDevice_t *device);

typedef nvmlReturn_t (*nvmlDeviceGetNvLinkCapability_t)(nvmlDevice_t device, unsigned int link, nvmlNvLinkCapability_t capability, unsigned int *capResult);
typedef nvmlReturn_t (*nvmlDeviceGetNvLinkRemotePciInfo_t)(nvmlDevice_t device, unsigned int link, nvmlPciInfo_t *pci); 
typedef nvmlReturn_t (*nvmlDeviceGetNvLinkState_t)(nvmlDevice_t device, unsigned int link, nvmlEnableState_t *isActive);

static nvmlDeviceGetHandleByIndex_t nvmlDeviceGetHandleByIndex = nullptr;
static nvmlDeviceGetPciInfo_t nvmlDeviceGetPciInfo = nullptr;
static nvmlDeviceGetHandleByUUID_t nvmlDeviceGetHandleByUUID = nullptr;
static nvmlDeviceGetHandleByPciBusId_t nvmlDeviceGetHandleByPciBusId = nullptr;
static nvmlDeviceGetNvLinkCapability_t nvmlDeviceGetNvLinkCapability = nullptr;
static nvmlDeviceGetNvLinkRemotePciInfo_t nvmlDeviceGetNvLinkRemotePciInfo = nullptr;
static nvmlDeviceGetNvLinkState_t nvmlDeviceGetNvLinkState = nullptr;

static bool _nvmlInitialized = false;
/// Look for the NVML dll/so and load it dynamically. Return 0 on success
/// When successful, all symbols that we need would be available for usage
/// After a successful call we must call deinitNVML when we're done using it.
int initNVML(LibHandle &hMod) {
	static int errorCode = 0; // 0 means success;
	if (_nvmlInitialized) {
		return errorCode;
	}
	_nvmlInitialized = true;

	//List of all possible locations of NVML
#ifdef _WIN32
	const char* nvmlPaths[] = {
		"nvml.dll",  //Search in PATH
		"C:/Program Files/NVIDIA Corporation/NVSMI/nvml.dll", // This is where it lives for Standard drivers
		"C:/Windows/System32/nvml.dll" // This is where it lives for DCH drivers
	};
#else
	const char* nvmlPaths[] = {"libnvidia-ml.so"};
#endif
	const int count = COUNT_OF(nvmlPaths);

	for (int i=0; i < count; i++) {
		hMod = loadLib(nvmlPaths[i]);
		if (hMod) {
			break;
		}
	}
	if (hMod) {
		nvmlInitialize = reinterpret_cast<nvmlInit_t>(loadSymbol(hMod, "nvmlInit"));
		nvmlDeinitialize = reinterpret_cast<nvmlShutdown_t>(loadSymbol(hMod, "nvmlShutdown"));
		nvmlGetDriverVersion = reinterpret_cast<nvmlSystemGetDriverVersion_t>(loadSymbol(hMod, "nvmlSystemGetDriverVersion"));

		//NVLink related

		LOAD_SYMBOL(nvmlDeviceGetHandleByIndex);
		LOAD_SYMBOL(nvmlDeviceGetPciInfo);
		LOAD_SYMBOL(nvmlDeviceGetHandleByUUID);
		LOAD_SYMBOL(nvmlDeviceGetHandleByPciBusId);
		LOAD_SYMBOL(nvmlDeviceGetNvLinkCapability);
		LOAD_SYMBOL(nvmlDeviceGetNvLinkRemotePciInfo);
		LOAD_SYMBOL(nvmlDeviceGetNvLinkState);
	}

	if (nullptr == nvmlInitialize ||
		nullptr == nvmlDeinitialize ||
		nullptr == nvmlGetDriverVersion ||
		nullptr == nvmlDeviceGetHandleByIndex ||
		nullptr == nvmlDeviceGetPciInfo ||
		nullptr == nvmlDeviceGetHandleByUUID ||
		nullptr == nvmlDeviceGetHandleByPciBusId ||
		nullptr == nvmlDeviceGetNvLinkCapability ||
		nullptr == nvmlDeviceGetNvLinkRemotePciInfo ||
		nullptr == nvmlDeviceGetNvLinkState
	) {
		errorCode = -1;
	}

	return errorCode;
}

/// Deinitialize NVML
int deinitNVML(LibHandle &hMod) {
	const bool res = unloadLib(hMod);
	hMod = nullptr;

	nvmlInitialize = nullptr;
	nvmlDeinitialize = nullptr;
	nvmlGetDriverVersion = nullptr;
	//NVLink
	nvmlDeviceGetHandleByIndex = nullptr;
	nvmlDeviceGetPciInfo = nullptr;
	nvmlDeviceGetHandleByUUID = nullptr;
	nvmlDeviceGetHandleByPciBusId = nullptr;
	nvmlDeviceGetNvLinkCapability = nullptr;
	nvmlDeviceGetNvLinkRemotePciInfo = nullptr;
	nvmlDeviceGetNvLinkState = nullptr;

	_nvmlInitialized = false;
	return res;
}

ErrorCode getDriverVersionWithNVML(std::string &driverVersion) {
	ErrorCode error;
	driverVersion = "(unknown)";
	LibHandle hMod = nullptr;
	const int res = initNVML(hMod);
	if (res) {
		return ErrorCode(nullptr, -1, "Failed to load NVML");
	}

	nvmlReturn_t err;
	err = nvmlInitialize();

	if(err == NVML_SUCCESS) {
		char version[256];
		err = nvmlGetDriverVersion(version, 256);
		if (err == NVML_SUCCESS) {
			driverVersion = std::string(version);
		}
		nvmlDeinitialize();
	} else if (err == NVML_ERROR_DRIVER_NOT_LOADED) {
		error = ErrorCode(nullptr, err, "NVidia driver is not running. Initialization failed.");
	} else if(err == NVML_ERROR_NO_PERMISSION) {
		error = ErrorCode(nullptr, err, "NVML does not have permission to talk to the driver.");
	} else {
		error = ErrorCode(nullptr, err, "NVML encountered an unexpected error during initialization.");
	}
	
	deinitNVML(hMod);
	return error;
}


nvmlDevice_t getNvmlDeviceByUUID(char* uuid) {
	nvmlDevice_t device = nullptr;
	nvmlReturn_t result = nvmlDeviceGetHandleByUUID(uuid, &device);
	if (result != NVML_SUCCESS) {
		printf("Could not get device handle for uuid %s\n", uuid);
	}
	return device;
}

nvmlDevice_t getNvmlDeviceByIndex(const int deviceID) {
	nvmlDevice_t device = nullptr;
	nvmlReturn_t result = nvmlDeviceGetHandleByIndex(deviceID, &device);
	if (result != NVML_SUCCESS) {
		printf("Could not get device handle for index %d\n", deviceID);
	}
	return device;
}

std::string getDevicePCIID(const int deviceID) {
	nvmlDevice_t device = getNvmlDeviceByIndex(deviceID);
	std::string res = "";
	if (device) {
		nvmlPciInfo_t pciInfo;
		memset(&pciInfo, 0, sizeof(pciInfo));

		nvmlReturn_t result = nvmlDeviceGetPciInfo(device, &pciInfo);
		if (NVML_SUCCESS == result) {
			res = std::string(pciInfo.busIdLegacy);
		}
	}
	return res;
}


ErrorCode queryNVLinkConnectedComponents(a7az0th::Device* devices, int numDevices) {

	LibHandle hMod = nullptr;
	const int res = initNVML(hMod);
	if (res) {
		return ErrorCode(nullptr, -1, "Failed to load NVML");
	}

	nvmlReturn_t err;
	err = nvmlInitialize();

	/// There are total of 6 maximum NVLink islands as of Mar.2020 so we loop on all of them.
	/// We find the link on which our current device is on
	/// Then we loop through all the other devices and look whether any of them has the same PCI ID
	/// Apparently this is what tells us whether they are in an NVLink connection
	const int NVML_NVLINK_MAX_LINKS = 6;
	for (int d = 0; d < numDevices; d++) {
		bool found = false;
		nvmlDevice_t device = getNvmlDeviceByIndex(devices[d].params.devId);

		for (int link = 0; link < NVML_NVLINK_MAX_LINKS; ++link) {
			unsigned int isSupported = 0;
			nvmlReturn_t result = nvmlDeviceGetNvLinkCapability(device, link, NVML_NVLINK_CAP_P2P_SUPPORTED, &isSupported);
			if (result != NVML_SUCCESS || !isSupported) {
				continue;
			}
			nvmlEnableState_t isActive = NVML_FEATURE_DISABLED;
			result = nvmlDeviceGetNvLinkState(device, link, &isActive);
			if (result != NVML_SUCCESS || isActive != NVML_FEATURE_ENABLED) {
				continue;
			}

			// Find the peer to which we are connected on this link
			nvmlPciInfo_t pci;
			memset(&pci, 0, sizeof(pci));
			result = nvmlDeviceGetNvLinkRemotePciInfo(device, link, &pci);
			if (result != NVML_SUCCESS) {
				continue;
			}

			// Loop though all devices to find the index of the peer.
			// We search by the PCI ID returned above
			std::string peerPciId(pci.busIdLegacy);
			for (int i = 0; i < numDevices; i++) {
				const int deviceID = devices[i].params.devId;
				const std::string pci = getDevicePCIID(deviceID);
				if (pci == peerPciId) {
					devices[d].params.nvLink |= 1 << deviceID;
					break;
				}
			}
		}
	}

	deinitNVML(hMod);
	return ErrorCode();
}
