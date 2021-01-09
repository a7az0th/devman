#pragma once

#include "error_code.h"

namespace a7az0th {
	struct Device;
	struct ProgressCallback;
}

ErrorCode getDriverVersionWithNVML(std::string &driverVersion);

ErrorCode queryNVLinkConnectedComponents(a7az0th::Device* devices, int numDevices);