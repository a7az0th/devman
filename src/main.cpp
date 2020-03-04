#include <stdio.h>
#include "devman.h"
#include "timer.h"
#include "progress.h"
#include "utils.h"
#include "version.h"
#include "threadman.h"

#include "nvml.h"

using namespace a7az0th;

void resetContexts() {
	CUcontext ctx=0;
	do {
		cuCtxGetCurrent(&ctx);
		cuCtxPopCurrent(NULL);
		cuCtxGetCurrent(&ctx);
	} while (ctx);
}

int main(int argc, char *argv[]) {
	printUsage();
	ProgressCallback progress;
#ifdef _DEBUG
	if (argc > 1) {
		progress.info("Entering debug loop...");
		while (true) {
			int a = 5;
		}
	}
#endif
	std::string driverVersion;
	ErrorCode err = getDriverVersionWithNVML(driverVersion);
	if (err.error()) {
		progress.error("%s",err.getError().ptr());
	} else {
		progress.info("NVidia driver version: %s", driverVersion.c_str());
	}

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	progress.info("%d devices found", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device &d = devman.getDevice(i);
		std::string info = d.getInfo();
		progress.info("%s", info.c_str());
	}

	Device* devices = &devman.getDevice(0);
	err = queryNVLinkConnectedComponents(devices, numDevices);
	if (err.error()) {
		progress.error("%s",err.getError().ptr());
	}

	for (int i = 0; i < numDevices; i++) {
		progress.info("Querying device[%d] for NVLink...", i);
		const unsigned nvLink = devices[i].params.nvLink;
		if (nvLink) {
			for (int j = 0; j < numDevices; j++) {
				if (nvLink & 1<<j) {
					progress.info("  Device[%d] is connected to Device[%d]", i, j);
				}
			}
		} else {
			progress.info("  Device[%d] has no NVLink connections", i);
		}
	}
	return 0;
}
