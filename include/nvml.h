#pragma once

#include "error_code.h"

struct ProgressCallback;
namespace a7az0th {
	struct Device;
}

ErrorCode getDriverVersionWithNVML(std::string &driverVersion);

ErrorCode queryNVLinkConnectedComponents(a7az0th::Device* devices, int numDevices, ProgressCallback &progress);