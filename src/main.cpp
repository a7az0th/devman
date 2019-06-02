#include <stdio.h>
#include "devman.h"
#include "timer.h"

using namespace a7az0th;

int main() {

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	printf("%d devices found\n", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device &d = devman.getDevice(i);
		std::string info = d.getInfo();
		printf("%s\n", info.c_str());
	}
	Device &d = devman.getDevice(0);
	d.makeCurrent();
	d.setSource("D:/code/devman_full/devman/gpu_code/kernel.ptx");

	DeviceBuffer buff;
	buff.alloc(sizeof(int));

	Kernel kernel("dummy", d.getProgram());
	kernel.addParamPtr(buff.get());

	ThreadData tData(d);
	tData.launch(kernel, 1600);
	tData.wait();

	int res = -1;
	buff.download(&res);
	return 0;
}
