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

void launchWork(DeviceManager &devman, ThreadManager &threadman, ProgressCallback &progress) {

}

int main(int argc, char *argv[]) {
	ThreadManager threadman;
	ProgressCallback progress;
	progress.setLogLevel(ProgressCallback::LogLevel::debug);
	printVersion(progress);
	std::string driverVersion;
	ErrorCode err = getDriverVersionWithNVML(driverVersion);
	if (err.error()) {
		progress.error("%s",err.getError().c_str());
	} else {
		progress.info("NVidia driver version: %s", driverVersion.c_str());
	}

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	progress.info("%d device(s) found", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device &d = devman.getDevice(i);
		progress.info("Device[%d] : %s | SM:%2d | TCC:%d | CC:%d.%d | Freq:%d | Mem:%.1fGB",
			i,
			d.params.name.c_str(),
			d.params.multiProcessorCount,
			d.params.tccMode,
			d.params.ccmajor,
			d.params.ccminor,
			d.params.clockRate / 1000,
			float(d.params.memory) / float(1024 * 1024 * 1024)
		);
	}

	Device* devices = &devman.getDevice(0);
	err = queryNVLinkConnectedComponents(devices, numDevices);
	if (err.error()) {
		progress.error("%s",err.getError().c_str());
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
	devman.deinit();

	launchWork(devman, threadman, progress);
	return 0;
}
